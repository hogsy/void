#ifndef RENDERER_EXPORT
#define RENDERER_EXPORT

#include "I_renderer.h"


//Forward declarations
struct I_ClientRenderer;
class  CRHud;
class  CRConsole;

class CRenExp : public I_Renderer,
			    public I_ConHandler
{
public:
	
	CRenExp();
	~CRenExp();

	//Startup/Shutdown
	bool InitRenderer();
	bool Shutdown();

	void Draw(const CCamera * camera=0);

	I_ConsoleRenderer * GetConsole();
	I_ClientRenderer  * GetClient();
	I_HudRenderer	  * GetHud();

	//Windowing
	void MoveWindow(int x, int y);
	void Resize();
	void ChangeDispSettings(unsigned int width, unsigned int height, 
							unsigned int bpp, bool fullscreen);

	bool LoadWorld(CWorld *level);
	bool UnloadWorld();

	bool Restart(void);

	//Console Handler
	bool HandleCVar(const CVarBase * cvar, const CStringVal &strVal);
	void HandleCommand(int cmdId, const CParms &parms) { }

private:

	CRHud		* m_pHud;
	CRConsole	* m_pRConsole;

	CVar   m_cFull;		//fullscreen
	CVar   m_cRes;		//resolution
	CVar   m_cBpp;		//bpp
	CVar   m_cRast;		// rasterizer

	//CVar Handlers
	bool CVar_FullScreen(const CStringVal &strVal);
	bool CVar_Res(const CStringVal &strVal);
	bool CVar_Bpp(const CStringVal &strVal);
	bool CVar_Rast(const CStringVal &strVal);
};

#endif