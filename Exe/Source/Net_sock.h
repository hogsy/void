#ifndef VOID_NET_SOCKET
#define VOID_NET_SOCKET

#include "Net_util.h"
#include "Com_defs.h"

namespace VoidNet {

class CNetSocket
{
public:

	CNetSocket(CNetBuffer ** buffer);
	~CNetSocket();

	bool Create(int addrFamily, int type, int protocol);
	void Close();
	
	bool Bind(const char * addr, short port, bool blocking = false);

	//Send data to given dest
	void Send(const char *ipaddr, const byte *data, int length);
	void Send(SOCKADDR_IN &addr,  const byte *data, int length);
	
	//Try to receive until there is no more data
	bool Recv();
 
	int  GetInterfaceList(INTERFACE_INFO ** addr, int numAddrs);

	//Instance Data
	SOCKET		m_socket;
	SOCKADDR_IN m_srcAddr;		//This might change on every recv
	CNetBuffer *m_pBuffer;
};

}

#endif


