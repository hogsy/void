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
	if(m_clients[clNum] && !reconnect)
	{
		m_net.SendRejectMsg("Couldn't find free client slot");
		return false;
	}



	m_clients[clNum] = new EntClient();
	
	strcpy(m_clients[clNum]->name, buffer.ReadString());
	
	strcpy(m_clients[clNum]->modelName,buffer.ReadString());
	m_clients[clNum]->modelIndex = RegisterModel(m_clients[clNum]->modelName);

	strcpy(m_clients[clNum]->skinName,buffer.ReadString());
	m_clients[clNum]->skinNum = RegisterImage(m_clients[clNum]->skinName);

	m_net.ChanSetRate(clNum, buffer.ReadInt());
	
	m_clients[clNum]->inUse = true;
	m_clients[clNum]->spawned = false;

	m_numEntities++;

	if(!reconnect)
		m_svState.numClients++;

	m_clients[clNum]->num = m_svState.numClients;

	m_net.BroadcastPrintf("%s connected", m_clients[clNum]->name);

	int len = 10 + strlen(m_clients[clNum]->name) + 
		strlen(m_clients[clNum]->modelName) + strlen(m_clients[clNum]->skinName);

	//Add client info to all connected clients
	for(int i=0;i<m_svState.maxClients;i++)
	{
		//dont send to source
		if(i == clNum)
			continue;

		if(m_clients[i] && m_clients[i]->spawned)
		{
			m_net.ChanBeginWrite(i,SV_CLIENTINFO, len);
			m_net.ChanWrite((short)m_clients[clNum]->num);
			m_net.ChanWrite(m_clients[clNum]->name);
			m_net.ChanWrite((short)m_clients[clNum]->modelIndex);
			m_net.ChanWrite(m_clients[clNum]->modelName);
			m_net.ChanWrite((short)m_clients[clNum]->skinNum);
			m_net.ChanWrite(m_clients[clNum]->skinName);
			m_net.ChanFinishWrite();
		}
	}
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
//	EntClient * client = reinterpret_cast<EntClient *>(m_entities[clNum]);

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
			len += strlen(m_clients[clNum]->name);
			
			//Add this to all other connected clients outgoing buffers
			for(int i=0;i<m_svState.maxClients;i++)
			{
				//dont send to source
//				if(i == clNum)
//					continue;

				if(m_clients[i])
				{
					m_net.ChanBeginWrite(i,SV_TALK, len);
					m_net.ChanWrite(m_clients[clNum]->name);
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
				m_net.BroadcastPrintf("%s renamed to %s", m_clients[clNum]->name, clname);
				strcpy(m_clients[clNum]->name, clname);
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
			m_net.SendDisconnect(clNum,CLIENT_QUIT);
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
	}
}

/*
======================================
Handle Client disconnection
======================================
*/
void CServer::OnClientDrop(int clNum, EDisconnectReason reason)
{
//	EntClient * client = reinterpret_cast<EntClient *>(m_entities[clNum]);

	switch(reason)
	{
	case CLIENT_QUIT:
ComPrintf("%s Disconnected", m_clients[clNum]->name);
		m_net.BroadcastPrintf("%s disconnected", m_clients[clNum]->name);
		break;
	case CLIENT_TIMEOUT:
ComPrintf("%s Timed out", m_clients[clNum]->name);
		m_net.BroadcastPrintf("%s timed out", m_clients[clNum]->name);
		break;
	case CLIENT_OVERFLOW:
ComPrintf("%s overflowed", m_clients[clNum]->name);
		m_net.BroadcastPrintf("%s overflowed", m_clients[clNum]->name);
		break;
	}

	//Run through resources to see if we can free anything ?

	delete m_clients[clNum];
	m_clients[clNum] = 0;
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
	m_net.BroadcastPrintf("%s entered the game", m_clients[clNum]->name);
	m_clients[clNum]->spawned = true;

	//send other clients info
	for(int i=0; i< m_svState.numClients; i++)
	{
		if(i == clNum || !m_clients[i] ||
			!m_clients[i]->inUse)
			continue;
		
		m_net.ChanBeginWrite(clNum,SV_CLIENTINFO, 0);
		m_net.ChanWrite((short)m_clients[i]->num);
		m_net.ChanWrite(m_clients[i]->name);
		m_net.ChanWrite((short)m_clients[i]->modelIndex);
		m_net.ChanWrite(m_clients[i]->modelName);
		m_net.ChanWrite((short)m_clients[i]->skinNum);
		m_net.ChanWrite(m_clients[i]->skinName);
		m_net.ChanFinishWrite();
	}


/*
	m_net.ChanBeginWrite(clNum,SV_CLIENTINFO, len);
	m_net.ChanWrite((short)m_clients[clNum]->num);
	m_net.ChanWrite(m_clients[clNum]->name);
	m_net.ChanWrite((short)m_clients[clNum]->modelIndex);
	m_net.ChanWrite(m_clients[clNum]->modelName);
	m_net.ChanWrite((short)m_clients[clNum]->skinNum);
	m_net.ChanWrite(m_clients[clNum]->skinName);
	m_net.ChanFinishWrite();
*/
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