#include "Com_defs.h"
#include "Com_mem.h"
#include <new.h>

/*
================================================
Utility Memorty Logging class
================================================
*/

class CMemLog
{
public:

	CMemLog(const char * memfile);
	~CMemLog();
	void PrintStats();

	ulong m_numAllocs;
	ulong m_curAllocs;
	ulong m_memAllocated;

#ifdef _DEBUG
	HFILE	* h_memfile;
#endif

};


/*
==========================================
Constructor, creates log file for mem debug info
==========================================
*/
CMemLog::CMemLog(const char * memfile)
{
	m_numAllocs=0;
	m_curAllocs = 0;
	m_memAllocated=0;

	_set_new_handler(HandleOutOfMemory);

	//make malloc use the c++ set_new_handler
	if(!_query_new_mode())
		_set_new_mode(1);

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
CMemLog::~CMemLog()
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

CMemLog & Mem_GetLog()
{
	static CMemLog g_memLog(MEM_SZLOGFILE);
	return g_memLog;
}



/*
==========================================
Malloc a block
==========================================
*/
void *	Mem_Malloc(uint size)
{
	if(size < 0)
	{
		ComPrintf("Mem_Malloc: Can't allocated less than 0 bytes\n");
		return 0;
	}

	void *ptr = ::malloc(size);
		
	if(ptr)
	{
		Mem_GetLog().m_numAllocs++;
		Mem_GetLog().m_curAllocs++;
		Mem_GetLog().m_memAllocated += size;
	}
	return ptr;
}

/*
==========================================
Re alloc the given block
==========================================
*/
void * Mem_Realloc(void *mem, uint size)
{
	if(mem)
	{
		if(size)
		{
			void * ptr = ::realloc(mem,size);
			Mem_GetLog().m_memAllocated += size;
			return ptr;
		}
		Mem_Free(mem);
	}
	else if(size)
		return Mem_Malloc(size);
	return 0;
}

/*
==========================================
Free the given block
==========================================
*/
void Mem_Free(void *mem )
{
	if(mem)
	{	
		::free(mem);
		Mem_GetLog().m_curAllocs --;
	}
}


//======================================================================================
//Debug mode
//======================================================================================

#ifdef _DEBUG

void * Mem_MallocDbg(uint size, const char * file, int line)
{
	if(size < 0)
	{
		ComPrintf("Mem_MallocDbg: Can't allocated less than 0 bytes\n");
		return 0;
	}

	void *ptr = ::_malloc_dbg(size,_NORMAL_BLOCK,file,line);
		
	if(ptr)
	{
		if (!_CrtIsValidHeapPointer(ptr))
		{
			char msg[256];
			ulong written = 0;
		
			sprintf(msg,"Invalid Heap Pointer : %s (%d)\n", file, line);
			WriteFile(Mem_GetLog().h_memfile,msg,strlen(msg),&written,0);
		}

		Mem_GetLog().m_numAllocs++;
		Mem_GetLog().m_curAllocs++;
		Mem_GetLog().m_memAllocated += size;
	}
	return ptr;
}

void * Mem_ReallocDbg(void *mem, uint size, const char * file, int line)
{
	if(mem)
	{
		if(size)
		{
			void * ptr = ::realloc(mem,size);
			Mem_GetLog().m_memAllocated += size;
			return ptr;
		}
		Mem_FreeDbg(mem);
	}
	else if(size)
		return Mem_MallocDbg(size,file,line);
	return 0;

/*
	void *ptr;

	if(mem) // && size) 
	{
		ptr = _realloc_dbg(mem,size,_NORMAL_BLOCK,file,line);
		m_memAllocated += size;
	}
	else  if(size) 
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
*/
}

void Mem_FreeDbg(void *mem)
{
	if(mem)
	{	
		::_free_dbg(mem,_NORMAL_BLOCK);
		Mem_GetLog().m_curAllocs --;
	}
}

#endif








































#if 0

/*
==========================================
Constructor, creates log file for mem debug info
==========================================
*/

CMemManager::CMemManager(const char * memfile)
{
	if(MEM_SZLOGFILE)
	{
		strlen(MEM_SZLOGFILE);
	}

	m_numAllocs=0;
	m_curAllocs = 0;
	m_memAllocated=0;

	_set_new_handler(HandleOutOfMemory);

	//make malloc use the c++ set_new_handler
	if(!_query_new_mode())
		_set_new_mode(1);

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
	if(size < 0)
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

	if(mem) // && size) 
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
	if(size < 0)
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

	if(mem) // && size) 
	{
		ptr = _realloc_dbg(mem,size,_NORMAL_BLOCK,file,line);
		m_memAllocated += size;
	}
	else  if(size) 
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

#endif