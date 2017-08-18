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
		puts("" );
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