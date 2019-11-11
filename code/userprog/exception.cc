// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------
void UserFunct(int addr)
{
	cout<<"HEEEERe \n";
	kernel->currentThread->RestoreUserState();
	
	kernel->machine->Run();

	return;
}
void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {

    // case TlbMissException:
    case PageFaultException:
    	{
    	
    	cout<< "PAGE FAULT \n";


    	kernel->stats->numPageFaults++;
    	// cout<< "Main memory "<<kernel->machine->mainMemory[0]<<endl;

    	int address = kernel->machine->ReadRegister(BadVAddrReg);
	    int virtualPageNumber = (unsigned)address / PageSize;


	    cout<<" virtual address = "<<virtualPageNumber<<endl;
	    cout<<" currentTrhead = "<<kernel->currentThread->id<<endl;
	    
	    int swapLocation = (kernel->currentThread->swapLocation + virtualPageNumber) * PageSize;

		cout<<" swapLocation = "<<swapLocation<<endl;
		// kernel->bitmap->Print();
		
		int index = kernel->bitmap->FindAndSet(); 
		cout<<" index = "<<index<<endl;
		TranslationEntry* entry = kernel->currentThread->space->getEntry(virtualPageNumber);

		kernel->machine->svn = -1;
		kernel->machine->spi = -1;

		if(index == -1)
		{	
			cout<<"needs to be swapped"<<endl;
			index = rand() % NumPhysPages; // swap random page
            cout<<"changing page at index VICTIM  "<<index<<endl;
            
            kernel->machine->svn = kernel->machine->pageTable[index].virtualPage;
            kernel->machine->spi = kernel->machine->pageTable[index].procId;
            
			kernel->swapfile->WriteAt(&(kernel->machine->mainMemory[index * PageSize]), PageSize, kernel->machine->pageTable[index].swap * PageSize);
			
		}

		kernel->swapfile->ReadAt(&(kernel->machine->mainMemory[index * PageSize]), PageSize, swapLocation);


		entry->procId = kernel->currentThread->id;
		entry->valid = TRUE;
		entry->virtualPage = virtualPageNumber;
		entry->swap = kernel->currentThread->swapLocation + virtualPageNumber;
		entry->physicalPage = index;

        kernel->machine->pageTable[index] = *entry;
        kernel->machine->pageTableSize++;


		// for (int i = 0; i < kernel->machine->pageTableSize; ++i)
		// {
		//     cout<<"##### "<<i<<" virtualPage"<< kernel->machine->pageTable[i].virtualPage<<" procId "<<kernel->machine->pageTable[i].procId<<" Phys page "<<kernel->machine->pageTable[i].physicalPage<<endl; 
		// }

		}
    	return;
    	break;
    case SyscallException:
    
	    switch(type) {
	    	
	    	case SC_ThreadYield:
	    		cout<<"YIELDDDed\n";
	    		kernel->currentThread->Yield();

				/* Prepare Result */
				kernel->machine->WriteRegister(2, (int)0);

				
				{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

				}	
				return;
		    
		    case SC_ConsoleRead:
		    	
		    	cout << "Read system call \n" << std::endl;
		    	
		    	{
				
				int size = kernel->machine->ReadRegister(5);
				
				int address = (int)kernel->machine->ReadRegister(4);

				int res = SysRead(address, size);

				/* Prepare Result */
				kernel->machine->WriteRegister(2, (int)res);

				/* Modify return point */
				{
				  /* set previous programm counter (debugging only)*/
				  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				  /* set next programm counter for brach execution */
				  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
				}

				return;

				ASSERTNOTREACHED();
				
				}
			  	break;

		    case SC_ConsoleWrite:
		    	cout << "Write system call \n";
		    	{
				

				int size = kernel->machine->ReadRegister(5);
				int address = kernel->machine->ReadRegister(4);

				int res = SysWrite(address, size);

				/* Prepare Result */
				kernel->machine->WriteRegister(2, (int)res);

				/* Modify return point */
				{
				  /* set previous programm counter (debugging only)*/
				  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				  /* set next programm counter for brach execution */
				  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
				}

				return;

				ASSERTNOTREACHED();

				}

				break;
					
			case SC_ThreadFork:
				{

				int address = kernel->machine->ReadRegister(4);

				Thread *t = new Thread("Forked Thread");
				// cout<<"swap "<< kernel->currentThread->swap<<endl;
				t->swapLocation = kernel->currentThread->swapLocation;
				t->space = new AddrSpace(*(kernel->currentThread->space));

				t->SaveUserState();
				t->userRegisters[PCReg] = address;
				t->userRegisters[NextPCReg] = address + 4;


				t->Fork((VoidFunctionPtr)UserFunct, (void *)0);


				/* Prepare Result */
				kernel->machine->WriteRegister(2, (int)address);
				
				
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

				}	


				return;

				ASSERTNOTREACHED();

				break;
		    
		    case SC_Halt:

				DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

				SysHalt();

				ASSERTNOTREACHED();
				break;

	    	case SC_Add:

				DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
				
				/* Process SysAdd Systemcall*/
				int result;
				result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
						/* int op2 */(int)kernel->machine->ReadRegister(5));

				DEBUG(dbgSys, "Add returning with " << result << "\n");
				
				/* Prepare Result */
				kernel->machine->WriteRegister(2, (int)result);
				
				/* Modify return point */
				{
				  /* set previous programm counter (debugging only)*/
				  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				  /* set next programm counter for brach execution */
				  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
				}

				return;
				
				ASSERTNOTREACHED();

				break;
			
			case SC_ThreadExit:
				{

				int tlbClear = FALSE;

				// for (int i = 0; i < TLBSize; i++)
				// {
					// cout<<"tlb[i] "<<kernel->machine->tlb[i].virtualPage<<endl;
					// if ((kernel->machine->tlb[i].procId == kernel->currentThread->id))
					// {

						// kernel->machine->tlb[i].procId = -1;

						// tlbClear = TRUE;
					// }
				// }

				int currentId = kernel->currentThread->id;

				for (int i = 0; i < NumPhysPages; i++) {
					if (kernel->machine->pageTable[i].procId == currentId) {
						TranslationEntry* entry = kernel->currentThread->space->getEntry(i);

						entry->valid = FALSE;
						kernel->bitmap->Clear(i);
						bzero(&kernel->machine->mainMemory[i * PageSize], PageSize);

					}
				}

				kernel->bitmap->Print();
				kernel->stats->Print();

				if(tlbClear == 1)
				{
					std::cout<<"Clearing the entry from TLB"<<std::endl;
					std::cout << std::endl;
				}

				kernel->currentThread->Finish();
				}
				return;
				
				ASSERTNOTREACHED();

				break;
			
			case SC_Exec:
				{
		        int prog = kernel->machine->ReadRegister(4);

		        char *programName = new char[100];
		        int i = 0; 
		        int c = 1;
		        
		        // while ( c != 0 && i < 98 ){
		        //   kernel->machine->ReadMem(prog+i, 1, &c);
		        //   programName[i++] = (char) c;
		        // }

		        programName[i] = '\0';

		  //       cout<< "program name: "<< programName<<endl;
		  //       // Thread *t = new Thread(programName);
		  //       // t->Fork(Exec, (int) programName);
		  //       // DEBUG('e', "  Exec(\"%s\") [%d] in thread '%s'.", programName, i-1,
		  //       //       currentThread->getName());
		  //       // machine->WriteRegister(2, (int) t->id);
		  //       // AdvanceProgramCounters();
		  //       				/* Prepare Result */

				kernel->machine->WriteRegister(2, (int)result);
				
				/* Modify return point */
				{
				  /* set previous programm counter (debugging only)*/
				  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				  /* set next programm counter for brach execution */
				  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
				}

		        return;
				
				ASSERTNOTREACHED();

				break;
		    	}


		    default:

				cerr << "Unexpected system call " << type << "\n";
				break;
		}
    
    	break;
    
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
