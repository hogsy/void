#include "Com_file.h"

/*
==========================================
Constructor and Destructor
==========================================
*/
CFile::CFile()
{
	m_filename = 0;
	m_curpos = 0;
	m_size = 0;
	
	m_buffer = 0;
}

CFile::~CFile()
{	
	//None of these conditions should ever be met
	if(m_filename)
		delete [] m_filename;
	if(m_buffer)
		delete [] m_buffer;
}

/*
=====================================
Is the File open ?
=====================================
*/
bool CFile::isOpen()
{
	if(m_buffer)
		return true;
	return false;
}


/*
===========================================
Read func
===========================================
*/
ulong CFile::Read(void *buf,const ulong &size, const ulong &num)
{
	//Otherwise use the buffer
	if(!buf || !size || !num)
	{
		ComPrintf("CFile::Read:Invalid parameters :%s\n",m_filename);
		return 0;
	}
	
	unsigned long bytes_req = size * num;
	if(bytes_req > (m_size - m_curpos))
	{
		ComPrintf("CFile::Read:File doesn't contain requested data from current offset: %s\n",m_filename);
		bytes_req = m_size - m_curpos;
	}

	memcpy(buf,m_buffer+m_curpos,bytes_req);
	m_curpos += (bytes_req);
	return bytes_req;
}
	
/*
===========================================
Get current character
===========================================
*/
int CFile::GetChar()
{
	if((m_curpos +1) >= m_size)
	{
		ComPrintf("CFile::GetByte:EOF reached: %s\n",m_filename);
		return -1; //(EOF)
	}
	return (m_buffer[m_curpos++]);
}

/*
===========================================
Seek to end/start, certain offset
===========================================
*/
bool CFile::Seek(const ulong &offset,  const int   &origin)
{
	switch(origin)
	{
	case SEEK_SET:
		if(offset > m_size)
			return false;
		m_curpos = offset;
		return true;
	case SEEK_CUR:
		if(m_curpos + offset > m_size)
			return false;
		m_curpos += offset;
		return true;
	case SEEK_END:
		if(offset > m_size)
			return false;
		m_curpos = m_size - offset;
		return true;
	}
	return false;
}