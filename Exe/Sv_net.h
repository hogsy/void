#include VNETSV
#define  VNETSV

#if 0

/*
======================================
The NETWORK server.
Engine should subclass this
======================================
*/

class CNetServer
{
public:
	CNetServer();
	virtual ~CNetServer();

protected:

	SVClient	m_clients[MAX_CLIENTS];

	bool		m_active;
	int			m_numClients;

	//to check for map changes while clients are connecting
	int			m_levelNum;		

	//Stores Entity baselines etc
	int			m_numSignOnBuffers;
	CBuffer		m_signOnBuf[MAX_SIGNONBUFFERS];


	bool NetInit();
	void NetShutdown();
	void NetRestart();

	//Called each server frame
	void ReadPackets();
	void WritePackets();

	//Misc
	void PrintServerStatus();

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

private:

	CBuffer		m_recvBuf;
	CBuffer		m_sendBuf;

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

	char		m_printBuffer[512];
};


#endif
#endif