#include "Net_util.h"
#include "Com_defs.h"

using namespace VoidNet;

//======================================================================================
//Network Address class
//======================================================================================

CNetAddr::CNetAddr()
{
	ip[0] = ip[1] = ip[2] = ip[3] =0;
	port = 0;
	valid = true;
}

//Assignment operators
CNetAddr & CNetAddr::operator = (const SOCKADDR_IN &saddr)
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
	char		stringaddr[128];
	SOCKADDR_IN sockAddr;
	
	strcpy (stringaddr,szaddr);

	//Strip port number if specified
	for (char * colon = stringaddr ; *colon ; colon++)
	{
		if (*colon == ':')
		{
			*colon = 0;
			sockAddr.sin_port = htons((short)atoi(colon+1));	
		}
	}
	
	//If an ip address was given then just convert to inetaddr
	if (stringaddr[0] >= '0' && stringaddr[0] <= '9')
	{
		*(int *)&sockAddr.sin_addr = inet_addr(stringaddr);
	}
	//Resolve hostname
	else
	{
		HOSTENT	*host = 0;
		if ((host = gethostbyname(stringaddr)) == 0)
		{
			valid = false;
			return (*this);
		}
		*(int *)&sockAddr.sin_addr = *(int *)host->h_addr_list[0];
	}

	//Convert to SOCKADDR_IN
	*(int *)&ip = *(int *)&sockAddr.sin_addr;
	port = sockAddr.sin_port;
	valid = true;
	return (*this);
}

//Utility
const char * CNetAddr::ToString()
{
	static char stringaddr[64];
	
	if(port > 0)
		sprintf(stringaddr,"%d.%d.%d.%d:%d\n", ip[0],ip[1],ip[2],ip[3], port);
	else
		sprintf(stringaddr,"%d.%d.%d.%d\n", ip[0],ip[1],ip[2],ip[3]);
	return stringaddr;
}

//Set the sockADDR struct to self
void CNetAddr::ToSockAddr(SOCKADDR_IN &saddr)
{
	saddr.sin_port = port;
	saddr.sin_family  = AF_INET;
	*(int *)&saddr.sin_addr  = *(int *)&ip;
}

//Util
void CNetAddr::Print() const
{	
	if(port > 0)
		ComPrintf("%d.%d.%d.%d:%d\n", ip[0],ip[1],ip[2],ip[3], port);
	else
		ComPrintf("%d.%d.%d.%d\n", ip[0],ip[1],ip[2],ip[3]);
}

bool CNetAddr::IsValid() const
{	return valid;
}

//Equality check
/*bool operator == (const CNetAddr &laddr, const CNetAddr &raddr)
{
	if((laddr.ip[0] == raddr.ip[0]) &&
	   (laddr.ip[1] == raddr.ip[1]) &&
	   (laddr.ip[2] == raddr.ip[2]) &&
	   (laddr.ip[3] == raddr.ip[3]) &&
	   (laddr.port  == raddr.port))
	   return true;
	return false;
}*/

//======================================================================================
//omrafi@hotmail.com
//======================================================================================

/*
==========================================
Constructor/Destructor
==========================================
*/
CNetBuffer::CNetBuffer(int size)
{
	if(size > MAX_BUFFER_SIZE)
		size = MAX_BUFFER_SIZE;

	m_buffer = new byte[size];
	m_maxSize = 0;
	m_curSize = 0;

	m_readCount = 0;

	m_badRead = false;
}

CNetBuffer::~CNetBuffer()
{
	if(m_buffer)
		delete [] m_buffer;
}

void  CNetBuffer::Reset()
{
	m_curSize = 0;
	m_readCount = 0;
}

byte* CNetBuffer::GetSpace(int size)
{
	if(m_curSize + size >= m_maxSize)
	{
		ComPrintf("CNetBuffer:: Buffer overflowed\n");
		m_curSize = 0;
	}
	byte * data = m_buffer + m_curSize;
	return data;
}

byte * CNetBuffer::GetWritePointer() const
{	return(m_buffer + m_curSize);
}

/*
==========================================
Writing funcs
==========================================
*/
void CNetBuffer::WriteChar(char c)
{
	char * buf = (char *)GetSpace(SIZE_CHAR);
	buf[0] = (char)c;
}

void CNetBuffer::WriteByte(byte b)
{
	byte  * buf = GetSpace(SIZE_CHAR);
	buf[0] = b;
}

void CNetBuffer::WriteShort(short s)
{
	byte * buf = GetSpace(SIZE_SHORT);
	buf[0] = s & 0xff;	//gives the lower byte
	buf[1] = s >> 8;	//shift right to get the high byte
}

void CNetBuffer::WriteInt(int i)
{
	byte * buf = GetSpace(SIZE_INT);
	buf[0] = i & 0xff;			
	buf[1] = (i >> 8)  & 0xff;	
	buf[2] = (i >> 16) & 0xff;	
	buf[3] = i >> 24;
}

void CNetBuffer::WriteFloat(float f)
{
	union
	{
		float f;
		int	  l;
	}floatdata;

	floatdata.f = f;
	byte * buf = GetSpace(SIZE_INT);
	memcpy(buf,&floatdata.l,SIZE_INT);
}

void CNetBuffer::WriteAngle(float f)
{	
	WriteByte((int)(f*256/360) & 255);
}
void CNetBuffer::WriteCoord(float f)
{	
	WriteShort((int)(f*8));
}

void CNetBuffer::WriteString(const char *s)
{
	int len = strlen(s);
	byte * buf = GetSpace(len);
	memcpy(buf,s,len);
}

/*
==========================================
Reading funcs
==========================================
*/

char  CNetBuffer::ReadChar()
{
	if(m_readCount + SIZE_CHAR >= m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	return ((signed char)m_buffer[m_readCount++]);
}

byte  CNetBuffer::ReadByte()
{ 
	if(m_readCount + SIZE_CHAR >= m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	return (m_buffer[m_readCount++]);
}


short CNetBuffer::ReadShort()
{
	if(m_readCount + SIZE_SHORT >= m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	short s = (short)(m_buffer[m_readCount]	+ (m_buffer[m_readCount+1]<<8));
	m_readCount +=SIZE_SHORT;
	return s;
}

int CNetBuffer::ReadInt()
{
	if(m_readCount + SIZE_INT >= m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	int i = (int)(m_buffer[m_readCount]	+ 
				 (m_buffer[m_readCount+1]<<8) +
				 (m_buffer[m_readCount+2]<<16) +
				 (m_buffer[m_readCount+3]<<32));
	m_readCount +=SIZE_INT;
	return i;

}

float CNetBuffer::ReadFloat()
{
	if(m_readCount + SIZE_FLOAT >= m_curSize)
	{
		m_badRead = true;
		return -1;
	}

	union
	{
		byte  b[4];
		float f;
	}fb;

	fb.b[0] = m_buffer[m_readCount];
	fb.b[1] = m_buffer[m_readCount+1];
	fb.b[2] = m_buffer[m_readCount+2];
	fb.b[3] = m_buffer[m_readCount+3];
	m_readCount += SIZE_FLOAT;
	return fb.f;
}

float CNetBuffer::ReadAngle()
{
	return ReadChar() * (360.0f/256);
}
float CNetBuffer::ReadCoord()
{
	return ReadShort() * 1.0f / 8;
	
}
char* CNetBuffer::ReadString(char delim)
{
	static char string[2048];
	char c=0;
	int len = 0;

	do
	{
		c = ReadChar();
		if(c == 0 || c == -1 || c == delim)
			break;
		string[len] = c;
		len++;
	}while(len < 2048);
	
	string[len] = 0;
	return string;
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