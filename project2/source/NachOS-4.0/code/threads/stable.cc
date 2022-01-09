#include "stable.h"

// Constructor
STable::STable() {	
	this->bm = new Bitmap(MAX_SEMAPHORE);
	
	for (int i =0; i < MAX_SEMAPHORE; i++) {
		this->semTab[i] = NULL;
	}
}

// Destructor
STable::~STable() {
	if (this->bm) {
		delete this->bm;
		this->bm = NULL;
	}

	for (int i=0; i < MAX_SEMAPHORE; i++) {
		if (this->semTab[i]) {
			delete this->semTab[i];
			this->semTab[i] = NULL;
		}
	}
}

int STable::Create(char *name, int semVal) {
	// check if semaphore is exists
	for (int i = 0; i < MAX_SEMAPHORE; ++i) {
		if (bm->Test(i) && strcmp(name, semTab[i]->GetName()) == 0) {
			return -1;
		}
	}

	// find free slot to create new semaphore
	int id = this->FindFreeSlot();
	
	// if free slot exist then create semaphore, otherwise return -1
	if (id < 0) {
		return -1;
	}
	this->semTab[id] = new Sem(name, semVal);
	return 0;
}

int STable::Wait(char *name) {
	// find semaphore with given name and down it
	for (int i = 0; i < MAX_SEMAPHORE; i++) {
		if (bm->Test(i) && strcmp(name, semTab[i]->GetName()) == 0) {
			semTab[i]->wait();
			return 0;
		}
	}
	DEBUG(dbgSynch, "Does not exists semaphore with given name " << name);
	return -1;
}

int STable::Signal(char *name) {
	// find semaphore with given name and up it
	for (int i = 0; i < MAX_SEMAPHORE; i++) {
		if (bm->Test(i) && strcmp(name, semTab[i]->GetName()) == 0)
		{
			semTab[i]->signal();
			return 0;
		}
	}
	DEBUG(dbgSynch, "Does not exists semaphore with given name " << name);
	return -1;
}

int STable::FindFreeSlot() {
	return this->bm->FindAndSet();
}
