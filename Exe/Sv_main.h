#ifndef VOID_GAME_SERVER
#define VOID_GAME_SERVER

#include "Sys_hdr.h"
#include "Net_server.h"
#include "Game_ents.h"
#include "Res_defs.h"

//Predeclarations
struct world_t;
struct EntClient;

const int GAME_MAXENTITES= 1024;


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

	int  RegisterModel(const char * model);
	int  RegisterSound(const char * image);
	int  RegisterImage(const char * sound);

private:

	bool Init();
	void Shutdown();
	void Restart();

	void InitGame();

	//Parse entity data from world file into 
	//Entity spawn buffers
	bool SpawnEntity(CBuffer &buf);

	void LoadEntities();
	
	void WriteSignOnBuffer(NetSignOnBufs &signOnBuf);


//	bool LoadEntities(NetSignOnBufs &signOnBuf);

	void LoadWorld(const char * mapname);
	void PrintServerStatus();

	//=================================================
	//List of currenly loaded Resources
	struct ResInfo
	{
		ResInfo()  { name = 0; } 
		~ResInfo() { if(name) delete[] name; }
		char * name; 
	};

	ResInfo	m_modelList[RES_MAXMODELS];
	int		m_numModels;

	ResInfo m_imageList[RES_MAXIMAGES];
	int     m_numImages;

	ResInfo m_soundList[RES_MAXSOUNDS];
	int		m_numSounds;
	
	//=================================================
	//World and Entities
	world_t	*	m_pWorld;
	bool		m_active;

	EntClient * m_clients;

	//=================================================

	CNetServer	m_net;
	ServerState m_svState;

	NetChanWriter & m_chanWriter;

	//=================================================
	//CVars
	CVar	m_cDedicated;
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir


	//=================================================
	//Should be handled by the game Dll
	Entity   ** m_entities;
	int			m_maxEntities;
	int			m_numEntities;

};


extern CServer * g_pServer;


#endif