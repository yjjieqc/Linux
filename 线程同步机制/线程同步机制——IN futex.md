# [Pthread] Linux中的线程同步机制(一) -- Futex


## 引子
在编译2.6内核的时候，你会在编译选项中看到[*] Enable futex support这一项，上网查，有的资料会告诉你"不选这个内核不一定能正确的运行使用glibc的程序"，那futex是什么？和glibc又有什么关系呢？

## 1. 什么是Futex
Futex是Fast Userspace muTexes的缩写，由Hubertus Franke, Matthew Kirkwood, Ingo Molnar and Rusty Russell共同设计完成。几位都是linux领域的专家，其中可能Ingo Molnar大家更熟悉一些，毕竟是O(1)调度器和CFS的实现者。

Futex按英文翻译过来就是快速用户空间互斥体。其设计思想其实不难理解，在传统的Unix系统中，System V IPC(inter process communication)，如 semaphores, msgqueues, sockets还有文件锁机制(flock())等进程间同步机制都是对一个内核对象操作来完成的，这个内核对象对要同步的进程都是可见的，其提供了共享的状态信息和原子操作。当进程间要同步的时候必须要通过系统调用(如semop())在内核中完成。可是经研究发现，很多同步是无竞争的，即某个进程进入互斥区，到再从某个互斥区出来这段时间，常常是没有进程也要进这个互斥区或者请求同一同步变量的。但是在这种情况下，这个进程也要陷入内核去看看有没有人和它竞争，退出的时侯还要陷入内核去看看有没有进程等待在同一同步变量上。这些不必要的系统调用(或者说内核陷入)造成了大量的性能开销。为了解决这个问题，Futex就应运而生，Futex是一种用户态和内核态混合的同步机制。首先，同步的进程间通过mmap共享一段内存，futex变量就位于这段共享的内存中且操作是原子的，当进程尝试进入互斥区或者退出互斥区的时候，先去查看共享内存中的futex变量，如果没有竞争发生，则只修改futex,而不用再执行系统调用了。当通过访问futex变量告诉进程有竞争发生，则还是得执行系统调用去完成相应的处理(wait 或者 wake up)。简单的说，futex就是通过在用户态的检查，如果了解到没有竞争就不用陷入内核了，大大提高了low-contention时候的效率。Linux从2.5.7开始支持Futex。

## 2. Futex系统调用
Futex是一种用户态和内核态混合机制，所以需要两个部分合作完成，linux上提供了sys_futex系统调用，对进程竞争情况下的同步处理提供支持。

其原型和系统调用号为

```c
    #include <linux/futex.h>
    #include <sys/time.h>
    int futex (int *uaddr, int op, int val, const struct timespec *timeout,int *uaddr2, int val3);
    #define __NR_futex              240
```
  
    虽然参数有点长，其实常用的就是前面三个，后面的timeout大家都能理解，其他的也常被ignore。
    uaddr就是用户态下共享内存的地址，里面存放的是一个对齐的整型计数器。
    op存放着操作类型。定义的有5中，这里我简单的介绍一下两种，剩下的感兴趣的自己去man futex
    FUTEX_WAIT: 原子性的检查uaddr中计数器的值是否为val,如果是则让进程休眠，直到FUTEX_WAKE或者超时(time-out)。也就是把进程挂到uaddr相对应的等待队列上去。
    FUTEX_WAKE: 最多唤醒val个等待在uaddr上进程。
    
    可见FUTEX_WAIT和FUTEX_WAKE只是用来挂起或者唤醒进程，当然这部分工作也只能在内核态下完成。有些人尝试着直接使用futex系统调用来实现进程同步，并寄希望获得futex的性能优势，这是有问题的。应该区分futex同步机制和futex系统调用。futex同步机制还包括用户态下的操作，我们将在下节提到。
        
## 3. Futex同步机制
所有的futex同步操作都应该从用户空间开始，首先创建一个futex同步变量，也就是位于共享内存的一个整型计数器。

当进程尝试持有锁或者要进入互斥区的时候，对futex执行"down"操作，即原子性的给futex同步变量减1。如果同步变量变为0，则没有竞争发生，进程照常执行。如果同步变量是个负数，则意味着有竞争发生，需要调用futex系统调用的futex_wait操作休眠当前进程。

当进程释放锁或者要离开互斥区的时候，对futex进行"up"操作，即原子性的给futex同步变量加1。如果同步变量由0变成1，则没有竞争发生，进程照常执行。如果加之前同步变量是负数，则意味着有竞争发生，需要调用futex系统调用的futex_wake操作唤醒一个或者多个等待进程。

这里的原子性加减通常是用CAS(Compare and Swap)完成的，与平台相关。CAS的基本形式是：CAS(addr,old,new),当addr中存放的值等于old时，用new对其替换。在x86平台上有专门的一条指令来完成它: cmpxchg。

可见: futex是从用户态开始，由用户态和核心态协调完成的。

## 4. 进/线程利用futex同步
进程或者线程都可以利用futex来进行同步。

对于线程，情况比较简单，因为线程共享虚拟内存空间，虚拟地址就可以唯一的标识出futex变量，即线程用同样的虚拟地址来访问futex变量。

对于进程，情况相对复杂，因为进程有独立的虚拟内存空间，只有通过mmap()让它们共享一段地址空间来使用futex变量。每个进程用来访问futex的虚拟地址可以是不一样的，只要系统知道所有的这些虚拟地址都映射到同一个物理内存地址，并用物理内存地址来唯一标识futex变量。 
      
小结：

- Futex变量的特征：1)位于共享的用户空间中 2)是一个32位的整型 3)对它的操作是原子的
- Futex在程序low-contention的时候能获得比传统同步机制更好的性能。
- 不要直接使用Futex系统调用。
- Futex同步机制可以用于进程间同步，也可以用于线程间同步。