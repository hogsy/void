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

#define ARCHIVEMAXOPENFILES 8;

class CArchive
{
public:

	//Load a listing of files in the archive, and order them
	virtual bool Init(const char * archivepath, const char * basepath)=0;

	//Open file at this path, fill buffer
	virtual uint LoadFile(byte ** ibuffer, 
						  uint buffersize, 
						  const char *ifilename)=0;

	//Check for presence of given file
	virtual bool HasFile(const char * filename)=0;

	//Print file listing
	virtual void ListFiles()=0;

	//Return list of files
	virtual bool GetFileList (CStringList * list) = 0;

	virtual	~CArchive() { }

	char	m_archiveName[COM_MAXPATH];
	int		m_numFiles;
};


#endif


