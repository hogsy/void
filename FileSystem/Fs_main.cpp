#include "Fs_hdr.h"
#include "I_void.h"
#include "I_filesystem.h"

/*
==========================================
Local vars
==========================================
*/
const char MEM_SZLOGFILE [] = "mem_fs.log";
//CMemManager		g_memManager("mem_fs.log");

I_HunkManager * g_pHunkManager = 0;
I_Console     * g_pConsole = 0;

static I_Void * g_pVoid = 0;

CFileSystem	  *	g_pFileSystem = 0;

/*
==========================================
Create the fileSystem and return it
copy pointer to console for cvar/printing functions
==========================================
*/
FILESYSTEM_API CFileSystem * FILESYSTEM_Create(I_Void * vexp, 
											   const char * exeDir, 
											   const char * baseDir)
{
	g_pVoid = vexp;
	g_pHunkManager = vexp->hunkManager;
	g_pConsole    = vexp->console;
	
	if(!g_pFileSystem)
		g_pFileSystem = new CFileSystem(exeDir,baseDir);
	return g_pFileSystem;
}

/*
==========================================
Destroy the fileSystem
==========================================
*/
FILESYSTEM_API void FILESYSTEM_Free()
{
	if(g_pFileSystem)
	{
		delete g_pFileSystem;
		g_pFileSystem = 0;
	}
	g_pVoid = 0;
	g_pConsole = 0;
	g_pHunkManager = 0;
}

/*
==========================================
Common print function for the module
==========================================
*/
void ComPrintf(const char* text, ...)
{
	static char buff[1024];

	va_list args;
	va_start(args, text);
	vsprintf(buff, text, args);
	va_end(args);

	g_pConsole->ComPrint(buff);
}

/*
======================================
Com mem hancler
======================================
*/
int HandleOutOfMemory(size_t size)
{	
	g_pVoid->SystemError("FileSystem: Out of Memory");
	return 0;
}


//======================================================================================
//======================================================================================
