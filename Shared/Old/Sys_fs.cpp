#include "Sys_fs.h"
#include "Com_pakfile.h"

#include <windows.h>		//need this for Win32 find functions
#include <string.h>

extern void dprintf(char *string,...);

/*
=================================================================================
CDirectory Declaration
The Folder Class
Holds a list of all the subfolders and files in it
Its filled by the FileSystem, and its accessed by the filesystem
=================================================================================
*/
class CDirectory
{
public:
	CDirectory();
	~CDirectory();

	void ListContents();

	char		 m_path[COM_MAXPATH];
	int			 m_pathlen;

	CDirectory * m_dirs;
	int		     m_numdirs;

	CFile *	     m_files;
	int		     m_numfiles;
};

/*
==========================================
Constructor and Destructor
==========================================
*/
CDirectory::CDirectory()
{
	m_dirs = 0;
	m_files = 0;

	m_numfiles =0;
	m_numdirs = 0;

	m_pathlen = 0;
}
CDirectory::~CDirectory()
{
	if(m_dirs)
		delete [] m_dirs;
	if(m_files)
		delete [] m_files;

	m_dirs = 0;
	m_files = 0;
}

/*
===========================================
List the contents of the directory
===========================================
*/

void CDirectory::ListContents()
{
	int i;
	dprintf("================================\n%s\n",m_path);
	for(i=0; i<m_numfiles;i++)
			dprintf("%s\n",m_files[i].filename);

	for(i=0;i<m_numdirs;i++)
	{
		dprintf("%s\n",m_dirs[i].m_path);
		m_dirs[i].ListContents();
	}
}

/*
===========================================
Search Path
doubly linked list
points to a directory, keeps the actual dir path
archive file name + base game path
or real game path
===========================================
*/
class CSearchPath
{
public:
	CSearchPath()  { path =0; dir=0; next=prev = 0; }
	~CSearchPath() { if(path) delete [] path; 
					 if(dir)  delete dir; 
					 prev = next=0; }
	
	char	    * path;		//path info needed in openinig this dir structure
	CDirectory  * dir;		//directory structure
	
	CSearchPath * next;
	CSearchPath * prev;

	//Extra info about Archive here ?
};


/*
=======================================================================
CFileManager
=======================================================================
*/
/*
==========================================
Constructor and Destructor
==========================================
*/
CFileManager::CFileManager()
{
	m_totalfiles=0;
	m_totaldirs=0;
	
	memset(m_exepath,0,COM_MAXPATH);

	m_numarchives    = 0;
	m_numopenfiles   = 0;

	m_numsearchpaths = 0;
	m_searchpaths = new CSearchPath();
	m_lastpath = m_searchpaths;

	memset(m_openfiles,0,FS_MAXOPENFILES);
}
CFileManager::~CFileManager()
{
	m_lastpath =0;
	if(m_searchpaths)
	{
		CSearchPath * temp1 = m_searchpaths;
		CSearchPath * temp2 = 0;
		while(temp1->next)
		{	
			temp2 = temp1;
			temp1 = temp1->next;
			delete temp2;
		}
		delete temp1;
	}
}

/*
===========================================
Initialize the file system
expects the basedirectory passed to it
===========================================
*/
bool CFileManager::Init(const char *exedir, const char * gamedir)
{
	//Validate given base dir
	if(!exedir || !gamedir)
	{
		dprintf("CFileManager::Init: No Base directory specified\n");
		return false;
	}
	int len =0;
	
	//Validate EXE dir name
	len = strlen(exedir);
	if((len + 2) > COM_MAXPATH)						//check for length
	{
		dprintf("CFileManager::Init: Base directory exceeds COM_MAXPATH : %s\n",exedir);
		return false;
	}
	if((exedir[0] == '\\') || (exedir[0] == '/'))	//get rid of any leading slash
	{
		len -=2;
		strcpy(m_exepath,exedir+1);
	}
	else
	{
		len -=1;
		strcpy(m_exepath,exedir);
	}
	//make sure there is a trailing slash since we'll use this path to open files
	if(m_exepath[len] != '/')
	{
		m_exepath[len+1] = '/';
		m_exepath[len+2] = '\0';
	}

	//Validate Game dir name (no slashes before or after)
	char basepath[COM_MAXDIRNAME];
	if(strlen(gamedir)+1 > COM_MAXDIRNAME)
	{
		dprintf("CFileManager::Init: Game directory exceeds COM_MAXDIRNAME: %s\n", basepath);	
		return false;
	}
	//Check for leading slash
	if((gamedir[0] == '\\') || (gamedir[0] == '/'))
		strcpy(basepath,gamedir+1);
	else
		strcpy(basepath,gamedir);
	
	//Get rid of trailing slash for base dirs
	len = strlen(basepath) - 1;
	if((basepath[len] == '\\') || (basepath[len] == '/'))
		basepath[len] = '\0';

	//Add to search path now
	return AddSearchPath(basepath,strlen(basepath));
}

/*
===========================================
Add SearchPath
===========================================
*/
bool CFileManager::AddSearchPath(const char * path, const int pathlen)
{
	m_lastpath->path = new char[pathlen +1];
	strcpy(m_lastpath->path, path);
	m_lastpath->dir = new CDirectory();

	if(!BuildDirectoryList(m_lastpath->dir,
						   0, m_lastpath->path))
	{
		dprintf("CFileManager::AddSearchPath:Error buidling dir list\n");
		return false;									
	}
	m_lastpath->next = new CSearchPath();
	m_lastpath->next->prev = m_lastpath;
	m_lastpath = m_lastpath->next;
	return true;
}

/*
===========================================
Remove SearchPath
===========================================
*/
bool CFileManager::RemoveSearchPath(const char *path)
{
	CSearchPath * node = m_searchpaths;

	while(strcmp(node->path, path))
	{
		if(!node->next)
			return false;
		node = node->next;
	}

	//If its the first entry, ignore it
	if(node->prev)
	{
		//Link the border nodes
		node->prev->next = node->next;
		node->next->prev = node->prev;
		delete node;
		return true;
	}
	node->next->prev = 0;
	delete node;
	return true;
}

/*
===========================================
Adds a game directory to override base content
===========================================
*/
bool CFileManager::AddGameDir(const char *dir)
{
	if(!dir)
		return false;

	//Validate Game dir name (no slashes before or after)
	if(strlen(dir)+1 > COM_MAXDIRNAME)
	{
		dprintf("CFileManager::Init: Game directory exceeds COM_MAXDIRNAME: %s\n", dir);	
		return false;
	}

	char gamedir[COM_MAXDIRNAME];

	//Check for leading slash
	if((dir[0] == '\\') || (dir[0] == '/'))
		strcpy(gamedir,dir+1);
	else
		strcpy(gamedir,dir);
	
	//Get rid of trailing slash for base dirs
	int len = strlen(gamedir) - 1;
	if((gamedir[len] == '\\') || (gamedir[len] == '/'))
		gamedir[len] = '\0';

	//Add to search path now
	return AddSearchPath(gamedir,strlen(gamedir));
}

/*
===========================================
Reinits the file system, then adds the new game dir
===========================================
*/
bool CFileManager::ChangeGameDir(const char *dir)
{
	return false;
}

/*
===========================================
Builds directory structure in the given 
directory object for the given path

Experiment with optimization here

===========================================
*/
bool CFileManager::BuildDirectoryList(CDirectory *dir,		//Directory that will be updated
									  const char *dirname,	//Proposed name of the directory
									  const char *path)		
{
	char	dirpath[COM_MAXPATH];

	if(!path)
	{
		dprintf("CFileManager::BuildDirectory: bad dirname or path\n");
		return false;
	}

	//Set the root directories name
	if(!dirname)
	{
		strcpy(dir->m_path, "/");
		dir->m_pathlen = 0;
		//Set the Search Query path
		sprintf(dirpath,"%s%s/*.*",m_exepath,path);
	}
	else
	{
		dir->m_pathlen = strlen(dirname);
		if(dir->m_pathlen < COM_MAXPATH)
			strcpy(dir->m_path, dirname);
		else
		{
			dprintf("CFileManager::BuildDirectoryList: dirname is too long\n");
			dir->m_pathlen = 0;
			return false;
		}
		sprintf(dirpath,"%s%s/%s*.*",m_exepath,path,dirname);
	}

	CStringList	dirlist, 
				filelist;
	int			numdirs=0, numfiles =0;
	
	if(!Win32_DirectoryList(dirpath,numdirs,&dirlist, numfiles, &filelist))
	{
		dprintf("CFileManager::BuildDirectoryList: failed to scan directory\n");
		return false;
	}

	CStringList *iterator=0;
	int i=0;

	if(numfiles)
	{
		//Sort the files alphabetically
		QuickSortStringList(&filelist,numfiles);
		iterator = &filelist;

		if(dir->m_files && dir->m_numfiles)
		{
			delete [] dir->m_files;
			m_totalfiles -= dir->m_numfiles;
		}
		//Alloc space
		dir->m_files = new CFile[numfiles];
		dir->m_numfiles = numfiles;

		for(i=0;i<numfiles;i++)
		{
			dir->m_files[i].SetInfo(iterator->string,path);
			
			//Check for pak file
//			if(CompareExts(iterator->string,"PAK"))
//				PrintQuakePakFile(&dir->m_files[i]);
			iterator = iterator->next;
		}
		m_totalfiles += numfiles;
	}
	if(numdirs)
	{
		//Sort the files
		QuickSortStringList(&dirlist,numdirs);
		iterator = &dirlist;

		if(dir->m_dirs && dir->m_numdirs)
		{
			delete [] dir->m_dirs;
			m_totalfiles -= dir->m_numdirs;
		}
		//Alloc space
		dir->m_dirs = new CDirectory[numdirs];
		dir->m_numdirs = numdirs;

		char dname[COM_MAXPATH];
		
		//add dirs
		for(i=0;i<numdirs;i++)
		{
			if(dirname)
			{
				strcpy(dname,dirname);
				strcat(dname,iterator->string);
			}
			else
				strcpy(dname,iterator->string);
			
			//Recurse and load the subdirectories here until there are no more
			BuildDirectoryList(&dir->m_dirs[i],dname,path);	
			iterator = iterator->next;
		}
		m_totaldirs += numdirs;
	}
	return true;
}

/*
===========================================
Scans passed path and fills the dir and filelist
updated number of files and dirs in the CDirectory
structure thats passed to it
===========================================
*/

bool CFileManager::Win32_DirectoryList(const char *path,
									   int & numdirs,
									   CStringList *dirlist,		
									   int & numfiles,
									   CStringList *filelist)
{
	WIN32_FIND_DATA	finddata;
	HANDLE	hsearch;
	bool	finished = false;
	CStringList	*dirs = dirlist;
	CStringList	*files = filelist;

	hsearch = FindFirstFile(path,&finddata);
	if(hsearch == INVALID_HANDLE_VALUE)
	{
		dprintf("CFileManager::Win_DirectoryList: FindFirstFile failed\nCurrent path : %s",path);
		return false;;
	}

	while(!finished)
	{
		if(dirs && finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(finddata.cFileName[0] != '.')
			{
				strcpy(dirs->string,finddata.cFileName);
				strcat(dirs->string,"/");
				dirs->next = new CStringList();
				dirs = dirs->next;
				numdirs++;
			}
		}
		else
		{
			strcpy(files->string,finddata.cFileName);
			files->next = new CStringList();
			files = files->next;
			numfiles++;
		}

		if (!FindNextFile(hsearch, &finddata))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
				finished = true;
		}
	}
	if(FindClose(hsearch) == FALSE)   // file search handle
	{	dprintf("Scan:Unable to close search handle\n");
	}
	return true;

}


/*
===========================================
Search and return a file
===========================================
*/
CFile * CFileManager::GetFile(const char *filename, CDirectory * dir)
{
	//Match initial paths to see if it matches with any of the subdirs
	for(int i=0;i<dir->m_numdirs;i++)
	{
		if(!_strnicmp(filename,
					  dir->m_dirs[i].m_path,
					  dir->m_dirs[i].m_pathlen))
			return GetFile(filename,&dir->m_dirs[i]);
	}
	//didnt find any directory matching this name, is it a file ?
	return BinarySearchForFile(filename,
							   dir->m_files,
							   dir->m_pathlen,
							   0,
							   dir->m_numfiles);
}

/*
===========================================

===========================================
*/

CFile * CFileManager::LoadArchivedFile(CFile * file)
{
	//We know the parent archive of the given file
	//Find extractor function and expand the file.
	return 0;
}



/*
==========================================
Print out the list of files and dirs
==========================================
*/
void CFileManager::ListFiles()
{
	CSearchPath * list = m_searchpaths;
	while(list->next)
	{
		list->dir->ListContents();
		list = list->next;
	}
	dprintf("Total files added :%d\n",m_totalfiles);
	dprintf("Total dirs  added :%d\n",m_totaldirs);
}


/*
===========================================
Try to open the requested file
-File name
-AccessMode
===========================================
*/

CFile * CFileManager::Open(const char *filename)
{
	//Validate filename
	if(!filename)
	{
		dprintf("CFileManager::OpenFile:no filename passed\n");
		return 0;
	}
	//See if the max number of open files has been reached
	if(m_numopenfiles >= FS_MAXOPENFILES)
	{
		dprintf("CFileManager::OpenFile:MAX number of openfiles reached\n");
		return 0;
	}

	//Start from the last dir in the search path
	CFile * file = 0;
	CSearchPath * iterator = m_lastpath->prev;
	while(file == 0)
	{
		file = GetFile(filename, iterator->dir);
		if(!iterator->prev || file)
			break;
		iterator = iterator->prev;
	}

	//File doesnt exist. if read access then return
	if(!file)
	{
		dprintf("CFileManger::OpenFile: File not found: %s\n",filename);
		return 0;
	}

	//Find open space in OpenFilesArray
	for(int i=0;i<FS_MAXOPENFILES;i++)
	{
		if(!m_openfiles[i])
		{
			//Check if parent is an archive file

			//Otherwise
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s%s/%s", m_exepath,file->m_parent,filename);

			file->fp = fopen(filepath,"r+b");
			if(!file->fp)
			{
				dprintf("CFileManager::OpenFile: Couldnt open file : %s\n", file->filename);
				return 0;
			}
			m_openfiles[i] = file;
			m_numopenfiles ++;

			//File is opened, check size
			fseek(m_openfiles[i]->fp,0,SEEK_END);
			file->m_size = ftell(m_openfiles[i]->fp);
			fseek(m_openfiles[i]->fp,0,SEEK_SET);

			//Alloc the Buffer and fill it with file data.
/*			if(file->m_size)
			{
				file->m_buffer = new unsigned char[file->m_size];

				unsigned int bytes_read = fread(file->m_buffer,sizeof(unsigned char),file->m_size,m_openfiles[i].fp);
				if(bytes_read != file->m_size)
				{
					dprintf("Warning: Couldnt read entire file %s, %d of %d bytes\n",
										file->filename, bytes_read, file->m_size);
				}
			}
*/
			return file;
		}
	}
	return 0;
}


/*
===========================================
Close the give file
===========================================
*/
bool CFileManager::Close(CFile * file)
{
	if(!file)
		return false;
	
	//verify that the file is open
	for(int i=0; i<FS_MAXOPENFILES; i++)
	{
		//remove it from the list, and close it (empty buffer)
		if(file == m_openfiles[i])
		{
			m_numopenfiles--;
			file->Close();
			m_openfiles[i] =0;
			return true;
		}
	}
	return false;
}


/*
===========================================
Binary search for file with in an ordered
CFile array
===========================================
*/
CFile * CFileManager::BinarySearchForFile(const char *name,	
										  CFile * array,
										  const int &offset,
										  int low, int high)
{
	//Array doenst have any items
	if(low == high)
		return 0;
	
	int mid = (low+high)/2;
	int ret = _stricmp(name + offset,array[mid].filename);

	if(ret == 0)		//name is equal to entry in array
		return &array[mid];
	else if(mid == low)
		return 0;
	else if(ret > 0)	//name is greater than array entry
		return BinarySearchForFile(name,array,offset,mid,high);
	else if(ret < 0)	//name is less than array entry
		return BinarySearchForFile(name,array,offset,low,mid);
	return 0;
}


/*
==========================================
Compare file Extensions
return true if equa
==========================================
*/
bool CFileManager::CompareExts(const char *file, const char *ext)		
{
	//Wildcards
	if(!ext || ext[0] == '*' || ext[0] == '?')
		return true;

	const char *p = file;
	while(*p && *p!='.' && *p!='\0')
		p++;
	
	//found the extension
	if(*p== '.')
	{
		p++;
		const char *q = ext;
		while(*p && *p!='\0')
		{
			if((!(*q) || *q=='\0')
				|| (*p != *q))
				return false;	//not equal
			p++;
			q++;
		}
		return true;		//equal
	}
	return false;
}

/*
===========================================
Finds a Directory that matches the given path
===========================================
*/
CDirectory * CFileManager::GetDirFromPath(const char * path, 
								  const int &pathlen,
								  CDirectory *dir)
{
	//Comparision is done for pathlen -1 characters to ignore the ending slash
	if((dir->m_pathlen == pathlen) &&
	   (!_strnicmp(path, dir->m_path, --dir->m_pathlen)))
			return dir;

	for(int i=0;i<dir->m_numdirs; i++)
	{
		if(!_strnicmp(path,
					  dir->m_dirs[i].m_path,
					  dir->m_dirs[i].m_pathlen-1))
		  return GetDirFromPath(path, pathlen, &dir->m_dirs[i]);

	}
	return 0;
}


/*
===========================================
QuickSorts the String list of files
===========================================
*/
void CFileManager::QuickSortStringList(CStringList * const list,const int numitems)
{
	if(numitems < 2)
		return;

	CStringList *	sorted = new CStringList[numitems];		//dest array
	CStringList * pivot = list;							//let the first one be the pivot
	CStringList *	iterator = list->next;					
	
	int maxindex = numitems-1;
	int left=0;
	int right = maxindex; 
	
	//loop one less time since the first item is the pivot
	for(int i=0,comp=0;i<maxindex;i++)
	{
		comp = _stricmp(iterator->string,pivot->string);
		
		if(comp < 0)
		{
			strcpy(sorted[left].string,iterator->string);
			sorted[left].next = &sorted[(left+1)];
			left++;
		}
		else if(comp >= 0)
		{
		    strcpy(sorted[right].string,iterator->string);
			sorted[(right-1)].next = &sorted[right];
			right--;
		}
		iterator = iterator->next;
	}
	
	//Copy the pivot point in the empty space
	strcpy(sorted[left].string, pivot->string);
	if(right == maxindex)
		sorted[left].next = 0;
	else
		sorted[left].next = &sorted[(left+1)];
		

	if(left > 1) 
		QuickSortStringList(sorted,left);								
	if((numitems - (right+1)) > 1)
		QuickSortStringList(&sorted[left+1],(numitems - (right+1)));		//starting from the one right after the pivot
	
	//List is sorted, copy into return filelist now
	//everything is sorted now copy it over

	iterator = list;
	for(i=0;i<numitems;i++)
	{
		strcpy(iterator->string,sorted[i].string);
		iterator=iterator->next;
		sorted[i].next = 0;						//get rid of the links so we dont have problems deleting the array
	}
	delete [] sorted;
}



#if 0
/*
===========================================
Returns a filelisting matching the criteria	
-returns number of files found
-extension
-File List to fill
-search in a specific path
===========================================
*/

int CFileManager::FindFiles(CFileList  *filelist,			//File List to fill
							const char *ext,				//extension
							const char *path)				//Search in a specific path
{
	int nummatches = 0;
	
	if(!path)
	{
	}

	//Path is given, search through just one dir
	if(path)
	{
		CDirectory * dir = 0;
		CFileList *fl = filelist;
		int len = strlen(path);
		
		if(path[len-1] == '\\' || path[len-1] == '/')
			--len;
		
		//Get rid of leading slash
		if(path[0] == '\\' || path[0] == '/')
			dir = GetDirFromPath(path+1,len,m_pbasedir);
		else
			dir = GetDirFromPath(path,len-1,m_pbasedir);

		if(!dir)
		{
			dprintf("CFileManager::FindFiles: Invalid Path\n");
			return 0;
		}

		for(int i=0; i< dir->m_numfiles; i++)
		{
			if(CompareExts(dir->m_files[i].filename,ext))
			{

				fl->file = &dir->m_files[i];
				fl->next = new CFileList();
				fl = fl->next;
				nummatches ++;
			}
		}
		return nummatches;
	}
	filelist = BuildSearchList(m_pbasedir,filelist,ext, nummatches);
	return nummatches;
}

/*
===========================================

===========================================
*/
CFileList * CFileManager::BuildSearchList(const CDirectory * dir, CFileList * list,
										  const char * ext, int &numfiles)
{
	CFileList * fl = list;

	//Add any local files
	for(int i=0; i< dir->m_numfiles; i++)
	{
		if(CompareExts(dir->m_files[i].filename,ext))
		{
			fl->file = &dir->m_files[i];
			fl->next = new CFileList();
			fl = fl->next;
			numfiles ++;
		}
	}

	//Iterate through subdirs and add those files
	for(i=0;i<dir->m_numdirs; i++)
		  fl = BuildSearchList(&dir->m_dirs[i],fl,ext,numfiles);
	return fl;
}
#endif





