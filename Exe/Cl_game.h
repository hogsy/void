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

	//network
	int  GetOutgoingRate() const;

	void RunFrame(float frameTime);
	
	void WriteCmdUpdate(CBuffer &buf);
	void WriteFullUpdate(CBuffer &buf);

	//Console Handler Implementation
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);

private:

	bool LoadWorld(CWorld * pWorld);
	void UnloadWorld();

	void ReadClientInfo(CBuffer &buffer);

	//==================================================
	//Movement
	void UpdatePosition(const float &time);
		
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void Jump();
	void Crouch();
	void RotateRight(const float &val);
	void RotateLeft(const float &val);
	void RotateUp(const float &val);
	void RotateDown(const float &val);
	void CamPath();

	//==================================================

	void UpdateViewAngles(const float &time);
	void UpdateViewBlends();

	void Spawn(vector_t	&origin, vector_t &angles);

	//==================================================
	//CVars and Commands

	void Talk(const char * string);
	bool ValidateName(const CParms &parms);
	bool ValidateRate(const CParms &parms);
	bool ValidateCharacter(const CParms &parms);

	CVar    m_cvKbSpeed;
	CVar	m_cvName;
	CVar	m_cvInRate;
	CVar	m_cvOutRate;
	CVar	m_cvCharacter;
	CVar	m_cvClip;
	CVar    m_cvDefaultChar;
	CVar	m_cvViewTilt;

	//==================================================

	int			m_hsTalk;		//handle to talk sound
	int			m_hsMessage;	//handle to server message sound
	
	bool		m_ingame;

	ClCmd		m_cmd;
	ClCmd		m_oldCmd;

	int			m_campath;
	float		m_camtime;
	
	//==================================================
	//Client Side Entities

	int			m_numEnts;
	ClEntity 	m_entities[GAME_MAXENTITIES];

	int			m_numClients;
	ClClient 	m_clients[GAME_MAXCLIENTS];
	
	int			m_clNum;
	ClClient *	m_pGameClient;


	//==================================================

	CClientGameInput *	m_pCmdHandler;
	I_ClientGame	 *	m_pClGame;

	CWorld	*	m_pWorld;

	CCamera	*	m_pCamera;
	
	bool		m_bOnGround;
	vector_t	m_vecBlend;
	vector_t	m_vecForward,
				m_vecRight,
				m_vecUp;
	vector_t	m_vecDesiredAngles;
};

#endif