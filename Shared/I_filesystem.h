#ifndef INC_FILESYSTEM_INTERFACE
#define INC_FILESYSTEM_INTERFACE

#ifdef FILESYSTEM_EXPORTS
#define FILESYSTEM_API __declspec(dllexport)
#else
#define FILESYSTEM_API __declspec(dllimport)
#endif

#include "Com_defs.h"
#include "I_void.h"

//====================================================================================
//Pre-declarations
class  CArchive;

//====================================================================================

/*
==========================================
The File Manager
maintains the Searchpaths with precedence
==========================================
*/

class FILESYSTEM_API CFileSystem : public I_ConHandler
{
public:
	CFileSystem();
	~CFileSystem();

	//Intialize the File system using this dir as the root
	bool Init(const char *exedir, const char * basedir);				

	//Set current "Game" directory for over-ridable content
	bool AddGameDir(const char *dir);

	//Remove any additional game dirs. reset to basedir
	void ResetGameDir();

	//List Current Search Paths
	void ListSearchPaths();

	//Print out the list of files in added archives
	void ListArchiveFiles();
	void ListFiles(const char *path, const char *ext);

	//Handle Console Commands
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms) { return false; } 

	//Finds the full file name and returns it
	bool FindFileName(char * buf, int buflen, const char * path);

	//Returns current path EXE+Game
	const char * GetCurrentPath() const;

private:

	friend class CFileStream;
	friend class CFileBuffer;

	uint OpenFileStream(FILE ** ifp, 
						int &ifileHandle, 
						CArchive ** iarchive, 
						const char *ifilename);

	//Loads data from file into given buffer. return size of buffer
	//after allocation and copying.
	uint LoadFileData(byte ** ibuffer, 
					  uint buffersize, 
					  const char *ifilename);

	//================================================================
	struct SearchPath_t;

	//Private Member variables
	//================================================================
	int			m_totalfiles;					//total number of Files in the archives
	char		m_exepath[COM_MAXPATH];			//Exe path
	char		m_basedir[COM_MAXPATH];			//Base Dir, IS NOT changed
	char		m_curpath[COM_MAXPATH];

	int			m_numsearchpaths;				//Number of searchpaths
	SearchPath_t * m_searchpaths;				//DoublyLinked list of searchpaths
	SearchPath_t * m_lastpath;					//Latest path

	//Private Member Functions
	//================================================================
	void AddSearchPath(const char * path, CArchive * file=0);
	void RemoveSearchPath(const char *path);

	//Scans directory for supported archives, returns list
	bool GetFilesInPath(StringList &strlist, const char *path, const char *ext);
};

//====================================================================================

FILESYSTEM_API CFileSystem * FILESYSTEM_Create(I_Void * vexp);
FILESYSTEM_API void FILESYSTEM_Free();

#endif
