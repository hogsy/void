#include "Net_sock.h"
#include "Cl_main.h"

using namespace VoidNet;

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
void CClient::HandleSpawnParms()
{
	//We got a response to reset Resend requests
	m_numResends = 0;

	m_netChan.BeginRead();
	int id = m_buffer.ReadInt();
	
	//Reconnect, the server probably changed maps 
	//while we were getting spawn info. start again
	if(id == SV_RECONNECT)
	{
		m_state = CL_INUSE;
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
				ComPrintf("Game : %s\n", game);
				char * map = m_buffer.ReadString();
				ComPrintf("Map : %s\n", map);

				LoadWorld(map);
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
void CClient::HandleOOBMessage()
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
ComPrintf("CL: Got challenge %d\n", m_challenge);
		return;
	}

	if(!strcmp(msg, S2C_REJECT))
	{
		Disconnect();
ComPrintf("CL: Server rejected connection: %s\n", m_buffer.ReadString());
		return;
	}
	
	//We have been accepted. Now get ready to receive spawn parms
	if(!strcmp(msg,S2C_ACCEPT))
	{
		m_levelId = m_buffer.ReadInt();
		m_state = CL_CONNECTED;

		m_szLastOOBMsg = 0;
		m_fNextSendTime = 0.0f;
		m_numResends =0;
		
		//Setup the network channel. 
		//Only reliable messages are sent until spawned
		m_netChan.Setup(m_pSock->GetSource(),&m_buffer);
		m_netChan.SetRate(m_clrate.ival);
		m_netChan.m_outMsgId = 1;

ComPrintf("CL: Connected\n");
		return;
	}
}

/*
======================================
Process any waiting packets
======================================
*/
void CClient::ReadPackets()
{
	if(m_state == CL_FREE)
		return;

	while(m_pSock->Recv())
	{
		//in game.
		if(m_state == CL_SPAWNED)
		{
//ComPrintf("CL: Reading update\n");
			m_netChan.BeginRead();

			int msgId = m_buffer.ReadInt();
			if(msgId == -1)
				continue;

			switch(msgId)
			{
			case SV_TALK:
				char name[32];
				strcpy(name,m_buffer.ReadString());
ComPrintf("%s: %s\n", name , m_buffer.ReadString());
System::GetSoundManager()->Play(m_hsTalk);
				break;
			}
			continue;
		}
		//in the connection phase, expecting spawn parms
		else if(m_state == CL_CONNECTED)
		{
			HandleSpawnParms();
			continue;
		}
		//socket is only active. not connected or anything
		else if(m_state == CL_INUSE)
		{
			if(m_buffer.ReadInt() == -1)
			{
				HandleOOBMessage();
				continue;
			}
//ComPrintf("CClient::ReadPacket::Unknown packet from %s\n", m_pSock->GetSource().ToString());
		}
	}				
}

/*
======================================
Send off any messages we need to
======================================
*/
void CClient::SendUpdates()
{
	if(!m_pSock->ValidSocket())
		return;

	//we have spawned. send update packet
	if(m_state == CL_SPAWNED)
	{
		if(m_netChan.CanSend() && m_netChan.m_buffer.GetSize())
		{
			m_netChan.PrepareTransmit();
			m_pSock->Send(m_netChan.m_sendBuffer);
			m_netChan.m_buffer.Reset();
//ComPrintf("CL: Client sending update\n");
		}
		return;
	}

	//If the client has NOT spawned, then limit resends before we decide to fail
	if(m_numResends >= 4)
	{
ComPrintf("CL: Timed out\n");
			Disconnect();
			return;
	}

	//We have connected. Need to ask server for baselines
	if(m_state == CL_CONNECTED)
	{
		//Its been a while and our reliable packet hasn't been answered, try again
		if(m_fNextSendTime < System::g_fcurTime)
			m_netChan.m_reliableBuffer.Reset();
		
		//Ask for next spawn parm if we have received a reply to the last
		//otherwise, keep asking for the last one
		if(m_netChan.CanSendReliable())
		{
			m_netChan.m_buffer.Reset();
			m_netChan.m_buffer += m_levelId;
			m_netChan.m_buffer += (m_spawnState + 1);
			m_netChan.PrepareTransmit();

			m_pSock->Send(m_netChan.m_sendBuffer);

			//We got all the necessary info. just ack and switch to spawn mode
			if(m_spawnState == SVC_BEGIN)
			{
ComPrintf("CL: Client is ready to SPAWN\n");
				m_state = CL_SPAWNED;

			}
			else
			{
				m_fNextSendTime = System::g_fcurTime + 1.0f;
				m_numResends ++;
//ComPrintf("CL: Asking for spawnstate %d\n", m_spawnState+1);
			}
		}
	}
	//Unconnected socket. sends OOB queries
	else if((m_state == CL_INUSE) && 
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
Send connection parms
======================================
*/
void CClient::SendConnectReq()
{
	//Create Connection less packet
	m_buffer.Reset();
	m_buffer += -1;

	//Write Connection Parms
	m_buffer += C2S_CONNECT;			//Header
	m_buffer += VOID_PROTOCOL_VERSION;	//Protocol Version
	m_buffer += m_challenge;			//Challenge Req
//	m_buffer += m_virtualPort;			//Virtual Port
	
	//User Info
	m_buffer += m_clname.string;
	m_buffer += m_clrate.ival;

	m_pSock->Send(m_buffer);

	m_szLastOOBMsg = C2S_CONNECT;
	m_fNextSendTime = System::g_fcurTime + 2.0f;
	m_numResends ++; 

ComPrintf("CL: Attempting to connect to %s\n", m_svServerAddr);
}

/*
=======================================
Disconnect any current connection and 
initiates a new connection request
to the specified address
=======================================
*/
void CClient::SendChallengeReq()
{
	CNetAddr netAddr(m_svServerAddr);
	if(!netAddr.IsValid())
	{
		ComPrintf("CClient::ConnectTo: Invalid address\n");
		Disconnect();
		return;
	}

	//Now initiate a connection request
	//Create Connection less packet
	m_buffer.Reset();
	m_buffer += -1;
	m_buffer += C2S_GETCHALLENGE;

	m_pSock->SendTo(m_buffer, netAddr);

	m_szLastOOBMsg = C2S_GETCHALLENGE;
	m_fNextSendTime = System::g_fcurTime + 2.0f;
	m_numResends ++;

ComPrintf("CL: Requesting Challenge from %s\n", m_svServerAddr);
}


//======================================================================================
//Console funcs
//======================================================================================
/*
======================================
Start Connecting to the given addr
======================================
*/
void CClient::ConnectTo(const char * ipaddr)
{
	if(!ipaddr)
	{
		ComPrintf("usage - connect ipaddr(:port)\n");
		return;
	}

	if(m_ingame)
		Disconnect();

	if(!m_pSock->ValidSocket())
	{
		if(!m_pSock->Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
		{
			PrintSockError(WSAGetLastError(),"CClient::Init: Couldnt create socket");
			return;
		}
	}

	//Send a connection request
	CNetAddr netAddr(ipaddr);
	if(!netAddr.IsValid())
	{
		ComPrintf("CClient::ConnectTo: Invalid address\n");
		return;
	}
	strcpy(m_svServerAddr, netAddr.ToString());

	CNetAddr localAddr("localhost");
	if(localAddr == netAddr)
		m_bLocalServer = true;

	//Now initiate a connection request
	m_state = CL_INUSE;
	SendChallengeReq();
}

/*
=====================================
Disconnect if connected to a server
=====================================
*/
void CClient::Disconnect()
{
	if(m_ingame)
	{
		UnloadWorld();

		if(m_bLocalServer)
			System::GetConsole()->ExecString("killserver");
	}

	m_netChan.Reset();
	
	m_svServerAddr[0] = 0;
	m_bLocalServer = false;
	
	m_levelId = 0;
	m_state = CL_FREE;
	m_spawnState = 0;

	//Flow Control
	m_fNextSendTime = 0.0f;
	m_szLastOOBMsg = 0;
	m_numResends = 0;
}


/*
=====================================
Talk message sent to server if 
we are connected
=====================================
*/
void CClient::Talk(const char *string)
{
	if(m_state != CL_SPAWNED)
		return;

	//parse to right after "say"
	const char * msg = string + 4;
	while(*msg && *msg == ' ')
		msg++;

	if(!*msg || *msg == '\0')
		return;

	ComPrintf("%s: %s\n", m_clname.string, msg);
	System::GetSoundManager()->Play(m_hsTalk);

	//Send this reliably
	m_netChan.m_buffer += CL_TALK;
	m_netChan.m_buffer += msg;
}
