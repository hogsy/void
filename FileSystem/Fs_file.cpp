#include "I_filesystem.h"

extern CFileSystem * g_pFileSystem;

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
	m_buffersize =0;
	m_staticbuffer = false;
}

CFileReader::~CFileReader()
{	
	if(m_filename)
		delete [] m_filename;
	if(m_buffer)
		free(m_buffer);
}

/*
=====================================
Is the File open ?
=====================================
*/
bool CFileReader::isOpen()
{	
	if(m_filename)
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
	if(isOpen())
		Close();

	int size = g_pFileSystem->LoadFile(&m_buffer,m_buffersize,m_staticbuffer,ifilename);
	
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
	//Don't release the buffer is using it statically
	if((!m_staticbuffer) && (m_buffer))
	{
		free(m_buffer);
		m_buffer = 0;
	}

	if(m_filename)
	{
		delete [] m_filename;
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
ulong CFileReader::Read(void *buf,const uint &size, const uint &count)
{
	if(!buf || !size || !count)
	{
		ComPrintf("CFileReader::Read: Invalid parameters :%s\n",m_filename);
		return 0;
	}
	
	uint bytes_req = size * count;
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
	if((m_curpos) >= m_size)
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
bool CFileReader::Seek(const uint &offset, EFilePos origin)
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


uint CFileReader::GetFileSize(const char * filename)
{
	return 0;
}

bool CFileReader::LoadFile(void *buf, const uint &bufsize, const char * filename)
{
	return false;
}


/*
===========================================
Lock a static buffer for file I/O which is not
released/realloc on each file open/close call.

the static buffer is only reallocated if it
is smaller than the requred size
===========================================
*/
void CFileReader::LockStaticBuffer(const uint &size)
{
	m_staticbuffer = true;
	m_buffer = (byte*)MALLOC(size);
	m_buffersize = size;
}

void CFileReader::ReleaseStaticBuffer()
{
	free(m_buffer);
	m_buffer = 0;
	m_staticbuffer = false;
	m_buffersize = 0;
}
