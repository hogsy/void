#ifndef VOID_COM_BUFFER
#define VOID_COM_BUFFER

#include "Com_defs.h"


/*
==========================================
Network buffer utility class
==========================================
*/
class CBuffer
{
public:
	
	enum
	{
		DEFAULT_BUFFER_SIZE = 1450
	};

	//Default size goes to 1450
	explicit CBuffer(int size=DEFAULT_BUFFER_SIZE);
	~CBuffer();

	//Writing funcs
	void WriteChar(char c);
	void WriteByte(byte b);
	void WriteShort(short s);
	void WriteInt(int i);
	void WriteFloat(float f);
	void WriteString(const char * string);
	void WriteBuffer(const CBuffer &buffer);
	void WriteAngle(float f);
	void WriteCoord(float f);
	void WriteData(byte * data, int len);
		
	//Reading funcs
	char  ReadChar();
	byte  ReadByte();
	short ReadShort();
	int   ReadInt();
	float ReadFloat();
	float ReadAngle();
	float ReadCoord();
	char* ReadString(char delim=0);
	void  ReadString(char * buf, int bufsize, char delim = 0);

	//Other util
	byte* GetData() const { return m_buffer;  }
	void  SetSize(int size){ m_curSize = size; }

	void  BeginRead() { m_badRead = false; m_readCount = 0; }
	void  Reset();
	
	bool  BadRead()	const { return m_badRead; }
	bool  OverFlowed()  const { return m_overFlowed; }
	
	int   GetSize() const { return m_curSize; }
	int   GetMaxSize()  const { return m_maxSize; }
	int   GetUnreadBytes() const { return m_curSize - m_readCount; }
	
	bool  HasSpace(int space) const { return (m_maxSize - m_curSize >= space); }
	
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