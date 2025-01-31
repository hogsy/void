#ifndef VOID_NET_UTIL
#define VOID_NET_UTIL


struct sockaddr_in;

namespace VoidNet {

class  CNetAddr
{
public:
	
	CNetAddr();
	CNetAddr(const char * szaddr);
	
	//Assignment operators
	CNetAddr & operator = (const sockaddr_in &saddr);
	CNetAddr & operator = (const CNetAddr &addr);
	CNetAddr & operator = (const char * szaddr);

//FIX ME ! How the hell do I get this to link outside ??
	//Equality check
	friend bool operator == (const CNetAddr &laddr, const CNetAddr &raddr)
	{
		if((laddr.ip[0] == raddr.ip[0]) &&
		   (laddr.ip[1] == raddr.ip[1]) &&
		   (laddr.ip[2] == raddr.ip[2]) &&
		   (laddr.ip[3] == raddr.ip[3]) &&
		   (laddr.port  == raddr.port))
			return true;
		return false;
	}

	void Reset();

	//Conversion
	const char * ToString() const;
	void ToSockAddr(sockaddr_in &saddr) const;
	
	//Util
	void Print()   const;
	bool IsValid() const;

	static void SetLocalServerAddr(const char * localaddy);
	static const char * GetLocalServerAddr();

private:

	void Set(const char * szaddr);

	byte	ip[4];
	ushort	port;
	bool	valid;
};

}

#endif
