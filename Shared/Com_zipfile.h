#ifndef _QUAKE_BSP_DEF_
#define _QUAKE_BSP_DEF_

#include "Com_filesys.h"


class CZipFile:public CArchive
{
public:
	CZipFile();
	~CZipFile();

	//CArchive Implementation
	bool  Init(const char * base, const char * archive);
	long  OpenFile(const char* path, byte ** buffer);
	void  ListFiles();
	bool  GetFileList (CStringList * list);

private:
	FILE * m_fp;
};

#endif