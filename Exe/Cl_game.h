#ifndef VOID_CLIENT_STATE
#define VOID_CLIENT_STATE

#include "Com_buffer.h"
#include "Net_client.h"

//Forward declarations
class  CWorld;
class  CCamera;
class  CClientGameInput;
struct I_ClientGame;
struct I_InKeyListener;
struct I_InCursorListener;

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
	
	CGameClient(I_ClientGame * pClGame);
	~CGameClient();

	//NetClient Implementation
	//Parse and handle a game message
	void HandleGameMsg(CBuffer &buffer); 
	//Parse and handle spawm parms
	void HandleSpawnMsg(byte msgId, CBuffer &buffer); 
	//Handle disconnect from server
	void HandleDisconnect();
	//Put Client in game. The clNum is the clients num on the server
	void BeginGame(int clNum, CBuffer &buffer);
	//Write userInfo to the given buffer
	void WriteUserInfo(CBuffer &buffer);
	//Util Print
	void Print(const char * msg, ...);

	//Needed by the Main Client System
	I_InKeyListener		* GetKeyListener();
	I_InCursorListener	* GetCursorListener();
	CCamera *			  GetCamera();

	void RunFrame(float frameTime);
	
	void WriteCmdUpdate(CBuffer &buf);
	void WriteFullUpdate(CBuffer &buf);

	//Console Handler Implementation
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);

private:

	bool LoadWorld(CWorld * pWorld);
	void UnloadWorld();

	//==================================================
	//Movement
	void UpdatePosition(vector_t &dir, float time);
	void UpdateAngles(const vector_t &angles, float time);
	
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

	void Spawn(vector_t	*origin, vector_t *angles);
	void UpdateViewBlends();


	//==================================================
	//CVars and Commands

	void Talk(const char * string);
	bool ValidateName(const CParms &parms);
	bool ValidateRate(const CParms &parms);

	CVar    m_cvKbSpeed;
	CVar	m_cvName;
	CVar	m_cvRate;
	CVar    m_cvModel;
	CVar    m_cvSkin;
	CVar	m_cvClip;

	//==================================================

	int			m_hsTalk;		//handle to talk sound
	int			m_hsMessage;	//handle to server message sound

	bool		m_ingame;
	ClCmd		m_cmd;
	ClCmd		m_oldCmd;

	int			m_campath;
	float		m_camtime;
	float		m_maxvelocity;
	
	//==================================================
	//Client Side Entities

	int			m_numEnts;
	ClEntity 	m_entities[GAME_MAXENTITIES];
	
	ClClient 	m_clients[GAME_MAXCLIENTS];
	ClClient *	m_pGameClient;


	//==================================================

	CClientGameInput *	m_pCmdHandler;
	I_ClientGame	 *	m_pClGame;

	CWorld	*	m_pWorld;

	CCamera	*	m_pCamera;
	
	vector_t	m_vecMouseAngles;

	vector_t	m_vecBlend;
	vector_t	m_vecForward,
				m_vecRight,
				m_vecUp,
				m_vecVelocity;
	
	vector_t	m_vecDesiredMove,
				m_vecDesiredAngles;
};

#endif