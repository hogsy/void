#include "Net_sock.h"
#include "Net_server.h"

using namespace VoidNet;

//======================================================================================
//======================================================================================

struct CNetServer::NetChallenge
{
	NetChallenge()	{ challenge = 0;	time = 0.0f;  }
	CNetAddr	addr;
	int			challenge;
	float		time;
};
	
/*
======================================
Constructor/Destructor
======================================
*/
CNetServer::CNetServer()
{
	m_challenges = new NetChallenge[MAX_CHALLENGES];
	m_pSock = new CNetSocket(&m_recvBuf);

	m_numSignOnBuffers=0;
	memset(m_printBuffer,0,sizeof(m_printBuffer));
}

CNetServer::~CNetServer()
{	
	delete [] m_challenges;
	delete m_pSock;
}


/*
==========================================
Initialize/Release Winsock
==========================================
*/
bool CNetServer::InitNetwork()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err; 
	
	wVersionRequested = MAKEWORD( 2, 0 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	
	if (err) 
	{
		ComPrintf("CServer::InitNetwork:Error: WSAStartup Failed\n");
		return false;
	} 
	
	//Confirm Version
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0 ) 
	{
		WSACleanup();
		return false; 
	}  
	return true;
}

void CNetServer::ShutdownNetwork()
{	WSACleanup();
}

/*
======================================
Initialize the Network Server
======================================
*/
bool CNetServer::NetInit()
{
	if(!m_pSock->Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, false))
	{
		PrintSockError(WSAGetLastError(),"CNetServer::Init: Couldnt create socket");
		return false;
	}

	// Assume there will be no more than 10 IP interfaces 
	int numAddrs = 0;
	INTERFACE_INFO localAddr[10];  
	
	numAddrs = m_pSock->GetInterfaceList((INTERFACE_INFO **)&localAddr,10);
	if(numAddrs == 0)
	{
		ComPrintf("CNetServer::Init: Unable to get network interfaces\n");
		return false;
	}

	//Pick one of the IP addresses we found
	ulong		 addrFlags=0;
	char	   * pAddrString=0;
	SOCKADDR_IN* pAddrInet=0;
	char		 boundAddr[24];
	
	memset(boundAddr,0,24);
	
	for (int i=0; i<numAddrs; i++) 
	{
		pAddrInet = (SOCKADDR_IN*)&localAddr[i].iiAddress;
		pAddrString = inet_ntoa(pAddrInet->sin_addr);
		
		if (pAddrString)
			ComPrintf("IP: %s ", pAddrString);
		
		addrFlags = localAddr[i].iiFlags;
		if (addrFlags & IFF_UP)
		{
			//if(!(addrFlags & IFF_LOOPBACK))
			if(!(addrFlags & IFF_LOOPBACK) && pAddrString[0] != '0')
			{
				strcpy(boundAddr,pAddrString);
				ComPrintf(": Active\n");
			}
		}
		if (addrFlags & IFF_LOOPBACK)
			ComPrintf(": Loopback\n");
//		if (addrFlags & IFF_POINTTOPOINT)
//			ComPrintf(". this is a point-to-point link");
	}

	//Default to loopback
	if(boundAddr[0] == '\0')
		strcpy(boundAddr,"127.0.0.1");
	sprintf(m_svState.localAddr,"%s:%d",boundAddr,m_svState.port);

	//Validate address
	CNetAddr netaddr(m_svState.localAddr);
	if(!netaddr.IsValid())
	{
		m_svState.localAddr[0] = 0;
		ComPrintf("CNetServer::Init:Unable to resolve ip address\n");
		return false;
	}

	//Save Local Address
	CNetAddr::SetLocalServerAddr(m_svState.localAddr);
	if(!m_pSock->Bind(netaddr))
	{
		ComPrintf("CNetServer::Init:Unable to bind socket\n");
		return false;
	}
	return true;
}

/*
======================================
Server is shutting down. 
Send disconnects to clients
======================================
*/
void CNetServer::NetShutdown()
{
	//Send disconnect messages to clients, if active
	for(int i=0;i<m_svState.maxClients;i++)
	{
		if(m_clChan[i].m_state != CL_SPAWNED) 
			continue;
		SendDisconnect(m_clChan[i],"Server quit");
	}

	//Update State
	m_svState.numClients = 0;
	m_svState.levelId = 0;

	m_pSock->Close();
}


/*
======================================
Server will be restarting. Ask all
clients to reconnect
======================================
*/
void CNetServer::NetRestart()
{
	for(int i=0;i<m_svState.maxClients;i++)
	{
		if(m_clChan[i].m_state == CL_SPAWNED)
			SendReconnect(m_clChan[i]);
	}
}

/*
======================================
Print network server state
======================================
*/
void CNetServer::NetPrintStatus()
{
	ComPrintf("Ip address  : %s\n", CNetAddr::GetLocalServerAddr());
	ComPrintf("Map name    : %s\n", m_svState.worldname);
	ComPrintf("Num Clients : %d\n========================\n", m_svState.numClients);
	for(int i=0; i<m_svState.numClients; i++)
	{
		if(m_clChan[i].m_state == CL_CONNECTED)
			ComPrintf("%s : Connecting\n", m_clChan[i].m_name);
		else if(m_clChan[i].m_state == CL_SPAWNED)
		{
			ComPrintf("%s :", m_clChan[i].m_name);
			m_clChan[i].m_pNetChan->PrintStats();
		}
//			ComPrintf("%s: Rate %.2f: Chokes %d\n", m_clChan[i].m_name, 
//				1/m_clChan[i].m_pNetChan->m_rate, m_clChan[i].m_pNetChan->m_numChokes);
	}
}

//======================================================================================
//OOB protocol
//======================================================================================

/*
======================================
Send a rejection message to the client
======================================
*/
void CNetServer::SendRejectMsg(const char * reason)
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
void CNetServer::HandleStatusReq()
{
	//Header
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);
	m_sendBuf.Write(S2C_STATUS);

	//Status info
	m_sendBuf.Write(VOID_PROTOCOL_VERSION);	//Protocol
	m_sendBuf.Write(m_svState.gameName);	//Game
	m_sendBuf.Write(m_svState.hostName);	//Hostname
	m_sendBuf.Write(m_svState.worldname);	//Map name
	m_sendBuf.Write(m_svState.maxClients);	//max clients
	m_sendBuf.Write(m_svState.numClients);//cur clients
	
	m_pSock->Send(m_sendBuf);
}

/*
==========================================
Respond to challenge request
==========================================
*/
void CNetServer::HandleChallengeReq()
{
	if(m_svState.numClients >= m_svState.maxClients)
	{
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
void CNetServer::HandleConnectReq()
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
	int emptySlot = -1;
	for(i=0;i<m_svState.maxClients;i++)
	{
		if((emptySlot == -1) && m_clChan[i].m_state == CL_FREE)
			emptySlot = i;
		
		if(m_clChan[i].m_pNetChan->MatchAddr(m_pSock->GetSource()))
		{
			//Is already connected, ignore dup connected
			if(m_clChan[i].m_state >= CL_CONNECTED)
			{
				ComPrintf("SV:DupConnect from %s\n", m_pSock->GetSource().ToString());
				return;
			}

			//last connection never finished
			m_clChan[i].Reset();
			ComPrintf("SV:Reconnect from %s\n", m_pSock->GetSource().ToString());
			break;
		}
	}

	//Didn't find any duplicates. now find an empty slot
	if(i == m_svState.maxClients)
	{
		//Reject if we didnt find a slot
		if(emptySlot == -1)
		{
			SendRejectMsg("Server full");
			return;
		}
		//update client counts
		m_svState.numClients ++;		
		i = emptySlot;
	}

	//We now have a new client slot. create it
	strcpy(m_clChan[i].m_name, m_recvBuf.ReadString());
	m_clChan[i].m_state = CL_CONNECTED;
	m_clChan[i].m_pNetChan->Setup(m_pSock->GetSource(),&m_recvBuf);
	m_clChan[i].m_pNetChan->SetRate(m_recvBuf.ReadInt());

	//last OOB message, send the client an accept packet
	//now the client needs to request spawn parms from use
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);
	m_sendBuf.Write(S2C_ACCEPT);
	m_sendBuf.Write(m_svState.levelId);
	m_pSock->Send(m_sendBuf);

ComPrintf("SV: %s connected\n",m_clChan[i].m_name) ;
}

/*
==========================================
Process an OOB Server Query message
==========================================
*/
void CNetServer::ProcessQueryPacket()
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
void CNetServer::SendSpawnParms(CClientChan &client)
{
	client.m_pNetChan->m_buffer.Reset();

	//What spawn level does the client want ?
	switch(client.m_spawnState)
	{
	case SVC_INITCONNECTION:
		{
			//Send 1st signon buffer
			client.m_pNetChan->m_buffer.Write(m_signOnBuf[0]);
			break;
		}
	case SVC_MODELLIST:
			client.m_pNetChan->m_buffer.Write(SVC_MODELLIST);
		break;
	case SVC_SOUNDLIST:
			client.m_pNetChan->m_buffer.Write(SVC_SOUNDLIST);
		break;
	case SVC_IMAGELIST:
			client.m_pNetChan->m_buffer.Write(SVC_IMAGELIST);
		break;
	case SVC_BASELINES:
		{
			client.m_pNetChan->m_buffer.Write(SVC_BASELINES);
			break;
		}
	case SVC_BEGIN:
		{
			//consider client to be spawned now
			client.m_pNetChan->m_buffer.Write(SVC_BEGIN);
		}
		break;
	}
	client.m_pNetChan->PrepareTransmit();
//ComPrintf("SV: Sending spawn parms %d\n",client.m_spawnState);
}


/*
======================================
Received a message from a spawning client
who wants to request the next round of spawn info
======================================
*/
void CNetServer::ParseSpawnMessage(CClientChan &client)
{
	//Check if client is trying to spawn into current map
	int levelid = m_recvBuf.ReadInt();
	
	if( levelid != m_svState.levelId)
	{
ComPrintf("SV: Client needs to reconnect, bad levelid %d != %d\n", levelid ,m_svState.levelId);
		SendReconnect(client);
		return;
	}

	//Find out what spawn message the client is asking for
	byte spawnparm = m_recvBuf.ReadByte();
	if(spawnparm == CL_DISCONNECT)
	{
		BroadcastPrintf(0,"%s disconnected", client.m_name);
		client.Reset();
		m_svState.numClients--;
		return;	
	}
	else if(spawnparm == SVC_BEGIN+1)
	{
ComPrintf("SV:%s entered the game\n", client.m_name);
		client.m_state = CL_SPAWNED;
		BroadcastPrintf(0,"%s entered the game", client.m_name);
	}
	else
	{
ComPrintf("SV: Client requesting spawn level %d\n", spawnparm);
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
void CNetServer::ClientPrintf(CClientChan &client, const char * message, ...)
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
void CNetServer::BroadcastPrintf(const CClientChan * client, const char* message, ...)
{
	va_list args;
	va_start(args, message);
	vsprintf(m_printBuffer, message, args);
	va_end(args);

	for(int i=0;i<m_svState.maxClients;i++)
	{
		if(&m_clChan[i] == client)
			continue;

		if(m_clChan[i].m_state == CL_SPAWNED)
		{
			m_clChan[i].BeginMessage(SV_PRINT,strlen(m_printBuffer));
			m_clChan[i].WriteString(m_printBuffer);
		}
	}
}

/*
======================================
Tell the client to disconnect
======================================
*/
void CNetServer::SendDisconnect(CClientChan &client, const char * reason)
{
	client.m_pNetChan->m_buffer.Reset();
	client.m_pNetChan->m_buffer.Write(SV_DISCONNECT);
	client.m_pNetChan->m_buffer.Write(reason);
	client.m_pNetChan->PrepareTransmit();

	m_pSock->SendTo(client.m_pNetChan);
	client.Reset();

	m_svState.numClients--;
}

/*
======================================
Ask client to reconnect
======================================
*/
void CNetServer::SendReconnect(CClientChan &client)
{
	client.m_pNetChan->m_buffer.Reset();
	client.m_pNetChan->m_buffer.Write(SV_RECONNECT);
	client.m_pNetChan->PrepareTransmit();
	
	m_pSock->SendTo(client.m_pNetChan);
	client.m_state = CL_INUSE;
}

/*
======================================
Parse client message
======================================
*/
void CNetServer::ParseClientMessage(CClientChan &client)
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
			//for(int i=0; i<m_cMaxClients.ival;i++)
			for(int i=0;i<m_svState.maxClients;i++)
			{
				//dont send to source
				if(&m_clChan[i] == &client)
					continue;

				if(m_clChan[i].m_state == CL_SPAWNED)
				{
					m_clChan[i].BeginMessage(SV_TALK,len);
					m_clChan[i].WriteString(client.m_name);
					m_clChan[i].WriteString(msg);
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
				//client.m_pNetChan->m_rate =1.0/rate;
				client.m_pNetChan->SetRate(rate);
			}
			break;
		}
	case CL_DISCONNECT:
		{
			BroadcastPrintf(0,"%s disconnected", client.m_name);
			client.Reset();
			m_svState.numClients --;
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
void CNetServer::ReadPackets()
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
		for(int i=0;i<m_svState.maxClients;i++)
		{
			//match client
			if(m_clChan[i].m_pNetChan->MatchAddr(m_pSock->GetSource()))
			{
				m_recvBuf.BeginRead();
				m_clChan[i].m_pNetChan->BeginRead();

				if(m_clChan[i].m_state == CL_SPAWNED)
				{
//ComPrintf("SV: Msg from Spawned client\n");
					ParseClientMessage(m_clChan[i]);
					m_clChan[i].m_bSend = true;
				}
				//client hasn't spawned yet. is asking for parms
				else if(m_clChan[i].m_state == CL_CONNECTED)
					ParseSpawnMessage(m_clChan[i]);
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
void CNetServer::WritePackets()
{
/*	static float lastTime = 0.0f;
	if(lastTime > System::g_fcurTime)
		return;

	lastTime = System::g_fcurTime + 1.0/30.0;
*/
	for(int i=0;i<m_svState.maxClients;i++)
	{
		if(m_clChan[i].m_state == CL_FREE)
			continue;

		//Will fail if we didnt receive a packet from 
		//this client this frame, or if channels chokes
		if(!m_clChan[i].ReadyToSend())
		{
//ComPrintf("SV:not ready to send\n");
			continue;
		}

		//In game clients
		if(m_clChan[i].m_state == CL_SPAWNED)
		{
			//Check timeouts and overflows here
			if(m_clChan[i].m_pNetChan->m_lastReceived + SV_TIMEOUT_INTERVAL < System::g_fcurTime)
			{
				BroadcastPrintf(&m_clChan[i],"%s timed out", m_clChan[i].m_name);
				SendDisconnect(m_clChan[i],"Timed out");
				continue;
			}

			if(m_clChan[i].m_bDropClient)
			{
				BroadcastPrintf(&m_clChan[i],"%s overflowed", m_clChan[i].m_name);
				SendDisconnect(m_clChan[i],"Overflowed");
				continue;
			}
			//flag resends if no response to a reliable packet

			m_clChan[i].m_pNetChan->PrepareTransmit();
			m_pSock->SendTo(m_clChan[i].m_pNetChan);
			//m_clChan[i].m_bSend = false;
//ComPrintf("SV:: writing to spawned client\n");
			continue;
		}
		
		//havent spawned yet. need to send spawn info
		if(m_clChan[i].m_state == CL_CONNECTED)
		{
			SendSpawnParms(m_clChan[i]);
			m_pSock->SendTo(m_clChan[i].m_pNetChan);
			m_clChan[i].m_bSend = false;
		}
	}
}
