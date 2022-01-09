#include "syscall.h"

int main()
{
    int i,j,k;
    OpenFileID out_file;
    // output data for write
    char data[2];
    data[0]= (char)(ProcessID() + '0');
    data[1] = ' ';

    out_file = Open("output.txt",0);

    for (i = 0;i<10;i++){
        Wait("tape"); // wait for tape
        
        // sleep for random time, then append result to file
        k = RandomNum()%100+100;
        for(j = 0 ; j < k; j++){}
        Append(data,2,out_file);

        Signal("tape"); // release tape
    }

    Close(out_file);

    Exit(0);
}

