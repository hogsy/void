#include "Com_mem.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

/*
==========================================
Constructor, creates log file for mem debug info
==========================================
*/
CMemManager::CMemManager()
{
	//Set the global memory manager to self
	g_pMemManager = this;

	m_numAllocations=0;
	m_numAllocated = 0;
	m_memAllocated=0;

	m_numHeapAllocs=0;
	m_numHeapAllocated=0;
	m_memHeapAllocated=0;

	m_hHeap = ::GetProcessHeap();

#ifdef _DEBUG
	h_memfile = (HFILE*)::CreateFile("mem.log",
					GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_WARN, h_memfile);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ERROR, h_memfile);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ASSERT, h_memfile);
#endif

}

/*
==========================================
Closes log file
==========================================
*/
CMemManager::~CMemManager()
{
	//Set the global memory manager to 0
	g_pMemManager = 0;
#ifdef _DEBUG

	char msg[256];
	ulong written = 0;
	
	sprintf(msg,"Total Allocs : %d\nTotal Bytes Alloced : %d\nCurrent Allocs : %d\n", 
		m_numAllocations, m_memAllocated, m_numAllocated);
	WriteFile(h_memfile,msg,strlen(msg),&written,0);
	
	sprintf(msg,"Total Heap Allocs : %d\nTotal Heap Bytes Alloced : %d\nCurrent Heap Allocs : %d\n", 
		m_numHeapAllocs, m_memHeapAllocated, m_numHeapAllocated);
	WriteFile(h_memfile,msg,strlen(msg),&written,0);

   _CrtDumpMemoryLeaks();
   	CloseHandle(h_memfile);
#endif
}


/*
==========================================
Init/Shutdown ?
==========================================
*/
void CMemManager::Init() {}
void CMemManager::Shutdown() {} 


/*
==========================================
Heap Alloc/Free funcs to alloc big chunks
==========================================
*/
void * CMemManager::HeapAlloc(uint size)
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
	m_numHeapAllocs ++;
	m_numHeapAllocated ++;
	m_memHeapAllocated +=size;
	return ptr;
}

void CMemManager::HeapFree(void * mem)
{
	if(mem)
	{
		::HeapFree(m_hHeap,0, mem);
		m_numHeapAllocated --;
	}
}

/*
==========================================
Malloc a block
==========================================
*/
void * CMemManager::Malloc(uint size)
{
	if(size <=0)
	{
		ComPrintf("CMemManger::Malloc: bad count\n");
		return 0;
	}

	void *ptr = malloc(size);
		
	if(!ptr)
	{
		ComPrintf("CMemManager::Malloc: Error allocating %d bytes\n",size);
		return 0;
	}

	m_numAllocations++;
	m_numAllocated  ++;
	m_memAllocated += size;
	return ptr;
}

/*
==========================================
Re alloc the given block
==========================================
*/
void * CMemManager::Realloc(void *mem, uint size)
{
	void *ptr;

	if(mem && size) 
	{
		ptr = realloc(mem,size);
		m_memAllocated += size;
	}
	else if(size) 
	{
		ptr = malloc(size);
		m_numAllocations++;
		m_numAllocated++;
		m_memAllocated += size;
	}
	else 
	{
		if(mem)
		{
			free(mem);
			m_numAllocated --;
		}
		ptr = NULL;
	}
	return ptr;
}

/*
==========================================
Free the given block
==========================================
*/
void CMemManager::Free(void *mem )
{
	if(mem)
	{	free(mem);
		m_numAllocated --;
	}
}

/*
==========================================
Print current mem stats
==========================================
*/
void CMemManager::PrintDebug()
{
}

/*
==========================================
run some vailidation on heap
==========================================
*/
void CMemManager::ValidateHeap()
{
}

//======================================================================================
//Debug Allocation funcs
//======================================================================================

#ifdef _DEBUG

void * CMemManager::MallocDbg(uint size, const char * file, int line)
{
	if(size <=0)
	{
		ComPrintf("CMemManger::Malloc: bad size\n");
		return 0;
	}

	void *ptr = _malloc_dbg(size,_NORMAL_BLOCK,file,line);
		
	if(!ptr)
	{
		ComPrintf("CMemManager::Malloc: Error allocating memory\n");
		return 0;
	}

	m_numAllocations++;
	m_numAllocated  ++;
	m_memAllocated += size;
	return ptr;
}

void * CMemManager::ReallocDbg(void *mem, uint size, const char * file, int line)
{
	void *ptr;

	if(mem && size) 
	{
		ptr = _realloc_dbg(mem,size,_NORMAL_BLOCK,file,line);
		m_memAllocated += size;
	}
	else if(size) 
	{
		ptr = _malloc_dbg(size,_NORMAL_BLOCK,file,line);
		m_numAllocations++;
		m_numAllocated++;
		m_memAllocated += size;
	}
	else 
	{
		if(mem)
		{
			_free_dbg(mem,_NORMAL_BLOCK);
			m_numAllocated --;
		}
		ptr = NULL;
	}
	return ptr;
}

void CMemManager::FreeDbg(void *mem)
{
	if(mem)
	{	_free_dbg(mem,_NORMAL_BLOCK);
		m_numAllocated --;
	}
}

#endif