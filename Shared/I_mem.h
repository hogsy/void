#ifndef INC_MEMORY_INTERFACE
#define INC_MEMORY_INTERFACE

/*
==========================================
The Memory Manager Interface
==========================================
*/

interface I_MemManager 
{
	virtual void Init(void)=0;
	virtual void Shutdown(void)=0;
	
	virtual void * Malloc(uint size)=0;
	virtual void * Realloc(void *mem, uint size)=0;
	virtual void Free(void *mem )=0;

#ifdef _DEBUG
	virtual void * MallocDbg(uint size, const char * file, int line)=0;
	virtual void * ReallocDbg(void *mem, uint size, const char * file, int line)=0;
	virtual void FreeDbg(void *mem)=0;
#endif

	virtual void * HeapAlloc(uint size)=0;
	virtual void HeapFree(void * mem)=0;
	
	virtual void PrintDebug()=0;
	virtual void ValidateHeap()=0;
};

//======================================================================================
//pointer to the global memory manager
//======================================================================================
extern I_MemManager * g_pMemManager;

//======================================================================================
//global Custom allocator funcs
//======================================================================================

#ifdef _DEBUG

#define MALLOC(size)		g_pMemManager->MallocDbg(size,__FILE__,__LINE__)
#define REMALLOC(p,size)	g_pMemManager->ReallocDbg(p,size,__FILE__,__LINE__)
#define FREE(p)				g_pMemManager->FreeDbg(p)

#else

#define MALLOC(size)		g_pMemManager->Malloc(size)
#define REALLOC(p,size)		g_pMemManager->Realloc(p, size)
#define FREE(p)				g_pMemManager->Free(p)

#endif


inline void* operator new(uint size)
{	return g_pMemManager->Malloc(size);
}

inline void* operator new[]( uint size )
{	return g_pMemManager->Malloc(size);
}

inline void operator delete(void* ptr)
{	g_pMemManager->Free( ptr);
}

inline void operator delete [](void* ptr)
{	g_pMemManager->Free(ptr);
}


#endif