//Data structure to manage files in one process in Nachos
#ifndef FTABLE_H
#define FTABLE_H
#include "openfile.h"

#define MAX_FILE 8

class FileTable {
private:
    OpenFile* _files[MAX_FILE];
    int _type[MAX_FILE];
    int _curOffset[MAX_FILE];
public:
    FileTable();
    ~FileTable();
    int Open(OpenFile* pFile, int type);
    int Close(int fid);
    int GetType(int fid);
    int ReadChar(char &c, int fid);
    int WriteChar(char &c, int fid);
    int Seek(int pos, int fid);
};
#endif
