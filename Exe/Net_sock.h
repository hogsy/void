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

	//Send to source
	void Send(const byte * data, int length);
	void Send(const CNetBuffer &buffer);

	//Send data to given dest
	void SendTo(const byte * data, int length, const CNetAddr &addr);
	void SendTo(const CNetBuffer &buffer, const CNetAddr &addr);
	
	//Try to receive until there is no more data
	bool Recv();

	const CNetAddr & GetSource() const { return m_srcAddr; }
 
	int  GetInterfaceList(INTERFACE_INFO ** addr, int numAddrs);

	bool ValidSocket() const { return !(m_socket == INVALID_SOCKET); }

private:

	CNetAddr	m_srcAddr;

	//Instance Data
	SOCKET		m_socket;
	
	CNetBuffer * m_pBuffer;
	
	SOCKADDR_IN m_srcSockAddr;
	SOCKADDR_IN m_destSockAddr;
};

}

#endif


