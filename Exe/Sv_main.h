#ifndef VOID_GAME_SERVER
#define VOID_GAME_SERVER

#include "Sys_hdr.h"
#include "World.h"
#include "Net_server.h"

//Temp client entities
class CEntClient
{
public:
	CEntClient(){ inUse = false; memset(name,0,32); }
	
	bool inUse;
	char name[32];
};


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
	bool ValidateClConnection(int chanId, bool reconnect,
							  CBuffer &buffer, 
							  char ** reason);

	void HandleClientMsg(int chanId, CBuffer &buffer);
	void OnClientSpawn(int chanId);
	void OnLevelChange(int chanId);
	void OnClientDrop(int chanId, int state, 
					  const char * reason = 0);

	//Console Handler Interface
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	ServerState m_svState;

	CEntClient  m_client[SV_MAX_CLIENTS];

	CNetServer * m_pNet;

	bool Init();
	void Shutdown();
	void Restart();

	void LoadWorld(const char * mapname);
	void PrintServerStatus();

	//=================================================
	world_t	*	m_pWorld;
	bool		m_active;

	//=================================================
	//CVars
	CVar	m_cDedicated;
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir
};

#endif