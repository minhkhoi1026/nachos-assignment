#include "ptable.h"
#include "main.h"

//----------------------------------------------------------------------
// PCB table
//----------------------------------------------------------------------

PTable::PTable(int size)
{
    psize = max(1, min(size, MAX_PROCESS));
    bm = new Bitmap(psize);
    bmsem = new Semaphore("bmsem", 1);

    for (int i = 0; i < MAX_PROCESS; ++i)
        pcb[i] = NULL;

    pcb[0] = new PCB(0);
    bm->Mark(0);
}

PTable::~PTable()
{
    if (bm != NULL)
        delete bm;

    if (bmsem != NULL)
        delete bmsem;

    if (pcb[0] != NULL)
    {
        delete pcb[0];
        pcb[0] = NULL;
    }
}

int PTable::ExecUpdate(char *filename)
{
    // Prevent multiple programs execute at once
    bmsem->P();

    // Check if file exists
    OpenFile *executable = kernel->fileSystem->Open(filename);
    if (executable == NULL)
    {
        cerr << "Unable to open file " << filename << endl;
        delete executable;
        bmsem->V();
        return -1;
    }
    delete executable;

    // Check if call itself
    if (!strcmp(filename, kernel->currentThread->GetName()))
    {
        cerr << "Cannot execute itself" << endl;
        bmsem->V();
        return -1;
    }

    // Check if free slot is available
    int id = GetFreeSlot();
    if (id == -1)
    {
        cerr << "No free slot" << endl;
        bmsem->V();
        return -1;
    }

    // Create a new PCB
    pcb[id] = new PCB(id);
    // Execute and release the mutex
    int pID = pcb[id]->Exec(filename, id);
    bmsem->V();
    return pID;
}

int PTable::JoinUpdate(int pID)
{
    // Check if process is valid
    if (!IsExist(pID) || pcb[pID] == NULL)
    {
        cerr << "Process does not exist." << endl;
        return -1;
    }

    // Check if running process is parent
    if (kernel->currentThread->processID != pcb[pID]->parentID)
    {
        cerr << "Cannot join to process which is not its parent." << endl;
        return -1;
    }

    pcb[pcb[pID]->parentID]->IncNumWait();
    pcb[pID]->JoinWait();

    pcb[pID]->ExitRelease();
    int ec = pcb[pID]->GetExitCode();

    // Successfully
    return ec;
}

int PTable::ExitUpdate(int ec)
{
    // Check if exist
    int pID = kernel->currentThread->processID;
    if (!IsExist(pID) || pcb[pID] == NULL)
    {
        cerr << "Process does not exist." << endl;
        return -1;
    }

    // Halt if it is main process
    if (pID == 0)
    {
        Remove(pID);
        kernel->interrupt->Halt();
        return 0;
    }

    pcb[pID]->SetExitCode(ec);
    pcb[pID]->JoinRelease();

    pcb[pID]->ExitWait();
    pcb[pcb[pID]->parentID]->DecNumWait();

    Remove(pID);
    return ec;
}

int PTable::GetFreeSlot()
{
    return bm->FindAndSet();
}

bool PTable::IsExist(int pID)
{
    if (pID < 0 || pID >= psize)
        return false;
    return bm->Test(pID);
}

void PTable::Remove(int pID)
{
    if (IsExist(pID))
    {
        bm->Clear(pID);
        delete pcb[pID];
        pcb[pID] = NULL;
    }
}

char *PTable::GetFileName(int pID)
{
    if (IsExist(pID))
    {
        return pcb[pID]->GetFileName();
    }
    return NULL;
}

PCB *PTable::GetCurrentProcess()
{
    int pID = kernel->currentThread->processID;
    if (IsExist(pID))
    {
        return pcb[pID];
    }
    return NULL;
}