#include "Fs_hdr.h"
#include "I_filesystem.h"

/*
==========================================
Local vars
==========================================
*/
CMemManager		g_memManager("mem_fs.log");

I_HunkManager * g_pHunkManager = 0;
I_Console     * g_pConsole = 0;
CFileSystem	  *	g_pFileSystem = 0;

/*
==========================================
Create the fileSystem and return it
copy pointer to console for cvar/printing functions
==========================================
*/
FILESYSTEM_API CFileSystem * FILESYSTEM_Create(I_Void * vexp)
{
	g_pHunkManager = vexp->hunkManager;
	g_pConsole    = vexp->console;
	
	if(!g_pFileSystem)
		g_pFileSystem = new CFileSystem();
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
	g_pConsole = 0;
	g_pHunkManager = 0;
}

/*
==========================================
Common print function for the module
==========================================
*/
void ComPrintf(char* text, ...)
{
	static char buff[1024];

	va_list args;
	va_start(args, text);
	vsprintf(buff, text, args);
	va_end(args);

	g_pConsole->ComPrint(buff);
}


//======================================================================================
//======================================================================================
