#ifndef COM_ZIPREADER_H
#define COM_ZIPREADER_H

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
	uint Read(void * buf, uint size, uint count, HFS handle);
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
	ZipOpenFile_t	m_openFiles[ARCHIVEMAXOPENFILES];

	int m_numOpenFiles;
	int m_numFiles;

	FILE * m_fp;

	ulong GetLastRecordOffset(FILE *fin);
	bool  BuildZipEntriesList(FILE * fp, int numfiles);
	bool  BinarySearchForEntry(const char *name,	
							  ZipEntry_t ** array, 
							  ZipEntry_t ** item,
							  int low, int high);

};

#endif