#include "Com_buffer.h"

/*
==========================================
Constructor/Destructor
==========================================
*/
CNetBuffer::CNetBuffer(int size)
{
	if(size > DEFAULT_BUFFER_SIZE)
		size = DEFAULT_BUFFER_SIZE;

	m_buffer = new byte[size];
	m_maxSize = size;
	m_curSize = 0;

	m_readCount = 0;

	m_badRead = false;
	m_overFlowed = false;
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
		m_overFlowed = true;
	}
	byte * data = m_buffer + m_curSize;
	m_curSize += size;
	return data;
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
	int len = strlen(s) + 1;
	byte * buf = GetSpace(len);
	memcpy(buf,s,len);
	buf[len-1] = 0;
}

/*
==========================================
Reading funcs
==========================================
*/

char  CNetBuffer::ReadChar()
{
	if(m_readCount + SIZE_CHAR > m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	return ((signed char)m_buffer[m_readCount++]);
}

byte  CNetBuffer::ReadByte()
{ 
	if(m_readCount + SIZE_CHAR > m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	return (m_buffer[m_readCount++]);
}


short CNetBuffer::ReadShort()
{
	if(m_readCount + SIZE_SHORT > m_curSize)
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
	if(m_readCount + SIZE_INT > m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	int i = (int)(m_buffer[m_readCount]	+ 
				 (m_buffer[m_readCount+1]<<8) +
				 (m_buffer[m_readCount+2]<<16) +
				 (m_buffer[m_readCount+3]<<24));
	m_readCount +=SIZE_INT;
	return i;

}

float CNetBuffer::ReadFloat()
{
	if(m_readCount + SIZE_FLOAT > m_curSize)
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
