#ifndef VOID_NETWORK_UTIL
#define VOID_NETWORK_UTIL

#include "Com_defs.h"

namespace VoidNet {

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

		PACKET_HEADER =	8,
		MAX_MSGLEN	  = 1450,
		MAX_DATAGRAM  =	1450,

		DEFAULT_BUFFER_SIZE = 2900,

		MAX_BUFFER_SIZE = 4192
	};

	CNetBuffer(int size);
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
	
	bool  BadRead()	const { return m_badRead; }
	byte* GetData() const { return m_buffer;  }
	int   GetSize() const { return m_curSize; }
	
	int   GetMaxSize()  const { return m_maxSize; }
	int   UnreadBytes() const { return m_curSize - m_readCount; }

	void  Reset();
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

