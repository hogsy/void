#ifndef VOID_NETWORK_SERVER
#define VOID_NETWORK_SERVER

#include "Sys_hdr.h"
#include "World.h"
#include "Com_buffer.h"
#include "Sv_client.h"

/*
======================================
Expose to system
======================================
*/
namespace VoidNet
{
	bool InitNetwork();
	void ShutdownNetwork();
}



/*
==========================================
Game independent network server
==========================================
*/
class CNetSocket;

class CServer : public I_ConHandler 
{
public:
	CServer();
	~CServer();

	void RunFrame();

	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	//world data currently loaded by the server.
	world_t	*	m_pWorld;
	char		m_worldName[COM_MAXPATH];

	//Net specfic

	enum
	{
		MAX_SIGNONBUFFERS = 8,
		MAX_CHALLENGES =  512,
		MAX_CLIENTS = 16
	};

	struct NetChallenge
	{
		NetChallenge()	{ challenge = 0;	time = 0.0f;  }
		CNetAddr	addr;
		int			challenge;
		float		time;
	};
	
	NetChallenge  m_challenges[MAX_CHALLENGES];

	//private data
	CNetSocket* m_pSock;
	
	CBuffer		m_recvBuf;
	CBuffer		m_sendBuf;

	SVClient	m_clients[MAX_CLIENTS];

	bool		m_active;
	int			m_numClients;

	//to check for map changes while clients are connecting
	int			m_levelNum;		

	//Stores Entity baselines etc
	int			m_numSignOnBuffers;
	CBuffer		m_signOnBuf[MAX_SIGNONBUFFERS];

	char		m_printBuffer[512];

	//=================================================
	//These arent called by the Sysmain on startup
	//The server is only initialized if/when needed
	bool Init();
	void Shutdown();
	void Restart();

	void LoadWorld(const char * mapname);

	//Handle Connectionless requests
	void HandleStatusReq();
	void HandleConnectReq();
	void HandleChallengeReq();

	//Parse message received
	void ProcessQueryPacket();					//Query packet
	void ParseClientMessage(SVClient &client);	//Client is in game
	void ParseSpawnMessage(SVClient &client);	//Client hasn't spawned yet

	//Called each server frame
	void ReadPackets();
	void RunEntities();
	void WritePackets();

	//Broadcast print message to all except
	void ClientPrintf(SVClient &client, const char * message, ...);
	void BroadcastPrintf(const SVClient * client, const char* message, ...);
	
	//Send Info to client
	void SendRejectMsg(const char * reason);	//Only for unconnected clients
	void SendSpawnParms(SVClient &client);
	void SendReconnect(SVClient &client);
	void SendDisconnect(SVClient &client, const char * reason);

	//Misc
	void PrintServerStatus();

	//=================================================
	//CVars
	CVar	m_cDedicated;
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir
};

#endif