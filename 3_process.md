# ３  进程

我们知道，每个进程在内核中都有一个进程控制块(PCB)来维护进程相关的信息，Linux内核进程控制块是task_struct结构体.

    * 进程id。系统中每个进程有唯一的id,在C语言中用pid_t类型表示,其实就是一个非负整数。

    * 进程的状态,有运行、挂起、停止、僵尸等状态。

    * 进程切换时需要保存和恢复的一些CPU寄存器。

    * 描述虚拟地址空间的信息。

    * 描述控制终端的信息。

    * 当前工作目录(Current Working Directory)。

    * umask掩码。

    * 文件描述符表,包含很多指向file结构体的指针。

    * 和信号相关的信息。

    * 用户id和组id。

    * 控制终端、Session和进程组。

    * 进程可以使用的资源上限(Resource Limit)。

    // cat /proc/self/limits
    
    // ulimit -a

## 3.1  环境变量

    libc中定义的全局变量environ指向环境变量表，environ没有包含在任何头文件中，所以在使用时要用extern声明。例如：

        #include <stdio.h>
        int main(void)
        {
            extern char **environ;
            int i;
            for(i=0; environ[i]!=NULL; i++)
                printf("%s\n", environ[i]);
            return 0;
        }
    
    由于父进程在调用fork创建子进程时会把自己的环境变量表也复制给子进程，所以a.out打印的环境变量和Shell进程的环境变量是相同的。
    
按照惯例，环境变量字符串都是name=value这样的形式，大多数name由大写字母加下划线组成，一般把name的部分叫做环境变量，value的部分则是环境变量的值。环境变量定义了进程的运行环境，一些比较重要的环境变量的含义如下：

PATH

* 可执行文件的搜索路径。ls命令也是一个程序，执行它不需要提供完整的路径名/bin/ls，然而通常我们执行当前目录下的程序a.out却需要提供完整的路径名./a.out，这是因为PATH环境变量的值里面包含了ls命令所在的目录/bin，却不包含a.out所在的目录。PATH环境变量的值可以包含多个目录，用:号隔开。在Shell中用echo命令可以查看这个环境变量的值：

$ echo $PATH

SHELL

* 当前Shell，它的值通常是/bin/bash。

TERM

* 当前终端类型，在图形界面终端下它的值通常是xterm，终端类型决定了一些程序的输出显示方式，比如图形界面终端可以显示汉字，而字符终端一般不行。

LANG

* 语言和locale，决定了字符编码以及时间、货币等信息的显示格式。

HOME
* 当前用户主目录的路径，很多程序需要在主目录下保存配置文件，使得每个用户在运行该程序时都有自己的一套配置。

    用environ指针可以查看所有环境变量字符串，但是不够方便，如果给出name要在环境变量表中查找它对应的value，可以用getenv函数。
    
        #include <stdlib.h>
        char *getenv(const char *name);
    
    getenv的返回值是指向value的指针，若未找到则为NULL。
    修改环境变量可以用以下函数
    
        #include <stdlib.h>
        int setenv(const char *name, const char *value, int rewrite);
        void unsetenv(const char *name);
    
    putenv和setenv函数若成功则返回为0，若出错则返回非0。
    setenv将环境变量name的值设置为value。如果已存在环境变量name，那么
    若rewrite非0，则覆盖原来的定义；
    若rewrite为0，则不覆盖原来的定义，也不返回错误。
    unsetenv删除name的定义。即使name没有定义也不返回错误。
    例 修改环境变量
    
        #include <stdlib.h>
        #include <stdio.h>
        int main(void)
        {
            printf("PATH=%s\n", getenv("PATH"));
            setenv("PATH", "hello", 1);
            printf("PATH=%s\n", getenv("PATH"));
            return 0;
        }
    
## 3.2  进程状态

    修改进程资源限制，软限制可改，最大值不能超过硬限制，硬限制只有root用户可以修改
```
        #include <sys/time.h>
        #include <sys/resource.h>
        int getrlimit(int resource, struct rlimit *rlim);
        int setrlimit(int resource, const struct rlimit *rlim);
```

    查看进程资源限制
        cat /proc/self/limits
        ulimit -a
        
##  3.3  进程原语

### 3.3.1  fork

        #include <unistd.h>
        
        pid_t fork(void);
        
        子进程复制父进程的0到3G空间和父进程内核中的PCB，但id不同。
        fork调用一次返回两次
        
        + 父进程中返回子进程ID
        + 子进程中返回0
        + 读时共享，写时复制

        fork
```
        #include <sys/types.h>
        #include <unistd.h>
        #include <stdio.h>
        #include <stdlib.h>
        int main(void)
        {
            pid_t pid;
            char *message;
            int n;
            pid = fork();
            if (pid < 0) {
                perror("fork failed");
                exit(1);
            }
            if (pid == 0) {
                message = "This is the child\n";
                n = 6;
            } else {
                message = "This is the parent\n";
                n = 3;
            }
            for(; n > 0; n--) {
                printf(message);
                sleep(1);
            }
            return 0;
        }
```

        进程相关函数
        getpid/getppid
        
            #include <sys/types.h>
            #include <unistd.h>
            pid_t getpid(void); //返回调用进程的PID号
            pid_t getppid(void); //返回调用进程父进程的PID号
        
        getuid
        
            #include <unistd.h>
            #include <sys/types.h>
            uid_t getuid(void); //返回实际用户ID
            uid_t geteuid(void); //返回有效用户ID

        getgid

            #include <unistd.h>
            #include <sys/types.h>
            gid_t getgid(void); //返回实际用户组ID
            gid_t getegid(void); //返回有效用户组ID

        vfork
        * 用于fork后马上调用exec函数
        * 父子进程，共用同一地址空间,子进程如果没有马上exec而是修改了父进程出得到的变量值，此修改会在父进程中生效
        * 设计初衷，提高系统效率，减少不必要的开销
        * 现在fork已经具备读时共享写时复制机制，vfork逐渐废弃

### 3.3.2  exec族
