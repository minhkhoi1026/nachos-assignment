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

void SysExit(int exitStatus) {
  kernel->processTable->ExitUpdate(exitStatus);
}

/************* system call for execute file *****************/
int SysJoin(int pid) {
  return kernel->processTable->JoinUpdate(pid);
}

/************* system call for execute file *****************/
int SysExec(char* filename) {
  if (!filename) {
    DEBUG(dbgSys, "\n Not enough memory in system");
    return -1;
  }

  // check if executeable file exists
  OpenFile *oFile = kernel->fileSystem->Open(filename);
  if (!oFile)
  {
    DEBUG(dbgSys, "Exec:: Can't open this file.");
    return -1;
  }
  delete oFile;
  
  // Return child process id
  return  kernel->processTable->ExecUpdate(filename);
}

/************* system call for create file *****************/
int SysCreateFile(char* filename) {
  // read filename from user space

  // if kernel buffer's memory is not allocated then halt system
  if (!filename) {
    DEBUG(dbgSys, "Cannot allocate kernel buffer for name string!");
    return -1;
  }
  DEBUG(dbgSys, "Creating " << filename);

  // create file with filename, then deallocate filename buffer, advoid memory leak
  int res = kernel->fileSystem->Create(filename);
  return (res == 1) ? 0 : -1;
}

/************* system call for open file *****************/
int SysOpen(char* filename, int type) {
  if (!filename) {
    DEBUG(dbgSys, "Cannot allocate kernel buffer for name string!");
    return -1;
  }

  DEBUG(dbgSys, "Opening " << filename);  

  
  if (type == 0 || type == 1)
  {
    int pid = kernel->currentThread->processID;
    return kernel->processTable->OpenFile(pid,filename,type);
  }
  else if (type == 2) {  // if type is 2 then return stdin
    return STDIN;
  }
  else if (type == 3) {  // if type is 3 then return stdout
    return STDOUT;
  }
  else {
    DEBUG(dbgSys, "Input Type Error: " << type);
    return -1;
  }
}

/************* system call for close file *****************/
int SysClose(OpenFileID fid) {
  DEBUG(dbgSys, "Closing file with id: " << fid);
  int pid = kernel->currentThread->processID;
  return kernel->processTable->CloseFile(pid, fid);
}

/************* system call for read file *****************/
int SysRead(char *buffer, int charcount, OpenFileID fid) {
  DEBUG(dbgSys, "Read file with id: " << fid);
  int pid = kernel->currentThread->processID;
  return kernel->processTable->ReadFile(pid, buffer, charcount, fid);
}

/************* system call for write file *****************/
int SysWrite(char *buffer, int charcount, OpenFileID fid) {
  DEBUG(dbgSys, "Closing file with id: " << fid);
  int pid = kernel->currentThread->processID;
  return kernel->processTable->WriteFile(pid, buffer, charcount, fid);
}

/************* system call for append file *****************/
int SysAppend(char *buffer, int charcount, OpenFileID fid) {
  DEBUG(dbgSys, "Closing file with id: " << fid);
  int pid = kernel->currentThread->processID;
  return kernel->processTable->AppendFile(pid, buffer, charcount, fid);
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

/************* system call for halt os *****************/
void SysHalt()
{
  kernel->interrupt->Halt();
}


/************* system call for add two number *****************/
int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

/************* system call for create semaphore *****************/
int SysCreateSem(char* name, int semval){
  if (name == NULL)
  {
    DEBUG(dbgSys, "Not enough memory in system");
    return -1;
  }

  return kernel->semTable->Create(name, semval);
}

/************* system call for wait a semaphore *****************/
int SysWait(char* name){
  if (name == NULL)
  {
    DEBUG(dbgSys, "Not enough memory in System");
    return -1;
  }
  return kernel->semTable->Wait(name);
}


/************* system call for signal a semaphore *****************/
int SysSignal(char* name){
  if (name == NULL)
  {
    DEBUG(dbgSys, "Not enough memory in System");
    return -1;
  }
  return kernel->semTable->Signal(name);
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
