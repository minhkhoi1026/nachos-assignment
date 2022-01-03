#include "syscall.h"


void main()
{
	int pingPID, pongPID;
	PrintString("Ping-Pong test starting...\n\n");
	CreateSemaphore("print",1);
	CreateSemaphore("ping",1);
	CreateSemaphore("pong",1);
	pongPID = Exec("./test/pong");
	pingPID = Exec("./test/ping");
	Join(pingPID);
	Join(pongPID);
}
