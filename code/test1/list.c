#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int main(void){
    struct person{
        char name[20];
        list_head list;
    }xiao;

    INIT_LIST_HEAD(&(xiao.list));
    return 0;
}