#include "Com_defs.h"
#include "Com_hunk.h"

I_HunkManager * g_pHunkManager=0;

/*
==========================================
Constructor/Dest
==========================================
*/
CHunkMem::CHunkMem()
{
	g_pHunkManager = this;

	m_numTotalAllocs=0;
	m_numCurAllocs=0;
	m_memAllocated=0;

	m_hHeap = ::GetProcessHeap();
}

CHunkMem::~CHunkMem()
{
	g_pHunkManager= 0;
}

/*
==========================================
Initialize to given size etc later on
==========================================
*/
void CHunkMem::Init(uint size){}
void CHunkMem::Shutdown(){}
	
/*
==========================================
Alloc a big chunk from heap memory
==========================================
*/
void * CHunkMem::HunkAlloc(uint size)
{
	if(size <=0)
	{	
		ComPrintf("CMemManager::HeapAlloc: Bad size\n");
		return 0;
	}

	void * ptr = ::HeapAlloc(m_hHeap,
							 HEAP_GENERATE_EXCEPTIONS,
							 size);
	if(!ptr)
	{
		ComPrintf("CMemManager::HeapAlloc: Unable to alloc %d bytes\n", size);
		return 0;
	}
	m_numTotalAllocs ++;
	m_numCurAllocs ++;
	m_memAllocated +=size;
	return ptr;
}

/*
==========================================
Free the given chunk
==========================================
*/
void CHunkMem::HunkFree(void * mem)
{
	if(mem)
	{
		::HeapFree(m_hHeap,0, mem);
		m_numCurAllocs --;
	}
}

/*
==========================================
Misc Util
==========================================
*/
void CHunkMem::PrintStats()
{
	ComPrintf("Total Hunk Allocs : %d\nTotal Hunk Bytes Alloced : %d\nCurrent Hunk Allocs : %d\n", 
				m_numTotalAllocs, m_memAllocated, m_numCurAllocs);
}

bool CHunkMem::Validate()
{	return true;
}

