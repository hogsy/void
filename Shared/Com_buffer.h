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

	CNetBuffer(int size);
	~CNetBuffer();

	CNetBuffer();
	void Create(int size);

	//Writing funcs
	CNetBuffer & operator += (char c);
	CNetBuffer & operator += (byte b);
	CNetBuffer & operator += (short s);
	CNetBuffer & operator += (int i);
	CNetBuffer & operator += (float f);
	CNetBuffer & operator += (const char * string);
	CNetBuffer & operator += (const CNetBuffer & buffer);

	void WriteData(byte * data, int len);
	void WriteAngle(float f);
	void WriteCoord(float f);
		
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
	bool  OverFlowed()  const { return m_overFlowed; }
	int   UnreadBytes() const { return m_curSize - m_readCount; }
	
	void  BeginRead() { m_readCount = 0; }
	void  Reset();
	void  SetSize(int size){ m_curSize = size; }

private:
	enum
	{
		SIZE_CHAR  = 1,
		SIZE_SHORT = 2, 
		SIZE_INT   = 4,
		SIZE_FLOAT = 4,
	};

	byte *	m_buffer;
	int		m_curSize;
	int		m_maxSize;
	int		m_readCount;	//how much have we read

	bool	m_badRead;
	bool	m_overFlowed;

	byte*	GetSpace(int size);
};

#endif