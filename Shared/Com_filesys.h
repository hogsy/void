#ifndef VOID_COM_FILEMANAGER
#define VOID_COM_FILEMANAGER

#include "Com_file.h"
#include "Com_list.h"

#define COM_MAXOPENFILES	32	///Max files open at a time

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
	virtual bool Init(const char * base, const char * archive)=0;

	//Open file at this path, fill buffer
	virtual long OpenFile(const char* path, byte ** buffer)=0;

	//Print file listing
	virtual void ListFiles()=0;

	//Return list of files
	virtual bool GetFileList (CStringList * list) = 0;

	virtual	~CArchive() { }

	char	archivename[COM_MAXPATH];
	int		numfiles;
};


/*
==========================================
The File Manager
-client code request CFiles from this
-transparently loads archived files, or standard files
-maintains the Searchpaths with precedence
-loads supported archive files for quicker access
-keeps a list of currently open files
==========================================
*/

class CSearchPath;		//Predeclartion for member variables

class CFileManager
{
public:
	CFileManager();
	~CFileManager();

	//Intialize the File system using this dir as the root
	bool Init(const char *exedir, const char * basedir);				
	
	//Add another directory which will override similar
	//contents in the root directory
	bool AddGameDir(const char *dir);
	
	//Change the Game dir
	bool ChangeGameDir(const char *dir);

	//Open a file and returns a pointer to it
	//The buffer pointer will be set to point to the file buffer
	CFile * Open(const char *filename);
	
	//Loads a file into the given buffer, returns size
	ulong   Load(const char *filename, byte ** buffer);	

	//Close the given file
	bool Close(CFile * file);

	//List Current Search Paths
	void ListSearchPaths();

	//Print out the list of files in added archives
	void ListArchiveFiles();

	//Returns a filelisting matching the criteria
	int FindFiles(CStringList *filelist,		//File List to fill
				  const char  *ext,				//Extension		  
				  const char  *path=0);			//Search in a specific path
private:


	//Member variables
	//================
	int			m_totalfiles;					//total number of Files in the archives
	char		m_exepath[COM_MAXPATH];			//Exe path
	char		m_basedir[COM_MAXPATH];			//Base Dir, IS NOT changed

	int			m_numopenfiles;					//Number of openfiles
	CFile *		m_openfiles[COM_MAXOPENFILES];	//Array of pointers to open files

	int			m_numsearchpaths;				//Number of searchpaths
	CSearchPath * m_searchpaths;				//DoublyLinked list of searchpaths
	CSearchPath * m_lastpath;					//Latest path


	//Member Functions
	//=================
	void AddSearchPath(const char * path, CArchive * file=0);
	void RemoveSearchPath(const char *path);

	//Scans directory for supported archives, returns list
	bool GetArchivesInPath(const char *path, CStringList * list);

	//FIXME, MOVE THIS SOMEPLACE MORE APPROPRIATE
	static bool CompareExts(const char *file, const char *ext);		//Compare file Extensions
};


extern CFileManager * g_pFileManager;

#endif