#include "Net_sock.h"
#include "Net_server.h"
#include "Net_clchan.h"
#include "Net_protocol.h"

using namespace VoidNet;

struct CNetServer::NetChallenge
{
	NetChallenge()	{ challenge = 0;	time = 0.0f;  }
	CNetAddr	addr;
	int			challenge;
	float		time;
};

//======================================================================================
/*
======================================
Constructor/Destructor
======================================
*/
CNetServer::CNetServer()
{	
	m_clChan = new CNetClChan[SV_MAX_CLIENTS];
	m_pSock  = new CNetSocket(&m_recvBuf);
	m_challenges = new NetChallenge[MAX_CHALLENGES];

	m_numSignOnBuffers=0;
	memset(m_printBuffer,0,sizeof(m_printBuffer));
}

void CNetServer::Create(I_Server * server, ServerState * state)
{	
	m_pServer = server;
	m_pSvState = state;
}

CNetServer::~CNetServer()
{	
	m_pSvState = 0;
	m_pServer = 0;
	delete [] m_challenges;
	delete [] m_clChan;
	delete m_pSock;
}

/*
======================================
Initialize the Network Server
======================================
*/
bool CNetServer::Init()
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
	}

	//Default to loopback
	if(boundAddr[0] == '\0')
		strcpy(boundAddr,"127.0.0.1");
	sprintf(m_pSvState->localAddr,"%s:%d",boundAddr,m_pSvState->port);

	//Validate address
	CNetAddr netaddr(m_pSvState->localAddr);
	if(!netaddr.IsValid())
	{
		m_pSvState->localAddr[0] = 0;
		ComPrintf("CNetServer::Init:Unable to resolve ip address\n");
		return false;
	}

	//Save Local Address
	CNetAddr::SetLocalServerAddr(m_pSvState->localAddr);
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
void CNetServer::Shutdown()
{
	//Send disconnect messages to clients, if active
	for(int i=0;i<m_pSvState->maxClients;i++)
	{
		if(m_clChan[i].m_state != CL_SPAWNED) 
			continue;
		SendDisconnect(i,"Server quit");
	}

	//Update State
	m_pSvState->numClients = 0;
	m_pSvState->levelId = 0;
	memset(m_pSvState->worldname,0,sizeof(m_pSvState->worldname));

	m_pSock->Close();
}

/*
======================================
Server will be restarting. Ask all
clients to reconnect
======================================
*/
void CNetServer::Restart()
{
	for(int i=0;i<m_pSvState->maxClients;i++)
	{
		if(m_clChan[i].m_state == CL_SPAWNED)
			SendReconnect(i);
	}
}

//======================================================================================
//OOB Connections
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
	m_sendBuf.Write(m_pSvState->gameName);	//Game
	m_sendBuf.Write(m_pSvState->hostName);	//Hostname
	m_sendBuf.Write(m_pSvState->worldname);	//Map name
	m_sendBuf.Write(m_pSvState->maxClients);	//max clients
	m_sendBuf.Write(m_pSvState->numClients);	//cur clients
	
	m_pSock->Send(m_sendBuf);
}

/*
==========================================
Respond to challenge request
==========================================
*/
void CNetServer::HandleChallengeReq()
{
	if(m_pSvState->numClients >= m_pSvState->maxClients)
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
	int  emptySlot = -1;
	bool reconnect = false;
	for(i=0;i<m_pSvState->maxClients;i++)
	{
		if((emptySlot == -1) && m_clChan[i].m_state == CL_FREE)
			emptySlot = i;
		
		if(m_clChan[i].m_netChan.MatchAddr(m_pSock->GetSource()))
		{
			//Is already connected, ignore dup connected
			if(m_clChan[i].m_state >= CL_CONNECTED)
			{
				ComPrintf("SV:DupConnect from %s\n", m_pSock->GetSource().ToString());
				return;
			}

			//last connection never finished
			m_clChan[i].Reset();
			reconnect = true;
			ComPrintf("SV:Reconnect from %s\n", m_pSock->GetSource().ToString());
			break;
		}
	}

	//Didn't find any duplicates. now find an empty slot
	if(i == m_pSvState->maxClients)
	{
		//Reject if we didnt find a slot
		if(emptySlot == -1)
		{
			SendRejectMsg("Server full");
			return;
		}
		//update client counts
		m_pSvState->numClients ++;		
		i = emptySlot;
	}


	//Initialize the ClientChannel
	m_clChan[i].m_state = CL_CONNECTED;
	m_clChan[i].m_netChan.Setup(m_pSock->GetSource(),&m_recvBuf);

	//Check if game server accepts the client
	if(!m_pServer->ValidateClConnection(i,reconnect,m_recvBuf))
	{
		m_clChan[i].Reset();
		return;
	}

	//All validation complete. This is now a connected client
	//last OOB message, send the client an accept packet
	//now the client needs to request spawn parms from use
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);
	m_sendBuf.Write(S2C_ACCEPT);
	m_sendBuf.Write(m_pSvState->levelId);
	m_pSock->Send(m_sendBuf);
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
//Spawning protocol
//======================================================================================
/*
======================================
Send requested spawn parms
======================================
*/
void CNetServer::SendSpawnParms(int chanId)
{
	m_clChan[chanId].m_netChan.m_buffer.Reset();

	//What spawn level does the client want ?
	switch(m_clChan[chanId].m_spawnState)
	{
	case SVC_INITCONNECTION:
		{
			//Send 1st signon buffer
			m_clChan[chanId].m_netChan.m_buffer.Write(m_signOnBuf[0]);
			break;
		}
	case SVC_MODELLIST:
			m_clChan[chanId].m_netChan.m_buffer.Write(SVC_MODELLIST);
		break;
	case SVC_SOUNDLIST:
			m_clChan[chanId].m_netChan.m_buffer.Write(SVC_SOUNDLIST);
		break;
	case SVC_IMAGELIST:
			m_clChan[chanId].m_netChan.m_buffer.Write(SVC_IMAGELIST);
		break;
	case SVC_BASELINES:
		{
			m_clChan[chanId].m_netChan.m_buffer.Write(SVC_BASELINES);
			break;
		}
	case SVC_BEGIN:
		{
			//consider client to be spawned now
			m_clChan[chanId].m_netChan.m_buffer.Write(SVC_BEGIN);
		}
		break;
	}
ComPrintf("SV:Client(%d) Sending spawn level %d\n", chanId, m_clChan[chanId].m_spawnState);
}

/*
======================================
Received a message from a spawning client
who wants to request the next round of spawn info
======================================
*/
void CNetServer::ParseSpawnMessage(int chanId)	//Client hasn't spawned yet
{
	//Check if client is trying to spawn into current map
	int levelid = m_recvBuf.ReadInt();
	if( levelid != m_pSvState->levelId)
	{
ComPrintf("SV:Client(%d) needs to reconnect, bad levelid %d != %d\n", chanId, levelid ,m_pSvState->levelId);
		SendReconnect(chanId);
		return;
	}

	//Find out what spawn message the client is asking for
	byte spawnparm = m_recvBuf.ReadByte();

ComPrintf("SV:Client(%d) requesting spawn level %d\n", chanId, spawnparm);

	//Client aborted connection
	if(spawnparm == CL_DISCONNECT)
	{
		m_pServer->OnClientDrop(chanId, CL_CONNECTED, "disconnected");
		m_clChan[chanId].Reset();
		m_pSvState->numClients--;
		return;	
	}
	else if(spawnparm == SVC_BEGIN+1)
	{
		m_clChan[chanId].m_state = CL_SPAWNED;
		m_pServer->OnClientSpawn(chanId);
	}
	else
	{
		m_clChan[chanId].m_spawnState = spawnparm;
	}
	m_clChan[chanId].m_bSend = true;
}

//======================================================================================
//======================================================================================

/*
======================================
Print a server message to a given client
======================================
*/
void CNetServer::ClientPrintf(int chanId, const char * message, ...)
{
	va_list args;
	va_start(args, message);
	vsprintf(m_printBuffer, message, args);
	va_end(args);

	if(m_clChan[chanId].m_state == CL_SPAWNED)
	{
		BeginWrite(chanId,SV_PRINT,strlen(m_printBuffer));
		Write(m_printBuffer);
		FinishWrite();
	}
}

/*
======================================
Broadcast message to all the clients
======================================
*/
void CNetServer::BroadcastPrintf(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	vsprintf(m_printBuffer, message, args);
	va_end(args);

	for(int i=0;i<m_pSvState->maxClients;i++)
	{
		if(m_clChan[i].m_state == CL_SPAWNED)
		{
			BeginWrite(i,SV_PRINT,strlen(m_printBuffer));
			Write(m_printBuffer);
			FinishWrite();
		}
	}
}

/*
======================================
Ask client to reconnect
======================================
*/
void CNetServer::SendReconnect(int chanId)
{
	m_clChan[chanId].m_netChan.m_buffer.Reset();
	m_clChan[chanId].m_netChan.m_buffer.Write(SV_RECONNECT);
	m_clChan[chanId].m_netChan.PrepareTransmit();
	m_pSock->SendTo(&m_clChan[chanId].m_netChan);

	m_pServer->OnLevelChange(chanId);

	m_clChan[chanId].m_state = CL_INUSE;
}

/*
======================================
Tell the client to disconnect
======================================
*/
void CNetServer::SendDisconnect(int chanId, const char * reason)
{
	m_clChan[chanId].m_netChan.m_buffer.Reset();
	m_clChan[chanId].m_netChan.m_buffer.Write(SV_DISCONNECT);
	m_clChan[chanId].m_netChan.m_buffer.Write(reason);
	m_clChan[chanId].m_netChan.PrepareTransmit();
	m_pSock->SendTo(&m_clChan[chanId].m_netChan);

	m_pServer->OnClientDrop(chanId,m_clChan[chanId].m_state,reason);

	m_clChan[chanId].Reset();
	m_pSvState->numClients--;
}

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
		for(int i=0;i<m_pSvState->maxClients;i++)
		{
			//match client
			if(m_clChan[i].m_netChan.MatchAddr(m_pSock->GetSource()))
			{
				m_recvBuf.BeginRead();
				m_clChan[i].m_netChan.BeginRead();

				if(m_clChan[i].m_state == CL_SPAWNED)
				{
//ComPrintf("SV: Msg from Spawned client\n");

					m_pServer->HandleClientMsg(i, m_recvBuf);
					m_clChan[i].m_bSend = true;
				}
				//client hasn't spawned yet. is asking for parms
				else if(m_clChan[i].m_state == CL_CONNECTED)
				{
					ParseSpawnMessage(i);
				}
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
void CNetServer::SendPackets()
{
	for(int i=0;i<m_pSvState->maxClients;i++)
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
			if(m_clChan[i].m_netChan.m_lastReceived + SV_TIMEOUT_INTERVAL < System::g_fcurTime)
			{
				SendDisconnect(i,"timed out");
				continue;
			}

			if(m_clChan[i].m_bDropClient)
			{
				SendDisconnect(i,"overflowed");
				continue;
			}
			
			//flag resends if no response to a reliable packet
			m_clChan[i].m_netChan.PrepareTransmit();
			m_pSock->SendTo(&(m_clChan[i].m_netChan));
			//m_clChan[i].m_bSend = false;
//ComPrintf("SV:: writing to spawned client\n");
			continue;
		}
		
		//havent spawned yet. need to send spawn info
		if(m_clChan[i].m_state == CL_CONNECTED)
		{
			SendSpawnParms(i);
			m_clChan[i].m_netChan.PrepareTransmit();
			m_pSock->SendTo(&(m_clChan[i].m_netChan));
			m_clChan[i].m_bSend = false;
		}
	}
}

//======================================================================================
//======================================================================================
/*
==========================================
Initialize/Release Winsock
==========================================
*/
bool CNetServer::InitWinsock()
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

void CNetServer::ShutdownWinsock()
{	WSACleanup();
}

