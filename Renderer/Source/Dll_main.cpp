#include <windows.h>
#include <crtdbg.h>
#include "Ren_exp.h"

/*
==========================================
These are the only functions directly
exported by the dll
==========================================
*/

static HFILE * g_hmemlog;
extern RenderInfo_t g_rInfo;

/*
==========================================
Create the renderer interface, and
start memory logging if in debug mode
==========================================
*/


RENDERER_API I_Renderer * RENDERER_Create(VoidExport_t * vexp)
{
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

	if(!g_pRenExp)
	{
		g_pRenExp = new CRenExp(vexp);
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

#ifdef _DEBUG
	// memory debugging stuff
   _CrtDumpMemoryLeaks();
	CloseHandle(g_hmemlog);
#endif

}


RENDERER_API RenderInfo_t * RENDERER_GetParms()
{	return &g_rInfo;
}



/*
HRESULT  __stdcall GetRendererAPI(I_Renderer ** pRender, RenderInfo_t *rinfo, VoidExport_t * vexp)
{
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

	if(!*pRender)
	{
		g_pRenExp = new CRenExp(rinfo,vexp);
		*pRender = g_pRenExp;
		return S_OK;
	}
	return E_FAIL;
}
*/
/*
HRESULT __stdcall FreeRenderer(I_Renderer ** pRender)
{
	if(!*pRender)
		return E_FAIL;

	*pRender = 0;
	delete g_pRenExp;
	g_pRenExp = 0;

#ifdef _DEBUG
	// memory debugging stuff
   _CrtDumpMemoryLeaks();
	CloseHandle(g_hmemlog);
#endif

	return S_OK;
}
*/