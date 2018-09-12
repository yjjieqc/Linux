# 12 网络IPC：套接字

## 12.1 引言

上一章我们考察了各种UNIX系统所提供的经典进程间通信机制（IPC）：管道、FIFO、消息队列、信号量以及共享储存。这些机制允许同一台计算机上运行的进程间可以互相通信。本章将通过考察不同计算机（通过网络连接）上的进程间通信（network IPC）。

在本章中，我们将讨论套接字网络进程间通信接口，进程用该接口能够和其他进程通信，无论它们是在同一台计算机上还是不同的计算机上。实际上，这正是套接字几口的设计目标之一：

## 12.2 套接字描述符

套接字是通信端点的抽象。正如使用文件描述符访问文件，应用程序会用套接字描述符访问套接字。套接字描述符在UNIX系统中被当作一中文件描述符。事实上，很多处理文件描述符的函数（如read和write）都可以用于处理套接字描述符。

创建一个套接字描述符，可以用socket函数。

```c
    #include <sys/types.h>
    #include <sys/socket.h>
    int socket(int domain, int type, int protocol);
    //返回值：若成功，返回文件（套接字）描述符；若失败，返回-1；
```

参数domain（域）确定通信的特性，包括地址格式。各个域的常数通常以AF_开头，意指地址族（address family）。

域|描述
--|--
AF_INET|IPv4因特网域
AF_INET6|IPv6因特网域
AF_UNIX|UNIX域
AF_UPSPEC|未指定

参数type确定套接字的类型，进一步确定通信特征。

类型|描述
--|--
SOCK_DGRAM|固定长度的、无连接的、不可靠的报文传递
SOCK_RAW|IP协议的数据报文接口
SOCK_SEQPACKET|固定长度的、有序的、可靠的、面向连接的报文传递
SOCK_STREAM|有序的、可靠的、双向的、面向连接的字节流

参数protocol通常是0，表示为给定的域和套接字类型选择默认协议。当对同一域和套接字类型支持多个协议时，可以使用protocol选择一个特定协议。在AF_INET通信域中，套接字类型SOCK_STREAM的默认协议时传输控制协议（TCP）。在AF_INET通信域中，套接字SOCK_DGRAM的默认协议是UDP，下表列出了因特网域套接字定义的协议

协议|描述
--|--
IPPROTO_IP|IPv4网际协议
IPPROTO_IPV6|IPv6网际协议
IPPROTO_ICMP|因特网控制报文协议
IPPROTO_RAW|原始IP数据包协议
IPPROTO_TCP|传输控制协议
IPPROTO_UDP|用户数据报协议

对于数据报（SOCK_DGRAM）接口，两个对等进程之间通信时不需要逻辑连接。只需要向对等进程所使用的套接字送出一个报文。

因此数据报提供了一个无连接服务另一方面，字节流（SOCK_STREAM）要求在交换数据之前，在贝蒂套接字和通信的对等进程的套接字之间建立一个逻辑连接。

SOCK_SEQPACKET套接字和SOCK_STREAM套接字很类似，只是从该套接字得到的是基于豹纹的服务，而不是字节流服务。这意味着从SOCK_SEQPACKET套接字接受的数据量与对方所发送的一致。流控制传输协议（SCTP）提供了因特网上的顺序数据包服务。

SOCK_RAW套接字提供一个数据报接口，用于直接访问下面的网络层（即因特网域中的IP层）。使用这个接口时，应用程序负责构造自己的协议头部，这是因为传输协议都被绕过了。

调用socket与调用open相类似。在两种情况下，均可获得用于I/O的文件描述符。当不再需要该文件描述符时，调用close来关闭对文件或套接字的访问，并且释放该描述符的资源以便重新使用。虽然套接字的本质是一个文件描述符，但是不是所有的函数都可以接受套接字描述符。下表总结了到目前为止所讨论的大多数以文件描述符为参数的函数使用套接字描述符时的行为。

函数|使用套接字的行为
--|--
close|释放套接字
dup和dup2|和一般文件描述符一样复制
fchdir|失败，并且将errno设置为ENOTDIR
fchomod|未指定
fchown|由实现定义
fcntl|支持一些命令
fdatasync和fsync|由实现定义
fstat|支持一些stat结构成员，但如何支持由实现定义
ftruncate|未指定
ioctl|支持部分命令，依赖于底层设备驱动
lseek|由实现定义（通常失败会将errno设置为ESPIPE）
mmap|未指定
poll|正常工作
pread和pwrite|失败时会将errno设为ESPIPE
read和readw|与没有任何标志位的recv等价
selete|正常工作
write和writev|与没有任何标志位的send等价

套接字通信是双向的。可以采用shutdown函数禁止一个套接字的I/O。
```c
    #include <syus/socket.h>
    int shutdown(int sockfd, int how);
```

如果how是SHUT_RD（关闭读端），那么无法从套接字读取数据。如果是SHUT_WR（关闭写端）那么无法使用套接字发送数据。如果how是SHUT_RDWR，则既无法读取数据，也无法发送数据。

能够关闭（close）一个套接字，为什么还要使用shutdown呢？有若干理由。首先，只有最后一个活动引用关闭时，close才释放网络端点。这意味着如果复制一个套接字（如采用dup），要知道关闭了最后一个引用它的文件描述符才会释放这个套接字。而shutdown允许使一个套接字处于不活动状态，和引用它的文件描述符数目无关。其次，有时可以很方便的关闭套接字双向传输中的一个方向。

## 12.3 寻址

在上一节学习了如何创建和销毁套接字。在学习用套接字做一些有意义的事情之前，需要知道如何表示一个目标通信进程。进程标识由两部分组成。一部分是计算机的网络地址，它可以帮助标识网络上我们想通信的计算机；另一部分是该计算机上用端口号表示的服务，它可以帮助标识特定的进程。

### 12.3.1 字节序

**大端（Big-endian）字节序**：最大字节地址出现在足底有效字节（Least Significant Bytes，LSB）上。

**小端（Little-endian）字节序**：最低有效字节包含最小字节地址。

对于TCP/IP应用程序，有4个用来在处理器字节序和网络字节需之间实施转换的函数

```c
    #include <arpa/inet.h>

    uint32_t htonl(uint32_t hostint32);
    uint16_t htons(uint16_t hostint16);
    uint32_t ntohl(uint32_t netint32);
    uint16_t ntons(uint16_t netint16);
```
h表示主机字节序，n表示网络字节序。

### 12.3.2 地址格式

一个地址标识一个特定通信域的套接字端点，地址格式与这个特定的通信域相关。为使不同格式地址能够传入到套接字函数，地址会被强制转换成一个通用的地质结构sockaddr

```c
    struct sockaddr{
        sa_family_t sa_family;  /* address family */
        char        sa_data[];  /* variable-length address */
        .
        .
        .
    };
```
套接字实现可以自由地添加额外的成员并且定义sa_data成员的大小。例如，在linux系统中，定义如下
```c
//defined in include/linux/socket.h
    struct sockaddr {
	    sa_family_t sa_family;	/* address family, AF_xxx	*/
	    char        sa_data[14];	/* 14 bytes of protocol address	*/
};
```
因特网地址定义在<netinet/in.h>头文件中。在IPv4因特网域（AF_INET)中，套接字地址用结构sockaddr_in表示：
```c
/* Internet address. */
    struct in_addr {
	    __be32	s_addr;
    };

    struct sockaddr_in {
        __kernel_sa_family_t    sin_family;	/* Address family		*/
        __be16                  sin_port;	/* Port number			*/
        struct in_addr          sin_addr;	/* Internet address		*/

  /* Pad to size of `struct sockaddr'. */
        unsigned char		__pad[__SOCK_SIZE__ - sizeof(short int) -
			sizeof(unsigned short int) - sizeof(struct in_addr)];
};
```
数据类型in_port_t定义成uint16_t。数据类型in_addr_t定义成uint32_t，这些整数类型定义在<stdint.h>中并制定了响应位数。

与AF_INET域相比较，IPv6（AF_INET6）套接字地址用结构sockaddr_in6表示：
```c
    struct sockaddr_in6 {
        unsigned short int  sin6_family;    /* AF_INET6 */
        __be16			    sin6_port;      /* Transport layer port # */
        __be32			    sin6_flowinfo;  /* IPv6 flow information */
        struct in6_addr		sin6_addr;      /* IPv6 address */
        __u32			    sin6_scope_id;  /* scope id (new in RFC2553) */
};
```
每个实现可以自由添加更多的字段。尽管sockaddr_in和sockaddr_in6结构相差比较大，但它们均被强制转化成sockaddr结构输入到套接字例程中。

有时，需要打印出能被人理解而不是计算机所能理解的地址格式，linux提供了两个函数**linux下未找到原型**

```c
    #include <arpa/inet.h>

    const char *inet_ntop(int domain, const void *restrict addr, char *restrict str, socklen_t size);
    //返回值：若成功，返回地址字符串地址；若失败，返回NULL
    int inet_pton(int domain, char *restrict str, void *restrict addr);
    //返回值：若成功，返回1；若格式无效，返回0；若出错，返回-1
```
### 12.3.3 地址查询

理想情况下，应用程序不需要知道一个套接字地址的内部结构。如果一个程序简单地传递一个类似于sockaddr结构的套接字地址，并且不依赖与任何协议相关特性，那么可以与提供相同服务类型的许多不同协议协作。通过调用gethostent，可以找到给定计算机系统的主机信息。
```c
       #include <netdb.h>
       extern int h_errno;

       struct hostent *gethostbyname(const char *name);

       #include <sys/socket.h>       /* for AF_INET */
       struct hostent *gethostbyaddr(const void *addr,
                                     socklen_t len, int type);

       void sethostent(int stayopen);

       void endhostent(void);
```
如果主机数据库文件没有打开，gethostent会打开它。函数gethostent返回文件中的下一条目。函数sethostent会打开文件，那么将其回绕。当stayopen参数设置成非0值时，调用gethostent之后，文件将依然是打开的。函数endhostent可以关闭文件。

当gethostent返回时，会得到一个指向hostent结构的指针，该结构课鞥能包含一个静态的数据缓冲区，每次调用gethostent，缓冲区都会被覆盖。hostent结构至少包含以下几个成员：
```c
    struct hostent {
        char  *h_name;            /* official name of host */
        char **h_aliases;         /* alias list */
        int    h_addrtype;        /* host address type */
        int    h_length;          /* length of address */
        char **h_addr_list;       /* list of addresses */
    };
```
返回的地址采用网络字节序。

另外两个函数gethostbyname和gethostbyaddr，原来包含在hostent函数中，现在被认为是过时的，应该使用getaddrinfo，getnameinfo代替。

netent结构至少包含以下字段：
```c
    struct netent{
        char        *name;          /* network name */
        char        **n_aliases;    /* alternate network name array pointer */
        int         n_addrtype;     /* address type */
        uint32_t    n_net;          /* network number */
        .
        .
    };
```
服务是由地址的端口号部分表示的。每个服务器由一个唯一的众所周知的端口号来确定。可以使用函数getservbyname讲一个服务名映射到一个端口号，使用函数getservbyport讲一个端口号映射一个服务名，使用getservent顺序扫描服务数据库。
```c
    #include <netdb.h>
    struct servent *getservent(void);
    struct servent *getservbyname(const char *name, const char *proto);
    struct servent *getservbyport(int port, const char *proto);
    void setservent(int stayopen);
    void endservent(void);
```
servent至少包含以下成员：
```c
    struct servent {
        char  *s_name;       /* official service name */
        char **s_aliases;    /* alias list */
        int    s_port;       /* port number */
        char  *s_proto;      /* protocol to use */
    }
```
POSIX.1定义了若干新的函数，允许一个应用程序讲一个主机名和一个服务名映射到一个地址，或者反之。这些函数代替了较老的gethostbyname和gethostbyaddr。getaddrinfo函数允许讲一个主机名和服务名映射到一个地址。
```c
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>

    int getaddrinfo(const char *node, const char *service,
                    const struct addrinfo *hints,
                    struct addrinfo **res);

    void freeaddrinfo(struct addrinfo *res);
```
```c
    struct addrinfo {
        int              ai_flags;
        int              ai_family;
        int              ai_socktype;
        int              ai_protocol;
        socklen_t        ai_addrlen;
        struct sockaddr *ai_addr;
        char            *ai_canonname;
        struct addrinfo *ai_next;
    };
```

可以提供一个可选的hints来选择符合条件的地址。hints是一个用于过滤地址的模板，包括ai_family、ai_flags、ai_protocol和ai_socktype字段。剩余的整数字段必须设置为0，指针字段必须为空。下表总结了ai_flags字段中的标志，可以用这些标志来定义如何处理地址和名字。

标志|描述
--|--
AI_ADDRCONFIG|查询配置的地址类型
AI_ALL|查询一个IPv4或者IPv6地址
AI_CANONNAME|需要一个规范化的名字
AI_NUMERICHOST|以数字格式指定主机地址，不翻译
AI_NUMERICSERV|将服务指定为数字端口号，不翻译
AI_PASSIVE|套接字地址用于监听绑定
AI_V4MAPPED|如没有找到IPv6地址，返回映射到IPv6格式的IPv4地址

如果getaddrinfo失败，不能使用perror或者strerror来生成错误消息，而是要调用gai_strerror将返回的错误码转换成错误消息。
```c
    #include <netdb.h>

    const char *gai_strerror(int error);
```
getnameinfo函数将一个地址转换成一个主机名和一个服务名。
```c
    #include <sys/socket.h>
    #include <netdb.h>

    int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                       char *host, socklen_t hostlen,
                       char *serv, socklen_t servlen, int flags);

```
套接字地址（addr）被翻译成一个主机名和一个服务名。如果host为非空，则指向一个长度为hostlen字节的缓冲区用于存放返回的主机名。同样，如果service非空，则指向一个长度为servlen字节的缓冲区用于存放反悔的主机名。

flags参数提供了一些控制翻译的选项。下表总结了支持的标志

标志|描述
--|--
NI_DGRAM|服务基于数据报而非基于流
NI_NAMEREQD|如果找不到主机名，将其作为一个错误对待
NI_NOFQDN|对于本地主机，仅返回全限定域名的节点部分
NI_NUMERICHOST|返回主机地址的数字形式，而非名字
NI_NUMERICSCOPE|对于IPv6，返回范围ID的数字形式，而非名字
NI_NUMERICSERV|返回服务地址的数字形式（即端口号），而非名字

**实例代码见/code/socket/getaddrinfo.c**

### 12.3.4 将套接字与地址关联

将一个客户端的套接字关联上一个地址没有多少新意，可以让系统选一个默认地址。然而，对于服务器，需要给一个接收客户端请求的服务器套接字关联上一个众所周知的地址。客户端应有一种方法来发现连接服务器所需要的地址，最简单的方法就是服务器保留一个地址并且注册在/etc/services或者某个服务中。

使用bind函数来关联地址和套接字。
```c
    #include <sys/types.h>          /* See NOTES */
    #include <sys/socket.h>

    int bind(int sockfd, const struct sockaddr *addr,
             socklen_t addrlen);
    //返回值：若成功，返回0；若出错，返回-1
```
对于使用地址有以下限制。

+ 在进程正在运行的计算机上，指定的地址必须有效；不能指定一个其他机器的地址。
+ 地址必须和创建套接字时的地址族所支持的格式相匹配。
+ 地址中的端口号必须不小于1024，除非进程具有相应的权限（即超级用户）。
+ 一般只能将一个套接字端点绑定到一个给定的地址上，尽管有些协议允许多重绑定。

可以调用getsockname函数发现绑定到套接字的地址。
```c
    #include <sys/socket.h>

    int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    //返回值：若成功，返回0；若出错，返回-1
```
调用getsockname之前，将addrlen设置为一个指向整数的指针，该整数指定缓冲区aockaddr的长度。返回时，该整数会被设置成返回地址的大小。如果地址和提供缓冲区长度不匹配，地址会被自动截断而不报错。如果当前没有地址绑定到该套接字，则其结果是未定义的。

如果套接字已经和对等方连接，可以调用getpeername函数来找到对方的地址。
```c
    #include <sys/socket.h>

    int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    //返回值：若成功，返回0；若出错，返回-1
```
除了返回对等方地址，函数getpeername和getsockname一样。

## 12.4 建立连接

如果要处理一个面向连接的网络服务（SOCK_STREAM或SOCK_SEQPACKET），那么在开始交换数据前，需要在请求服务的进程套接字（客户端）和提供服务的进程套接字（服务器）之间建立一个连接。使用connect函数来建立连接。

```c
    #include <sys/types.h>          /* See NOTES */
    #include <sys/socket.h>

    int connect(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen);
    //返回值：若成功，返回0；若出错，返回-1
```
在connect中指定的地址是我们想要与之通信的的服务器地址。如果sockfd没有绑定到一个地址，connect会给调用者绑定一个默认地址。

服务器调用listen函数来宣告它愿意接受请求
```c
    #include <sys/socket.h>

    int listen(int sockfd, int backlog);
    //返回值：若成功，返回0；若出错，返回-1    
```
参数backlog提供了一个提示，提示系统该进程所要入队的未完成连接请求数量。其实际值由系统决定，但上限由<sys/socket.h>中SOMAXCONN指定。

一旦队列满，系统就会拒绝多余的连接请求，所以backlog的值应该基于服务器期望负载和处理量来选择，其中处理量是指接受请求与启动服务的数量。

一旦服务器调用了listen，所用的套接字就能接受请求。使用accept函数获得连接请求并建立连接。
```c
    #include <sys/types.h>          /* See NOTES */
    #include <sys/socket.h>

    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    //返回值：若成功，返回套接字描述符；若出错，返回-1  
```
函数accept所反悔的文件描述符是套接字描述符，该描述符连接到调用connect的客户端。这个新的套接字描述符和原始的套接字（sockfd）具有相同的套接字类型和地址族。传给accept的原始套接字没有关联到这个连接，二是继续保持可用状态并接受其他连接请求。

如果不关心客户端标识，可以将参数addr和addrlen设置为NULL。否则，在调用accept之前，将addr参数设为足够大的缓冲区来存放地址，并且将addrlen指向的整数设为这整个缓冲区的大小。返回时，accept会在缓冲区填充客户端地址，并且更新指向len的整数版来反应该地址的大小。

如果没有连接请求在等待，accept会阻塞直到一个请求到来，如果sockfd处于非阻塞模式，accept会返回-1，并将errno设置为EAGAIN或EWOULDBLOCK

如果服务器调用accept，并且当前没有连接请求，服务器会阻塞直到一个请求到来。另外，服务器可以使用poll和select来等待一个请求道来，这种情况下，一个带有等待连接请求的套接字会以可读的方式出现。

## 12.5 数据传输

既然一个套接字端点表示为一个文件描述符，那么只要建立连接，就可以使用read和write来通过套接字通信。回忆前面所讲，通过在connect函数里面设置默认对等地址，数据包套接字也可以被“连接”。在套接字描述符上使用read和write是非常有意义的，因为这意味着可以将套接字描述符传递给那些原先为处理温蒂文件而设计的函数。而且还可以安排将套接字描述符传递给子进程，而该子进程执行程序并不了解套接字。

尽管可以通过read和write交换数据，但这就是这两个函数所能做的一切。如果想指定选项，从多个客户端接收数据包，或者发送带外数据，就需要使用6个位数据传递而设计的套接字函数中的一个。3个函数用来发送数据，3个用来接收数据。

```c
    #include <sys/types.h>
    #include <sys/socket.h>

    ssize_t send(int sockfd, const void *buf, size_t len, int flags);

    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen);

    ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
    //三个函数返回值：若成功，返回发送的字节数；若出错，返回-1  
```
send类似于write，使用send时套接字必须已经连接。参数buf和nbytes的含义与write中的一致。然而，send支持第4个参数flags。

标志|描述
--|--
MSG_CONFIRM (since Linux 2.3.15)|
MSG_DONTROUTE|
MSG_DONTWAIT (since Linux 2.2)|
MSG_EOF|
MSG_EOR (since Linux 2.2)|
MSG_MORE (since Linux 2.4.4)|
MSG_NOSIGNAL (since Linux 2.2)|
MSG_OOB|

即使send成功返回，也并不表示连接的另一端的进程就一定接受了数据。我们所能保证的只是当send成功返回时，数据已经被无错误的发送到网络驱动器上。

对于面向连接的套接字，目标地址是忽略的，因为连接中隐含了目标地址。对于无连接的套接字，除非先调用connect设置了目标地址，否则不能使用send。sendto提供了发送报文的另一种方式。

通过套接字发送数据时，还有一个选择。可以调用带有msghdr结构的sendmsg来指定多冲缓冲区传输数据，这和writev函数很相似。

POSIX.1定义了msghdr结构，它至少有以下成员：
```c
    struct msghdr {
        void         *msg_name;       /* optional address */
        socklen_t     msg_namelen;    /* size of address */
        struct iovec *msg_iov;        /* scatter/gather array */
        size_t        msg_iovlen;     /* # elements in msg_iov */
        void         *msg_control;    /* ancillary data, see below */
        size_t        msg_controllen; /* ancillary data buffer len */
        int           msg_flags;      /* flags (unused) */
    };
```