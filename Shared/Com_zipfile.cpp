#include "Com_zipfile.h"

/*
=======================================================================
CZipFile Implementation
=======================================================================
*/


CZipFile::CZipFile()
{
	m_hFile = 0;
}


CZipFile::~CZipFile()
{
    if (m_hFile)
		unzClose(m_hFile);
	m_hFile = 0;
}


//CArchive Implementation

bool CZipFile::Init(const char * archivepath, const char * basepath)
{
	//Open File
	char filepath[COM_MAXPATH];
	sprintf(filepath,"%s/%s",basepath,archivepath);

	m_hFile = unzOpen(filepath);
	if(!m_hFile)
	{
		ComPrintf("CZipFile::Init: Unable to open %s\n", filepath);
		return false;
	}

	unz_global_info unzGlobal;
	if(!unzGetGlobalInfo(m_hFile,&unzGlobal) == UNZ_OK)
	{
		ComPrintf("CZipFile::Init: Unable to read %s\n", filepath);
		return false;
	}
	m_numFiles = unzGlobal.number_entry;
	strcpy(m_archiveName,archivepath);
	ComPrintf("%s, Added %d entries\n",archivepath, unzGlobal.number_entry);
	return true;
}

void  CZipFile::ListFiles()
{
    int ret=0;
	char filename[64];

    ret = unzGoToFirstFile(m_hFile);
    while (ret == UNZ_OK)
    {
		unzGetCurrentFileInfo(m_hFile, NULL, filename, 64, NULL, 0, NULL, 0);
		ret = unzGoToNextFile(m_hFile);
		ComPrintf("%s\n",filename);
    }
}

bool  CZipFile::GetFileList (CStringList * list)
{
	if(list)
	{
		ComPrintf("CZipFile::GetFileList: List needs to be deleted!, %s\n", m_archiveName);
		return false;
	}
	
	if(!m_hFile)
	{
		ComPrintf("CZipFile::GetFileList: No zipfile opened\n");
		return false;
	}

	char filename[64];
	int ret=0;
	list = new CStringList();
	CStringList  *iterator = list;

	ret = unzGoToFirstFile(m_hFile);
    while (ret == UNZ_OK)
    {
		unzGetCurrentFileInfo(m_hFile, NULL, filename, 64, NULL, 0, NULL, 0);
		ret = unzGoToNextFile(m_hFile);

		strcpy(iterator->string,filename);
		
		iterator->next = new CStringList;
		iterator = iterator->next;
    }
	return true;
}


long  CZipFile::OpenFile(const char* filename, byte ** buffer)
{
	if (unzLocateFile(m_hFile,filename, 2) != UNZ_OK)
	{	return 0;
	}

	if (unzOpenCurrentFile(m_hFile) != UNZ_OK)
	{
		ComPrintf("CZipFile::OpenFile: Unable to open %s , %s\n", filename, m_archiveName);
	    return 0;
	}

	unz_file_info fileinfo;
	long size = 0;

    if (unzGetCurrentFileInfo(m_hFile, &fileinfo, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
	{
		ComPrintf("CZipFile::OpenFile: Unable to get file info %s , %s\n", filename, m_archiveName);
		return 0;
	}
	
	if(fileinfo.uncompressed_size != fileinfo.compressed_size)
	{
		ComPrintf("CZipFile::OpenFile: File is compressed %s , %s\n", filename, m_archiveName);
		return 0;
	}

	size =  fileinfo.uncompressed_size;

	*buffer = new byte[size];
	int readbytes = unzReadCurrentFile(m_hFile, *buffer,size);

	if(readbytes != size)
		ComPrintf("CZipFile::OpenFile: Warning, only read %d of %d. %s , %s\n", 
		readbytes, size,filename, m_archiveName);
		
	unzCloseCurrentFile(m_hFile);
	return size;
}


#if 0

int pak_open(const char *path)
{
   if (unzLocateFile(pakfile, path, 2) != UNZ_OK)
       return 0;
   if (unzOpenCurrentFile(pakfile) != UNZ_OK)
       return 0;
   return 1;
}

void pak_close(void)
{
    unzCloseCurrentFile(pakfile);
}

unsigned int pak_read(void *buf, unsigned int size, unsigned int nmemb)
{
    unsigned int len;

    len = unzReadCurrentFile(pakfile, buf, size * nmemb);
    return len / size;
}

unsigned int pak_getlen(void)
{
    unz_file_info info;

    if (unzGetCurrentFileInfo(pakfile, &info, NULL, 0, NULL, 0, NULL, 0)
	!= UNZ_OK)
	return 0;
    return info.uncompressed_size;
}

unsigned int pak_readfile(const char *path, unsigned int bufsize, unsigned char *buf)
{
    unsigned int len;
    
    if (!pak_open(path))
	return 0;

    if ((len = pak_getlen()) != 0)
    {
	if (pak_read((void*)buf, 1, len) != len)
	    len = 0;
    }
    pak_close();    
    return len;
}


int pak_checkfile(const char *path)
{
    int status;

    status = unzLocateFile(pakfile, path, 2);
    return (status == UNZ_OK);
}

#endif