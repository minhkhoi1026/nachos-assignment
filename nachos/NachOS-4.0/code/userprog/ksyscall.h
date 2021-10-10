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
  bool isNeg = false;
  bool legal = true;
  for (int i = 0; i >= 0; i++)
  {
    digitChar = Kernel->SynchConsoleInput->GetChar();
    if (i == 0)
    {
	if (digitChar == '-') {
	  isNeg = true;
	  continue; }
	else if (ditgitChar == ' ')
	  i--;
	else if (digitChar >= '0' && digitChar <= '9')
	  result = 10*result +(int)(digitChar - '0');
	else 
	  legal = false;
    }
    else 
    {
	if (digitChar >='0' && digitChar <='9') 
	  result = 10*result +(int)(digitChar -'0');
	else if (digitChar == ' ' && i > 0) break;
	else legal = false;
    }
    
  } 
  
  if (isNeg == true) result = -1*result;

  if (legal == false) return 0;
  return result;
}

void SysWriteNum (int n) {
  n = Machine->ReadRegister(4);
  int digits = 0;
  if (n == 0) 
	SynchConsoleOutput->PutChar('0');

  else 
  {
    if (n < 0)
      SynchConsoleOutput->PutChar('-');
  
  for (int i = 1; i > 0; i = 10*i)
  {
     if (i <= n) digits++;
     else break;
  }

  int i = 1;
  for (int j = 1; j <= digits; j++)
	i = 10*i;
  
  for (int j = 1; j <= digits; j++)
  {
    int tmp;
    tmp = n/i;
    SynchConsoleOutput((char)(tmp + 48));
    n = n%i;
    i = i/10;
  }
  }
  IncreasePC();
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
