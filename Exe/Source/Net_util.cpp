#include "Net_util.h"

char	g_computerName[256];
char	g_ipaddr[16];


/*
======================================
Tries to make sure that a string is a valid ip address
======================================
*/

bool ValidIP(char *ip)
{
	int i = strlen(ip);
	
	if(!i || i<7)
		return false;						//too short
	
	int j,k,dot,num;
	char digit[4];
	
	memset(digit,0,4);
	num=dot=j=k=0;

	while(*ip || k<i)
	{
		if(*ip == '.')
		{
			dot++;
			j=0;
			if(!sscanf(digit,"%d",&num))
				return false;				//not a digit
			if(num<0 || num > 255)	
				return false;				//bad range
			memset(digit,0,4);
		}
		else
		{
			if(j>3)
				return false;				//more than 3 digits after a dot
			digit[j] = *ip;
			j++;
		}
		ip++;
		k++;
	}
	if(dot!=3)
		return false;						//not 3 dots
	return true;
}


/*
======================================
Common Winsock errors
======================================
*/

void PrintSockError(int err)
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
	ComPrintf(":%s\n",error);
}

/*
=====================================
Initialize Winsock
=====================================
*/

bool InitWinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err; 
	
	wVersionRequested = MAKEWORD( 2, 0 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	
	if (err) 
	{
		ComPrintf("InitWinsock:Error: WSAStartup Failed\n");
		return false;
	} 
	
	//Confirm Version
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
         HIBYTE( wsaData.wVersion ) != 0 ) 
	{
		/* Tell the user that we couldn't find a usable 
		   WinSock DLL.                                 */    
		WSACleanup();
		return false; 
	}  
	// The WinSock DLL is acceptable. Proceed. 

	//init listener sock
	struct hostent *hp;
	unsigned long ulNameLength = sizeof(g_computerName);

	if(!GetComputerName(g_computerName, &ulNameLength))
	{
		ComPrintf("InitWinsock:Error: couldnt find computer name\n");
		return false;
	}
	hp = gethostbyname(g_computerName);							
  
	if (hp == NULL)	
	{						
		ComPrintf("InitWinsock:ERROR:Couldnt find hostname\n");
		return false;
	}

	IN_ADDR HostAddr;
	memcpy(&HostAddr,hp->h_addr_list[0],sizeof(HostAddr));
	sprintf(g_ipaddr, "%s", inet_ntoa(HostAddr));
	return true;
}


//============================================================================
//============================================================================

char	CNBuffer::tstring[2048];

/*
======================================
CNBuffer - Constructor
======================================
*/

CNBuffer :: CNBuffer(int size):CBaseNBuffer(size)
{	
/*	if (size < 256)
		size = 256;
	data = (unsigned char*) malloc(size);
	maxsize = size;
	cursize = 0;
*/
	readcount =0;
	overflowed = false;
	allowoverflow = false;
	badread = false;
}

/*
======================================
CNBuffer - Destructor
======================================
*/
CNBuffer::~CNBuffer()
{
//	free(data);
}

/*
======================================
GetSpace
======================================
*/

void * CNBuffer::GetSpace (int len)
{
	void	*ndata;
	
	if (cursize + len > maxsize)
	{
		if (!allowoverflow)
			ComPrintf("CNBuffer::GetSpace overflow without permission\n");
		
		if (len > maxsize)
			ComPrintf("CNBuffer::GetSpace %i is larger than max_size\n",len);
			
		overflowed = true;
		Clear (); 
	}

	ndata = data + cursize;
	cursize += len;
	return ndata;
}



int CNBuffer::LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}


//============================================================================
// Writing Funcs
//============================================================================


/*
======================================
WriteChar
======================================
*/

void CNBuffer::WriteChar (int i)
{
	unsigned char	*buf;
	
#ifdef PARANOID
	if (i < -128 || i > 127)
		ComPrintf("WriteChar: range error");
#endif

	buf = (unsigned char*)GetSpace (1);
	buf[0] = i;
}


/*
======================================
Write Byte
======================================
*/

void CNBuffer::WriteByte (int i)
{
	unsigned char	*buf;
	
#ifdef PARANOID
	if (i < 0 || i > 255)
		ComPrintf("WriteByte: range error");
#endif

	buf = (unsigned char*)GetSpace (1);
	buf[0] = i;
}


/*
=======================================
Write Short
=======================================
*/

void CNBuffer::WriteShort(int c)
{
	byte	*buf;
	
#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		ComPrintf("WriteShort: range error");
#endif

	buf = (unsigned char*)GetSpace (2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}


/*
=======================================
Write Long
=======================================
*/
void CNBuffer::WriteLong (int c)
{
	byte	*buf;

	buf = (unsigned char*)GetSpace (4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}


/*
=======================================
WriteFloat
=======================================
*/

void CNBuffer::WriteFloat (float f)
{
	union
	{
		float	f;
		int	l;
	} dat;
	
	dat.f = f;
	dat.l = LongSwap (dat.l);

	Write (&dat.l, 4);
}

/*
=======================================
Write String
=======================================
*/
void CNBuffer::WriteString (const char *s)
{
	if(!s)
		memcpy(GetSpace(1),"",1);
	else
	{
		int len = strlen(s) + 1;
		memcpy(GetSpace(len),s,len);
	}
/*	if (!s)
		Write ("", 1);
	else
		Write (s, strlen(s)+1);
*/
}


/*
=======================================
Write co-ords,
quantized by multiplying by 8 on writing
and dividing by 8 on receive
=======================================
*/
void CNBuffer::WriteCoord (float f)
{
	WriteShort ((int)(f*8));
}


/*
=======================================
Write angles
=======================================
*/

void CNBuffer::WriteAngle ( float f)
{
	WriteByte (((int)f*256/360) & 255);
}


//===========================================================================

void  CNBuffer::Write (void *data, int length)
{
	memcpy(GetSpace(length),data,length);
}




//============================================================================
// Reading Funcs
//============================================================================

/*
======================================
Read Char from the buffer
======================================
*/
int CNBuffer::ReadChar (void)
{
	int	c;
	
	if (readcount+1 > cursize)
	{
		badread = true;
		return -1;
	}
		
	c = (signed char)data[readcount];
	readcount++;
	return c;
}

/*
======================================
Read a Byte from the buffer
======================================
*/
int CNBuffer::ReadByte (void)
{
	int	c;
	
	if (readcount+1 > cursize)
	{
		badread = true;
		return -1;
	}
		
	c = (unsigned char)data[readcount];
	readcount++;
	return c;
}

/*
=======================================
Read a Short from the buffer 
=======================================
*/

int CNBuffer::ReadShort (void)
{
	int	c;
	
	if (readcount+2 > cursize)
	{
		badread = true;
		return -1;
	}
		
	c = (short)( data[readcount] + (data[readcount+1]<<8) );
	
	readcount += 2;
	return c;
}


/*
=======================================
Read a long from the buffer
=======================================
*/
int CNBuffer::ReadLong (void)
{
	int	c;
	
	if (readcount+4 > cursize)
	{
		badread = true;
		return -1;
	}
		
	c = data[readcount]
		+ (data[readcount+1]<<8)
		+ (data[readcount+2]<<16)
		+ (data[readcount+3]<<24);
		
	readcount += 4;
	
	return c;
}


/*
=======================================
Read a float from the buffer
=======================================
*/

float CNBuffer::ReadFloat (void)
{
	union
	{
		byte	b[4];
		float	f;
		int		l;
	} dat;
	
	dat.b[0] =	data[readcount];
	dat.b[1] =	data[readcount+1];
	dat.b[2] =	data[readcount+2];
	dat.b[3] =	data[readcount+3];
	readcount += 4;
	
	dat.l = LongSwap(dat.l);

	return dat.f;	
}


/*
=======================================
Read a string from the buffer
=======================================
*/

char *CNBuffer::ReadString (void)
{
	int		l,c;

	memset(tstring,0,sizeof(char)*2048);
	
	l = 0;
	do
	{
		c = ReadChar ();
		if (c == -1 || c == 0)
			break;
		tstring[l] = c;
		l++;
	} while (l < sizeof(tstring)-1);
	
	tstring[l] = 0;
	readcount +=l;
	return tstring;
}

/*
=======================================
Read a string from the buffer
=======================================
*/

char *CNBuffer::ReadString(char delim)
{
	int		l,c,p;

	memset(tstring,0,sizeof(char)*2048);
	
	l = 0;
	p = 0;
	do
	{
		c = ReadChar ();
		
		if ((c == -1) || (c == 0) || 
			(l>0 && c == delim))		//get rid of leading delims
			break;

		if(c != delim)
		{
			tstring[p] = c;
			p++;
		}
		l++;
	} while (p < sizeof(tstring)-1);
	
	tstring[p] = 0;
	readcount +=1;
	return tstring;
}


/*
=======================================
Read co-ords
=======================================
*/

float CNBuffer::ReadCoord (void)
{
	return ReadShort() * (1.0f/8);
}


/*
=======================================
read Angles
=======================================
*/
float CNBuffer::ReadAngle (void)
{
	return ReadChar() * (360.0f/256);
}


//============================================================================

void CNBuffer::Reset()
{
	if(!cursize)
		return;

	readcount =0; 
	cursize =0; 
	memset(data,0,maxsize);
	overflowed = false;
	allowoverflow = false;
	badread = false;
}



//=====================================================================================
//=====================================================================================


/*
=====================================

=====================================
*/

CBaseNBuffer :: CBaseNBuffer(int size)
{	
	if (size < 512)
		size = 512;
	data = (unsigned char*) malloc(size);
//	data = new unsigned char[size];
	maxsize = size;
	memset(data,0,size);
	cursize = 0;
}


CBaseNBuffer :: ~CBaseNBuffer()
{
	free(data);
//	delete [] data;
}


void CBaseNBuffer::Reset()
{
	if(!cursize)
		return;

	cursize =0; 
	memset(data,0,maxsize);
}

