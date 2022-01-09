#ifndef STABLE_H
#define STABLE_H

#include "bitmap.h"
#include "sem.h"

#define MAX_SEMAPHORE 10

class STable
{
private:
	Bitmap* bm;	// bitmap array for marking used slot
	Sem* semTab[MAX_SEMAPHORE];	// core semaphore array
public:
	STable();		
	~STable();

	int Create(char *name, int init);
	int Wait(char *name);
	int Signal(char *name);
	int FindFreeSlot();
};

#endif // STABLE_H