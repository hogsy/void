#ifndef VOID_NET_SOCKET
#define VOID_NET_SOCKET

#include "Net_hdr.h"
#include "Net_chan.h"

class CNetSocket
{
public:

	CNetSocket(CBuffer * buffer);
	~CNetSocket();

	bool Create(int addrFamily, int type, int protocol, bool blocking = false);
	void Close();

	bool Bind(const CNetAddr &addr);

	//Send to source
	void Send(const byte * data, int length);
	void Send(const CBuffer &buffer);
	

	//Send data to given dest
	void SendTo(const CNetChan &netchan);
	void SendTo(const byte * data, int length, const CNetAddr &addr);
	void SendTo(const CBuffer &buffer, const CNetAddr &addr);
	
	//Receive any data waiting on the socket
	bool RecvFrom();	//listener socket
	bool Recv();		//connected client socket

	const CNetAddr & GetSource() const { return m_srcAddr; }
	bool  ValidSocket() const { return !(m_socket == INVALID_SOCKET); }
 
	int   GetInterfaceList(INTERFACE_INFO ** addr, int numAddrs);

private:

	CNetAddr	m_srcAddr;

	//Instance Data
	SOCKET		m_socket;
	
	CBuffer *   m_pBuffer;
	
	SOCKADDR_IN m_srcSockAddr;
	SOCKADDR_IN m_destSockAddr;
};

#endif