#include "Fs_zipfile.h"

/*
==========================================
Constructor/Destructor
==========================================
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


/*
==========================================
Initialize the zip file
Open it, get info
==========================================
*/
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

/*
==========================================
List all the files in the zip
==========================================
*/
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


/*
==========================================
Get a list of files
==========================================
*/
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


bool CZipFile::HasFile(const char * filename)
{
	if (unzLocateFile(m_hFile,filename, 2) == UNZ_OK)
		return true;
	return false;
}


/*
==========================================
Load a file in the zip to the given buffer
==========================================
*/
uint CZipFile::LoadFile(byte ** ibuffer, 
						uint buffersize, 
						const char *ifilename)
{
	if (unzLocateFile(m_hFile,ifilename, 2) != UNZ_OK)
		return 0;
	
	if (unzOpenCurrentFile(m_hFile) != UNZ_OK)
	{
		ComPrintf("CZipFile::OpenFile: Unable to open %s , %s\n", ifilename, m_archiveName);
	    return 0;
	}

	unz_file_info fileinfo;
    if (unzGetCurrentFileInfo(m_hFile, &fileinfo, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
	{
		ComPrintf("CZipFile::OpenFile: Unable to get file info %s , %s\n", ifilename, m_archiveName);
		return 0;
	}
	
	if(fileinfo.uncompressed_size != fileinfo.compressed_size)
	{
		ComPrintf("CZipFile::OpenFile: File is compressed %s , %s\n", ifilename, m_archiveName);
		return 0;
	}

	uint size =  fileinfo.uncompressed_size;
	if(!buffersize)
	{
		*ibuffer = (byte*)MALLOC(size);
	}
	else
	{
		if(size > buffersize)
		{
			ComPrintf("CZipFile::LoadFile: Buffer is smaller than size of file %s, %d>%d\n", 
					ifilename, size, buffersize);
			return 0;
		}
	}

	int readbytes = unzReadCurrentFile(m_hFile, *ibuffer,size);
	if(readbytes != size)
		ComPrintf("CZipFile::OpenFile: Warning, only read %d of %d. %s , %s\n", 
		readbytes, size,ifilename, m_archiveName);

	unzCloseCurrentFile(m_hFile);
	return size;
}

