#ifndef VOID_COM_BUFFER
#define VOID_COM_BUFFER

#include "Com_defs.h"
//#include "3dmath.h"

/*
==========================================
Network buffer utility class
==========================================
*/
class CBuffer
{
public:

	//Default size goes to 1450
	CBuffer(int size=1450);
	~CBuffer();

	//Writing funcs
	void Write(char c);
	void Write(byte b);
	void Write(short s);
	void Write(int i);
	void Write(float f);
	void Write(const char * string);
	void Write(const CBuffer &buffer);

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

//	void  ReadVector(vector_t &vec);

	//Other util
	const byte* GetData() const { return m_buffer;  }
	bool  BadRead()	const { return m_badRead; }
	int   GetSize() const { return m_curSize; }
	int   GetMaxSize()  const { return m_maxSize; }
	bool  OverFlowed()  const { return m_overFlowed; }
	int   UnreadBytes() const { return m_curSize - m_readCount; }
	bool  HasSpace(int space) const { return (m_maxSize - m_curSize >= space); }
	
	void  BeginRead() { m_badRead = false; m_readCount = 0; }
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