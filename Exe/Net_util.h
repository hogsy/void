#ifndef VOID_NETWORK_HDR
#define VOID_NETWORK_HDR

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
	const char * ToString();
	void ToSockAddr(SOCKADDR_IN &saddr);
	
	//Util
	void Print()   const;
	bool IsValid() const;

private:
	int		ip[4];
	short	port;
	bool	valid;
};

//======================================================================================
//======================================================================================

/*
==========================================
Network buffer utility class
==========================================
*/
class CNetBuffer
{
public:

	enum
	{
		SIZE_CHAR  = 1,
		SIZE_SHORT = 2, 
		SIZE_INT   = 4,
		SIZE_FLOAT = 4,
		DEFAULT_BUFFER_SIZE = 4096,
		MAX_BUFFER_SIZE = 8192
	};

	CNetBuffer(int size= DEFAULT_BUFFER_SIZE);
	~CNetBuffer();

	//Writing funcs
	void WriteChar(char c);
	void WriteByte(byte b);
	void WriteShort(short s);
	void WriteInt(int i);
	void WriteFloat(float f);
	void WriteAngle(float f);
	void WriteCoord(float f);
	void WriteString(const char *s);
	
	//Reading funcs
	char  ReadChar();
	byte  ReadByte();
	short ReadShort();
	int   ReadInt();
	float ReadFloat();
	float ReadAngle();
	float ReadCoord();
	char* ReadString(char delim=0);

	//Other util
	int   UnreadBytes() const { return m_curSize - m_readCount; }
	int   FreeBytes()   const { return m_maxSize - m_curSize;   }
	bool  BadRead()		const { return m_badRead; }
	int   MaxSize()		const { return m_maxSize; }
	void  Reset();
	
	//To enable the socket to write to the buffer directly
	byte* GetWritePointer() const;
	void  SetCurSize(int size){ m_curSize = size; }

private:
	
	byte *	m_buffer;
	int		m_curSize;
	int		m_maxSize;
	int		m_readCount;	//how much have we read

	bool	m_badRead;

	byte*	GetSpace(int size);
};

}


#endif

