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

	void Setup(const CNetAddr &addr, CNetBuffer * recvBuffer);
	void Reset();

	bool CanSend();
	bool CanSendReliable();
	
	void Write(int length, byte *data);
	bool Read();

	//The address the channel will send to
	CNetAddr	m_addr;

	//This is what the client writes to send to the server
	//and the server uses this to send/recv unreliable messages to the client
	CNetBuffer	m_buffer;

private:

	//the channel writes data to here for sending
	CNetBuffer	m_sendBuffer;	

	//ptr to receiving sockets buffer
	CNetBuffer *m_pRecvBuffer;
	CNetBuffer  m_reliableBuffer;

	int	m_inMsgId;				//Latest incoming messageId
	int m_inAckedMsgId;			//Latest remotely acked message.
	int m_outMsgId;				//Outgoing messageId
	int m_lastOutReliableMsgId;	//Id of the last reliable message sent

	int m_bInReliableMsg;		//Is the message recived supposed to be reliable ?
	int m_bInReliableAcked;		//Was the last reliabled message acked by the remote host ?

	int	m_port;					//Client port
	
	//Stats
	int	m_dropCount;
	int	m_goodCount;
	int m_numChokes;

	double	m_clearTime;
	double  m_lastReceived;
	double	m_rate;				//Seconds/Byte

	bool m_bFatalError;
};

#endif