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
	m_net.ChanWriteShort(m_clients[clNum]->mdlIndex);
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
//ComPrintf("SV: %d - %s wants to disconnect\n", clNum, m_clients[clNum]->name);
				m_net.SendDisconnect(clNum,DR_CLQUIT);
				break;
			}
		case CL_MOVE:
			{
				m_incomingCmd.time = ((int)buffer.ReadByte())/1000.0f;
				m_incomingCmd.forwardmove = buffer.ReadShort();
				m_incomingCmd.rightmove = buffer.ReadShort();
				m_incomingCmd.upmove = buffer.ReadShort();
				m_incomingCmd.angles[0] = buffer.ReadFloat();
				m_incomingCmd.angles[1] = buffer.ReadFloat();
				m_incomingCmd.angles[2] = buffer.ReadFloat();

				if(buffer.BadRead())
				{
					ComPrintf("SV: Bad command from client %s:%d\n", m_clients[clNum]->name,clNum);
					return;
				}
				m_clients[clNum]->clCmd = m_incomingCmd;
				m_clients[clNum]->clCmd.flags = 1;

/*
				m_clients[clNum]->clCmd.time = ((int)buffer.ReadByte())/1000.0f;
				m_clients[clNum]->clCmd.forwardmove = buffer.ReadShort();
				m_clients[clNum]->clCmd.rightmove = buffer.ReadShort();
				m_clients[clNum]->clCmd.upmove = buffer.ReadShort();
				m_clients[clNum]->clCmd.angles[0] = buffer.ReadFloat();
				m_clients[clNum]->clCmd.angles[1] = buffer.ReadFloat();
				m_clients[clNum]->clCmd.angles[2] = buffer.ReadFloat();
				ComPrintf("SV: %d %d %d\n", m_clients[clNum]->clCmd.forwardmove,
					m_clients[clNum]->clCmd.rightmove, m_clients[clNum]->clCmd.upmove);
				if(buffer.BadRead())
					return;
*/
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

	//TODO: Only if the server isn't dedicated
/*	if(m_svState.numClients == 0)
	{
ComPrintf("SV: No Clients left. Shutting down\n");
		AddServerCmd("killserver");
	}
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
bool CServer::WriteConfigString(CBuffer &buffer, int stringId, int numBuffer)
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
			for(int i=0; i< m_svState.numClients; i++)
			{
				if(!m_clients[i] ||!m_clients[i]->spawned)
					continue;

				buffer.WriteByte(SV_CLFULLINFO);
				buffer.WriteByte(m_clients[i]->num);
				buffer.WriteString(m_clients[i]->name);
				buffer.WriteShort(m_clients[i]->mdlIndex);
				buffer.WriteString(m_clients[i]->modelName);
				buffer.WriteShort(m_clients[i]->skinNum);
				buffer.WriteString(m_clients[i]->skinName);
			}
			return true;
		}
	}
	return false;
}
