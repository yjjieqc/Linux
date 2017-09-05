# 信号

## 5.1 信号的概念

### 5.1.1 信号编号

查看linux系统信号

    kill -l

xingwenpeng@ubuntu:~$ kill -l

-----|-------|--------|--------|---------
1) SIGHUP |2) SIGINT |3) SIGQUIT| 4) SIGILL |5) SIGTRAP
6) SIGABRT |7) SIGBUS |8) SIGFPE |9) SIGKILL |10) SIGUSR1
11) SIGSEGV |12) SIGUSR2 |13) SIGPIPE |14) SIGALRM |15) SIGTERM
16) SIGSTKFLT |17) SIGCHLD| 18) SIGCONT| 19) SIGSTOP| 20) SIGTSTP
21) SIGTTIN |22) SIGTTOU |23) SIGURG |24) SIGXCPU |25) SIGXFSZ
26) SIGVTALRM |27) SIGPROF |28) SIGWINCH |29) SIGIO |30) SIGPWR
31) SIGSYS| 34) SIGRTMIN |35) SIGRTMIN+1 |36) SIGRTMIN+2| 37) SIGRTMIN+3
38) SIGRTMIN+4 |39) SIGRTMIN+5 |40) SIGRTMIN+6 |41) SIGRTMIN+7 |42) SIGRTMIN+8
43) SIGRTMIN+9| 44) SIGRTMIN+10 |45) SIGRTMIN+11| 46) SIGRTMIN+12 |47) SIGRTMIN+13
48) SIGRTMIN+14 |49) SIGRTMIN+15| 50) SIGRTMAX-14 |51) SIGRTMAX-13| 52) SIGRTMAX-12
53) SIGRTMAX-11 |54) SIGRTMAX-10 |55) SIGRTMAX-9 |56) SIGRTMAX-8 |57) SIGRTMAX-7
58) SIGRTMAX-6 |59) SIGRTMAX-5| 60) SIGRTMAX-4 |61) SIGRTMAX-3| 62) SIGRTMAX-2
63) SIGRTMAX-1 |64) SIGRTMAX


### 5.1.2 信号机制

man 7 signal

Term Default action is to terminate the process.
Ign Default action is to ignore the signal.
Core Default action is to terminate the process and dump core (see core(5)).
Stop Default action is to stop the process.
Cont Default action is to continue the process if it is currently stopped.
First the signals described in the original POSIX.1-1990 standard.

Signal Value Action Comment

----|---|---
SIGHUP |1 |Term| Hangup detected on controlling terminal or death of controlling process
SIGINT |2| Term| Interrupt from keyboard
SIGQUIT |3 |Core |Quit from keyboard
SIGILL| 4 |Core |Illegal Instruction
SIGABRT| 6| Core| Abort signal from abort(3)
SIGFPE |8| Core| Floating point exception
SIGKILL |9 |Term |Kill signal
SIGSEGV |11 |Core| Invalid memory reference
SIGPIPE |13 |Term| Broken pipe: write to pipe with no readers
SIGALRM |14| Term| Timer signal from alarm(2)
SIGTERM| 15| Term| Termination signal
SIGUSR1 |30,10,16 |Term| User-defined signal 1
SIGUSR2 |31,12,17| Term| User-defined signal 2
SIGCHLD |20,17,18| Ign |Child stopped or terminated
SIGCONT |19,18,25 |Cont| Continue if stopped
SIGSTOP| 17,19,23| Stop| Stop process
SIGTSTP |18,20,24 |Stop |Stop typed at tty
SIGTTIN| 21,21,26 |Stop |tty input for background process
SIGTTOU |22,22,27| Stop| tty output for background process
The signals SIGKILL and SIGSTOP cannot be caught, blocked, or ignored.

表中第一列是各信号的宏定义名称，第二列是各信号的编号，第三列是默认处理动作:
Term表示终止当前进程。

Core表示终止当前进程并且Core Dump（Core Dump 用于gdb调试）。
Ign表示忽略该信号。

Stop表示停止当前进程。

Cont表示继续执行先前停止的进程。

表中最后一列是简要介绍，说明什么条件下产生该信号。

###  5.1.3 信号产生种类

####  终端特殊按键

ctl+c SIGINT
ctl+z SIGTSTP
ctl+\ SIGQUIT

####  硬件异常
* 除0操作
* 访问非法内存

####  kill函数或者kill命令

不过，kill向调用者返回测试结果时，原来存在的被测试结果可能刚终止

    int kill (pid_t pid, int sig)
    	pid > 0
    		sig发送给ID为pid的进程
    	pid == 0
    		sig发送给与发送进程同组的所有进程
    	pid < 0
    		sig发送给组ID为|-pid|的进程，并且发送进程具有向起发送信号的权限
    	pid = -1
    		sig发送给发送进程有权限向他们发送信号的系统上的所有进程
    	sig为0时，用于检测，特定的pid进程是否存在，返回-1

除此之外还有`raise`和`abort`函数

    int raise(int sig)	//自己向自己发送信号
    void abort(void)	//调用进程向自己发送SIGABRT信号

#### 某种软件条件已发生

定时器`alarm`到时时，进程向自己发送SIGALRM信号，每个进程只有一个定时器

    
    unsigned int alarm(unsigned int seconds)	//返回值为未到达的秒数，到达设定时间后返回SIGALRM信号

例：

    #include <unsitd.h>
    #include <stdio.h>
    
    int main(void){
    int counter;
    alarm(1);
    for(counter = 0; 1; counter++)
    	printf("counter = %d \n", counter);
    return 0;
    }

管道读端关闭，写端写数据

### 5.1.4  信号产生的原因

* SIGHUP：当用户推出shell时，由该shell启动的所有进程将收到这个信号，默认动作为终止进程
* SIGINT：当用户按下<Ctrl+C>组合键时，用户终端向正在运行中的由该终端启动的程序发送此信号。默认动作为终止进程。


## 5.2  进程处理信号行为

manpage里信号3种处理方式：

SIG_IGN
SIG_DFL
a signal handling function

进程处理信号的行为：

1. 默认处理动作

	term

	core

		gcc -g file.c
		ulimit -c 1024
		gdb a.out core
		进程死之前的内存情况，进程异常退出时检查

	ign

	stop

	cont

2.忽略

3.捕捉（用户自定义信号处理函数）

## 5.3  信号集处理函数

sigset_t 为信号集，可sizeof(sigset_t)查看

    int sigemptyset(sigset_t *set)	//信号集全部置0
    int sigfillset(sigset_t *set)	//信号集全部置1
    int sigaddset(sigset_t *set, int signo)	//某一位置0或置1
    int sigdelset(sigset_t *set, int signo)
    int sigismember(const sigset_t *set, int signo)

## 5.4  PCB信号集

信号在内核中的表示示意图，画图

如果在进程解除对某信号的阻塞之前这种信号产生多次，将如何处理？POSIX.1允许系统传递该信号一次或者多次。Linux是这样实现的：常规信号在递达之前产生多次只记一次，而实时信号在递达之前产生多次可以依次放在一个队列里面。本章不讨论实时信号。而从上图来看，一个信号只有一个bit的未决标志，非0即1，不记录该信号产生了多少次，阻塞标志也是这样表示的。因此，未决和阻塞标志可以使用相同的sigset_t来储存，sigset_t也被称作信号集，这个类型可以表示每个信号的“有效”和“无效”状态，在未决信号集中，“有效”和“无效”的含义是该信号是否处于未决状态；而在阻塞信号集中，“有效”和“无效”的含义是该信号是否被阻塞。

阻塞信号集也叫做当前进程的信号屏蔽字（Signal Mask)，这里的屏蔽应理解为阻塞而不是忽略。

未决态：信号产生，但没被相应

递达态：信号产生并被相应

PEND未决信号集合|Block阻塞信号集合
-----------|-----------
用户不可设置，内核自动设置，用户可读|用户可以设置

### 5.4.1  sigprocmask

调用函数`sigprocmask`可以读取或更改进程的信号屏蔽字。

	#include <signal.h>
	
	int sigprocmask(int how, const sigset_t, sigset_t *oset);
	//返回值：若成功则为0，若出错则为-1

如果oset是非空指针，则读取进程当前信号屏蔽字并通过oset参数传出。如果set是非空指针，则更改进程的信号屏蔽字，参数how知识如何修改。如果oset和set都是非空指针，则先将原来的i新年好屏蔽字备份到oset里，然后根据set和how参数更改信号屏蔽字。假设当前的信号屏蔽字为mask，下表说明了how参数的可选值。

how参数的含义

	SIG_BLOCK		set包含了我们希望添加到当前信号屏蔽字里的信号，相当于mask=mask|set
	SIG_UNBLOCK	set包含了我们希望从当前屏蔽字中解除阻塞的信号，相当于mask=mask&~set
	SIG_SETMASK	设置当前信号屏蔽字为set所指向的值，相当于mask=set

如果调用`sigprocmask`解除了对当前若干个未决信号的阻塞，则在`sigprocmask`返回前，至少将其中一个信号递达。

### 5.4.2  sigpending

	#include <signal.h>
	
	int sigpending(sigset_t *set);

`sigpending`用于读取当前进程的未决信号集，通过`set`参数导出。调用成功返回0，失败返回-1。

下面用上面的函数做个实验。

	#include <signal.h>
	#include <stdio.h>
	
	void printsigset(const sigset_t *set)
	{
		int i;
		for(i = 1; i < 32; i++)
			if(sigismember(set, i ) == 1)
				putchar('1');
			else
				putchar('0');
		puts("");  
	}
	
	int main(void){
		sigset_t s, p;
		sigemptyset(&s);
		sigaddset(&s, SIGINT);
		sigprocmask(SIG_BLOCK, &s, NULL);
		while(1) {
			sigpending(&p);
			printsigset(&p);
			sleep(1);
		}
		return 0;
	}

## 5.5  信号捕捉设定

	#include <signal.h>
	
	int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
	
	struct sigaction{
		void (*sa_handler)(int);
		void (*sa_sigaction)(int, siginfo_t *, void *);	//两者互斥
		sigset_t sa_mask;
		int sa_flags;
		void (*sa_restorer)(void);
	};

	sa_handler : 早期的捕捉函数
	sa_sigaction : 新添加的捕捉函数，可以传参，和sa_handler互斥，两者通过sa_flags选择采用哪种捕捉函数
	sa_mask : 在执行捕捉函数时，设置阻塞其他信号，sa_mask | 进程阻塞信号集，退出捕捉函数后，还原回原有的阻塞信号集
	sa_flags : SA_SIGINFO 或者0
	sa_restorer : 保留，已过时

举例SA_SIGINT被捕捉：

当前进程从内核返回用户空间代码前检查是否有信号递达，有则去响应。

### 5.5.1  利用SIGUSR1和SIGUSR2实现父子进程同步输出

注意：子进程继承了父进程的信号屏蔽字和信号处理动作。

## 5.6  C标准库信号处理函数

	typedef void (*sighandler_t)(int)
	sighandler_t signal(int signum, sighandler_t handler)
	
	int system(const char * command)
	集合fork,exec,wait一体

### 5.6.1  signal

	#include <stdio.h>
	#include <signal.h>
	
	void do_sig(int n)
	{
	        printf("Hello\n");
	}
	
	int main(void)
	{
	        signal(SIGINT, do_sig);
	        while(1)
	        {
	                printf("***********\n");
	                sleep(1);
	        }
	}

优点使用简单，缺点是不能设置屏蔽字，功能没有`sigaction`功能强大

## 5.7  可重入函数

可重入：可重新进入函数

* 不含全局变量和静态变量是可重入函数的一个要素
* 可重入函数见man 7 signal
* 在信号捕捉函数里面应该使用可重入函数
* 在信号捕捉函数里禁止调用不可重入函数

例如：`strtok`就是一个不可重入函数，因为`strtok`内部维护了一个内部静态指针，保存上次切割到的位置，如果信号的捕捉函数中也调用了`strtok`函数，则会造成切割字符串混乱，应用`strtok_r`版本，r表示可重入。

	#include <stdio.h>
	#include <string.h>
	
	int main(void)
	{
		char buf[] = "hello world";
		char *save = buf, *p;
	
		while((p = strtok_r(save, " ", &save))!= NULL)
			printf("%s\n", p);

		return 0;
	}

## 5.8  信号引起的竟态和异步I/O

### 5.8.1  时序竟态

	int pause(void)
		使用进程挂起，直到有信号递达，如果递达信号是忽略，则继续挂起
	int sigsuspend(const sigset_t *mask)
		以通过制定mask来临时解除对某个信号的屏蔽；
		然后挂起等待；
		当sigsuspend返回时，进程的信号屏蔽字恢复为原来的值

`mysleep`实现，这种方式是否存在BUG？

	#include <stdio.h>
	#include <signal.h>
	
	int mysleep(int n)
	{
		signal(SIGALRM, do_sig);
		alarm(n);	//定时n秒
		pause();
	}
	void do_sig(int n)
	{
		
	}
	
	int main(void)
	{
		struct sigaction act;
	
		//act.sa_handler = do_sig;
		//act.sa_handler = SIG_IGN;
		act.sa_handler = SIG_DFL;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
	
		sigaction(SIGUSR1, &act, NULL);
	
		pause();
	
		return 0;
	}

### 5.8.2  全局变量异步I/O



### 5.8.3  可重入函数



### 5.8.4  避免异步I/O的类型

sig_atomic_t		//平台下的原子类型

volatile			//防止编译器开启优化选项时，优化对内存的读写

## 5.9  SIGCHLD信号处理

### 5.9.1  SIGCHLD信号处理

子进程终止时

子进程接收到SIGCHLD信号停止时

子进程处在停止态，接收到SIGCHLD后唤醒时

### 5.9.2  status处理方式

	pid_t waitpid(pid_t pid, int *status, int options)
	options
		WNOHANG
			没有子进程结束，立即返回
		WUNTRACED
			如果子进程由于被停止产生的SIGCHLD， waitpid则立即返回
		WCONTINUED
			如果子进程由于被SIGCONT唤醒而产生的SIGCHLD， waitpid则立即返回
	获取status
		WIFEXITED(status)
			子进程正常exit终止，返回真
				WEXITSTATUS(status)返回子进程正常退出值
		WIFSIGNALED(status)
			子进程被信号终止，返回真
				WTERMSIG(status)返回终止子进程的信号值
		WIFSTOPPED(status)
			子进程被停止，返回真
				WSTOPSIG(status)返回停止子进程的信号值
		WIFCONTINUED(status)
			子进程由停止态转为就绪态，返回真

举例：

	#include <stdio.h>
	#include <signal.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <errno.h>
	#include <wait.h>
	#include <sys/types.h>
	
	void do_sig_child(int signo){
	/*      while(!(waitpid(0, NULL, WNOHANG)== -1))
	        ;
	*/
	        int status;
	        pid_t pid;
	
	        while(waitpid(0, &status,WNOHANG)>0){
	                if(WIFEXITED(status))
	                        printf("child %d exit %d\n",pid, WEXITSTATUS(status));
	                else if(WIFSIGNALED(status))
	                        printf("child %d cancle signal %d\n", pid, WTERMSIG(status));
	}
	}
	void sys_err(char *str)
	{
	        perror(str);
	        exit(1);
	}
	
	int main(void){
	        pid_t pid;
	        int i;
	        for (i = 0; i< 10; i++)
	        {
	                if((pid = fork()) == 0)
	                        break;
	                else if(pid< 0)
	                        sys_err("fork");
	        }
	
	        if(pid == 0) {
	        int n = 3;
	        while(n --) {
	                printf("child ID %d\n", getpid());
	                sleep(1);
	                }
	        return i;
	        }
	        else if(pid > 0){
	        struct sigaction act;
	        act.sa_handler = do_sig_child;
	        sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
	
	        sigaction(SIGCHLD, &act, NULL);
	        while(1) {
	                printf("parent ID %d\n", getpid());
	                sleep(1);
	                }
	        }
	
	}


### 5.10.1  sigqueue

	int sigqueue(pid_t pid, int sig, const union sigval value)
	
	union sigbal{
		int sival_int;
		void *sival_ptr;
	}

### 5.10.2  sigaction

	void (*sa_sigaction)(int, siginfo_t *, void *)

	siginfo_t {
		int si_int;
		void 	*siptr;
		sigval_t	si_value;
		...
	}

	sa_flags = SA_SIGINFO

实例

* 进程自己收发信号，在同一地址空间
* 不同进程之间收发信号，不在同一地址空间，不适合传地址

## 5.11  信号中断系统调用

read阻塞时，信号中断系统调用：

- 返回部分读到的数据
- read调用失败，errno设置成EINTER


