#include "Sv_main.h"
#include "Com_util.h"
#include "Net_defs.h"
#include "Net_protocol.h"



//Network Handler
bool CServer::ValidateClConnection(int chanId, bool reconnect,
							  CBuffer &buffer, 
							  char ** reason)
{	

	if(m_client[chanId].inUse)
	{
		*reason = "Bad Client slot";
		return false;
	}

	strcpy(m_client[chanId].name, buffer.ReadString());
	m_pNet->ChanSetRate(chanId, buffer.ReadInt());
	m_client[chanId].inUse = true;

	m_pNet->BroadcastPrintf(-1,"%s connected", m_client[chanId].name);
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
//				if(&m_clChan[i] == &client)
//					continue;
				if(m_client[i].inUse)
				{
					m_pNet->ChanBegin(i,SV_TALK, len);
					m_pNet->ChanWrite(i,m_client[clNum].name);
					m_pNet->ChanWrite(i,msg);
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
				m_pNet->BroadcastPrintf(-1,"%s renamed to %s", m_client[clNum].name, clname);
				strcpy(m_client[clNum].name, clname);
			}
			else if (id == 'r')
			{
				int rate = buffer.ReadInt();
ComPrintf("SV: %s changed rate to %d\n", m_client[clNum].name, rate);
				//client.m_pNetChan->m_rate =1.0/rate;
				m_pNet->ChanSetRate(clNum,rate);
			}
			break;
		}
	case CL_DISCONNECT:
		{
			m_pNet->SendDisconnect(clNum,0);
		
/*			m_p
			//client.Reset();
			m_svState.numClients --;
*/
			break;
		}
	}
}





void CServer::OnClientSpawn(int clNum)
{
	//Check chanIds to see what client spawned
	m_pNet->BroadcastPrintf(-1, "%s entered the game", m_client[clNum].name);
//	m_client[clNum].name
}

void CServer::OnClientDrop(int clNum, int state, 
					  const char * reason )
{
	m_pNet->BroadcastPrintf(-1,"%s disconnected", m_client[clNum].name);
	m_client[clNum].inUse = false;
	m_svState.numClients --;
}

void CServer::OnLevelChange(int clNum)
{
}

