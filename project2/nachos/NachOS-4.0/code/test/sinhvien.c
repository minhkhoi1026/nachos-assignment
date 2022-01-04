#include "syscall.h"

int main()
{
    int i,j,k;
    for (i = 0;i<10;i++){
        Wait("");
        // Cai cho nay la de lay process id xong xuat ra
        k = RandomNum()%10;
        PrintNum(k);
        PrintString("\n");
        for(j = 0 ; j < k; j++)
        {

        }
        Signal("");
    }
    Exit(0);
}