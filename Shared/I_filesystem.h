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
class  CArchive;

//====================================================================================
//====================================================================================

struct FILESYSTEM_API I_FileReader
{
	virtual bool Open(const char * filename)=0;
	virtual void Close()=0;
	virtual bool isOpen()=0;
	virtual ulong Read(void * buf, uint size, uint count)=0;
	virtual int  GetChar()=0;
	virtual bool Seek(uint offset, int origin)=0;
	virtual uint GetPos() const =0;
	virtual uint GetSize() const =0;
};


class FILESYSTEM_API CFileBuffer : public I_FileReader
{
public:
	
	CFileBuffer(int bufsize=0);
	virtual ~CFileBuffer();

	//Open the file at the given path
	bool Open(const char * ifilename);
	//Close the currently opened file
	void Close();
	//Do we have a file open right now ?
	bool isOpen();
	//Read "count" number of items of "size" into buffer
	ulong Read(void *buf,uint size, uint count);
	//Return the current byte, advance current position
	int GetChar();
	//Seek to given offset starting from given origin
	bool Seek(uint offset,  int origin);

	uint GetPos() const;
	uint GetSize()const;

private:
	
	uint	m_size;			//File Size
	uint	m_curpos;		//File Position
	
	//Change this so its allocated from a static pool later on
	byte *  m_buffer;		//File data is copied into this buffer in Buffered mode
	uint	m_buffersize;	//Size of Buffer

	char *  m_filename;
};


class FILESYSTEM_API CFileStream : public I_FileReader
{
public:
	
	CFileStream();
	virtual ~CFileStream();

	//Open the file at the given path
	bool Open(const char * ifilename);
	//Close the currently opened file
	void Close();
	//Do we have a file open right now ?
	bool isOpen();
	//Read "count" number of items of "size" into buffer
	ulong Read(void *buf,uint size, uint count);
	//Return the current byte, advance current position
	int GetChar();
	//Seek to given offset starting from given origin
	bool Seek(uint offset,  int origin);

	uint GetPos() const;
	uint GetSize()const;

private:

	friend class CFileSystem;
	
	uint	m_size;			//File Size
	char *  m_filename;

	FILE *  m_fp;			//if its a real file, then use a filestream
	uint    m_filehandle;	//handle to filestream in an archive
	CArchive * m_archive;   //Arhive containing the given file
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

	//Loads data from file into given buffer. return size of buffer
	//after allocation and copying.
	uint LoadFileData(byte ** ibuffer, uint buffersize, const char *ifilename);
	uint OpenFileStream(FILE * ifp, int &ifileHandle, const char *ifilename);
	
	uint GetFileSize(const char * filename);

	//Returns a filelisting matching the criteria
	int FindFiles(CStringList *filelist,		//File List to fill
				  const char  *ext,				//Extension		  
				  const char  *path=0);			//Search in a specific path

private:

	struct SearchPath_t;

	//Member variables
	//================================================================
	int			m_totalfiles;					//total number of Files in the archives
	char		m_exepath[COM_MAXPATH];			//Exe path
	char		m_basedir[COM_MAXPATH];			//Base Dir, IS NOT changed

	int			m_numsearchpaths;				//Number of searchpaths
	SearchPath_t * m_searchpaths;				//DoublyLinked list of searchpaths
	SearchPath_t * m_lastpath;					//Latest path


	//Member Functions
	//================================================================
	void AddSearchPath(const char * path, CArchive * file=0);
	void RemoveSearchPath(const char *path);

	//Scans directory for supported archives, returns list
	bool GetArchivesInPath(const char *path, CStringList * list);

	//Console Funcs
	//================================================================
	friend void CFuncListFiles(int argc, char** argv);

	//MOVE THESE SOMEPLACE MORE APPROPRIATE
	//================================================================
	static bool CompareExts(const char *file, const char *ext);		
	static bool PathExists(const char * path);
};

//====================================================================================

FILESYSTEM_API CFileSystem * CreateFileSystem(I_ExeConsole * pconsole);
FILESYSTEM_API void DestroyFileSystem();

#endif
