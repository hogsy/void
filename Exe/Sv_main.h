#ifndef VOID_GAME_SERVER
#define VOID_GAME_SERVER

#include "Sys_hdr.h"
#include "Net_server.h"

//Predeclarations
struct world_t;
struct EntClient;

/*
======================================
The Main Server class
======================================
*/
class CServer : public I_Server,
				public I_ConHandler
{
public:

	static bool InitNetwork();
	static void ShutdownNetwork();

	CServer();
	~CServer();

	void RunFrame();

	//Network Handler
	bool ValidateClConnection(int clNum, bool reconnect,CBuffer &buffer);
	void HandleClientMsg(int clNum, CBuffer &buffer);
	void OnClientSpawn(int clNum);
	void OnLevelChange(int clNum);
	void OnClientDrop(int clNum, EDisconnectReason reason);
	void WriteGameStatus(CBuffer &buffer);

	//Console Handler Interface
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	bool Init();
	void Shutdown();
	void Restart();

	//Parse entity data from world file into 
	//Entity spawn buffers
	bool ParseEntities(NetSignOnBufs &signOnBuf);

	void LoadWorld(const char * mapname);
	void PrintServerStatus();

	//=================================================
	world_t	*	m_pWorld;
	bool		m_active;

	CNetServer	m_net;
	ServerState m_svState;

	EntClient *m_client;

	NetChanWriter & m_chanWriter;

	//=================================================
	//CVars
	CVar	m_cDedicated;
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir
};

#endif