#include "syscall.h"

int main()
{
	SpaceId id_file;
	int f_Success;
	char* data;

	f_Success = CreateFile("mydata.txt");
	if(f_Success == -1)
		return 1;
	
	id_file = Open("mydata.txt", 0);

	Read(data, 2, id_file);
	PrintString(data);
	Close(id_file);
}
