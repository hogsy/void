#include "Net_util.h"
#include "Net_sock.h"
#include "Sys_hdr.h"

#include <assert.h>


int CSocket::m_numSocks=0;

/*
=======================================
Default constructor
=======================================
*/

CSocket::CSocket(CBaseNBuffer * recv, CBaseNBuffer *send)
{
//	assert(recv);
//	assert(send);

	m_recvBuf = recv;
	m_sendBuf = send;

//	timeoutperiod = timeout;

	m_id = m_numSocks;
	m_numSocks++;

	connecttime=0.0;
	lastrecvtime=0.0;
	lastsendtime=0.0;

	m_state=SOCK_INACTIVE;

	bcansend=false;
	bsend=false;
	brecv=false;

	m_socket= INVALID_SOCKET;

	memset(&m_addr, 0, sizeof(struct sockaddr_in));

	m_pNetworkEvents = new WSANETWORKEVENTS;

//	sendseq=0;
//	recvseq=0;
}

/*
=======================================
Destructor
=======================================
*/
CSocket::~CSocket()
{
	if(m_socket != INVALID_SOCKET)
		closesocket(m_socket);

	delete m_pNetworkEvents;

//	m_pNetworkEvents =0;
}


/*
=====================================
Socket Initialization
-create the socket
-pass pointers to the incoming and outgoing buffers objec
 we want to use for this socket

=====================================
*/

bool CSocket::Init()
{
	if(m_state == SOCK_IDLE)
		Close();

	m_socket = socket(AF_INET,SOCK_DGRAM,0);

	if (m_socket == INVALID_SOCKET)
	{
		SockError("CSocket::Init:ERROR:Couldnt create socket");
		Close();
		return false;
	}


	// Disable send buffering on the socket.  Setting SO_SNDBUF
    // to 0 causes winsock to stop bufferring sends and perform
    // sends directly from our buffers, thereby reducing CPU usage.

    int zero = 0;
	if(setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero)) == SOCKET_ERROR)
	{
		SockError("CSocket::Init: setsockopt(SO_SNDBUF)");
		Close();
		return false;
	}

    // Disable receive buffering on the socket.  Setting SO_RCVBUF 
    // to 0 causes winsock to stop bufferring receive and perform
    // receives directly from our buffers, thereby reducing CPU usage.
    zero = 0;
	if(setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero)) == SOCKET_ERROR)
	{
		SockError("CSocket::Init: setsockopt(SO_RCVBUF)");
		Close();
		return false;
	}

/*        
    nRet = setsockopt(m_socket, SOL_SOCKET, SO_LINGER,
                  (char *)&lingerStruct, sizeof(lingerStruct) );
    if (SOCKET_ERROR == nRet) 
       {
        printf("setsockopt(SO_LINGER): %d\n", WSAGetLastError());
        return(FALSE);
        }
  
*/
	m_state = SOCK_IDLE;
	return true;
}


/*
======================================
Close
======================================
*/
bool CSocket::Close()
{
	if(m_state == SOCK_INACTIVE)
		return true;

	Disconnect();

/*	if(m_state == SOCK_CONNECTING ||
	   m_state == SOCK_CONNECTED)
	{
		if(WSAEventSelect(m_socket,m_event,0) == SOCKET_ERROR)
				SockError("CSocket::Run:Error in WSAEventSelect reset to 0");
	}

	m_state = SOCK_INACTIVE;
	
	connecttime=0.0;
	lastrecvtime=0.0;
	lastsendtime=0.0;
	
	bcansend=false;
	bsend=false;
	brecv=false;
*/
//	sendseq=0;
//	recvseq=0;

	closesocket(m_socket);
	m_state = SOCK_INACTIVE;
	memset(&m_addr, 0, sizeof(struct sockaddr_in));
	return true;
}


/*
=====================================
Disconnect
=====================================
*/

bool CSocket::Disconnect()
{
	if(m_state == SOCK_CONNECTING ||
		m_state == SOCK_CONNECTED)
	{
		if(WSAEventSelect(m_socket,m_event,0) == SOCKET_ERROR)
				SockError("CSocket::Run:Error in WSAEventSelect reset to 0");
	}

	m_state = SOCK_IDLE;
	
	connecttime=0.0;
	lastrecvtime=0.0;
	lastsendtime=0.0;
	
	bcansend=false;
	bsend=false;
	brecv=false;
	return true;
}



/*
=====================================
Connect to the specified ipaddr and the port
=====================================
*/

bool CSocket::Connect(char *ipaddr, int port)
{
	if(m_state == SOCK_CONNECTING ||
	   m_state == SOCK_CONNECTED)
		Disconnect();
	if(m_state == SOCK_INACTIVE)
	{
		if(!Init())		
		{
			ComPrintf("CSocket::Connect:Couldnt reinit socket\n");
			return false;
		}
	}
	
	memset(&m_addr, 0, sizeof(struct sockaddr_in));
	m_addr.sin_family = AF_INET; 
	m_addr.sin_port = htons(port);
	m_addr.sin_addr.s_addr = inet_addr(ipaddr);

	if(connect(m_socket,(LPSOCKADDR)&m_addr,sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if((err != WSAEWOULDBLOCK) && (err != WSAEALREADY))
		{
			SockError("CSocket::Connect:: Error trying to connect\n");
			Close();
			return false;
		}
	}

	//Create Event
	m_event = WSACreateEvent();
	if(m_event ==WSA_INVALID_EVENT)
	{
		SockError("CSocket::Init: Error creating event");
		Close();
		return false;
	}

	if(WSAEventSelect(m_socket,m_event,FD_WRITE|FD_READ) == SOCKET_ERROR)
	{
		SockError("CSocket::Connect:Error in WSAEventSelect");
		return false;
	}

	ComPrintf("%d:CSocket::Connect:: Request sent\n",m_id);
	m_state = SOCK_CONNECTING;
	lastsendtime = System::g_fcurTime;
	return true;
}


/*
=====================================
Connect to the specified addr info
=====================================
*/
#if 1
bool CSocket::Connect(SOCKADDR_IN raddr, int port)
{
	if(m_state == SOCK_CONNECTING ||
	   m_state == SOCK_CONNECTED)
		Disconnect();

	if(m_state == SOCK_INACTIVE)
	{
		if(!Init())
		{
			ComPrintf("CSocket::Connect:Couldnt reinit socket\n");
			return false;
		}
	}
	
	memset(&m_addr, 0, sizeof(struct sockaddr_in));
	m_addr = raddr;

	if(port)
		m_addr.sin_port = htons(port);

	if(connect(m_socket,(LPSOCKADDR)&m_addr,sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if((err != WSAEWOULDBLOCK) && (err != WSAEALREADY))
		{
			SockError("CSocket::Connect:: Error trying to connect\n");
			Close();
			return false;
		}
	}

	//Create Event
	m_event = WSACreateEvent();
	if(m_event ==WSA_INVALID_EVENT)
	{
		SockError("CSocket::Init: Error creating event");
		Close();
		return false;
	}

	if(WSAEventSelect(m_socket,m_event,FD_WRITE|FD_READ) == SOCKET_ERROR)
	{
		SockError("CSocket::Connect:Error in WSAEventSelect");
		return false;
	}

	ComPrintf("%d:CSocket::Connect:: Request sent\n",m_id);
	m_state = SOCK_CONNECTING;
	lastsendtime = System::g_fcurTime;
	return true;
}
#endif


/*
=====================================
Bind socket to a port
=====================================
*/

bool CSocket::Bind(SOCKADDR_IN addr, int port, bool loopback)
{
	if(m_state == SOCK_INACTIVE)
		Init();

	if(loopback)
	{
//	   addr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
	   BOOL on = 1;
	   if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) == SOCKET_ERROR)
		{
			SockError("CSocket::Init: setsockopt(SO_REUSE)");
			Close();
			return false;
		}
	}

	// bind the socket to the internet address 
//	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(m_socket, (struct sockaddr *)&addr, sizeof(m_addr)) == SOCKET_ERROR) 
	{
		SockError("CSocket::Bind:ERROR:Couldnt bind socket");
		return false;
	}
	ComPrintf("%d:Socket bound on %s:%d\n",m_id,inet_ntoa(addr.sin_addr),port);
	return true;
}



/*
=======================================
Send data in buffer
=======================================
*/
bool CSocket::Send()
{
	if(bcansend)
	{
		int sent= send (m_socket, (char *)m_sendBuf->data, m_sendBuf->cursize, 0);
		if(sent == SOCKET_ERROR)
		{
			bsend= false;
			bcansend = false;
			m_sendBuf->Reset();
			SockError("CSocket::Send: error sending data");
			return false;
		}

		if(sent != (m_sendBuf->cursize))
			ComPrintf("%d:CSocket::Send:couldnt send entire buffer\n",m_id);

//		ComPrintf("%d:CSocket::Sent %s (%d/%d bytes)\n",
//							m_id,(char*)m_sendBuf->data,sent,m_sendBuf->cursize);

		lastsendtime = System::g_fcurTime;
		bsend= false;
		m_sendBuf->Reset();
//		sendseq++;
		return true;
	}
	ComPrintf("%d:CSocket::Send: error not connected\n,m_id");
	return false;
}

/*
=======================================
Receive Data into Buffer
=======================================
*/
bool CSocket::Recv()
{
	if((m_state == SOCK_CONNECTED) || 
	   (m_state == SOCK_CONNECTING))
	{
		m_recvBuf->Reset();

		int recvMsgLen = recv(m_socket,(char *)m_recvBuf->data,MAX_DATAGRAM,0);
		if(recvMsgLen == SOCKET_ERROR)
		{
			SockError("CSocket::Recv: error receiving data");
			return false;
		}

//		ComPrintf("%d:CSocket::Recv:%s (%d bytes)\n",m_id,(char *)m_recvBuf->data,recvMsgLen);
		
		brecv = true;
		m_recvBuf->cursize = recvMsgLen;
		lastrecvtime = System::g_fcurTime;
//		recvseq++;
		return true;
	}
	ComPrintf("CSocket::Recv: error not connected\n");
	return false;
}


/*
======================================
Socket Frame
run once every whatever
======================================
*/


void CSocket::Run()
{
	//see if there is any incoming data if we were listening ESockState::
	switch(m_state)
	{
	case SOCK_INACTIVE:			//socket is closed
	case SOCK_IDLE:				//socket is active, but not doing anything
		break;
	case SOCK_CONNECTING:		
		{
			int error;
			error = WSAEnumNetworkEvents(m_socket,m_event,m_pNetworkEvents);
	
			if(error == SOCKET_ERROR)
			{
				SockError("CSocket::Run:Connecting WSAEnumNetevents\n");
				return;
			}

			if(m_pNetworkEvents->lNetworkEvents & FD_WRITE)
			{
//				ComPrintf("%d:CSocket::Run:Connecting ready to write\n",m_id);
				bcansend = true;
			}

			if(bsend && bcansend)
			{
				Send();
			}

			//the socket doenst change to "connected" until the client accepts the connection
			if(m_pNetworkEvents->lNetworkEvents & FD_READ)
			{
				Recv();
				return;
			}
			
			//havent been able to send out anything for this time
			//looks like the connection didnt go through
			if((System::g_fcurTime - lastsendtime) > CON_TIMEOUT)
			{
				if(WSAEventSelect(m_socket,m_event,0) == SOCKET_ERROR)
				{
					SockError("CSocket::Run:Error in WSAEventSelect reset to 0");
				}
				Close();
				ComPrintf("%d:CSocket::Run:Connection req timed out\n",m_id);
			}
			break;
		}
	case SOCK_CONNECTED:
		{
			int error;
			error = WSAEnumNetworkEvents(m_socket,m_event,m_pNetworkEvents);
	
			if(error == SOCKET_ERROR)
			{
				SockError("CSocket::Run: WSAEnumNetevents Error\n");
				return;
			}

			if(m_pNetworkEvents->lNetworkEvents & FD_WRITE)
			{
				bcansend =true;
			}

			if(bsend && bcansend)
				Send();
			
			if(m_pNetworkEvents->lNetworkEvents & FD_READ)
			{
				Recv();
				return;
			}
			
			//havent received anything for a while
			if((System::g_fcurTime - lastrecvtime) > GAME_TIMEOUT)
			{
				if(WSAEventSelect(m_socket,m_event,0) == SOCKET_ERROR)
				{
					SockError("CSocket:: connected -Error in WSAEventSelect reset to 0");
				}
				Close();
				ComPrintf("%d:CSocket::Run:Connection timed out\n",m_id);
			}
			break;
		}
	}
}


/*
=====================================
Client says "Accept the connection"
=====================================
*/

bool CSocket::AcceptConnection()
{
	if(m_state == SOCK_CONNECTING &&
	   bcansend == true)
	{
		m_state = SOCK_CONNECTED;
		connecttime = System::g_fcurTime;
		return true;
	}
	return false;
}



/*
=====================================
Socket Error
=====================================
*/

void CSocket::SockError(char *err)
{
	if(err)
	{
		ComPrintf("%d:%s:",m_id,err);
		PrintSockError();
	}
}


