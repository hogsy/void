#ifndef VOID_CLIENT_NETHANDLER
#define VOID_CLIENT_NETHANDLER

#include "Net_sock.h"
#include "Net_defs.h"
#include "I_client.h"

/*
======================================
Responsible for handling all 
network communication for the client
======================================
*/
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
	CNetSocket  m_sock;
	
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
