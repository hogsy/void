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
	m_netChan.BeginRead();

	m_numResends = 0;

	int id = m_buffer.ReadInt();

ComPrintf("CL: Got spawn parms %d\n",id);

	if(id == m_spawnState + 1)
	{
		m_spawnState = id;
		m_fNextSendTime = 0.0f;

		//got initial data, now ask for more
		if(m_spawnState == SVC_INITCONNECTION)
		{
			char * game = m_buffer.ReadString();
			ComPrintf("Game : %s\n", game);
			char * map = m_buffer.ReadString();
			ComPrintf("Map : %s\n", map);
			
			LoadWorld(map);

		}
		else if(m_spawnState == SVC_SPAWN)
		{
ComPrintf("CL: Client is ready to SPAWN\n");
			m_state = CL_SPAWNED;
			return;
		}
	}
}


/*
======================================

======================================
*/
void CClient::HandleOOBMessage()
{
	char * msg = m_buffer.ReadString();
	
	if(!strcmp(msg,S2C_CHALLENGE))
	{
		m_challenge = m_buffer.ReadInt();
		
		m_szLastOOBMsg = 0;
		m_fNextSendTime = 0.0f;
		m_numResends =0;

		//Got challenge, now send connection request
		SendConnectReq();

//ComPrintf("CL: challenge %d\n", m_challenge);
		return;
	}
	
	//We have been accepted. Now get ready to receive spawn parms
	if(!strcmp(msg,S2C_ACCEPT))
	{
		m_state = CL_CONNECTED;
		m_levelId = m_buffer.ReadInt();

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
			return;
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

======================================
*/
void CClient::SendUpdates()
{
	if(!m_pSock->ValidSocket())
		return;

	if(m_state == CL_SPAWNED)
	{
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
		{	m_netChan.m_reliableBuffer.Reset();
		}

		//Ask for next spawn parm if we have received a reply to the last
		//otherwise, keep asking for the last one
		if(m_netChan.CanSendReliable())
		{
			m_netChan.m_buffer.Reset();
			m_netChan.m_buffer += m_levelId;
			m_netChan.m_buffer += (m_spawnState + 1);
			m_netChan.PrepareTransmit();

			m_pSock->Send(m_netChan.m_sendBuffer);
			m_fNextSendTime = System::g_fcurTime + 1.0f;
			m_numResends ++;
ComPrintf("Client sending spawnstate %d\n", m_spawnState+1);
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

ComPrintf("Requesting Challenge from %s\n", m_svServerAddr);
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
	
	m_state = CL_FREE;
	m_spawnState = 0;

	//Flow Control
	m_fNextSendTime = 0.0f;
	m_szLastOOBMsg = 0;
	m_numResends = 0;
}
