#include "Standard.h"
#include "Ren_exp.h"

I_Console	  *	g_pConsole=0;
I_Void		  *	g_pVoidExp=0;
I_HunkManager * g_pHunkManager=0;
CRenExp		  * g_pRenExp=0;
CMemManager		g_memManager("mem_ren.log");

/*
==========================================
Create the renderer interface, and
start memory logging if in debug mode
==========================================
*/
RENDERER_API I_Renderer * RENDERER_Create(I_Void * vexp)
{
	g_pVoidExp    = vexp;
	g_pHunkManager= vexp->hunkManager;
	g_pConsole	  = vexp->console;

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
	g_pConsole=0;
	g_pHunkManager=0;
	g_pVoidExp=0;
}

/*
==========================================
Return current renderering parms
==========================================
*/
RENDERER_API RenderInfo_t * RENDERER_GetParms()
{	return &g_rInfo;
}

//======================================================================================
//Global utility funcs
//======================================================================================

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

	g_pConsole->ComPrint(buff);
}

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
=======================================
local Error and FATAL error funcs
=======================================
*/
// fatal error - MUST quit
void FError(char *error, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, error);
	vsprintf(textBuffer, error, args);
	va_end(args);
	
	MessageBox(NULL, textBuffer, "Error", MB_OK);
	
	//Win32 func
	PostMessage(g_rInfo.hWnd,	// handle of destination window 
				WM_QUIT,			// message to post 
				0,					// first message parameter 
				0);					// second message parameter 
}

// just a small booboo. let us know and keep going
void Error(char *error, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, error);
	vsprintf(textBuffer, error, args);
	va_end(args);
	MessageBox(NULL, textBuffer, "Error", MB_OK);
}
