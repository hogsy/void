#ifndef VOID_CLIENT_NETHANDLER
#define VOID_CLIENT_NETHANDLER

#include "Com_buffer.h"
#include "Net_defs.h"

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
	virtual void HandleSpawnMsg(const byte &msgId, CBuffer &buffer)=0; 
	
	//Handle disconnect from server
	virtual void HandleDisconnect(bool listenserver)=0;
	
	//Writes the Game clients userinfo
	virtual void WriteUserInfo(CBuffer &buffer)=0;

	//Client print func. Route this to the console/Client area. whatever
	virtual void Print(const char * msg, ...)=0;
};


//Internal Class declarations
namespace VoidNet
{
	class CNetSocket;
	class CNetChan;
}

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

	bool CanSend();
	CBuffer & GetSendBuffer();			//Access message buffer

	bool CanSendReliable();
	CBuffer & GetReliableBuffer();		//Access reliable buffer

	void ConnectTo(const char * ipaddr);
	void Disconnect(bool serverPrompted);
	void Reconnect();

	void SetPort(short port);
	void SetRate(int rate);
	
	const NetChanState & GetChanState() const;

private:

	void HandleSpawnParms();
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
	float	m_fNextSendTime;	//Next send time
	const char* m_szLastOOBMsg;	//Keep Track of the last OOB message sent

	I_NetClientHandler  * m_pClient;
	
	VoidNet::CNetChan   * m_pNetChan;
	VoidNet::CNetSocket * m_pSock;

	CBuffer	m_buffer;
	CBuffer	m_reliableBuf;
};

#endif