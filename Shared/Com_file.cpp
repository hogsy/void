#include "Com_defs.h"
#include "Com_file.h"

/*
================================================
Constructor/Destructor
================================================
*/
VFile::VFile() : m_bOpen(false), 
				 m_hFile(INVALID_HANDLE_VALUE)
{
	memset(m_szPath,0,COM_MAXPATH);
}

VFile::~VFile()
{
	if(m_bOpen)
		Close();
}

/*
================================================
Open
================================================
*/
bool VFile::Open(const char* szPath, int openMode, EOpenType openType, int shareMode)
{
	if(m_bOpen)
		Close();
	
	DWORD nAccess = 0;
	DWORD nShare =0;
	DWORD nCreate =0;

	if(openMode == OPEN_READ)
		nAccess = GENERIC_READ;
	else if(openMode == OPEN_WRITE)
		nAccess = GENERIC_WRITE;
	else if(openMode == OPEN_READ_WRITE)
		nAccess = (GENERIC_WRITE|GENERIC_READ);
	else
		return false;

	if(shareMode & SHARE_READ)
		nShare  |= FILE_SHARE_READ;
	if(shareMode & SHARE_WRITE)
		nShare  |= FILE_SHARE_WRITE;

	switch (openType)
	{
	  case E_CREATE_NEW :
		 nCreate = CREATE_NEW;
		 break;
	  case E_CREATE_ALWAYS :
		 nCreate = CREATE_ALWAYS;
		 break;
	  case E_OPEN_EXISTING :
		 nCreate = OPEN_EXISTING;
		 break;
	  case E_OPEN_ALWAYS :
		 nCreate = OPEN_ALWAYS;
		 break;
	  case E_TRUNCATE_EXISTING:
		 nCreate = TRUNCATE_EXISTING;
		 break;
	  default:
		 return false;
	}

	m_hFile = ::CreateFile(szPath,nAccess,nShare,
							NULL,nCreate,FILE_ATTRIBUTE_NORMAL,NULL);
	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		m_bOpen = true;
		strcpy(m_szPath,szPath);
		return true;
	}
	return false;	
}


/*
================================================
Close
================================================
*/

void VFile::Close()
{
   if(m_bOpen)
   {
      m_bOpen = false;
      ::CloseHandle(m_hFile);
      m_hFile = INVALID_HANDLE_VALUE;
	  memset(m_szPath,0,COM_MAXPATH);
   }
}


/*
================================================
Misc Util
================================================
*/

int VFile::GetLength() const
{
	if(m_bOpen)
		return ::GetFileSize(m_hFile, NULL);
	return false;
}

const char * VFile::GetFilePath() const
{	return m_szPath;
}

bool VFile::IsOpen() const
{	return m_bOpen;
}

/*
================================================
File Pointer handling
================================================
*/

int VFile::GetPosition() const
{  
	if(m_bOpen)
      return ::SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
   return 0;
}

bool VFile::Seek(int offset, int origin)
{
	if(!m_bOpen)
		return false;

	DWORD nMethod=0;

	switch(origin)
	{
	case SEEK_SET:
		nMethod = FILE_BEGIN;
		break;
	case SEEK_CUR:
		nMethod = FILE_CURRENT;
		break;
	case SEEK_END:
		nMethod = FILE_END;
		break;
	default:
		return false;
		break;
	}

	if(::SetFilePointer(m_hFile, offset, NULL, nMethod) == 0xFFFFFFFF)
	   return false;

	return true;
}

bool VFile::SeekToStart()
{   return Seek(0, SEEK_SET);
}

bool VFile::SeekToEnd()
{   return Seek(0, SEEK_END);
}


/*
================================================
Read funcs
================================================
*/
ulong VFile::Read(void* buffer, ulong numBytes)
{
	assert(m_bOpen);

	DWORD bytesRead = 0;
	if(::ReadFile(m_hFile, buffer, numBytes, &bytesRead, NULL) == FALSE)
		return 0;
	return bytesRead;
}

int VFile::ReadChar()
{
	assert(m_bOpen);

	char buf[1];
	buf[0] = 0;
	DWORD bytesRead = 0;	
	if((::ReadFile(m_hFile, (void*)buf, 1, &bytesRead, NULL) == FALSE) ||  (bytesRead == 0))
		return EOF;
	bytesRead;
	return buf[0];
}

/*
================================================
Write funcs
================================================
*/
ulong VFile::Write( void* buffer, ulong numBytes)
{
	assert(m_bOpen);

	DWORD bytesWritten = 0;
	if(::WriteFile(m_hFile,buffer,numBytes,&bytesWritten,0) == FALSE)
		return 0;
	return bytesWritten;
}

bool VFile::Flush()
{   return (::FlushFileBuffers(m_hFile) != 0);
}



//=====================================================================================
//=====================================================================================


/*
================================================
Basic File management
================================================
*/
bool VFile::FileExists(const char * szPath)
{
   SECURITY_ATTRIBUTES securityAttrib; 

   memset(&securityAttrib,0,sizeof(SECURITY_ATTRIBUTES));
   securityAttrib.nLength  = sizeof(SECURITY_ATTRIBUTES);
   securityAttrib.lpSecurityDescriptor = NULL;
   securityAttrib.bInheritHandle       = FALSE;

   HANDLE hFile = CreateFile(szPath,
                       GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       &securityAttrib,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);
   if(hFile != INVALID_HANDLE_VALUE)
   {
       CloseHandle(hFile);
       return true;
   }
   return false;
}

bool VFile::FileDelete(const char * szPath)
{	return (::DeleteFile(szPath) != 0);
}

bool VFile::FileRename(const char* szOldPath, const char* szNewPath)
{	return (::MoveFileEx(szOldPath, szNewPath, MOVEFILE_COPY_ALLOWED) != 0);
}

bool VFile::FileCopy(const char* szSourcePath, const char* szDestPath, bool bOverwrite)
{	return (::CopyFile(szSourcePath,szDestPath, !bOverwrite) != 0);
}



/*
================================================
Other file related Utility funcs
================================================
*/

char * VFile::ParseExtension(char *ext,  int bufsize, const char *filename)
{
	char fileext[16];
	::_splitpath(filename,0,0,0,fileext);
	if(!fileext[0])
		return 0;
	if(bufsize < strlen(fileext))
		return 0;
	strcpy(ext,fileext);
	return ext;
}

char * VFile::ParseFilePath (char *path, int pathlen, const char *filename)
{
	char szDriveName[8];
	char szPath[_MAX_PATH];

	::_splitpath(filename,szDriveName,szPath,0,0);
	if(!szDriveName[0] || !szPath[0])
		return 0;

	if((strlen(szDriveName) + strlen(szPath)) < pathlen)
		return 0;
	::sprintf(path,"%s%s",szDriveName,szPath);
	return path;
}

char * VFile::ParseFileName (char *name, int namelen, const char *path)
{
	char fileName[128];
	::_splitpath(path,0,0,fileName,0);
	if(!fileName[0])
		return 0;
	if(namelen < strlen(fileName))
		return 0;
	strcpy(name,fileName);
	return name;
}

char * VFile::ForceExtension(char *szPath, const char *extension)
{
	char fileext[16];
	
	if(ParseExtension(fileext,16,szPath))
	{
		//has a different extension
		if(_stricmp(fileext,extension)==0)
			return szPath;
		RemoveExtension(szPath);
	}
	
	strcat(szPath,extension);
	return szPath;
}


void VFile::RemoveExtension(char * filename)
{
	//find extension first
	char * p = filename;
	while(*p && *p != '.')
		p++;
	
	//if found extension, end it here
	if(*p)
		*p = '\0';
}

bool VFile::CompareExtension(const char *szPath, const char *ext)
{
	char fileext[16];
	if(!ParseExtension(fileext,16,szPath))
		return false;
	return (_stricmp(fileext,ext) == 0);
}



void VFile::ConfirmDir(const char * szPath)
{
	//try creating each dir - nothing will change if it already exists
	char path[COM_MAXPATH];
	const char *pDir = szPath;
	
	while (*pDir)
	{
		if (*pDir == '\\')
		{
			memset(path,0,COM_MAXPATH);
			strncpy(path,pDir,pDir-szPath);
			::CreateDirectory(path, NULL);
		}
		pDir++;
	}
}

