#include "Sys_hdr.h"
#include "Net_chan.h"
#include "Net_defs.h"

enum
{
	MAX_BACKUP = 200
};

using namespace VoidNet;

//======================================================================================
//======================================================================================

/*
======================================
Constructor/Destructor
======================================
*/
CNetChan::CNetChan() : m_buffer(MAX_DATAGRAM_SIZE),
					   m_reliableBuffer(MAX_DATAGRAM_SIZE),
					   m_sendBuffer(MAX_DATAGRAM_SIZE)
{	
	Reset();
	m_pRecvBuffer = 0;
}

CNetChan::~CNetChan()
{	m_pRecvBuffer =0;
}


/*
======================================
Setup the NetChannel for listening
======================================
*/
void CNetChan::Setup(const CNetAddr &addr, CBuffer * recvBuffer )
{
	Reset();
	m_addr = addr;
	m_pRecvBuffer = recvBuffer;

	m_outMsgId = 1;
	m_bInReliableAcked = 1;
}

/*
======================================
Reset the Network Channel
======================================
*/
void CNetChan::Reset()
{
	m_addr.Reset();

	m_buffer.Reset();
	m_reliableBuffer.Reset();
	m_sendBuffer.Reset();
	
	m_inMsgId=0;				//Latest incoming messageId
	m_inAckedMsgId=0;			//Latest remotely acked message.
	m_outMsgId=0;				//Outgoing messageId
	m_lastOutReliableMsgId=0;	//Id of the last reliable message sent

	m_bOutReliableMsg = 0;
	m_bInReliableMsg=0;			//Is the message recived supposed to be reliable ?
	m_bInReliableAcked=0;		//Was the last reliabled message acked by the remote host ?

	m_dropCount = m_goodCount= 0;
	m_numChokes = 0;

	m_clearTime = 0.0f;
	m_rate = 0.0f;

	m_bFatalError = false;
}


/*
======================================
True if the bandwidth choke isn't active
======================================
*/
bool CNetChan::CanSend() 
{
	if (m_clearTime < System::g_fcurTime + MAX_BACKUP * m_rate)
		return true;
	m_numChokes ++;
	return false;
}

/*
======================================
Can we safely write to the reliable buffer ?
======================================
*/
bool CNetChan::CanSendReliable() 
{
	// waiting for ack
	if (m_reliableBuffer.GetSize())
		return false;			
	return CanSend();
}


void CNetChan::SetRate(int rate)
{
	if(rate < 1000 || rate > 10000)
		rate = 2500;
	m_rate = 1.0/rate;
}


/*
===============
Tries to send an unreliable message to a connection, and handles the
transmition / retransmition of the reliable messages.
A 0 length will still generate a packet and deal with the reliable messages.
================
*/
void CNetChan::PrepareTransmit()
{
	if(m_buffer.OverFlowed()) // || m_reliableBuffer.OverFlowed())
	{
		m_bFatalError = true;
		ComPrintf("%s: Outgoing message overflow\n", m_addr.ToString());
		return;
	}

	int	send_reliable = 0;

	// the remote side dropped the last reliable message we sent, resend it
	if((m_inAckedMsgId > m_lastOutReliableMsgId) &&	
	   (m_bInReliableAcked != m_bOutReliableMsg))
	{
	   send_reliable = 1;
	}
	
	//if the reliable transmit buffer is empty, then copy unreliable contents to it
	if (!m_reliableBuffer.GetSize() && m_buffer.GetSize())
	{
		m_reliableBuffer.Reset();
		m_reliableBuffer.Write(m_buffer);
		m_buffer.Reset();
		send_reliable = 1;
	}

	//write packet header
	//Outgoing sequence num. add reliable bit if needed
	int h1 = m_outMsgId | (send_reliable << 31);	
	//Ack the last received message, add a reliable bit, if it was reliable
	int h2 = m_inMsgId  | (m_bInReliableMsg << 31); 

	m_sendBuffer.Reset();
	m_sendBuffer.Write(h1);
	m_sendBuffer.Write(h2);

	// copy the reliable message to the packet first
	if(send_reliable)
	{
//ComPrintf("CNetChan: Sending reliably\n");
		m_sendBuffer.Write(m_reliableBuffer);
		m_lastOutReliableMsgId = m_outMsgId;
		m_bOutReliableMsg = 1;
		//m_bInReliableMsg = 1;
	}
	
	//Add unreliable message if it has space
	if(m_buffer.GetSize() &&
	  (m_sendBuffer.GetSize() + m_buffer.GetSize() > m_sendBuffer.GetMaxSize()))
		m_sendBuffer.Write(m_buffer);

	//increment outgoing sequence
	m_outMsgId++;

	if (m_clearTime < System::g_fcurTime)
		m_clearTime = System::g_fcurTime + (m_sendBuffer.GetSize() * (m_rate));
	else
		m_clearTime += (m_sendBuffer.GetSize() * (m_rate));
}


/*
======================================
Read Packet header
======================================
*/
bool CNetChan::BeginRead()
{
	uint seq = m_pRecvBuffer->ReadInt();
	uint seqacked = m_pRecvBuffer->ReadInt();
	
	uint bReliable	    = (seq >> 31);
	uint bReliableAcked = (seqacked >> 31);

	//get rid of high bits
	seq &= ~(1<<31);	
	seqacked &= ~(1<<31);	

	//Message is a duplicate or old, ignore it
	if (seq <= m_inMsgId)
	{
//		ComPrintf("CNetChan: dropping old/dup packet, %d <= %d\n", seq, m_inMsgId);
		return false;
	}

	//A message was never reccived. This one is more than just the next one in the sequence
	if(seq > (m_inMsgId+1))
		m_dropCount += 1;

	//If the last reliable message we sent has been acknowledged
	//then clear the buffer to make way for the next
	if ((bReliableAcked == m_bOutReliableMsg) ||
		(seqacked > m_lastOutReliableMsgId))
	{
		m_reliableBuffer.Reset();
		m_bOutReliableMsg = 0;
	}
	
	//Update sequence numbers. set reliable flags if this was sent reliably
	m_inMsgId = seq;
	m_inAckedMsgId = seqacked;
	
	//We have been sent a reliable message which will need to be acked.accordingly
	m_bInReliableMsg = bReliable;
	m_bInReliableAcked = bReliableAcked;

	//Update stats
	m_goodCount ++;
	m_lastReceived = System::g_fcurTime;
	return true;
}


void CNetChan::PrintStats() const
{
	ComPrintf("Rate %.2f: Chokes %d\nGoodcount %d Dropped %d\n", 1/m_rate, m_numChokes, m_goodCount, m_dropCount);
	ComPrintf("In:%d  InAcked:%d Out:%d\n", m_inMsgId, m_inAckedMsgId, m_outMsgId);
}

//also check Port here ?
bool CNetChan::MatchAddr(const CNetAddr &addr) const
{	return (addr == m_addr);
}

const char * CNetChan::GetAddrString() const
{	return m_addr.ToString();
}