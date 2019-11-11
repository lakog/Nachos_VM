//write.c

#include "syscall.h"

int
main()
{

    int res;
    char * a;

    res = ConsoleRead(a, 11);
    // ConsoleWrite("WriteSysCall", 25);
    res = ConsoleWrite("WriteSysCall", 25);

    ThreadExit(0);    
    /* not reached */
}