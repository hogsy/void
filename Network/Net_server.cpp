#include "Net_hdr.h"
#include "Net_sock.h"
#include "Net_chan.h"
#include "Net_clchan.h"
#include "Net_protocol.h"
#include "Net_server.h"

using namespace VoidNet;

struct CNetServer::NetChallenge
{
	NetChallenge()	{ challenge = 0;	time = 0.0f;  }
	CNetAddr	addr;
	int			challenge;
	float		time;
};

enum
{	MAX_CHALLENGES =  512
};

//======================================================================================
/*
======================================
Constructor/Destructor
======================================
*/
CNetServer::CNetServer()
{	
	m_pSock  = new CNetSocket(&m_recvBuf);
	m_challenges = new NetChallenge[MAX_CHALLENGES];
	m_clChan = 0;
	m_pMultiCast = 0;
}

CNetServer::~CNetServer()
{	
	m_pSvState = 0;
	m_pServer = 0;
	m_pMultiCast = 0;
	if(m_challenges)
		delete [] m_challenges;
	if(m_clChan)
		delete [] m_clChan;
	if(m_pSock)
		delete m_pSock;
}

/*
======================================
Initialize the Network Server
======================================
*/
bool CNetServer::Init(I_Server * server, const ServerState * state)
{
	m_pServer = server;
	m_pSvState = state;

	if(m_pSvState->maxClients < 0 || m_pSvState->maxClients > NET_MAXCLIENTS)
	{
		ComPrintf("CNetServer::Create: Network library cannot support more than %d clients.\n", 
			NET_MAXCLIENTS);
		return false;
	}
	m_clChan = new CNetClChan[m_pSvState->maxClients];

	//Initialize Network sockets
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
		Shutdown();
		return false;
	}

	//Pick one of the IP address we found
	SOCKADDR_IN* pAddrInet=0;
	ulong	addrFlags=0;
	char  * pAddrString=0;
	bool	bFoundAddr = false;
	char	szBestAddr[NET_IPADDRLEN];
	
	memset(szBestAddr,0,NET_IPADDRLEN);
	
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
			if(m_pSvState->localAddr[0] &&  (strcmp(m_pSvState->localAddr,pAddrString) ==0))
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


	
	char ipAddr[NET_IPADDRLEN];
	sprintf(ipAddr,"%s:%d",szBestAddr,m_pSvState->port);

	//Validate address
	CNetAddr netaddr(ipAddr);
	if(!netaddr.IsValid())
	{
		ComPrintf("CNetServer::Init: Unable to resolve ip address: %s\n", ipAddr);
		Shutdown();
		return false;
	}

	//Bind Socket
	if(!m_pSock->Bind(netaddr))
	{
		ComPrintf("CNetServer::Init:Unable to bind socket to  %s\n", ipAddr );
		Shutdown();
		return false;
	}

	//Save IP Addr
	strcpy(m_szIPAddr,szBestAddr);

	//Save Local Address
	CNetAddr::SetLocalServerAddr(ipAddr);
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
	if(m_clChan)
	{
		delete [] m_clChan;
		m_clChan = 0;
	}

	memset(m_szIPAddr,0,sizeof(m_szIPAddr));
	CNetAddr::SetLocalServerAddr(m_szIPAddr);

	m_pMultiCast = 0;
	m_pSock->Close();
}

/*
======================================
Server will be restarting. Ask all
clients to reconnect
======================================
*/
void CNetServer::SendReconnectToAll()
{
	for(int i=0;i<m_pSvState->maxClients;i++)
	{
		if(m_clChan[i].m_state > CL_INUSE)
			SendReconnect(i);
	}
}


bool CNetServer::IsActive() const
{	return m_pSock->ValidSocket();
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
	m_sendBuf.WriteInt(-1);
	m_sendBuf.WriteString(S2C_REJECT);
	m_sendBuf.WriteString(reason);
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
	m_sendBuf.WriteInt(-1);

	if(!full)
		m_sendBuf.WriteString(S2C_STATUS);
	else
		m_sendBuf.WriteString(S2C_FULLSTATUS);

	//Status info
	m_sendBuf.WriteInt(VOID_PROTOCOL_VERSION);		//Protocol
	m_sendBuf.WriteString(m_pSvState->gameName);	//Game
	m_sendBuf.WriteString(m_pSvState->hostName);	//Hostname
	m_sendBuf.WriteString(m_pSvState->worldname);	//Map name
	m_sendBuf.WriteInt(m_pSvState->maxClients);		//max clients
	m_sendBuf.WriteInt(m_pSvState->numClients);		//cur clients
	
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
	//Retire inactive clients here ?

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
	}
	m_challenges[i].time = System::GetCurTime();

ComPrintf("SVNet: Sent challenge %d to %s\n", m_challenges[i].challenge,
		  m_challenges[i].addr.ToString());

	//Send response packet
	m_sendBuf.Reset();
	m_sendBuf.WriteInt(-1);
	m_sendBuf.WriteString(S2C_CHALLENGE);
	m_sendBuf.WriteInt(m_challenges[i].challenge);
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
		sprintf(msg,"Bad Protocol version. Need %d, not %d", VOID_PROTOCOL_VERSION, protocolVersion);
		SendRejectMsg(msg);
		return;
	}

	//Validate Challenge
	int challenge = m_recvBuf.ReadInt();
	for(int i=0; i<MAX_CHALLENGES; i++)
	{
		if(m_challenges[i].addr == m_pSock->GetSource()) 
		{
			if(m_challenges[i].challenge == challenge)
				break;
			else
			{
ComPrintf("SVNET: Challenge for %s should be %d. is %d\n", m_challenges[i].addr.ToString(), 
					m_challenges[i].challenge,  challenge);
			}
		}
	}

	if(i == MAX_CHALLENGES)
	{
ComPrintf("SVNET: Unable to find a challenge number for %s\n", m_challenges[i].addr.ToString());
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
				ComPrintf("SVNET: Ignoring DupConnect from %s\n", m_pSock->GetSource().ToString());
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
	m_sendBuf.WriteInt(-1);
	m_sendBuf.WriteString(S2C_ACCEPT);
	m_sendBuf.WriteInt(m_pSvState->levelId);
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
//======================================================================================
/*
======================================
Send requested spawn parms
======================================
*/
void CNetServer::SendSpawnParms(int chanId)
{
	int  lastInSeq = 0;
	int  reqNum  =  m_clChan[chanId].m_spawnReqId;

	//We got the last message
	if((m_clChan[chanId].m_spawnLevel == SVC_BEGIN) && 
	   (reqNum == 0))
	{
		lastInSeq = 1;
		m_clChan[chanId].m_netChan.m_buffer.Reset();
		m_clChan[chanId].m_netChan.m_buffer.WriteByte(SV_CONFIGSTRING);
		m_clChan[chanId].m_netChan.m_buffer.WriteByte(SVC_BEGIN);
		m_clChan[chanId].m_netChan.m_buffer.WriteInt((reqNum | (lastInSeq << 31)));
		m_clChan[chanId].m_netChan.m_buffer.WriteByte(chanId);

		//Begin client
		m_clChan[chanId].m_spawnLevel = 0;
		m_clChan[chanId].m_state = CL_INGAME;
		m_pServer->OnClientBegin(chanId);
		return;
	}

	bool error = false;
	int  numBufs =  m_pServer->NumConfigStringBufs(m_clChan[chanId].m_spawnLevel);

	//Was an invalid request, game server has no config strings for that id
	if(!numBufs)
		error = true;
	else
	{
		m_clChan[chanId].m_netChan.m_buffer.Reset();
		m_clChan[chanId].m_netChan.m_buffer.WriteByte(SV_CONFIGSTRING);
		m_clChan[chanId].m_netChan.m_buffer.WriteByte(m_clChan[chanId].m_spawnLevel);

		if(reqNum + 1 >= numBufs )
			lastInSeq = 1;

		m_clChan[chanId].m_netChan.m_buffer.WriteInt((reqNum | (lastInSeq << 31)));
		if(!m_pServer->WriteConfigString(chanId,
										 m_clChan[chanId].m_netChan.m_buffer, 
										 m_clChan[chanId].m_spawnLevel,
										 reqNum))
			error = true;
	}

	if(error)
	{
		ComPrintf("SV:Client(%d) Requested bad spawnlevel %d(%d)\n", 
			chanId, m_clChan[chanId].m_spawnLevel, reqNum);
		SendDisconnect(chanId,DR_CLBADMSG);
		return;
	}
//ComPrintf("SV:Client(%d) Sending spawn level %d\n", chanId, m_clChan[chanId].m_spawnLevel);
}

/*
======================================
Received a message from a spawning client
who wants to request the next round of spawn info
======================================
*/
void CNetServer::ParseSpawnMessage(int chanId)
{
	//Check if client is trying to spawn into current map
	int levelid = m_recvBuf.ReadInt();
	if(m_recvBuf.BadRead())
	{	
		m_recvBuf.Reset();
		SendDisconnect(chanId,DR_CLBADMSG);
		return;
	}

	if( levelid != m_pSvState->levelId)
	{
		ComPrintf("SV:Client(%d) needs to reconnect, bad levelid %d != %d\n", 
					chanId, levelid ,m_pSvState->levelId);
		SendReconnect(chanId);
		return;
	}

	//Find out what spawn message the client is asking for
	byte spawnparm = m_recvBuf.ReadByte();
	if(m_recvBuf.BadRead())
	{
		m_recvBuf.Reset();
		SendDisconnect(chanId,DR_CLBADMSG);
		return;
	}

	int  reqNum    = m_recvBuf.ReadInt();
	if(m_recvBuf.BadRead())
	{
		m_recvBuf.Reset();
		SendDisconnect(chanId,DR_CLBADMSG);
		return;
	}

//ComPrintf("SV:Client(%d) Requesting Spawn, Level:%d  Num:%d\n", chanId, spawnparm, reqNum);
	//Client aborted connection
	if(spawnparm == CL_DISCONNECT)
	{
		m_pServer->OnClientDrop(chanId, DR_CLQUIT);
		m_clChan[chanId].Reset();
		return;	
	}
	m_clChan[chanId].m_spawnReqId = reqNum;
	m_clChan[chanId].m_spawnLevel = spawnparm;
}

//======================================================================================
//======================================================================================
/*
======================================
Print a server message to a given client
======================================
*/
void CNetServer::ClientPrintf(int chanId, const char * message)
{
	if(m_clChan[chanId].m_state == CL_INGAME)
	{
		ChanBeginWrite(chanId,SV_PRINT,strlen(message));
		ChanWriteString(message);
		ChanFinishWrite();
	}
}

/*
======================================
Broadcast message to all the clients
======================================
*/
void CNetServer::BroadcastPrintf(const char* message)
{
	for(int i=0;i<m_pSvState->maxClients;i++)
	{
		if(m_clChan[i].m_state == CL_INGAME)
		{
			ChanBeginWrite(i,SV_PRINT,strlen(message));
			ChanWriteString(message);
			ChanFinishWrite();
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
	m_clChan[chanId].m_netChan.m_buffer.WriteByte(SV_RECONNECT);
	m_clChan[chanId].m_netChan.PrepareTransmit();

	m_pSock->SendTo(&m_clChan[chanId].m_netChan);

	m_clChan[chanId].m_spawnLevel = 0;
	m_clChan[chanId].m_spawnReqId = 0;
	m_clChan[chanId].m_numBuf = 0;
	m_clChan[chanId].m_bBackbuf = false;
	m_clChan[chanId].m_bDropClient = false;
	m_clChan[chanId].m_bSend = false;
	m_clChan[chanId].m_state = CL_INUSE;

	m_pServer->OnLevelChange(chanId);
}

/*
======================================
Tell the client to disconnect
======================================
*/
void CNetServer::SendDisconnect(int chanId, const DisconnectReason &reason) 
{
	if(m_clChan[chanId].m_state == CL_FREE)
		return;

	//Let the client know of the reason if the server is disconnecting it
	//Dont bother if it was on its own request
	if(reason.disconnectMsg)
	{
		m_clChan[chanId].m_netChan.m_buffer.Reset();
		m_clChan[chanId].m_netChan.m_buffer.WriteByte(SV_DISCONNECT);
		m_clChan[chanId].m_netChan.m_buffer.WriteString(reason.disconnectMsg);
		m_clChan[chanId].m_netChan.PrepareTransmit();
		m_pSock->SendTo(&m_clChan[chanId].m_netChan);
	}

	//Let the main server know that this client is disconnecting
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
//ComPrintf("Msg from :
			//match client
			if(m_clChan[i].m_netChan.MatchAddr(m_pSock->GetSource()))
			{
				m_recvBuf.BeginRead();
				m_clChan[i].m_netChan.BeginRead();
				m_clChan[i].m_bSend = true;

				if(m_clChan[i].m_state == CL_INGAME)
					m_pServer->HandleClientMsg(i, m_recvBuf);
				else if(m_clChan[i].m_state == CL_CONNECTED)
					ParseSpawnMessage(i);
				break;
			}
		}
//ComPrintf("SV: Unknown packet from %s\n", m_pSock->GetSource().ToString());
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

		//Check timeout
		if(m_clChan[i].m_netChan.m_lastReceived + NET_TIMEOUT_INTERVAL < System::GetCurTime())
		{
			SendDisconnect(i,DR_CLTIMEOUT);
			continue;
		}

		//Check overflow
		if(m_clChan[i].m_bDropClient)
		{
			SendDisconnect(i,DR_CLOVERFLOW);
			continue;
		}

		//Will fail if we didnt receive a packet from 
		//this client this frame, or if channels chokes
		if(!m_clChan[i].ReadyToSend())
			continue;

		//In game clients
		if(m_clChan[i].m_state == CL_INGAME)
		{
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
