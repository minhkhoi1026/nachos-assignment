
#include "syscall.h"

int
main()
{
    int res = CreateFile("test.txt");
    if (res == 0)
        PrintString("Create file successful");
    else
        PrintString("Error, cannot create file");
    Halt();
}