#ifndef VOID_SYS_MEMORYMANAGER
#define VOID_SYS_MEMORYMANAGER

//======================================================================================
//======================================================================================

#include "Com_defs.h"
#include "I_mem.h"


class CMemManager : public I_MemManager 
{
public:

	CMemManager();
	~CMemManager();

	void Init(void);
	void Shutdown(void);
	
	void * Malloc(uint size);
	void * Realloc(void *mem, uint size);
	void Free(void *mem);

#ifdef _DEBUG
	void * MallocDbg(uint size, const char * file, int line);
	void * ReallocDbg(void *mem, uint size, const char * file, int line);
	void FreeDbg(void *mem);
#endif
	
	void * HeapAlloc(uint size);
	void HeapFree(void * mem);

	void PrintDebug();
	void ValidateHeap();

private:

	ulong m_numAllocations;
	ulong m_numAllocated;
	ulong m_memAllocated;

	ulong m_numHeapAllocs;
	ulong m_numHeapAllocated;
	ulong m_memHeapAllocated;

	HANDLE m_hHeap;

#ifdef _DEBUG
	HFILE	* h_memfile;
#endif
};

#endif