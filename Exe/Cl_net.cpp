#include "Cl_net.h"

using namespace VoidNet;

/*
======================================
Constructor/Destructor
======================================
*/
CClientNetHandler::CClientNetHandler(CClient &owner): 
								m_refClient(owner), 
								m_buffer(VoidNet::MAX_BUFFER_SIZE),
								m_sock(&m_buffer)
{
	m_netChan.Reset();
	memset(m_szServerAddr,0,24);

	m_bLocalServer = false;
	m_bCanSend = false;
	m_bInitialized = false;
	
	m_challenge = 0;
	m_levelId = 0;

	//Flow Control
	m_fNextSendTime= 0.0f;	
	m_numResends= 0;		
	m_szLastOOBMsg= 0;
	
	m_spawnState=0;
	m_netState= CL_FREE;
}

CClientNetHandler::~CClientNetHandler()
{	m_szLastOOBMsg = 0;
}

/*
======================================
Process any waiting packets
fix me, change his to look for multiple messages
		should only look from messages from the server once connected
======================================
*/
void CClientNetHandler::ReadPackets()
{
	if(m_netState == CL_FREE)
		return;

	if(m_netState == CL_SPAWNED)
	{
		while(m_sock.RecvFromServer())
		{
//m_refClient.Print(CClient::DEFAULT,"CL: Reading update\n");
			m_netChan.BeginRead();
			
			byte msgId = 0;
			while(msgId != 255)
			{
			msgId= (int)m_buffer.ReadByte();

			//bad message
			if(msgId == -1)
				break;
				//continue;

			switch(msgId)
			{
			case SV_TALK:
				{
					char name[32];
					strcpy(name,m_buffer.ReadString());
					m_refClient.Print(CClient::TALK_MESSAGE,"%s: %s\n",name,m_buffer.ReadString());
					m_bCanSend = true;
					break;
				}
			case SV_DISCONNECT:
				{
					m_refClient.Print(CClient::SERVER_MESSAGE,"Server quit\n");
					Disconnect(true);
					break;
				}
			case SV_PRINT:	//just a print message
				{
					m_refClient.Print(CClient::SERVER_MESSAGE,"%s\n",m_buffer.ReadString());
					break;
				}
			case SV_RECONNECT:
				{
					Reconnect();
					break;
				}
			}
			}
		}
	}
	else 
	{
		while(m_sock.Recv())
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
	//m_refClient.Print(CClient::DEFAULT"CClient::ReadPacket::Unknown packet from %s\n", m_pSock->GetSource().ToString());
			}
		}	
	}



/*
	if(m_netState == CL_FREE)
		return;
	while(m_sock.Recv())
	{
		//in game.
		if(m_netState == CL_SPAWNED)
		{
//m_refClient.Print(CClient::DEFAULT,"CL: Reading update\n");
			m_netChan.BeginRead();
			byte msgId = m_buffer.ReadByte();

			//bad message
			if(msgId == -1)
				continue;

			switch(msgId)
			{
			case SV_TALK:
				{
					char name[32];
					strcpy(name,m_buffer.ReadString());
					m_refClient.Print(CClient::TALK_MESSAGE,"%s: %s\n",name,m_buffer.ReadString());
					m_bCanSend = true;
					break;
				}
			case SV_DISCONNECT:
				{
					m_refClient.Print(CClient::SERVER_MESSAGE,"Server quit\n");
					Disconnect(true);
					break;
				}
			case SV_PRINT:	//just a print message
				{
					m_refClient.Print(CClient::SERVER_MESSAGE,"%s\n",m_buffer.ReadString());
					break;
				}
			case SV_RECONNECT:
				{
					Reconnect();
					break;
				}
			}
			continue;
		}
		//in the connection phase, expecting spawn parms
		else if(m_netState == CL_CONNECTED)
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
//m_refClient.Print(CClient::DEFAULT"CClient::ReadPacket::Unknown packet from %s\n", m_pSock->GetSource().ToString());
		}
	}	
*/
}

/*
======================================
Send off any messages we need to
======================================
*/
void CClientNetHandler::SendUpdates()
{
	if(!m_sock.ValidSocket())
		return;

	//we have spawned. send update packet
	if(m_netState == CL_SPAWNED)
	{
		//Checks local rate
		if(m_netChan.CanSend())
		{
			m_netChan.PrepareTransmit();
			m_sock.Send(m_netChan.m_sendBuffer);
			m_netChan.m_buffer.Reset();
//m_refClient.Print(CClient::DEFAULT"CL: Client sending update\n");
		}
		return;
	}

	//If the client has NOT spawned, then limit resends before we decide to fail
	if(m_numResends >= 4)
	{
m_refClient.Print(CClient::DEFAULT,"CL: Timed out\n");
		Disconnect();
		return;
	}

	//We have connected. Need to ask server for baselines
	if(m_netState == CL_CONNECTED)
	{
		//Its been a while and our reliable packet hasn't been answered, try again
		if(m_fNextSendTime < System::g_fcurTime)
			m_netChan.m_reliableBuffer.Reset();
		
		//Ask for next spawn parm if we have received a reply to the last
		//otherwise, keep asking for the last one
		if(m_netChan.CanSendReliable())
		{
			m_netChan.m_buffer.Reset();
			m_netChan.m_buffer.Write(m_levelId);
			m_netChan.m_buffer.Write((m_spawnState + 1));
			m_netChan.PrepareTransmit();

			m_sock.Send(m_netChan.m_sendBuffer);

			//We got all the necessary info. just ack and switch to spawn mode
			if(m_spawnState == SVC_BEGIN)
			{
m_refClient.Print(CClient::DEFAULT,"CL: Client is ready to SPAWN\n");
				m_netState = CL_SPAWNED;

			}
			else
			{
				m_fNextSendTime = System::g_fcurTime + 1.0f;
				m_numResends ++;
//m_refClient.Print(CClient::DEFAULT"CL: Asking for spawnstate %d\n", m_spawnState+1);
			}
		}
	}
	//Unconnected socket. sends OOB queries
	else if((m_netState == CL_INUSE) && 
			(m_fNextSendTime < System::g_fcurTime))
	{
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
void CClientNetHandler::HandleSpawnParms()
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

		SendChallengeReq();
		return;
	}

	//Got spawn data
	if(id == m_spawnState + 1)
	{
		//Update spawnstate
		m_spawnState = id;
		m_fNextSendTime = 0.0f;

		switch(m_spawnState)
		{
		case SVC_INITCONNECTION:
			{
				char * game = m_buffer.ReadString();
m_refClient.Print(CClient::DEFAULT,"Game : %s\n", game);
				char * map = m_buffer.ReadString();
m_refClient.Print(CClient::DEFAULT,"Map : %s\n", map);
				m_refClient.LoadWorld(map);
				break;
			}
		case SVC_MODELLIST:
			break;
		case SVC_SOUNDLIST:
			break;
		case SVC_IMAGELIST:
			break;
		case SVC_BASELINES:
			break;
		case SVC_BEGIN:
			{
				break;
			}
		}
	}
}

/*
======================================
Handle response to any OOB messages
the client might have sent
======================================
*/
void CClientNetHandler::HandleOOBMessage()
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
m_refClient.Print(CClient::DEFAULT, "CL: Got challenge %d\n", m_challenge);
		return;
	}

	if(!strcmp(msg, S2C_REJECT))
	{
		Disconnect();
m_refClient.Print(CClient::DEFAULT,"CL: Server rejected connection: %s\n", m_buffer.ReadString());
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
//		m_netChan.SetRate(m_clrate.ival);
		m_netChan.m_outMsgId = 1;

m_refClient.Print(CClient::DEFAULT, "CL: Connected\n");
		return;
	}
}

/*
======================================
Send connection parms
======================================
*/
void CClientNetHandler::SendConnectReq()
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
	m_buffer.Write(m_refClient.m_clname.string);
	m_buffer.Write(m_refClient.m_clrate.ival);

	m_sock.Send(m_buffer);

	m_szLastOOBMsg = C2S_CONNECT;
	m_fNextSendTime = System::g_fcurTime + 2.0f;
	m_numResends ++; 

m_refClient.Print(CClient::DEFAULT,"CL: Attempting to connect to %s\n", m_szServerAddr);
}

/*
=======================================
Disconnect any current connection and 
initiates a new connection request
to the specified address
=======================================
*/
void CClientNetHandler::SendChallengeReq()
{
	CNetAddr netAddr(m_szServerAddr);
	if(!netAddr.IsValid())
	{
		m_refClient.Print(CClient::DEFAULT,"CClient::ConnectTo: Invalid address\n");
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

m_refClient.Print(CClient::DEFAULT,"CL: Requesting Challenge from %s\n", m_szServerAddr);
}

//======================================================================================
//======================================================================================

/*
======================================
Start Connecting to the given addr
======================================
*/
void CClientNetHandler::ConnectTo(const char * ipaddr)
{
	if(!ipaddr)
	{
		m_refClient.Print(CClient::DEFAULT,"usage - connect ipaddr(:port)\n");
		return;
	}

	if(m_netState != CL_FREE)
		Disconnect();

	//Create Socket
	if(!m_sock.ValidSocket())
	{
		if(!m_sock.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
		{
			PrintSockError(WSAGetLastError(),"CClient::Init: Couldnt create socket");
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
		m_refClient.Print(CClient::DEFAULT,"CClient::ConnectTo: Invalid address\n");
		return;
	}
	strcpy(m_szServerAddr, netAddr.ToString());

	CNetAddr localAddr("localhost");
	if(localAddr == netAddr)
	{
m_refClient.Print(CClient::DEFAULT,"CL: Connecting to local Server\n");
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
void CClientNetHandler::Disconnect(bool serverPrompted)
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
			m_sock.Send(m_netChan.m_sendBuffer);
//			m_sock.Disconnect();

			//Kill server if local
			if(m_bLocalServer)
				System::GetConsole()->ExecString("killserver");
		}
		m_refClient.UnloadWorld();
	}

	m_netChan.Reset();
	
	m_szServerAddr[0] = 0;
	m_bLocalServer = false;
	
	m_levelId = 0;
	m_netState = CL_FREE;
	m_spawnState = 0;
	m_bCanSend = false;

	//Flow Control
	m_fNextSendTime = 0.0f;
	m_szLastOOBMsg = 0;
	m_numResends = 0;

m_refClient.Print(CClient::DEFAULT, "CL: Disconnected\n");
}

/*
======================================
Disconnect and reconnect
======================================
*/
void CClientNetHandler::Reconnect()
{
	char svaddr[24];
	strcpy(svaddr,m_szServerAddr);
	Disconnect(true);
	ConnectTo(svaddr);
	m_refClient.Print(CClient::DEFAULT, "Reconnecting ...\n");
}


//======================================================================================
//======================================================================================

/*
=====================================
Talk message sent to server if 
we are connected
=====================================
*/
void CClientNetHandler::SendTalkMsg(const char *string)
{
	if(m_netState != CL_SPAWNED)
		return;

	//Send this reliably ?
	m_netChan.m_buffer.Write(CL_TALK);
	m_netChan.m_buffer.Write(string);
}

/*
======================================
update name on the server
======================================
*/
void CClientNetHandler::UpdateName(const char *name)
{
	if(m_netState == CL_SPAWNED)
	{
		m_netChan.m_buffer.Write(CL_UPDATEINFO);
		m_netChan.m_buffer.Write('n');
		m_netChan.m_buffer.Write(name);
	}
}

/*
======================================
update rate on the server
======================================
*/
void CClientNetHandler::UpdateRate(int rate)
{
	m_netChan.SetRate(rate);
	if(m_netState == CL_SPAWNED)
	{
		m_netChan.m_buffer.Write(CL_UPDATEINFO);
		m_netChan.m_buffer.Write('r');
		m_netChan.m_buffer.Write(rate);
	}
}
