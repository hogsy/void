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

enum
{
	MAX_CHALLENGES =  512
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

	memset(m_printBuffer,0,sizeof(m_printBuffer));
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
Create the Server
======================================
*/
void CNetServer::Create(I_Server * server, const ServerState * state)
{	
	m_pServer = server;
	m_pSvState = state;
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

	//Assume there will be no more than 10 IP interfaces 
	INTERFACE_INFO localAddr[10];  
	int numAddrs = m_pSock->GetInterfaceList((INTERFACE_INFO **)&localAddr,10);
	if(numAddrs == 0)
	{
		ComPrintf("CNetServer::Init: Unable to get network interfaces\n");
		return false;
	}

	//Pick one of the IP address we found
	SOCKADDR_IN* pAddrInet=0;
	ulong	addrFlags=0;
	char  * pAddrString=0;
	bool	bFoundAddr = false;
	char	szBestAddr[24];
	
	memset(szBestAddr,0,24);
	
	for (int i=0; i<numAddrs; i++) 
	{
		pAddrInet  = (SOCKADDR_IN*)&localAddr[i].iiAddress;
		pAddrString= inet_ntoa(pAddrInet->sin_addr);
		addrFlags  = localAddr[i].iiFlags;
		
		if (!pAddrString)
			continue;
		
		ComPrintf("IP: %s ", pAddrString);
		if(addrFlags & IFF_UP)
		{
			//If we need to bind to a specific address
			if(m_pSvState->localAddr[0] &&
			   (strcmp(m_pSvState->localAddr,pAddrString) ==0))
			{
			   bFoundAddr = true;
			   break;
			}
			
			if(!(addrFlags & IFF_LOOPBACK) && pAddrString[0] != '0')
			{
				strcpy(szBestAddr,pAddrString);
				ComPrintf(": Active\n");
			}
		}
		if (addrFlags & IFF_LOOPBACK)
			ComPrintf(": Loopback\n");
	}


	//If we found theaddress we were looking for then use that
	if(bFoundAddr)
		strcpy(szBestAddr,m_pSvState->localAddr);
	else
	{
		//Say that we couldnt find the address we were supposed to bind to
		if(m_pSvState->localAddr[0])
			ComPrintf("CNetServer::Init: Unable to bind to %s. Defaulting to best address\n", 
															m_pSvState->localAddr);
		//Default to loopback if we couldnt find any online addresses
		if(szBestAddr[0] == '\0')
			strcpy(szBestAddr,"127.0.0.1");
	}

	sprintf(m_szLocalAddr,"%s:%d",szBestAddr,m_pSvState->port);

	//Validate address
	CNetAddr netaddr(m_szLocalAddr);
	if(!netaddr.IsValid())
	{
		ComPrintf("CNetServer::Init: Unable to resolve ip address: %s\n", m_szLocalAddr);
		memset(m_szLocalAddr,0,sizeof(m_szLocalAddr));
		return false;
	}

	//Save Local Address
	CNetAddr::SetLocalServerAddr(m_szLocalAddr);

	//Bind Socket
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
		if(m_clChan[i].m_state != CL_INGAME) 
			continue;
		SendDisconnect(i,SERVER_QUIT);
	}
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
		if(m_clChan[i].m_state == CL_INGAME)
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
void CNetServer::HandleStatusReq(bool full)
{
	//Header
	m_sendBuf.Reset();
	m_sendBuf.Write(-1);

	if(!full)
		m_sendBuf.Write(S2C_STATUS);
	else
		m_sendBuf.Write(S2C_FULLSTATUS);

	//Status info
	m_sendBuf.Write(VOID_PROTOCOL_VERSION);	//Protocol
	m_sendBuf.Write(m_pSvState->gameName);	//Game
	m_sendBuf.Write(m_pSvState->hostName);	//Hostname
	m_sendBuf.Write(m_pSvState->worldname);	//Map name
	m_sendBuf.Write(m_pSvState->maxClients);//max clients
	m_sendBuf.Write(m_pSvState->numClients);//cur clients
	
	if(full)
		m_pServer->WriteGameStatus(m_sendBuf);

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
	else if(strcmp(msg, C2S_GETFULLSTATUS) == 0)
		HandleStatusReq(true);
	else if(strcmp(msg, C2S_GETSTATUS) == 0)
		HandleStatusReq(false);
	else if(strcmp(msg, C2S_CONNECT) == 0)
		HandleConnectReq();
	else if(strcmp(msg, C2S_GETCHALLENGE) == 0)
		HandleChallengeReq();
}


//======================================================================================
//Spawning protocol
//VERY messy right now
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
			m_clChan[chanId].m_netChan.m_buffer.Write(m_signOnBufs.gameInfo);
//			m_pSock->SendTo(m_signOnBufs.gameInfo, m_clChan[chanId].m_netChan.GetAddr());
//			return;
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
		m_pServer->OnClientDrop(chanId, CLIENT_QUIT);
		m_clChan[chanId].Reset();
		return;	
	}
	else if(spawnparm == SVC_BEGIN+1)
	{
		m_clChan[chanId].m_state = CL_INGAME;
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

	if(m_clChan[chanId].m_state == CL_INGAME)
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
		if(m_clChan[i].m_state == CL_INGAME)
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
void CNetServer::SendDisconnect(int chanId, EDisconnectReason reason) 
{
	m_clChan[chanId].m_netChan.m_buffer.Reset();
	m_clChan[chanId].m_netChan.m_buffer.Write(SV_DISCONNECT);

	switch(reason)
	{
	case SERVER_QUIT:
		m_clChan[chanId].m_netChan.m_buffer.Write("Server quit");
		break;
	case CLIENT_TIMEOUT:
		m_clChan[chanId].m_netChan.m_buffer.Write("Timed out");
		break;
	case CLIENT_OVERFLOW:
		m_clChan[chanId].m_netChan.m_buffer.Write("Overflowed");
		break;
	}
	
	m_clChan[chanId].m_netChan.PrepareTransmit();
	m_pSock->SendTo(&m_clChan[chanId].m_netChan);

	m_pServer->OnClientDrop(chanId,reason);

	m_clChan[chanId].Reset();
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

				if(m_clChan[i].m_state == CL_INGAME)
					m_pServer->HandleClientMsg(i, m_recvBuf);
				else if(m_clChan[i].m_state == CL_CONNECTED)
					ParseSpawnMessage(i);

				m_clChan[i].m_bSend = true;
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
		if(m_clChan[i].m_state < CL_CONNECTED)
			continue;

		//Will fail if we didnt receive a packet from 
		//this client this frame, or if channels chokes
		if(!m_clChan[i].ReadyToSend())
			continue;

		//In game clients
		if(m_clChan[i].m_state == CL_INGAME)
		{
			//Check timeout
			if(m_clChan[i].m_netChan.m_lastReceived + SV_TIMEOUT_INTERVAL < System::g_fcurTime)
			{
				SendDisconnect(i,CLIENT_TIMEOUT);
				continue;
			}

			//Check overflow
			if(m_clChan[i].m_bDropClient)
			{
				SendDisconnect(i,CLIENT_OVERFLOW);
				continue;
			}
			
			//flag resends if no response to a reliable packet
			m_clChan[i].m_netChan.PrepareTransmit();
			m_pSock->SendTo(&m_clChan[i].m_netChan);
			m_clChan[i].m_bSend = false;
//ComPrintf("SV:: writing to spawned client\n");
			continue;
		}
		
		//havent spawned yet. need to send spawn info
		if(m_clChan[i].m_state == CL_CONNECTED)
		{
			SendSpawnParms(i);
			m_clChan[i].m_netChan.PrepareTransmit();
			m_pSock->SendTo(&m_clChan[i].m_netChan);
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

