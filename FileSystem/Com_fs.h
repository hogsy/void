#ifndef VOID_COM_FILE
#define VOID_COM_FILE

#ifdef FILESYSTEM_EXPORTS
#define FILESYSTEM_API __declspec(dllexport)
#else
#define FILESYSTEM_API __declspec(dllimport)
#endif

#include "Com_defs.h"
#include "Com_list.h"


FILESYSTEM_API bool InitFileSystem();
FILESYSTEM_API bool ShutdownFileSystem();

//====================================================================================
//====================================================================================

//Pre-declarations
class CFileSystem;
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
			   const ulong &size, 
			   const ulong &count);
	
	//Return the current byte, advance current position
	int GetChar();

	//Seek to given offset starting from given origin
	bool Seek(const ulong &offset, 
			  EFilePos origin);

	//Info
	ulong GetPos() const { return m_curpos;}
	ulong GetSize()const { return m_size;  } 

	//Static Utility functions

	//Try to load the given file into the given buffer
	//Return size of buffer after allocation/copying
	//Buffer has to be null. its allocated to proper size
	static int LoadFile(void **buf, const char * ifilename);

	//Set the static filesystem object the class uses to find/read files
	static void SetFileSystem(CFileSystem * pfs){ m_pFileSystem = pfs; }

private:

	//CFile class talks to filesystem to delegate opening/closing files
	static CFileSystem * m_pFileSystem;

	ulong	m_buffersize;
	ulong	m_size;		//File Size
	ulong	m_curpos;	//File Position

	byte *  m_buffer;	//Buffer can hold file data
	char *	m_filename;	//Current filename
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
	ulong LoadFile(byte ** ibuffer, const char * ifilename);

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



/*
===========================================
Abstract Base class defining interface
for other Archive handling classes
===========================================
*/
class CArchive
{
public:
	//Load a listing of files in the archive, and order them
	virtual bool Init(const char * archivepath, const char * basepath)=0;

	//Open file at this path, fill buffer
	virtual long OpenFile(const char* filename, byte ** buffer)=0;

	//Print file listing
	virtual void ListFiles()=0;

	//Return list of files
	virtual bool GetFileList (CStringList * list) = 0;

	virtual	~CArchive() { }

	char	m_archiveName[COM_MAXPATH];
	int		m_numFiles;
};


#endif




