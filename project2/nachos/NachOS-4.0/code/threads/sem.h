
#ifndef SEM_H
#define SEM_H
#include "synch.h"
#define MAX_SEM_NAME 50

#endif // SEM_H

// Lop Sem dung de quan ly semaphore.
class Sem
{
private:
	char name[MAX_SEM_NAME];		// Ten cua semaphore
	Semaphore* sem;		// Tao semaphore de quan ly
public:
	// Khoi tao doi tuong Sem. Gan gia tri ban dau la null
	// Nho khoi tao sem su dung
	Sem(char* name, int i)
	{
		strcpy(this->name, name);
		sem = new Semaphore(this->name, i);
	}

	~Sem()
	{
		if(sem)
			delete sem;
	}

	void wait()
	{
		sem->P();	// Down(sem)
	}

	void signal()
	{
		sem->V();	// Up(sem)
	}
	
	char* GetName()
	{
		return this->name;
	}
};
