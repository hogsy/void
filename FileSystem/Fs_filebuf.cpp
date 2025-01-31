#include "Fs_hdr.h"
#include "Fs_readers.h"
#include "Fs_filesys.h"

extern CFileSystem * g_pFileSystem;

/*
==========================================
Constructor and Destructor
==========================================
*/
CFileBuffer::CFileBuffer()
{
	m_filename = 0;
	m_curpos = 0;
	m_size = 0;
	m_buffer = 0;
	m_buffersize = 0;
}

CFileBuffer::~CFileBuffer()
{	Close();
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

	m_size = g_pFileSystem->OpenFileReader(this,ifilename);
	
	//File opened successfully
	if(m_size)
	{
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
	if(m_buffer)
	{
		g_pHunkManager->HunkFree(m_buffer);
		m_buffersize = 0;
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
	if(!buf)
	{
		ComPrintf("CFileReader::Read: Invalid parameters :%s\n",m_filename);
		return 0;
	}

	// don't really want to read anything
	if(!size || !count)
		return 0;


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
int CFileBuffer::GetChar(bool perror)
{
	if((m_curpos) >= m_size)
	{
		if (perror)
			ComPrintf("CFileReader::GetByte:EOF reached: %s\n",m_filename);
		return -1;
	}
	return (m_buffer[m_curpos++]);
}

	
/*
===========================================
Get current character
===========================================
*/
void CFileBuffer::GetToken(char *buff, bool newline)
{
	if(!m_size)
		return;

	char tmp;

	// if we want a new line, find the first '\n'
	if (newline)
		while (1)
		{
			tmp = GetChar(false);
			if (tmp == '\n')
				break;

			if ((tmp == EOF) || (tmp == -1))
			{
				(*buff) = '\0';
				return;
			}
		}


	// advance until we find characters
	while (1)
	{
		tmp = GetChar(false);

		// dont want a new line but we found one
		if ((tmp == '\n' && !newline) || (tmp == EOF) || (tmp == -1))
		{
			(*buff) = '\0';
			return;
		}

		if (tmp > ' ')
			break;
	}

	// copy over the token we are at
	char *ptr = buff;
	while (1)
	{
		if ((tmp <= ' ') || (tmp == EOF) || (tmp == -1))
			break;

		(*ptr) = tmp;
		ptr++;

		tmp = GetChar(false);
	}

	// null terminate
	(*ptr) = '\0';


	// if it's a comment, we don't want it
	if ((buff[0] == '/') && (buff[1] == '/'))
	{
		if (!newline)
			buff[0] = '\0';

		else
			GetToken(buff, true);
	}
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
		if(offset > 0)
			return false;
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
byte * CFileBuffer::GetBuffer() const  {	return m_buffer; }
const char * CFileBuffer::GetFileName() const {	return m_filename; }
