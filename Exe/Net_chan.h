#ifndef VOID_NET_CHANNEL
#define VOID_NET_CHANNEL

#include "Net_sock.h"

/*
======================================
Network channel
======================================
*/
class CNetChan
{
public:

	CNetChan();
	~CNetChan();

	bool CanSend();
	bool CanSendReliable();
	
	void Transmit(int length, byte *data);
	bool Receive();

private:

	VoidNet::CNetAddr	m_addr;

	bool		m_bFatalError;
	
	int			m_port;				//Client port

	CNetBuffer	m_sockBuffer;	//used internally to send/receive data from the socket

	//This is what the client writes to send to the server
	//and the server uses this to send/recv unreliable messages to the client
	CNetBuffer	m_buffer;
	int			m_inSeq;	//every unrelabled packet has an id
	int			m_inAcked;	//don't need to ack on every unreliable msg
	int			m_outSeq;	//used by server mostly.

	CNetBuffer  m_reliableBuffer;
	int			m_lastReliableSeq;		// sequence number of last send

	//true/false bits
	int			m_bInReliableSeq;	
	int			m_bInReliableAcked;
	int			m_bReliableSeq;

		//Stats
	int			m_dropCount;
	int			m_goodCount;

	double		m_clearTime;
	double		m_rate;			// Seconds/Byte

};

#endif