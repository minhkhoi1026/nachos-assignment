#include "syscall.h"

int main()
{
	SpaceId id_file;
	int f_Success;
	char* data;
	
	Read(data, 2, 0);
	PrintString(data);
	Close(id_file);
}
