#ifndef FILESYSTEM_HEADER
#define FILESYSTEM_HEADER

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4786)		// bleh, template names expand to more than what vc can handle

#include "Com_defs.h"
#include "Com_mem.h"
#include "I_hunkmem.h"
#include "Com_util.h"
#include "Com_file.h"

//======================================================================================
//======================================================================================

typedef int HFS;

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

	//File Stream Reading funcs
	virtual HFS  OpenFile(const char *ifilename) =0;
	virtual void CloseFile(HFS handle) =0;
	virtual uint Read(void * buf, uint size, uint count, HFS handle) =0;
	virtual int  GetChar(HFS handle) =0;
	virtual bool Seek(int offset, int origin, HFS handle) =0;
	virtual uint GetPos(HFS handle) = 0;
	virtual uint GetSize(HFS handle) = 0;

	//Open file at this path, alloc and fill buffer
	virtual uint LoadFile(byte ** ibuffer, const char *ifilename)=0;

	//Check for presence of given file
	virtual bool FindFile(char * buf, int buflen, const char * filename)=0;

	//Print file listing
	virtual void ListFiles()=0;

	//append any files matching the criteria to the given list
	virtual int  GetFileList (StrList &list, 
							  const char * path,
							  const char *ext)=0;

	virtual	~CArchive() { }

	char	m_archiveName[COM_MAXPATH];
	int		m_numFiles;

protected:

	enum
	{	ARCHIVEMAXOPENFILES = 32
	};
};

#endif