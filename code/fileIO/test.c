#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int main(void){
    if(chdir("/tmp") < 0)
    perror("chdir failed");
    printf("chdir to /tmp successed\n");
    exit(0);
}