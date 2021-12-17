#include "syscall.h"

// function to compare two integer base on mode
int cmp(int a, int b, int mode) {
    if (a < b && mode == 0)
        return 1;
    if (a > b && mode == 1)
        return 1;
    return 0;
}

int main()
{
    int mode; //0 -> ascending sort; 1 -> descending sort
    int n = 0;
    int i = 0;
    int j = 0;
    int tmp;
    int a[100];
    
    //chose mode (ascending/descending)
    do
    {
        PrintString("What order do you want?\n0.Ascending\n1.Descending\n");
        mode = ReadNum();
    } while (mode != 0 && mode != 1);
    
    //read n as size for array
    do
    {
        PrintString("Input size of array n (n > 0)\n");
        n = ReadNum();
    } while (n <= 0);

    // read array
    PrintString("Input integer array of size n\n");
    for (i = 0; i < n; i++)
    {
        a[i] = ReadNum();
    }

    // bubble sort
    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (!cmp(a[j], a[j + 1], mode))
            {
                tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }

    // print array after sort
    i = 0;
    for (; i < n; i++) {
        PrintNum(a[i]);
        PrintChar(' ');
    }
        
    Halt();
}
