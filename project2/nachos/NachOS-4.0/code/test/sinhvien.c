#include "syscall.h"

int main()
{
    int i,j,k;
    OpenFileID out_file;
    char data[2];
    data[1] = ' ';

    Wait("tape");
    out_file = Open("output.txt",0);
    Signal("tape");

    for (i = 0;i<10;i++){
        Wait("tape");
        data[0]= (char)(ProcessID() + '0');
        
        PrintNum(ProcessID());
        PrintString("\n");

        Append(data,2,out_file);

        k = RandomNum()%10000+100000;
        for(j = 0 ; j < k; j++){}

        Signal("tape");
    }

    Wait("tape");
    Close(out_file);
    Signal("tape");

    Exit(0);
}

