#include "Sv_main.h"
#include "World.h"
#include "Game_ents.h"
#include "Net_defs.h"
#include "Net_protocol.h"

/*
======================================
Validate connection request from a client
======================================
*/
bool CServer::ValidateClConnection(int clNum, bool reconnect,
									CBuffer &buffer)
{	
	if(m_clients[clNum].inUse && !reconnect)
	{
		m_net.SendRejectMsg("Couldn't find free client slot");
		return false;
	}

	strcpy(m_clients[clNum].name, buffer.ReadString());
	m_net.ChanSetRate(clNum, buffer.ReadInt());
	
	m_clients[clNum].inUse = true;

	if(!reconnect)
		m_svState.numClients++;

	m_net.BroadcastPrintf("%s connected", m_clients[clNum].name);
	return true;
}

/*
======================================
Handle Network Message from a client
======================================
*/
void CServer::HandleClientMsg(int clNum, CBuffer &buffer)
{
	//Check packet id to see what the client send
	byte packetId = buffer.ReadByte();
	switch(packetId)
	{
	//Talk message
	case CL_TALK:
		{
			char msg[256];
			strcpy(msg,buffer.ReadString());

			int len = strlen(msg);
			msg[len] = 0;
			len += 4;
			len += strlen(m_clients[clNum].name);
			
			//Add this to all other connected clients outgoing buffers
			for(int i=0;i<m_svState.maxClients;i++)
			{
				//dont send to source
				if(i == clNum)
					continue;

				if(m_clients[i].inUse)
				{
					m_net.ChanBeginWrite(i,SV_TALK, len);
					m_net.ChanWrite(m_clients[clNum].name);
					m_net.ChanWrite(msg);
					m_net.ChanFinishWrite();
				}
			}
			break;	
		}
	//client updating its local info
	case CL_UPDATEINFO:
		{
			char id = buffer.ReadChar();
			if(id == 'n')
			{
				const char * clname = buffer.ReadString();
				m_net.BroadcastPrintf("%s renamed to %s", m_clients[clNum].name, clname);
				strcpy(m_clients[clNum].name, clname);
			}
			else if (id == 'r')
			{
				int rate = buffer.ReadInt();
ComPrintf("SV: %s changed rate to %d\n", m_clients[clNum].name, rate);
				m_net.ChanSetRate(clNum,rate);
			}
			break;
		}
	case CL_DISCONNECT:
		{
			m_net.SendDisconnect(clNum,CLIENT_QUIT);
			break;
		}
	}
}

/*
======================================
Handle Client disconnection
======================================
*/
void CServer::OnClientDrop(int clNum, EDisconnectReason reason)
{
	switch(reason)
	{
	case CLIENT_QUIT:
		m_net.BroadcastPrintf("%s disconnected", m_clients[clNum].name);
		break;
	case CLIENT_TIMEOUT:
		m_net.BroadcastPrintf("%s timed out", m_clients[clNum].name);
		break;
	case CLIENT_OVERFLOW:
		m_net.BroadcastPrintf("%s overflowed", m_clients[clNum].name);
		break;
	}

	m_clients[clNum].inUse = false;
	m_svState.numClients --;
}

/*
======================================
Write Game Status to buffer
======================================
*/
void CServer::WriteGameStatus(CBuffer &buffer)
{
}

/*
======================================
Handle Client spawning
======================================
*/
void CServer::OnClientSpawn(int clNum)
{
	//Check chanIds to see what client spawned
	m_net.BroadcastPrintf("%s entered the game", m_clients[clNum].name);
}

/*
======================================
Handle Map change on client ?
======================================
*/
void CServer::OnLevelChange(int clNum)
{
}


//======================================================================================
//======================================================================================

bool CServer::InitNetwork()
{	return CNetServer::InitWinsock();
}

void CServer::ShutdownNetwork()
{	CNetServer::ShutdownWinsock();
}