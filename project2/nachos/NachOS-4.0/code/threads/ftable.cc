#include "ftable.h"
#include "filesys.h"
#

FileTable::FileTable() {
	for (int i = 0; i < N_FILE; ++i)
	{
		core[i] = NULL;
	}
}

int FileTable::FindFreeSlot()
{
	for(int i = 2; i < N_FILE; i++)
	{
		if (core[i] == NULL) return i;		
	}
	return -1;
}


OpenFileID FileTable::Open(char* name, int type) {
    int freeSlot = FindFreeSlot();
    if (freeSlot == -1) return -1;

    int fileDescriptor = OpenForReadWrite(name, FALSE);
    if (fileDescriptor == -1) return NULL;
    core[freeSlot] = new OpenFile(fileDescriptor, type);
    return freeSlot;
}

int FileTable::Close(OpenFileID fid) {
    if (fid >= 2 && fid < N_FILE) //Chi xu li khi fid nam trong [0, 14]
    {
        if (core[fid]) //neu mo file thanh cong
        {
            delete core[fid]; //Xoa vung nho luu tru file
            core[fid] = NULL; //Gan vung nho NULL
        }
    }
    return -1;
}

int FileTable::Read(char *buffer, int charcount, OpenFileID id) {

}

int FileTable::Write(char *buffer, int charcount, OpenFileID id) {

}