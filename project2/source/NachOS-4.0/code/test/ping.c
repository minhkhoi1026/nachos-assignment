#include "syscall.h"

int main()
{
	int i;
	for(i = 0 ; i < 5; i++)
	{
		Wait("ping");
		Wait("print");
		PrintChar('A');
		Signal("pong");
		Signal("print");
	}
}