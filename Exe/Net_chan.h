#ifndef VOID_NET_CHANNEL
#define VOID_NET_CHANNEL

#include "Net_util.h"
#include "Com_buffer.h"

/*
======================================
Network channel
manages synchronization and combines
reliable/unreliable data streams
======================================
*/
class CNetChan
{
public:

	CNetChan();
	~CNetChan();

	//Management
	void Setup(const CNetAddr &addr, CBuffer * recvBuffer);
	void Reset();
	
	void SetRate(int rate);	
	void PrintStats() const;
	bool MatchAddr(const CNetAddr &addr) const;
	const char * GetAddrString() const;

	//Check status
	bool CanSend();
	bool CanSendReliable();

	//Prepere channel for transmission
	void PrepareTransmit();
	//Just got a message. start by reading the id headers
	bool BeginRead();

	//================================================================
	
	CBuffer	m_buffer;				//Write to this buffer for transmission

	float	m_lastReceived;
	
	uint	m_inMsgId;				//Latest incoming messageId
	uint	m_inAckedMsgId;			//Latest remotely acked message.
	uint	m_outMsgId;				//Outgoing messageId
	uint	m_lastOutReliableMsgId;	//Id of the last reliable message sent

	//Stats
	int		m_dropCount;
	int		m_goodCount;
	int		m_numChokes;

	bool	m_bFatalError;

private:

	friend class CNetSocket;

	double	m_clearTime;
	double	m_rate;					//Byte/Sec

	int		m_bInReliableMsg;		//Is the message recived supposed to be reliable ?
	int		m_bInReliableAcked;		//Was the last reliabled message acked by the remote host ?
	int		m_bOutReliableMsg;		//Did we send a reliable message

	CNetAddr  m_addr;				//Client addr
	int		  m_vPort;				//Client vport

	CBuffer	  m_sendBuffer;			//the channel writes data to here for sending
	CBuffer   m_reliableBuffer;		//Internal, keep reliable messages for retransmit
	CBuffer * m_pRecvBuffer;		//ptr to receiving sockets buffer
};

#endif