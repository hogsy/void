#ifndef VOID_FILESYSTEM_MAIN
#define VOID_FILESYSTEM_MAIN

#include "I_filesystem.h"


class  CArchive;

class CFileSystem : public I_FileSystem
{
public:
	CFileSystem(const char * exedir, const char *basedir);
	~CFileSystem();

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

	friend class CFileStream;
	friend class CFileBuffer;


	void AddSearchPath(const char * path, CArchive * file=0);
	void RemoveSearchPath(const char *path);

	//Scans directory for supported archives, returns list
	bool GetFilesInPath(StringList &strlist, const char *path, const char *ext);

	uint OpenFileStream(FILE ** ifp, 
						int &ifileHandle, 
						CArchive ** iarchive, 
						const char *ifilename);

	//Loads data from file into given buffer. return size of buffer
	//after allocation and copying.
	uint LoadFileData(byte ** ibuffer, 
					  uint buffersize, 
					  const char *ifilename);
};

#endif