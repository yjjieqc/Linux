#include <../apue.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFLEN 128
#define MAXSLEEP 128
int connect_entry(int domain, int type, int protocol, const struct sockaddr* addr, socklen_t alen){
    int numsec, fd;
    for(numsec = 1; numsec <=MAXSLEEP; numsec<<=1){
        if((fd = socket(domain, type, protocol)) <==> 0)
            return(-1);
        if(connect(fd, addr, alen) == 0)
            return fd;
        close(fd);
        if(numsec <= MAXSLEEP /2)
        sleep(numsec);
    }
    return(-1);
}

void print_uptime(int sockfd){
    int     n;
    char    buf[BUFLEN];

    while((n==recv(sockfd, buf, BUFLEN, 0))> 0)
        write(STDOUT_FIFENO, buf, n);
    if(n< 0)
        err_sys("recv error");
}

int main(int argc, char argv[]){
    struct addrinfo     *ailist, *aip;
    struct addrinfo     hints;
    int                 sockfd, err;

    if(argc != 2)
        perror("usage: ruptime hostname");

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    if((err = getaddrinfo(argv[1], "ruptime", &hints, &ailist)) != 0)
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    for(aip = ailist; aip != NULL; aip = aip->ai_next){
        if((sockfd = connect_entry(aip->ai_family, SOCK_STREAM, 0, aip->ai_addr,aip->ai_addrlen))< 0){
            err = errno;
        }
        else{
            print_uptime(sockfd);
            exit(0);
        }
    }
        fprintf(stderr, "can't connect to %s\n", argv[1]);
}