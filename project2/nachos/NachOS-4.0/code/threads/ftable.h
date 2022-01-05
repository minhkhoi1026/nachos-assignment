#ifndef FTABLE_H
#define FTABLE_H
#define N_FILE 10
#define STDIN 0
#define STDOUT 1
#include "openfile.h"

typedef int OpenFileID;	
class FileTable {
private:
    OpenFile* core[N_FILE];
    int FindFreeSlot();
public:
    FileTable();
    ~FileTable();
    OpenFileID Open(char* name, int type);
    int Close(OpenFileID fid);
    int Read(char *buffer, int charcount, OpenFileID id);
    int Write(char *buffer, int charcount, OpenFileID id);
    int Append(char *buffer, int charcount, OpenFileID id);
};
#endif
