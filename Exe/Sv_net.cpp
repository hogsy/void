#include "Sys_hdr.h"
#include "Sv_main.h"
#include "Com_world.h"

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
		m_net.SendRejectMsg("Server rejected connection");
		return false;
	}

	m_net.ChanSetRate(clNum, buffer.ReadInt());
	m_svState.numClients = m_pGame->numClients;

	//Add client info to all connected clients
	int len = strlen(m_clients[clNum]->name) + 
			  strlen(m_clients[clNum]->charPath) + 2;

	GetMultiCastSet(m_multiCastSet,MULTICAST_ALL_X,clNum);
	
	m_net.ChanBeginWrite(m_multiCastSet, SV_CLFULLINFO, len);
	m_net.ChanWriteByte(m_clients[clNum]->num);
	m_net.ChanWriteString(m_clients[clNum]->name);
	m_net.ChanWriteString(m_clients[clNum]->charPath);
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
				else
				{
					return;
				}
				break;
			}
		case CL_DISCONNECT:
			{
				m_net.SendDisconnect(clNum,DR_CLQUIT);
				break;
			}
		case CL_MOVE:
			{
				m_incomingCmd.time = ((int)buffer.ReadByte())* 1000.0f;

				m_clients[clNum]->origin.x = buffer.ReadFloat();
				m_clients[clNum]->origin.y = buffer.ReadFloat();
				m_clients[clNum]->origin.z = buffer.ReadFloat();
				
				m_clients[clNum]->angles.x = buffer.ReadFloat();
				m_clients[clNum]->angles.y = buffer.ReadFloat();
				m_clients[clNum]->angles.z = buffer.ReadFloat();

#if 0
				m_incomingCmd.moveFlags = buffer.ReadByte();
				m_incomingCmd.angles.x = buffer.ReadFloat();
				m_incomingCmd.angles.y = buffer.ReadFloat();
				m_incomingCmd.angles.z = buffer.ReadFloat();

				if(buffer.BadRead())
				{
					ComPrintf("SV: Bad command from client %s:%d\n", m_clients[clNum]->name,clNum);
					return;
				}
				m_clients[clNum]->clCmd.UpdateCmd(m_incomingCmd);
#endif
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
Handle Client spawning
======================================
*/
void CServer::OnClientBegin(int clNum)
{
	//This should set the clients spawn pos, and write any temp effects
	m_pGame->ClientBegin(clNum);
}


/*
======================================
Handle Client disconnection
======================================
*/
void CServer::OnClientDrop(int clNum, const DisconnectReason &reason)
{
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
======================================
Handle Map change on client ?
======================================
*/
void CServer::OnLevelChange(int clNum)
{	
	m_clients[clNum]->bSpawned = false;
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
Return the number of buffers for the given
config string
======================================
*/
int CServer::NumConfigStringBufs(int stringId) const
{
	switch(stringId)
	{
	case SVC_GAMEINFO:
		return 1;
	case SVC_MODELLIST:
		return m_signOnBufs.numModelBufs;
	case SVC_SOUNDLIST:
		return m_signOnBufs.numSoundBufs;
	case SVC_IMAGELIST:
		return m_signOnBufs.numImageBufs;
	case SVC_BASELINES:
		return m_signOnBufs.numEntityBufs;
	case SVC_CLIENTINFO:
		return 1;
	}
	return 0;
}

/*
======================================
Write the requested config string to 
the given buffer
======================================
*/
bool CServer::WriteConfigString(int clNum, CBuffer &buffer, int stringId, int numBuffer)
{
	switch(stringId)
	{
	case SVC_GAMEINFO:
		{
			buffer.WriteBuffer(m_signOnBufs.gameInfo);
			return true;
		}
	case SVC_MODELLIST:
		{
			if(numBuffer >= m_signOnBufs.numModelBufs)
				return false;
			buffer.WriteBuffer(m_signOnBufs.modelList[numBuffer]);
			return true;
		}
	case SVC_SOUNDLIST:
		{
			if(numBuffer >= m_signOnBufs.numSoundBufs)
				return false;
			buffer.WriteBuffer(m_signOnBufs.soundList[numBuffer]);
			return true;
		}
	case SVC_IMAGELIST:
		{
			if(numBuffer >= m_signOnBufs.numImageBufs)
				return false;
			buffer.WriteBuffer(m_signOnBufs.imageList[numBuffer]);
			return true;
		}
	case SVC_BASELINES:
		{
			if(numBuffer >= m_signOnBufs.numEntityBufs)
				return false;
			buffer.WriteBuffer(m_signOnBufs.entityList[numBuffer]);
			return true;
		}
	case SVC_CLIENTINFO:
		{
			//This shouldn't go above max packet size
			//Write info about all currently connected clients
			buffer.WriteByte(m_svState.numClients);

			for(int i=0; i< m_svState.numClients; i++)
			{
				if(!m_clients[i]  || i==clNum  )
					continue;

				buffer.WriteByte(SV_CLFULLINFO);
				buffer.WriteByte(m_clients[i]->num);
				buffer.WriteString(m_clients[i]->name);
				buffer.WriteString(m_clients[i]->charPath);
			}
			return true;
		}
	}
	return false;
}
