#ifndef VOID_COM_PAK_FILE
#define VOID_COM_PAK_FILE

#include "Com_filesys.h"


class CPakFile:public CArchive
{
public:
	CPakFile();
	~CPakFile();

	//CArchive Implementation
	bool  Init(const char * base, const char * archive);
	long  OpenFile(const char* path, byte ** buffer);
	long  OpenFile(const char* path, FILE * fp);
	void  ListFiles();
	bool  GetFileList (CStringList * list);

private:
	
	class CPakEntry
	{
	public:
		CPakEntry() { filepos = filelen = 0; };
		char filename[56];
		long filepos, filelen;
	};

	CPakEntry ** files;
	FILE	   * m_fp;

	bool BinarySearchForEntry(const char *name,	
							  CPakEntry ** array, 
							  CPakEntry ** item,
							  int low, int high);

	void QuickSortFileEntries(CPtrList<CPakEntry> * list, const int numitems);
	void QuickSortFileEntries(CPakEntry ** list, const int numitems);
};

#endif