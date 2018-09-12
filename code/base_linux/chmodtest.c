#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>

int main(void){
    struct stat statbuf;
    if(stat("foo", &statbuf) <0)
    perror("stat error for foo");
    if(chmod("foo", (statbuf.st_mode  & ~S_IXGRP)| S_ISGID)< 0)
    perror("chmod error for foo");
}
