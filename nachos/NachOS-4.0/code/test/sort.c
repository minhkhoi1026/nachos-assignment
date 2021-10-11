#include "syscall.h"


int main()
{
    int mode; //0 -> ascending sort; 1 -> descending sort
    int n = 0;
    int i = 0;
    int j = 0;
    int tmp;
    int a[100];
    
    //chọn mode
    do
    {
        mode = ReadNum();
    } while (mode != 0 && mode != 1);
    

    //nhập n
    do
    {
       n = ReadNum();
    } while (n <= 0);

    // nhập mảng
    for (; i < n; i++)
    {
        a[i] = ReadNum();
    }

    // bubble sort tăng dần
    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (a[j] > a[j + 1])
            {
                tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }

    //xoay nguoc mang neu sort descending
    if (mode == 1) i = 0;
    {
        for (; i <= n/2 - 1; i++)
        {
            tmp = a[i];
            a[i] = a[n - 1 - i];
            a[n - 1 -i] = tmp;
        }
    }
    
    //in mảng sau khi đã sort
    i = 0;
    for (; i < n; i++) {
        PrintNum(a[i]);
    }
    Halt();
}
