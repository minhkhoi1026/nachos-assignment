// Process Control Block
// Save all information to control the process
// All rights reserved.

#ifndef PCB_H
#define PCB_H

#include "ftable.h"
#include "thread.h"
#include "synch.h"
#define MAX_FILENAME_LENGTH 100

class PCB {
private:
    Semaphore* joinsem; // semaphore for join process
    Semaphore* exitsem; // semaphore for exit process
    Semaphore* multex; // semaphore to protect data
    FileTable* ftable;

    int exitcode;		
    int numwait; // n.o process that current process joined

    char FileName[MAX_FILENAME_LENGTH]; // name of process

    Thread* thread; // correspond process
public:
    int parentID; // parent process id
    
    char boolBG; // check if current process is background process
    
    PCB(int);
    ~PCB();

    int Exec(char*, int);
    int GetID();
    int GetNumWait();


    void JoinWait();      
    void ExitWait();

    void JoinRelease();
    void ExitRelease();

    void IncNumWait();
    void DecNumWait();

    void SetExitCode(int);
    int GetExitCode();

    void SetFileName(char*);
    char* GetFileName();

    OpenFileID OpenFile(char* name, int type);
    int CloseFile(OpenFileID fid);
    int ReadFile(char *buffer, int charcount, OpenFileID id);
    int WriteFile(char *buffer, int charcount, OpenFileID id);
    int AppendFile(char *buffer, int charcount, OpenFileID id);
};

#endif // PCB_H