#include "Net_chan.h"
#include "Sys_hdr.h"

enum
{
	CHAN_BUFFER_SIZE = 1450,
	MAX_BACKUP =200
};

using namespace VoidNet;

//======================================================================================
//======================================================================================

CNetChan::CNetChan() : m_buffer(CHAN_BUFFER_SIZE),
					   m_reliableBuffer(CHAN_BUFFER_SIZE),
					   m_sockBuffer(CHAN_BUFFER_SIZE)
{
	m_inSeq = m_inAcked = m_outSeq= m_lastReliableSeq = 0;
	m_dropCount = m_goodCount= 0;

	m_bInReliableSeq = m_bInReliableAcked = m_bReliableSeq = 0;

	m_clearTime = 0.0;
	m_rate = 0.0;

	m_bFatalError = false;
}

CNetChan::~CNetChan()
{
}



/*
===============
Returns true if the bandwidth choke isn't active
================
*/
bool CNetChan::CanSend()
{
	if (m_clearTime < System::g_fcurTime + MAX_BACKUP * m_rate)
		return true;
	return false;
}

/*
===============
Returns true if the bandwidth choke isn't 
================
*/
bool CNetChan::CanSendReliable()
{
	// waiting for ack
	if (m_reliableBuffer.GetSize())
		return false;			
	return CanSend();
}


/*
===============
tries to send an unreliable message to a connection, and handles the
transmition / retransmition of the reliable messages.
A 0 length will still generate a packet and deal with the reliable messages.
================
*/
void CNetChan::Transmit(int length, byte *data)
{
	if(m_buffer.OverFlowed())
	{
		m_bFatalError = true;
		ComPrintf("%s: Outgoing message overflow\n", m_addr.ToString());
		return;
	}

	int	send_reliable = 0;

	//we got confirmation from the last reliable message we sent
	//and if this message is
	if((m_inAcked > m_lastReliableSeq)	&&	
	   (m_bInReliableAcked != m_bInReliableSeq))
	   send_reliable = 1;

	// if the reliable transmit buffer is empty, copy the current message out
	if (!m_reliableBuffer.GetSize() && m_buffer.GetSize())
	{
		m_reliableBuffer.Reset();
		m_reliableBuffer += m_buffer;
		m_buffer.Reset();
		send_reliable = 1;
	}

	int h1,h2;

	//write packet headers
	
	//Outgoing sequence num. add reliable bit if needed
	h1 = m_outSeq | (send_reliable << 31);	
	//Ack the last received message, add a reliable bit, if it was reliable
	h2 = m_inSeq  | (m_bInReliableSeq << 31); 

	m_sockBuffer.Reset();
	m_sockBuffer += h1;
	m_sockBuffer += h2;

	// copy the reliable message to the packet first
	if(send_reliable)
	{
		m_sockBuffer += m_reliableBuffer;
		m_lastReliableSeq = m_outSeq;
	}

// add the unreliable part if space is available
	if (m_sockBuffer.GetMaxSize() - m_sockBuffer.GetSize() >= length)
		m_sockBuffer.WriteData(data, length);

	//increment outgoing sequence
	m_outSeq++;

	if (m_clearTime < System::g_fcurTime)
		m_clearTime = System::g_fcurTime + m_sockBuffer.GetSize() * m_rate;
	else
		m_clearTime += m_sockBuffer.GetSize() * m_rate;
		
/*
	NET_SendPacket (send.cursize, send.data, chan->remote_address);
	if (showpackets.value)
		Con_Printf ("--> s=%i(%i) a=%i(%i) %i\n"
			, chan->outgoing_sequence
			, send_reliable
			, chan->incoming_sequence
			, chan->incoming_reliable_sequence
			, send.cursize);
*/
}
/*

bool CNetChan::Receive()
{
	unsigned		sequence, sequence_ack;
	unsigned		reliable_ack, reliable_message;

#ifdef SERVERONLY
	int			qport;
#endif
	int i;

	if (
#ifndef SERVERONLY
			!cls.demoplayback && 
#endif
			!NET_CompareAdr (net_from, chan->remote_address))
		return false;
	
// get sequence numbers		
	MSG_BeginReading ();
	sequence = MSG_ReadLong ();
	sequence_ack = MSG_ReadLong ();

	// read the qport if we are a server
#ifdef SERVERONLY
	qport = MSG_ReadShort ();
#endif

	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;

	sequence &= ~(1<<31);	
	sequence_ack &= ~(1<<31);	

	if (showpackets.value)
		Con_Printf ("<-- s=%i(%i) a=%i(%i) %i\n"
			, sequence
			, reliable_message
			, sequence_ack
			, reliable_ack
			, net_message.cursize);

// get a rate estimation
#if 0
	if (chan->outgoing_sequence - sequence_ack < MAX_LATENT)
	{
		int				i;
		double			time, rate;
	
		i = sequence_ack & (MAX_LATENT - 1);
		time = realtime - chan->outgoing_time[i];
		time -= 0.1;	// subtract 100 ms
		if (time <= 0)
		{	// gotta be a digital link for <100 ms ping
			if (chan->rate > 1.0/5000)
				chan->rate = 1.0/5000;
		}
		else
		{
			if (chan->outgoing_size[i] < 512)
			{	// only deal with small messages
rate = chan->outgoing_size[i]/time;
				if (rate > 5000)
					rate = 5000;
				rate = 1.0/rate;
				if (chan->rate > rate)
					chan->rate = rate;
			}
		}
	}
#endif

//
// discard stale or duplicated packets
//
	if (sequence <= (unsigned)chan->incoming_sequence)
	{
		if (showdrop.value)
			Con_Printf ("%s:Out of order packet %i at %i\n"
				, NET_AdrToString (chan->remote_address)
				,  sequence
				, chan->incoming_sequence);
		return false;
	}

//
// dropped packets don't keep the message from being used
//
	net_drop = sequence - (chan->incoming_sequence+1);
	if (net_drop > 0)
	{
		chan->drop_count += 1;

		if (showdrop.value)
			Con_Printf ("%s:Dropped %i packets at %i\n"
			, NET_AdrToString (chan->remote_address)
			, sequence-(chan->incoming_sequence+1)
			, sequence);
	}

//
// if the current outgoing reliable message has been acknowledged
// clear the buffer to make way for the next
//
	if (reliable_ack == (unsigned)chan->reliable_sequence)
		chan->reliable_length = 0;	// it has been received
	
//
// if this message contains a reliable message, bump incoming_reliable_sequence 
//
	chan->incoming_sequence = sequence;
	chan->incoming_acknowledged = sequence_ack;
	chan->incoming_reliable_acknowledged = reliable_ack;
	if (reliable_message)
		chan->incoming_reliable_sequence ^= 1;

//
// the message can now be read from the current message pointer
// update statistics counters
//
	chan->frame_latency = chan->frame_latency*OLD_AVG	+ (chan->outgoing_sequence-sequence_ack)*(1.0-OLD_AVG);
	chan->frame_rate = chan->frame_rate*OLD_AVG	+ (realtime-chan->last_received)*(1.0-OLD_AVG);		
	
	chan->good_count += 1;

	chan->last_received = realtime;

	return true;
}




*/



















#if 0


/*
=================
Netchan_Process

called when the current net_message is from remote_address
modifies net_message so that it points to the packet payload
=================
*/
qboolean Netchan_Process (netchan_t *chan)
{
	unsigned		sequence, sequence_ack;
	unsigned		reliable_ack, reliable_message;

#ifdef SERVERONLY
	int			qport;
#endif
	int i;

	if (
#ifndef SERVERONLY
			!cls.demoplayback && 
#endif
			!NET_CompareAdr (net_from, chan->remote_address))
		return false;
	
// get sequence numbers		
	MSG_BeginReading ();
	sequence = MSG_ReadLong ();
	sequence_ack = MSG_ReadLong ();

	// read the qport if we are a server
#ifdef SERVERONLY
	qport = MSG_ReadShort ();
#endif

	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;

	sequence &= ~(1<<31);	
	sequence_ack &= ~(1<<31);	

	if (showpackets.value)
		Con_Printf ("<-- s=%i(%i) a=%i(%i) %i\n"
			, sequence
			, reliable_message
			, sequence_ack
			, reliable_ack
			, net_message.cursize);

// get a rate estimation
#if 0
	if (chan->outgoing_sequence - sequence_ack < MAX_LATENT)
	{
		int				i;
		double			time, rate;
	
		i = sequence_ack & (MAX_LATENT - 1);
		time = realtime - chan->outgoing_time[i];
		time -= 0.1;	// subtract 100 ms
		if (time <= 0)
		{	// gotta be a digital link for <100 ms ping
			if (chan->rate > 1.0/5000)
				chan->rate = 1.0/5000;
		}
		else
		{
			if (chan->outgoing_size[i] < 512)
			{	// only deal with small messages
rate = chan->outgoing_size[i]/time;
				if (rate > 5000)
					rate = 5000;
				rate = 1.0/rate;
				if (chan->rate > rate)
					chan->rate = rate;
			}
		}
	}
#endif

//
// discard stale or duplicated packets
//
	if (sequence <= (unsigned)chan->incoming_sequence)
	{
		if (showdrop.value)
			Con_Printf ("%s:Out of order packet %i at %i\n"
				, NET_AdrToString (chan->remote_address)
				,  sequence
				, chan->incoming_sequence);
		return false;
	}

//
// dropped packets don't keep the message from being used
//
	net_drop = sequence - (chan->incoming_sequence+1);
	if (net_drop > 0)
	{
		chan->drop_count += 1;

		if (showdrop.value)
			Con_Printf ("%s:Dropped %i packets at %i\n"
			, NET_AdrToString (chan->remote_address)
			, sequence-(chan->incoming_sequence+1)
			, sequence);
	}

//
// if the current outgoing reliable message has been acknowledged
// clear the buffer to make way for the next
//
	if (reliable_ack == (unsigned)chan->reliable_sequence)
		chan->reliable_length = 0;	// it has been received
	
//
// if this message contains a reliable message, bump incoming_reliable_sequence 
//
	chan->incoming_sequence = sequence;
	chan->incoming_acknowledged = sequence_ack;
	chan->incoming_reliable_acknowledged = reliable_ack;
	if (reliable_message)
		chan->incoming_reliable_sequence ^= 1;

//
// the message can now be read from the current message pointer
// update statistics counters
//
	chan->frame_latency = chan->frame_latency*OLD_AVG	+ (chan->outgoing_sequence-sequence_ack)*(1.0-OLD_AVG);
	chan->frame_rate = chan->frame_rate*OLD_AVG	+ (realtime-chan->last_received)*(1.0-OLD_AVG);		
	
	chan->good_count += 1;

	chan->last_received = realtime;

	return true;
}

#endif