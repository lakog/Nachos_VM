//READ.c

#include "syscall.h"

int
main()
{
    int res;
    char* a;
    
    res = ConsoleRead(a, 11);
    ConsoleWrite(a, 7);
    ConsoleWrite("ReadSysCall", 25);

    ThreadExit(0);

    /* not reached */
}