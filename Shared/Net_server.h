#ifndef  VOID_NETWORK_SERVER
#define  VOID_NETWORK_SERVER

#include "Com_buffer.h"
#include "Net_defs.h"

//Predeclarations
namespace VoidNet 
{	
	class CNetSocket;
	class CNetChan;
}

/*
======================================
Server State Struct
======================================
*/
struct ServerState
{
	ServerState()
	{	port = 0;
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
The "Game" Server needs to be able
to write directly to client channels.
======================================
*/
class CClientChan
{
public:
	CClientChan();
	virtual ~CClientChan();

	void BeginMessage(byte msgid, int estSize);
	void WriteByte(byte b);
	void WriteChar(char c);
	void WriteShort(short s);
	void WriteInt(int i);
	void WriteFloat(float f);
	void WriteCoord(float c);
	void WriteAngle(float a);
	void WriteString(const char *string);
	void WriteData(byte * data, int len);

	const NetChanState & GetChanState() const;

private:
	
	friend class CNetServer;
	enum
	{	MAX_BACKBUFFERS = 4
	};

	void MakeSpace(int maxsize);
	void ValidateBuffer();
	void Reset();
	bool ReadyToSend();

	VoidNet::CNetChan *	m_pNetChan;

	//Flags and States
	bool	m_bDropClient;	//drop client if this is true
	bool	m_bSend;

	//keep track of how many spawn messages have been sent
	//when it equals SVC_BEGIN, then the client will be assumed to have spawned
	byte	m_spawnState;
	int		m_state;

	//back buffers for client reliable data
	bool	m_bBackbuf;
	int		m_numBuf;
	CBuffer	m_backBuffer[MAX_BACKBUFFERS];

	//Game specifc
	int		m_id;
	char	m_name[32];
};


//callback interface for game server
struct I_NetServerHandler
{
	//process client message
	virtual void HandleClientMsg(int clientNum, CBuffer &buffer)=0;
	
	//notify client connection
	//will return a clientId. -1 if connection failed for some reason
	virtual int HandleClientConnect(CBuffer &buffer)=0;
	
	//notify client disconnection
	virtual void HandleClientDrop(int clientNum)=0;

	//validate connection
	virtual void ValidateConnRequest(CBuffer &buffer)=0;
};




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
	void ProcessQueryPacket();						//Query packet
	void ParseClientMessage(CClientChan &client);	//Client is in game
	void ParseSpawnMessage(CClientChan &client);	//Client hasn't spawned yet

	//Broadcast print message to all except
	void ClientPrintf(CClientChan &client, const char * message, ...);
	void BroadcastPrintf(const CClientChan * client, const char* message, ...);
	
	//Send Info to client
	void SendRejectMsg(const char * reason);		//Only for unconnected clients
	void SendSpawnParms(CClientChan &client);
	void SendReconnect(CClientChan &client);
	void SendDisconnect(CClientChan &client, const char * reason);

	//===================================================
	//Server Statue
	ServerState m_svState;

	//Game clients
	CClientChan	m_clChan[MAX_CLIENTS];

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