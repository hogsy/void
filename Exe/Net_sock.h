#ifndef VOID_NET_SOCKET
#define VOID_NET_SOCKET

#include "Net_hdr.h"
#include "Com_buffer.h"

namespace VoidNet {

class CNetSocket
{
public:

	CNetSocket(CNetBuffer * buffer);
	~CNetSocket();

	bool Create(int addrFamily, int type, int protocol, bool blocking = false);
	void Close();
	
	bool Bind(const CNetAddr &addr);

	//Send data to given dest
	void Send(const CNetAddr &addr, const byte * data, int length);

	//Try to receive until there is no more data
	bool Recv();
 
	int  GetInterfaceList(INTERFACE_INFO ** addr, int numAddrs);

	CNetAddr	m_srcAddr;

	bool ValidSocket() const { return !(m_socket == INVALID_SOCKET); }

private:

	//Instance Data
	SOCKET		m_socket;
	
	CNetBuffer * m_pBuffer;
	
	SOCKADDR_IN m_srcSockAddr;
	SOCKADDR_IN m_destSockAddr;
};

}

#endif


