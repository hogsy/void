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

	enum 
	{
		CL_DISCONNECTED,
		CL_RECONNECTING,
		CL_INGAME

	};

	void RunFrame();

	void SetInputState(bool on);

	//Console Interface
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	
private:

	//Hacks 
	void SetState(int state);
	void SetNetworkRate(int rate);

	CBuffer & GetSendBuffer();
	CBuffer & GetOutgoingSendBuffer();


	//Handle disconnect from server
	void HandleDisconnect(bool listenserver);

	//Write userInfo to the given buffer
	void WriteUserInfo(CBuffer &buffer);


	bool LoadWorld(const char *worldname);
	void UnloadWorld();

	//==================================================
	//Console commands
	void Talk(const char * string);
	bool ValidateName(const CParms &parms);
	bool ValidateRate(const CParms &parms);
	void ShowNetStats();

	//==================================================
	//Client CVars
	CVar	m_cvPort;
	CVar	m_cvName;
	CVar	m_cvRate;
	CVar    m_cvModel;
	CVar    m_cvSkin;

//	CVar    m_cvKbSpeed;
	CVar	m_cvClip;
	CVar	m_cvNetStats;

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