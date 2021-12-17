
#include "syscall.h"

int
main()
{
    int res = CreateFile("test.txt");
    Halt();
}