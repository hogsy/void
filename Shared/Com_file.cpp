#include "Com_fs.h"

CFileSystem * CFileReader::m_pFileSystem=0;

/*
==========================================
Constructor and Destructor
==========================================
*/
CFileReader::CFileReader()
{
	m_filename = 0;
	m_curpos = 0;
	m_size = 0;
	m_buffer = 0;
}

CFileReader::~CFileReader()
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
bool CFileReader::isOpen()
{	
	if(m_buffer)
		return true;
	return false;
}

/*
=====================================
Open the requested file
=====================================
*/
bool CFileReader::Open(const char * ifilename)
{
	int size = m_pFileSystem->LoadFile(&m_buffer,ifilename);
	//File opened successfully
	if(size)
	{
		m_size = size;
		m_curpos = 0;
		m_filename = new char[strlen(ifilename)+1];
		strcpy(m_filename, ifilename);
		return true;
	}
	return false;
}

/*
=====================================
Close file
=====================================
*/
bool CFileReader::Close()
{
	if(m_buffer)
	{
		delete [] m_buffer;
		m_buffer = 0;
	}
	if(m_filename)
	{
		delete m_filename;
		m_filename = 0;
	}
	m_curpos = 0;
	m_size = 0;	
	return true;
}

/*
===========================================
Read items of given size of given count
into given buffer
===========================================
*/
ulong CFileReader::Read(void *buf,const ulong &size, const ulong &count)
{
	if(!buf || !size || !count)
	{
		ComPrintf("CFileReader::Read: Invalid parameters :%s\n",m_filename);
		return 0;
	}
	
	unsigned long bytes_req = size * count;
	if(m_curpos + bytes_req > m_size)
	{
		ComPrintf("CFileReader::Read: File doesn't contain requested data from current offset: %s\n",m_filename);
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
int CFileReader::GetChar()
{
	if((m_curpos +1) >= m_size)
	{
		ComPrintf("CFileReader::GetByte:EOF reached: %s\n",m_filename);
		return -1;
	}
	return (m_buffer[m_curpos++]);
}

/*
===========================================
Seek to end/start, certain offset
===========================================
*/
bool CFileReader::Seek(const ulong &offset, EFilePos origin)
{
	switch(origin)
	{
	case EFILE_START:
		if(offset > m_size)
			return false;
		m_curpos = offset;
		return true;
	case EFILE_CUR:
		if(m_curpos + offset > m_size)
			return false;
		m_curpos += offset;
		return true;
	case EFILE_END:
		if(offset > m_size)
			return false;
		m_curpos = m_size - offset;
		return true;
	}
	return false;
}



int CFileReader::LoadFile(void **buf, const char * ifilename)
{	return m_pFileSystem->LoadFile((byte**)buf,ifilename);
}