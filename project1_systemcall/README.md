# Project 1 - System call

Workflow:
1. Khai báo syscall tại syscall.h
2. Khai báo syscall trong start.s
3. Thêm system call vào Exception Handler
4. Viết chương trình xài thử system call

## Khai báo syscall tại syscall.h
Trong file syscall.h thực hiện các bước sau:
- Define một hằng số là mã của system call, mã này chỉ của riêng system call đó, không được trùng với system call nào khác.
```C++
#define SC_ReadNum 45
```
- Khai báo prototype của system call
```C++
int ReadNum();
```

## Khai báo syscall trong start.s
Có prototype rồi, giờ muốn trỏ tới cài đặt thì sao? Đáp án: khai báo trong start.s
```
	.globl ReadNum
	.ent	ReadNum
ReadNum:
	addiu $2,$0,SC_ReadNum
	syscall
	j 	$31
	.end ReadNum
```

## Thêm system call vào Exception Handler
Khai báo xong rồi, giờ xài ở đâu? Đáp án: trong hàm ExceptionHandler của file `exception.cc`. Hàm này có nhiệm vụ xử lý một exception (có thể là syscall, có thể là interrupt,...).

Trong hàm này có 1 cái switch case lớn để xem đây là loại exception nào, tuy nhiên trong project này mình quan tâm tới xử cái case `SyscallException` thôi. Trong cái case này lại có một cái switch-case nhỏ để xử lý từng loại system call. Bingo, đây là chỗ mình sẽ làm việc!

Trong switch-case nhỏ, mình thêm một case mã system call lúc nãy mình đặt ở bước 1 để bắt đầu xử lý
```C++
void ExceptionHandler(ExceptionType which)
{
    //...
    int type = kernel->machine->ReadRegister(2);

    switch (which) {
		case SyscallException:
			switch(type) {
				case SC_ReadNum: //  cái SC_ReadNum này nãy define bên syscall.h nè 
					// xử lý system call ở đây
					break;
				case SC_Halt:
					//...
                //...
            }
        //...
    }
```

Để xử lý cho sạch đẹp, mình nên code syscall ở file khác, cụ thể là file `kernel.h`, khi đó các syscall sẽ nằm 1 chỗ nên có gì sai thì mình dễ quản lý. 
```C++
void ExceptionHandler(ExceptionType which)
{
    //...
    int type = kernel->machine->ReadRegister(2);

    switch (which) {
		case SyscallException:
			switch(type) {
				case SC_ReadNum: //  cái SC_ReadNum này nãy define bên syscall.h nè 
					SysReadNum(); // cài đặt cái này bên file ksyscall.h
					break;
				case SC_Halt:
					//...
                //...
            }
        //...
    }
```

Xong thì mình qua file `ksyscall.h` để cài đặt thực sự cái syscall nãy bên exception mình khai báo (SysReadNum)
```
int SysReadNum() {
  char digitChar;
  int result = 0; 
  do
  {
    digitChar = kernel->synchConsoleIn->GetChar();
    if ('0' <= digitChar && digitChar <= '9')
      result = result * 10 + int(digitChar - '0');
  } while (digitChar != '\n');
  
  return result;
}
```

## Viết chương trình xài thử system call
Để viết chương trình test system call, mình vào thư mục `nachos/NachOS-4.0/code/test`. Tạo một file mới và bắt đầu code, ví dụ như ở đây mình có file `readnum.c` như sau:
```C++
#include "syscall.h"

int main()
{
    int a = ReadNum();
    Halt();
    /* not reached */
}
```

Giải thích:
- Cần `include` thư viện `syscall.h` để cung cấp cho người dùng các system call của hệ điều hành nachos (chính là mình lập trình).
- Hàm `ReadNum()` lúc nãy được khai báo trong `syscall.h` và `start.S`.

Sau khi viết hàm xong, mình cần cấu hình lại file `Makefile` trong thư mục của file vừa lập trình (thư mục `nachos/NachOS-4.0/code/test`). với file `readnum.c` hồi nãy mình sẽ cấu hình thế này:
```
PROGRAMS = .... readnum ....

readnum.o: readnum.c
	$(CC) $(CFLAGS) -c readnum.c
readnum: readnum.o start.o
	$(LD) $(LDFLAGS) start.o readnum.o -o readnum.coff
	$(COFF2NOFF) readnum.coff readnum
```

Ở đây, mình có rule `readnum.o` cần dependency là file `readnum.c` để có thể chạy, khi chạy thì rule này sẽ dịch từ c sang file object (đuôi .o). 

Tiếp đến, mình có rule `readnum` cần dependencies là file `readnum.o` lúc nãy vừa tạo và file `start.o` (nếu không có thì chạy rule tương ứng), khi chạy rule này thì mình sẽ có được file thực thi `readnum` để đưa vào chương trình nachos thực thi.

Mình chỉ mới định nghĩa các rule mà chưa gọi nó, để thêm rule vào danh sách được make thì phải thêm nó vào biến `PROGRAMS` chính là danh sách các file được make.

Cuối cùng, khi setup xong, mình mở terminal, vào đúng thư mục có chứa makefile và gõ `make`.

## Chạy thử và debug
Để chạy được file thực thi vừa nãy thì mình phải compile hệ điều vào `nachos/NachOS-4.0/code/build.linux`, mở terminal ở thư mục này, gõ `make` (nếu lỗi thì gõ `make depend` trước rồi thử gõ `make` lại) để compile version mới của hệ điều hành nachos (lúc nãy mình lập trình ở các bước 1, 2, 3).

Sau đó, mình sẽ dùng hệ điều hành nachos vừa compile để chạy file thực thi `readnum` của người dùng, mở terminal lên trong đúng thư mục lúc nãy compile nachos và gõ:
```
./nachos -d u -x ../test/readnum
```

Giải thích:
- option `-d`: để in ra các thông số cần khi debug, tham số `u` ở đây là in ra các thông báo debug của chương trình, các tham số được định nghĩa trong file `debug.h`, có các loại sau:
```cpp
const char dbgAll = '+';		// turn on all debug messages
const char dbgThread = 't';		// threads
const char dbgSynch = 's';		// locks, semaphores, condition vars
const char dbgInt = 'i'; 		// interrupt emulation
const char dbgMach = 'm'; 		// machine emulation
const char dbgDisk = 'd'; 		// disk emulation
const char dbgFile = 'f'; 		// file system
const char dbgAddr = 'a'; 		// address spaces
const char dbgNet = 'n'; 		// network emulation
const char dbgSys = 'u';        // systemcall
```
- option `-x`: để cho biết file nào sẽ được thực thi, ở đây là file `readnum` ở thư mục `test`.
- Ngoài ra còn các option khác, tham khảo ở `kernel.cc`.