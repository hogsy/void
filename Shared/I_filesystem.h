#ifndef INC_FILESYSTEM_INTERFACE
#define INC_FILESYSTEM_INTERFACE

#ifdef FILESYSTEM_EXPORTS
#define FILESYSTEM_API __declspec(dllexport)
#else
#define FILESYSTEM_API __declspec(dllimport)
#endif

#include "Com_defs.h"
#include "Com_list.h"
#include "I_console.h"

//====================================================================================
//====================================================================================

//Pre-declarations
class FILESYSTEM_API CFileSystem;
class CArchive;

//====================================================================================
//====================================================================================

/*
==========================================
The Resource File class
Used for Buffered Binary Input 
==========================================
*/
class FILESYSTEM_API CFileReader
{
public:

	enum EFilePos
	{
		EFILE_START,
		EFILE_CUR,
		EFILE_END
	};
	
	CFileReader();	
	~CFileReader();

	//Open the file at the given path
	bool Open(const char * ifilename);

	//Close the currently opened file
	bool Close();

	//Do we have a file open right now ?
	bool isOpen();

	//Read "count" number of items of "size" into buffer
	ulong Read(void *buf, 
			   const uint &size, 
			   const uint &count);
	
	//Return the current byte, advance current position
	int GetChar();

	//Seek to given offset starting from given origin
	bool Seek(const uint &offset,  EFilePos origin);

	void LockStaticBuffer(const uint &size);
	void ReleaseStaticBuffer();

	//Info
	uint GetPos() const { return m_curpos;}
	uint GetSize()const { return m_size;  } 

	//Static functions for quicker access/ avoid memory copying if all we need
	//is just a buffer full of the requested files data.
	static uint GetFileSize(const char * filename);
	static bool LoadFile(void *buf, const uint &bufsize, const char * filename);

private:

	//CFile class talks to filesystem to delegate opening/closing files
	static CFileSystem * m_pFileSystem;

	uint	m_buffersize;	//Current size of buffer
	bool    m_staticbuffer;	//Is it using a static buffer

	uint	m_size;			//File Size
	uint	m_curpos;		//File Position

	byte *  m_buffer;		//Buffer can hold file data
	char *	m_filename;		//Current filename
};


//====================================================================================
//====================================================================================

/*
==========================================
The File Manager
-maintains the Searchpaths with precedence
-loads supported archive files
-loads actual file data (zip/pak/real)
 into given buffers
==========================================
*/

class FILESYSTEM_API CFileSystem
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

	//Returns a filelisting matching the criteria
	int FindFiles(CStringList *filelist,		//File List to fill
				  const char  *ext,				//Extension		  
				  const char  *path=0);			//Search in a specific path

private:

	friend class CFileReader;

	struct SearchPath_t;

	//Member Functions
	//================================================================

	//Loads data from file into given buffer. return size of buffer
	//after allocation and copying.
	uint LoadFile(byte ** ibuffer, uint &buffersize, bool staticbuffer, const char *ifilename);
	uint GetFileSize(const char * filename);

	void AddSearchPath(const char * path, CArchive * file=0);
	void RemoveSearchPath(const char *path);

	//Scans directory for supported archives, returns list
	bool GetArchivesInPath(const char *path, CStringList * list);

	static bool PathExists(const char * path);

	//FIXME, MOVE THIS SOMEPLACE MORE APPROPRIATE
	//Compare file Extensions
	static bool CompareExts(const char *file, const char *ext);		

	//Member variables
	//================================================================
	int			m_totalfiles;					//total number of Files in the archives
	char		m_exepath[COM_MAXPATH];			//Exe path
	char		m_basedir[COM_MAXPATH];			//Base Dir, IS NOT changed

	int			m_numsearchpaths;				//Number of searchpaths
	SearchPath_t * m_searchpaths;				//DoublyLinked list of searchpaths
	SearchPath_t * m_lastpath;					//Latest path
};

//====================================================================================

FILESYSTEM_API CFileSystem * CreateFileSystem(I_ExeConsole * pconsole);
FILESYSTEM_API void DestroyFileSystem();

#endif