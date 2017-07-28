# 4  进程间通信（IPC，Inter-Process Communication）

        每个进程各自有不同的用户地址空间，任何一个进程的全局变量在另一个进程中都看不到，所以进程之间要交换数据必须通过内核，在内核中开辟一块缓冲区，进程1把数据从用户空间拷到内核缓冲区，进程2再从内核缓冲区把数据读走，内核提供的这种机制称为进程间通信（IPC，InterProcess Communication）。
        
        
## 4.1  pipe管道

        管道是一种最基本的IPC机制，由pipe函数创建：
        
        #Include <unistd.h>
        int pipe(int filedes[2]);
        
        管道作用于有血缘关系的进程之间,通过fork来传递调用pipe函数时在内核中开辟一块缓冲区（称为管道）用于通信，它有一个读端一个写端，然后通过filedes参数传出给用户程序两个文件描述符，filedes[0]指向管道的读端，filedes[1]指向管道的写端（很好记，就像0是标准输入1是标准输出一样）。所以管道在用户程序看起来就像一个打开的文件，通过read(filedes[0]);或者write(filedes[1]);向这个文件读写数据其实是在读写内核缓冲区。pipe函数调用成功返回0，调用失败返回-1。
        
        开辟了管道之后如何实现两个进程间的通信呢？比如可以按下面的步骤通信。
        
        1.父进程调用pipe开辟管道，得到两个文件描述符指向管道的两端。
        2.父进程调用fork创建子进程，那么子进程也有两个文件描述符指向同一管道。
        3.父进程关闭管道读端，子进程关闭管道写端。父进程可以往管道里写，子进程可以从管道里读，管道是用环形队列实现的，数据从写端流入从读端流出，这样就实现了进程间通信。
        
        __父写子读： 关闭父读，关闭子写__
        
        __子写父读： 关闭子读，关闭父写__

        例 pipe管道
`        
        #include <stdlib.h>
        #include <unistd.h>
        #define MAXLINE 80
        int main(void)
        {
            int n;
            int fd[2];
            pid_t pid;
            char line[MAXLINE];
            if (pipe(fd) < 0) {
                perror("pipe");
                exit(1);
            }
            if ((pid = fork()) < 0) {
                perror("fork");
                exit(1);
                }
            if (pid > 0) { /* parent */
                close(fd[0]);
                write(fd[1], "hello world\n", 12);
                wait(NULL);
                } 
            else { /* child */
                close(fd[1]);
                n = read(fd[0], line, MAXLINE);
                write(STDOUT_FILENO, line, n);
            }
            return 0;
        }

`



