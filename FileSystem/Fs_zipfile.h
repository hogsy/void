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

	HFS OpenFile(const char *ifilename); 
	void CloseFile(HFS handle);
	ulong Read(void * buf, uint size, uint count, HFS handle);
	int  GetChar(HFS handle);
	bool Seek(uint offset, int origin, HFS handle);
	uint GetPos(HFS handle);
	uint GetSize(HFS handle);
	
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