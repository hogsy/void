#ifndef VOID_COM_PAK_FILE
#define VOID_COM_PAK_FILE

#include "Fs_hdr.h"

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
	
	struct PakEntry_t;
	struct PakOpenFile_t;

	PakEntry_t **   m_files;
	FILE	   *	m_fp;

	PakOpenFile_t * m_openFiles[ARCHIVEMAXOPENFILES];
	int				m_numOpenFiles;

	bool BinarySearchForEntry(const char *name,	
							  PakEntry_t ** array, 
							  PakEntry_t ** item,
							  int low, int high);
	void QuickSortFileEntries(PakEntry_t ** list, const int numitems);
};

#endif