#ifndef VOID_GAME_SERVER
#define VOID_GAME_SERVER

#include "Sys_hdr.h"
#include "Com_buffer.h"
#include "Net_server.h"
#include "Com_vector.h"
#include "I_game.h"

/*
============================================================================
This info is sent to the client in a series of stages during the
connection phase. Giving the client indexes for Images/Sounds/Models and
Entity baselines will save a LOT of network traffic during gameplay.
Therefore the Game Server NEEDs to update this data on every map change.
The struct is HUGE in size, About 22k, 
But making a list doesnt seem worth the hassle
============================================================================
*/
struct NetSignOnBufs
{
	enum
	{	MAX_IMAGE_BUFS = 4,
		MAX_MODEL_BUFS = 4,
		MAX_SOUND_BUFS = 4,
		MAX_ENTITY_BUFS = 4
	};

	NetSignOnBufs() 
	{	Reset();
	}

	void Reset()
	{	
		gameInfo.Reset();
		numImageBufs = 1;
		for(int i=0; i< MAX_IMAGE_BUFS; i++)
			imageList[i].Reset();
		numSoundBufs = 1;
		for(i=0; i< MAX_SOUND_BUFS; i++)
			soundList[i].Reset();
		numModelBufs = 1;
		for(i=0; i< MAX_MODEL_BUFS; i++)
			modelList[i].Reset();
		numEntityBufs = 1;
		for(i=0; i< MAX_ENTITY_BUFS; i++)
			entityList[i].Reset();
	}

	CBuffer  gameInfo;
	int		 numImageBufs;
	CBuffer  imageList[MAX_IMAGE_BUFS];
	int      numModelBufs;
	CBuffer  modelList[MAX_MODEL_BUFS];
	int		 numSoundBufs;
	CBuffer  soundList[MAX_SOUND_BUFS];
	int      numEntityBufs;
	CBuffer  entityList[MAX_ENTITY_BUFS];
};


//Predeclarations
class CWorld;

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
	void OnClientDrop(int clNum, const DisconnectReason &reason);
	void OnLevelChange(int clNum);
	void WriteGameStatus(CBuffer &buffer);
	int  NumConfigStringBufs(int stringId) const;
	bool WriteConfigString(CBuffer &buffer, int stringId, int numBuffer=0);

	//Game Interface
/*	int  PointContents(const vector_t &v);
	void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end);
	void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end, 
									 const vector_t &mins, const vector_t &maxs);
*/	
	I_World * GetWorld();
	void ExecCommand(const char * cmd);
	void DebugPrint(const char * msg);
	void FatalError(const char * msg);

	void BroadcastPrintf(const char * msg,...);
	void ClientPrintf(int clNum, const char * msg,...);

	NetChanWriter & GetNetChanWriter();
	void GetMultiCastSet(MultiCastSet &set, MultiCastType type, int clId);
	void GetMultiCastSet(MultiCastSet &set, MultiCastType type, const vector_t &source);
	
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
	void WriteSignOnBuffer();
	
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
	CWorld	*	m_pWorld;
	bool		m_active;
	
	//=================================================
	ServerState		m_svState;

	char			m_printBuffer[512];
	NetSignOnBufs	m_signOnBufs;
	
	CNetServer		m_net;
	NetChanWriter & m_chanWriter;
	MultiCastSet	m_multiCastSet;
	
	//The Game Interface
	HINSTANCE m_hGameDll;
	I_Game *  m_pGame;
	float     m_fGameTime;

	//=================================================
	//CVars
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir

	//=================================================

	//These should just point to the data in the GAME code
	Entity    ** m_entities;
	EntClient ** m_clients;
};

extern CServer * g_pServer;

#endif