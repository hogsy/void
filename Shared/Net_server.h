#ifndef  VOID_NETWORK_SERVER
#define  VOID_NETWORK_SERVER

#include "Com_buffer.h"

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


//Predeclarations
namespace VoidNet 
{	class CNetSocket;
}
//FIXME : this should be changed 
//to a base GameClient so that the game
//code can access is
class SVClient;

/*
======================================
The Server class in the Exe code should
subclass this
======================================
*/
class CNetServer
{
public:
	CNetServer();
	virtual ~CNetServer();

	//Call these on Application Startup/Shutdown
	static bool InitNetwork();
	static void ShutdownNetwork();

protected:

	enum
	{
		MAX_SIGNONBUFFERS = 8,
		MAX_CHALLENGES =  512,
		MAX_CLIENTS = 16
	};

	//Server Init/Shutdown/Restart
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

	//Game clients
	SVClient *	m_clients;

	//Used to stores Entity baselines etc which
	//are transmitted to the client on connection
	int			m_numSignOnBuffers;
	CBuffer		m_signOnBuf[MAX_SIGNONBUFFERS];

private:

	struct NetChallenge;
	NetChallenge  * m_challenges;

	//private data
	VoidNet::CNetSocket* m_pSock;

	CBuffer		m_recvBuf;
	CBuffer		m_sendBuf;
	
	char		m_printBuffer[512];
};

#endif