#include "Fs_hdr.h"
#include "I_file.h"
#include "I_filesystem.h"
#include "Fs_pakfile.h"
#include "Fs_zipfile.h"
#include "Com_cvar.h"

//======================================================================================
#define CMD_LISTFILES	0
#define CMD_LISTPATHS	1
#define CMD_DIRPATH		2

extern I_Console * g_pConsole;		//pointer to console

//======================================================================================

/*
===========================================
Supported File Types
===========================================
*/
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

	g_pConsole->RegisterCommand("fs_listarchives",CMD_LISTFILES,this);
	g_pConsole->RegisterCommand("fs_path",CMD_LISTPATHS,this);
	g_pConsole->RegisterCommand("fs_dir",CMD_DIRPATH,this);
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
	if(!FileUtil::PathExists(m_exepath))
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
	sprintf(m_curpath,"%s/%s",m_exepath,gamedir);
	if(!FileUtil::PathExists(m_curpath))
	{
		ComPrintf("CFileSystem::AddGameDir: Game dir does not exist : %s\n",m_curpath);
		memset(m_curpath,0, COM_MAXPATH);
		return false;
	}

	//Directoy is Valid. Reset the current GAME directoy before changing to new one
	ResetGameDir();
	//Add to SearchPath
	AddSearchPath(gamedir);

	//Add any archive files into the search path as well
	char		archivepath[COM_MAXPATH];
	StringList	archivelist;

	for(int i=0; archive_exts[i]; i++)
		GetFilesInPath(archivelist,gamedir,archive_exts[i]);
	
	archivelist.sort();
	
	for(std::list<std::string>::iterator itor = archivelist.begin(); itor != archivelist.end(); itor++)
	{
		if(FileUtil::CompareExts(itor->c_str(),"zip"))
		{
			CZipFile * zipfile = new CZipFile();
			sprintf(archivepath,"%s/%s", gamedir,itor->c_str());

			if(zipfile->Init(archivepath, m_exepath))
				AddSearchPath(gamedir,(CArchive*)zipfile);
			else
				delete zipfile;
		}
		//Found a Pak file, try adding
		if(FileUtil::CompareExts(itor->c_str(),"pak"))
		{
			CPakFile * pakfile = new CPakFile();
			sprintf(archivepath,"%s/%s", gamedir,itor->c_str());

			if(pakfile->Init(archivepath, m_exepath))
				AddSearchPath(gamedir,(CArchive*)pakfile);
			else
				delete pakfile;
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
	while(iterator->prev)
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
==========================================
Returns current path
==========================================
*/
const char * CFileSystem::GetCurrentPath() const
{	return m_curpath;
}

/*
===========================================
Loads the requested file into given buffer.
buffer needs to be null. Its allocated here
===========================================
*/
uint CFileSystem::LoadFileData(byte ** ibuffer, uint buffersize, const char *ifilename)
{
	uint size = 0;
	SearchPath_t * iterator = m_lastpath;
	
	while(iterator->prev)
	{
		iterator = iterator->prev;
		//Try opening as an archive
		if(iterator->archive)
		{
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

				if(!buffersize)
				{
					*ibuffer = (byte*)g_pHunkManager->HunkAlloc(size);
				}
				else
				{
					if(size > buffersize)
					{
						ComPrintf("CFileSystem::LoadFileData: Buffer is smaller than size of file %s, %d>%d\n", 
							ifilename, size, buffersize);
						fclose(fp);
						return 0;
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
==========================================
Try to  Open a fileStream
FILE pointer is set if its a real file
otherwise the fileHandle and archive pointers are set
==========================================
*/
uint CFileSystem::OpenFileStream(FILE ** ifp, 
								 int &ifileHandle, CArchive ** iarchive, 
								 const char *ifilename)
{
	SearchPath_t * iterator = m_lastpath;
	uint size = 0;
	
	while(iterator->prev)
	{
		iterator = iterator->prev;
		//Try opening as an archive
		if(iterator->archive)
		{
			int handle = iterator->archive->OpenFile(ifilename);
			if(handle >= 0)
			{
				size = iterator->archive->GetSize(handle);
				ifileHandle = handle;
				*iarchive = iterator->archive;
				return size;
			}
		}
		//Try opening as a standard file
		else
		{
			char filepath[COM_MAXPATH];
			sprintf(filepath,"%s/%s/%s", m_exepath, iterator->path, ifilename);
			
			FILE * fp = fopen(filepath,"r+b");
			if(fp)
			{
				fseek(fp ,0,SEEK_END);
				size = ftell(fp );
				fseek(fp ,0,SEEK_SET);
				
				*ifp = fp;
				ifileHandle = 0;
				return size;
			}
		}
	}
	ComPrintf("CFileSystem::Open:File not found %s\n", ifilename);
	return 0;
}

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
			ComPrintf("%s\n",iterator->archive->m_archiveName);
			iterator->archive->ListFiles();
			ComPrintf("\n");
		}
	}
}

/*
==========================================
Print out files matching the criteria
==========================================
*/
void CFileSystem::ListFiles(const char *path, const char *ext)
{
	if(!m_lastpath)
	{
		ComPrintf("CFileSystem::ListFiles: File System is uninitialized\n");
		return;
	}

	int pathlen = 0;
	StringList		strlist;
	std::list<std::string>::iterator itor;

	char			searchPath[COM_MAXPATH];
	SearchPath_t *	iterator = m_lastpath;
	
	if(path)
		pathlen = strlen(path) + 1;

	//Sort of a hack to get file names right
	//First just go through normal dir paths
	while(iterator->prev)
	{
		iterator = iterator->prev;

		if(iterator->archive)
		{
			iterator->archive->GetFileList(strlist,path,ext);
			if(strlist.size())
			{
				if(ext)
					ComPrintf("%s/*.%s",iterator->archive->m_archiveName,ext);
				else
					ComPrintf("%s/*.*",iterator->archive->m_archiveName);
				ComPrintf("===================================\n");

				for(itor = strlist.begin(); itor != strlist.end(); itor++)
					ComPrintf("%s\n",itor->c_str() + pathlen);
				ComPrintf("\n");
				strlist.clear();
			}
		}
		else
		{	
			if(path)
			{
				sprintf(searchPath,"%s/%s",iterator->path,path);
				GetFilesInPath(strlist,searchPath,ext);
			}
			else
			{	
				strcpy(searchPath,iterator->path);
				GetFilesInPath(strlist,searchPath,ext);
			}
			if(strlist.size())
			{
				if(ext)
					ComPrintf("%s/*.%s",searchPath,ext);
				else
					ComPrintf("%s/*.*",searchPath);
				ComPrintf("===================================\n");

				for(itor = strlist.begin(); itor != strlist.end(); itor++)
					ComPrintf("%s\n",itor->c_str());

			}
			ComPrintf("\n");
			strlist.clear();
		}
		
	}
}


/*
==========================================
Find the full name of a file at the given path
==========================================
*/
bool CFileSystem::FindFileName(char * buf, int buflen, const char * path)
{
	if(!m_lastpath)
	{
		ComPrintf("CFileSystem::ListFiles: File System is uninitialized\n");
		return false;
	}

	WIN32_FIND_DATA	finddata;
	HANDLE hsearch;
	int pathlen = strlen(path);
	char searchpath[COM_MAXPATH];
	bool found = false;
	
	SearchPath_t *	iterator = m_lastpath;
	
	while(iterator->prev)
	{
		iterator = iterator->prev;
		if(iterator->archive)
		{
			found = iterator->archive->FindFile(buf,buflen,path);
			if(found == true)
				return true;
		}
		else
		{
			sprintf(searchpath,"%s/%s/%s*", m_exepath, iterator->path, path);
ComPrintf("Searching in %s\n",searchpath);
			
			hsearch= FindFirstFile(searchpath,&finddata);
			if(hsearch != INVALID_HANDLE_VALUE)
			{
				char ext[8];
				FileUtil::ParseExtension(finddata.cFileName,ext,8);

				if(FindClose(hsearch) == FALSE)   // file search handle
					ComPrintf("CFileSystem::FindFileName:Unable to close search handle\n");

				if(strlen(ext) + pathlen < buflen)
				{
					sprintf(buf,"%s.%s", path,ext);
					return true;
				}
			}
			
		}
	}
	return false;
}


/*
==========================================
Console Command Hanlder
==========================================
*/
void CFileSystem::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case CMD_LISTFILES:
		{
			ListArchiveFiles();
			break;
		}
	case CMD_LISTPATHS:
		{
			ListSearchPaths();
			break;
		}
	case CMD_DIRPATH:
		{
			if(numArgs == 2)
				ListFiles(szArgs[1],0);
			else if(numArgs == 3)
				ListFiles(szArgs[1], szArgs[2]);
			else
				ListFiles(0,0);
			break;
		}
	}
}

/*
===========================================
Scan a directory for supported archive types
===========================================
*/
bool CFileSystem::GetFilesInPath(StringList &strlist, const char *path, const char *ext)
{
	WIN32_FIND_DATA	finddata;
	char	searchpath[COM_MAXPATH];
	bool	finished = false;

	if(ext)
		sprintf(searchpath,"%s/%s/*.%s",m_exepath,path,ext);
	else
		sprintf(searchpath,"%s/%s/*.*",m_exepath,path);

	HANDLE hsearch = FindFirstFile(searchpath,&finddata);
	if(hsearch == INVALID_HANDLE_VALUE)
	{
		ComPrintf("CFileSystem::GetArchivesInPath: Couldnt find %s\n",searchpath);
		return false;
	}

	while(!finished)
	{
//		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		strlist.push_back(std::string(finddata.cFileName));

		if (!FindNextFile(hsearch, &finddata))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
				finished = true;
		}
	}
	if(FindClose(hsearch) == FALSE)   // file search handle
		ComPrintf("CFileSystem::Win32_DirectoryList:Unable to close search handle\n");
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


//======================================================================================
//======================================================================================
//Not sure