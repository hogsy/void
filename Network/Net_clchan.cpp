#include "Net_hdr.h"
#include "Net_chan.h"
#include "Net_server.h"
#include "Net_clchan.h"

using namespace VoidNet;

/*
======================================
Constructor/Destructor
======================================
*/
CNetClChan::CNetClChan()
{	Reset();
}

CNetClChan::~CNetClChan() 
{
}

/*
======================================
Reset the client
======================================
*/
void CNetClChan::Reset()
{
	m_netChan.Reset();
	
	m_spawnLevel = 0;
	m_spawnReqId = 0;

	m_state = CL_FREE;
	m_numBuf=0;
	m_bBackbuf = false;
	m_bDropClient = false;
	m_bSend = false;
}

/*
======================================
make sure that the buffers havent overflowed or anything
if all the backbuffers are full, and it has STILL overflowed
then just give up
======================================
*/
void CNetClChan::ValidateBuffer()
{
	if(m_bBackbuf)
	{
		//drop client if overflowed
		if(m_backBuffer[m_numBuf].OverFlowed())
		{
ComPrintf("backbuffer[%d] overflowed for %s\n",	m_numBuf, m_netChan.GetAddrString());
			m_bDropClient = true;
		}
	}
}

/*
======================================
make space for a data chunk of the given size
change current backbuffer if needed
======================================
*/
void CNetClChan::MakeSpace(int reqsize)
{
	//a message of this size will overflow the buffer
	if((m_netChan.m_buffer.GetSize() + reqsize) >= m_netChan.m_buffer.GetMaxSize())
	{
		//havent been using any backbuffers, so start now
ComPrintf("Using backbuffer for %s\n", m_netChan.GetAddrString());
		if(!m_bBackbuf)
			m_bBackbuf = true;
		else
		{
			//if the current backbuffer doesnt have any space, then move to the next one
			if(m_backBuffer[m_numBuf].GetSize() + reqsize >= m_backBuffer[m_numBuf].GetMaxSize())
			{
				//drop client if we have filled up the LAST backbuffer,
				if(m_numBuf + 1 == MAX_BACKBUFFERS)
				{
ComPrintf("All backbuffers overflowed for %s\n", m_netChan.GetAddrString());
					m_bDropClient = true;
					return;
				}
				m_numBuf++;
				m_backBuffer[m_numBuf].Reset();
			}
		}
	}
}

/*
======================================
Server wants to send a new message now
======================================
*/
bool CNetClChan::ReadyToSend()
{
	if(m_bBackbuf)
	{
		//Does the outgoing buffer have space ?
		if(m_netChan.m_buffer.GetSize() + m_backBuffer[m_numBuf].GetSize() <
		   m_netChan.m_buffer.GetMaxSize())
		{
ComPrintf("SV Writing to backbuffer for %s\n", m_netChan.GetAddrString());
			//Write to sock buffer
			m_netChan.m_buffer.WriteBuffer(m_backBuffer[m_numBuf]);
			//reset buffer. 
			m_backBuffer[m_numBuf].Reset();

			if(m_numBuf == 0)		
				m_bBackbuf = false;	//No more backbuffers
			else
				m_numBuf --;		//One less backbuffer
		}
	}

	//drop client if the outgoing buffer has overflowed
	if(m_netChan.m_buffer.OverFlowed())
	{
		m_netChan.m_buffer.Reset();
		m_bDropClient = true;
	}

	// only send messages if the client has sent one
	// and the bandwidth is not choked
	if(!m_bSend || !m_netChan.CanSend())
		return false;
	return true;
}

/*
======================================
Client Writing funcs
======================================
*/
void CNetClChan::WriteByte(byte b)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteByte(b);
	else
	{
		m_backBuffer[m_numBuf].WriteByte(b);
		ValidateBuffer();
	}
}
void CNetClChan::WriteChar(char c)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteChar(c);
	else
	{
		m_backBuffer[m_numBuf].WriteChar(c);
		ValidateBuffer();
	}
}

void CNetClChan::WriteShort(short s)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteShort(s);
	else
	{
		m_backBuffer[m_numBuf].WriteShort(s);
		ValidateBuffer();
	}
}

void CNetClChan::WriteInt(int i)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteInt(i);
	else
	{
		m_backBuffer[m_numBuf].WriteInt(i);
		ValidateBuffer();
	}
}

void CNetClChan::WriteFloat(float &f)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteFloat(f);
	else
	{
		m_backBuffer[m_numBuf].WriteFloat(f);
		ValidateBuffer();
	}
}


void CNetClChan::WriteString(const char *string)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteString(string);
	else
	{
		m_backBuffer[m_numBuf].WriteString(string);
		ValidateBuffer();
	}
}

void CNetClChan::WriteCoord(float &c)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteCoord(c);
	else
	{
		m_backBuffer[m_numBuf].WriteCoord(c);
		ValidateBuffer();
	}
}

void CNetClChan::WriteAngle(float &a)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteAngle(a);
	else
	{
		m_backBuffer[m_numBuf].WriteAngle(a);
		ValidateBuffer();
	}
}

void CNetClChan::WriteData(byte * data, int len)
{
	//no backbuffer. just write to channel
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteData(data,len);
	else	
	{
		m_backBuffer[m_numBuf].WriteData(data,len);
		ValidateBuffer();
	}
}

//======================================================================================
//Network Server Channel funcs
//======================================================================================

bool CNetServer::ChanCanSend(int chanId)
{	return (m_clChan[chanId].m_bSend && m_clChan[chanId].m_netChan.CanSend());
}

void CNetServer::ChanBeginWrite(const MultiCastSet &set, byte msgid, int estSize)
{
	ChanFinishWrite();
	m_pMultiCast = &set;
	for(int i=0; i< m_pSvState->maxClients; i++)
	{
		if(m_pMultiCast->dest[i])
			m_clChan[i].MakeSpace(estSize);
	}
	ChanWriteByte(msgid);
}

void CNetServer::ChanBeginWrite(int chanId, byte msgid, int estSize)
{
	ChanFinishWrite();
	m_curChanId = chanId;
	m_clChan[chanId].MakeSpace(estSize);
	ChanWriteByte(msgid);
}

void CNetServer::ChanWriteByte(byte b)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteByte(b);
		return;
	}
	m_clChan[m_curChanId].WriteByte(b);
}


void CNetServer::ChanWriteChar(char c)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteChar(c);
		return;
	}
	m_clChan[m_curChanId].WriteChar(c);
}

void CNetServer::ChanWriteShort(short s)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteShort(s);
		return;
	}
	m_clChan[m_curChanId].WriteShort(s);
}

void CNetServer::ChanWriteInt(int i)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteInt(i);
		return;
	}
	m_clChan[m_curChanId].WriteInt(i);
}

void CNetServer::ChanWriteFloat(float f)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteFloat(f);
		return;
	}
	m_clChan[m_curChanId].WriteFloat(f);
}

void CNetServer::ChanWriteString(const char *string)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteString(string);
		return;
	}
	m_clChan[m_curChanId].WriteString(string);
}

void CNetServer::ChanWriteCoord(float c)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteCoord(c);
		return;
	}
	m_clChan[m_curChanId].WriteCoord(c);
}

void CNetServer::ChanWriteAngle(float a)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteAngle(a);
		return;
	}
	m_clChan[m_curChanId].WriteAngle(a);
}


void CNetServer::ChanWriteData(byte * data, int len)
{
	if(m_pMultiCast)
	{
		for(int i=0; i< m_pSvState->maxClients; i++)
			if(m_pMultiCast->dest[i])
				m_clChan[i].WriteData(data,len);
		return;
	}
	m_clChan[m_curChanId].WriteData(data,len);
}

void CNetServer::ChanFinishWrite()
{	
	m_curChanId = -1;
	m_pMultiCast = 0;			
}
	
/*
======================================
Misc Channel related funcs
======================================
*/
const NetChanState & CNetServer::ChanGetState(int chanId) const
{	return m_clChan[chanId].m_netChan.m_state;
}

void CNetServer::ChanSetRate(int chanId,int rate)
{	m_clChan[chanId].m_netChan.SetRate(rate);
}

int CNetServer::ChanGetRate(int chanId)
{	return m_clChan[chanId].m_netChan.GetRate();
}
