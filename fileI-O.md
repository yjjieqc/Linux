# 1  文件I/O

## 1.1  C标准函数与系统函数的区别




### 1.1.1  I/O缓冲区

每一个FILE文件流都有一个缓冲区buffer，默认大小8192Byte。

### 1.1.2  效率

### 1.1.3  程序的跨平台性

事实上Unbuffered I/O这个名词是有些误导的，虽然write系统调用位于C标准库I/O缓冲区的底层，但在write的底层也可以分配一个内核I/O缓冲区，所以write也不一定是直接写到文件的，也可能写到内核I/O缓冲区中，至于究竟写到了文件中还是内核缓冲区中对于进程来说是没有差别的，如果进程A和进程B打开同一文件，进程A写到内核I/O缓冲区中的数据从进程B也能读到，而C标准库的I/O缓冲区则不具有这一特性（想一想为什么）。

## 1.2 PCB概念——进程控制块

### 1.2.1 task_struct结构

	定义位置：/usr/src/linux-headers/include/linux/sched.h

### 1.2.2 files_struct结构体

相当于一张表，或者指针，文件描述符表

	FILE *		fopen("abc")

## 1.3 open/close

FLAG|作用
------|-----
O_CREAT|创建文件
O_EXCL|创建文件时，如果文件存在则出错返回
O_TRUNC|把文件截断为0
O_RDONLY|只读（互斥）
O_WRONLY|只写（互斥）
O_RDWR|读写（互斥）
O_APPEND|追加，移动文件读写指针到文件末尾
O_NONBLOCK|非阻塞标志
O_SYNC|每次write都等到物理I/O操作完成，包括文件属性的更新

### 1.3.1 文件描述符

	一个进程默认打开3个文件描述符

	STDIN_FILENO 0
	STDOUT_FILENO 1
	STDERR_FILENO 2

	open函数可以打开或者创建一个文件
    使用open函数，需要包含以下三个头文件

```
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
```

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

```
	#include <unistd.h>
	ssize_t read(int fd, void *buf, size_t count);
```

返回值：成功返回读取的**字节数**，出错则返回**-1并设置errno**，如果在调用read之前已经到达文件末尾，则这次read返回**0**；

**ssize_t 有符号int，size_t 无符号int**

参数count是请求读取的字节数,读上来的数据保存在缓冲区buf中,同时文件的当前读写位置向后移。注意这个读写位置和使用C标准I/O库时的读写位置有可能不同,这个读写位置是记在内核中的,而使用C标准I/O库时的读写位置是用户空间I/O缓冲区中的位置。比如用fgetc读一个字节,fgetc有可能从内核中预读1024个字节到I/O缓冲区中,再返回第一个字节,这时该文件在内核中记录的读写位置是1024,而在FILE结构体中记录的读写位置是1。注意返回值类型是ssize_t,表示有符号的size_t,这样既可以返回正的字节数、0(表示到达文件末尾)也可以返回负值-1(表示出错)。read函数返回时,返回值说明了buf中前多少个字节是刚读上来的。有些情况下,实际读到的字节数(返回值)会小于请求读的字节数count,例如:
	读常规文件时,在读到count个字节之前已到达文件末尾。例如,距文件末尾还有30个字节而请求读100个字节,则read返回30,下次read将返回0。
	从终端设备读,通常以行为单位,读到换行符就返回了。
	从网络读,根据不同的传输层协议和内核缓存机制,返回值可能小于请求的字节数,后面socket编程部分会详细讲解。

	write函数向打开的设备或文件中写数据。
``````
	#include <unistd.h>

	ssize_t write(int fd, void *buf, size_t count);
```

返回值：成功返回写入的字节数，出错返回-1并设置errno

写常规文件时,write的返回值通常等于请求写的字节数count,而向终端设备或网络写则不一定。

## 1.5 阻塞和非阻塞

读常规文件是不会阻塞的，不管读多少字节，read都会在有限时间内返回。从终端设备和网络读则不一定，如果从终端输入的数据 没有换行符，调用read读终端的设备就会阻塞，如果网络上没有接收到数据包，调用read从网络读就会阻塞，至于阻塞多长时间是不确定的，如果一直没有数据到达就一直阻塞在哪里。同样，写常规文件也是不会阻塞的，而向终端设备或网络写则不一定。

现在明确一下阻塞(Block)这个概念。当进程调用一个阻塞的系统函数时,该进程被置于睡眠(Sleep)状态,这时内核调度其它进程运行,直到该进程等待的事件发生了(比如网络上接收到数据包,或者调用sleep指定的睡眠时间到了)它才有可能继续运行。与睡眠状态相对的是运行(Running)状态,在Linux内核中,处于运行状态的进程分为两种情况:

正在被调度执行。CPU处于该进程的上下文环境中,程序计数器(eip)里保存着该进程的指令地址,通用寄存器里保存着该进程运算过程的中间结果,正在执行该进程的指令,正在读写该进程的地址空间。

就绪状态。该进程不需要等待什么事件发生,随时都可以执行,但CPU暂时还在执行另一个进程,所以该进程在一个就绪队列中等待被内核调度。系统中可能同时有多个就绪的进程,那么该调度谁执行呢?内核的调度算法是基于优先级和时间片的,而且会根据每个进程的运行情况动态调整它的优先级和时间片,让每个进程都能比较公平地得到机会执行,同时要兼顾用户体验,不能让和用户交互的进程响应太慢。

下面这个小程序从终端读数据再写回终端。

### 1.5.1 阻塞读终端

```
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
```

### 1.5.2  非阻塞读终端
	轮寻模式

### 1.5.3  非阻塞读终端和等待超时

## 1.6  lseek

类似于标准I/O库的`fseek`函数类似，可以移动当前读写位置，但有一个例外，如果文件以O_APEND模式打开，每次写操作都会在文件末尾追加数据，然后讲读写位置移动到新的文件末尾。
`fseek`函数在底层调用`lseek`实现。

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
```
    #include <unistd.h>
    #include <fcntl.h>

    int fcntl(int fd, int cmd, ... /* arg */ );
    int fcntl(int fd, int cmd);
    int fcntl(int fd, int cmd, long arg);
    int fcntl(int fd, int cmd, struct flock *lock);
```

参数  |作用
:-----：|：------：
__F_DUPFD__ | 复制一个文件描述符
__F_GETFL__ | 获得一个文件标志状态
__F_SETFL__ | 设置文件状态标志


## 1.8 ioctl

ioctl用于向设备发控制和配置命令，有些命令也需要读写一些数据，但是这些数据是不能用read/write读写的，称为Out-of-Band数据。也就是说，read/write读写数据是in-band数据，是I/O操作的主体。而ioctl命令传送的是控制信息，其中的数据是辅助的数据。例如，在串口线上收发数据童工read/write操作，而串口的波特率、校验位、停止位通过ioctl设置，A/D转换的结果通过read图区，而A/D转换的精度和工作频率通过ioctl设置。

![ioctl工作模式](./figures/3-file-io/ioctl.png)
ioctl 工作模式
```
    #inlcude <sys/ioctl.h>

    int ioctl(int d, int request, ...);
```

d是某个设备文件描述符。request是ioctl命令，可变参数取决于request。

注意:open命令创建文件呢权限不能超过执行用户的自有权限
如果有O_CREAT参数时，不要忘记mode参数。


# 2  文件系统

## 2.1  ext2文件系统

|1024|ext2管理单元|
----|-----
|boot block| 以4k为单元元|

1block = 4096Bytes
1block = 8磁盘扇区
1磁盘扇区 = 512Bytes
1block = 32768bit

    磁盘格式化指令mkfs命令
    文件系统中存储的最小单位是块(Block),一个块究竟多大是在格式化时确定的,例如mke2fs的-b选项可以设定块大小为1024、2048或4096字节。而上图中启动块(BootBlock)的大小是确定的,就是1KB,启动块是由PC标准规定的,用来存储磁盘分区信息和启
动信息,任何文件系统都不能使用启动块。启动块之后才是ext2文件系统的开始,ext2文件系统将整个分区划成若干个同样大小的块组(Block Group),每个块组都由以下部分组成。

    超级块(Super Block) 描述整个分区的文件系统信息,例如块大小、文件系统版本号、上次mount的时间等等。超级块在每个块组的开头都有一份拷贝。

    块组描述符表(GDT,Group Descriptor Table) 由很多块组描述符组成,整个分区分成多少个块组就对应有多少个块组描述符。每个块组描述符(Group Descriptor)存储一个块组的描述信息,例如在这个块组中从哪里开始是inode表,从哪里开始是数据块,空闲的inode和数据块还有多少个等等。和超级块类似,块组描述符表在每个块组的开头也都有一份拷贝,这些信息是非常重要的,一旦超级块意外损坏就会丢失整个分区的数据,一旦块组描述符意外损坏就会丢失整个块组的数据,因此它们都有多份拷贝。通常内核只用到第0个块组中的拷贝,当执行e2fsck检查文件系统一致性时,第0个块组中的超级块和块组描述符表就会拷贝到其它块组,这样当第0个块组的开头意外损坏时就可以用其它拷贝来恢复,从而减少损失。

    块位图(Block Bitmap) 一个块组中的块是这样利用的:数据块存储所有文件的数据,比如某个分区的块大小是1024字节,某个文件是2049字节,那么就需要三个数据块来存,即使第三个块只存了一个字节也需要占用一个整块;超级块、块组描述符表、块位图、inode位图、inode表这几部分存储该块组的描述信息。那么如何知道哪些块已经用来存储文件数据或其它描述信息,哪些块仍然空闲可用呢?块位图就是用来描述整个块组中哪些块已用哪些块空闲的,它本身占一个块,其中的每个bit代表本块组中的一个块,这个bit为1表示该块已用,这个bit为0表示该块空闲可用。

    为什么用df命令统计整个磁盘的已用空间非常快呢?因为只需要查看每个块组的块位图即可,而不需要搜遍整个分区。相反,用du命令查看一个较大目录的已用空间就非常慢,因为不可避免地要搜遍整个目录的所有文件。
与此相联系的另一个问题是:在格式化一个分区时究竟会划出多少个块组呢?主要的限制在于块位图本身必须只占一个块。用mke2fs格式化时默认块大小是1024字节,可以用-b参数指定块大小,现在设块大小指定为b字节,那么一个块可以有8b个bit,这样大小的一个块
位图就可以表示8b个块的占用情况,因此一个块组最多可以有8b个块,如果整个分区有s个块,那么就可以有s/(8b)个块组。格式化时可以用-g参数指定一个块组有多少个块,但是通常不需要手动指定,mke2fs工具会计算出最优的数值。

    inode位图(inode Bitmap) 和块位图类似,本身占一个块,其中每个bit表示一个inode是否空闲可用。

    inode表(inode Table) 我们知道,一个文件除了数据需要存储之外,一些描述信息也需要存储,例如文件类型(常规、目录、符号链接等),权限,文件大小,创建/修改/访问时间等,也就是ls -l命令看到的那些信息,这些信息存在inode中而不是数据块中。每个文件都有一个inode,一个块组中的所有inode组成了inode表。
inode表占多少个块在格式化时就要决定并写入块组描述符中,mke2fs格式化工具的默认策略是一个块组有多少个8KB就分配多少个inode。由于数据块占了整个块组的绝大部分,也可以近似认为数据块有多少个8KB就分配多少个inode,换句话说,如果平均每个文件的大小是8KB,当分区存满的时候inode表会得到比较充分的利用,数据块也不浪费。如果这个分区存的都是很大的文件(比如电影),则数据块用完的时候inode会有一些浪费,如果这个分区存的都是很小的文件(比如源代码),则有可能数据块还没用完inode就已经用完了,
数据块可能有很大的浪费。如果用户在格式化时能够对这个分区以后要存储的文件大小做一个预测,也可以用mke2fs的-i参数手动指定每多少个字节分配一个inode。__一个inode单元128B，保存文件的访问信息，所属用户和群组信息。__

    数据块(Data Block) 根据不同的文件类型有以下几种情况对于常规文件,文件的数据存储在数据块中。

    对于目录,该目录下的所有文件名和目录名存储在数据块中,注意文件名保存在它所在目录的数据块中,除文件名之外,ls -l命令看到的其它信息都保存在该文件的inode中。注意这个概念:目录也是一种文件,是一种特殊类型的文件。

    对于符号链接,如果目标路径名较短则直接保存在inode中以便更快地查找,如果目标路径名较长则分配一个数据块来保存。

    设备文件、FIFO和socket等特殊文件没有数据块,设备文件的主设备号和次设备号保存在inode中。

### 2.1.1  目录中纪录项文件类型

编码|文件类型
------|-----
0|Unknown
1|Regular file
2|Directory
3|Character device
4|Block device
5|Named pipe
6|Socket
7|Symbolic link

### 2.1.2  数据块寻址

    从上图可以看出,索引项Blocks[13]指向两级的间接寻址块,最多可表示(b/4)2+b/4+12个数据块,对于1K的块大小最大可表示64.26MB的文件。索引项Blocks[14]指向三级的间接寻址块,最多可表示(b/4)3+(b/4)2+b/4+12个数据块,对于1K的块大小最大可表示16.06GB的文件。
    可见,这种寻址方式对于访问不超过12个数据块的小文件是非常快的,访问文件中的任意数据只需要两次读盘操作,一次读inode(也就是读索引项)一次读数据块。而访问大文件中的数据则需要最多五次读盘操作:inode、一级间接寻址块、二级间接寻址块、三级间接寻址块、数据块。实际上,磁盘中的inode和数据块往往已经被内核缓存了,读大文件的效率也不会太低。

## 2.2 stat
```c
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>

    int stat(const char *path, struct stat *buf);
    int fstat(int fd, struct stat *buf);
    int lstat(const char *path, struct stat *buf);

    struct stat {
        dev_t st_dev; /* ID of device containing file */
        ino_t st_ino; /* inode number */
        mode_t st_mode; /* protection */
        nlink_t st_nlink; /* number of hard links */
        uid_t st_uid; /* user ID of owner */
        gid_t st_gid; /* group ID of owner */
        dev_t st_rdev; /* device ID (if special file) */
        off_t st_size; /* total size, in bytes */
        blksize_t st_blksize; /* blocksize for file system I/O */
        blkcnt_t st_blocks; /* number of 512B blocks allocated */
        time_t st_atime; /* time of last access */
        time_t st_mtime; /* time of last modification */
        time_t st_ctime; /* time of last status change */
    };
```

    stat既有命令，也有同名函数，用来获取文件Inode里面的主要信息，stat跟踪符号链接，lstat不跟踪符号链接
    stat里面时间辨析：
    atime(最近访问时间): mtime(最近更改时间):指最近修改文件内容的时间 ctime(最近改动时间):指最近改动Inode的时间

##  access

```C
     #include <unistd.h>

     int access(const char *pathname, int mode);
```

    按用户ID和实际组ID测试，跟踪符号链接，参数mode：

 | 
    -----|-----
    R_OK |是否具有读权限
    W_OK | 是否具有写权限
    X_OK | 是否具有执行权限
    F_OK | 测试一个文件是否存在

## link

### link
创建一个硬链接，当`rm`删除文件时，只是删除了目录下记录项和把inode硬链接计数减1，当硬链接计数为0时，才会真正删除文件。<br>
    __硬链接通常要求位于同意文件系统中，POSIX允许跨文件系统__<br>
    __符号链接没有文件系统限制__<br>
    __通常不允许创建目录的硬链接__  会出现死循环<br>
    __创建目录以及增加硬链接计数应当是一个原子操作__<br>

```C
    #include <unistd.h>
    
    int link(const char *oldpath, const char *newpath);
```
### symlink

```C
    int symlink(const char *target, const char *linkpath);
```
### readlink

    读符号链接所指向的文件名，不读文件内容。
```
    #include <unistd.h>

    ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
```

### unlink

```
    #include <unistd.h>

    int unlink(const char *pathname);
```
1. 如果是符号链接,删除符号链接
2. 如果是硬链接,硬链接数减1,当减为0时,释放数据块和inode
3. 如果文件硬链接数为0,但有进程已打开该文件,并持有文件描述符,则等该进程关闭该文件时,kernel才真正去删除该文件
4. 利用该特性创建临时文件,先open或creat创建一个文件,马上unlink此文件

## rename

```
    #include <stdio.h>

    int rename(const char *oldpath, const char *newpath);
```

## chdir

```
    #include <unistd.h>

    int chdir(const char *path);
    int fchdir(int fd);
```
    改变当前进程的工作目录

## 2.10  getcwd

```
    #include <unistd.h>

    char *getcwd(char *buf, size_t size);
```
    获取当前进程工作目录

## 2.11  pathconf

```
    #include <unistd.h>

    long fpathconf(int fd, int name);
    long pathconf(const char *path, int name);
```

## 2.12  目录操作

### 2.12.1  mkdir

       #include <sys/stat.h>
       #include <sys/types.h>

       int mkdir(const char *pathname, mode_t mode);

### 2.12.2  rmdir

       #include <unistd.h>

       int rmdir(const char *pathname);

### 2.12.3  opendir/fdopendir

       #include <sys/types.h>
       #include <dirent.h>

       DIR *opendir(const char *name);
       DIR *fdopendir(int fd);

### 2.12.4  readdir

       #include <dirent.h>

       struct dirent *readdir(DIR *dirp);//返回一个纪录项指针

                  struct dirent {
                       ino_t          d_ino;       /* inode number */
                       off_t          d_off;       /* not an offset; see NOTES */
                       unsigned short d_reclen;    /* length of this record */
                       unsigned char  d_type;      /* type of file; not supported
                                                      by all filesystem types */
                       char           d_name[256]; /* filename */
                   };