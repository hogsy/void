#include "Fs_hdr.h"
#include "Fs_readers.h"
#include "Fs_filesys.h"

extern CFileSystem * g_pFileSystem;

/*
==========================================
Constructor and Destructor
==========================================
*/

CFileStream::CFileStream()
{
	m_filename = 0;
	m_size = 0;

	m_fp =0;
	m_archive = 0;
	m_filehandle = -1;
}

CFileStream::~CFileStream()
{	
	if(m_filename)
		delete [] m_filename;
	if(m_fp)
		::fclose(m_fp);

	if(m_archive && m_filehandle >= 0)
	{
		m_archive->CloseFile(m_filehandle);
		m_filehandle = -1;
		m_archive = 0;
	}
}

/*
==========================================
Get current position
==========================================
*/
uint CFileStream::GetPos() const  
{ 
	if(m_fp)	
		return ::ftell(m_fp);
	else if(m_archive)
		return m_archive->GetPos(m_filehandle);
	return 0;
}

/*
==========================================
Access funcs
==========================================
*/
uint CFileStream::GetSize() const  { return m_size;  }
const char * CFileStream::GetFileName() const {	return m_filename; }

/*
=====================================
Is the File open ?
=====================================
*/
bool CFileStream::isOpen() const
{	
	if(m_size && (m_fp || (m_filehandle >= 0 && m_archive)))
		return true;
	return false;
}

/*
=====================================
Open the requested file
=====================================
*/
bool CFileStream::Open(const char * ifilename)
{
	if(isOpen())
		Close();

	m_size = g_pFileSystem->OpenFileReader(this,ifilename);

	if(m_size)
	{
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
void CFileStream::Close()
{
	if(m_filename)
	{
		delete [] m_filename;
		m_filename = 0;
	}
	m_size = 0;	

	if(m_fp)
	{
		::fclose(m_fp);
		m_fp =0;
	}
	if(m_archive)
	{
		m_archive->CloseFile(m_filehandle);
		m_filehandle = -1;
		m_archive = 0;
	}
}

/*
===========================================
Read items of given size of given count
into given buffer
===========================================
*/
uint CFileStream::Read(void *buf,uint size, uint count)
{
	if(!buf)
	{
		ComPrintf("CFileReader::Read: Invalid parameters :%s\n",m_filename);
		return 0;
	}

	// don't really want to read anything
	if(!size || !count)
		return 0;

	if(m_fp)
		return ::fread(buf,size,count,m_fp);
	if(m_archive)
		return m_archive->Read(buf,size,count,m_filehandle);
	return 0;
}
	
/*
===========================================
Get current character
===========================================
*/
int  CFileStream::GetChar()
{
	if(m_fp)
		return ::fgetc(m_fp);
	if(m_archive)
		return m_archive->GetChar(m_filehandle);
	return 0;
}

	
/*
===========================================
Get current character
===========================================
*/
void CFileStream::GetToken(char *buff, bool newline)
{
	char tmp;

	// if we want a new line, find the first '\n'
	if (newline)
		while (1)
		{
			tmp = GetChar();
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
		tmp = GetChar();

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

		tmp = GetChar();
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
bool CFileStream::Seek(int offset, int origin)
{
	if(m_fp)
		return !(::fseek(m_fp,offset,origin));
	if(m_archive)
		return m_archive->Seek(offset,origin,m_filehandle);
	return false;
}