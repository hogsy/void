#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "I_renderer.h"
#include "Snd_main.h"
#include "Mus_main.h"
#include "Net_client.h"
#include "Game_ents.h"

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
	void WriteBindTable(FILE *fp);

	//Client Interface
	
	//Parse and handle a game message
	void HandleGameMsg(CBuffer &buffer); 
	
	//Parse and handle spawm parms
	void HandleSpawnMsg(const byte &msgId, CBuffer &buffer); 

	//Handle disconnect from server
	void HandleDisconnect(bool listenserver);

	//Write userInfo to the given buffer
	void WriteUserInfo(CBuffer &buffer);

	//Util Print
	void Print(const char * msg, ...);
	
	//Console Interface
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	
private:

	//spawn for the first time.
	void BeginGame();
	bool LoadWorld(const char *worldname);
	void UnloadWorld();

	//==================================================
	//Movement
	void Move(vector_t *dir, float time);
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void RotateRight(float val=5.0);
	void RotateLeft(float val=5.0);
	void RotateUp(float val=5.0);
	void RotateDown(float val=5.0);

	//==================================================
	//Console commands
	void Talk(const char * string);
	bool ValidateName(const CParms &parms);
	bool ValidateRate(const CParms &parms);
	void CamPath();

	//==================================================
	//Client CVars
	CVar	m_clport;
	CVar	m_clname;
	CVar	m_clrate;
	CVar	m_noclip;

	//==================================================
	//Subsystems

	friend class CClientCmdHandler;
	
	I_Renderer		  * m_pRender;
	I_RHud		 	  *	m_pHud;
	CSoundManager	  * m_pSound;
	CMusic		      * m_pMusic;
	CClientCmdHandler * m_pCmdHandler;
	CNetClient		  * m_pNetCl;

	//==================================================
	//Client side stuff
	int			m_hsTalk;		//handle to talk sound
	int			m_hsMessage;	//handle to server message sound

	float		m_fFrameTime;
	bool		m_ingame;

	//==================================================
	//the following should be accessible by the game dll

	//This is the client we should do local prediction on
	EntClient	m_gameClient;

	//This should hook up to the game client whne the client
	//enters a game
	CCamera	*	m_pCamera;

	vector_t	m_screenBlend;

	vector_t	desired_movement;
	
	int			m_campath;
	float		m_camtime;
	float		m_acceleration;
	float		m_maxvelocity;

	void Spawn(vector_t	*origin, vector_t *angles);
};

#endif