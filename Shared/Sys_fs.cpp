#include "Sys_fs.h"
#include "Com_pakfile.h"
#include "Com_zipfile.h"

extern void dprintf(char *string,...);

const char  * archive_exts[] =	{ "zip","pak",  0 };


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
	CSearchPath()  { next=prev = 0; archive = 0; }
	~CSearchPath() { if(archive) delete archive;
					 prev=next=0;   }
	
	//Search Path will either be an archive, or a standard dir path
	CArchive  * archive;				
	char		path[COM_MAXPATH];		
	
	CSearchPath * next;
	CSearchPath * prev;
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

	m_numopenfiles   = 0;
	memset(m_openfiles,0,FS_MAXOPENFILES*sizeof(CFile*));

	m_numsearchpaths = 0;
	m_searchpaths = new CSearchPath();
	m_lastpath = m_searchpaths;
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

	//Validate EXE dir name
	int len =0;
	len = strlen(exedir);
	if((len) > COM_MAXPATH)							//check for length
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
		len --;
		strcpy(m_exepath,exedir);
	}
	//make sure there is a trailing slash since we'll use this path to open files
	if((m_exepath[len] == '/') || (m_exepath[len] == '\\'))
		m_exepath[len] = '\0';

	//Add to search path now
	return AddGameDir(gamedir);
}


/*
===========================================
Adds a game directory to override base content
===========================================
*/
bool CFileManager::AddGameDir(const char *dir)
{
	//Validate Game dir name (no slashes before or after)
	if(strlen(dir) > COM_MAXPATH)
	{
		dprintf("CFileManager::Init: Game directory exceeds COM_MAXDIRNAME: %s\n", dir);	
		return false;
	}

	char gamedir[COM_MAXPATH];

	//Check for leading slash
	if((dir[0] == '\\') || (dir[0] == '/'))
		strcpy(gamedir,dir+1);
	else
		strcpy(gamedir,dir);
	
	//Get rid of trailing slash for base dirs
	int len = strlen(gamedir) - 1;
	if((gamedir[len] == '\\') || (gamedir[len] == '/'))
		gamedir[len] = '\0';

	//Add to SearchPath
	AddSearchPath(gamedir);

	//Add any archive files in here
	int  i=0;
	CStringList	archives;
	char archivepath[COM_MAXPATH];
	
	if(GetArchivesInPath(gamedir,&archives))
	{
		CStringList * iterator = &archives;
		while(iterator->next)
		{
			dprintf("%s\n",iterator->string);

			if(CompareExts(iterator->string,"zip"))
			{
			}
			if(CompareExts(iterator->string,"pak"))
			{
				CPakFile * pakfile = new CPakFile;
				
				sprintf(archivepath,"%s/%s", gamedir,iterator->string);
				if(pakfile->Init(m_exepath, archivepath))
					AddSearchPath(pakfile);
				else
					delete pakfile;
			}
			iterator = iterator->next;
		}
	}
	return true;
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
Opens the requested file
===========================================
*/

CFile * CFileManager::Open(const char *filename)
{
	if(m_numopenfiles >= FS_MAXOPENFILES)
	{
		dprintf("CFileManager::Open: Can't open %s, reached max open files\n", filename);
		return 0;
	}

	//Find space in open file array
	for(int i=0; i< FS_MAXOPENFILES; i++)
	{
		if(!m_openfiles[i])
			break;
	}
	//Unable to find a space in openfile array
	if(m_openfiles[i])
	{
		dprintf("CFileManager::Open: Can't open %s, unable to find blank entry in openfiles\n", filename);
		return 0;
	}

	byte * buffer = 0;
	long size = 0;
	CSearchPath * iterator = m_lastpath;

	while(iterator->prev)
	{
		iterator = iterator->prev;

		//Try opening as an archive
		if(iterator->archive)
		{
			size = iterator->archive->OpenFile(filename,&buffer);
			if(size)
			{
				m_openfiles[i] = new CFile();
				m_openfiles[i]->m_buffer = buffer;
				m_openfiles[i]->m_size = size;
				m_numopenfiles ++;
				return m_openfiles[i];
			}
		}
		//Try opening as a standard dir
		else
		{
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s/%s/%s", m_exepath, iterator->path, filename);
			FILE * fp = fopen(filepath,"r+b");
			
			if(fp)
			{
				m_openfiles[i] = new CFile();
				m_openfiles[i]->m_fp = fp;
				m_numopenfiles ++;
				return m_openfiles[i];
			}

		}
	}
	return 0;
}


/*
===========================================

===========================================
*/

bool CFileManager::Close(CFile *file )
{
	if(!file)
		return false;
	
	//verify that the file is open
	for(int i=0; i<FS_MAXOPENFILES; i++)
	{
		//remove it from the list, and close it (empty buffer)
		if(file == m_openfiles[i])
		{
			file = 0;
			m_numopenfiles--;
			m_openfiles[i]->Close();
			delete m_openfiles[i];
			m_openfiles[i] = 0;
			return true;
		}
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

===========================================
*/

void CFileManager::ListArchiveFiles()
{
	if(!m_lastpath)
		return;

	CSearchPath * iterator = m_lastpath;
	while(m_lastpath->prev)
	{
		m_lastpath = m_lastpath->prev;
		if(m_lastpath->archive)
			m_lastpath->archive->ListFiles();
	}
}



/*
===========================================

===========================================
*/
bool CFileManager::GetArchivesInPath(const char *path, CStringList * list)
{
	char		 searchpath[COM_MAXPATH];
	CStringList	*iterator = list;
	
	for(int i=0; archive_exts[i]; i++)
	{
		WIN32_FIND_DATA	finddata;
		HANDLE	hsearch;
		bool	finished = false;

		sprintf(searchpath,"%s/%s/*.%s",m_exepath,path,archive_exts[i]);

		hsearch = FindFirstFile(searchpath,&finddata);
		if(hsearch == INVALID_HANDLE_VALUE)
		{
			dprintf("CFileManager::Win_DirectoryList: FindFirstFile failed\nCurrent path : %s",path);
			return false;;
		}

		while(!finished)
		{
			if(!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strcpy(iterator->string,finddata.cFileName);
				iterator->next = new CStringList();
				iterator = iterator->next;
			}
			if (!FindNextFile(hsearch, &finddata))
			{
				if (GetLastError() == ERROR_NO_MORE_FILES) 
					finished = true;
			}
		}
		if(FindClose(hsearch) == FALSE)   // file search handle
			dprintf("CFileManager::Win32_DirectoryList:Unable to close search handle\n");
	}
	return true;
}


/*
===========================================
Add SearchPath
===========================================
*/
void CFileManager::AddSearchPath(const char * path)
{
	strcpy(m_lastpath->path, path);
	m_lastpath->archive = 0;

	m_lastpath->next = new CSearchPath();
	m_lastpath->next->prev = m_lastpath;
	m_lastpath = m_lastpath->next;
}

/*
===========================================

===========================================
*/

void CFileManager::AddSearchPath(CArchive * file)
{
	m_lastpath->archive = file;

	m_lastpath->next = new CSearchPath();
	m_lastpath->next->prev = m_lastpath;
	m_lastpath = m_lastpath->next;
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
==========================================
Compare file Extensions
return true if equa
==========================================
*/
bool CFileManager::CompareExts(const char *file, const char *ext)		
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



#if 0

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


#endif



