#ifndef RENDERER_EXPORT
#define RENDERER_EXPORT

#include "I_renderer.h"

struct I_ClientRenderer;

/*
================================================

================================================
*/
class CRenExp : public I_Renderer,
			    public I_ConHandler
{
public:
	
	CRenExp();
	~CRenExp();

	//Startup/Shutdown
	bool InitRenderer();
	bool Shutdown();

	void Draw(const CCamera * camera);
	void DrawConsole();

	I_ConsoleRenderer * GetConsole();
	I_ClientRenderer  * GetClient();

	//Windowing
	void MoveWindow(int x, int y);
	void Resize();
	void ChangeDispSettings(unsigned int width, unsigned int height, 
							unsigned int bpp, bool fullscreen);

	bool LoadWorld(CWorld *level, int reload);
	bool UnloadWorld();

	bool Restart(void);

	//Console Handler
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms) { }

private:

	CVar   m_cFull;		//fullscreen
	CVar   m_cRes;		//resolution
	CVar   m_cBpp;		//bpp
	CVar   m_cRast;		// rasterizer

	//CVar Handlers
	bool CVar_FullScreen(const CVar * var, const CParms &parms);
	bool CVar_Res(const CVar * var, const CParms &parms);
	bool CVar_Bpp(const CVar * var, const CParms &parms);
	bool CVar_Rast(const CVar * var, const CParms &parms);
};

#endif