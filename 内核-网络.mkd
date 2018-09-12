    struct socket{
        socket_state            state;  //套接字连接状态
        const struct proto_ops  *ops;
        struct file             *file;  //指向伪文件的file实例
        struct sock             *sk;
        short                   type;   //指定所用协议的数字标识符
    };

    typedef enum{
        SS_FREE = 0;        //未分配
        SS_UNCONNECTED,     //未连接到任何套接字
        SS_CONNECTING,      //处于连接过程中
        SS_CONNECTED        //已经连接到另一个套接字
        SS_DISCONNECTING,   //处于断开连接过程中
    } socket_state;


C库函数会通过socketcall系统调用下列的函数指针，socket结构的ops成员所包含的函数指针用于和系统调用通信

    struct proto_ops{
        int             family;
        struct module   *owner;
        int             (*release);
        int             (*bind);
    
        int             (*connect);
    
        int             (*socketpair);
    
        int             (*accept);
        int             (*getname);

        unsigned int    (*poll);
        int             (*ioctl);
        int             (*compat_ioctl);
        int             (*listen);
        int             (*shutdown);
        int             (*setsockopt)
        int             (*getsockopt)
        int             (*compat_setsockopt)
        int             (*compat_getsockopt)
        int             (*sendmsg)
        int             (*recvmsg)
        int             (*mmap)
        ssize_t         (*sendpage)
    }
    
struct socket的ops成员类型是struct proto_ops,与之相似的一个结构是struct proto,sock的port成员类型是struct proto，
里面的成员函数主要用于（内核端）套接字层和传输层之间的通信。

    include/net/sock.h
    struct proto{
        void            (*close)(struct sock*sk,
                                long timeout);
        int             (*connect)(struct sock *sk,
                                struct sockaddr *uaddr,
                                int addr_len);
        int             (*disconnect)(struct sock *sk, int flags);
        struct sock*    (*accept)(struct sock*sk, int flags, int *err);
    }

内核将自身最重要的一些成员放到sock_common结构中

    include/net/sock.h
    
    struct sock_common{
        unsigned short          skc_family;
        volatile unsigned char  skc_state;
        struct hlist_node       skc_node;
        unsigned int            skc_hash;
        atomic_t                skc_refcnt;
        struct proto            *skc_prot;
    };

并将sock_common嵌入在struct sock开始处

    struct sock{
        struct sock_common      __sk_common;
        struct sk_buff_head     sk_receive_queue;
        struct sk_buff_head     sk_write_queue;
        struct timer_list       sk_timer;
        void                    (*sk_date_ready) (struct sock *sk, int bytes);
        ...
    }

### 套接字和文件

每个套接字都被分配了一个该类型的inode，inode又关联到另一个普通文件相关的结构。
因而，对套接字文件描述符的操作，可以透明的重定向到网络子系统的代码。
套接字使用的file_operations如下：

     net/socket.c
    
     struct file_operations socket_file_ops = {
        .owner =                THIS_MODULE,
        .llseek =               no_llseek,
        .aio_read =             sock_aio_read,
        .aio_write =            sock_aio_write,
        .poll =                 sock_poll,
        .unlocked_ioctl =       sock_ioctl,
        .compat_ioctl =         compat_sock_ioctl,
        .mmap =                 sock_mmap,
        .open =                 sock_no_open,
        .release =              sock_close,
        .fasync =               sock_fasync,
        .sendpage =             sock_sendapge,
        .splice_write =         generic_splice_sendpage,
     };

### socketcall系统调用

17个套接字操作只对应一个系统调用，由于要处理任务不同，参数列表可能差别很大
该系统调用的第一个参数与时一个数值常数，选择所需要的系统调用。
可能的值包括SYS_SOCKET，SYS_BIND，SYS_ACCEPT和SYS_RECV。

    net/socket.c
    
    asmlinkage long sys_socketcall(int call, unsigned long __user * args){
        unsigned long a[6];
        unsigned long a0, a1;
        int err;
    
        if(call<1 || call>SYS_RECVMSG)
            return -EINVAL;
    }

sys_socket时创建新套接字的起点。首先使用socket_create创建一个新的套接字数据结构，该函数直接调用了__sock_create。
分配内存的任务委托给sock_alloc，该函数不仅为struct socket实例分配了空间，还紧接着该实例为inode分配了内存空间。

内核的所有传输协议都群集在net/socket.c中定义的数组static struct net_proto_family * net_families[NPROTO]中
（sock_register用于向该数据库增加新的数据项）。各个数组项都提供了特定于协议的初始化函数。

    net.h
    struct net_proto_family{
        int             family;
        int             (*create)(struct socket*sock, int protocol);
        struct module   *owner;
    }

在为套接字分配了内存之后，刚好调用函数create。inet_create用于因特网连接（TCP和UDP都使用该函数）。
map_sock_fd为套接字创建一个伪文件（文件通过socket_ops指定）。还要分配一个文件描述符，将其作为系统调用的结果返回。

###接收数据

使用recvfrom和recv以及与文件相关的readv和read函数来接收数据。

sys_recvfrom() -> fget_light() -> sock_from_file() -> sock_recvmsg -> sock->ops->recvmsg() -> move_addr_to_user()

第一步找到对应的套接字，首先，fget_light()根据task_struct的描述符表，查找对应的file实例。sock_from_file确认与之关联的inode，并通过使用SOCKET_I最终找到相关的套接字。

在一些准备工作后，sock_recvmsg调用特定的协议接受例程sock->ops->recv->msg0。例如，TCP使用tcp_recvmsg来完成工作。UDP使用udp_recvmsg。

- 如果接收队列（通过sock的receive_queue成员实现）上至少有一个分组，则移除并返回该分组
- 如果接收队列是空的，进程使用wait_for_packet使自身睡眠，直至数据到达。

新数据到达时总是调用sock结构的data_ready函数，因而进程可以在此时被唤醒

move_addr_to_user()将数据从内核空间复制到用户空间，使用了copy_to_user()函数。

### 发送数据

用户空间程序发送数据时，还有几种可以选择的方法。可以使用两个和网络相关的库函数（sendto和send）或文件层的write和writev函数。同样的，这些函数在内核中特定位置会合并为一。如 sendto函数：

sys_sendto() ->fget_light() -> sock_from_file() -> move_addr_to_kernel() -> sock_sendmsg() -> sock->ops->sendmsg()

fget_light()和sock_from_file根据文件描述符查找相关的套接字。发送的数据使用move_addr_to_kernel()从用户空间复制到内核空间，然后sock_sendmsg()调用特定协议的发送例程sock->ops->sendmsg()。该例程产生一个所需协议格式的分组，并转发到更低的协议层。

## 内核内部的网络通信

与其他主机通信不仅是用户层程序的需求，即使没有用户层的显示请求，内核同样也需要和其他计算机通信。不仅对一些古怪的特性（如某些发行版包含在内核内部的Web服务器）有用，网络文件系统如CIFS



