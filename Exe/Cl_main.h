#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "Com_buffer.h"
#include "Net_defs.h"
#include "Net_client.h"
#include "Com_vector.h"
#include "Cl_defs.h"
#include "I_clientRenderer.h"

//Pre-declarations
class  CSoundManager;
class  CMusic;
class  CCamera;
class  CWorld;
struct I_Renderer;
struct I_HudRenderer;


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
class CClient :	public I_ConHandler,
				public I_NetClientHandler
{
public:
	CClient(I_Renderer * prenderer,
			CSoundManager * psound,
			CMusic	* pmusic);

	~CClient();

	void RunFrame();
	void SetInputState(bool on);

	//Client Interface
	//Parse and handle a game message
	void HandleGameMsg(CBuffer &buffer); 
	
	//Parse and handle spawm parms
	void HandleSpawnMsg(byte msgId, CBuffer &buffer); 

	//Handle disconnect from server
	void HandleDisconnect(bool listenserver);

	//Put Client in game. The clNum is the clients num on the server
	void BeginGame(int clNum, CBuffer &buffer);

	//Write userInfo to the given buffer
	void WriteUserInfo(CBuffer &buffer);

	//Util Print
	void Print(const char * msg, ...);
	
	//Console Interface
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	
private:

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
	CVar    m_cvKbSpeed;
	CVar	m_cvClip;
	CVar	m_cvNetStats;

	void WriteUpdate();

	//==================================================
	//Subsystems
	friend class CClientGameCmd;
	friend class CClientState;

	CClientGameCmd    * m_pCmdHandler;
	CClientState      *	m_pClState;

	I_Renderer		  * m_pRender;
	I_ClientRenderer  * m_pClRen;
	I_HudRenderer	  * m_pHud;

	CSoundManager	  * m_pSound;
	CMusic		      * m_pMusic;
	CNetClient		  * m_pNetCl;

	

	CWorld	 *  m_pWorld;

	int			m_hsTalk;		//handle to talk sound
	int			m_hsMessage;	//handle to server message sound

	float		m_fFrameTime;

};

#endif