#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "Com_vector.h"
#include "Cl_defs.h"


//Pre-declarations
class  CSoundManager;
class  CMusic;
class  CWorld;
class  CNetClient;
class  CBuffer;

class CClientExports;

struct I_Renderer;
struct I_HudRenderer;
struct I_ClientRenderer;

/*
=====================================
Client class
-controls the camera
-the game commands,
-the hud while IN GAME
-processes server messages to update world
-predicts item positions
-basically all the user interactive elements which are only available when in game
=====================================
*/
class CClient :	public I_ConHandler
{
public:
	CClient(I_Renderer * prenderer,
			CSoundManager * psound,
			CMusic	* pmusic);

	~CClient();

	void RunFrame();

	void SetInputState(bool on);

	//Console Interface
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	
private:

	friend class CClientExports;
	CClientExports	* m_pExports;

	//Hacks 
	void SetClientState(int state);
	void SetNetworkRate(int rate);
	bool LoadWorld(const char *worldname);
	void UnloadWorld();

	CBuffer & GetSendBuffer();
	CBuffer & GetReliableSendBuffer();


	//==================================================
	//Console commands

	void ShowNetStats();

	//==================================================
	//Client CVars
	CVar	m_cvPort;


	void WriteUpdate();

	//==================================================
	//Subsystems
	friend class CGameClient;
	CGameClient      *	m_pClState;

	I_Renderer		  * m_pRender;
	I_ClientRenderer  * m_pClRen;
	I_HudRenderer	  * m_pHud;

	CSoundManager	  * m_pSound;
	CMusic		      * m_pMusic;
	CNetClient		  * m_pNetCl;

	CWorld	 *  m_pWorld;
	float		m_fFrameTime;

};

#endif