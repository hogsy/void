#ifndef VOID_GAME_SERVER
#define VOID_GAME_SERVER

#include "Sys_hdr.h"
#include "Net_server.h"
#include "Game_ents.h"
#include "Game_defs.h"

//Predeclarations
struct world_t;

/*
======================================
interface exported by the main server
class to game dlls
======================================
*/
struct I_GameHandler
{
	virtual void BroadcastPrint(const char * msg)=0;
	virtual void ClientPrint(int clNum, const char * msg)=0;

	virtual NetChanWriter & GetNetChanWriter() =0;

	virtual void DebugPrint(const char * msg)=0;
	virtual void FatalError(const char * msg)=0;

	virtual void PlaySnd(const Entity &ent, int index, int channel, float vol, float atten) =0;
	virtual void PlaySnd(vector_t &origin,  int index, int channel, float vol, float atten) =0;

	virtual void ExecCommand(const char * cmd)=0;

	virtual int  RegisterModel(const char * model)=0;
	virtual int  RegisterSound(const char * image)=0;
	virtual int  RegisterImage(const char * sound)=0;
};


/*
======================================
The Main Server class
======================================
*/
class CServer : public I_Server,
				public I_GameHandler,
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

	//Game Interface
	void ExecCommand(const char * cmd);
	void DebugPrint(const char * msg);
	void FatalError(const char * msg);

	void BroadcastPrint(const char * msg);
	void ClientPrint(int clNum, const char * msg);

	NetChanWriter & GetNetChanWriter();

	void PlaySnd(const Entity &ent, int index, int channel, float vol, float atten);
	void PlaySnd(vector_t &origin,  int index, int channel, float vol, float atten);

	int  RegisterModel(const char * model);
	int  RegisterSound(const char * image);
	int  RegisterImage(const char * sound);

	//Console Handler Interface
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

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

	ResInfo	m_modelList[GAME_MAXMODELS];
	int		m_numModels;
	ResInfo m_imageList[GAME_MAXIMAGES];
	int     m_numImages;
	ResInfo m_soundList[GAME_MAXSOUNDS];
	int		m_numSounds;
	
	//=================================================
	//World and Entities
	world_t	*	m_pWorld;
	bool		m_active;
	
	//=================================================

	ServerState m_svState;
	CNetServer	m_net;
	NetChanWriter & m_chanWriter;

	//=================================================
	//CVars
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir


	//=================================================
	//Should be handled by the game Dll

	//the first num MAXClient entities are reserved for clients
	Entity    ** m_entities;
	int			 m_maxEntities;
	int			 m_numEntities;
	
	EntClient ** m_clients;
};


extern CServer * g_pServer;


#endif