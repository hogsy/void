#include "I_filesystem.h"

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
	}
	m_archive = 0;
	m_filehandle = -1;
}


uint CFileStream::GetPos() const  
{ 
	if(m_fp)	
		return ::ftell(m_fp);
	return 0;
}

uint CFileStream::GetSize() const 
{ return m_size; 
}

/*
=====================================
Is the File open ?
=====================================
*/
bool CFileStream::isOpen()
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

	if(g_pFileSystem->OpenFileStream(this,ifilename))
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
		::fclose(m_fp);
}

/*
===========================================
Read items of given size of given count
into given buffer
===========================================
*/
ulong CFileStream::Read(void *buf,uint size, uint count)
{
	if(!buf || !size || !count)
	{
		ComPrintf("CFileStream::Read: Invalid parameters :%s\n",m_filename);
		return 0;
	}

	if(m_fp)
		return ::fread(buf,size,count, m_fp);
	return 0;
}
	
/*
===========================================
Get current character
===========================================
*/
int CFileStream::GetChar()
{
	if(m_fp)
		return ::fgetc(m_fp);
	return 0;
}

/*
===========================================
Seek to end/start, certain offset
===========================================
*/
bool CFileStream::Seek(uint offset, int origin)
{
	if(m_fp)
		return !(::fseek(m_fp,offset,origin));
	return false;
}