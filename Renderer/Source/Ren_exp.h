#ifndef RENDERER_EXPORT
#define RENDERER_EXPORT

#include "I_renderer.h"


class CRenExp : public I_Renderer
{
public:

	//Startup/Shutdown
	bool PreInit(RenderInfo_t *rinfo, VoidExport_t ** vexp);

	bool InitRenderer();
	bool Shutdown();

	void DrawFrame(vector_t *origin,vector_t *angles);

	I_RConsole * GetConsole();
	I_RHud *	 GetHud();


	//Windowing
	void MoveWindow(int x, int y);
	void Resize();
	void ChangeDispSettings(unsigned int width, unsigned int height, 
									unsigned int bpp, unsigned int fullscreen);

	bool LoadWorld(world_t *level, int reload);
	bool UnloadWorld();

	bool Restart(void);

	//CVar Handlers
	static bool CVar_FullScreen(const CVar * var, int argc, char** argv);
	static bool CVar_Res(const CVar * var, int argc, char** argv);
	static bool CVar_Bpp(const CVar * var, int argc, char** argv);
		

	CRenExp();
	~CRenExp();

private:

	static CVar *	m_cFull;		//fullscreen
	static CVar *   m_cRes;			//resolution
	static CVar *   m_cBpp;			//bpp
	static CVar *	m_cWndX;
	static CVar *   m_cWndY;
	static CVar *   m_cGLExt;

	VoidExport_t*	m_pImport;		//Imported Interface
};

extern CRenExp * g_pRenExp;

#endif


