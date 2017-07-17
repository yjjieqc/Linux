## 1.2 PCB概念——进程控制块

### 1.2.1 task_struct结构

	定义位置：/usr/src/linux-headers/include/linux/sched.h

### 1.2.2 files_struct结构体
	
	相当于一张表，或者指针，文件描述符表
	FILE *		fopen("abc")

## 1.3 open/close

### 1.3.1 文件描述符

	一个京城默认打开3个文件描述符
	STDIN_FILENO 0
	STDOUT_FILENO 1
	STDERR_FILENO 2

	open函数可以打开或者创建一个文件
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
