#include "Net_hdr.h"
#include "Net_chan.h"
#include "Net_sock.h"

using namespace VoidNet;

/*
==========================================
Constructor Destructor
==========================================
*/
CNetSocket::CNetSocket(CBuffer * buffer) : m_pRecvBuf(buffer), 
										   m_socket(INVALID_SOCKET)
{
	memset(&m_srcSockAddr,0,sizeof(SOCKADDR_IN));
	memset(&m_destSockAddr,0,sizeof(SOCKADDR_IN));
}

CNetSocket::~CNetSocket()
{
	m_pRecvBuf = 0;
	Close();
}

/*
==========================================
Create the socket
==========================================
*/
bool CNetSocket::Create(int addrFamily, int type, int protocol, bool blocking)
{
	Close();
	m_socket = socket(addrFamily, type, protocol);
	if(m_socket == INVALID_SOCKET)
		return false;

	if(blocking == false)
	{
		ulong val = 1;
		if(ioctlsocket(m_socket, FIONBIO, &val) == SOCKET_ERROR)
		{
			PrintSockError(WSAGetLastError(),"CNetSocket::Create:IoctlSocket:");
			return false;
		}
	}
	return true;
}

/*
==========================================
Close the socket
==========================================
*/
void CNetSocket::Close()
{
	if(m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

/*
==========================================
Bind the socket
==========================================
*/
bool CNetSocket::Bind(const CNetAddr &addr)
{
	SOCKADDR_IN sockAddr;
	addr.ToSockAddr(sockAddr);
	
	ComPrintf("CNetSocket::Bind: binding to %s\n", addr.ToString());
	
	if(bind(m_socket,(SOCKADDR*)&sockAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		PrintSockError(WSAGetLastError(),"CNetSocket::Bind:");
		return false;
	}
	return true;
}

/*
==========================================
Get Interface List associated with the socket
==========================================
*/
int CNetSocket::GetInterfaceList(INTERFACE_INFO ** addr, int numAddrs)
{
	int   wsError =0;
	DWORD bytesReturned =0;
	
	wsError = WSAIoctl(m_socket, SIO_GET_INTERFACE_LIST, 
						NULL, 0, 
						addr, sizeof(INTERFACE_INFO)*numAddrs,
						&bytesReturned, 
						NULL, NULL);

	if (wsError == SOCKET_ERROR || bytesReturned == 0)
	{
		PrintSockError(WSAGetLastError(),"CNetSocket::GetInterfaceList:WSAIoctl failed");
		return 0;
	}
	return (bytesReturned/sizeof(INTERFACE_INFO));
}

/*
==========================================
Try to receive data
==========================================
*/
bool CNetSocket::RecvFrom()
{
	int srcLen=  sizeof(SOCKADDR_IN);
	
	m_pRecvBuf->Reset();
	int ret = recvfrom(m_socket,
					   (char*)m_pRecvBuf->GetData(), 
					   m_pRecvBuf->GetMaxSize(), 
					   0, (SOCKADDR*)&m_srcSockAddr, &srcLen);
	
	if(ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK)
			return false;
		//Socket has been shutdown
		if(err == WSAECONNRESET)
		{
			return false;
		}
		if(err == WSAEMSGSIZE)
		{
			ComPrintf("CNetSocket::RecvFrom: Oversize packet from %s\n", inet_ntoa (m_srcSockAddr.sin_addr));
			return false;
		}
		PrintSockError(err,"CNetSocket::RecvFrom:");
		return false;
	}

	if(ret == m_pRecvBuf->GetMaxSize())
	{
		ComPrintf("CNetSocket::RecvFrom: Oversize packet from %s\n",inet_ntoa (m_srcSockAddr.sin_addr));
		return false;
	}

//	ComPrintf("Recved %d from %s\n", ret,inet_ntoa (m_srcSockAddr.sin_addr));
	m_srcAddr = m_srcSockAddr;
	m_pRecvBuf->SetSize(ret);
	return true;
}


/*
======================================
recv from only the source now. 
used by client to only listen to message
from the server once its connected
======================================
*/
bool CNetSocket::Recv()
{
	int srcLen=  sizeof(SOCKADDR_IN);
	
	m_pRecvBuf->Reset();
	int ret = recv(m_socket,
				   (char*)m_pRecvBuf->GetData(), 
				   m_pRecvBuf->GetMaxSize(), 
				   0);
	
	if(ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK)
			return false;
		
		//Socket has been shutdown
		if(err == WSAECONNRESET)
		{	return false;
		}
		if(err == WSAEMSGSIZE)
		{
			ComPrintf("CNetSocket::Recv: Oversize packet from %s\n", inet_ntoa (m_srcSockAddr.sin_addr));
			return false;
		}
		PrintSockError(err,"CNetSocket::Recv:");
		return false;
	}

	if(ret == m_pRecvBuf->GetMaxSize())
	{
		ComPrintf("CNetSocket::Recv: Oversize packet from %s\n",inet_ntoa (m_srcSockAddr.sin_addr));
		return false;
	}

//	ComPrintf("Recved %d from %s\n", ret,inet_ntoa (m_srcSockAddr.sin_addr));
//	m_srcAddr = m_srcSockAddr;
	m_pRecvBuf->SetSize(ret);
	return true;
}


/*
======================================
Send to the given destination
======================================
*/
void CNetSocket::SendTo(const CBuffer &buffer, const CNetAddr &addr)
{
	addr.ToSockAddr(m_destSockAddr);

	int ret = sendto (m_socket, (char*)buffer.GetData(), buffer.GetSize(),0, 
					(SOCKADDR *)&m_destSockAddr, sizeof(SOCKADDR_IN));

	if(ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK)
		{
			ComPrintf("CNetSocket::Send: to %s, WSAEWOULDBLOCK\n", inet_ntoa(m_destSockAddr.sin_addr));
			return;
		}
		PrintSockError(err,"CNetSocket::Send:");
		return;
	}
//	ComPrintf("Sent %d bytes to %s\n", buffer.GetSize(), addr.ToString());

	/*
// wouldblock is silent
        if (err == WSAEWOULDBLOCK)
	        return;

#ifndef SERVERONLY
		if (err == WSAEADDRNOTAVAIL)
			Con_DPrintf("NET_SendPacket Warning: %i\n", err);
		else
#endif
			Con_Printf ("NET_SendPacket ERROR: %i\n", errno);
	}
*/
}


/*
==========================================
Try to send data
==========================================
*/
void CNetSocket::SendTo(const byte * data, int length, const CNetAddr &addr)
{
	addr.ToSockAddr(m_destSockAddr);

	int ret = sendto (m_socket, (char*)data, length,0, (SOCKADDR *)&m_destSockAddr, sizeof(SOCKADDR_IN));

	if(ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK)
		{
			ComPrintf("CNetSocket::Send: to %s, WSAEWOULDBLOCK\n", inet_ntoa(m_destSockAddr.sin_addr));
			return;
		}
		PrintSockError(err,"CNetSocket::Send:");
		return;
	}
//	ComPrintf("Sent %d bytes to %s\n", length, addr.ToString());

	/*
// wouldblock is silent
        if (err == WSAEWOULDBLOCK)
	        return;

#ifndef SERVERONLY
		if (err == WSAEADDRNOTAVAIL)
			Con_DPrintf("NET_SendPacket Warning: %i\n", err);
		else
#endif
			Con_Printf ("NET_SendPacket ERROR: %i\n", errno);
	}
*/
}



void CNetSocket::Send(const byte * data, int length)
{	SendTo(data,length,m_srcAddr);
}

void CNetSocket::Send(const CBuffer &buffer)
{	SendTo(buffer,m_srcAddr);
}

void CNetSocket::SendTo(const CNetChan * pNetchan)
{	SendTo(pNetchan->m_sendBuffer,pNetchan->m_addr);
}











