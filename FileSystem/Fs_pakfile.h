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
	uint LoadFile(byte ** ibuffer, 
				  uint buffersize, 
				  const char *ifilename);
	bool HasFile(const char * filename);
	void ListFiles();
	bool GetFileList (CStringList * list);

private:
	
	struct PakEntry_t;

	PakEntry_t **   m_files;
	FILE	   *	m_fp;

	bool BinarySearchForEntry(const char *name,	
							  PakEntry_t ** array, 
							  PakEntry_t ** item,
							  int low, int high);
	void QuickSortFileEntries(PakEntry_t ** list, const int numitems);
};

/*
struct BasicFStreamReader
{
	virtual ~BasicFStreamReader(){}
	virtual bool Open(const char * filename)=0;
	virtual void Close()=0;
	virtual uint Read(void * buf, uint size, uint count)=0;
	virtual int  GetChar()=0;
	virtual int  Seek(const uint offset, int origin)=0;
};
*/

#endif