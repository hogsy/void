#ifndef VOID_NETWORK_PRIVATE_HDR
#define VOID_NETWORK_PRIVATE_HDR

#include <winsock2.h>
#include <ws2tcpip.h>

namespace VoidNet {

void PrintSockError(int err=0,const char *msg=0);

/*
==========================================
Internal network address
==========================================
*/
class  CNetAddr
{
public:
	
	CNetAddr();
	CNetAddr(const char * szaddr);
	
	//Assignment operators
	VoidNet::CNetAddr & operator = (const SOCKADDR_IN &saddr);
	VoidNet::CNetAddr & operator = (const VoidNet::CNetAddr &addr);
	VoidNet::CNetAddr & operator = (const char * szaddr);

//FIX ME ! How the hell do I get this to link outside ??
	//Equality check
	friend bool operator == (const VoidNet::CNetAddr &laddr, const VoidNet::CNetAddr &raddr)
	{
		if((laddr.ip[0] == raddr.ip[0]) &&
		   (laddr.ip[1] == raddr.ip[1]) &&
		   (laddr.ip[2] == raddr.ip[2]) &&
		   (laddr.ip[3] == raddr.ip[3]) &&
		   (laddr.port  == raddr.port))
			return true;
		return false;
	}

	//Conversion
	const char * ToString() const;
	void ToSockAddr(SOCKADDR_IN &saddr) const;
	
	//Util
	void Print()   const;
	bool IsValid() const;

	static void SetLocalAddress(const char * localaddy);

private:

	void Set(const char * szaddr);

	byte	ip[4];
	short	port;
	bool	valid;

	static char  m_szLocalAddress[24];
};


}


#endif