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
				case SC_Exec:
				{
					int bufferUser = kernel->machine->ReadRegister(4);
					
					char* filename = User2System(bufferUser, MAX_FILENAME_LENGTH + 1);
					int pid = SysExec(filename);
					delete[] filename;

					kernel->machine->WriteRegister(2, pid);
					IncreasePC();
					return;
				}
				case SC_Join:
				{       
					int pid = kernel->machine->ReadRegister(4);
				
					int res = SysJoin(pid);
					
					kernel-> machine->WriteRegister(2, res);
					IncreasePC();
					return;
				}
				case SC_Exit:
				{
					int exitStatus = kernel->machine->ReadRegister(4);

					SysExit(exitStatus);

					IncreasePC();
					return; 
				}
				case SC_CreateFile:{
					int bufferUser = kernel->machine->ReadRegister(4);

  					char* filename = User2System(bufferUser, MAX_FILENAME_LENGTH);
					int res = SysCreateFile(filename);
					delete[] filename;
					if (res == 0) {
						DEBUG(dbgSys, "\nCreate file successful");
					}
					else {
						DEBUG(dbgSys, "\nError when create file!");
					}

					kernel->machine->WriteRegister(2, (int)res);
					
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}
				case SC_Open:{
					int bufferUser = kernel->machine->ReadRegister(4); // get pointer of name
					int type = kernel->machine->ReadRegister(5); // get type of open file
					char* filename;

					filename = User2System(bufferUser, MAX_FILENAME_LENGTH); 
					int res = SysOpen(filename, type);
					delete[] filename;

					if (res != -1) {
						DEBUG(dbgSys, "\nOpen file successful");
					}
					else {
						DEBUG(dbgSys, "\nError when open file!");
					}

					kernel->machine->WriteRegister(2, (int)res);
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}				
				case SC_Close:{
					OpenFileID id = kernel->machine->ReadRegister(4);

					int res = SysClose(id);
					if (res == 0) {
						DEBUG(dbgSys, "\nClose file successful");
					}
					else {
						DEBUG(dbgSys, "\nError when close file!");
					}

					kernel->machine->WriteRegister(2, (int)res);
					
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}
				case SC_Read:{
					int bufferUser = kernel->machine->ReadRegister(4); // get user buffer for assign result
					int charcount = kernel->machine->ReadRegister(5); // get charcount as maximum number of byte to read
					OpenFileID id = kernel->machine->ReadRegister(6); // get file id

					char* buffer = User2System(bufferUser,charcount);
					int size = SysRead(buffer,charcount,id);
					System2User(bufferUser, size,buffer);
					delete[] buffer;

					if (size >= 0) {
						DEBUG(dbgSys, "\nRead file successful");
					}
					else {
						DEBUG(dbgSys, "\nError when read file!");
					}
					kernel->machine->WriteRegister(2, (int)size);
					
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}

				case SC_Write:{
					int bufferUser = kernel->machine->ReadRegister(4); // get user buffer for write result
					int charcount = kernel->machine->ReadRegister(5); // get charcount as maximum number of byte to write
					OpenFileID id = kernel->machine->ReadRegister(6); // get file id

					char* buffer = User2System(bufferUser,charcount);
					int res = SysWrite(buffer,charcount,id);
					delete[] buffer;

					if (res == 0) {
						DEBUG(dbgSys, "\nWrite file successful");
					}
					else {
						DEBUG(dbgSys, "\nError when Write file!");
					}

					kernel->machine->WriteRegister(2, (int)res);
					
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}

				case SC_Append:{
					int bufferUser = kernel->machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
					int charcount = kernel->machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
					int id = kernel->machine->ReadRegister(6); // Lay id cua file tu thanh ghi so 6 
					char* buffer = User2System(bufferUser,charcount);
					int res = SysAppend(buffer,charcount,id);
					delete[] buffer;
					if (res == 0) {
						DEBUG(dbgSys, "\nAppend file successful");
					}
					else {
						DEBUG(dbgSys, "\nError when append file!");
					}
					kernel->machine->WriteRegister(2, (int)res);
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}
				
				case SC_ReadNum: {
					int res = SysReadNum();
					DEBUG(dbgSys, "My result of ReadNum is " << res);
					kernel->machine->WriteRegister(2, (int)res);
					IncreasePC();
					return;

					ASSERTNOTREACHED();
					break;
				}				
				case SC_ProcessID:{
					int res = kernel->currentThread->processID;
					DEBUG(dbgSys, "My ProcessID is " << res);
					kernel->machine->WriteRegister(2, (int)res);
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
					int res = SysRandomNum();
					kernel->machine->WriteRegister(2, res);

					DEBUG(dbgSys, "Random number is " << res);
					IncreasePC();
					return;

					ASSERTNOTREACHED();
					break;
				}

				case SC_ReadChar: {
					int res = SysReadChar();
					kernel->machine->WriteRegister(2, (int)res);
					DEBUG(dbgSys, "Read char " << (int)res << '\n');
					IncreasePC();
					return;
					ASSERTNOTREACHED();
					break;
				}
				case SC_PrintChar: {
					int res = kernel->machine->ReadRegister(4);
					DEBUG(dbgSys, "Print char " << (int)res << '\n');
					SysPrintChar((char)res);
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
					int res;
					res = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
							/* int op2 */(int)kernel->machine->ReadRegister(5));

					DEBUG(dbgSys, "Add returning with " << res << "\n");
					/* Prepare Result */
					kernel->machine->WriteRegister(2, (int)res);
					
					IncreasePC();

					return;
					
					ASSERTNOTREACHED();
					break;

				case SC_CreateSemaphore: {
					int bufferUser = kernel->machine->ReadRegister(4);
					int semval = kernel->machine->ReadRegister(5);

    				char *name = User2System(bufferUser, MAX_FILENAME_LENGTH + 1);
					int res = SysCreateSem(name, semval);

					kernel->machine->WriteRegister(2, res);

      				delete[] name;

					IncreasePC();

					return;

					ASSERTNOTREACHED();
					break;
				}

				case SC_Wait: {
					int virtAddr = kernel->machine->ReadRegister(4);

					char *name = User2System(virtAddr, MAX_FILENAME_LENGTH + 1);
					int res = SysWait(name);
					delete[] name;
					kernel->machine->WriteRegister(2, res);
					IncreasePC();
					return;
					
					ASSERTNOTREACHED();
					break;
				}

				case SC_Signal:	{
					int virtAddr = kernel->machine->ReadRegister(4);

					char *name = User2System(virtAddr, MAX_FILENAME_LENGTH + 1);
					int res = SysSignal(name);
					delete[] name;
					kernel->machine->WriteRegister(2, res);
					IncreasePC();
					return;
					
					ASSERTNOTREACHED();
					break;
				}

				default:
						DEBUG(dbgSys, " Create file result: ");
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