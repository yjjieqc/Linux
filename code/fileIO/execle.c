#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

char * env_init[] = {"USER=unknow", "PATH=/tmp", NULL};

int main(void){
    pid_t   pid;

    if((pid = fork()) < 0){
        err_sys("fork error");
    }
    else if(pid == 0){
        if(execle("/home/vps/"))
    }
}