#include "Sv_main.h"
#include "Com_util.h"
#include "Net_defs.h"
#include "Net_protocol.h"


//Network Handler
bool CServer::ValidateClConnection(int chanId, bool reconnect,
							  CBuffer &buffer)
{	

	if(m_client[chanId].inUse)
	{
		m_net.SendRejectMsg("Couldn't find free client slot");
		return false;
	}

	strcpy(m_client[chanId].name, buffer.ReadString());
	m_net.SetChanRate(chanId, buffer.ReadInt());
	
	m_client[chanId].inUse = true;

	m_svState.numClients++;

	m_net.BroadcastPrintf("%s connected", m_client[chanId].name);
	return true;
}


void CServer::HandleClientMsg(int clNum, CBuffer &buffer)
{
	//Check packet id to see what the client send

	byte packetId = buffer.ReadByte();
	switch(packetId)
	{
	//Talk message
	case CL_TALK:
		{
//ComPrintf("SV:%s : %s\n", client.m_name, msg);

			char msg[256];
			strcpy(msg,buffer.ReadString());

			int len = strlen(msg);
			msg[len] = 0;
			len += 4;
			len += strlen(m_client[clNum].name);
			
			//Add this to all other connected clients outgoing buffers
			//for(int i=0; i<m_cMaxClients.ival;i++)
			for(int i=0;i<m_svState.maxClients;i++)
			{
				//dont send to source
				if(i == clNum)
					continue;
				if(m_client[i].inUse)
				{
					m_net.BeginWrite(i,SV_TALK, len);
					m_net.Write(m_client[clNum].name);
					m_net.Write(msg);
					m_net.FinishWrite();
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
				m_net.BroadcastPrintf("%s renamed to %s", m_client[clNum].name, clname);
				strcpy(m_client[clNum].name, clname);
			}
			else if (id == 'r')
			{
				int rate = buffer.ReadInt();
ComPrintf("SV: %s changed rate to %d\n", m_client[clNum].name, rate);
				m_net.SetChanRate(clNum,rate);
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




void CServer::OnClientDrop(int chanId, EDisconnectReason reason)
{
//	m_net.BroadcastPrintf("%s disconnected", m_client[clNum].name);

	m_client[chanId].inUse = false;
	m_svState.numClients --;
}

void CServer::WriteGameStatus(CBuffer &buffer)
{
}


void CServer::OnClientSpawn(int clNum)
{
	//Check chanIds to see what client spawned
	m_net.BroadcastPrintf("%s entered the game", m_client[clNum].name);
//	m_client[clNum].name
}


void CServer::OnLevelChange(int clNum)
{
}

