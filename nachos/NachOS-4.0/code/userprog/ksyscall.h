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
#include "synchconsole.h"

#define MAX_PRINT_LENGTH 512
#define MAX_RAND 1e9

int SysReadNum() {
  char digitChar;
  int result = 0; 
  do
  {
    digitChar = kernel->synchConsoleIn->GetChar();
    if ('0' <= digitChar && digitChar <= '9')
      result = result * 10 + int(digitChar - '0');
  } while (digitChar != '\n');

  return result;
}

int SysRandomNum() {
  srand(kernel->stats->totalTicks); // set seed number
  return rand() * 1LL * rand() % MAX_RAND;
}

/*
Input: - User space address (int)
- Limit of buffer (int)
- Buffer (char[])
Output:- Number of bytes copied (int)
Purpose: Copy buffer from System memory space to User memory space
*/
int System2User(int virtAddr,int length, char* buffer)
{
	if (length < 0) return -1;
	if (length == 0) return length;
	int i = 0;
	for (int i = 0; i < length; ++i) {
		int charAsInt= (int) buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, charAsInt);
    if (charAsInt == 0) break;
	}
	return i;
}

/*
Input: - User space address (int)
- Limit of buffer (int)
Output:- Buffer (char*)
Purpose: Copy buffer from User memory space to System memory space
*/
char* User2System(int virtAddr,int limit)
{
  int i; // index
  int charAsInt;
  char* kernelBuf = NULL;
  kernelBuf = new char[limit + 1]; //need for terminal string

  if (kernelBuf == NULL)
    return kernelBuf;

  memset(kernelBuf, 0, limit + 1);
  
  for (i = 0; i < limit; i++)
  {
    kernel->machine->ReadMem(virtAddr + i, 1, &charAsInt);
    kernelBuf[i] = (char)charAsInt;
    if (charAsInt == 0)
      break;
  }
  return kernelBuf;
}

void SysReadString(int bufferUser, int maxLength) {
  // if maxLength is <= 0 then halt system
  if (maxLength <= 0) {
    DEBUG(dbgSys, "Buffer length is less than or equal than zero!");
    kernel->interrupt->Halt();
  }

  char* bufferKernel = new char[maxLength];

  // if kernel buffer's memory is not allocated then halt system
  if (!bufferKernel) {
    DEBUG(dbgSys, "Cannot allocate kernel buffer for read string!");
    kernel->interrupt->Halt();
  }

  // read at most maxLength character from console
  int length = 0;
  while (length < maxLength) {
    bufferKernel[length] = kernel->synchConsoleIn->GetChar();
    ++length;
    if (bufferKernel[length - 1] == '\n') {
      bufferKernel[length - 1] = '\0';
      break;
    }
  }

  // if user input is too long then set buffer to zero
  if (bufferKernel[length - 1] != '\0') {
    DEBUG(dbgSys, "Not enough space for input string, reset buffer to empty!");
    bufferKernel[0] = '\0';
    length = 1;
  }

  DEBUG(dbgSys, "Read string is " << bufferKernel);

  // pass data from kernel buffer to user buffer
  System2User(bufferUser, length, bufferKernel);

  delete[] bufferKernel;
}

void SysPrintString(int bufferUser) {
  char* bufferKernel = User2System(bufferUser, MAX_PRINT_LENGTH);

  // if kernel buffer's memory is not allocated then halt system
  if (!bufferKernel) {
    DEBUG(dbgSys, "Cannot allocate kernel buffer for print string!");
    kernel->interrupt->Halt();
  }

  // print at most MAX_PRINT_LENGTH character
  for (int i = 0; i < MAX_PRINT_LENGTH; ++i) {
    if (bufferKernel[i] == '\0') break; // end of string
    kernel->synchConsoleOut->PutChar(bufferKernel[i]);
  }
}

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}






#endif /* ! __USERPROG_KSYSCALL_H__ */
