#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "Net_client.h"
#include "I_clientRenderer.h"
#include "Cl_game.h"

//Pre-declarations
class  CSoundManager;
class  CMusic;
class  CCamera;
class  CWorld;
struct I_Renderer;

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
	void RotateRight(const float &val);
	void RotateLeft(const float &val);
	void RotateUp(const float &val);
	void RotateDown(const float &val);

	//==================================================
	//Console commands
	void Talk(const char * string);
	bool ValidateName(const CParms &parms);
	bool ValidateRate(const CParms &parms);
	void CamPath();
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

	//==================================================
	//Subsystems

	friend class CClientCmdHandler;
	
	I_Renderer		  * m_pRender;
	I_ClientRenderer  * m_pClRen;

	CSoundManager	  * m_pSound;
	CMusic		      * m_pMusic;
	CClientCmdHandler * m_pCmdHandler;
	CNetClient		  * m_pNetCl;

	//==================================================
	//Client side stuff

	CWorld	 *  m_pWorld;

	int			m_hsTalk;		//handle to talk sound
	int			m_hsMessage;	//handle to server message sound

	float		m_fFrameTime;
	bool		m_ingame;

	int			m_numGameClient;
	ClClient *	m_pClient;

	int			m_numEnts;
	ClEntity 	m_entities[GAME_MAXENTITIES];
	ClClient 	m_clients[GAME_MAXCLIENTS];
	
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