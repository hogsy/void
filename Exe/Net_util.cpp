#include "Net_hdr.h"
#include "Net_util.h"
#include "Net_defs.h"

using namespace VoidNet;

//======================================================================================
//Network Address class
//======================================================================================

char CNetAddr::m_szLocalAddress[24];
void CNetAddr::SetLocalAddress(const char * localaddy)
{	strcpy(m_szLocalAddress, localaddy);
}

/*
==========================================
Constructors
==========================================
*/
CNetAddr::CNetAddr()
{	Reset();
}

CNetAddr::CNetAddr(const char * szaddr)
{	Set(szaddr);
}

void CNetAddr::Reset()
{
	ip[0] = ip[1] = ip[2] = ip[3] =0;
	port = 0;
	valid = true;
}

/*
==========================================
Assignment operators
==========================================
*/
CNetAddr & CNetAddr::operator = (const sockaddr_in &saddr)
{
	*(int *)&ip = *(int *)&saddr.sin_addr;
	port = saddr.sin_port;
	valid = true;
	return (*this);
}

CNetAddr & CNetAddr::operator = (const CNetAddr &addr)
{
	if(addr == *this)
		return *this;

	ip[0] = addr.ip[0];	ip[1] = addr.ip[1];
	ip[2] = addr.ip[2];	ip[3] = addr.ip[3];
	port = addr.port;
	valid = true;
	return (*this);
}

CNetAddr & CNetAddr::operator = (const char * szaddr)
{
	Set(szaddr);
	return (*this);
}

void CNetAddr::Set(const char * szaddr)
{
	char		stringaddr[128];
	sockaddr_in sockAddr;
	
	if(!szaddr || !strcmp(szaddr,"localhost"))
		strcpy(stringaddr, m_szLocalAddress);
	else	
		strcpy (stringaddr,szaddr);

	//Strip port number if specified
	sockAddr.sin_port = 0;
	for (char * colon = stringaddr ; *colon ; colon++)
	{
		if (*colon == ':')
		{
			*colon = 0;
			short sport = (short)atoi(colon+1);
			sockAddr.sin_port = htons(sport);	
		}
	}

	//if no port number then default
	if(sockAddr.sin_port == 0)
		sockAddr.sin_port = htons(SV_DEFAULT_PORT);

	if (stringaddr[0] >= '0' && stringaddr[0] <= '9')
		*(int *)&sockAddr.sin_addr = inet_addr(stringaddr);
	else	//Resolve hostname
	{
		HOSTENT	*host = 0;
		if ((host = gethostbyname(stringaddr)) == 0)
		{
			valid = false;
			return;
		}
		*(int *)&sockAddr.sin_addr = *(int *)host->h_addr_list[0];
	}

	//Convert to SOCKADDR_IN
	*(int *)&ip = *(int *)&sockAddr.sin_addr;
	port = sockAddr.sin_port;
	valid = true;
}

/*
==========================================
Utility
==========================================
*/
const char * CNetAddr::ToString() const
{
	static char stringaddr[64];
	
	if(port > 0)
		sprintf(stringaddr,"%i.%i.%i.%i:%i", ip[0],ip[1],ip[2],ip[3], ntohs(port));
	else
		sprintf(stringaddr,"%i.%i.%i.%i", ip[0],ip[1],ip[2],ip[3]);
	return stringaddr;
}

//Set the sockADDR struct to self
void CNetAddr::ToSockAddr(sockaddr_in &saddr) const
{
	saddr.sin_port = port;
	saddr.sin_family  = AF_INET;
	*(int *)&saddr.sin_addr  = *(int *)&ip;
}

void CNetAddr::Print() const
{	
	if(port > 0)
		ComPrintf("%i.%i.%i.%i:%i\n", ip[0],ip[1],ip[2],ip[3], ntohs(port));
	else
		ComPrintf("%i.%i.%i.%i\n", ip[0],ip[1],ip[2],ip[3]);
}

bool CNetAddr::IsValid() const
{	return valid;
}

//======================================================================================
//======================================================================================

namespace VoidNet {

/*
======================================
Common Winsock errors
======================================
*/

void PrintSockError(int err,const char *msg)
{
	char error[128];

	if(!err)
		err=WSAGetLastError();
	
	switch(err)
	{
	case 10013:strcpy(error,"WSAEACCES - error in accessing socket"); break;
	case 10048:strcpy(error,"WSAEADDRINUSE - address is in use"); break;
	case 10049:strcpy(error,"WSAEADDRNOTAVAIL - address is not valid in context"); break;
	case 10047:strcpy(error,"WSAEAFNOSUPPORT - address family not supported by protocol"); break;
	case 10037:strcpy(error,"WSAEALREADY - operation already in progress"); break;
	case 10053:strcpy(error,"WSACONNABORTED - software caused connection aborted"); break;
	case 10061:strcpy(error,"WSAECONNREFUSED - connection refused"); break;
	case 10054:strcpy(error,"WSAECONNRESET - connection reset by peer"); break;
	case 10039:strcpy(error,"WSAEDESTADDRREQ - destination address required"); break;
	case 10014:strcpy(error,"WSAEFAULT - bad address"); break;
	case 10064:strcpy(error,"WSAEHOSTDOWN - host is down"); break;
	case 10065:strcpy(error,"WSAEHOSTUNREACH - no route to host"); break;
	case 10036:strcpy(error,"WSAEINPROGRESS - operation now in progress"); break;
	case 10004:strcpy(error,"WSAEINTR - interrupted function call"); break;
	case 10022:strcpy(error,"WSAEINVAL - invalid argument"); break;
	case 10056:strcpy(error,"WSAEISCONN - socket is already connected"); break;
	case 10024:strcpy(error,"WSAEMFILE - too many open files"); break;
	case 10040:strcpy(error,"WSAEMSGSIZE - message to long"); break;
	case 10050:strcpy(error,"WSAENETDOWN - network is down"); break;
	case 10052:strcpy(error,"WSAENETRESET - network dropped connection on reset"); break;
	case 10051:strcpy(error,"WSAENETUNREACH - network is unreachable"); break;
	case 10055:strcpy(error,"WSAENOBUFS - no buffer space available"); break;
	case 10042:strcpy(error,"WSAENOPROTOOPT - bad protocol option"); break;
	case 10057:strcpy(error,"WSAENOTCONN - socket is not connected"); break;
	case 10038:strcpy(error,"WSAENOTSOCK - socket operation on non-socket"); break;
	case 10045:strcpy(error,"WSAEOPNOTSUPP - operation not supported"); break;
	case 10046:strcpy(error,"WSAEPFNOSUPPORT - protocol family not supported"); break;
	case 10067:strcpy(error,"WSAEPROCLIM - too many processes"); break;
	case 10043:strcpy(error,"WSAEPROTONOSUPPORT - protocol not supported"); break;
	case 10041:strcpy(error,"WSAEPROTOTYPE - protocol wrong type for socket"); break;
	case 10058:strcpy(error,"WSAESHUTDOWN - cannot send after socket shutdown"); break;
	case 10044:strcpy(error,"WSAESOCKTNOSUPPORT - socket type not supported"); break;
	case 10060:strcpy(error,"WSAETIMEDOUT - connection timed out"); break;
	case 10035:strcpy(error,"WSAEWOULDBLOCK - resource temporarily unavailable"); break;
	case 11001:strcpy(error,"WSAHOST_NOT_FOUND - host not found"); break;
	case 10093:strcpy(error,"WSANOTINITIALISED - WSAStartup not yet performed"); break;
	case 11004:strcpy(error,"WSANO_DATA - valid name, no data record of requested type"); break;
	case 11003:strcpy(error,"WSANO_RECOVERY - non-recoverable error"); break;
	case 10091:strcpy(error,"WSASYSNOTREADY - network subsystem is unavailable"); break;
	case 11002:strcpy(error,"WSATRY_AGAIN - non-authoritative host not found"); break;
	case 10092:strcpy(error,"WSAVERNOTSUPPORTED - winsock.dll verison out of range"); break;
	case 10094:strcpy(error,"WSAEDISCON - graceful shutdown in progress"); break;
	default:strcpy(error,"Unknown error occured");break;
	}
	if(msg)
		ComPrintf("%s:%s\n",msg, error);
	else
		ComPrintf("Winsock Error:%s\n",error);
}


/*
==========================================
Initialize/Release Winsock
==========================================
*/
bool InitNetwork()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err; 
	
	wVersionRequested = MAKEWORD( 2, 0 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	
	if (err) 
	{
		ComPrintf("CServer::InitNetwork:Error: WSAStartup Failed\n");
		return false;
	} 
	
	//Confirm Version
	if ( LOBYTE( wsaData.wVersion ) != 2 ||  HIBYTE( wsaData.wVersion ) != 0 ) 
	{
		WSACleanup();
		return false; 
	}  
	return true;
}

void ShutdownNetwork()
{	WSACleanup();
}

}