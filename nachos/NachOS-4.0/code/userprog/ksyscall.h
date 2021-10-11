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

 void SysPrintNum (int n) {
   int len = 0;
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

int SysRandomNum () {

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
