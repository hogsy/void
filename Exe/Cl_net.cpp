#include "Cl_net.h"
#include "Sys_hdr.h"

using namespace VoidNet;

/*
======================================
Constructor/Destructor
======================================
*/
CNetClient::CNetClient(I_ClientNetHandler * client): 
								m_pClient(client),
								m_buffer(VoidNet::MAX_BUFFER_SIZE),
								m_sock(&m_buffer)
{
	m_netChan.Reset();
	memset(m_szServerAddr,0,24);

	m_bLocalServer = false;
	
	m_challenge = 0;
	m_levelId = 0;

	//Flow Control
	m_fNextSendTime= 0.0f;	
	m_numResends= 0;		
	m_szLastOOBMsg= 0;
	
	m_spawnState=0;
	m_netState= CL_FREE;
}

CNetClient::~CNetClient()
{	
	m_pClient = 0;
	m_szLastOOBMsg = 0;
}

//======================================================================================
//======================================================================================
/*
======================================
Process any waiting packets
fix me, change his to look for multiple messages
		should only look from messages from the server once connected
======================================
*/
void CNetClient::ReadPackets()
{
	if(m_netState == CL_FREE)
		return;

	if(m_netState == CL_SPAWNED)
	{
		//Look for messages only from the server
		while(m_sock.Recv())
		{
//m_pClient->Print(CLMSG_DEFAULT,"CLNet:Reading update\n");
			m_netChan.BeginRead();
			m_pClient->HandleGameMsg(m_buffer);
		}
	}
	else 
	{
		while(m_sock.RecvFrom())
		{
			if(m_netState == CL_CONNECTED)
			{
				HandleSpawnParms();
				continue;
			}
			//socket is only active. not connected or anything
			else if(m_netState == CL_INUSE)
			{
				if(m_buffer.ReadInt() == -1)
				{
					HandleOOBMessage();
					continue;
				}
//m_pClient->Print(CLMSG_DEFAULT,"CLNet:Unknown packet from %s\n", m_pSock->GetSource().ToString());
			}
		}	
	}
}

/*
======================================
Send off any messages we need to
======================================
*/
void CNetClient::SendUpdate()
{
	if(!m_sock.ValidSocket())
		return;

	//are we spawned ?
	if(m_netState == CL_SPAWNED)
	{
		//Checks local rate
		if(m_netChan.CanSend())
		{
			m_netChan.PrepareTransmit();
			m_sock.SendTo(m_netChan);
			m_netChan.m_buffer.Reset();
//m_pClient->Print(CLMSG_DEFAULT"CL: Client sending update\n");
		}
		return;
	}

	//If the client has NOT spawned, then limit resends before we decide to fail
	if(m_numResends >= 4)
	{
		m_pClient->Print(CLMSG_DEFAULT,"Timed out\n");
		Disconnect();
		return;
	}

	//We have connected. Need to ask server for baselines
	if(m_netState == CL_CONNECTED)
	{
		//Its been a while and our reliable packet hasn't been answered, try again
//		if(m_fNextSendTime > System::g_fcurTime)
//			return;
//			m_netChan.m_reliableBuffer.Reset();
		
		//Ask for next spawn parm if we have received a reply to the last
		//otherwise, keep asking for the last one
		if(m_netChan.CanSendReliable())
		//if(m_netChan.CanSend())
		{
			m_netChan.m_buffer.Reset();
			m_netChan.m_buffer.Write(m_levelId);
			m_netChan.m_buffer.Write((m_spawnState + 1));
			m_netChan.PrepareTransmit();

//			m_sock.Send(m_netChan.m_sendBuffer);
			m_sock.SendTo(m_netChan);

			//We got all the necessary info. just ack and switch to spawn mode
			if(m_spawnState == SVC_BEGIN)
			{
m_pClient->Print(CLMSG_DEFAULT,"CL: Client is ready to SPAWN\n");
				m_netState = CL_SPAWNED;
			}

/*			else
			{
				m_fNextSendTime = System::g_fcurTime + 1.0f;
				m_numResends ++;
//m_pClient->Print(CLMSG_DEFAULT"CL: Asking for spawnstate %d\n", m_spawnState+1);
			}
*/
		}
	}
	//Unconnected socket. sends OOB queries
	else if((m_netState == CL_INUSE) && (m_fNextSendTime < System::g_fcurTime))
	{
m_pClient->Print(CLMSG_DEFAULT,"CL: Resending OOB request\n");

		if(m_szLastOOBMsg == C2S_GETCHALLENGE)
			SendChallengeReq();
		else if(m_szLastOOBMsg == C2S_CONNECT)
			SendConnectReq();
		return;
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
	//We got a response to reset Resend requests
	m_numResends = 0;

	m_netChan.BeginRead();
	byte id = m_buffer.ReadByte();
	
	//Reconnect, the server probably changed maps 
	//while we were getting spawn info. start again
	if(id == SV_RECONNECT)
	{
		m_netState = CL_INUSE;
		m_spawnState = 0;

		//Flow Control
		m_fNextSendTime = 0.0f;
		m_szLastOOBMsg = 0;
		m_numResends = 0;

		Disconnect();

ComPrintf("CL: Server asked to reconnect\n");

		SendChallengeReq();
		return;
	}

	//Got spawn data
	if(id == m_spawnState + 1)
	{
		//Update spawnstate
		m_spawnState = id;
		m_fNextSendTime = 0.0f;

		m_pClient->HandleSpawnMsg(m_spawnState,m_buffer);
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
		m_numResends =0;

		//Got challenge, now send connection request
		SendConnectReq();
		m_pClient->Print(CLMSG_DEFAULT, "Got challenge %d\n", m_challenge);
		return;
	}

	if(!strcmp(msg, S2C_REJECT))
	{
		Disconnect();
		m_pClient->Print(CLMSG_SERVER,"Server rejected connection: %s\n", m_buffer.ReadString());
		return;
	}
	
	//We have been accepted. Now get ready to receive spawn parms
	if(!strcmp(msg,S2C_ACCEPT))
	{
		m_levelId = m_buffer.ReadInt();
		m_netState = CL_CONNECTED;

		m_szLastOOBMsg = 0;
		m_fNextSendTime = 0.0f;
		m_numResends =0;
		
		//Setup the network channel. 
		//Only reliable messages are sent until spawned
		m_netChan.Setup(m_sock.GetSource(),&m_buffer);
		m_netChan.SetRate(m_pClient->GetUserInfo().rate);

m_pClient->Print(CLMSG_DEFAULT, "CLNet: Connected\n");
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
//	m_buffer.Write(m_virtualPort);			//Virtual Port
	
	//User Info
	m_buffer.Write(m_pClient->GetUserInfo().name);
	m_buffer.Write(m_pClient->GetUserInfo().rate);

	m_sock.Send(m_buffer);

	m_szLastOOBMsg = C2S_CONNECT;
	m_fNextSendTime = System::g_fcurTime + 2.0f;
	m_numResends ++; 

	m_pClient->Print(CLMSG_DEFAULT,"Attempting to connect to %s\n", m_szServerAddr);
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
		m_pClient->Print(CLMSG_DEFAULT,"CClient::ConnectTo: Invalid address\n");
		Disconnect();
		return;
	}

	//Now initiate a connection request
	//Create Connection less packet
	m_buffer.Reset();
	m_buffer.Write(-1);
	m_buffer.Write(C2S_GETCHALLENGE);

	m_sock.SendTo(m_buffer,netAddr);

	m_szLastOOBMsg = C2S_GETCHALLENGE;
	m_fNextSendTime = System::g_fcurTime + 2.0f;
	m_numResends ++;

	m_pClient->Print(CLMSG_DEFAULT,"Requesting Challenge from %s\n", m_szServerAddr);
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
		m_pClient->Print(CLMSG_DEFAULT,"Usage - connect address(:port)\n");
		return;
	}

	if(m_netState != CL_FREE)
		Disconnect();

	//Create Socket
	if(!m_sock.ValidSocket())
	{
		if(!m_sock.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
		{
			PrintSockError(WSAGetLastError(),"CLNet::Init: Couldnt create socket");
			return;
		}
		//Bind to local addr
/*		char localAddr[32];
		sprintf(localAddr,"127.0.0.1:%d",CL_DEFAULT_PORT);

		CNetAddr naddr(localAddr);
		if(!m_sock.Bind(naddr))
		{
			PrintSockError(WSAGetLastError(),"CClient::Init: Couldnt bind socked\n");
			return;
		}
*/
	}

	//Send a connection request
	CNetAddr netAddr(ipaddr);
	if(!netAddr.IsValid())
	{
		m_pClient->Print(CLMSG_DEFAULT,"Aborting connection. Invalid address\n");
		return;
	}
	strcpy(m_szServerAddr, netAddr.ToString());

	CNetAddr localAddr("localhost");
	if(localAddr == netAddr)
	{
m_pClient->Print(CLMSG_DEFAULT,"CL: Connecting to local Server\n");
		m_bLocalServer = true;
	}

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
			m_netChan.m_buffer.Reset();
			m_netChan.m_buffer.Write(CL_DISCONNECT);
			m_netChan.PrepareTransmit();
			m_sock.SendTo(m_netChan);
		}
		m_pClient->HandleDisconnect((m_bLocalServer && !serverPrompted));
	}

	m_netChan.Reset();
	
	m_szServerAddr[0] = 0;
	m_bLocalServer = false;
	
	m_levelId = 0;
	m_netState = CL_FREE;
	m_spawnState = 0;

	//Flow Control
	m_fNextSendTime = 0.0f;
	m_szLastOOBMsg = 0;
	m_numResends = 0;

	m_pClient->Print(CLMSG_DEFAULT, "Disconnected\n");
}

/*
======================================
Disconnect and reconnect
======================================
*/
void CNetClient::Reconnect()
{
	char svaddr[24];
	strcpy(svaddr,m_szServerAddr);
	Disconnect(true);
	ConnectTo(svaddr);
	m_pClient->Print(CLMSG_DEFAULT, "Reconnecting ...\n");
}


//======================================================================================

/*
======================================
update rate on the server
======================================
*/
void CNetClient::SetRate(int rate)
{	m_netChan.SetRate(rate);
}
