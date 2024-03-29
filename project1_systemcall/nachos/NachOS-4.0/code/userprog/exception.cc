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

void IncreasePC() {
	/* Modify return point */
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	
	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
}

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

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
		case NoException:
			return;
		case PageFaultException:
			cerr << "No valid translation found.\n";
			DEBUG(dbgAddr, "No valid translation found.\n");
			SysHalt();
			break;
		case ReadOnlyException:
			cerr << "Write attempted to page marked read-only.\n";
			DEBUG(dbgAddr, "Write attempted to page marked read-only.\n");
			SysHalt();
			break;
		case BusErrorException:
			cerr << "Translation resulted in an invalid physical address.\n";
			DEBUG(dbgAddr, "Translation resulted in an invalid physical address.\n");
			SysHalt();
			break;
		case AddressErrorException:
			cerr << "Unaligned reference or one that was beyond the end of the address space.\n";
			DEBUG(dbgAddr, "Unaligned reference or one that was beyond the end of the address space.\n");
			SysHalt();
			break;
		case OverflowException:
			cerr << "Integer overflow in add or sub.\n";
			DEBUG(dbgAddr, "Integer overflow in add or sub.\n");
			SysHalt();
			break;
		case IllegalInstrException:
			cerr << "Unimplemented or reserved instr.\n";
			DEBUG(dbgAddr, "Unimplemented or reserved instr.\n");
			SysHalt();
			break;
		case NumExceptionTypes:
			cerr << "Number Exception Types.\n";
			DEBUG(dbgAddr, "Number Exception Types.\n");
			SysHalt();
			break;
		case SyscallException:
			switch(type) {
				case SC_ReadNum: {
					int result = SysReadNum();
					DEBUG(dbgSys, "My result of ReadNum is " << result);
					kernel->machine->WriteRegister(2, (int)result);
					IncreasePC();
					return;

					ASSERTNOTREACHED();
					break;
				}
				
				case SC_PrintNum: {
					DEBUG(dbgSys, "My result of WriteNum is ");
					int tmp = kernel->machine->ReadRegister(4);
					SysPrintNum(tmp);
					IncreasePC();
					return;

					ASSERTNOTREACHED();
					break;
				}
				case SC_RandomNum: {
					int result = SysRandomNum();
					kernel->machine->WriteRegister(2, result);

					DEBUG(dbgSys, "Random number is " << result);
					IncreasePC();
					return;

					ASSERTNOTREACHED();
					break;
				}

				case SC_ReadChar: {
					int result = SysReadChar();
					kernel->machine->WriteRegister(2, (int)result);
					DEBUG(dbgSys, "Read char " << (int)result << '\n');
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}
				case SC_PrintChar: {
					int result = kernel->machine->ReadRegister(4);
					DEBUG(dbgSys, "Print char " << (int)result << '\n');
					SysPrintChar((char)result);
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}

				case SC_ReadString: {
					int bufferUser = kernel->machine->ReadRegister(4);
					int length = kernel->machine->ReadRegister(5); 

					SysReadString(bufferUser, length);
					
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}

				case SC_PrintString: {
					int bufferUser = kernel->machine->ReadRegister(4);
					SysPrintString(bufferUser);
					
					IncreasePC();
					return;

					ASSERTNOTREACHED();
					break;
				}
	
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
					
					IncreasePC();

					return;
					
					ASSERTNOTREACHED();
					break;

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