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

//	void Disconnect();
	
	bool Bind(const CNetAddr &addr);

	//Send to source
	void Send(const byte * data, int length);
	void Send(const CBuffer &buffer);
	
	void Send(const CNetChan &netchan);

	//Send data to given dest
	void SendTo(const byte * data, int length, const CNetAddr &addr);
	void SendTo(const CBuffer &buffer, const CNetAddr &addr);
	
	//Try to receive until there is no more data
	bool Recv();

	//recv from only the source now. used by client to only listen to message
	//from the server once its connected
	bool RecvFromServer();

	const CNetAddr & GetSource() const { return m_srcAddr; }
 
	int  GetInterfaceList(INTERFACE_INFO ** addr, int numAddrs);

	bool ValidSocket() const { return !(m_socket == INVALID_SOCKET); }

private:

	CNetAddr	m_srcAddr;

	//Instance Data
	SOCKET		m_socket;
	
	CBuffer * m_pBuffer;
	
	SOCKADDR_IN m_srcSockAddr;
	SOCKADDR_IN m_destSockAddr;
};


#endif


