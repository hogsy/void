#include "Com_mem.h"

/*
==========================================
Constructor, creates log file for mem debug info
==========================================
*/

CMemManager::CMemManager(const char * memfile)
{
	m_numAllocs=0;
	m_curAllocs = 0;
	m_memAllocated=0;

#ifdef _DEBUG
	h_memfile = (HFILE*)::CreateFile(memfile,
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
#ifdef _DEBUG

	char msg[256];
	ulong written = 0;
	
	sprintf(msg,"Total Allocs : %d\nTotal Bytes Alloced : %d\nCurrent Allocs : %d\n", 
		m_numAllocs, m_memAllocated, m_curAllocs);
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

	m_numAllocs++;
	m_curAllocs++;
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
		m_numAllocs++;
		m_curAllocs++;
		m_memAllocated += size;
	}
	else 
	{
		if(mem)
		{
			free(mem);
			m_curAllocs --;
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
		m_curAllocs --;
	}
}
//======================================================================================
//Debug mode
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

	if (!_CrtIsValidHeapPointer(ptr))
	{
		char msg[256];
		ulong written = 0;
	
		sprintf(msg,"Invalid Heap Pointer : %s (%d)\n", file, line);
		WriteFile(h_memfile,msg,strlen(msg),&written,0);
	}

	m_numAllocs++;
	m_curAllocs  ++;
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
		m_numAllocs++;
		m_curAllocs++;
		m_memAllocated += size;
	}
	else 
	{
		if(mem)
		{
			_free_dbg(mem,_NORMAL_BLOCK);
			m_curAllocs --;
		}
		ptr = NULL;
	}
	return ptr;
}

void CMemManager::FreeDbg(void *mem)
{
	if(mem)
	{	
		if (!_CrtIsValidHeapPointer(mem))
		{
			int blah=0;
			blah ++;
		}
		_free_dbg(mem,_NORMAL_BLOCK);
		m_curAllocs --;
	}
}

#endif


/*
==========================================
Print current mem stats
==========================================
*/
void CMemManager::PrintStats()
{
}

/*
==========================================
run some vailidation on heap
==========================================
*/
bool CMemManager::Validate()
{	return true;
}