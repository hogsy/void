#ifndef FS_PAKREADER_H
#define FS_PAKREADER_H

#include "Fs_hdr.h"

/*
================================================
Pak File
================================================
*/

class CPakFile:public CArchive
{
public:
	CPakFile();
	~CPakFile();

	//CArchive Implementation
	bool Init(const char * archivepath, const char * basepath);

	HFS  OpenFile(const char *ifilename); 
	void CloseFile(HFS handle);
	uint Read(void * buf, uint size, uint count, HFS handle);
	int  GetChar(HFS handle);
	bool Seek(int offset, int origin, HFS handle);
	uint GetPos(HFS handle);
	uint GetSize(HFS handle);
	uint LoadFile(byte ** ibuffer,  const char *ifilename);
	
	bool FindFile(char * buf, int buflen,const char * filename);
	int  GetFileList (StrList &strlst, const char * path, const char *ext);
	void ListFiles();

private:

	enum 
	{	PAKENTRYSIZE =	64
	};

	struct PakHeader_t;
	
	struct PakEntry_t
	{
		PakEntry_t() { filepos = filelen = 0; }
		char filename[56];
		long filepos, 
			 filelen;
	};

	struct PakOpenFile_t
	{
		PakOpenFile_t() { file = 0; curpos = 0; };
		~PakOpenFile_t(){ file = 0;}

		PakEntry_t * file;
		long	   curpos;
	};

	PakEntry_t **   m_files;
	FILE	   *	m_fp;

	PakOpenFile_t   m_openFiles[CArchive::ARCHIVEMAXOPENFILES];
	int				m_numOpenFiles;

	bool BinarySearchForEntry(const char *name,	
							  PakEntry_t ** item,
							  int low, int high, 
							  int nameoffset= -1);
};

#endif