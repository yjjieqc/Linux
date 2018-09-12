#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFLEN 128
#define QLEN 10
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

extern int initserver(int, const struct sockaddr *, socklen_t int);

void serve(int sockfd){
    int     clfd;
    FILE    *fp;
    char    buf[BUFLEN];
    set_cloexec(sockfd);
    for(;;){
        if(clfd = accept(sockfd, NULL, NULL)< 0){
            fprintf(STDERR, "ruptime: accept error: %s", strerror(errno));
            exit(1);
        }
        set_cloexec(clfd);
        if((fp = popen("/usr/bin/uptime", "r")) == NULL){
            sprintf(buf, "error: %s\n", strerror(errno));
            send(clfd, buf, strlen(buf), 0);
        }
        else{
            while(fgets(buf, BUFLEN, fp)!= NULL)
                send(clfd, buf, strlen(buf), 0);
            pclose(fd);
        }
        close(clfd);
    }
}

int main(int argc, char *argv[]){
    struct addrinfo     *ailist, *aip;
    struct addrinfo     hints;
    int                 sockfd, err, n;
    char                *host;

    if(argc != 1)
        fprintf(stderr, "usage: ruptime");
    if((n = sysconf(_SC_HOST_NAME_MAX))<0)
        n = HOST_NAME_MAX;
    if((host = malloc(n))==NULL)
        fprintf(stderr, "malloc error");
    if(gethostname(host, n)< 0)
        fprintf(stderr, "gethostname error");
    daemonize("ruptimed");
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    if((err = getaddrinfo(host, "ruptime", &hints, &ailist))!= 0){
        fprintf(stderr, "ruptime: getaddrinfo error: %s", gai_strerror(err));
        exit(1);
    }

    for(aip = result; aip != NULL; aip = aip->ai_next){
        if((sockfd = initserver(SOCK_STREAM, aip->ai_addr, aip->ai_addrlen, QLEN))> =0){
            serve(sockfd);
            exit(0);
        }
    }
    exit(1);
}