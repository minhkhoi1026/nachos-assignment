/////////////////////////////////////////////////
// 	DH KHTN - DHQG TPHCM			/
// 	1512034 Nguyen Dang Binh		/
// 	1512042 Nguyen Thanh Chung		/
// 	1512123 Hoang Ngoc Duc			/
/////////////////////////////////////////////////

#include "syscall.h"


void main()
{
	int pingPID, pongPID;
	PrintString("Ping-Pong test starting...\n\n");
	pongPID = Exec("./test/pong");
	pingPID = Exec("./test/ping");
	CreateSemaphore("print",1);
	CreateSemaphore("ping",1);
	CreateSemaphore("pong",1);
	Join(pingPID);
	Join(pongPID);
}
