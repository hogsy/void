#ifndef VOID_MEMORY_MANAGER
#define VOID_MEMORY_MANAGER

#include "Com_defs.h"

//Class to manage big chunks of memory

#define COM_MAXHUNKS			256
#define COM_DEFAULTHUNKSIZE		(1024*1024*4)

class CMemManager
{
public:
	CMemManager(int defhunksize);
	~CMemManager();

	bool  Init();

	uint  CreateHunk(const char * name, uint size);	 //-1 if invalid
	void* AllocHunk(uint size, uint id);
	void  FreeHunk(uint id);
	void  ResetHunk(uint id);
	
	uint  GetHunkMaxSize(uint id);
	uint  GetHunkCurSize(uint id);
	void* GetHunkAddr(uint id);
	const char * GetHunkName(uint id);
	
	uint  GetHunkByName(const char * name);			//-1 if invalid
};

extern CMemManager * g_pMem;

#endif