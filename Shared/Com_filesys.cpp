#include "../Exe/Source/Sys_hdr.h"			//Exe

#include "Com_filesys.h"
#include "Com_pakfile.h"
#include "Com_zipfile.h"

//extern void ComPrintf(char *string,...);

const char  * archive_exts[] =	{ "zip","pak",  0 };

/*
===========================================
Search Path
singly linked list

And entry might points to 
only a real directoy
or
an archive with the real directoys name
===========================================
*/
class CSearchPath
{
public:
	CSearchPath()  { prev = 0; archive = 0; }
	~CSearchPath() { if(archive) delete archive;
					 prev=0; }
	
	//Search Path will either be an archive, or a standard dir path
	CArchive  * archive;				
	char		path[COM_MAXPATH];		//This is always set to the directory itself, 
										//or the name of directly containing the archive
	
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

	memset(m_openfiles,0,COM_MAXOPENFILES*sizeof(CFile*));
	
	m_numsearchpaths = 0;
	m_searchpaths = new CSearchPath();
	m_lastpath = m_searchpaths;
}


CFileManager::~CFileManager()
{
	//Close any open files
	for(int i=0;i<COM_MAXOPENFILES; i++)
	{
		if(m_openfiles[i])
			Close(m_openfiles[i]);
	}

	CSearchPath * iterator = m_lastpath;
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
bool CFileManager::Init(const char *exedir, const char * basedir)
{
	//Validate given base dir
	if(!exedir || !basedir)
	{
		ComPrintf("CFileManager::Init: No Base directory specified\n");
		return false;
	}

	//Validate EXE dir name
	int len =0;
	len = strlen(exedir);
	
	//check for length
	if(!len || (len) > COM_MAXPATH)		
	{
		ComPrintf("CFileManager::Init: Base directory exceeds COM_MAXPATH : %s\n",exedir);
		return false;
	}
	
	//get rid of any leading slash
	if((exedir[0] == '\\') || (exedir[0] == '/'))	
	{
		len -=2;
		strcpy(m_exepath,exedir+1);
	}
	else
	{
		len --;
		strcpy(m_exepath,exedir);
	}
	
	//make sure there is no trailing slash
	if((m_exepath[len] == '/') || (m_exepath[len] == '\\'))
		m_exepath[len] = '\0';

	//Add to search path now
	if(AddGameDir(basedir))
	{
		strcpy(m_basedir,basedir);
		return true;
	}
	ComPrintf("CFileManager::Init:Unable to add base dir, %s\n", basedir);
	return false;
}


/*
===========================================
Adds a game directory to override content in base
===========================================
*/
bool CFileManager::AddGameDir(const char *dir)
{
	//Validate Game dir name (no slashes before or after)
	if(strlen(dir) > COM_MAXPATH)
	{
		ComPrintf("CFileManager::Init: Game directory exceeds COM_MAXDIRNAME: %s\n", dir);	
		return false;
	}

	char gamedir[COM_MAXPATH];

	//Dont copy leading slash.
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

	//Add any archive files into the search path as well
	int  i=0;
	CStringList	archives;
	char archivepath[COM_MAXPATH];
	
	if(GetArchivesInPath(gamedir,&archives))
	{
		CStringList * iterator = &archives;
		while(iterator->next)
		{
			ComPrintf("%s\n",iterator->string);

			if(CompareExts(iterator->string,"zip"))
			{
			}
			if(CompareExts(iterator->string,"pak"))
			{
				CPakFile * pakfile = new CPakFile;
				
				sprintf(archivepath,"%s/%s", gamedir,iterator->string);
				if(pakfile->Init(m_exepath, archivepath))
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
===========================================
Reinits the file system, then adds the new game dir
===========================================
*/
bool CFileManager::ChangeGameDir(const char *dir)
{
	//Close any open files
	for(int i=0;i<COM_MAXOPENFILES; i++)
	{
		if(m_openfiles[i])
			Close(m_openfiles[i]);
	}


	CSearchPath * iterator = m_lastpath;
	while(iterator)
	{
		//remove this entry if it doesnt match the base dir
		if(strcmp(iterator->path,m_basedir))
		{
			CSearchPath * temp = iterator;
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


/*
===========================================
Opens the requested file
===========================================
*/
CFile * CFileManager::Open(const char *filename)
{
	if(m_numopenfiles >= COM_MAXOPENFILES)
	{
		ComPrintf("CFileManager::Open:Can't open %s, reached max open files\n", filename);
		return 0;
	}

	//Find space in open file array
	for(int i=0; i< COM_MAXOPENFILES; i++)
	{
		if(!m_openfiles[i])
			break;
	}
	//Unable to find a space in openfile array
	if(m_openfiles[i])
	{
		ComPrintf("CFileManager::Open:Can't open %s, unable to find blank entry in openfiles\n", filename);
		return false;
	}

	long size = 0;
	CSearchPath * iterator = m_lastpath;
	byte *buffer=0;

	while(iterator->prev)
	{
		iterator = iterator->prev;

		//Try opening as an archive
		if(iterator->archive)
		{
			//size = iterator->archive->OpenFile(filename,&(file->m_buffer));
			size = iterator->archive->OpenFile(filename,&buffer);
			if(size)
			{
				CFile * file = new CFile();

				//Set the size and the name
				file->m_filename = new char[strlen(filename)+1];
				strcpy(file->m_filename, filename);
				file->m_size = size;
				file->m_buffer = buffer;
				m_openfiles[i] = file;
				m_numopenfiles ++;
				return file;
			}
		}
		//Try opening as a standard file
		else
		{
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s/%s/%s", m_exepath, iterator->path, filename);
			FILE * fp = fopen(filepath,"r+b");
			
			if(fp)
			{
				CFile * file = new CFile();
				
				fseek(fp,SEEK_END,0);
				file->m_size = ftell(fp);
				fseek(fp,SEEK_SET,0);

				//fill the file buffer
				file->m_buffer = new unsigned char[file->m_size];
				fread(file->m_buffer,file->m_size,1,fp);
				fclose(fp);

				file->m_filename = new char[strlen(filename)+1];
				strcpy(file->m_filename, filename);
				
				m_openfiles[i] = file;
				m_numopenfiles ++;
				return file;
			}

		}
	}
	
	ComPrintf("CFileManager::Open:File not found %s\n", filename);
	return false;
}


/*
=====================================
Loads a File into the given buffer
=====================================
*/
ulong CFileManager::Load(const char *filename, byte ** buffer)
{
	long size = 0;
	CSearchPath * iterator = m_lastpath;
	while(iterator->prev)
	{
		*buffer = 0;
		iterator = iterator->prev;

		//Try opening as an archive
		if(iterator->archive)
		{
			size = iterator->archive->OpenFile(filename,buffer);
			if(size)
				return size;
		}
		//Try opening as a standard file
		else
		{
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s/%s/%s", m_exepath, iterator->path, filename);
			FILE * fp = fopen(filepath,"r+b");
			
			if(fp)
			{
				fseek(fp,SEEK_END,0);
				size = ftell(fp);
				fseek(fp,SEEK_SET,0);

				//fill the file buffer
				*buffer = new unsigned char[size];
				fread(*buffer,size,1,fp);
				fclose(fp);
				return size;
			}

		}
	}
	ComPrintf("CFileManager::Load:File not found %s\n", filename);
	return 0;
}

/*
===========================================
Close the given file
===========================================
*/
bool CFileManager::Close(CFile *file )
{
	if(!file)
	{
		ComPrintf("CFileManager::Close:Error, File does not exist\n");
		return false;
	}

	//Check to see if file matches an entry in our list of openfiles
	for(int i=0;i<COM_MAXOPENFILES;i++)
	{
		if(m_openfiles[i] == file)
			break;
	}

	if(m_openfiles[i] != file)
	{
		ComPrintf("CFileManager::Close:Error,given file ,%s, is not in our list of" 
								"open files\n", file->m_filename);
		return false;
	}

	if(file->m_buffer)
	{
		delete [] file->m_buffer;
		file->m_buffer = 0;
	}
	if(file->m_filename)
	{
		delete file->m_filename;
		file->m_filename = 0;
	}
	
	delete file;
	file = 0;
	m_openfiles[i] = 0;
	return true;
}

/*
===========================================
Lists Added search paths
===========================================
*/
void CFileManager::ListSearchPaths()
{
	if(!m_lastpath)
		return;
	
	ComPrintf("Listing Current Search Paths:\n");

	CSearchPath * iterator = m_lastpath;
	while(iterator->prev)
	{
		iterator = iterator->prev;
		if(iterator->archive)
			ComPrintf("%s\n",	iterator->archive->archivename);
		else
			ComPrintf("%s\n",	iterator->path);
	}
}

/*
===========================================
Lists files in added archives
===========================================
*/
void CFileManager::ListArchiveFiles()
{
	if(!m_lastpath)
		return;

	ComPrintf("Listing Files in Search Paths:\n");

	CSearchPath * iterator = m_lastpath;
	while(iterator->prev)
	{
		iterator = iterator->prev;
		if(iterator->archive)
			iterator->archive->ListFiles();
	}
}

/*
===========================================
Scan a directory for supported archive types
===========================================
*/
bool CFileManager::GetArchivesInPath(const char *path, CStringList * list)
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
			ComPrintf("CFileManager::GetArchivesInPath:Couldnt find %s\n",searchpath);
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
			ComPrintf("CFileManager::Win32_DirectoryList:Unable to close search handle\n");
	}
	list->QuickSortStringList(list,num);
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
void CFileManager::AddSearchPath(const char * path, CArchive * archive)
{
	strcpy(m_lastpath->path, path);
	m_lastpath->archive = archive;	//Will be zero, if a real dir was added

	CSearchPath * newpath = new CSearchPath();
	newpath->prev = m_lastpath;
	m_lastpath = newpath;
}

/*
===========================================
Remove Any SearchPaths
archives, or real dirs, that are in this path
===========================================
*/
void CFileManager::RemoveSearchPath(const char *path)
{
	CSearchPath * iterator = m_lastpath;

	while(iterator)
	{
		//remove this entry if matches
		if(strcmp(iterator->path,path)==0)
		{
			CSearchPath * temp = iterator;
			iterator = iterator->prev;
			delete temp;
			m_lastpath = iterator;
		}
		else
			iterator = iterator->prev;
	}
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







/*	
	//Returns a filelisting matching the criteria	
	int FindFiles(CStringList *filelist,		//File List to fill
				  const char  *ext,				//Extension		  
				  const char  *path=0);			//Search in a specific path
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
			ComPrintf("CFileManager::FindFiles: Invalid Path\n");
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



