#ifndef COM_ZIPREADER_H
#define COM_ZIPREADER_H

#define ZLIB_DLL
#include "Infozip/unzip.h"

#include "Fs_hdr.h"


class CZipFile:public CArchive
{
public:
	CZipFile();
	~CZipFile();

	//CArchive Implementation
	bool Init(const char * archivepath, const char * basepath);
	
	uint LoadFile(byte ** ibuffer, 
				  uint buffersize, 
				  const char *ifilename);
	
	bool HasFile(const char * filename);
	
	void ListFiles();
	
	bool GetFileList (CStringList * list);

private:

	unzFile m_hFile;
};

#endif