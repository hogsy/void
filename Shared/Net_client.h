#ifndef VOID_CLIENT_NETHANDLER
#define VOID_CLIENT_NETHANDLER

#include "Net_defs.h"
//#include "Net_chan.h"
#include "Com_buffer.h"

//======================================================================================
//======================================================================================

enum ClMsgType
{
	CLMSG_DEFAULT,
	CLMSG_SERVER,
	CLMSG_TALK
};

struct ClUserInfo
{
	ClUserInfo() { strcpy(name,"Player"); rate = 2500; vport = 0; }

	char	name[32];
	int		rate;
	short	vport;
};


/*
======================================
The game client will have to implement this interface
so that the Network Client can do callbacks to it
======================================
*/
struct I_ClientNetHandler
{
	//Client print func
	virtual void Print(ClMsgType type, const char * msg, ...)=0;

	//Parse and handle a game message
	virtual void HandleGameMsg(CBuffer &buffer)=0; 
	
	//Parse and handle spawm parms
	virtual void HandleSpawnMsg(const byte &msgId, CBuffer &buffer)=0; 
	
	//Handle disconnect from server
	virtual void HandleDisconnect(bool listenserver)=0;

	virtual const ClUserInfo & GetUserInfo() const = 0;
};

/*
======================================
Responsible for handling all 
network communication for the client
======================================
*/
class CNetSocket;

class CNetClient
{
public:
	CNetClient(I_ClientNetHandler * client);
	~CNetClient();

	//Read any waiting packets. 
	//should be the first thing in a client frame
	void ReadPackets();
	
	//Send any updates, 
	//should be the last thing in a client frame
	void SendUpdate();

	void ConnectTo(const char * ipaddr);
	void Disconnect(bool serverPrompted = false);
	void Reconnect();

	//Access reliable message buffer
	//this gets resent until it is acknowledged
	CBuffer & GetReliableBuffer() { return m_backBuffer; }

	//Access outgoing message buffer
	CBuffer & GetSendBuffer() { return m_netChan.m_buffer; }
	
	void SetRate(int rate);			

	const CNetChan & GetChan() const { return m_netChan; }

private:

	void HandleSpawnParms();
	void HandleOOBMessage();

	//Send a connection request wtih a challenge number
	void SendChallengeReq();
	void SendConnectReq();

	CBuffer		m_buffer;
	CBuffer		m_backBuffer;

	CNetChan	m_netChan;
	CNetSocket * m_pSock;
	
	char		m_szServerAddr[24];
	bool		m_bLocalServer;
	
	int			m_levelId;
	int			m_challenge;

	//Flow Control for an Unspawned client
	float		m_fNextSendTime;	//Next send time
	int			m_numResends;		//Max number of resends
	const char* m_szLastOOBMsg;		//Keep Track of the last OOB message sent
	
	byte		m_spawnState;
	int			m_netState;

	I_ClientNetHandler * m_pClient;
};

#endif
