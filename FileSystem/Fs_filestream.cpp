#include "Fs_hdr.h"
#include "I_file.h"
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
		m_archive->CloseFile(m_filehandle);
		m_filehandle = -1;
		m_archive = 0;
	}
}


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
Get Files size
==========================================
*/
uint CFileStream::GetSize() const 
{ return m_size; 
}

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

	m_size = g_pFileSystem->OpenFileStream(&m_fp, 
										   m_filehandle, &m_archive,
										   ifilename);
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
	if(!size || !count)
		return 0;

	if(!buf)
	{
		ComPrintf("CFileStream::Read: Invalid parameters :%s\n",m_filename);
		return 0;
	}
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
int CFileStream::GetChar()
{
	if(m_fp)
		return ::fgetc(m_fp);
	if(m_archive)
		return m_archive->GetChar(m_filehandle);
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
	if(m_archive)
		return m_archive->Seek(offset,origin,m_filehandle);
	return false;
}