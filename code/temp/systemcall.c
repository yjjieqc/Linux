#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <error.h>

int main (void){
    int status;
    if((status = system("data"))< 0)
    perror("system() error");

    pr_exit(status);

    if((status = system("nosuchcommand"))< 0)
    perror("system() error");

    pr_exit(status);

    if((status = system("who; exit 44")) < 0)
    perror("system() error");
    exit(0);
}