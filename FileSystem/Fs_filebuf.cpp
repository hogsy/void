#include "Fs_hdr.h"
#include "I_file.h"
#include "I_filesystem.h"

extern CFileSystem * g_pFileSystem;

/*
==========================================
Constructor and Destructor
==========================================
*/
CFileBuffer::CFileBuffer(int bufsize)
{
	m_filename = 0;
	m_curpos = 0;
	m_size = 0;
	m_buffer = 0;

	m_buffersize = bufsize;
	if(m_buffersize)
	{	
		m_buffer = (byte*)g_pHunkManager->HunkAlloc(m_buffersize);
	}
}

CFileBuffer::~CFileBuffer()
{	
	if(m_filename)
		delete [] m_filename;
	if(m_buffer)
	{
		g_pHunkManager->HunkFree(m_buffer);
	}

}

/*
=====================================
Is the File open ?
=====================================
*/
bool CFileBuffer::isOpen() const
{	
	if(m_size)
		return true;
	return false;
}

/*
=====================================
Open the requested file
=====================================
*/
bool CFileBuffer::Open(const char * ifilename)
{
	if(isOpen())
		Close();

	int size = g_pFileSystem->LoadFileData(&m_buffer,m_buffersize,ifilename);
	
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
void CFileBuffer::Close()
{
	//Don't release the buffer is using it statically
	if(m_buffer)
	{
		g_pHunkManager->HunkFree(m_buffer);
		m_buffer = 0;
	}

	if(m_filename)
	{
		delete [] m_filename;
		m_filename = 0;
	}
	m_curpos = 0;
	m_size = 0;	
}

/*
===========================================
Read items of given size of given count
into given buffer
===========================================
*/
uint CFileBuffer::Read(void *buf,uint size, uint count)
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
		bytes_req -= (bytes_req % size);
	}

	memcpy(buf,m_buffer+m_curpos,bytes_req);
	m_curpos += (bytes_req);
	return (bytes_req/size);
}
	
/*
===========================================
Get current character
===========================================
*/
int CFileBuffer::GetChar()
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
bool CFileBuffer::Seek(int offset, int origin)
{
	if(!m_size)
		return false;
	switch(origin)
	{
	case SEEK_SET:
		if(offset > m_size)
			offset = m_size;
		m_curpos = offset;
		break;
	case SEEK_CUR:
		if(m_curpos + offset > m_size)
			offset = m_size;
		m_curpos += offset;
		break;
	case SEEK_END:
		//Offset should be negative for this
		if(offset)
			offset = 0;
		m_curpos = m_size + offset;
		break;
	}
	return true;
}

//======================================================================================
//Access funcs
//======================================================================================

uint	CFileBuffer::GetPos()  const  { return m_curpos; }
uint	CFileBuffer::GetSize() const  { return m_size;   }
byte *	CFileBuffer::GetData() const  {	return m_buffer; }
const char * CFileBuffer::GetFileName() const {	return m_filename; }
