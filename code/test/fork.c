//fork.c

#include "syscall.h"

void 
forked(int x){
 
	int res;
	char* a = "From Forked";
	res = ConsoleWrite(a, 14);
	ThreadExit(0);
}

int
main()
{
    int i = 0;
    char* a = "Forked!";
    char* b = "Yielded";

    ConsoleWrite(a, 7);
    ThreadFork(forked);
    ConsoleWrite(b, 7);

    ThreadYield();
    ThreadExit(0);

    /* not reached */
}