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

/*
==========================================
Create the renderer interface, and
start memory logging if in debug mode
==========================================
*/

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


/*
==========================================
Free the renderer interface, and
end memory logging if in debug mode
==========================================
*/

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
