#ifndef VOID_NETWORK_SERVER
#define VOID_NETWORK_SERVER

#include "Sys_hdr.h"
#include "World.h"
#include "Com_buffer.h"
#include "Net_defs.h"

/*
==========================================
Public defs
==========================================
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

	//HACK
	CServer();
	~CServer();

	void RunFrame();

	bool HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs);
	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);

private:

	enum
	{
		MAX_SIGNONBUFFERS = 8,
		MAX_CHALLENGES =  512
	};

	struct NetChallenge;
	NetChallenge * m_challenges;

	//private data
	VoidNet::CNetSocket* m_pSock;
	
	CNetBuffer  m_recvBuf;
	CNetBuffer  m_sendBuf;

	//world data currently loaded by the server.
	world_t	* m_pWorld;
	char	m_worldName[COM_MAXPATH];

	bool	m_active;
	int		m_numClients;

	//Stores Entity baselines etc
	int		m_numSignOnBuffers;
	int		m_signOnBufSize[MAX_SIGNONBUFFERS];
	char	m_szSignOnBuf[MAX_SIGNONBUFFERS][VoidNet::MAX_DATAGRAM];

	//=================================================
	//These arent called by the Sysmain on startup
	//The server is only initialized if/when needed
	bool Init();
	void Shutdown();

	void LoadWorld(const char * mapname);

	//Send Info to client
	void SendRejectMsg(const char * reason);

	//Handle Connectionless requests
	void HandleStatusReq();
	void HandleConnectReq();
	void HandleChallengeReq();

	//FRAME proceedures
	void ProcessQueryPacket();
	void ReadPackets();

	//=================================================
	//CVars
	CVar	m_cDedicated;
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir
};

#endif