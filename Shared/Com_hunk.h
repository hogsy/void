#ifndef VOID_HUNKMEM_IMPLMENTATION
#define VOID_HUNKMEM_IMPLMENTATION

#include "I_hunkmem.h"

class CHunkMem : public I_HunkManager 
{
public:
	CHunkMem();
	virtual ~CHunkMem();

	void Init(uint size);
	void Shutdown();
	
	void *HunkAlloc(uint size);
	void HunkFree(void * mem);
	
	void PrintStats();
	bool Validate();

private:

	HANDLE m_hHeap;

	ulong m_numTotalAllocs;
	ulong m_numCurAllocs;
	ulong m_memAllocated;
};
	

#endif

