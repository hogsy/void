#ifndef COM_ZIPREADER_H
#define COM_ZIPREADER_H

#define ZLIB_DLL
#include "Infozip/Iz_unzip.h"

#include "Com_fs.h"


class CZipFile:public CArchive
{
public:
	CZipFile();
	~CZipFile();

	//CArchive Implementation
	bool Init(const char * archivepath, const char * basepath);
	long OpenFile(const char* filename, byte ** buffer);
	void ListFiles();
	bool GetFileList (CStringList * list);

private:

	unzFile m_hFile;
};

#endif