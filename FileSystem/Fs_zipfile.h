#ifndef FS_ZIPREADER_H
#define FS_ZIPREADER_H

#include "Fs_hdr.h"

/*
================================================
ZipFile 
================================================
*/
class CZipFile:public CArchive
{
public:
	CZipFile();
	~CZipFile();

	//CArchive Implementation
	bool Init(const char * archivepath, const char * basepath);

	HFS  OpenFile(const char *ifilename); 
	void CloseFile(HFS handle);
	uint Read(void * buf, uint size, uint count, HFS handle);
	int  GetChar(HFS handle);
	bool Seek(int offset, int origin, HFS handle);
	uint GetPos(HFS handle);
	uint GetSize(HFS handle);
	uint LoadFile(byte ** ibuffer, const char *ifilename);
	
	bool FindFile(char * buf, int buflen,const char * filename);
	int  GetFileList (StrList &strlst,  const char * path, const char *ext);
	void ListFiles();

private:

	struct ZipEntry_t
	{
		ZipEntry_t() { filepos = filelen = 0; }

		char  filename[COM_MAXFILENAME];
		ulong filepos, 
			  filelen;
	};

	struct ZipOpenFile_t
	{
		ZipOpenFile_t() { file= 0; curpos = 0;}
		~ZipOpenFile_t(){ file= 0; }

		ZipEntry_t * file;
		ulong		 curpos;
	};

	ZipEntry_t	 ** m_files;
	ZipOpenFile_t	m_openFiles[CArchive::ARCHIVEMAXOPENFILES];

	int m_numOpenFiles;
	int m_numFiles;

	FILE * m_fp;

	void ZipReadShort(ushort &is);
	void ZipReadLong(ulong &il);


	ulong GetLastRecordOffset();
	bool  BuildZipEntriesList(int numfiles);
	bool  BinarySearchForEntry(const char *name,	
							  ZipEntry_t ** item,
							  int low, int high,
							  int nameoffset = -1);
};

#endif