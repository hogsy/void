#ifndef RENDERER_EXPORT
#define RENDERER_EXPORT

#include "I_renderer.h"

class CRenExp : public I_Renderer,
			    public I_CVarHandler
{
public:
	
	CRenExp();
	~CRenExp();

	//Startup/Shutdown
	bool InitRenderer();
	bool Shutdown();

	void DrawFrame(vector_t *origin,vector_t *angles, vector_t *blend);

	I_ConsoleRenderer * GetConsole();
	I_RHud *			GetHud();

	//Windowing
	void MoveWindow(int x, int y);
	void Resize();
	void ChangeDispSettings(unsigned int width, unsigned int height, 
							unsigned int bpp, bool fullscreen);

	bool LoadWorld(world_t *level, int reload);
	bool UnloadWorld();

	bool Restart(void);

	//CVar Handler
	bool HandleCVar(const CVarBase *cvar,int numArgs, char ** szArgs);

private:

	CVar   m_cFull;		//fullscreen
	CVar   m_cRes;		//resolution
	CVar   m_cBpp;		//bpp

	//CVar Handlers
	bool CVar_FullScreen(const CVar * var, int argc, char** argv);
	bool CVar_Res(const CVar * var, int argc, char** argv);
	bool CVar_Bpp(const CVar * var, int argc, char** argv);

};

#endif