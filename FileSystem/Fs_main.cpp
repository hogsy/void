#include "Fs_hdr.h"
#include "Fs_filesys.h"
#include "Com_hunk.h"

const char MEM_SZLOGFILE [] = "mem_fs.log";

CFileSystem	  *	g_pFileSystem = 0;

static PrintFunc	m_pPrintFunc = 0;
static ErrorFunc	m_pErrorFunc = 0;

/*
================================================
Create the fileSystem and return it
copy pointer to console for cvar/printing functions
-===============================================
*/
FILESYSTEM_API I_FileSystem * FILESYSTEM_Create(PrintFunc pPrint, ErrorFunc pError,
						const char * exeDir,const char * baseGameDir)
{
	static CHunkMem m_hunkMem;
	g_pHunkManager = &m_hunkMem;

	m_pPrintFunc = pPrint;
	m_pErrorFunc = pError;

	if(!g_pFileSystem)
		g_pFileSystem = new CFileSystem(exeDir,baseGameDir);
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
	m_pPrintFunc = 0;
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

	m_pPrintFunc(buff);
}

/*
======================================
Com mem hancler
======================================
*/
int HandleOutOfMemory(size_t size)
{	
	m_pErrorFunc("FileSystem: Out of Memory");
	return 0;
}