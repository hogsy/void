#include "Com_filesys.h"
#include <windows.h>		//need this for Win32 find functions

void dprintf(char *string,...);

int  CFileManager::totalfiles;
int	 CFileManager::totaldirs;


/*
==============================================================================================
Directory Class
==============================================================================================
*/

char CDirectory::m_basedir[MAXFILEPATH];

/*
==========================================
Constructor and Destructor
==========================================
*/
CDirectory::CDirectory()
{
	m_dir = 0;
	m_files = 0;

	m_numfiles = 0;
	m_numdirs = 0;
	pathlen =0;
}

CDirectory::~CDirectory()
{
	if(m_dir)
		delete [] m_dir;
	if(m_files)
		delete [] m_files;
}


/*
==========================================
Start Recursively loading a directory tree
==========================================
*/
bool CDirectory::Init(const char* base)
{
	if(!base)
		return false;
	
	//save path info, we'll need this to use the Win32 functions
//	int len = strlen(base) -1;
	memset(m_basedir,0,MAXFILEPATH);
	strcpy(m_basedir,base);
	
	//make sure the last char isnt a \\ because we'll add that ourselves in the first scan
//	if(!((m_basedir[len] == '\\') || (m_basedir[len] == '/')))
//		strcat(m_basedir,"\\");

	return Load(0);
}


/*
==========================================
Release
Release everything under this dir
==========================================
*/

void CDirectory::Release()
{
	if(m_dir)
	{
		delete [] m_dir;
		m_dir = 0;
	}
	if(m_files)
	{
		delete [] m_files;
		m_files = 0;
	}
	m_numfiles = 0;
	m_numdirs = 0;
}


/*
==========================================
Recursively load a directory tree

A directories internal name will just be models, sounds etc to speed up searching
However the scan function needs to append /*.* after each name, and add the base dir path
before the name to get a list of files
==========================================
*/

bool CDirectory::Load(const char* name)
{
	if(name)
	{
		strcpy(path,name);
		pathlen = strlen(path);
	}
	else
	{
		memset(path,0,MAXFILEPATH);
		pathlen = 0;
	}

	CItemList *dirlist= new CItemList; 
	CItemList *filelist= new CItemList;
		
	//Scan the number of subdirs and files
	if(!Scan(dirlist,filelist))
	{
		delete dirlist;
		delete filelist;
		return false;
	}

	//Sort Files and Allocate space
	if(m_numfiles)
	{
		QuickSortList(filelist,m_numfiles);
		m_files = new CFile[m_numfiles];
	}
	if(m_numdirs)
	{
		QuickSortList(dirlist,m_numdirs);
		m_dir   = new CDirectory[m_numdirs];
	}

	CItemList *iterator=0;
	int i=0;

	//add file names
	iterator = filelist;
	for(i=0;i<m_numfiles;i++)
	{
		m_files[i].SetName(iterator->name);
		iterator = iterator->next;
		CFileManager::totalfiles++;
	}

	//add dirs
	iterator = dirlist;
	for(i=0;i<m_numdirs;i++)
	{
		m_dir[i].Load(iterator->name);		//Recurse and load the subdirectories here until there are no more
		iterator = iterator->next;
		CFileManager::totaldirs++;		
	}

	iterator = 0;
	delete dirlist;
	delete filelist;
	return true;
}


/*
==========================================
Scan the directory to find out number
of files and subdirectores and adds their names to the itemlist
does not recurse
==========================================
*/

bool CDirectory::Scan(CItemList *dirlist, 
					  CItemList *filelist)
{
	WIN32_FIND_DATA	finddata;
	HANDLE	hsearch;
	bool	finished = false;
	CItemList	*dir = dirlist;
	CItemList	*file = filelist;

	m_numdirs = 0;
	m_numfiles = 0;

	char temp [MAXFILEPATH];
	strcpy(temp,m_basedir);
	if(pathlen)
		strcat(temp,path);
	strcat(temp,"*.*");

	hsearch = FindFirstFile(temp,&finddata);
	if(hsearch == INVALID_HANDLE_VALUE)
		return false;;

	while(!finished)
	{
		if(dir && finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(finddata.cFileName[0] != '.')
			{
				if(pathlen)
					strcpy(dir->name,path);	
				strcat(dir->name,finddata.cFileName);
				strcat(dir->name,"/");
				dir->next = new CItemList();
				dir = dir->next;
				m_numdirs ++;
			}
		}
		else
		{
			strcpy(file->name,finddata.cFileName);
			file->next = new CItemList();
			file = file->next;
			m_numfiles ++;
		}

		if (!FindNextFile(hsearch, &finddata))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{	finished = true;
			}
		}
	}

	if(FindClose(hsearch) == FALSE)   // file search handle
	{	dprintf("Scan:Unable to close search handle\n");
	}
	return true;
}

/*
==========================================
Sort the file or dirrectory list
using the Quick Sort algorithm

using _stricmp right now, Win9x,NT

FIXME- really need to change this, so it doesnt have to allocate memory on every pass.
setup one buffer on the start of the sort and just use that.
should get a 50% speed increase
==========================================
*/
void CDirectory::QuickSortList(CItemList * const list, const int numitems)
{
	if(numitems < 2)
		return;

	CItemList *	sorted = new CItemList[numitems];		//dest array
	CItemList * pivot = list;							//let the first one be the pivot
	CItemList *	iterator = list->next;					
	
	int maxindex = numitems-1;
	int left=0;
	int right = maxindex; 
	
	//loop one less time since the first item is the pivot
	for(int i=0,comp=0;i<maxindex;i++)
	{
		comp = _stricmp(iterator->name,pivot->name);
		
		if(comp < 0)
		{
			strcpy(sorted[left].name,iterator->name);
			sorted[left].next = &sorted[(left+1)];
			left++;
		}
		else if(comp >= 0)
		{
			strcpy(sorted[right].name,iterator->name);
			sorted[(right-1)].next = &sorted[right];
			right--;
		}
		iterator = iterator->next;
	}
	
	//Copy the pivot point in the empty space
	strcpy(sorted[left].name, pivot->name);
	if(right == maxindex)
		sorted[left].next = 0;
	else
		sorted[left].next = &sorted[(left+1)];
		

	if(left > 1) 
		QuickSortList(sorted,left);								
	if((numitems - (right+1)) > 1)
		QuickSortList(&sorted[left+1],(numitems - (right+1)));		//starting from the one right after the pivot
	
	if((left <= 1) && (right <= 1))
	{
		iterator = list;
		for(i=0;i<numitems;i++)
		{
			strcpy(iterator->name,sorted[i].name);
			iterator=iterator->next;
			sorted[i].next = 0;						//get rid of the links so we dont have problems deleting the array
		}
		delete [] sorted;
		return;
	}

	//everything is sorted now copy it over
	iterator = list;
	for(i=0;i<numitems;i++)
	{
		strcpy(iterator->name,sorted[i].name);
		iterator=iterator->next;
		sorted[i].next = 0;						//get rid of the links so we dont have problems deleting the array
	}
	delete [] sorted;
}


/*
==========================================
Compare Paths
compares the directory paths in 2 strings
returns the character upto which they are equal
0 if they are not equal

used to get the directory names one by one from a filepath
so that they can be compared with dir names in the list
==========================================
*/

bool CDirectory::ComparePaths(const char *file, const char *path)
{
	if(!file || !path)
		return false;

	const char *p = file;
	const char *q = path;

	while(*p && *q &&
		 (*q != '\\' && *q != '/') &&	//hit a directory slash
		 (*q == *p))
	{
		p++; q++;
	}
	
	if(*p && *q && *p == *q)	//we stopped at a directory slash earlier
		return true;
	return false;;
}

/*
==========================================
Binary Search for a file
==========================================
*/
CFile * CDirectory::BinarySearchForFile(const char *name,				
										CFile * array, 
										int low, int high)
{
	//Array doenst have any items
	if(low > high)
		return 0;
	
	int mid = (low+high)/2;
	int ret = _stricmp(name,array[mid].filename);

	if(ret == 0)		//name is equal to entry in array
		return &array[mid];
	else if(ret > 0)	//name is greater than array entry
		return BinarySearchForFile(name,array,mid,high);
	else if(ret < 0)	//name is less than array entry
		return BinarySearchForFile(name,array,low,mid);
	return 0;
}


/*
==========================================
Find the requested file in 
==========================================
*/

CFile * CDirectory::GetFile(const char *name)
{
	//See if its in one of our directories
	for(int i=0;i<m_numdirs;i++)
	{
		if(ComparePaths(name + pathlen,m_dir[i].path + pathlen))
			return m_dir[i].GetFile(name);
	}
	//didnt find any directory matching this name, is it a file ?
	return BinarySearchForFile(name + pathlen,m_files,0,m_numfiles);
}

/*
==========================================
Find the requested file and update the directory
it was found in
==========================================
*/
CFile * CDirectory::GetFile(const char *name, CDirectory ** dir)
{
	//See if its in one of our directories
	for(int i=0;i<m_numdirs;i++)
	{
		if(ComparePaths(name + pathlen,m_dir[i].path + pathlen))
			return m_dir[i].GetFile(name,dir);
	}
	
	//didnt find any directory matching this name, is it a file ?
	CFile *f = BinarySearchForFile(name + pathlen,m_files,0,m_numfiles);
	if(f)
	{
		*dir = this;
		return f;
	}
	return 0;
}


/*
==========================================
Print out the current dir and the loaded files
==========================================
*/
void CDirectory::Print()
{
	int i;
	dprintf("================================\n%s\n",path);
	for(i=0; i<m_numfiles;i++)
			dprintf("%s\n",m_files[i].filename);

	for(i=0;i<m_numdirs;i++)
	{
		dprintf("%s\n",m_dir[i].path);
		m_dir[i].Print();
	}
}

/*
==========================================
Compare file Extensions
return true if equa
==========================================
*/
bool CDirectory::CompareExts(const char *file, const char *ext)		
{
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
==========================================
Scan files
==========================================
*/

bool CDirectory::ScanFiles(CItemList **filelist, 
						   int &numfiles,
						   const char *pattern)
{
	WIN32_FIND_DATA	finddata;
	HANDLE	hsearch;
	bool	finished = false;
	char	temp [MAXFILEPATH];
//	CItemList * file = *filelist;
	
	strcpy(temp,m_basedir);
	if(pathlen)
		strcat(temp,path);
	strcat(temp,pattern);

	hsearch = FindFirstFile(temp,&finddata);
	if(hsearch == INVALID_HANDLE_VALUE)
		return false;

	while(!finished)
	{
		if(!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			numfiles ++;
			if(pathlen)
				strcpy((*filelist)->name,path);
			strcat((*filelist)->name,finddata.cFileName);
			(*filelist)->next = new CItemList();
			(*filelist) = (*filelist)->next;
		}

		if (!FindNextFile(hsearch, &finddata))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{	finished = true;
			}
		}
	}

	if(FindClose(hsearch) == FALSE)   // file search handle
	{	dprintf("Scan:Unable to close search handle\n");
	}
	return true;
}


/*
==========================================
Copy the names+paths of all the files matching this pattern

==========================================
*/

void CDirectory::CreateFileList(const char *ext, CItemList **filelist, int &numfiles)
{
	ScanFiles(filelist,numfiles,ext);
	for(int i=0; i< m_numdirs; i++)
		m_dir[i].CreateFileList(ext,filelist,numfiles);
}

TFilePath * CDirectory::FindFiles(const char *ext, int  &numfiles)
{
	int		   nfiles=0;
	int		   i=0;
	
	CItemList *filelist = new CItemList;
	CItemList *iterator= filelist;

	CreateFileList(ext,&filelist,nfiles);
	
	TFilePath * fnames = new TFilePath[nfiles]; 
	CItemList * top = iterator;

	for(i=0;i<nfiles;i++)
	{
		strcpy(fnames[i],iterator->name);
//		dprintf("Print :%s\n",fnames[i]);
		iterator = iterator->next;
	}
	numfiles = nfiles;
//	dprintf("Num files found %d with ext %s\n", nfiles, ext);
	delete top;
	return fnames; 
}


/*
==============================================================================================
File Manager
==============================================================================================
*/

/*
==========================================
Constructor and Destructor
==========================================
*/

CFileManager::CFileManager()
{
	m_pbasedir = 0;
	totalfiles=0;
	totaldirs=0;
}

CFileManager::CFileManager(const char *basepath)
{
	int len = strlen(basepath) -1;
	strcpy(m_basedir,basepath);
	
	//make sure the last char is a \\ 
	if(!((basepath[len]  != '\\') || (basepath[len] != '/')))
		strcat(m_basedir,"/");
	
	m_pbasedir = 0;
	
	totalfiles=0;
	totaldirs=0;
}


CFileManager::~CFileManager()
{
	if(m_pbasedir)
		delete m_pbasedir;
}


/*
==========================================
Initialize the File manager
build the directory tree and filelist
==========================================
*/

bool CFileManager::Init(const char *basedir,	//specific base dir, or use current
						const char ** ext)		//pointer to list of file extensions to add
{
	if(basedir)
	{
		int len = strlen(basedir) - 1;
		
		//get rid of the leading \\ or /
		if((basedir[0] == '\\') || (basedir[0] == '/'))
			strcat(m_basedir,basedir+1);
		else
			strcat(m_basedir,basedir);

		if(!((basedir[len] == '\\') || (basedir[len] == '/')))
			strcat(m_basedir,"\\");
	}
	
	m_pbasedir = new CDirectory();
	if(!m_pbasedir->Init(m_basedir))	//This will recursively load the entire tree beneath the specified dir
	{
		dprintf("Error loading directory : %s\n", m_basedir);
		return false;
	}
	return true;
}

/*
==========================================
Destory everything
==========================================
*/

bool CFileManager::Shutdown()
{
	if(m_pbasedir)
		delete m_pbasedir;

	memset(m_basedir,0,MAXFILEPATH);
	m_pbasedir = 0;
	
	totalfiles=0;
	totaldirs=0;
	return true;
}


/*
==========================================
Print out the list of files and dirs
==========================================
*/

void CFileManager::ListFiles()
{
	m_pbasedir->Print();
	
	dprintf("Total files added :%d\n",totalfiles);
	dprintf("Total dirs  added :%d\n",totaldirs);
}



/*
==========================================
returns a pointer to the requested file
There should NOT be a leading \\ in the given filepath
==========================================
*/

CFile * CFileManager::GetFile(const char *file)			//return file pointer to this file
{	
	if(!file)
		return 0;
	if((file[0] == '\\') || (file[0] == '/'))
		return m_pbasedir->GetFile(file+1);
	return  m_pbasedir->GetFile(file);

/*	CFile * test = m_pbasedir->GetFile(file);
	dprintf("Found %s\n", test->filename);
	return test;
*/	
}

/*
==========================================
Open a file and return it
==========================================
*/

CFile * CFileManager::OpenFile(const char *file, const char *mode)
{
	CFile *f;
	CDirectory *fdir;
	char filepath[MAXFILEPATH];

	if(!file)
		return 0;
	if((file[0] == '\\') || (file[0] == '/'))
		f = m_pbasedir->GetFile(file+1, &fdir);
	f = m_pbasedir->GetFile(file, &fdir);
	
	if(f)
	{
		strcpy(filepath,m_basedir);
		if(fdir->path)
			strcat(filepath,fdir->path);
		f->Open(filepath,mode);
		return f;
	}
	return 0;
}


/*
==========================================
make a listing of all the files 
in the given pattern
==========================================
*/

TFilePath *  CFileManager::FindFiles(const char *ext,
									 int   &numfiles,
									 const char *path)
{
	if(!ext)// || *fnames)
		return 0;
	if(!path)
		return m_pbasedir->FindFiles(ext, numfiles);

	//otherwise start from the specified dir

	return 0;

}




