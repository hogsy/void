#ifndef  VOID_NET_SERVER
#define  VOID_NET_SERVER

#include "Com_buffer.h"

/*
======================================
These should be called by the main application
======================================
*/
bool InitNetwork();
void ShutdownNetwork();


/*
======================================
Server State Struct
======================================
*/
struct ServerState
{
	ServerState()
	{
		port = 0;
		maxClients = numClients = levelId = 0;
		memset(hostName,0,sizeof(hostName));
		memset(gameName,0,sizeof(gameName));
		memset(worldname,0,sizeof(worldname));
		memset(localAddr,0,sizeof(localAddr));
	}

	int		maxClients;		//Max num of clients
	int		numClients;		//Current num of clients
	
	char	worldname[32];	//Map name
	int		levelId;		//Current map id
	
	char	localAddr[24];	//Server address
	short	port;			//Server port num
	
	char	hostName[32];	//Server name
	char	gameName[32];	//Game name
};

/*
======================================
The NETWORK server.
Engine should subclass this
======================================
*/
class CNetSocket;
class SVClient;

class CNetServer
{
public:
	CNetServer();
	virtual ~CNetServer();

protected:

	enum
	{
		MAX_SIGNONBUFFERS = 8,
		MAX_CHALLENGES =  512,
		MAX_CLIENTS = 16
	};

	bool NetInit();
	void NetShutdown();
	void NetRestart();

	//Called each server frame
	void ReadPackets();
	void WritePackets();

	//Misc
	void NetPrintStatus();

	//Handle Connectionless requests
	void HandleStatusReq();
	void HandleConnectReq();
	void HandleChallengeReq();

	//Parse message received
	void ProcessQueryPacket();					//Query packet
	void ParseClientMessage(SVClient &client);	//Client is in game
	void ParseSpawnMessage(SVClient &client);	//Client hasn't spawned yet

	//Broadcast print message to all except
	void ClientPrintf(SVClient &client, const char * message, ...);
	void BroadcastPrintf(const SVClient * client, const char* message, ...);
	
	//Send Info to client
	void SendRejectMsg(const char * reason);	//Only for unconnected clients
	void SendSpawnParms(SVClient &client);
	void SendReconnect(SVClient &client);
	void SendDisconnect(SVClient &client, const char * reason);

	//===================================================

	//Server Statue
	ServerState m_svState;

	//Total clients
	SVClient *	m_clients; //[MAX_CLIENTS];

	//Use to stores Entity baselines etc which
	//are transmitted to the client on connection
	int			m_numSignOnBuffers;
	CBuffer		m_signOnBuf[MAX_SIGNONBUFFERS];

private:

/*
	struct NetChallenge;
	{
		NetChallenge()	{ challenge = 0;	time = 0.0f;  }
		CNetAddr	addr;
		int			challenge;
		float		time;
	};
	
	NetChallenge  m_challenges[MAX_CHALLENGES];
*/

	struct NetChallenge;
	NetChallenge  * m_challenges;


	//private data
	CNetSocket* m_pSock;

	CBuffer		m_recvBuf;
	CBuffer		m_sendBuf;
	
	char		m_printBuffer[512];
};


#endif