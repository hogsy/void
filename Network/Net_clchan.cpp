#include "Net_server.h"
#include "Net_clchan.h"


using namespace VoidNet;

//======================================================================================
//======================================================================================
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
ComPrintf("SV Writing to backbuffer\n");
			//Write to sock buffer
			m_netChan.m_buffer.Write(m_backBuffer[m_numBuf]);
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

//======================================================================================
//======================================================================================

bool CNetServer::ChanCanSend(int chanId)
{	return m_clChan[chanId].m_netChan.CanSend();
}


void CNetServer::ChanBeginWrite(int chanId, byte msgid, int estSize)
{
	m_curChanId = chanId;
	m_clChan[chanId].MakeSpace(estSize);
	ChanWrite(msgid);
}

void CNetServer::ChanWrite(byte b)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.Write(b);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].Write(b);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}
void CNetServer::ChanWrite(char c)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.Write(c);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].Write(c);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanWrite(short s)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.Write(s);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].Write(s);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanWrite(int i)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.Write(i);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].Write(i);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanWrite(float f)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.Write(f);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].Write(f);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanWrite(const char *string)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.Write(string);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].Write(string);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanWriteCoord(float c)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.WriteCoord(c);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].WriteCoord(c);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanWriteAngle(float a)
{
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.WriteAngle(a);
	else
	{
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].WriteAngle(a);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanWriteData(byte * data, int len)
{
	//no backbuffer. just write to channel
	if(!m_clChan[m_curChanId].m_bBackbuf)
		m_clChan[m_curChanId].m_netChan.m_buffer.WriteData(data,len);
	else	
	{
		//write to current backbuffer
		m_clChan[m_curChanId].m_backBuffer[m_clChan[m_curChanId].m_numBuf].WriteData(data,len);
		m_clChan[m_curChanId].ValidateBuffer();
	}
}

void CNetServer::ChanFinishWrite()
{	m_curChanId = 0;
}
	

const NetChanState & CNetServer::ChanGetState(int chanId) const
{	return m_clChan[chanId].m_netChan.m_state;
}

void CNetServer::ChanSetRate(int chanId,int rate)
{	m_clChan[chanId].m_netChan.SetRate(rate);
}

int CNetServer::ChanGetRate(int chanId)
{	return m_clChan[chanId].m_netChan.GetRate();
}
