#include "Net_hdr.h"
#include "Net_chan.h"
#include "Net_sock.h"
#include "Net_protocol.h"
#include "Net_client.h"

using namespace VoidNet;

/*
======================================
Constructor
======================================
*/
CNetClient::CNetClient(I_NetClientHandler * client): 
						m_pClient(client),
						m_buffer(NET_MAXBUFFERSIZE),
						m_reliableBuf(NET_MAXBUFFERSIZE)
{
	m_pSock = new CNetSocket(&m_buffer);
	
	m_pNetChan = new CNetChan;
	m_pNetChan->Reset();

	memset(m_szServerAddr,0,NET_IPADDRLEN);

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
				HandleSpawnStrings();
		}
		return;
	}
	
	//Look for OOB unconnected messages
	while(m_pSock->RecvFrom())
	{
		if((m_netState == CL_INUSE) &&
		   (m_buffer.ReadInt() == -1))
		{
			m_pNetChan->m_lastReceived = System::GetCurTime();
			HandleOOBMessage();
			continue;
		}
m_pClient->Print("CL: Unknown packet from %s\n", m_pSock->GetSource().ToString());
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
	if((m_pNetChan->m_lastReceived + NET_TIMEOUT_INTERVAL) < System::GetCurTime())
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
				m_pNetChan->m_buffer.WriteBuffer(m_reliableBuf);
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
//m_pClient->Print("CL: Req spawn level %d. Packet %d\n", m_spawnLevel, m_spawnNextPacket);
				m_pNetChan->m_buffer.Reset();
				m_pNetChan->m_buffer.WriteInt(m_levelId);
				m_pNetChan->m_buffer.WriteByte(m_spawnLevel);
				m_pNetChan->m_buffer.WriteInt(m_spawnNextPacket);
				m_pNetChan->PrepareTransmit();
				m_pSock->SendTo(m_pNetChan);
			}
		}
		//Resent OOB queries
		else if((m_netState == CL_INUSE) && 
				(m_szLastOOBMsg != 0)  &&
				(m_fNextSendTime < System::GetCurTime()))
		{

			if(m_szLastOOBMsg == C2S_GETCHALLENGE)
				SendChallengeReq();
			else if(m_szLastOOBMsg == C2S_CONNECT)
				SendConnectReq();
		}
	}
}



/*
============================================================================
Handle reading of all the Config Strings
the server sends to the client before it first enters the game
map name, server parms
modellist
soundlist
imagelist
entity baselines
============================================================================
*/
void CNetClient::HandleSpawnStrings()
{
	byte msgId  = m_buffer.ReadByte();
	switch(msgId)
	{
	case SV_DISCONNECT:
		{
			//Server told us to get lost
			m_pClient->Print("%s\n", m_buffer.ReadString());
			Disconnect(true);
			return;
		}
	case SV_RECONNECT:
		{
			//Reconnect, the server probably changed maps 
			//while we were getting spawn info. start again
			m_pClient->Print("CL: Server asked to reconnect\n");
			Reconnect(true);
			return;
		}
	case SV_CONFIGSTRING:
		{
			m_spawnLevel = m_buffer.ReadByte();
			uint packNum = m_buffer.ReadInt();
			uint lastinSeq = (packNum >> 31);		//Get last packet flag
			
			//get rid of high bits
			packNum &= ~(1<<31);
			
			//Err, Packet is not what we requested
			//ignore this and rerequest
			if(packNum != m_spawnNextPacket)
			{
				m_pClient->Print("Error getting spawn parms from server\n");
				Disconnect(false);
				return;
			}

			//we just got a begin message from the server, which means we
			//have recevied all the spawn messages we needed to
			if(m_spawnLevel == SVC_BEGIN)
			{
//				void BeginGame(int clNum, CBuffer &buffer)=0;
				m_pClient->BeginGame(m_buffer.ReadByte(), m_buffer);
				m_spawnLevel = 0;
				m_spawnNextPacket = 0;
				m_netState = CL_INGAME;
				return;
			}

//m_pClient->Print("CL: Got spawn level %d. Packet %d\n", m_spawnLevel, packNum);
			m_pClient->HandleSpawnMsg(m_spawnLevel,m_buffer);

			if(!lastinSeq)
				m_spawnNextPacket++;
			else
			{
				//increment to request next id
				m_spawnLevel ++;
				m_spawnNextPacket = 0;
			}
			break;
		}
	default:
		{	
			//Should never happen
			m_pClient->Print("Disconnected: Got bad spawnStrings\n");
			Disconnect(false);
			return;
		}
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
	m_buffer.WriteInt(-1);

	//Write Connection Parms
	m_buffer.WriteString(C2S_CONNECT);			//Header
	m_buffer.WriteInt(VOID_PROTOCOL_VERSION);	//Protocol Version
	m_buffer.WriteInt(m_challenge);				//Challenge Req
	
	//User Info
	m_pClient->WriteUserInfo(m_buffer);

	m_pSock->Send(m_buffer);

	m_szLastOOBMsg = C2S_CONNECT;
	m_fNextSendTime = System::GetCurTime() + 2.0f;

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
	m_buffer.WriteInt(-1);
	m_buffer.WriteString(C2S_GETCHALLENGE);

	m_pSock->SendTo(m_buffer,netAddr);

	m_szLastOOBMsg = C2S_GETCHALLENGE;
	m_fNextSendTime = System::GetCurTime() + 2.0f;

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

	m_pNetChan->m_lastReceived = System::GetCurTime();

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
m_pClient->Print("CL: Telling server we disconnect %s\n", m_pNetChan->GetAddrString());
			//send disconnect message if remote
			m_pNetChan->m_buffer.Reset();
			m_pNetChan->m_buffer.WriteByte(CL_DISCONNECT);
			m_pNetChan->PrepareTransmit();
			m_pSock->SendTo(m_pNetChan);
		}
//		m_pClient->HandleDisconnect((m_bLocalServer && !serverPrompted));
		m_pClient->HandleDisconnect();
	}

	m_pNetChan->Reset();
	
	memset(m_szServerAddr,0, NET_IPADDRLEN);
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
void CNetClient::Reconnect(bool serverPrompted)
{
	char svaddr[NET_IPADDRLEN];
	strcpy(svaddr,m_szServerAddr);
	Disconnect(serverPrompted);

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
