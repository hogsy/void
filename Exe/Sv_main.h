#ifndef VOID_GAME_SERVER
#define VOID_GAME_SERVER

#include "Sys_hdr.h"
#include "Net_server.h"
#include "Game_defs.h"
#include "I_game.h"

//Predeclarations
struct world_t;

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
	void OnClientBegin(int clNum);
	void OnClientDrop(int clNum, EDisconnectReason reason);
	void OnLevelChange(int clNum);
	void WriteGameStatus(CBuffer &buffer);

	//Game Interface
	void ExecCommand(const char * cmd);
	void DebugPrint(const char * msg);
	void FatalError(const char * msg);

	void BroadcastPrintf(const char * msg,...);
	void ClientPrintf(int clNum, const char * msg,...);

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
	void PrintServerStatus();

	void LoadEntities();
	void LoadWorld(const char * mapname);
	
	bool WriteEntBaseLine(const Entity * ent, CBuffer &buf) const;
	void WriteSignOnBuffer(NetSignOnBufs &signOnBuf);
	
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
	char		m_printBuffer[512];

	//The Game Interface
	HINSTANCE m_hGameDll;
	I_Game *  m_pGame;

	//=================================================
	//CVars
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir

	//=================================================

	//These should just point to the data in the GAME code
	Entity    ** m_entities;
	int			 m_maxEntities;
	int			 m_numEntities;
	EntClient ** m_clients;
};


extern CServer * g_pServer;


#endif