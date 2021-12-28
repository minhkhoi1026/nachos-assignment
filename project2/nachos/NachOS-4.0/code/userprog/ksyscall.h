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
#define MAX_FILENAME_LENGTH 256

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

int SysExit(int exitStatus) {
  if(exitStatus != 0) {
    return exitStatus;
  }			
  
  int res = kernel->processTable->ExitUpdate(exitStatus);

  kernel->currentThread->FreeSpace();
  kernel->currentThread->Finish();
  return res;
}

/************* system call for execute file *****************/
int SysJoin(int pid) {
  return kernel->processTable->JoinUpdate(pid);
}

/************* system call for execute file *****************/
SpaceId SysExec(int bufferUser) {
  char* name;
  name = User2System(bufferUser, MAX_FILENAME_LENGTH + 1); // Lay ten chuong trinh, nap vao kernel

  if (!name)
  {
    DEBUG(dbgSys, "\n Not enough memory in System");
    return -1;
  }
  OpenFile *oFile = kernel->fileSystem->Open(name);
  if (!oFile)
  {
    DEBUG(dbgSys, "\nExec:: Can't open this file.");
    return -1;
  }
  delete oFile;
  
  DEBUG(dbgThread, name);
  // Return child process id
  int pid = kernel->processTable->ExecUpdate(name); 
  
  delete[] name;	
  return pid;
}

/************* system call for create file *****************/
int SysCreateFile(int bufferUser) {
  // read filename from user space
  char* filename = User2System(bufferUser, MAX_FILENAME_LENGTH);

  // if kernel buffer's memory is not allocated then halt system
  if (!filename) {
    DEBUG(dbgSys, "Cannot allocate kernel buffer for name string!");
    return -1;
  }
  DEBUG(dbgSys, "Creating " << filename);

  // create file with filename, then deallocate filename buffer, advoid memory leak
  int res = kernel->fileSystem->Create(filename);
  delete filename;
  
  return (res == 1) ? 0 : -1;
}

/************* system call for read number *****************/
int SysReadNum() {
  char digitChar;
  int result = 0;
  bool legal = true;
  bool isNeg = false;
  bool hasNum = false;
  do
  {
    digitChar = kernel->synchConsoleIn->GetChar();
    if (digitChar >= '0' && digitChar <= '9') {
      result = result*10 + (int)(digitChar - '0');
      hasNum = true;
    }
    else if (digitChar == '-')
    {
      if (hasNum == true) legal = false;
      else isNeg = true;
    }
    else if (digitChar == ' ' || digitChar == '\n')
    {
      if (legal == false) break;
      if (hasNum == false && isNeg == false) continue;
      else break;
    }
    else legal = false;
  }while (true);

  if (isNeg == true) result = -result;
  if (legal == false) return 0;
  return result;
}

/************* system call for print number *****************/
 void SysPrintNum (int n) {
   char *digits = new char[10];
   bool isNeg = false;
   int i = 0;

   if (n == 0)
   {
     kernel->synchConsoleOut->PutChar('0');
     return;
   }

   if (n < 0)
   {
     isNeg = true;
     n = -n;
   }
   
   while (n > 0)
   {
     digits[i]=(char)(n%10+'0');
     i++;
     n/=10;
   }

  if (isNeg == true)
    kernel->synchConsoleOut->PutChar('-');
  for (int j = i -1; j >= 0; j--) 
    kernel->synchConsoleOut->PutChar(digits[j]);

  delete[] digits;
  return;
 }

/************* system call for random number *****************/
int SysRandomNum() {
  srand(kernel->stats->totalTicks + time(0)); // set seed number
  return rand() * 1LL * rand() % (long long)(MAX_RAND);
}

int SysReadChar() {
  char wordChar;
  wordChar = kernel->synchConsoleIn->GetChar();
  if (wordChar > 31 && wordChar < 126){
    return wordChar;
  } else if (wordChar > 9 && wordChar < 12){
      return wordChar;
  }
  else {
    //printf("Ky tu nhap vao khong hop le!\n");
    DEBUG('u', "\nERROR: Ky tu nhap vao khong hop le!");
    return '\0';
  }
}

/************* system call for print character *****************/
void SysPrintChar(char wordChar) {
  if (wordChar > 31 && wordChar < 126){
    kernel->synchConsoleOut->PutChar(wordChar);
  } else if (wordChar > 9 && wordChar < 12){      // Ky tu xuong dong va tab
    kernel->synchConsoleOut->PutChar(wordChar);
  }
  else {
    //printf("Ky tu in khong hop le!\n");
    DEBUG('u', "\nERROR: Ky tu in khong hop le!");
  }
}

/************* system call for read string *****************/
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

/************* system call for print string *****************/
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
