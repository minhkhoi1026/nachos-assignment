// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -n -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you are using the "stub" file system, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "machine.h"
#include "noff.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
#ifdef RDATA
    noffH->readonlyData.size = WordToHost(noffH->readonlyData.size);
    noffH->readonlyData.virtualAddr = 
           WordToHost(noffH->readonlyData.virtualAddr);
    noffH->readonlyData.inFileAddr = 
           WordToHost(noffH->readonlyData.inFileAddr);
#endif 
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);

#ifdef RDATA
    DEBUG(dbgAddr, "code = " << noffH->code.size <<  
                   " readonly = " << noffH->readonlyData.size <<
                   " init = " << noffH->initData.size <<
                   " uninit = " << noffH->uninitData.size << "\n");
#endif
}

// constructor for addrspace, not init anything (Load will do that)
AddrSpace::AddrSpace(){}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
}


//----------------------------------------------------------------------
// AddrSpace::Load
// 	Load a user program into memory from a file.
//
//	Assumes that the page table has been initialized, and that
//	the object code file is in NOFF format.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

bool 
AddrSpace::Load(char *filename) {
    NoffHeader noffH;
    unsigned int i, size;
    OpenFile *executable = kernel->fileSystem->Open(filename);
 
    if (executable == NULL) {
        DEBUG(dbgFile, "\n Error opening file: " << filename);
        return FALSE;
    }
 
    // read file header
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);
 
    kernel->addrLock->P();

    // how many page the user program need?
#ifdef RDATA
    size = noffH.code.size + noffH.readonlyData.size + noffH.initData.size +
           noffH.uninitData.size + UserStackSize;	
#else
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;
#endif
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
 
    DEBUG(dbgAddr, "\nSize: " << size << 
                " | numPages: " << numPages << 
                " | PageSize: " << PageSize << 
                " | Numclear: " << kernel->gPhysPageBitMap->NumClear() << "\n");  
    // Check the available memory enough to load new process
    if ((int)numPages > kernel->gPhysPageBitMap->NumClear()) {
        DEBUG(dbgAddr, "Not enough memory for new process");
        numPages = 0;
        delete executable;
        kernel->addrLock->V();
        return FALSE;
    }
 
    // set up the translation page
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; ++i) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = kernel->gPhysPageBitMap->FindAndSet();
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;
        bzero(
            &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                + pageTable[i].physicalPage * PageSize, 
            PageSize);
    }

    kernel->addrLock->V();
 
    // copy code segment into memory
    int numCodePages = divRoundUp(noffH.code.size, PageSize);
    int lastCodePageSize = (noffH.code.size - 1) % PageSize + 1;

    i = 0;
    if (noffH.code.size > 0) {
        for (int j = 0; j < numCodePages; ++j, ++i) {
            executable->ReadAt(
                &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                    + pageTable[j].physicalPage*PageSize,
                (j < numCodePages - 1) ? PageSize : lastCodePageSize,
                noffH.code.inFileAddr + j*PageSize
            );
        }
    }
 
#ifdef RDATA
    // copy read-only data segment to main
    int firstReadonlyPageSize = min(PageSize - lastCodePageSize, noffH.readonlyData.size);
    int numReadonlyPages = divRoundUp(noffH.readonlyData.size - firstReadonlyPageSize, PageSize);
    int lastReadonlyPageSize = (noffH.readonlyData.size - firstReadonlyPageSize - 1) % PageSize + 1;

    if (noffH.readonlyData.size > 0) {
        // copy first part of readonly data into remain space of last code page
        if (lastCodePageSize != PageSize && numCodePages > 0) {
            executable->ReadAt(
                &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                    + pageTable[i - 1].physicalPage * PageSize + lastCodePageSize,
                firstReadonlyPageSize,
                noffH.readonlyData.inFileAddr
            );
        }

        // copy remain readonly data into main memory
        for (int j = 0; j < numReadonlyPages; ++j, ++i) {
            executable->ReadAt(
                &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                    + pageTable[i].physicalPage * PageSize,
                (j < numReadonlyPages - 1) ? PageSize : lastReadonlyPageSize,
                noffH.readonlyData.inFileAddr + firstReadonlyPageSize + j*PageSize
            );
        }
    }

    // copy init data segment into memory
    int firstDataPageSize = min(PageSize - lastReadonlyPageSize, noffH.initData.size);
    int numDataPages = divRoundUp(noffH.initData.size - firstDataPageSize, PageSize);
    int lastDataPageSize = (noffH.initData.size - firstDataPageSize - 1) % PageSize + 1;

    if (noffH.initData.size > 0) {
        // copy first part of initial data into remain space of last readonly page
        if (lastReadonlyPageSize != PageSize && numReadonlyPages > 0) {
            executable->ReadAt(
                &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                    + pageTable[i - 1].physicalPage*PageSize + lastReadonlyPageSize,
                firstDataPageSize,
                noffH.initData.inFileAddr
            );
        }

        // copy remain initial data into main memory
        for (int j = 0; j < numDataPages; ++j, ++i) {
            executable->ReadAt(
                &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                    + pageTable[i].physicalPage*PageSize,
                (j < numDataPages - 1) ? PageSize : lastDataPageSize,
                noffH.initData.inFileAddr + firstDataPageSize + j*PageSize
            );
        }
    }
#else // not exitst RDATA
    // copy init data segment into memory
    int firstDataPageSize = min(PageSize - lastCodePageSize, noffH.initData.size);
    int numDataPages = divRoundUp(noffH.initData.size - firstDataPageSize, PageSize);
    int lastDataPageSize = (noffH.initData.size - firstDataPageSize - 1) % PageSize + 1;

    if (noffH.initData.size > 0) {
        // copy first part of initial data into remain space of last code page
        if (lastCodePageSize != PageSize && numCodePages > 0) {
            executable->ReadAt(
                &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                    + pageTable[i - 1].physicalPage*PageSize + lastCodePageSize,
                firstDataPageSize,
                noffH.initData.inFileAddr
            );
        }

        // copy remain initial data into main memory
        for (int j = 0; j < numDataPages; ++j, ++i) {
            executable->ReadAt(
                &(kernel->machine->mainMemory[noffH.code.virtualAddr]) 
                    + pageTable[i].physicalPage*PageSize,
                (j < numDataPages - 1) ? PageSize : lastDataPageSize,
                noffH.initData.inFileAddr + firstDataPageSize + j*PageSize
            );
        }
    }
#endif

    delete executable;
    return TRUE;
}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program using the current thread
//
//      The program is assumed to have already been loaded into
//      the address space
//
//----------------------------------------------------------------------

void 
AddrSpace::Execute() 
{

    kernel->currentThread->space = this;

    this->InitRegisters();		// set the initial register values
    this->RestoreState();		// load page table register

    kernel->machine->Run();		// jump to the user progam

    ASSERTNOTREACHED();			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    Machine *machine = kernel->machine;
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start", which
    //  is assumed to be virtual address zero
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    // Since instructions occupy four bytes each, the next instruction
    // after start will be at virtual address four.
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG(dbgAddr, "Initializing stack pointer: " << numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
    pageTable = kernel->machine->pageTable ;
    numPages = kernel->machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    kernel->machine->pageTable = pageTable;
    kernel->machine->pageTableSize = numPages;
}


//----------------------------------------------------------------------
// AddrSpace::Translate
//  Translate the virtual address in _vaddr_ to a physical address
//  and store the physical address in _paddr_.
//  The flag _isReadWrite_ is false (0) for read-only access; true (1)
//  for read-write access.
//  Return any exceptions caused by the address translation.
//----------------------------------------------------------------------
ExceptionType
AddrSpace::Translate(unsigned int vaddr, unsigned int *paddr, int isReadWrite)
{
    TranslationEntry *pte;
    int               pfn;
    unsigned int      vpn    = vaddr / PageSize;
    unsigned int      offset = vaddr % PageSize;

    if(vpn >= numPages) {
        return AddressErrorException;
    }

    pte = &pageTable[vpn];

    if(isReadWrite && pte->readOnly) {
        return ReadOnlyException;
    }

    pfn = pte->physicalPage;

    // if the pageFrame is too big, there is something really wrong!
    // An invalid translation was loaded into the page table or TLB.
    if (pfn >= NumPhysPages) {
        DEBUG(dbgAddr, "Illegal physical page " << pfn);
        return BusErrorException;
    }

    pte->use = TRUE;          // set the use, dirty bits

    if(isReadWrite)
        pte->dirty = TRUE;

    *paddr = pfn*PageSize + offset;

    ASSERT((*paddr < MemorySize));

    //cerr << " -- AddrSpace::Translate(): vaddr: " << vaddr <<
    //  ", paddr: " << *paddr << "\n";

    return NoException;
}




