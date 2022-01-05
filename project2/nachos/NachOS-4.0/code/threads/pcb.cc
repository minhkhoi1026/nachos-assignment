// pcb.cc, "Process Control Block"
// All rights reserved.

#include "pcb.h"
#include "main.h"

void StartProcess(void* pid) {
	int id = *((int*) pid);
    char* filename = kernel->processTable->GetFileName(id);

    AddrSpace *space = new AddrSpace();
	ASSERT(space != (AddrSpace *)NULL);
	if (space->Load(filename)) {  // load the program into the space
		space->Execute(); // run the program
		ASSERTNOTREACHED(); // Execute never returns
	}

	DEBUG(dbgThread, "Cannot allocate addrspace for program!");
}

PCB::PCB(int id) {
	this->parentID = (id == 0) ? -1 : kernel->currentThread->processID;

	this->numwait = this->exitcode = this->boolBG = 0;
	this->thread = NULL;

	this->joinsem = new Semaphore("joinsem",0);
	this->exitsem = new Semaphore("exitsem",0);
	this->multex = new Semaphore("multex",1);
	this->ftable = new FileTable();
}

PCB::~PCB() {
	if (joinsem != NULL)
		delete this->joinsem;
	if (exitsem != NULL)
		delete this->exitsem;
	if (multex != NULL)
		delete this->multex;
	if (ftable != NULL)
		delete this->ftable;

	if (thread != NULL) {		
		thread->FreeSpace();
		thread->Finish();
	}
}
int PCB::GetID(){ return this->thread->processID; }
int PCB::GetNumWait() { return this->numwait; }
int PCB::GetExitCode() { return this->exitcode; }

void PCB::SetExitCode(int ec){ this->exitcode = ec; }

void PCB::JoinWait(){ joinsem->P(); }

void PCB::JoinRelease() { joinsem->V(); }

void PCB::ExitWait(){ exitsem->P(); }

void PCB::ExitRelease() { exitsem->V(); }

void PCB::IncNumWait() {
	multex->P();
	++numwait;
	multex->V();
}

void PCB::DecNumWait() {
	multex->P();
	if (numwait > 0)
		--numwait;
	multex->V();
}

void PCB::SetFileName(char* fn) { 
	strcpy(FileName, fn);
}

char* PCB::GetFileName() { return this->FileName; }

int PCB::Exec(char* filename, int id) {  
    // down multex to avoid exec 2 program at the same time
	multex->P();
          
	this->thread = new Thread(filename);

	// check if thread is allocated
	if (this->thread == NULL) {
		printf("\nPCB::Exec:: Not enough memory..!\n");
		multex->V();
		return -1;
	}

	// init thread info and call fork to put process into schedule queue
	this->thread->processID = id;
	this->parentID = kernel->currentThread->processID;
 	this->thread->Fork((VoidFunctionPtr) StartProcess, (void*) &(this->thread->processID));

	// up mutex to return resource
    multex->V();
	return id;
}

OpenFileID PCB::OpenFile(char* name, int type) {
	return this->ftable->Open(name,type);
}

int PCB::CloseFile(OpenFileID fid) {
	return this->ftable->Close(fid);
}

int PCB::ReadFile(char *buffer, int charcount, OpenFileID id) {
	return this->ftable->Read(buffer,charcount,id);
}

int PCB::WriteFile(char *buffer, int charcount, OpenFileID id) {
	return this->ftable->Write(buffer,charcount,id);
}

int PCB::AppendFile(char *buffer, int charcount, OpenFileID id) {
	return this->ftable->Append(buffer,charcount,id);
}