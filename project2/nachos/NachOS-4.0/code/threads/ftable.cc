#include "ftable.h"
#include "filesys.h"
#include "synchconsole.h"
#include "main.h"

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

int FileTable::Read(char *buffer, int charcount, OpenFileID fid) {
    // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
    if (fid < 0 || fid > N_FILE)
    {
        printf("\nKhong the read vi id nam ngoai bang mo ta file.");
        return -1;
    }

    if (fid == 1) // stdout
    {
        printf("\nKhong the read file stdout.");
        return -1;
    }

    if (fid == 0) // stdin
    {
        // Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
        int size = kernel->synchConsoleIn->GetChar();
        return charcount;
    }

    // Kiem tra file co ton tai khong
    if (core[fid] == NULL)
    {
        printf("\nKhong the read vi file nay khong ton tai.");
        return -1;
    }

    // Xet truong hop doc file binh thuong thi tra ve so byte thuc su
    int oldPos = core[fid]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
    if ((core[fid]->Read(buffer, charcount)) > 0)
    {
        // So byte thuc su = NewPos - OldPos
        int newPos = core[fid]->GetCurrentPos();
        return newPos - oldPos;
    }
    else
    {
        // Truong hop con lai la doc file co noi dung la NULL tra ve -2
        //printf("\nDoc file rong.");
        return -2;
    }

    ASSERTNOTREACHED();
}

int FileTable::Write(char *buffer, int charcount, OpenFileID fid) {
    // Kiem tra fid cua file truyen vao co nam ngoai bang mo ta file khong
    if (fid < 0 || fid > N_FILE)
    {
        printf("\nKhong the write vi fid nam ngoai bang mo ta file.");
        return -1;
    }

    // read-only file and stdin cannot write
    if (core[fid]->type == 1 || fid == 1)
    {
        printf("\nKhong the write file stdin hoac file only read.");
        return -1;
    }

    
    if (fid == 1) // stdout
    {
        int i = 0;
        for (int i = 0; buffer[i] != 0 && buffer[i] != '\n'; ++i) // Vong lap de write den khi gap ky tu '\n'
        {
            kernel->synchConsoleOut->PutChar(buffer[i]); // Su dung ham Write cua lop SynchConsole 
        }
        buffer[i] = '\n';
        kernel->synchConsoleOut->PutChar(buffer[i]); // Write ky tu '\n'
        return i - 1;
    }

    // Kiem tra file co ton tai khong
    if (core[fid] == NULL)
    {
        printf("\nKhong the write vi file nay khong ton tai.");
        return -1;
    }

    int oldPos = core[fid]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
    // Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
    if (core[fid]->type == 0) //stdout
    {
        if ((core[fid]->Write(buffer, charcount)) > 0)
        {
            // So byte thuc su = NewPos - OldPos
            int newPos = core[fid]->GetCurrentPos();
            return newPos - oldPos;
        }
    }
}