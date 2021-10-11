#include "syscall.h"

int main() {
    char s[512];
    ReadString(s, 512);
    PrintString(s);
    Halt();
}