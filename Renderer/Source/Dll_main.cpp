#include <windows.h>
//#include <crtdbg.h>
#include "Ren_exp.h"

/*
==========================================
These are the only functions directly
exported by the dll
==========================================
*/
//static HFILE * g_hmemlog;

extern RenderInfo_t g_rInfo;

I_Console	 * g_pConsole=0;
I_Void		 * g_pVoidExp=0;
I_MemManager * g_pMemManager=0;


float & GetCurTime()
{	return g_pVoidExp->GetCurTime();
}

float & GetFrameTime()
{	return g_pVoidExp->GetFrameTime();
}

const char * GetCurPath()
{	return g_pVoidExp->GetCurPath();
}

/*
==========================================
Create the renderer interface, and
start memory logging if in debug mode
==========================================
*/
//RENDERER_API I_Renderer * RENDERER_Create(VoidExport_t * vexp)
RENDERER_API I_Renderer * RENDERER_Create(I_Void * vexp)
{
	g_pMemManager = vexp->memManager;
	g_pConsole	  = vexp->console;
/*
#ifdef _DEBUG
	
	// memory debugging stuff
	g_hmemlog = (HFILE*)CreateFile("mem_render.log", 
					GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Send all reports to STDOUT, since this example is a console app
   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_WARN, g_hmemlog);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ERROR, g_hmemlog);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ASSERT, g_hmemlog);
#endif
*/
	if(!g_pRenExp)
	{
		g_pRenExp = new CRenExp();
		if(!g_pRenExp)
		{
			::MessageBox(0,"CreateRenderer:: Unable to create Renderer",
							"Error : CreateRenderer",MB_OK|MB_ICONERROR);
			return 0;
		}
		return g_pRenExp;
	}
	::MessageBox(0,"CreateRenderer:: Renderer is already intialized",
					"Warning : CreateRenderer",MB_OK|MB_ICONERROR);
	return 0;
}

/*
==========================================
Free the renderer interface, and
end memory logging if in debug mode
==========================================
*/
RENDERER_API void RENDERER_Free()
{
	if(g_pRenExp)
	{
		delete g_pRenExp;
		g_pRenExp = 0;
	}
/*
#ifdef _DEBUG
	// memory debugging stuff
   _CrtDumpMemoryLeaks();
	CloseHandle(g_hmemlog);
#endif
*/
}

/*
==========================================
Return current renderering parms
==========================================
*/
RENDERER_API RenderInfo_t * RENDERER_GetParms()
{	return &g_rInfo;
}


/*
==========================================
Dll Print func
==========================================
*/
void ComPrintf(char* text, ...)
{
	if ((!text) || (!text[0]))
		return;

	static char buff[2048];

	va_list args;
	va_start(args, text);
	vsprintf(buff, text, args);
	va_end(args);

	g_pConsole->ConPrint(buff);
}