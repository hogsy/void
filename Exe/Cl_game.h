#ifndef VOID_CLIENT_STATE
#define VOID_CLIENT_STATE

#include "Sys_hdr.h"
#include "Com_buffer.h"
#include "Com_vector.h"
#include "Cl_defs.h"
#include "Net_client.h"


class  CWorld;

class  CCamera;

class  CClient;
class  CSoundManager;
class  CMusic;
class  CCamera;
struct I_HudRenderer;
struct I_ClientRenderer;


//This will need Resource loading interfaces
//And simple sound/music interfaces from the main client


/*
================================================
Maintains clients game state
Nearly all cleint side systems act on this data
================================================
*/
class CGameClient : public I_ConHandler,
				    public I_NetClientHandler
{
public:
	CGameClient(CClient	   & rClient,
				 I_ClientRenderer	   * pRenderer,
				 I_HudRenderer * pHud,
				 CSoundManager * pSound,
				 CMusic		   * pMusic);

	~CGameClient();


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



	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);

	vector_t m_moveAngles;
	CVar    m_cvKbSpeed;

	//spawn for the first time.
	void BeginGame();
	
	bool LoadWorld(CWorld * pWorld);
	void UnloadWorld();

	void RunFrame(float frameTime);
	void WriteCmdUpdate(CBuffer &buf);
	void UpdateView();

	int			m_hsTalk;		//handle to talk sound
	int			m_hsMessage;	//handle to server message sound


	//==================================================
	//Movement
	void Move(vector_t &dir, float time);
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void RotateRight(const float &val);
	void RotateLeft(const float &val);
	void RotateUp(const float &val);
	void RotateDown(const float &val);
	void CamPath();

	//==================================================
	//Client side stuff
	I_ClientRenderer	  * m_pRenderer;
	I_HudRenderer * m_pHud;
	CSoundManager * m_pSound;
	CMusic		  * m_pMusic;

	friend class CClientGameInput;
	CClientGameInput * m_pCmdHandler;


	CWorld	 *  m_pWorld;
	CClient	 &	m_refClient;

//	float		m_fFrameTime;
	bool		m_ingame;

	int			m_numEnts;
	ClEntity 	m_entities[GAME_MAXENTITIES];
	ClClient 	m_clients[GAME_MAXCLIENTS];

	ClClient *	m_pGameClient;
	
	ClCmd		m_cmd;
	ClCmd		m_oldCmd;

	//This should hook up to the game client whne the client
	//enters a game
	CCamera	*	m_pCamera;

	vector_t	m_screenBlend;
	vector_t	desired_movement;
	
	int			m_campath;
	float		m_camtime;
	float		m_maxvelocity;

	void Spawn(vector_t	*origin, vector_t *angles);
};

#endif