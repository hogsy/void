#ifndef VOID_COM_BUFFER
#define VOID_COM_BUFFER

#include "Com_defs.h"

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

		DEFAULT_BUFFER_SIZE = 2900
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
	bool  OverFlowed() const { return m_overFlowed; }
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
	bool	m_overFlowed;

	byte*	GetSpace(int size);
};


#endif