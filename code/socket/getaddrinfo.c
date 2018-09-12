#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define LEN 500

void print_family(struct addrinfo *aip){
    printf(" family ");
    switch(aip->ai_family){
        case AF_INET:
        printf("inet");
        break;
        case AF_INET6:
        printf("inet6");
        break;
        case AF_UNIX:
        printf("unix");
        break;
        case AF_UNSPEC:
        printf("unspecified");
        break;
        default:
        printf("unknown");
    }
}

void print_type(struct addrinfo *aip){
    printf(" type ");
    switch (aip->ai_socktype){
        case SOCK_STREAM:
            printf("stream");
            break;
        case SOCK_DGRAM:
            printf("datagram");
            break;
        case SOCK_SEQPACKET:
            printf("seqpacket");
            break;
        case SOCK_RAW:
            printf("raw");
            break;
        default:
            printf("unknown (%d)", aip->ai_socktype);
    }
}

void print_protocol(struct addrinfo *aip){
    printf(" protocol ");
    switch (aip->ai_protocol){
        case 0:
            printf("default");
            break;
        case IPPROTO_TCP:
            printf("TCP");
            break;
        case IPPROTO_UDP:
            printf("UDP");
            break;
        case IPPROTO_RAW:
            printf("raw");
            break;
        default:
            printf("unknown (%d)", aip->ai_protocol);
    }
}

void print_flags(struct addrinfo *aip){
    printf("flags");
    if(aip->ai_flags==0)
        printf(" 0 ");
    else{
        if(aip->ai_flags & AI_PASSIVE)
            printf(" passvie");
        if(aip->ai_flags & AI_CANONNAME)
            printf(" canon");
        if(aip->ai_flags & AI_NUMERICHOST)
            printf(" numhost");
        if(aip->ai_flags & AI_NUMERICSERV)
            printf(" numserv");
        if(aip->ai_flags & AI_V4MAPPED)
            printf(" v4mapped");
        if(aip->ai_flags & AI_ALL)
            printf(" all");
    }
}

int main(int argc, char *argv[]){
    struct addrinfo     *result, *rp;
    struct addrinfo     hints;
    struct sockaddr_in  *sinp;
    const char          *addr;
    int                 s;
    char                abuf[LEN];

    if(argc != 3)
        fprintf(stderr, "usage: %s nodename service", argv[0]);
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

   /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully bind(2).
      If socket(2) (or bind(2)) fails, we (close the socket
      and) try the next address. */

    for(rp = result; rp != NULL; rp = rp->ai_next){
        print_family(rp);
        print_flags(rp);
        print_type(rp);
        print_protocol(rp);
        printf("\n\thost %s", rp->ai_canonname?rp->ai_canonname:"-");
        if(rp->ai_family == AF_INET){
            sinp = (struct sockaddr_in *)rp->ai_addr;
            addr = inet_ntop(AF_INET, &sinp->sin_addr, abuf, INET_ADDRSTRLEN);
            printf(" address %s", addr?addr:"unknown");
            printf(" prot %d", ntohs(sinp->sin_port));
        }
        printf("\n");
    }
    exit(0);
}