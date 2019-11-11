/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"




void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysRead(int address, int size) {
	cout << "Inside sysRead\n";
	cout << "Enter a string: ";
	char str[size];
	cin >> str;
	
	for (int i = 0; i < size; i++) {
		int tmp = (int)(str[i]);
		kernel->machine->WriteMem(address, 1, tmp);
		address++;
	}
	cout << " sysRead end\n";
	return 0;
}

int SysWrite(int address, int size) {
	cout << "Writing! size: " << size << "\n";
	cout<<endl;
	cout<<"----------------\n";
	for (int i = 0; i < size; i++) {
		int b;
		kernel->machine->ReadMem(address, 1, &b);
		cout << char(b)<<endl;
		address++;
	}
	cout<<endl;
	cout<<"----------------\n";
	cout << " sysWrite end\n";
	return 0;

}






#endif /* ! __USERPROG_KSYSCALL_H__ */
