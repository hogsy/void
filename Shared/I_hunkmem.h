#ifndef INC_HUNK_INTERFACE
#define INC_HUNK_INTERFACE

#include "Com_defs.h"

/*
==========================================
The Hunk Manager Interface

The hunk is only maintained by the exe
it passes the interface to other modules
==========================================
*/
struct I_HunkManager 
{
	virtual void Init(uint size)=0;
	virtual void Shutdown()=0;
	
	//Parms will probably change here. 
	//add support for zones and whatnot
	virtual void *HunkAlloc(uint size)=0;
	virtual void HunkFree(void * mem)=0;
	
	virtual void PrintStats()=0;
	virtual bool Validate()=0;
};

//pointer to the global hunk memory manager
//must be defined and set somewhere

extern I_HunkManager * g_pHunkManager;

#endif