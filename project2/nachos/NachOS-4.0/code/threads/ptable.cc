#include "ptable.h"
#include "openfile.h"
#include "main.h"

PTable::PTable(int size) {
    if (size < 0)
        return;
    psize = size;
    bm = new Bitmap(size);
    bmsem = new Semaphore("bmsem",1);

    for (int i = 0; i < MAX_PROCESS; ++i) {
		pcb[i] = 0;
    }

	bm->Mark(0);
	pcb[0] = new PCB(0);
	pcb[0]->parentID = -1;
}

PTable::~PTable() {
    if (bm != 0 )
	delete bm;
    
	if (pcb[0] != 0)
		delete pcb[0];
		
	if (bmsem != 0)
		delete bmsem;
}

int PTable::ExecUpdate(char* filename) {
    // down semaphore for avoid load 2 program at the same time
	bmsem->P();
	
	// check if filename is null
	if (filename == NULL) {
		DEBUG(dbgThread, "Can't not execute because filename is NULL.");
		bmsem->V();
		return -1;
	}

	// ensure the program does not exec itself, which may cause infinity recursion
	if (strcmp(filename, kernel->currentThread->getName()) == 0 )
	{
		DEBUG(dbgThread, "PTable::Exec : Process cannot not execute itself.");	
		bmsem->V();
		return -1;
	}

	// find free slot in process table
	int index = this->GetFreeSlot();

    // Check if have free slot
	if (index < 0)
	{
		printf("There is no free slot.");
		bmsem->V();
		return -1;
	}

	// create new process with correspond index
	pcb[index] = new PCB(index);
	// init infomation of new process then execute
	pcb[index]->SetFileName(filename);
    pcb[index]->parentID = kernel->currentThread->processID;
	int pid = pcb[index]->Exec(filename, index);

	// up semaphore to return resource
	bmsem->V();
	return pid;
}

int PTable::JoinUpdate(int id) {
	// check if id is valid
	if (!IsExist(id)) {
		DEBUG(dbgThread, "Can't join in process with id = " << id);
		return -1;
	}

	// check if running process is parent of join process
	if (kernel->currentThread->processID != pcb[id]->parentID) {
		DEBUG(dbgThread, "Cannot join process which is not parent process of given process.");
		return -1;
	}

    // increase number of child process that parent need to wait 
	// and down join semaphore to wait for child process
	pcb[pcb[id]->parentID]->IncNumWait();
	pcb[id]->JoinWait();

	// get exit code from child process
	int ec = pcb[id]->GetExitCode();
    // up exit semaphore to allow child process to exit
	pcb[id]->ExitRelease();

	return ec;
}


int PTable::ExitUpdate(int exitcode) {              
    // get id of process from current running thread
	int id = kernel->currentThread->processID;

	// if main process is exiting then halt the system
	if (id == 0) {
		kernel->interrupt->Halt();
		return exitcode;
	}
    
	// check if process is exists
    if (!IsExist(id)) {
		DEBUG(dbgThread, "The process with id = " << id << " not exists.");
		return -1;
	}

	// set exit code for given process
	pcb[id]->SetExitCode(exitcode);
	pcb[pcb[id]->parentID]->DecNumWait();
    
    // up join semaphore to release parent process
	pcb[id]->JoinRelease();
    // down exit semaphore to wait parent process allow current process to exit
	pcb[id]->ExitWait();
	
	this->Remove(id);
	return exitcode;
}

// find free slot in order to save new process infomation
int PTable::GetFreeSlot(){ return bm->FindAndSet();}

// check if process id is exists
bool PTable::IsExist(int pid) { 
	if (pid < 0 || pid >= MAX_PROCESS) {
		return FALSE;
	}
	return bm->Test(pid);
}

// remove process from ptable
void PTable::Remove(int pid) {
	bm->Clear(pid);

	if (pcb[pid] != 0) {
		delete pcb[pid];
		pcb[pid] = NULL;
	}
}

char* PTable::GetFileName(int id) {
	return (pcb[id]->GetFileName());
}

OpenFileID PTable::OpenFile(int pid, char* name, int type) {
	return this->pcb[pid]->OpenFile(name, type);
}

int PTable::CloseFile(int pid, OpenFileID fid) {
	return this->pcb[pid]->CloseFile(fid);
}

int PTable::ReadFile(int pid, char *buffer, int charcount, OpenFileID fid) {
	return this->pcb[pid]->ReadFile(buffer, charcount, fid); 
}

int PTable::WriteFile(int pid, char *buffer, int charcount, OpenFileID fid) {
	return this->pcb[pid]->WriteFile(buffer, charcount, fid);
}

int PTable::AppendFile(int pid, char *buffer, int charcount, OpenFileID fid) {
	return this->pcb[pid]->AppendFile(buffer, charcount, fid);
}