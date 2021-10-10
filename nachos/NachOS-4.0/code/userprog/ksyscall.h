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

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}






#endif /* ! __USERPROG_KSYSCALL_H__ */
