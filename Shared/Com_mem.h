#ifndef VOID_COM_MEMORYMANAGER
#define VOID_COM_MEMORYMANAGER

#include <crtdbg.h>

/*
======================================
Must be defined in module to handle
out of memory conditions
======================================
*/
extern const char MEM_SZLOGFILE[];
extern int HandleOutOfMemory(size_t size);


/*
================================================
Memory Funcs
================================================
*/
void *	Mem_Malloc(uint size);
void *	Mem_Realloc(void *mem, uint size);
void	Mem_Free(void *mem);

#ifdef _DEBUG

void *	Mem_MallocDbg(uint size, const char * file, int line);
void *	Mem_ReallocDbg(void *mem, uint size, const char * file, int line);
void	Mem_FreeDbg(void *mem);

#endif

/*
================================================
Definitions to override regular funcs
================================================
*/
//Debug mode
#ifdef _DEBUG

#define MALLOC(size)		Mem_MallocDbg(size,__FILE__,__LINE__)
#define REALLOC(p,size)		Mem_ReallocDbg(p,size,__FILE__,__LINE__)
#define FREE(p)				Mem_FreeDbg(p)

//default new/delete
inline void* __cdecl operator new(uint size)
{	return Mem_MallocDbg(size,__FILE__,__LINE__);
}
inline void  __cdecl operator delete (void* ptr)
{	Mem_FreeDbg(ptr);
}

//Custom new/delete
inline void* __cdecl operator new(uint size, const char * file, int line)
{	return Mem_MallocDbg(size,file,line);
}

inline void  __cdecl operator delete (void* ptr, const char * file, int line)
{	Mem_FreeDbg(ptr);	
}

//Release mode
#else

#define MALLOC(size)		Mem_Malloc(size)
#define REALLOC(p,size)		Mem_Realloc(p, size)
#define FREE(p)				Mem_Free(p)

inline void* __cdecl operator new(uint size)
{	return Mem_Malloc(size);
}

inline void  __cdecl operator delete(void* ptr)	 
{	Mem_Free(ptr);	
}
#endif

/*
================================================
Override new to use 3 parms in debug mode
================================================
*/
#ifdef _DEBUG
	#define VOID_NEW new(__FILE__,__LINE__)
#else
	#define VOID_NEW new
#endif

#define new	VOID_NEW

#endif