#include "Net_svclient.h"
#include "Net_defs.h"

//using namespace VoidNet;

//======================================================================================
//======================================================================================
/*
======================================
Constructor/Destructor
======================================
*/
SVClient::SVClient()
{	Reset();
}

SVClient::~SVClient() {}

/*
======================================
Reset the client
======================================
*/
void SVClient::Reset()
{
	m_id = 0;
	m_netChan.Reset();
	m_state = CL_FREE;
	m_numBuf=0;
	m_bBackbuf = false;
	m_bDropClient = false;
	m_bSend = false;
	memset(m_name,0,sizeof(m_name));
}

/*
======================================
make sure that the buffers havent overflowed or anything
if all the backbuffers are full, and it has STILL overflowed
then just give up
======================================
*/
void SVClient::ValidateBuffer()
{
	if(m_bBackbuf)
	{
		//drop client if overflowed
		if(m_backBuffer[m_numBuf].OverFlowed())
		{
ComPrintf("backbuffer[%d] overflowed for %s(%s)\n",	m_numBuf, m_name, m_netChan.GetAddrString());
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
void SVClient::MakeSpace(int reqsize)
{
	//a message of this size will overflow the buffer
	if((m_netChan.m_buffer.GetSize() + reqsize) >= m_netChan.m_buffer.GetMaxSize())
	{
		//havent been using any backbuffers, so start now
ComPrintf("Using backbuffer for %s\n", m_name);
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
ComPrintf("All backbuffers overflowed for %s(%s)\n", m_name, m_netChan.GetAddrString());
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
bool SVClient::ReadyToSend()
{
	if(m_bBackbuf)
	{
		//Does the outgoing buffer have space ?
//		if(m_netChan.m_buffer.GetSize() + m_backBuffer[m_numBuf]->GetSize() <
		if(m_netChan.m_buffer.GetSize() + m_backBuffer[m_numBuf].GetSize() <
		   m_netChan.m_buffer.GetMaxSize())
		{
ComPrintf("SV Writing to backbuffer\n");
			//Write to sock buffer
			//m_netChan.m_buffer.Write((*m_backBuffer[m_numBuf]));
			m_netChan.m_buffer.Write(m_backBuffer[m_numBuf]);
			//reset buffer. 
			//m_backBuffer[m_numBuf]->Reset();
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
		//broadcast and drop client here
/*		SV_BroadcastPrintf (PRINT_HIGH, "%s overflowed\n", c->name);
		Con_Printf ("WARNING: reliable overflow for %s\n",c->name);
		SV_DropClient (c);
		c->send_message = true;
		c->netchan.cleartime = 0;	// don't choke this message
*/
	}

	// only send messages if the client has sent one
	// and the bandwidth is not choked
	if(!m_bSend || !m_netChan.CanSend())
		return false;
	return true;
}

//======================================================================================
//======================================================================================

void SVClient::BeginMessage(byte msgid, int estSize)
{
	MakeSpace(estSize);
	WriteByte(msgid);
}


void SVClient::WriteByte(byte b)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.Write(b);
	else
	{
//		m_backBuffer[m_numBuf]->Write(b);
		m_backBuffer[m_numBuf].Write(b);
		ValidateBuffer();
	}
}

void SVClient::WriteChar(char c)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.Write(c);
	else
	{
		m_backBuffer[m_numBuf].Write(c);
		ValidateBuffer();
	}
}

void SVClient::WriteShort(short s)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.Write(s);
	else
	{
		m_backBuffer[m_numBuf].Write(s);
		ValidateBuffer();
	}
}

void SVClient::WriteInt(int i)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.Write(i);
	else
	{
		m_backBuffer[m_numBuf].Write(i);
		ValidateBuffer();
	}
}

void SVClient::WriteFloat(float f)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.Write(f);
	else
	{
		m_backBuffer[m_numBuf].Write(f);
		ValidateBuffer();
	}
}

void SVClient::WriteString(const char *string)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.Write(string);
	else
	{
		m_backBuffer[m_numBuf].Write(string);
		ValidateBuffer();
	}
}

void SVClient::WriteCoord(float c)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteCoord(c);
	else
	{
		m_backBuffer[m_numBuf].WriteCoord(c);
		ValidateBuffer();
	}
}

void SVClient::WriteAngle(float a)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteAngle(a);
	else
	{
		m_backBuffer[m_numBuf].WriteAngle(a);
		ValidateBuffer();
	}
}

void SVClient::WriteData(byte * data, int len)
{
	//no backbuffer. just write to channel
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteData(data,len);
	else	
	{
		//write to current backbuffer
		m_backBuffer[m_numBuf].WriteData(data,len);
		ValidateBuffer();
	}
}
