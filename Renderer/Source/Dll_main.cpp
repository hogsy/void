#include <windows.h>
#include "Ren_exp.h"

extern RenderInfo_t g_rInfo;
I_Console	 * g_pConsole=0;
I_Void		 * g_pVoidExp=0;
I_MemManager * g_pMemManager=0;


//======================================================================================
//These are the only functions directly exported by the dll
//======================================================================================

/*
==========================================
Create the renderer interface, and
start memory logging if in debug mode
==========================================
*/
RENDERER_API I_Renderer * RENDERER_Create(I_Void * vexp)
{
	g_pVoidExp    = vexp;
	g_pMemManager = vexp->memManager;
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
	g_pVoidExp=0;
	g_pMemManager=0;
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

	g_pConsole->ConPrint(buff);
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