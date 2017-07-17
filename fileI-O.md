## 1.2 PCB概念——进程控制块

### 1.2.1 task_struct结构

	定义位置：/usr/src/linux-headers/include/linux/sched.h

### 1.2.2 files_struct结构体
	
	相当于一张表，或者指针，文件描述符表
	FILE *		fopen("abc")

## 1.3 open/close

### 1.3.1 文件描述符

	一个进程默认打开3个文件描述符
	STDIN_FILENO 0
	STDOUT_FILENO 1
	STDERR_FILENO 2

	open函数可以打开或者创建一个文件
使用open函数，需要包含以下三个头文件
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	
	最后的可变参数可以是0个或1个,由flags参数中的标志位决定,见下面的详细说明。
	pathname参数是要打开或创建的文件名,和fopen一样,pathname既可以是相对路径也可以是绝对路径。flags参数有一系列常数值可供选择,可以同时选择多个常数用按位或运算符连接起来,所以这些常数的宏定义都以O_开头,表示or。
必选项:以下三个常数中必须指定一个,且仅允许指定一个。
 * O_RDONLY 只读打开
 * O_WRONLY 只写打开
 * O_RDWR 可读可写打开
以下可选项可以同时指定0个或多个,和必选项按位或起来作为flags参数。可选项有很多,这里只介绍一部分,其它选项可参考open(2)的Man Page:
 * O_APPEND 表示追加。如果文件已有内容,这次打开文件所写的数据附加到文件的末尾而不覆盖原来的内容。
 * O_CREAT 若此文件不存在则创建它。使用此选项时需要提供第三个参数mode,表示该文件的访问权限。
 * O_EXCL 如果同时指定了O_CREAT,并且文件已存在,则出错返回。
 * O_TRUNC 如果文件已存在,并且以只写或可读可写方式打开,则将其长度截断(Trun-cate)为0字节。
 * O_NONBLOCK 对于设备文件,以O_NONBLOCK方式打开可以做非阻塞I/O(Nonblock I/O),非阻塞I/O在下一节详细讲解。
	open函数和C标准I/O库的fopen函数细微区别在于：
		以可写方式fopen一个文件时，如果文件不存在则会自动创建，而open一个文件时必须使用O_CREAT才会创建文件，否则文件不存在就出错返回。
		以w或者w+方式fopen一个文件时，如果文件存在就截断为0字节，而open一个文件必须明确指定O_TRUNC才会截断文件，否则在原来的数据上改写。
		以三个参数mode指定文件权限，可以用八进制数表示，比如0644表示-rw-r--r--,也可以用S_IRUSR、S_IWUSR等宏定义按位或起来表示。要注意的是，文件权限由open的mode参数和当前进程的umask共同决定。

	close函数
	
	最大文件打开个数
	默认一个进程最多可以打开1024个文件，可以使用
	cat /proc/sys/fs/file-max
	通过ulimit -a可以查看和修改文件打开个数
	ulimit -n 4096

## 1.4 read/write

	#include <unistd.h>

	ssize_t read(int fd, void *buf, size_t count);
	返回值：成功返回读取的字节数，出错则返回-1并设置errno，如果在调用read之前已经到达文件末尾，则这次read返回0；
	ssize_t 有符号int，size_t 无符号int
	参数count是请求读取的字节数,读上来的数据保存在缓冲区buf中,同时文件的当前读写位置向后移。注意这个读写位置和使用C标准I/O库时的读写位置有可能不同,这个读写位置是记在内核中的,而使用C标准I/O库时的读写位置是用户空间I/O缓冲区中的位置。比如用fgetc读一个字节,fgetc有可能从内核中预读1024个字节到I/O缓冲区中,再返回第一个字节,这时该文件在内核中记录的读写位置是1024,而在FILE结构体中记录的读写位置是1。注意返回值类型是ssize_t,表示有符号的size_t,这样既可以返回正的字节数、0(表示到达文件末尾)也可以返回负值-1(表示出错)。read函数返回时,返回值说明了buf中前多少个字节是刚读上来的。有些情况下,实际读到的字节数(返回值)会小于请求读的字节数count,例如:
	读常规文件时,在读到count个字节之前已到达文件末尾。例如,距文件末尾还有30个字节而请求读100个字节,则read返回30,下次read将返回0。
	从终端设备读,通常以行为单位,读到换行符就返回了。
	从网络读,根据不同的传输层协议和内核缓存机制,返回值可能小于请求的字节数,后面socket编程部分会详细讲解。

	write函数向打开的设备或文件中写数据。
	#include <unistd.h>

	ssize_t write(int fd, void *buf, size_t count);
	返回值：成功返回写入的字节数，出错返回-1并设置errno
	写常规文件时,write的返回值通常等于请求写的字节数count,而向终端设备或网络写则不一定。

## 1.5 阻塞和非阻塞

	读常规文件是不会阻塞的，不管读多少字节，read都会在有限时间内返回。从终端设备和网络读则不一定，如果从终端输入的数据 没有换行符，调用read读终端的设备就会阻塞，如果网络上没有接收到数据包，调用read从网络读就会阻塞，至于阻塞多长时间是不确定的，如果一直没有数据到达就一直阻塞在哪里。同样，写常规文件也是不会阻塞的，而向终端设备或网络写则不一定。
	现在明确一下阻塞(Block)这个概念。当进程调用一个阻塞的系统函数时,该进程被置于睡眠(Sleep)状态,这时内核调度其它进程运行,直到该进程等待的事件发生了(比如网络上接收到数据包,或者调用sleep指定的睡眠时间到了)它才有可能继续运行。与睡眠状态相对的是运行(Running)状态,在Linux内核中,处于运行状态的进程分为两种情况:
正在被调度执行。CPU处于该进程的上下文环境中,程序计数器(eip)里保存着该进程的指令地址,通用寄存器里保存着该进程运算过程的中间结果,正在执行该进程的指令,正在读写该进程的地址空间。
	就绪状态。该进程不需要等待什么事件发生,随时都可以执行,但CPU暂时还在执行另一个进程,所以该进程在一个就绪队列中等待被内核调度。系统中可能同时有多个就绪的进程,那么该调度谁执行呢?内核的调度算法是基于优先级和时间片的,而且会根据每个进程的运行情况动态调整它的优先级和时间片,让每个进程都能比较公平地得到机会执行,同时要兼顾用户体验,不能让和用户交互的进程响应太慢。
	下面这个小程序从终端读数据再写回终端。

### 1.5.1 阻塞读终端

<pre>
	#include <unistd.h>
	#include <stdlib.h>
	int main(void)
	{
	char buf[10];
	int n;
	n = read(STDIN_FILENO, buf, 10);
	if (n < 0) {
	perror("read STDIN_FILENO");
	exit(1);
	}
	write(STDOUT_FILENO, buf, n);
	return 0;
 }
<code>

### 1.5.2  非阻塞读终端
	轮寻模式

### 1.5.3  非阻塞读终端和等待超时

## 1.6  lseek

类似于标准I/O库的fseek函数类似，可以移动当前读写位置，但有一个例外，如果文件以O_APEND模式打开，每次写操作都会在文件末尾追加数据，然后讲读写位置移动到新的文件末尾。
fseek函数在底层调用lseek实现。

### lseek

 #include <sys/types.h>
 #include <unistd.h>
 
 off_t lseek(int fd, off_t offset, int whence);

额外功能：
1、拓展一个文件，一定要有一次写操作
2、同时可以获取文件大小

### fseek函数

 #include <stdio.h>
 
 int fseek(FILE *stream, long offset, int whence);
 
 long ftell(FILE *stream);
 
 void rewind(FILE *stream);
 
 int fgetpos(FILE *stream, fpos_t *pos);
 int fsetpos(FILE *stream, const fpos_t *pos);

# 1.7 fcntl

获取和设置文件访问属性
<pre>
 #include <unistd.h>
 #include <fcntl.h>

 int fcntl(int fd, int cmd, ... /* arg */ );
 int fcntl(int fd, int cmd);
 int fcntl(int fd, int cmd, long arg);
 int fcntl(int fd, int cmd, struct flock *lock);
<code>

# 1.8 ioctl

ioctl用于向设备发控制和配置命令，有些命令也需要读写一些数据，但是这些数据是不能用read/write读写的，称为Out-of-Band数据。也就是说，read/write读写数据是in-band数据，是I/O操作的主体。













# 15  错误处理机制

## errno

 #define	EPERM		 1	/* Operation not permitted */
 #define	ENOENT		 2	/* No such file or directory */
 #define	ESRCH		 3	/* No such process */
 #define	EINTR		 4	/* Interrupted system call */
 #define	EIO		 5	/* I/O error */
 #define	ENXIO		 6	/* No such device or address */
 #define	E2BIG		 7	/* Argument list too long */
 #define	ENOEXEC		 8	/* Exec format error */
 #define	EBADF		 9	/* Bad file number */
 #define	ECHILD		10	/* No child processes */
 #define	EAGAIN		11	/* Try again */
 #define	ENOMEM		12	/* Out of memory */
 #define	EACCES		13	/* Permission denied */
 #define	EFAULT		14	/* Bad address */
 #define	ENOTBLK		15	/* Block device required */
 #define	EBUSY		16	/* Device or resource busy */
 #define	EEXIST		17	/* File exists */
 #define	EXDEV		18	/* Cross-device link */
 #define	ENODEV		19	/* No such device */
 #define	ENOTDIR		20	/* Not a directory */
 #define	EISDIR		21	/* Is a directory */
 #define	EINVAL		22	/* Invalid argument */
 #define	ENFILE		23	/* File table overflow */
 #define	EMFILE		24	/* Too many open files */
 #define	ENOTTY		25	/* Not a typewriter */
 #define	ETXTBSY		26	/* Text file busy */
 #define	EFBIG		27	/* File too large */
 #define	ENOSPC		28	/* No space left on device */
 #define	ESPIPE		29	/* Illegal seek */
 #define	EROFS		30	/* Read-only file system */
 #define	EMLINK		31	/* Too many links */
 #define	EPIPE		32	/* Broken pipe */
 #define	EDOM		33	/* Math argument out of domain of func */
 #define	ERANGE		34	/* Math result not representable */

## perror

 #include <stdio.h>
 void perror(const char *s);
 #include <errno.h>
 const char *sys_errlist[];
 int sys_nerr;
 int errno;

## strerror
 #include <string.h>
 
 char **strerror(int errnum);
 
 int strerror_r(int errnum, char *buf, size_t buflen);
 /* XSI-compliant */
 char *strerror_r(int errnum, char *buf, size_t buflen);
 /* GNU-specific */



