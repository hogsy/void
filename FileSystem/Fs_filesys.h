#ifndef VOID_FILESYSTEM_MAIN
#define VOID_FILESYSTEM_MAIN

#include "I_filesystem.h"

class CArchive;
class CFileStream;
class CFileBuffer;

/*
================================================
FileSystem Implementation
================================================
*/

class CFileSystem : public I_FileSystem
{
public:
	CFileSystem(const char * exedir, 
				const char *basedir);
	
	~CFileSystem();

	//Create a file Reader
	I_FileReader * CreateReader(EFileMode mode);

	//Check after creating it the first time
	bool IsActive() const;

	//Set current "Game" directory for over-ridable content
	bool AddGameDir(const char *dir);

	//Remove any additional game dirs. reset to basedir
	void ResetGameDir();

	//List Current Search Paths
	void ListSearchPaths();

	//Print out the list of files in added archives
	void ListArchiveFiles();
	void ListFiles(const char *path, const char *ext);

	//Finds the full file name and returns it
	bool FindFileName(char * buf, int buflen, const char * path);

	//Returns current path EXE+Game
	const char * GetCurrentPath() const;

	//Opem a fileStream. Will either set the FILE pointer, 
	//or update the handle+archive pointers
	uint OpenFileReader(CFileStream * pFile, const char *ifilename);

	//Loads data from file into given buffer. return size of buffer
	//after allocation and copying.
	uint OpenFileReader(CFileBuffer * pFile, const char *ifilename);

private:

	//Private Member variables
	//================================================================
	int		m_totalfiles;					//total number of Files in the archives
	char	m_exepath[COM_MAXPATH];			//Exe path
	char	m_basedir[COM_MAXPATH];			//Base Dir, IS NOT changed
	char	m_curpath[COM_MAXPATH];

	bool	m_bActive;
	int		m_numsearchpaths;				//Number of searchpaths

	struct SearchPath_t;
	SearchPath_t * m_searchpaths;			//DoublyLinked list of searchpaths
	SearchPath_t * m_lastpath;				//Latest path

	//Private Member Functions
	//================================================================

	void AddSearchPath(const char * path, CArchive * file=0);
	void RemoveSearchPath(const char *path);

	//Scans directory for supported archives, returns list
	bool GetFilesInPath(StrList &strlist, const char *path, const char *ext);
};

#endif