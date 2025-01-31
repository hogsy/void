#ifndef VOID_CLIENT_NETHANDLER
#define VOID_CLIENT_NETHANDLER

/*
======================================================================================
The game client will have to implement this interface
so that the Network Client can do callbacks to it
======================================================================================
*/
struct I_NetClientHandler
{
	//Parse and handle a game message
	virtual void HandleGameMsg(CBuffer &buffer)=0; 
	
	//Parse and handle spawm parms
	virtual void HandleSpawnMsg(byte msgId, CBuffer &buffer)=0; 
	
	//Handle disconnect from server
	virtual void HandleDisconnect()=0;

	//Writes the Game clients userinfo
	virtual void WriteUserInfo(CBuffer &buffer)=0;

	//Put Client in game. The clNum is the clients num on the server
	virtual void BeginGame(int clNum, CBuffer &buffer)=0;

	//Client print func. Route this to the console/Client area. whatever
	virtual void Print(const char * msg, ...)=0;
};

//Internal Class declarations
namespace VoidNet
{
	class CNetSocket;
	class CNetChan;
}

struct NetChanState;

/*
======================================================================================
Responsible for handling all network communication for the client
======================================================================================
*/
class CNetClient
{
public:
	CNetClient(I_NetClientHandler * client);
	~CNetClient();

	//Read any waiting packets. 
	//should be the first thing in a client frame
	void ReadPackets();
	
	//Send any updates, 
	//should be the last thing in a client frame
	void SendUpdate();

	//Access outgoing message buffer
	bool CanSend();
	CBuffer & GetSendBuffer();			

	//Access outgoing reliable buffer
	bool CanSendReliable();
	CBuffer & GetReliableBuffer();		

	void ConnectTo(const char * ipaddr);
	void Disconnect(bool serverPrompted);
	void Reconnect(bool serverPrompted);

	void SetPort(short port);
	void SetRate(int rate);
	
	bool IsLocalServer() const;
	const NetChanState & GetChanState() const;

private:

	void HandleSpawnStrings();
	void HandleOOBMessage();

	//Send a connection request wtih a challenge number
	void SendChallengeReq();
	void SendConnectReq();

	char	m_szServerAddr[24];
	bool	m_bLocalServer;
	
	int		m_levelId;
	int		m_challenge;

	//Spawning Info
	byte	m_spawnLevel;
	int		m_spawnNextPacket;

	int		m_netState;

	//Flow Control for an Unspawned client
	float	m_fNextSendTime;
	const char* m_szLastOOBMsg;

	I_NetClientHandler  * m_pClient;
	
	VoidNet::CNetChan   * m_pNetChan;
	VoidNet::CNetSocket * m_pSock;

	CBuffer	m_buffer;
	CBuffer	m_reliableBuf;
};

#endif