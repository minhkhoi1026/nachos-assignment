#include "syscall.h"

int main()
{
	int i;
	for(i = 0 ; i < 5; i++)
	{
		Wait("pong");
		Wait("print");
		PrintChar('B');
		Signal("print");
		Signal("ping");
	}
}