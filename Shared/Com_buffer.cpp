#include "Com_buffer.h"

/*
==========================================
Constructor/Destructor
==========================================
*/
CBuffer::CBuffer(int size)
{
	m_buffer = new byte[size];
	m_maxSize = size;
	m_curSize = 0;

	m_readCount = 0;

	m_badRead = false;
	m_overFlowed = false;
}


CBuffer::~CBuffer()
{
	if(m_buffer)
		delete [] m_buffer;
}

void  CBuffer::Reset()
{
	m_badRead = false;
	m_overFlowed = false;
	m_curSize = 0;
	m_readCount = 0;
}

byte* CBuffer::GetSpace(int size)
{
	if(m_curSize + size >= m_maxSize)
	{
//		ComPrintf("CBuffer:: Buffer overflowed\n");
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
void CBuffer::Write(char c)
{ 
	char * buf = (char *)GetSpace(SIZE_CHAR);
	buf[0] = (char)c;	
}

void CBuffer::Write(byte b) 
{ 
	byte  * buf = GetSpace(SIZE_CHAR);
	buf[0] = b;	
}

void CBuffer::Write(short s)
{ 
	byte * buf = GetSpace(SIZE_SHORT);
	buf[0] = s & 0xff;	//gives the lower byte
	buf[1] = s >> 8;	//shift right to get the high byte	
}
void CBuffer::Write(int i)
{ 
	byte * buf = GetSpace(SIZE_INT);
	buf[0] = i & 0xff;			
	buf[1] = (i >> 8)  & 0xff;	
	buf[2] = (i >> 16) & 0xff;	
	buf[3] = i >> 24;	
} 
void CBuffer::Write(float f) 
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

void CBuffer::Write(const char * string) 
{ 
	int len = strlen(string) + 1;
	byte * buf = GetSpace(len);
	memcpy(buf,string,len);
	buf[len-1] = 0;	
}

void CBuffer::Write(const CBuffer & buffer)
{
	byte * buf = GetSpace(buffer.GetSize());
	memcpy(buf,buffer.GetData(),buffer.GetSize());
}

void CBuffer::WriteAngle(float f)
{	Write((int)(f*256/360) & 255);
}
void CBuffer::WriteCoord(float f)
{	Write((int)(f*8));
}

void CBuffer::WriteData(byte * data, int len)
{
	byte * buf = GetSpace(len);
	memcpy(buf,data,len);
}

/*
==========================================
Reading funcs
==========================================
*/
char  CBuffer::ReadChar()
{
	if(m_readCount + SIZE_CHAR > m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	return ((signed char)m_buffer[m_readCount++]);
}

byte  CBuffer::ReadByte()
{ 
	if(m_readCount + SIZE_CHAR > m_curSize)
	{
		m_badRead = true;
		return -1;
	}
	return (m_buffer[m_readCount++]);
}


short CBuffer::ReadShort()
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

int CBuffer::ReadInt()
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

float CBuffer::ReadFloat()
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

float CBuffer::ReadAngle()
{	return ReadChar() * (360.0f/256);
}
float CBuffer::ReadCoord()
{	return ReadShort() * 1.0f / 8;
	
}

char* CBuffer::ReadString(char delim)
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


void CBuffer::ReadVector(vector_t &vec)
{
	vec.x = vec.y = vec.z = 0.0f;

	int  comp = 0, i=0;
	char coord[12];
	memset(coord,0,12);
	m_badRead = true;

	do
	{
		coord[i] = ReadChar();
		if(coord[i] == 0 || coord[i] == -1)
			break;

		if(coord[i] == ' ')
		{
			if(comp == 0)
				vec.x = atof(coord);
			else if(comp == 1)
				vec.y = atof(coord);
			memset(coord,0,12);
			i = 0;
			comp++;
		}

	}while(comp < 3);

	//read the final comp
	if(comp == 2)
	{
		m_badRead = false;
		vec.z = atof(coord);
	}
}