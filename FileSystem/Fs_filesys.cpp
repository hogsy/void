#include "Fs_pakfile.h"
#include "Fs_zipfile.h"
#include "I_filesystem.h"


/*===========================================
Supported File Types
===========================================*/
const char  * archive_exts[] =	
{	
	"zip",
	"pak",  
	0 
};


/*
=========================================
Search Path - singly linked list
an entry might be an archive, 
or a standard dir path
=========================================
*/
struct CFileSystem::SearchPath_t
{
	SearchPath_t()  { prev = 0; archive = 0; }
	~SearchPath_t() { if(archive) delete archive;
					 prev=0; }
	
	//The path is always set to the directory itself, 
	//or the name of directly containing the archive
	char		   path[COM_MAXPATH];		
	CArchive     * archive;
	SearchPath_t * prev;
};


/*
=======================================================================
CFileSystem
=======================================================================
*/
/*
==========================================
Constructor and Destructor
==========================================
*/
CFileSystem::CFileSystem()
{
	m_numsearchpaths = 0;
	m_searchpaths = new SearchPath_t();
	m_lastpath = m_searchpaths;
}


CFileSystem::~CFileSystem()
{
	SearchPath_t * iterator = m_lastpath;
	while(iterator->prev)
	{
		m_lastpath = iterator->prev;
		delete iterator;
		iterator = m_lastpath;
	}
	delete iterator;
}

/*
===========================================
Initialize the file system
expects the basedirectory passed to it
===========================================
*/
bool CFileSystem::Init(const char *exedir, const char * basedir)
{
	//Validate given base dir
	if(!exedir || !basedir)
	{
		ComPrintf("CFileSystem::Init: No Exe directory specified\n");
		return false;
	}

	//Validate EXE dir name

	//check for length
	int exepathlen =0;
	exepathlen = strlen(exedir);
	if(!exepathlen || (exepathlen) > COM_MAXPATH)		
	{
		ComPrintf("CFileSystem::Init: Exe directory exceeds COM_MAXPATH : %s\n",exedir);
		return false;
	}
	//make sure there is no trailing slash
	strcpy(m_exepath,exedir);
	--exepathlen;
	if((m_exepath[exepathlen] == '/') || (m_exepath[exepathlen] == '\\'))
		m_exepath[exepathlen] = '\0';

	//Make sure the given path exists.
	if(!PathExists(m_exepath))
	{
		ComPrintf("CFileSystem::Init: Exe directory does not exist : %s\n",m_exepath);
		return false;
	}

	//Add to search path now
	if(!AddGameDir(basedir))
	{
		ComPrintf("CFileSystem::Init: Unable to add base dir, %s\n", basedir);
		return false;
	}
	strcpy(m_basedir,basedir);
	return true;
}

/*
===========================================
Add a content dir to the file system

The first dir is always the base content dir
However we can add another directory (only 1 at a time)
which can override content in the base dir.

if there already is an additional game dir, then
subsequent calls to AddGameDir will remove that game
dir, and replace it with the new one
===========================================
*/
bool CFileSystem::AddGameDir(const char *dir)
{
	//Validate Game dir name (no slashes before or after)
	if(strlen(dir) > COM_MAXPATH)
	{
		ComPrintf("CFileSystem::AddGameDir: Game directory exceeds COM_MAXDIRNAME: %s\n", dir);	
		return false;
	}

	//Dont copy any leading slash.
	char gamedir[COM_MAXPATH];
	if((dir[0] == '\\') || (dir[0] == '/'))
		strcpy(gamedir,dir+1);
	else
		strcpy(gamedir,dir);
	
	//Get rid of trailing slash
	int dirnamelen = strlen(gamedir) - 1;
	if((gamedir[dirnamelen] == '\\') || (gamedir[dirnamelen] == '/'))
		gamedir[dirnamelen] = '\0';

	//Check to see Dir exists.
	char fullpath[COM_MAXPATH];
	sprintf(fullpath,"%s/%s",m_exepath,gamedir);
	if(!PathExists(fullpath))
	{
		ComPrintf("CFileSystem::AddGameDir: Game dir does not exist : %s\n",fullpath);
		return false;
	}

	//Directoy is Valid. Reset the current GAME directoy before changing to new one
	ResetGameDir();

	//Add to SearchPath
	AddSearchPath(gamedir);

	//Add any archive files into the search path as well
	CStringList	archives;
	char archivepath[COM_MAXPATH];
	
	if(GetArchivesInPath(gamedir,&archives))
	{
		CStringList * iterator = &archives;
		while(iterator->next)
		{
			if(CompareExts(iterator->string,"zip"))
			{
				CZipFile * zipfile = new CZipFile();
sprintf(archivepath,"%s/%s", gamedir,iterator->string);
				if(zipfile->Init(archivepath, m_exepath))
					AddSearchPath(gamedir,(CArchive*)zipfile);
				else
					delete zipfile;
			}
			//Found a Pak file, try adding
			if(CompareExts(iterator->string,"pak"))
			{
				CPakFile * pakfile = new CPakFile();
sprintf(archivepath,"%s/%s", gamedir,iterator->string);
				if(pakfile->Init(archivepath, m_exepath))
					AddSearchPath(gamedir,(CArchive*)pakfile);
				else
					delete pakfile;
			}
			iterator = iterator->next;
		}
	}
	return true;
}


/*
==========================================
Reset the current game dir.
Nothing happens if no game dir is specified
otherwise, all the game dir entries are removed
and only base ones are left
==========================================
*/
void CFileSystem::ResetGameDir()
{
	//Remove all search paths associated with the current game dir
	//including the archive files
	SearchPath_t * iterator = m_lastpath;
	while(iterator)
	{
		//remove this entry if it doesnt match the base dir
		if(strcmp(iterator->path,m_basedir))
		{	
			SearchPath_t * temp = iterator;
			iterator = iterator->prev;
			delete temp;
			m_lastpath = iterator;
		}
		else
			iterator = iterator->prev;
	}
}

/*
===========================================
Loads the requested file into given buffer.
buffer needs to be null. Its allocated here
===========================================
*/
uint CFileSystem::LoadFileData(byte ** ibuffer, uint &buffersize, bool staticbuffer, const char *ifilename)
{
	uint size = 0;
	SearchPath_t * iterator = m_lastpath;
	
	while(iterator->prev)
	{
		iterator = iterator->prev;
		//Try opening as an archive
		if(iterator->archive)
		{
			//size = iterator->archive->OpenFile(ifilename,ibuffer);
			size = iterator->archive->LoadFile(ibuffer,buffersize,ifilename);
			if(size)
				return size;
		}
		//Try opening as a standard file
		else
		{
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s/%s/%s", m_exepath, iterator->path, ifilename);
			FILE * fp = fopen(filepath,"r+b");
			if(fp)
			{
				fseek(fp,0,SEEK_END);
				size = ftell(fp);
				fseek(fp,0,SEEK_SET);

				if(!staticbuffer)
				{
					*ibuffer = (byte*)MALLOC(size);
				}
				else
				{
					if(size > buffersize)
					{
						free(*ibuffer);
						*ibuffer = (byte*)MALLOC(size);
						buffersize = size;
					}
				}
				//fill the file buffer
				fread(*ibuffer,sizeof(byte),size,fp);
				fclose(fp);
				return size;
			}
		}
	}
	ComPrintf("CFileSystem::Open:File not found %s\n", ifilename);
	return 0;
}

/*

SearchPath_t * CFileSystem::GetFilePath(const char *ifilename)
{
	SearchPath_t * iterator = m_lastpath;
	while(iterator->prev)
	{
		iterator = iterator->prev;
		
		//Try opening as an archive
		if(iterator->archive)
		{
			if(iterator->archive->HasFile(ifilename))
				return iterator;
		}
		//Try opening as a standard file
		else
		{
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s/%s/%s", m_exepath, iterator->path, ifilename);
			FILE * fp = fopen(filepath,"r+b");
			if(fp)
			{
				fclose(fp);
				return iterator;
			}
		}
	}
	ComPrintf("CFileSystem::GetFilePath:File not found %s\n", ifilename);
	return 0;
}

*/
/*
int  CFileSystem::LoadFileBuffer(byte ** ibuffer, const char *ifilename);
{
	uint size = 0;
	SearchPath_t * iterator = m_lastpath;
	
	while(iterator->prev)
	{
		iterator = iterator->prev;
		
		//Try opening as an archive
		if(iterator->archive)
		{
			if(size = iterator->archive->LoadFile(fbuffer->m_buffer,
									   fbuffer->m_buffersize,
									   fbuffer->m_staticbuffer,ifilename))
				return size;
		}
		//Try opening as a standard file
		else
		{
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s/%s/%s", m_exepath, iterator->path, ifilename);
			FILE * fp = fopen(filepath,"r+b");
			if(fp)
			{
				fseek(fp,0,SEEK_END);
				size = ftell(fp);
				fseek(fp,0,SEEK_SET);

				if(!staticbuffer)
				{
					if(fbuffer->m_buffer)
					{
						free(fbuffer->m_buffer);
						fbuffer->m_buffer = 0;
					}
					fbuffer->m_buffer = (byte*)MALLOC(size);
					fbuffer->m_buffersize = size;
				}
				else
				{
					if(size > buffersize)
					{
						free(fbuffer->m_buffer);
						fbuffer->m_buffer = (byte*)MALLOC(size);
						fbuffer->m_buffersize = size;
					}
				}

				//fill the file buffer
				fread(fbuffer->m_buffer,sizeof(byte),size,fp);
				fclose(fp);
				return size;
			}
		}
	}
	ComPrintf("CFileSystem::Open:File not found %s\n", ifilename);
	return 0;
}
*/


/*
===========================================
Lists Added search paths
===========================================
*/
void CFileSystem::ListSearchPaths()
{
	if(!m_lastpath)
	{
		ComPrintf("CFileSystem::ListSearchPaths: File System is uninitialized\n");
		return;
	}
	ComPrintf("Current Search Paths:\n");

	SearchPath_t * iterator = m_lastpath;
	while(iterator->prev)
	{
		iterator = iterator->prev;
		if(iterator->archive)
			ComPrintf("%s\n",iterator->archive->m_archiveName);
		else
			ComPrintf("%s\n",iterator->path);
	}
}

/*
===========================================
Lists files in added archives
===========================================
*/
void CFileSystem::ListArchiveFiles()
{
	if(!m_lastpath)
	{
		ComPrintf("CFileSystem::ListArchiveFiles: File System is uninitialized\n");
		return;
	}
	ComPrintf("Archived files in search paths:\n");

	SearchPath_t * iterator = m_lastpath;
	while(iterator->prev)
	{
		iterator = iterator->prev;
		if(iterator->archive)
		{
			ComPrintf("\n%s\n",iterator->archive->m_archiveName);
			iterator->archive->ListFiles();
		}
	}
}

/*
===========================================
Scan a directory for supported archive types
===========================================
*/
bool CFileSystem::GetArchivesInPath(const char *path, CStringList * list)
{
	char		 searchpath[COM_MAXPATH];
	CStringList	*iterator = list;
	int			 num=0;
	
	for(int i=0; archive_exts[i]; i++)
	{
		WIN32_FIND_DATA	finddata;
		HANDLE	hsearch;
		bool	finished = false;

		sprintf(searchpath,"%s/%s/*.%s",m_exepath,path,archive_exts[i]);

		hsearch = FindFirstFile(searchpath,&finddata);
		if(hsearch == INVALID_HANDLE_VALUE)
		{
			ComPrintf("CFileSystem::GetArchivesInPath:Couldnt find %s\n",searchpath);
			continue;
		}

		while(!finished)
		{
			if(!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strcpy(iterator->string,finddata.cFileName);
				iterator->next = new CStringList();
				iterator = iterator->next;
				num++;
			}
			if (!FindNextFile(hsearch, &finddata))
			{
				if (GetLastError() == ERROR_NO_MORE_FILES) 
					finished = true;
			}
		}
		if(FindClose(hsearch) == FALSE)   // file search handle
			ComPrintf("CFileSystem::Win32_DirectoryList:Unable to close search handle\n");
	}
	list->QuickSortStringList(list,num);
	return true;
}


/*
===========================================
Check if given directoy exists
===========================================
*/
bool CFileSystem::PathExists(const char *path)
{
	WIN32_FIND_DATA	finddata;
	HANDLE hsearch = FindFirstFile(path,&finddata);
	if(hsearch == INVALID_HANDLE_VALUE)
		return false;

	if(FindClose(hsearch) == FALSE)
		ComPrintf("CFileSystem::PathExists:Unable to close search handle\n");
	return true;
}

/*
===========================================
Add SearchPath a search path
might be a real dir or an archive,
if its an archive, then the "path" var
will point to the real dir containing the 
archive
===========================================
*/
void CFileSystem::AddSearchPath(const char * path, CArchive * archive)
{
	strcpy(m_lastpath->path, path);
	m_lastpath->archive = archive;	//Will be zero, if a real dir was added

	SearchPath_t * newpath = new SearchPath_t();
	newpath->prev = m_lastpath;
	m_lastpath = newpath;
}

/*
===========================================
Remove Any SearchPaths
archives, or real dirs, that are in this path
===========================================
*/
void CFileSystem::RemoveSearchPath(const char *path)
{
	SearchPath_t * iterator = m_lastpath;

	while(iterator)
	{
		//remove this entry if matches
		if(strcmp(iterator->path,path)==0)
		{
			SearchPath_t * temp = iterator;
			iterator = iterator->prev;
			delete temp;
			m_lastpath = iterator;
		}
		else
			iterator = iterator->prev;
	}
}

/*
===========================================
Create a listing of files with the given
characteristics
===========================================
*/
int CFileSystem::FindFiles(CStringList *filelist,	//File List to fill
							const char  *ext,		//Extension		  
							const char  *path)		//Search in a specific path
{
	return 0;
}


/*
==========================================
Compare file Extensions
return true if equa
==========================================
*/
bool CFileSystem::CompareExts(const char *file, const char *ext)		
{
	const char *p = file;
	while(*p && *p!='.' && *p!='\0')
		p++;

	if(*p=='.')
	{
		if(!_stricmp(++p,ext))
			return true;
	}
	return false;
}

























































/*	
	//Returns a filelisting matching the criteria	
	int FindFiles(CStringList *filelist,		//File List to fill
				  const char  *ext,				//Extension		  
				  const char  *path=0);			//Search in a specific path
*/

/*
===========================================
Reinits the file system, then adds the new game dir
===========================================
*/
/*
bool CFileSystem::ChangeGameDir(const char *dir)
{
	//Remove all search paths associated with the current game dir
	//including the archive files
	SearchPath_t * iterator = m_lastpath;
	while(iterator)
	{
		//remove this entry if it doesnt match the base dir
		if(strcmp(iterator->path,m_basedir))
		{
			SearchPath_t * temp = iterator;
			iterator = iterator->prev;
			delete temp;
			m_lastpath = iterator;
		}
		else
			iterator = iterator->prev;
	}
	//Now add the new dir
	return AddGameDir(dir);
}
*/


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

int CFileSystem::FindFiles(CFileList  *filelist,			//File List to fill
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
			ComPrintf("CFileSystem::FindFiles: Invalid Path\n");
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
CFileList * CFileSystem::BuildSearchList(const CDirectory * dir, CFileList * list,
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




#if 0

/*
===========================================
QuickSorts the String list of files
===========================================
*/
void CFileSystem::QuickSortStringList(CStringList * const list,const int numitems)
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


#endif


