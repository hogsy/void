#include "Sv_main.h"
#include "World.h"
#include "Net_defs.h"
#include "Net_protocol.h"

/*
======================================
Validate connection request from a client
======================================
*/
bool CServer::ValidateClConnection(int clNum, 
								   bool reconnect,
								   CBuffer &buffer)
{
	if(!m_pGame->ClientConnect(clNum,buffer,reconnect))
	{
		m_net.SendRejectMsg("Couldn't find free client slot");
		return false;
	}

	m_net.ChanSetRate(clNum, buffer.ReadInt());
	m_svState.numClients = m_pGame->numClients;

	//Add client info to all connected clients
	int len = strlen(m_clients[clNum]->name) + 
			  strlen(m_clients[clNum]->modelName) + 
			  strlen(m_clients[clNum]->skinName) + 10;

	GetMultiCastSet(m_multiCastSet,MULTICAST_ALL_X,clNum);
	
	m_net.ChanBeginWrite(m_multiCastSet, SV_CLFULLINFO, len);
	m_net.ChanWriteByte(m_clients[clNum]->num);
	m_net.ChanWriteString(m_clients[clNum]->name);
	m_net.ChanWriteShort(m_clients[clNum]->modelIndex);
	m_net.ChanWriteString(m_clients[clNum]->modelName);
	m_net.ChanWriteShort(m_clients[clNum]->skinNum);
	m_net.ChanWriteString(m_clients[clNum]->skinName);
	m_net.ChanFinishWrite();
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
/*
	if(packetId != CL_MOVE)
	{
ComPrintf("SV :message from %s\n", m_clients[clNum]);
	}
*/
	while(packetId != 255)
	{
		switch(packetId)
		{
		//Talk message
		case CL_TALK:
			{
				char msg[256];
				strcpy(msg,buffer.ReadString());
				int len = strlen(msg) + 8;
				msg[len] = 0;

				//Write to other clients
				GetMultiCastSet(m_multiCastSet,MULTICAST_ALL_X,clNum);
				m_net.ChanBeginWrite(m_multiCastSet, SV_TALK, len);
				m_net.ChanWriteByte(clNum);
				m_net.ChanWriteString(msg);
				m_net.ChanFinishWrite();
				break;	
			}
		//client updating its local info
		case CL_INFOCHANGE:
			{
				char id = buffer.ReadChar();
				if(id == 'n')
				{
					const char * clname = buffer.ReadString();
ComPrintf("SV: %s renamed to %s\n", m_clients[clNum]->name, clname);
					strcpy(m_clients[clNum]->name, clname);

					GetMultiCastSet(m_multiCastSet,MULTICAST_ALL_X, clNum);
					m_net.ChanBeginWrite(m_multiCastSet,SV_CLINFOCHANGE, strlen(m_clients[clNum]->name)+2);
					m_net.ChanWriteByte(clNum);
					m_net.ChanWriteByte('n');
					m_net.ChanWriteString(m_clients[clNum]->name);
					m_net.ChanFinishWrite();
				}
				else if (id == 'r')
				{
					int rate = buffer.ReadInt();
ComPrintf("SV: %s changed rate to %d\n", m_clients[clNum]->name, rate);
					m_net.ChanSetRate(clNum,rate);
				}
				break;
			}
		case CL_DISCONNECT:
			{
ComPrintf("SV: %d - %s wants to disconnect\n", clNum, m_clients[clNum]->name);
				m_net.SendDisconnect(clNum,DR_CLQUIT);
				break;
			}
		case CL_MOVE:
			{
				m_clients[clNum]->origin.x = buffer.ReadCoord();
				m_clients[clNum]->origin.y = buffer.ReadCoord();
				m_clients[clNum]->origin.z = buffer.ReadCoord();
				m_clients[clNum]->angles.x = buffer.ReadAngle();
				m_clients[clNum]->angles.y = buffer.ReadAngle();
				m_clients[clNum]->angles.z = buffer.ReadAngle();
				break;
			}
		default:
			{
				m_net.SendDisconnect(clNum,DR_CLBADMSG);
				break;
			}
		}
		//Check for more messages in the buffer
		packetId = buffer.ReadByte();
	}
}

/*
======================================
Handle Client disconnection
======================================
*/
void CServer::OnClientDrop(int clNum, const DisconnectReason &reason)
{
//	if(reason.broadcastMsg)
//		BroadcastPrintf(reason.broadcastMsg, m_clients[clNum]->name);

	GetMultiCastSet(m_multiCastSet,MULTICAST_ALL_X, clNum);
	if(reason.broadcastMsg)
	{
		m_net.ChanBeginWrite(m_multiCastSet,SV_CLDISCONNECT, 
							 strlen(reason.broadcastMsg) + 2);
		m_net.ChanWriteByte(clNum);
		m_net.ChanWriteString(reason.broadcastMsg);
	}
	else
	{
		m_net.ChanBeginWrite(m_multiCastSet,SV_CLDISCONNECT, 
							 strlen(DR_CLQUIT.broadcastMsg) + 2);
		m_net.ChanWriteByte(clNum);
		m_net.ChanWriteString(DR_CLQUIT.broadcastMsg);
	}
	m_net.ChanFinishWrite();
	

	m_pGame->ClientDisconnect(clNum);
	m_svState.numClients = m_pGame->numClients;
}

/*
	m_net.ChanBeginWrite(
	m_net.ChanBeginWrite(i,SV_CLINFO, len);
	m_net.ChanWriteByte(m_clients[clNum]->num);
	m_net.ChanWriteString(m_clients[clNum]->name);
*/

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
void CServer::OnClientBegin(int clNum)
{
	//This should set the clients spawn pos, and write an temp effects
	m_pGame->ClientBegin(clNum);
}

/*
======================================
Handle Map change on client ?
======================================
*/
void CServer::OnLevelChange(int clNum)
{
}


/*
======================================

======================================
*/
void CServer::GetMultiCastSet(MultiCastSet &set, MultiCastType type, int clId)
{
	if((type == MULTICAST_ALL) || (type == MULTICAST_ALL_X))
	{
		set.Reset();
		for(int i=0;i<m_svState.numClients;i++)
		{
			if(m_clients[i] && m_clients[i]->spawned)
					set.dest[i] = true;
		}
		if(type == MULTICAST_ALL_X)
			set.dest[clId] = false;
	}
	else if((type == MULTICAST_PVS) || (type == MULTICAST_PVS_X))
	{
	}
	else if((type == MULTICAST_PHS) || (type == MULTICAST_PHS_X))
	{
	}
}

void CServer::GetMultiCastSet(MultiCastSet &set, MultiCastType type, const vector_t &source)
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