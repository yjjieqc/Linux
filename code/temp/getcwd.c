#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

int main(void){
    char *ptr;
    size_t size = 256;
    if(chdir("/home/vps/joy/github/linux")< 0)
    perror("chdir failed");
    ptr = (char *)malloc(sizeof(char)* size);
    if(getcwd(ptr, size)==NULL)
    perror("getced failed");
    printf("cwd = %s\n", ptr);
    free(ptr);
    exit(0);
}