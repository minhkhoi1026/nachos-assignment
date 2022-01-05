// ptable.h 
//      Control process running include PCB table with MAX (10) element.
//      Constructor of PTable class will create parent process on 0 position.
//      From parent process, we will create other process by calling Exec().
// All rights reserved.

#ifndef PTABLE_H
#define PTABLE_H

#include "bitmap.h"
#include "pcb.h"

#define MAX_PROCESS 10

class PTable {
private:
	int psize;
	Bitmap *bm;                 // bitmap for marking used id
	PCB* pcb[MAX_PROCESS];      // core process array

	Semaphore* bmsem;           // semaphore to protect bitmap

public:
    PTable(int = MAX_PROCESS); 
    ~PTable();
		
    int ExecUpdate(char*);
    int ExitUpdate(int);
    int JoinUpdate(int);

    int GetFreeSlot();
    bool IsExist(int pid);
    
    void Remove(int pid);

    char* GetFileName(int id);

    OpenFileID OpenFile(int pid, char* name, int type);
    int CloseFile(int pid, OpenFileID fid);
    int ReadFile(int pid, char *buffer, int charcount, OpenFileID id);
    int WriteFile(int pid, char *buffer, int charcount, OpenFileID id);
    int AppendFile(int pid, char *buffer, int charcount, OpenFileID id);
};
#endif // PTABLE_H

