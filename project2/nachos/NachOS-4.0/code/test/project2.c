#include "syscall.h"

int main() {
    int num_sv = 0;
    int i;
    int ProcessID[5];
    char* data;
	int inp_file = Open("input.txt", 1);
    CreateSemaphore("",1);
    if (inp_file == -1){
        num_sv = ReadNum();
    } else {
        Read(data, 1,inp_file);
        if (data[0]>='0'&&data[0]<='5'){
            num_sv = data[0]-'0';             
        } else {
            PrintString("Input Khong Hop Le!");
            return 0;
        }
    }
    for (i=0;i<num_sv;i++){
        ProcessID[i] = Exec("./test/sinhvien");
        Join(ProcessID[i]);
    }
}