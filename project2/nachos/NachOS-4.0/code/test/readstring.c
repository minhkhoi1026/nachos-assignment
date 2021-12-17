#include "syscall.h"

int main() {
    char buffer[20];
    ReadString(buffer, 20);
    Halt();
}