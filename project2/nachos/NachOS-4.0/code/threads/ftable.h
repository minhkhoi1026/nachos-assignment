#ifndef FTABLE_H
#define FTABLE_H
#define N_FILE 10
#define OpenFileID int
#define STDIN 0
#define STDOUT 1
#include "openfile.h"

class FileTable {
private:
    OpenFile* core[N_FILE];
    int FindFreeSlot();
public:
    FileTable();
    OpenFileID Open(char* name, int type);
    int Close(OpenFileID fid);
    int Read(char *buffer, int charcount, OpenFileID id);
    int Write(char *buffer, int charcount, OpenFileID id);
};
#endif