#include "syscall.h"

int main() {
    char* buffer;
    ReadString(buffer, 5);
    Halt();
}