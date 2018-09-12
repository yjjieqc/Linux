# 线程同步机制(二)--In glibc

在linux中进行多线程开发，同步是不可回避的一个问题。在POSIX标准中定义了三种线程同步机制: Mutexes(互斥量), Condition Variables(条件变量)和POSIX Semaphores(信号量)。NPTL基本上实现了POSIX，而glibc又使用NPTL作为自己的线程库。因此glibc中包含了这三种同步机制的实现(当然还包括其他的同步机制，如APUE里提到的读写锁)。

Glibc中常用的线程同步方式举例:

Semaphore：
变量定义：    sem_t sem;
初始化：      sem_init(&sem,0,1);
进入加锁:     sem_wait(&sem);
退出解锁:     sem_post(&sem);

Mutex:
变量定义：    pthread_mutex_t mut;
初始化：      pthread_mutex_init(&mut,NULL);
进入加锁:     pthread_mutex_lock(&mut);
退出解锁:     pthread_mutex_unlock(&mut);


这些用于同步的函数和futex有什么关系？下面让我们来看一看:
以Semaphores为例，
进入互斥区的时候，会执行sem_wait(sem_t *sem)，sem_wait的实现如下：
int sem_wait (sem_t *sem)
{
  int *futex = (int *) sem;
  if (atomic_decrement_if_positive (futex) > 0)
    return 0;
  int   err = lll_futex_wait (futex, 0);
    return -1;
)
atomic_decrement_if_positive()的语义就是如果传入参数是正数就将其原子性的减一并立即返回。如果信号量为正，在Semaphores的语义中意味着没有竞争发生，如果没有竞争，就给信号量减一后直接返回了。

如果传入参数不是正数，即意味着有竞争，调用lll_futex_wait(futex,0),lll_futex_wait是个宏，展开后为：
#define lll_futex_wait(futex, val) /
  ({                                          /
    ...
    __asm __volatile (LLL_EBX_LOAD                          /
              LLL_ENTER_KERNEL                          /
              LLL_EBX_LOAD                          /
              : "=a" (__status)                          /
              : "0" (SYS_futex), LLL_EBX_REG (futex), "S" (0),          /
            "c" (FUTEX_WAIT), "d" (_val),                  /
            "i" (offsetof (tcbhead_t, sysinfo))              /
              : "memory");                          /
    ...                                      /
  })
可以看到当发生竞争的时候，sem_wait会调用SYS_futex系统调用，并在val=0的时候执行FUTEX_WAIT,让当前线程休眠。

从这个例子我们可以看出，在Semaphores的实现过程中使用了futex，不仅仅是说其使用了futex系统调用(再重申一遍只使用futex系统调用是不够的)，而是整个建立在futex机制上，包括用户态下的操作和核心态下的操作。其实对于其他glibc的同步机制来说也是一样,都采纳了futex作为其基础。所以才会在futex的manual中说：对于大多数程序员不需要直接使用futexes，取而代之的是依靠建立在futex之上的系统库，如NPTL线程库(most programmers will in fact not be using futexes directly but instead rely on system libraries built on them, such as the NPTL pthreads implementation)。所以才会有如果在编译内核的时候不 Enable futex support，就"不一定能正确的运行使用Glibc的程序"。

小结:
1. Glibc中的所提供的线程同步方式，如大家所熟知的Mutex,Semaphore等，大多都构造于futex之上了，除了特殊情况，大家没必要再去实现自己的futex同步原语。
2. 大家要做的事情，似乎就是按futex的manual中所说得那样: 正确的使用Glibc所提供的同步方式，并在使用它们的过程中，意识到它们是利用futex机制和linux配合完成同步操作就可以了。
