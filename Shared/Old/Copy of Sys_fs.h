#ifndef _SYS_FILESYSTEM_
#define _SYS_FILESYSTEM_

#include "Com_file.h"


#define FS_MAXARCHIVES	64			//Max archives included
#define FS_MAXOPENFILES	128			//Max files open at a time
#define FS_NUMHASHES	128

class CSearchPath;

/*
===========================================
StringList, 
-client manually allocs strings
-string are deleted on destruction of list
===========================================
*/
class CStringList
{
public:
	CStringList()  {  next = 0; } 
	~CStringList() { if(next) delete next; next =0; }

	char		string[COM_MAXPATH];
	CStringList *next;

	friend void QuickSortStringList(CStringList * const list,const int numitems);
};



/*
==========================================
The File Manager
-builds the directory structure
-parses archive files and includes them properly
-keeps a list of included zip file names
-keeps a list of currently open files
-keeps an unzip interface open when its requested for an archived file

Client should be UNAWARE whether the requested file is archived
or a regular file.
==========================================
*/
class CFileManager
{
public:
	CFileManager();
	~CFileManager();

	//Intialize the File system using this dir as the root
	bool Init(const char *exedir, const char * gamedir);				
	
	//Add another directory which will override similar
	//contents in the root directory
	bool AddGameDir(const char *dir);
	
	//Reinits the game dir
	bool ChangeGameDir(const char *dir);

	//Print out the list of files and subdirectories
	void ListFiles();

	//Open a file and returns a pointer to it
	//The buffer pointer will be set to point to the file buffer
	CFile * Open(const char *filename);
	CFile * Open(CFile * file);

	//Close the given file
	bool Close(CFile * file);

	//Returns a filelisting matching the criteria	
	int FindFiles(CStringList *filelist,		//File List to fill
				  const char  *ext,				//Extension		  
				  const char  *path=0);			//Search in a specific path
private:

	class CFileList
	{
	public:
		CFileList()  { file = 0; next = 0; }
		~CFileList() { if(file) delete file; 
					   if(next) delete next;    next =0;
					}

		CFile * file;
		CFileList * next;
	};

	//Member variables
	//================
	int			m_totalfiles;				//total number of files in the system
	char		m_exepath[COM_MAXPATH];		//Exe path

	CFileList   m_filetable[FS_NUMHASHES];

	int			m_numopenfiles;
	CFile *		m_openfiles[FS_MAXOPENFILES];

	int			m_numsearchpaths;
	CSearchPath * m_searchpaths;
	CSearchPath * m_lastpath;


	//Member Functions
	//=================
	bool AddSearchPath(const char * path, const int pathlen);
	bool RemoveSearchPath(const char *path);

	//Recursively loads and orders all the files and directory in the given dir
	bool BuildDirectoryList(const char *basepath,	//Current search path
							const char *dirname);	//Proposed name of the directory
							
		//Creates a list of file and directories in the specified path
	bool Win32_DirectoryList(const char *path,
							 int & numdirs,
							 CStringList *dirlist,		
							 int & numfiles,
							 CStringList *filelist);
/*


	//recursive search through a file in the specified folder.
	CFile * GetFile(const char *filename, CDirectory * dir);


	//Opens the requested archived file using the registered archive handler
	CFile * LoadArchivedFile(CFile * file);



	//Finds the matching directory to the file
	CDirectory * GetDirFromPath(const char * path, 
								const int &pathlen,
								CDirectory *dir);

	//Binary search for file with in a directory
	CFile * BinarySearchForFile(const char *name,	
								CFile * array, 
								const int &offset,
								int low, int high);

	void QuickSortStringList(CStringList * const list,const int numitems);

	//Searches through internal directory list of all the files
	//that match the given patten
	CFileList * BuildSearchList(const CDirectory * dir, 
								CFileList * list, 
								const char * ext,
								int &numfiles);

	bool SubDirExists(const char *path);
*/

	//FIXME, MOVE THIS SOMEPLACE MORE APPROPRIATE
	static bool CompareExts(const char *file, const char *ext);		//Compare file Extensions


};





#if 0
	CFile * LoadTexture(const char *file);	//Texture dir
	CFile * LoadModel(const char *file);	//Model dir
	CFile * LoadSkin(const char *file);		//Model dir
	CFile * LoadMap(const char *file);		//Map dir
	CFile * LoadSound(const char *file);	//Sound dir
	CFile * LoadImage(const char *file);	//Image dir
#endif


#endif
