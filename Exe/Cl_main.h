#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "Net_client.h"
#include "Clgame_defs.h"

//#include "I_renderer.h"
#include "Game_ents.h"


//Pre-declarations
class  CCamera;
class  CSoundManager;
class  CMusic;
struct I_Renderer;
struct I_RHud;
struct I_Model;
struct I_Image;

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
	CVar    m_clmodel;
	CVar    m_clskin;
	
	CVar	m_noclip;
	

	//==================================================
	//Subsystems

	friend class CClientCmdHandler;
	
	I_Renderer		  * m_pRender;
	I_RHud		 	  *	m_pHud;
	I_Model			  * m_pModel;
	I_Image			  * m_pImage;


	CSoundManager	  * m_pSound;
	CMusic		      * m_pMusic;
	CClientCmdHandler * m_pCmdHandler;
	CNetClient		  * m_pNetCl;

	//==================================================
	//the following should be accessible by the client game dll

	//==================================================
	//Client side stuff
	int			m_hsTalk;		//handle to talk sound
	int			m_hsMessage;	//handle to server message sound

	float		m_fFrameTime;
	bool		m_ingame;



	//This is the client we should do local prediction on

	ClClient	m_gameClient;
	ClClient 	m_clients[GAME_MAXCLIENTS];

	int			m_numEnts;
	ClEntity 	m_entities[GAME_MAXENTITIES];

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