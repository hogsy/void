#ifndef VOID_COM_PAK_FILE
#define VOID_COM_PAK_FILE

#include "Com_fs.h"

class CPakFile:public CArchive
{
public:
	CPakFile();
	~CPakFile();

	//CArchive Implementation
	bool Init(const char * archivepath, const char * basepath);
	long OpenFile(const char* filename, byte ** buffer);
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

#endif