#include "Net_chan.h"
#include "Sv_main.h"
#include "Sv_client.h"

//======================================================================================
//======================================================================================

CSVClient::CSVClient()
{
	m_pNetChan = new CNetChan();	
}


CSVClient::~CSVClient()
{
	delete m_pNetChan;
}











#if 0

#include "Sv_client.h"
#include "Sv_main.h"		//for status/info printing


extern CNetClients g_netclients;

/*
=====================================
Constructor
=====================================
*/

CSVClient::CSVClient():m_sock(&m_recvBuf,&m_sendBuf)
{
	m_active = false;
	m_ingame = false;
	m_connected = false;			
	m_lastmessagetime = 0.0f;
	m_rate = 0.0f;
	m_retries =0;

	memset(name,0,sizeof(name));
	memset(ipaddr,0,sizeof(ipaddr));
}


/*
=====================================
Destructor
=====================================
*/

CSVClient::~CSVClient()
{
}


/*
=====================================
Initialize the socket and bind it to
this addr info
=====================================
*/

bool CSVClient::SV_InitClient(SOCKADDR_IN laddr, int port)
{
	m_recvseq= 0;
	m_sendseq= 0;

	if(!m_sock.Init())
	{
		ComPrintf("CNetclient::SV_InitClient:couldnt init client socket\n");
		return false;
	}

	SOCKADDR_IN addr;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	addr.sin_family = AF_INET;
	addr.sin_port = htons((int)port);

	if(!m_sock.Bind(addr,port))
	{
		ComPrintf("CNetclient::SV_InitClient:couldnt bind client socket\n");
		return false;
	}
	m_port = port;
	ComPrintf("SVClient bound at %d\n",port);
	return true;
}


/*
=====================================
Connect to this ip
=====================================
*/

bool CSVClient::SV_Connect(SOCKADDR_IN raddr, int rport, int numclient)
{
	if(!m_sock.Connect(raddr,rport))
	{
		ComPrintf("CNetclient::SV_InitClient:Connection FAILED!\n");
		return false;
	}

	strcpy(ipaddr,inet_ntoa(raddr.sin_addr));
	ComPrintf("SV_Client connecting to %s:%d\n",ipaddr,rport);
	num = numclient;
	m_active = true;
	return true;
}


/*
=====================================
Write to Client
=====================================
*/

void CSVClient::WriteToClient(int type, unsigned char *msg)
{
	if(m_connected)
	{
		m_sendBuf.Reset();

		m_sendBuf.WriteLong(m_sendseq);
		m_sendBuf.WriteByte(type);
		m_sendBuf.WriteString((char *)msg);
	
		m_sock.bsend = true;
	}
}


/*
=====================================
Run the server side Client 
=====================================
*/

void CSVClient::Run()
{
	m_sock.Run();

	//able to send now
	if(m_sock.bcansend)
	{
		char *buf;
		
		if(!m_connected && !m_sock.brecv && 
			((g_fcurTime - m_lastmessagetime) > CON_TIMEOUT) &&
			(m_retries < 3))
		{
			m_sendBuf.Reset();
			g_pServer->WriteInfoMsg(&m_sendBuf);
			m_sock.bsend = true;
			
			m_lastmessagetime = g_fcurTime;
			m_retries++;

			if(m_retries >= 3)
			{
				ComPrintf("CSVClient::connnecting:%s Timed out\n",ipaddr);
				SV_Disconnect();
			}
			return;
		}
		
		//client is trying to connect
		if(!m_connected)
		{
			if(!m_sock.brecv)
				return;

			buf = m_recvBuf.ReadString(INFOSTRING_DELIM);
		
			//get name
			if(buf && buf[0] == 'n')
			{
				buf = m_recvBuf.ReadString(INFOSTRING_DELIM);
				if(buf)
				{
					strcpy(name,buf);
					buf = m_recvBuf.ReadString(INFOSTRING_DELIM);
					
					//get rate
					if(buf && buf[0] == 'r')
					{
						//got all the info, accept the connection
						//and send the server info
						m_rate = (float)m_recvBuf.ReadLong();
						if(m_rate != -1)
						{
							m_sendBuf.Reset();
							g_pServer->WriteInfoMsg(&m_sendBuf);
							m_sock.bsend = true;
								
							if(m_sock.AcceptConnection())
							{
								m_connected = true;
								ComPrintf("%d:CSVClient::%s connected\n",num,name);
								return;
							}
						}
					}
				}
			}
			ComPrintf("CSVClient::Run():bad connection request\n");
			return;
		}

		//he is connected now
		if(!m_ingame)
		{
			//send server info

			//send spawn parms

			//wait for acknowledgement first though.

			//client is connected and ingame
			ComPrintf("%d:CSVClient::%s entered the game\n",num,name);
			m_ingame = true;
			return;
		}

		//received something
		if(m_sock.brecv)
		{
			int i=m_recvBuf.ReadLong();
			char b= m_recvBuf.ReadByte();
			
			switch(b)
			{
			case CL_NOP:
				{
					m_recvBuf.Reset();
					break;
				}
			case CL_STRING:
				{
					buf = m_recvBuf.ReadString();
					if(!buf) { return;}

					if(!strncmp(buf,"say",3))
					{
						char message[256];
						
						buf+=4;
						sprintf(message,"%s:%s\n",name,buf);
						g_netclients.WriteTalkMessage(message,num,0);
					}
					m_recvBuf.Reset();
					break;
				}
			}
		}

		
		if((g_fcurTime - m_sock.lastsendtime) > 1.0)
		{
			m_sendBuf.Reset();
					
			//Client info
			m_sendBuf.WriteLong(m_sendseq);
			m_sendBuf.WriteByte(SV_NOP);
			m_sock.bsend = true;
			m_sendseq++;
		}
	}
}


/*
=====================================
Disconnect the client
=====================================
*/
bool CSVClient::SV_Disconnect()
{
	if(m_active)
	{
		if((g_fcurTime - m_sock.lastrecvtime) > GAME_TIMEOUT)
			ComPrintf("%s timed out\n",name);
		else
			ComPrintf("%s disconnected\n",name);
	}
	
	m_active = false;
	m_ingame = false;
	m_connected = false;
	m_lastmessagetime = 0.0f;
	m_rate = 0;
	m_retries =0;
	
	num = 0;
	memset(name,0,sizeof(name));
	memset(ipaddr,0,sizeof(ipaddr));

	m_sendBuf.Reset();
	m_recvBuf.Reset();

	return m_sock.Close();
}

#endif