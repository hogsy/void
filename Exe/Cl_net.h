#ifndef VOID_CLIENT_NETHANDLER
#define VOID_CLIENT_NETHANDLER

#include "Net_sock.h"
#include "Net_defs.h"
#include "Cl_main.h"

/*
======================================
Responsible for handling all 
network communication for the client
======================================
*/
class CClientNetHandler
{
public:
	CClientNetHandler(CClient &owner);
	~CClientNetHandler();

	//Read any waiting packets. 
	//should be the first thing in a client frame
	void ReadPackets();
	
	//Send any updates, 
	//should be the last thing in a client frame
	void SendUpdates();

	void ConnectTo(const char * ipaddr);
	void Disconnect(bool serverPrompted = false);
	void Reconnect();

	//Send talk message
	//Validate message first
	void SendTalkMsg(const char * string);

	//UserInfo update funcs
	//These values should be validate by the client first
	void UpdateName(const char *name);	//Should be non null
	void UpdateRate(int rate);			//Should be b/w 1000 and 30000

	//Client needs access to netchan for statistics
	const CNetChan & GetChan() const { return m_netChan; }

private:

	void HandleSpawnParms();
	void HandleOOBMessage();

	//Send a connection request wtih a challenge number
	void SendChallengeReq();
	void SendConnectReq();

	CBuffer		m_buffer;
	CNetChan	m_netChan;
	CNetSocket  m_sock;
	
	char		m_szServerAddr[24];
	bool		m_bLocalServer;
	
	int			m_levelId;
	int			m_challenge;

	//Flow Control for an Unspawned client
	float		m_fNextSendTime;	//Next send time
	int			m_numResends;		//Max number of resends
	const char* m_szLastOOBMsg;		//Keep Track of the last OOB message sent
	
	bool		m_bCanSend;
	byte		m_spawnState;
	int			m_netState;

	bool		m_bInitialized;

	CClient &	m_refClient;
};

#endif
