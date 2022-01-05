#ifndef SEM_H
#define SEM_H
#include "synch.h"
#define MAX_SEM_NAME 50

// Sem class as a wraper for Semaphore class
class Sem {
private:
	char name[MAX_SEM_NAME]; // semaphore name
	Semaphore* sem;		// core semaphore
public:
	// constructor sem object
	Sem(char* name, int i) {
		strcpy(this->name, name);
		sem = new Semaphore(this->name, i);
	}

	~Sem() {
		if(sem) delete sem;
	}

	// down(sem)
	void wait() { sem->P();}

	// up(sem)
	void signal() { sem->V();}
	
	char* GetName() { return this->name;}
};

#endif // SEM_H