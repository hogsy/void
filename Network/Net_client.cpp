#include "Net_sock.h"
#include "Net_client.h"
#include "Net_protocol.h"

using namespace VoidNet;

/*
======================================
Constructor
======================================
*/
CNetClient::CNetClient(I_NetClientHandler * client): 
						m_pClient(client),
						m_buffer(MAX_BUFFER_SIZE),
						m_reliableBuf(MAX_BUFFER_SIZE)
{
	m_pSock = new CNetSocket(&m_buffer);
	
	m_pNetChan = new CNetChan();
	m_pNetChan->Reset();

	memset(m_szServerAddr,0,MAX_IPADDR_LEN);

	m_bLocalServer = false;
	
	m_challenge = 0;
	m_levelId = 0;

	//Flow Control
	m_fNextSendTime= 0.0f;	
	m_szLastOOBMsg= 0;

	m_spawnLevel=0;
	m_spawnNextPacket=0;

	m_netState= CL_FREE;
}

/*
======================================
Destructor
======================================
*/

CNetClient::~CNetClient()
{
	delete m_pSock;
	delete m_pNetChan;
	
	m_pClient = 0;
	m_szLastOOBMsg = 0;
}

//======================================================================================

/*
======================================
Process any waiting packets
======================================
*/
void CNetClient::ReadPackets()
{
	if(m_netState == CL_FREE)
		return;

	if(m_netState != CL_INUSE) //INGAME)
	{
		//Look for messages only from the server
		while(m_pSock->Recv())
		{
			//m_pClient->Print("CL: Reading update\n");
			m_pNetChan->BeginRead();

			if(m_netState == CL_INGAME)
				m_pClient->HandleGameMsg(m_buffer);
			else if(m_netState == CL_CONNECTED)
				HandleSpawnParms();
		}
		return;
	}
	
	//Look for OOB unconnected messages
	while(m_pSock->RecvFrom())
	{
		if((m_netState == CL_INUSE) &&
		   (m_buffer.ReadInt() == -1))
		{
			m_pNetChan->m_lastReceived = System::g_fcurTime;
			HandleOOBMessage();
			continue;
		}
		//m_pClient->Print("CL: Unknown packet from %s\n", m_pSock->GetSource().ToString());
	}	
}

/*
======================================
Send off any messages we need to
======================================
*/
void CNetClient::SendUpdate()
{
	if(m_netState == CL_FREE)
		return;

	//Check for timeouts
	if(m_pNetChan->m_lastReceived + NET_TIMEOUT_INTERVAL < System::g_fcurTime)
	{
		m_pClient->Print("CL: Connection timed out\n");
		Disconnect(false);
		return;
	}

	//Checks local rate
	if(m_pNetChan->CanSend())
	{
		if(m_netState == CL_INGAME)
		{
			//Check for overflows in buffers
			if(m_pNetChan->m_bFatalError || m_reliableBuf.OverFlowed())
			{
				m_pClient->Print("CL: Connection overflowed\n");
				Disconnect(false);
				return;
			}

			//Try to write reliable data if needed to
			if(m_reliableBuf.GetSize() && m_pNetChan->CanSendReliable())
			{
				m_pNetChan->m_buffer.Write(m_reliableBuf);
				m_reliableBuf.Reset();
			}
			
			m_pNetChan->PrepareTransmit();
			m_pSock->SendTo(m_pNetChan);
			//m_pClient->Print("CL:  sending update\n");
		}
		else if(m_netState == CL_CONNECTED)
		{
			//We have connected. Need to ask server for baselines
			if(m_pNetChan->CanSendReliable())
			{
				m_pNetChan->m_buffer.Reset();
				m_pNetChan->m_buffer.Write(m_levelId);
				m_pNetChan->m_buffer.Write(m_spawnLevel);
				m_pNetChan->m_buffer.Write(m_spawnNextPacket);

//m_pClient->Print("CL: Req spawn level %d. Packet %d\n", m_spawnLevel, m_spawnNextPacket);

				m_pNetChan->PrepareTransmit();
				m_pSock->SendTo(m_pNetChan);

				//We just sent an Ack to the last spawn message of a sequence
				//Switch to next sequence now, or in game mode, if this was
				//the last sequence
				//we got everything we needed
/*
				if(m_spawnLevel == SVC_BEGIN)
				{
					m_spawnLevel = 0;
					m_spawnNextPacket = 0;
					m_netState = CL_INGAME;
				}
*/
			}
		}
		//Resent OOB queries
		else if((m_netState == CL_INUSE) && 
				(m_szLastOOBMsg != 0)  &&
				(m_fNextSendTime < System::g_fcurTime))
		{

			if(m_szLastOOBMsg == C2S_GETCHALLENGE)
				SendChallengeReq();
			else if(m_szLastOOBMsg == C2S_CONNECT)
				SendConnectReq();
		}
	}
}


/*
======================================
Read spawn info
map name, server parms
modellist
soundlist
imagelist
entity baselines
======================================
*/
void CNetClient::HandleSpawnParms()
{
	byte id      = m_buffer.ReadByte();

	if(id == SV_DISCONNECT)
	{
		m_pClient->Print("Disconnected: %s\n", m_buffer.ReadString());
		Disconnect(true);
		return;
	}

	//Reconnect, the server probably changed maps 
	//while we were getting spawn info. start again
	if(id == SV_RECONNECT)
	{
		m_pClient->Print("CL: Server asked to reconnect\n");
		Reconnect();
		return;
	}
	
	uint packNum = m_buffer.ReadInt();
	uint lastinSeq = (packNum >> 31);		//Get last packet flag
	
	//get rid of high bits
	packNum &= ~(1<<31);
	
	//Err, Packet is not what we requested
	//ignore this and rerequest
	if(packNum != m_spawnNextPacket)
	{
		m_pClient->Print("Error in getting spawn parms from server\n");
		Disconnect(false);
		return;
	}

//m_pClient->Print("CL: Got spawn level %d. Packet %d\n", id, packNum);

/*
	m_fNextSendTime = 0.0f;
	//Update spawnstate
	m_spawnLevel = id;
	m_spawnNextPacket = packNum;
*/

	m_pClient->HandleSpawnMsg(m_spawnLevel,m_buffer);

	if(!lastinSeq)
		m_spawnNextPacket++;
	else
	{
		//we just got a begin message from the server
		if(m_spawnLevel == SVC_BEGIN)
		{
			m_spawnLevel = 0;
			m_spawnNextPacket = 0;
			m_netState = CL_INGAME;
			return;
		}
		m_spawnLevel ++;
		m_spawnNextPacket = 0;
	}
}

/*
======================================
Handle response to any OOB messages
the client might have sent
======================================
*/
void CNetClient::HandleOOBMessage()
{
	const char * msg = m_buffer.ReadString();
	
	if(!strcmp(msg,S2C_CHALLENGE))
	{
		m_challenge = m_buffer.ReadInt();
		
		m_szLastOOBMsg = 0;
		m_fNextSendTime = 0.0f;

		//Got challenge, now send connection request
		SendConnectReq();
		m_pClient->Print("CL: Got challenge %d\n", m_challenge);
		return;
	}

	if(!strcmp(msg, S2C_REJECT))
	{
		Disconnect(true);
		m_pClient->Print("CL: Server rejected connection: %s\n", m_buffer.ReadString());
		return;
	}
	
	//We have been accepted. Now get ready to receive spawn parms
	if(!strcmp(msg,S2C_ACCEPT))
	{
		m_levelId = m_buffer.ReadInt();
		m_netState = CL_CONNECTED;

		m_szLastOOBMsg = 0;
		m_fNextSendTime = 0.0f;

		m_spawnLevel = SVC_GAMEINFO;
		m_spawnNextPacket = 0;
		
		//Setup the network channel. 
		//Only reliable messages are sent until spawned
		m_pNetChan->Setup(m_pSock->GetSource(),&m_buffer);

		m_pClient->Print("CL: Connected\n");
		return;
	}
}

/*
======================================
Send connection parms
======================================
*/
void CNetClient::SendConnectReq()
{
	//Create Connection less packet
	m_buffer.Reset();
	m_buffer.Write(-1);

	//Write Connection Parms
	m_buffer.Write(C2S_CONNECT);			//Header
	m_buffer.Write(VOID_PROTOCOL_VERSION);	//Protocol Version
	m_buffer.Write(m_challenge);			//Challenge Req
	
	//User Info
	m_pClient->WriteUserInfo(m_buffer);

	m_pSock->Send(m_buffer);

	m_szLastOOBMsg = C2S_CONNECT;
	m_fNextSendTime = System::g_fcurTime + 2.0f;

	m_pClient->Print("CL: Attempting to connect to %s\n", m_szServerAddr);
}

/*
=======================================
Disconnect any current connection and 
initiates a new connection request
to the specified address
=======================================
*/
void CNetClient::SendChallengeReq()
{
	CNetAddr netAddr(m_szServerAddr);
	if(!netAddr.IsValid())
	{
		m_pClient->Print("CL: ConnectTo: Invalid address\n");
		Disconnect(false);
		return;
	}

	//Now initiate a connection request
	//Create Connection less packet
	m_buffer.Reset();
	m_buffer.Write(-1);
	m_buffer.Write(C2S_GETCHALLENGE);

	m_pSock->SendTo(m_buffer,netAddr);

	m_szLastOOBMsg = C2S_GETCHALLENGE;
	m_fNextSendTime = System::g_fcurTime + 2.0f;

	m_pClient->Print("CL: Requesting Challenge from %s\n", m_szServerAddr);
}

//======================================================================================
//======================================================================================

/*
======================================
Start Connecting to the given addr
======================================
*/
void CNetClient::ConnectTo(const char * ipaddr)
{
	if(!ipaddr)
	{
		m_pClient->Print("Usage - connect address(:port)\n");
		return;
	}

	if(m_netState != CL_FREE)
		Disconnect(false);

	//Create Socket
	if(!m_pSock->ValidSocket())
	{
		if(!m_pSock->Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
		{
			PrintSockError(WSAGetLastError(),"CLNet::Init: Couldnt create socket");
			return;
		}
		//Bind to local addr ?
	}

	//Send a connection request
	CNetAddr netAddr(ipaddr);
	if(!netAddr.IsValid())
	{
		m_pClient->Print("CL: Aborting connection. Invalid address\n");
		return;
	}
	strcpy(m_szServerAddr, netAddr.ToString());

	if(strcmp(m_szServerAddr, CNetAddr::GetLocalServerAddr()) == 0)
	{
		m_pClient->Print("CL: Connecting to local Server\n");
		m_bLocalServer = true;
	}

	m_pNetChan->m_lastReceived = System::g_fcurTime;

	//Now initiate a connection request
	m_netState = CL_INUSE;
	SendChallengeReq();
}

/*
=====================================
Disconnect if connected to a server
=====================================
*/
void CNetClient::Disconnect(bool serverPrompted)
{
	if(m_netState >= CL_CONNECTED)
	{
		//Did the server prompt us to disconnect ?
		if(!serverPrompted)
		{
			//send disconnect message if remote
			m_pNetChan->m_buffer.Reset();
			m_pNetChan->m_buffer.Write(CL_DISCONNECT);
			m_pNetChan->PrepareTransmit();
			m_pSock->SendTo(m_pNetChan);
		}
		m_pClient->HandleDisconnect((m_bLocalServer && !serverPrompted));
	}

	m_pNetChan->Reset();
	
	memset(m_szServerAddr,0, MAX_IPADDR_LEN);
	m_bLocalServer = false;
	
	m_levelId = 0;
	m_netState = CL_FREE;
	
	m_spawnLevel = 0;
	m_spawnNextPacket=0;

	//Flow Control
	m_fNextSendTime = 0.0f;
	m_szLastOOBMsg = 0;

	m_pClient->Print("CL: Disconnected\n");
}

/*
======================================
Disconnect and reconnect
======================================
*/
void CNetClient::Reconnect()
{
	char svaddr[MAX_IPADDR_LEN];
	strcpy(svaddr,m_szServerAddr);
	Disconnect(true);
	ConnectTo(svaddr);
	m_pClient->Print("CL: Reconnecting ...\n");
}


//======================================================================================

/*
======================================
Update local net vars
======================================
*/
void CNetClient::SetRate(int rate)
{	m_pNetChan->SetRate(rate);
}
void CNetClient::SetPort(short port)
{	
}

/*
======================================
Access functions for the game Client
======================================
*/
CBuffer & CNetClient::GetSendBuffer()
{	return m_pNetChan->m_buffer;
}
CBuffer & CNetClient::GetReliableBuffer()
{	return m_reliableBuf;
}
const NetChanState & CNetClient::GetChanState() const
{	return m_pNetChan->m_state;
}


bool CNetClient::CanSend()
{	return m_pNetChan->CanSend();
}

bool CNetClient::CanSendReliable()
{	return m_reliableBuf.HasSpace(1);
}
