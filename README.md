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


## Một số lưu ý khi debug