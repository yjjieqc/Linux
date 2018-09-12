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
        fork()在父进程中的返回值与子进程中的getpid()返回值相同。

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
            uid_t geteuid(void); //返回有效用户ID，如调用sudo指令

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

        用fork创建子进程后执行的是和父进程相同的程序（但有可能执行不同的代码分支），子进程往往要调用一种exec函数以执行另一个程序。当进程调用一种exec函数时，该进程的用户空间代码和数据完全被新程序替换，从新程序的启动例程开始执行。调用exec并不创建新进程，所以调用exec前后该进程的id并未改变。其实有六种以exec开头的函数，统称exec函数：

           #include <unistd.h>

           extern char **environ;

           int execl(const char *path, const char *arg, ...
                           /* (char  *) NULL */);
           int execlp(const char *file, const char *arg, ...
                           /* (char  *) NULL */);
           int execle(const char *path, const char *arg, ...
                           /*, (char *) NULL, char * const envp[] */);
           int execv(const char *path, char *const argv[]);
           int execvp(const char *file, char *const argv[]);
           int execvpe(const char *file, char *const argv[],
                           char *const envp[]);

        这些函数如果调用成功则加载新的程序从启动代码开始执行，不再返回，如果调用出错则返回-1，所以exec函数只有出错的返回值而没有成功的返回值。
        这些函数原型看起来很容易混，但只要掌握了规律就很好记。不带字母p（表示path）的exec函数第一个参数必须是程序的相对路径或绝对路径，例如“/bin/ls”或“./a.out”，而不能是“ls”或“a.out”。对于带字母p的函数：
        如果参数中包含/，则将其视为路径名。
        否则视为不带路径的程序名，在PATH环境变量的目录列表中搜索这个程序。
        带有字母l（表示list）的exec函数要求将新程序的每个命令行参数都当作一个参数传给它，命令行参数的个数是可变的，因此函数原型中有…，…中的最后一个可变参数应该是NULL，起sentinel的作用。对于带有字母v（表示vector）的函数，则应该先构造一个指向各参数的指针数组，然后将该数组的首地址当作参数传给它，数组中的最后一个指针也应该是NULL，就像main函数的argv参数或者环境变量表一样。
        对于以e（表示environment）结尾的exec函数，可以把一份新的环境变量表传给它，其他exec函数仍使用当前的环境变量表执行新程序。
        exec调用举例如下：

        char *const ps_argv[] ={"ps", "-o", "pid,ppid,pgrp,session,tpgid,comm", NULL};
        char *const ps_envp[] ={"PATH=/bin:/usr/bin", "TERM=console", NULL};
        execl("/bin/ps", "ps", "-o", "pid,ppid,pgrp,session,tpgid,comm", NULL);
        execv("/bin/ps", ps_argv);
        execle("/bin/ps", "ps", "-o", "pid,ppid,pgrp,session,tpgid,comm", NULL, ps_envp);
        execve("/bin/ps", ps_argv, ps_envp);
        execlp("ps", "ps", "-o", "pid,ppid,pgrp,session,tpgid,comm", NULL);
        execvp("ps", ps_argv);

        事实上，只有execve()是真正的系统调用，其它五个函数最终都调用execve()，所以execve()在man手册第2节，其它函数在man手册第3节。这些函数之间的关系如下图所示。

        一个完整的例子：
        #include <unistd.h>
        #include <stdlib.h>
        int main(void)
        {
        execlp("ps", "ps", "-o", "pid,ppid,pgrp,session,tpgid,comm", NULL);
        perror("exec ps");
        exit(1);
        }

        由于exec函数只有错误返回值，只要返回了一定是出错了，所以不需要判断它的返回值，直接在后面调用perror即可。注意在调用execlp时传了两个“ps”参数，第一个“ps”是程序名，execlp函数要在PATH环境变量中找到这个程序并执行它，而第二个“ps”是第一个命令行参数，execlp函数并不关心它的值，只是简单地把它传给ps程序，ps程序可以通过main函数的argv[0]取到这个参数。
        调用exec后，原来打开的文件描述符仍然是打开的。利用这一点可以实现I/O重定向。
        先看一个简单的例子，把标准输入转成大写然后打印到标准输出：
        例 upper

        /* upper.c */
        #include <stdio.h>
        int main(void)
        {
        int ch;
        while((ch = getchar()) != EOF) {
        putchar(toupper(ch));
        }
        return 0;
        }

        例 wrapper

        /* wrapper.c */
        #include <unistd.h>
        #include <stdlib.h>
        #include <stdio.h>
        #include <fcntl.h>
        int main(int argc, char *argv[])
        {
        int fd;
        if (argc != 2) {
        fputs("usage: wrapper file\n", stderr);
        exit(1);
        }
        fd = open(argv[1], O_RDONLY);
        if(fd<0) {
        perror("open");
        exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
        execl("./upper", "upper", NULL);
        perror("exec ./upper");
        exit(1);
        }


        wrapper程序将命令行参数当作文件名打开，将标准输入重定向到这个文件，然后调用exec执行upper程序，这时原来打开的文件描述符仍然是打开的，upper程序只负责从标准输入读入字符转成大写，并不关心标准输入对应的是文件还是终端。运行结果如下：

        exec族
        l 命令行参数列表
        p 搜素file时使用path变量
        v 使用命令行参数数组
        e 使用环境变量数组,不使用进程原有的环境变量，设置新加载程序运行的环境变量

### 3.3.3  wait/waitpid

        僵尸进程: 子进程退出，父进程没有回收子进程资源（PCB），则子进程变成僵尸进程
        孤儿进程: 父进程先于子进程结束，则子进程成为孤儿进程,子进程的父进程成为1号进程init进程，称为init进程领养孤儿进程

        #include <sys/types.h>
        #include <sys/wait.h>
        pid_t wait(int *status);
        pid_t waitpid(pid_t pid, int *status, int options);
        < -1 回收指定进程组内的任意子进程
        -1 回收任意子进程
        0 回收和当前调用waitpid一个组的所有子进程
        \> 0 回收指定ID的子进程

        一个进程在终止时会关闭所有文件描述符，释放在用户空间分配的内存，但它的PCB还保留着，内核在其中保存了一些信息：如果是正常终止则保存着退出状态，如果是异常终止则保存着导致该进程终止的信号是哪个。这个进程的父进程可以调用wait或waitpid获取这些信息，然后彻底清除掉这个进程。我们知道一个进程的退出状态可以在Shell中用特殊变量$?查看，因为Shell是它的父进程，当它终止时Shell调用wait或waitpid得到它的退出状态同时彻底清除掉这个进程。
        如果一个进程已经终止，但是它的父进程尚未调用wait或waitpid对它进行清理，这时的进程状态称为僵尸（Zombie）进程。任何进程在刚终止时都是僵尸进程，正常情况下，僵尸进程都立刻被父进程清理了，为了观察到僵尸进程，我们自己写一个不正常的程序，父进程fork出子进程，子进程终止，而父进程既不终止也不调用wait清理子进程：

        #include <unistd.h>
        #include <stdlib.h>
        int main(void)
        {
        pid_t pid=fork();
        if(pid<0) {
            perror("fork");
            exit(1);
        }
        if(pid>0) { /* parent */
            while(1);
        }
        /* child */
        return 0;
        }

        若调用成功则返回清理掉的子进程id，若调用出错则返回-1。父进程调用wait或waitpid时可能会：
        * 阻塞（如果它的所有子进程都还在运行）。
        * 带子进程的终止信息立即返回（如果一个子进程已终止，正等待父进程读取其终止信
        息）。
        * 出错立即返回（如果它没有任何子进程）。
        这两个函数的区别是：
        * 如果父进程的所有子进程都还在运行，调用wait将使父进程阻塞，而调用waitpid时如果在options参数中指定WNOHANG可以使父进程不阻塞而立即返回0。
        * wait等待第一个终止的子进程，而waitpid可以通过pid参数指定等待哪一个子进程。可见，调用wait和waitpid不仅可以获得子进程的终止信息，还可以使父进程阻塞等待子进程终止，起到进程间同步的作用。如果参数status不是空指针，则子进程的终止信息通过这个参数传出，如果只是为了同步而不关心子进程的终止信息，可以将status参数指定为NULL。
        例 waitpid

        #include <sys/types.h>
        #include <sys/wait.h>
        #include <unistd.h>
        #include <stdio.h>
        #include <stdlib.h>
        int main(void)
        {
            pid_t pid;
            pid = fork();
            if (pid < 0) {
                perror("fork failed");
                exit(1);
        }
        if (pid == 0) {
                int i;
                for (i = 3; i > 0; i--) {
                printf("This is the child\n");
                sleep(1);
        }
        exit(3);
        } else {
        int stat_val;
        waitpid(pid, &stat_val, 0);
        if (WIFEXITED(stat_val))
        printf("Child exited with code %d\n", WEXITSTATUS(stat_val));
        else if (WIFSIGNALED(stat_val))
        printf("Child terminated abnormally, signal %d\n", WTERMSIG(stat_val));
        }
        return 0;
        }

        wait阻塞函数，阻塞等待子进程结束waitpid 4种情况 < -1 = -1 = 0 > 0
        进程的退出状态
        非阻塞状态，WNOHANG
        获取进程退出状态的函数见manpages
