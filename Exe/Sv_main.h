#ifndef VOID_GAME_SERVER
#define VOID_GAME_SERVER

#include "Sys_hdr.h"
#include "Net_server.h"

//Predeclarations
struct world_t;
struct EntClient;


const int GAME_MAXMODELS = 256;
const int GAME_MAXSOUNDS = 256;
const int GAME_MAXIMAGES = 256;

typedef int hMdl;
typedef int hImg;

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

	int  RegisterEntity(const char * ent);
	hMdl RegisterModel(const char * model);
	int  RegisterSound(const char * image);
	hImg RegisterImage(const char * sound);

private:

	bool Init();
	void Shutdown();
	void Restart();

	//Parse entity data from world file into 
	//Entity spawn buffers
	void SpawnEntities(CBuffer &buf);
	bool ParseEntities(NetSignOnBufs &signOnBuf);

	void LoadWorld(const char * mapname);
	void PrintServerStatus();

	//=================================================
	//List of currenly loaded Resources
	struct ResInfo
	{
		ResInfo()  { id =0; name = 0; }
		~ResInfo() { if(name) delete[] name; }

		int    id;
		char * name; 
	};

	ResInfo	m_modelList[256];
	int		m_numModels;

	ResInfo m_imageList[256];
	int     m_numImages;

	ResInfo m_soundList[256];
	int		m_numSounds;
	
	//=================================================
	//World and Entities
	world_t	*	m_pWorld;
	bool		m_active;
	
	EntClient * m_client;

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
};

#endif