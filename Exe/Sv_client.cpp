#include "Sv_client.h"
#include "Net_defs.h"

using namespace VoidNet;

//======================================================================================
//======================================================================================
/*
======================================
Constructor/Destructor
======================================
*/
SVClient::SVClient()
{
	for(int i=0; i< MAX_BACKBUFFERS; i++)
		m_backBuffer[i] = new CNetBuffer(MAX_DATAGRAM_SIZE);
	Reset();
}

SVClient::~SVClient()
{
	for(int i=0; i< MAX_BACKBUFFERS; i++)
		delete m_backBuffer[i];
}

/*
======================================
Reset the client
======================================
*/
void SVClient::Reset()
{
	m_netChan.Reset();
	m_bSentSpawn = false;
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
void SVClient::ValidateBuffer()
{
	if(m_bBackbuf)
	{
		//drop client if overflowed
		if(m_backBuffer[m_numBuf]->OverFlowed())
		{
			ComPrintf("backbuffer[%d] overflowed for %s(%s)\n",
						m_numBuf, m_name, m_netChan.m_addr.ToString());
			m_bDropClient = true;
		}
	}
}

/*
======================================
make space for a data chunk of the given size
======================================
*/
void SVClient::MakeSpace(int maxsize)
{
	//a message of this size will overflow the buffer
	if(m_netChan.m_buffer.GetSize() + (maxsize >= m_netChan.m_buffer.GetMaxSize()))
	{
		//havent been using any backbuffers until now
		if(!m_bBackbuf)
			m_bBackbuf = true;
		else
		{
			//if the current backbuffer doesnt have any space, then move to the next one
			if(m_backBuffer[m_numBuf]->GetSize() + maxsize >= m_backBuffer[m_numBuf]->GetMaxSize())
			{
				//drop client if we have filled up the LAST backbuffer,
				if(m_numBuf + 1 == MAX_BACKBUFFERS)
				{
					ComPrintf("All backbuffers overflowed for %s(%s)\n", m_name, m_netChan.m_addr.ToString());
					m_bDropClient = true;
					return;
				}
				m_numBuf++;
				m_backBuffer[m_numBuf]->Reset();
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
		if(m_netChan.m_buffer.GetSize() + m_backBuffer[m_numBuf]->GetSize() <
		   m_netChan.m_buffer.GetMaxSize())
		{
			//Write to sock buffer
			m_netChan.m_buffer += *m_backBuffer[m_numBuf];

			//rotate buffers
			m_backBuffer[m_numBuf]->Reset();
			CNetBuffer * temp = m_backBuffer[m_numBuf];
			for(int i=1;i<MAX_BACKBUFFERS;i++)
				m_backBuffer[i-1] = m_backBuffer[i];
			m_backBuffer[i] = temp;

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
/*
	if (c->state == cs_spawned)
		SV_SendClientDatagram (c);
	else
		Netchan_Transmit (&c->netchan, 0, NULL);	// just update reliable
*/
}

//======================================================================================
//======================================================================================

void SVClient::BeginMessage(char msgid, int estSize)
{
	MakeSpace(estSize);
	WriteByte(msgid);
}


void SVClient::WriteByte(byte b)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer += b;
	else
	{
		m_backBuffer[m_numBuf] += b;
		ValidateBuffer();
	}
}

void SVClient::WriteChar(char c)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer += c;
	else
	{
		m_backBuffer[m_numBuf] += c;
		ValidateBuffer();
	}
}

void SVClient::WriteShort(short s)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer += s;
	else
	{
		m_backBuffer[m_numBuf] += s;
		ValidateBuffer();
	}
}

void SVClient::WriteInt(int i)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer += i;
	else
	{
		m_backBuffer[m_numBuf] += i;
		ValidateBuffer();
	}
}

void SVClient::WriteFloat(float f)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer += f;
	else
	{
		*m_backBuffer[m_numBuf] += f;
		ValidateBuffer();
	}
}

void SVClient::WriteString(const char *string)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer += string;
	else
	{
		*m_backBuffer[m_numBuf] += string;
		ValidateBuffer();
	}
}

void SVClient::WriteCoord(float c)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteCoord(c);
	else
	{
		m_backBuffer[m_numBuf]->WriteCoord(c);
		ValidateBuffer();
	}
}

void SVClient::WriteAngle(float a)
{
	if(!m_bBackbuf)
		m_netChan.m_buffer.WriteAngle(a);
	else
	{
		m_backBuffer[m_numBuf]->WriteAngle(a);
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
		m_backBuffer[m_numBuf]->WriteData(data,len);
		ValidateBuffer();
	}
}
