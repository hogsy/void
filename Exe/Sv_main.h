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
	class CNetSocket;

	bool InitNetwork();
	void ShutdownNetwork();
}

/*
==========================================
Game independent network server
==========================================
*/
class CServer : public I_CVarHandler,
				public I_CmdHandler
{
public:
	CServer();
	~CServer();

	void RunFrame();

	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

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
	VoidNet::CNetSocket* m_pSock;
	
	CNetBuffer  m_recvBuf;
	CNetBuffer  m_sendBuf;

	SVClient    m_clients[MAX_CLIENTS];

	//world data currently loaded by the server.
	world_t	* m_pWorld;
	char	m_worldName[COM_MAXPATH];

	bool	m_active;
	int		m_numClients;

	int		m_levelNum;		//to check for map changes when clients are connecting

	//Stores Entity baselines etc
	int			m_numSignOnBuffers;
	CNetBuffer	m_signOnBuf[MAX_SIGNONBUFFERS];

	//=================================================
	//These arent called by the Sysmain on startup
	//The server is only initialized if/when needed
	bool Init();
	void Shutdown();
	void Restart();

	void LoadWorld(const char * mapname);

	//Send Info to client
	void SendRejectMsg(const char * reason);

	//Handle Connectionless requests
	void HandleStatusReq();
	void HandleConnectReq();
	void HandleChallengeReq();

	void ParseClientMessage(SVClient &client);

	//FRAME proceedures
	void ProcessQueryPacket();
	void ReadPackets();
	void WritePackets();

	//Broadcast print message to all except
	void BroadcastPrintf(const SVClient * client, char* message, ...); //const char * message, int msglen);
	void PrintServerStatus();
	
	void SendSpawnParms(SVClient &client);
	void SendReconnect(SVClient &client);
	void SendDisconnect(SVClient &client, const char * reason);

	//=================================================
	//CVars
	CVar	m_cDedicated;
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir
};

#endif