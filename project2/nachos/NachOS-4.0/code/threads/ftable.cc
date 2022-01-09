#include "ftable.h"
#include "filesys.h"
#include "synchconsole.h"
#include "main.h"

FileTable::FileTable() {
	for (int i = 2; i < N_FILE; ++i) {
		core[i] = NULL;
	}
    core[0]= new OpenFile(0,1);
    core[0]->type = 1;
    core[1]= new OpenFile(0);
    core[0]->type = 0;
}

FileTable::~FileTable() {}

int FileTable::FindFreeSlot() {
	for(int i = 2; i < N_FILE; i++) {
		if (core[i] == NULL) return i;		
	}
	return -1;
}


OpenFileID FileTable::Open(char* name, int type) {
    int freeSlot = FindFreeSlot();
    if (freeSlot == -1) return -1;

    int fileDescriptor = OpenForReadWrite(name, FALSE);
    if (fileDescriptor == -1) return -1;
    core[freeSlot] = new OpenFile(fileDescriptor, type);

    return freeSlot;
}

int FileTable::Close(OpenFileID fid) {
    if (fid >= 2 && fid < N_FILE) // file id should be in range [0,N_FILE)
    {
        if (core[fid]) // only close opened file
        {
            delete core[fid];
            core[fid] = NULL;
        }
    }
    return -1;
}

int FileTable::Read(char* buffer, int charcount, OpenFileID fid) {
    // file id should be in range [0,N_FILE)
    if (fid < 0 || fid > N_FILE) {
        DEBUG(dbgFile, "\nThe file index should be in range [0," << N_FILE << ")");
        return -1;
    }

    // handle stdout
    if (fid == STDOUT) {
        DEBUG(dbgFile, "Cannot read stdout");
        return -1;
    }

    // handle stdin
    if (fid == STDIN) {
        if (buffer != NULL) {
            delete[] buffer;
        }
        buffer = new char[charcount];
        // Use synch console input to get char, loop charcount time
        for (int i = 0; i < charcount; i++) {
            buffer[i] = kernel->synchConsoleIn->GetChar();
        }
        return charcount;
    }

    // check if file is already opened
    if (core[fid] == NULL)
    {
        DEBUG(dbgFile, "The file is not exists!");
        return -1;
    }

    // normal file read
    int oldPos = core[fid]->GetCurrentPos(); 
    if ((core[fid]->Read(buffer, charcount)) > 0)
    {
        // number of readed bytes = new seek position - old seek position
        int newPos = core[fid]->GetCurrentPos();
        return newPos - oldPos;
    }
    else
    {
        // file does not have any character to read
        return -2;
    }

    ASSERTNOTREACHED();
}

int FileTable::Write(char *buffer, int charcount, OpenFileID fid) {
    // file id should be in range [0,N_FILE)
    if (fid < 0 || fid > N_FILE) {
        DEBUG(dbgFile, "\nThe file index should be in range [0," << N_FILE << ")");
        return -1;
    }

    // read-only file and stdin cannot write
    if (fid == STDIN || core[fid]->type == 1) {
        DEBUG(dbgFile, "Cannot write to stdin/read-only file");
        return -1;
    }

    // handle stdout
    if (fid == STDOUT) {
        int i = 0;
        // write until read maximum print character or read end of string
        for (int i = 0; i < charcount && buffer[i] != 0; ++i)
        {
            kernel->synchConsoleOut->PutChar(buffer[i]);
        }
        return i - 1;
    }

    // check if file is already opened
    if (core[fid] == NULL)
    {
        DEBUG(dbgFile, "The file is not exists!");
        return -1;
    }

    // normal file handle
    int oldPos = core[fid]->GetCurrentPos();
    if (core[fid]->type == 0) {
        if ((core[fid]->Write(buffer, charcount)) > 0) {
            // number of writed bytes = new seek position - old seek position
            int newPos = core[fid]->GetCurrentPos();
            return newPos - oldPos;
        }
    }
}

int FileTable::Append(char *buffer, int charcount, OpenFileID fid) {
    // file id should be in range [0,N_FILE)
    if (fid < 0 || fid > N_FILE) {
        DEBUG(dbgFile, "\nThe file index should be in range [0," << N_FILE << ")");
        return -1;
    }

    // read-only file and stdin cannot write
    if (core[fid]->type == 1 || fid == STDIN) {
        DEBUG(dbgFile, "Cannot write to stdin/read-only file");
        return -1;
    }

    // handle stdout
    if (fid == STDOUT) {
        int i = 0;
        for (int i = 0; i < charcount && buffer[i] != 0; ++i) {
            kernel->synchConsoleOut->PutChar(buffer[i]);
        }
        return i - 1;
    }

    // check if file is already opened
    if (core[fid] == NULL) {
        DEBUG(dbgFile, "The file is not exists!");
        return -1;
    }

    // normal file handle
    if (core[fid]->type == 0) {
        int writesize = core[fid]->Append(buffer, charcount);
        return (writesize > 0) ? writesize : -1;
    }
}