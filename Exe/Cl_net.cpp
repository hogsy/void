#include "Net_sock.h"
#include "Cl_main.h"

using namespace VoidNet;

/*
======================================
Read spawn info
======================================
*/
void CClient::HandleSpawnParms()
{

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
		ComPrintf("challenge %d\n", m_challenge);

		//Got challenge, now send connection request
		SendConnectParms();
		return;
	}
	
	if(!strcmp(msg,S2C_ACCEPT))
	{
		m_netChan.Setup(m_pSock->GetSource(),&m_buffer);
		char *map = m_buffer.ReadString();
		ComPrintf("map %s\n", map);
		LoadWorld(map);

		m_state = CL_CONNECTED;
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
			ComPrintf("CClient::ReadPacket::Unknown packet from %s\n", m_pSock->GetSource().ToString());
		}
	}				
}

/*
======================================

======================================
*/
void CClient::SendConnectParms()
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

	m_pSock->Send(m_buffer);

	m_szLastOOBMsg = C2S_CONNECT;
	m_fNextConReq = System::g_fcurTime + 2.0f;

	ComPrintf("Sending Connection Parms %s\n", m_svServerAddr);
}



/*
=======================================
Disconnect any current connection and 
initiates a new connection request
to the specified address
=======================================
*/
void CClient::SendConnectReq()
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
	m_fNextConReq = System::g_fcurTime + 2.0f;

	ComPrintf("Requesting Challenge : %s\n", m_svServerAddr);
}

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
	SendConnectReq();
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
	m_state = CL_FREE;
	m_fNextConReq = 0.0f;
	m_bLocalServer = false;
}



