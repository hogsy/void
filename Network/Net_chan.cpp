#include "Net_hdr.h"
#include "Net_chan.h"

enum
{	MAX_BACKUP = 200
};

using namespace VoidNet;

/*
======================================
Constructor/Destructor
======================================
*/
CNetChan::CNetChan() : m_buffer(NET_MAXDATAGRAMSIZE),
					   m_reliableBuffer(NET_MAXDATAGRAMSIZE),
					   m_sendBuffer(NET_MAXDATAGRAMSIZE),
					   m_pRecvBuffer(0)
{	Reset();
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

	m_state.outMsgId = 1;
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

	m_state.Reset();
	
	m_rate = 0.0;

	m_lastReceived = System::GetCurTime();
	m_clearTime = 0.0f;
	
	m_bOutReliableMsg = 0;
	m_bInReliableMsg=0;			//Is the message recived supposed to be reliable ?
	m_bInReliableAcked=0;		//Was the last reliabled message acked by the remote host ?

	m_bFatalError = false;
}


/*
======================================
True if the bandwidth choke isn't active
======================================
*/
bool CNetChan::CanSend() 
{
	if (m_clearTime < System::GetCurTime() + MAX_BACKUP * m_rate)
		return true;
	m_state.numChokes ++;
	return false;
}

/*
======================================
Can we safely write to the reliable m_buffer ?
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
	if(rate < 100 || rate > 10000)
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
	if((m_state.inAckedId > m_state.lastOutReliableId) &&	
	   (m_bInReliableAcked != m_bOutReliableMsg))
	{
	   send_reliable = 1;
	}
	
	//if the reliable transmit m_buffer is empty, then copy unreliable contents to it
	if (!m_reliableBuffer.GetSize() && m_buffer.GetSize())
	{
		m_reliableBuffer.Reset();
		m_reliableBuffer.WriteBuffer(m_buffer);
		m_buffer.Reset();
		send_reliable = 1;
	}

	//write packet header
	//Outgoing sequence num. add reliable bit if needed
	int h1 = m_state.outMsgId | (send_reliable << 31);	
	//Ack the last received message, add a reliable bit, if it was reliable
	int h2 = m_state.inMsgId  | (m_bInReliableMsg << 31); 

	m_sendBuffer.Reset();
	m_sendBuffer.WriteInt(h1);
	m_sendBuffer.WriteInt(h2);

	// copy the reliable message to the packet first
	if(send_reliable)
	{
//ComPrintf("CNetChan: Sending reliably\n");
		m_sendBuffer.WriteBuffer(m_reliableBuffer);
		m_state.lastOutReliableId = m_state.outMsgId;
		m_bOutReliableMsg = 1;
		//m_bInReliableMsg = 1;
	}
	
	//Add unreliable message if it has space
	if(m_buffer.GetSize() &&
	  ((m_sendBuffer.GetSize() + m_buffer.GetSize()) < m_sendBuffer.GetMaxSize()))
		m_sendBuffer.WriteBuffer(m_buffer);

	//increment outgoing sequence
	m_state.outMsgId++;

	if (m_clearTime < System::GetCurTime())
		m_clearTime = System::GetCurTime() + (m_sendBuffer.GetSize() * m_rate);
	else
		m_clearTime += (m_sendBuffer.GetSize() * m_rate);
	m_sendTime = System::GetCurTime();
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

	if(seqacked == m_state.outMsgId -1)
		m_state.latency = System::GetCurTime() - m_sendTime;

	//Message is a duplicate or old, ignore it
	if (seq <= m_state.inMsgId)
	{
//		ComPrintf("CNetChan: dropping old/dup packet, %d <= %d\n", seq, m_state.inMsgId);
		return false;
	}

	//A message was never reccived. This one is more than just the next one in the sequence
	if(seq > (m_state.inMsgId+1))
		m_state.dropCount += 1;

	//If the last reliable message we sent has been acknowledged
	//then clear the m_buffer to make way for the next
	if ((bReliableAcked == m_bOutReliableMsg) ||
		(seqacked > m_state.lastOutReliableId))
	{
		m_reliableBuffer.Reset();
		m_bOutReliableMsg = 0;
	}

	
	//Update sequence numbers. set reliable flags if this was sent reliably
	m_state.inMsgId = seq;
	m_state.inAckedId = seqacked;
	
	//We have been sent a reliable message which will need to be acked.accordingly
	m_bInReliableMsg = bReliable;
	m_bInReliableAcked = bReliableAcked;

	//Update stats
	m_state.goodCount ++;
	m_lastReceived = System::GetCurTime();
	return true;
}


/*
======================================
Misc Util
======================================
*/
//also check Port here ?
bool CNetChan::MatchAddr(const CNetAddr &addr) const
{	
	return (addr == m_addr);
}

const char * CNetChan::GetAddrString() const
{	return m_addr.ToString();
}

const CNetAddr & CNetChan::GetAddr() const
{	return m_addr;
}

int CNetChan::GetRate() const
{	return (1.0/m_rate);
}