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

	void Setup(const CNetAddr &addr, CBuffer * recvBuffer);
	void Reset();

	bool CanSend();
	bool CanSendReliable();
	
	//We have written to the sendbuffer, 
	//now we would like to prepare it for sending
	void PrepareTransmit();

	//Just got a message. start by reading the id headers
	bool BeginRead();

	void SetRate(int rate);

	//The address the channel will send to
	CNetAddr	m_addr;

	//This is what the client writes to send to the server
	//and the server uses this to send/recv unreliable messages to the client
	CBuffer	m_buffer;
	
	//the channel writes data to here for sending
	CBuffer	m_sendBuffer;

	//ptr to receiving sockets buffer
	CBuffer *m_pRecvBuffer;
	CBuffer  m_reliableBuffer;	//Internal, keep reliable messages for retransmit

	uint	m_inMsgId;				//Latest incoming messageId
	uint	m_inAckedMsgId;			//Latest remotely acked message.
	uint	m_outMsgId;				//Outgoing messageId
	uint	m_lastOutReliableMsgId;	//Id of the last reliable message sent

	int		m_bInReliableMsg;		//Is the message recived supposed to be reliable ?
	int		m_bInReliableAcked;		//Was the last reliabled message acked by the remote host ?

	int		m_port;					//Client port

	float	m_lastReceived;
	
	//Stats
	int		m_dropCount;
	int		m_goodCount;
	int		m_numChokes;

	double	m_clearTime;
	double	m_rate;					//Byte/Sec
	
	bool	m_bFatalError;
};

#endif