#ifndef VOID_COM_MEMORYMANAGER
#define VOID_COM_MEMORYMANAGER

#include "Com_defs.h"

/*/
======================================================================================
Every module includes this
a global object is created on the stack, and we use that to allocate
all dynamic memory in the module
======================================================================================
*/
class CMemManager 
{
public:

	CMemManager(const char * memfile);
	~CMemManager();

	void Init();
	void Shutdown(void);
	
	void * Malloc(uint size);
	void * Realloc(void *mem, uint size);
	void Free(void *mem);

#ifdef _DEBUG
	void * MallocDbg(uint size, const char * file, int line);
	void * ReallocDbg(void *mem, uint size, const char * file, int line);
	void FreeDbg(void *mem);
#endif
	
	void PrintStats();
	bool Validate();

private:

	ulong m_numAllocs;
	ulong m_curAllocs;
	ulong m_memAllocated;

#ifdef _DEBUG
	HFILE	* h_memfile;
#endif
};

//The global reference to the mem manager
extern CMemManager g_memManager;

//======================================================================================
//global Custom allocator funcs
//======================================================================================

//Debug mode
#ifdef _DEBUG

#define MALLOC(size)		g_memManager.MallocDbg(size,__FILE__,__LINE__)
#define REALLOC(p,size)		g_memManager.ReallocDbg(p,size,__FILE__,__LINE__)
#define FREE(p)				g_memManager.FreeDbg(p)

//standard new/delete
inline void* operator new(uint size)
{	return g_memManager.MallocDbg(size,__FILE__,__LINE__);
}
inline void  operator delete (void* ptr, const char * file, int line)
{	g_memManager.FreeDbg(ptr);	
}

//Placement new/delete
inline void* operator new(uint size, const char * file, int line)
{	return g_memManager.MallocDbg(size,file,line);
}

inline void  operator delete (void* ptr)
{	g_memManager.FreeDbg(ptr);	
}

#define DEBUG_NEW new(__FILE__,__LINE__)
#define new DEBUG_NEW

//Release mode
#else

#define MALLOC(size)		g_memManager.Malloc(size)
#define REALLOC(p,size)		g_memManager.Realloc(p, size)
#define FREE(p)				g_memManager(p)

inline void* operator new(uint size)
{	return g_memManager.Malloc(size);
}

inline void  operator delete(void* ptr)	 
{	g_memManager.Free(ptr);	
}

#endif

#endif