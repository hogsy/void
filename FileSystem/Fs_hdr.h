#ifndef FILESYSTEM_HEADER
#define FILESYSTEM_HEADER


#include "Com_defs.h"
#include "Com_list.h"

#pragma warning(disable : 4018)     // signed/unsigned mismatch

/*
===========================================
Abstract Base class defining interface
for other Archive handling classes
===========================================
*/
class CArchive
{
public:
	//Load a listing of files in the archive, and order them
	virtual bool Init(const char * archivepath, const char * basepath)=0;

	//Open file at this path, fill buffer
	virtual uint LoadFile(byte ** ibuffer, uint &buffersize, 
							bool staticbuffer, const char *ifilename)=0;

	//Print file listing
	virtual void ListFiles()=0;

	//Return list of files
	virtual bool GetFileList (CStringList * list) = 0;

	virtual	~CArchive() { }

	char	m_archiveName[COM_MAXPATH];
	int		m_numFiles;
};


#endif


