#include "Net_sock.h"
#include "Sv_main.h"
#include "Net_defs.h"

using namespace VoidNet;

//======================================================================================
//OOB query protocol
//======================================================================================
/*
======================================
Send a rejection message to the client
======================================
*/
void CServer::SendRejectMsg(const char * reason)
{
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);
	m_sendBuf.Write(S2C_REJECT);
	m_sendBuf.Write(reason);
	m_pSock->Send(m_sendBuf);
}
/*
======================================
Handle a status request
======================================
*/
void CServer::HandleStatusReq()
{
	//Header
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);
	m_sendBuf.Write(S2C_STATUS);

	//Status info
	m_sendBuf.Write(VOID_PROTOCOL_VERSION);	//Protocol
	m_sendBuf.Write(m_cGame.string);		//Game
	m_sendBuf.Write(m_cHostname.string);	//Hostname
	m_sendBuf.Write(m_worldName);			//Map name
	m_sendBuf.Write(m_numClients);			//cur clients
	m_sendBuf.Write(m_cMaxClients.ival);	//max clients
	
	m_pSock->Send(m_sendBuf);
}

/*
==========================================
Respond to challenge request
==========================================
*/
void CServer::HandleChallengeReq()
{
	if(m_numClients >= m_cMaxClients.ival)
	{
ComPrintf("SV: Rejecting, server full\n");
		SendRejectMsg("Server is full");
		return;
	}

	int	  oldestchallenge = 0;
	float oldesttime  = 0.0f;

	for(int i=0; i< MAX_CHALLENGES; i++)
	{
		//Found a match, we already got a request from this guy
		if(m_challenges[i].addr == m_pSock->GetSource())
			break;
		
		//Keep a track of what the oldest challenge is
		if(m_challenges[i].time > oldesttime)
		{
			oldestchallenge = i;
			oldesttime = m_challenges[i].time;
		}
	}

	//Didn't find any old challenges from the same addy
	if (i == MAX_CHALLENGES)
	{
		// overwrite the oldest
		i = oldestchallenge;
		m_challenges[i].challenge = (rand() << 16) ^ rand();
		m_challenges[i].addr = m_pSock->GetSource();
		m_challenges[i].time = System::g_fcurTime;
	}

	//Send response packet
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);
	m_sendBuf.Write(S2C_CHALLENGE);
	m_sendBuf.Write(m_challenges[i].challenge);
	m_pSock->SendTo(m_sendBuf, m_challenges[i].addr); 
}

/*
======================================
Handle a connection request
======================================
*/
void CServer::HandleConnectReq()
{
	//Validate Protocol Version
	int protocolVersion = m_recvBuf.ReadInt();
	if(protocolVersion != VOID_PROTOCOL_VERSION)
	{
		char msg[64];
		sprintf(msg,"Bad Protocol version. Need %d, not %d", 
						VOID_PROTOCOL_VERSION, protocolVersion);
		SendRejectMsg(msg);
		return;
	}

	//Validate Challenge
	int challenge = m_recvBuf.ReadInt();
	for(int i=0; i< MAX_CHALLENGES; i++)
	{
		if((m_challenges[i].addr == m_pSock->GetSource()) &&
		   (m_challenges[i].challenge = challenge))
		   break;
	}
	if(i == MAX_CHALLENGES)
	{
		SendRejectMsg("Bad Challenge");
		return;
	}

	//Check if this client is already connected
	for(i=0;i<m_cMaxClients.ival;i++)
	{
		if(m_clients[i].m_netChan.m_addr == m_pSock->GetSource())
		{
			//Is connected, ignore dup connected
			if(m_clients[i].m_state == CL_CONNECTED)
			{
ComPrintf("SV:DupConnect from %s\n", m_pSock->GetSource().ToString());
				return;
			}
			
			//last connection never finished
			m_clients[i].Reset();
ComPrintf("SV:Reconnect from %s\n", m_pSock->GetSource().ToString());
			break;
		}
	}

	//Didn't find any duplicates. now find an empty slot
	if(i == m_cMaxClients.ival)
	{
		for(i=0;i<m_cMaxClients.ival; i++)
		{
			if(m_clients[i].m_state == CL_FREE)
				break;
		}
		
		//Reject if we didnt find a slot
		if(i == m_cMaxClients.ival)
		{
			SendRejectMsg("Server full");
			return;
		}

		//update client counts
		m_numClients ++;
	}

	//We now have a new client slot. create it
	strcpy(m_clients[i].m_name, m_recvBuf.ReadString());
	m_clients[i].m_state = CL_CONNECTED;
	m_clients[i].m_netChan.Setup(m_pSock->GetSource(),&m_recvBuf);
	m_clients[i].m_netChan.SetRate(m_recvBuf.ReadInt());
	m_clients[i].m_netChan.m_outMsgId = 1;	//we send packet1 when we receive

	//This is the last connectionless message
	//Send the client an accept packet
	//now the client needs to call us to get spawn parms etc
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);
	m_sendBuf.Write(S2C_ACCEPT);
	m_sendBuf.Write(m_levelNum);
	m_pSock->Send(m_sendBuf);

ComPrintf("SV: %s connected\n",m_clients[i].m_name) ;
}

/*
==========================================
Process an OOB Server Query message
==========================================
*/
void CServer::ProcessQueryPacket()
{
	char * msg = m_recvBuf.ReadString();
	
	//Ping Request
	if(strcmp(msg, VNET_PING) == 0)
		m_pSock->Send((byte*)VNET_PING, strlen(VNET_PING));
	else if(strcmp(msg, C2S_GETSTATUS) == 0)
		HandleStatusReq();
	else if(strcmp(msg, C2S_CONNECT) == 0)
		HandleConnectReq();
	else if(strcmp(msg, C2S_GETCHALLENGE) == 0)
		HandleChallengeReq();
}


//======================================================================================
//Connection protocol
//======================================================================================
/*
======================================
Send requested spawn parms
======================================
*/
void CServer::SendSpawnParms(SVClient &client)
{
	client.m_netChan.m_buffer.Reset();

	//What spawn level does the client want ?
	switch(client.m_spawnState)
	{
	case SVC_INITCONNECTION:
		{
			//Send 1st signon buffer
			client.m_netChan.m_buffer.Write(m_signOnBuf[0]);
			break;
		}
	case SVC_MODELLIST:
			client.m_netChan.m_buffer.Write(SVC_MODELLIST);
		break;
	case SVC_SOUNDLIST:
			client.m_netChan.m_buffer.Write(SVC_SOUNDLIST);
		break;
	case SVC_IMAGELIST:
			client.m_netChan.m_buffer.Write(SVC_IMAGELIST);
		break;
	case SVC_BASELINES:
		{
			client.m_netChan.m_buffer.Write(SVC_BASELINES);
			break;
		}
	case SVC_BEGIN:
		{
			//consider client to be spawned now
			client.m_netChan.m_buffer.Write(SVC_BEGIN);
		}
		break;
	}
	client.m_netChan.PrepareTransmit();
//ComPrintf("SV: Sending spawn parms %d\n",client.m_spawnState);
}


/*
======================================
Received a message from a spawning client
who wants to request the next round of spawn info
======================================
*/
void CServer::ParseSpawnMessage(SVClient &client)
{
	//Check if client is trying to spawn into current map
	int levelid = m_recvBuf.ReadInt();
	
	if( levelid != m_levelNum)
	{
//ComPrintf("SV: Client needs to reconnect, bad levelid %d != %d\n", levelid ,m_levelNum);
		SendReconnect(client);
		return;
	}

	//Find out what spawn message the client is asking for
	byte spawnparm = m_recvBuf.ReadByte();
	if(spawnparm == CL_DISCONNECT)
	{
		BroadcastPrintf(0,"%s disconnected", client.m_name);
		client.Reset();
		m_numClients--;
		return;	
	}
	else if(spawnparm == SVC_BEGIN+1)
	{
//ComPrintf("SV:%s entered the game\n", m_clients[i].m_name);
		client.m_state = CL_SPAWNED;
		BroadcastPrintf(0,"%s entered the game", client.m_name);
	}
	else
	{
//ComPrintf("SV: Client requesting spawn level %d\n", m_clients[i].m_spawnState);
		client.m_spawnState = spawnparm;
	}
	client.m_bSend = true;
}


//======================================================================================
//Game protocol
//======================================================================================
/*
======================================
Print the message to the given client
======================================
*/
void CServer::ClientPrintf(SVClient &client, const char * message, ...)
{
	va_list args;
	va_start(args, message);
	vsprintf(m_printBuffer, message, args);
	va_end(args);

	if(client.m_state == CL_SPAWNED)
	{
		client.BeginMessage(SV_PRINT,strlen(m_printBuffer));
		client.WriteString(m_printBuffer);
	}
}

/*
======================================
Print the message to all the clients
except the given one
======================================
*/
void CServer::BroadcastPrintf(const SVClient * client, const char* message, ...)
{
	va_list args;
	va_start(args, message);
	vsprintf(m_printBuffer, message, args);
	va_end(args);

	for(int i=0;i<m_cMaxClients.ival;i++)
	{
		if(&m_clients[i] == client)
			continue;

		if(m_clients[i].m_state == CL_SPAWNED)
		{
			m_clients[i].BeginMessage(SV_PRINT,strlen(m_printBuffer));
			m_clients[i].WriteString(m_printBuffer);
		}
	}
}

/*
======================================
Tell the client to disconnect
======================================
*/
void CServer::SendDisconnect(SVClient &client, const char * reason)
{
	client.m_netChan.m_reliableBuffer.Reset();
	client.m_netChan.m_buffer.Reset();
	client.m_netChan.m_buffer.Write(SV_DISCONNECT);
	client.m_netChan.m_buffer.Write(reason);
	client.m_netChan.PrepareTransmit();
	m_pSock->SendTo(client.m_netChan.m_sendBuffer, client.m_netChan.m_addr);
	client.Reset();
}

/*
======================================
Ask client to reconnect
======================================
*/
void CServer::SendReconnect(SVClient &client)
{
	client.m_netChan.m_reliableBuffer.Reset();
	client.m_netChan.m_buffer.Reset();
	client.m_netChan.m_buffer.Write(SV_RECONNECT);
	client.m_netChan.PrepareTransmit();
	m_pSock->SendTo(client.m_netChan.m_sendBuffer, client.m_netChan.m_addr);
	client.m_state = CL_INUSE;
}

/*
======================================
Parse client message
======================================
*/
void CServer::ParseClientMessage(SVClient &client)
{
	//Check packet id to see what the client send
	byte packetId = m_recvBuf.ReadByte();
	switch(packetId)
	{
	//Talk message
	case CL_TALK:
		{
//ComPrintf("SV:%s : %s\n", client.m_name, msg);
			char msg[256];
			strcpy(msg,m_recvBuf.ReadString());
			int len = strlen(msg);
			msg[len] = 0;
			len += strlen(client.m_name);
			//Add this to all other connected clients outgoing buffers
			for(int i=0; i<m_cMaxClients.ival;i++)
			{
				//dont send to source
				if(&m_clients[i] == &client)
					continue;

				if(m_clients[i].m_state == CL_SPAWNED)
				{
					m_clients[i].BeginMessage(SV_TALK,len);
					m_clients[i].WriteString(client.m_name);
					m_clients[i].WriteString(msg);
				}
			}
			break;	
		}
	//client updating its local info
	case CL_UPDATEINFO:
		{
			char id = m_recvBuf.ReadChar();
			if(id == 'n')
			{
				const char * clname = m_recvBuf.ReadString();
				BroadcastPrintf(0,"%s renamed to %s", client.m_name, clname);
				strcpy(client.m_name, clname);
			}
			else if (id == 'r')
			{
				int rate = m_recvBuf.ReadInt();
ComPrintf("SV: %s changed rate to %d\n", client.m_name, rate);
				client.m_netChan.m_rate =1.0/rate;
			}
			break;
		}
	case CL_DISCONNECT:
		{
			BroadcastPrintf(0,"%s disconnected", client.m_name);
			client.Reset();
			m_numClients --;
			break;
		}
	}
}


//======================================================================================
//======================================================================================

/*
==========================================
Read any waiting packets
==========================================
*/
void CServer::ReadPackets()
{
	while(m_pSock->RecvFrom())
	{
		//Check if its an OOB message
		if(m_recvBuf.ReadInt() == -1)
		{
//ComPrintf("Query from: %s\n", m_pSock->GetSource().ToString());
			ProcessQueryPacket();
			continue;
		}

		//then it should belong to a client
		for(int i=0; i<m_cMaxClients.ival;i++)
		{
			//match client
			if(m_clients[i].m_netChan.m_addr == m_pSock->GetSource())
			{
				m_recvBuf.BeginRead();
				m_clients[i].m_netChan.BeginRead();

				if(m_clients[i].m_state == CL_SPAWNED)
				{
//ComPrintf("SV: Msg from Spawned client\n");
					ParseClientMessage(m_clients[i]);
					m_clients[i].m_bSend = true;
				}
				//client hasn't spawned yet. is asking for parms
				else if(m_clients[i].m_state == CL_CONNECTED)
					ParseSpawnMessage(m_clients[i]);
				break;
			}
		}
//ComPrintf("SV: unknown packet from %s\n", m_pSock->GetSource().ToString());
	}
}

/*
======================================
Send updates to clients
======================================
*/
void CServer::WritePackets()
{
	for(int i=0; i<m_cMaxClients.ival;i++)
	{
		if(m_clients[i].m_state == CL_FREE)
			continue;

		//Will fail if we didnt receive a packet from 
		//this client this frame, or if channels chokes
		if(!m_clients[i].ReadyToSend())
			continue;

		//In game clients
		if(m_clients[i].m_state == CL_SPAWNED)
		{
			//Check timeouts and overflows here
			if(m_clients[i].m_netChan.m_lastReceived + SV_TIMEOUT_INTERVAL < System::g_fcurTime)
			{
				BroadcastPrintf(&m_clients[i],"%s timed out", m_clients[i].m_name);
				SendDisconnect(m_clients[i],"Timed out");
				m_clients[i].Reset();
				m_numClients --;
				continue;
			}

			if(m_clients[i].m_bDropClient)
			{
				BroadcastPrintf(&m_clients[i],"%s overflowed", m_clients[i].m_name);
				SendDisconnect(m_clients[i],"Overflowed");
				m_clients[i].Reset();
				m_numClients --;
				continue;
			}

			//flag resends if no response to a reliable packet

			m_clients[i].m_netChan.PrepareTransmit();
			m_pSock->SendTo(m_clients[i].m_netChan.m_sendBuffer, m_clients[i].m_netChan.m_addr);
			//m_clients[i].m_bSend = false;
//ComPrintf("SV:: writing to spawned client\n");
			continue;
		}
		
		//havent spawned yet. need to send spawn info
		if(m_clients[i].m_state == CL_CONNECTED)
		{
			SendSpawnParms(m_clients[i]);
			m_pSock->SendTo(m_clients[i].m_netChan.m_sendBuffer, m_clients[i].m_netChan.m_addr);
			m_clients[i].m_bSend = false;
		}
	}
}
