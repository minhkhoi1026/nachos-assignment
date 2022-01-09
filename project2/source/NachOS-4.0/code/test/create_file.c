#include "syscall.h"

int
main()
{
    int id = Exec("./test/sinhvien");
    Join(id);
    Exit(0);
}