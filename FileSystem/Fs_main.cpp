#include "I_filesystem.h"

/*
==========================================
Local vars
==========================================
*/

#ifdef _DEBUG
#include <crtdbg.h>
static HFILE	* m_phMemfile=0;
#endif

I_Console    * g_pConsole = 0;
CFileSystem  * g_pFileSystem = 0;

/*
==========================================
Memory Reporting, used in Debug mode
==========================================
*/
static void InitMemReporting()
{
#ifdef _DEBUG
   
   if(m_phMemfile)
	   return;
	
   m_phMemfile = (HFILE*)CreateFile("mem_fs.log",
						GENERIC_WRITE, FILE_SHARE_WRITE, 
						NULL, CREATE_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL, NULL);

   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_WARN, m_phMemfile);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ERROR, m_phMemfile);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ASSERT, m_phMemfile);
#endif
}

static void EndMemReporting()
{
#ifdef _DEBUG
	if(m_phMemfile)
	{
		_CrtDumpMemoryLeaks();
		CloseHandle(m_phMemfile);
	}
#endif
}

/*
==========================================
Create the fileSystem and return it
copy pointer to console for cvar/printing functions
==========================================
*/
FILESYSTEM_API CFileSystem * FILESYSTEM_Create(I_Console * pconsole)
{	
	InitMemReporting();

	g_pConsole = pconsole;
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
		g_pConsole = 0;
	}
	EndMemReporting();
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

	g_pConsole->ConPrint(buff);
}