#ifndef _SYS_FILESYSTEM_
#define _SYS_FILESYSTEM_


#include "Com_file.h"
#include <iostream.h>
#include <string.h>

typedef char TFilePath[MAXFILEPATH];

/*
==========================================
The Directory Class
Loads a directory
creates an ordered list of files and directories
and returns files when wanted
==========================================
*/

class CDirectory
{
public:
	CDirectory();
	~CDirectory();

	bool Init(const char* base);
	void Release();					

	//Return this file
	CFile * GetFile(const char *name);					
	CFile * GetFile(const char *name, CDirectory ** dir);
	
	//Find a file with this extension, the one after this num
	TFilePath * FindFiles(const char *ext, int  &numfiles);
	
	char    path[MAXFILEPATH];
	int		pathlen;

	void	Print();

private:

	static char m_basedir[MAXFILEPATH];

	class CItemList
	{
	public:
		CItemList() 
		{ 
			memset(name,0,MAXFILEPATH);
			next =0;
		}
		~CItemList() 
		{	
			if(next!=0)	
			{
				delete next;
				next=0; 
			}
		}
	private:
		friend class CDirectory;

		char	    name[MAXFILEPATH];
		CItemList * next;
	};

	bool Load(const char *name);								//Load this directory
	bool Scan(CItemList *dirlist,								//Creates lists of dirs and files, and updates numbers
			  CItemList *filelist);

	bool ScanFiles(CItemList **filelist, 
				   int &numfiles,
				   const char *pattern);
						   
	void QuickSortList(CItemList * const list,					//Sorts the lists of dirs and files
						const int numitems);
	void CreateFileList(const char *ext, CItemList **filelist, 
						int &numfiles);
	CFile * BinarySearchForFile(const char *name,				//Binary search for file
						CFile * array, 
						int low, int high);
	
	

	static bool ComparePaths(const char *file, const char *path);		//Compare the directory paths
	static bool CompareExts(const char *file, const char *ext);		//Compare file Extensions
	
	CFile   *	m_files;	//list of files in this directory
	int			m_numfiles;
	CDirectory* m_dir;		//pointer to dirs inside this 
	int			m_numdirs;

};



/*
==========================================
The File Manager
==========================================
*/
class CFileManager
{
public:
	CFileManager();
	CFileManager(const char *basepath);			// e.g C:\\Void\\ , expect an ending slash
	virtual ~CFileManager();

	//Creates the list
	bool Init(const char *basedir,				//what do we want as the primary base directory
			  const char ** ext=0);				//pointer to list of file extensions to add
	bool Shutdown();							//destroy filelisting

	CFile * GetFile(const char *file);			//return file pointer to this file
	CFile * OpenFile(const char *file, const char *mode);

	//Crappy Search function
	TFilePath *  FindFiles(const char *ext,				//fill given char array with complete name of files 
						   int   &numfiles,
						   const char *path=0);	

	//Print out the list
	void	ListFiles();

	static int		totalfiles;
	static int		totaldirs;

private:

	char		 m_basedir[MAXFILEPATH];
	CDirectory * m_pbasedir;
};














#if 0
	CDirectory * m_pgamedir;
	//Game Directory stuff
	bool InitGameDir(const char *gamedir);		//Load user game directory, if any
	bool ReleaseGameDir();						//Release user game directory

	CFile * LoadTexture(const char *file);	//Texture dir
	CFile * LoadModel(const char *file);	//Model dir
	CFile * LoadSkin(const char *file);		//Model dir
	CFile * LoadMap(const char *file);		//Map dir
	CFile * LoadSound(const char *file);	//Sound dir
	CFile * LoadImage(const char *file);	//Image dir
#endif

/*	bool	GetFilePath(const char *filename, char *path);
	bool	ParseDir(const char *string, char *dir);	//Parse a directory name out of a string
	
	CDirectory * FindDirectory(const char * path);
*/	

#endif