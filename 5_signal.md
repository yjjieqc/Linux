信号
5.1 信号的概念
5.1.1 信号编号
kill -l
xingwenpeng@ubuntu:~$ kill -l
1) SIGHUP 2) SIGINT 3) SIGQUIT 4) SIGILL 5) SIGTRAP
6) SIGABRT 7) SIGBUS 8) SIGFPE 9) SIGKILL 10) SIGUSR1
11) SIGSEGV 12) SIGUSR2 13) SIGPIPE 14) SIGALRM 15) SIGTERM
16) SIGSTKFLT 17) SIGCHLD 18) SIGCONT 19) SIGSTOP 20) SIGTSTP
21) SIGTTIN 22) SIGTTOU 23) SIGURG 24) SIGXCPU 25) SIGXFSZ
26) SIGVTALRM 27) SIGPROF 28) SIGWINCH 29) SIGIO 30) SIGPWR
31) SIGSYS 34) SIGRTMIN 35) SIGRTMIN+1 36) SIGRTMIN+2 37) SIGRTMIN+3
38) SIGRTMIN+4 39) SIGRTMIN+5 40) SIGRTMIN+6 41) SIGRTMIN+7 42) SIGRTMIN+8
43) SIGRTMIN+9 44) SIGRTMIN+10 45) SIGRTMIN+11 46) SIGRTMIN+12 47) SIGRTMIN
+13
48) SIGRTMIN+14 49) SIGRTMIN+15 50) SIGRTMAX-14 51) SIGRTMAX-13 52) SIGRTMAX-12
53) SIGRTMAX-11 54) SIGRTMAX-10 55) SIGRTMAX-9 56) SIGRTMAX-8 57) SIGRTMAX-7
58) SIGRTMAX-6 59) SIGRTMAX-5 60) SIGRTMAX-4 61) SIGRTMAX-3 62) SIGRTMAX-2
63) SIGRTMAX-1 64) SIGRTMAX
5.1.2 信号机制
man 7 signal
Term Default action is to terminate the process.
Ign Default action is to ignore the signal.
Core Default action is to terminate the process and dump core (see core(5)).
Stop Default action is to stop the process.
Cont Default action is to continue the process if it is currently stopped.
First the signals described in the original POSIX.1-1990 standard.
Signal Value Action Comment
SIGHUP 1 Term Hangup detected on controlling terminal
or death of controlling process
SIGINT 2 Term Interrupt from keyboard
SIGQUIT 3 Core Quit from keyboard
SIGILL 4 Core Illegal Instruction
SIGABRT 6 Core Abort signal from abort(3)
SIGFPE 8 Core Floating point exception
SIGKILL 9 Term Kill signal
SIGSEGV 11 Core Invalid memory reference
SIGPIPE 13 Term Broken pipe: write to pipe with no
readers
SIGALRM 14 Term Timer signal from alarm(2)
SIGTERM 15 Term Termination signal
SIGUSR1 30,10,16 Term User-defined signal 1
SIGUSR2 31,12,17 Term User-defined signal 2
SIGCHLD 20,17,18 Ign Child stopped or terminated
SIGCONT 19,18,25 Cont Continue if stopped
SIGSTOP 17,19,23 Stop Stop process
SIGTSTP 18,20,24 Stop Stop typed at tty
SIGTTIN 21,21,26 Stop tty input for background process
SIGTTOU 22,22,27 Stop tty output for background process
The signals SIGKILL and SIGSTOP cannot be caught, blocked, or ignored.
表中第一列是各信号的宏定义名称，第二列是各信号的编号，第三列是默认处理动作:
Term表示终止当前进程.
Core表示终止当前进程并且Core Dump（Core Dump 用于gdb调试）.
Ign表示忽略该信号.
Stop表示停止当前进程.
Cont表示继续执行先前停止的进程.
表中最后一列是简要介绍，说明什么条件下产生该信号。
5.1.3 信号产生种类
ctl+c SIGINT
ctl+z SIGTSTP
ctl+\ SIGQUIT
终端特殊按键
硬件异常
* 除0操作
* 访问非法内存