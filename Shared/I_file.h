#ifndef INC_FILE_INTERFACE
#define INC_FILE_INTERFACE

#ifdef FILESYSTEM_EXPORTS
#define FILESYSTEM_API __declspec(dllexport)
#else
#define FILESYSTEM_API __declspec(dllimport)
#endif

#include "Com_defs.h"

//====================================================================================

struct I_FileReader
{
	virtual bool Open(const char * filename)=0;
	virtual void Close()=0;
	virtual bool isOpen()const =0;
	virtual uint Read(void * buf, uint size, uint count)=0;
	virtual int  GetChar()=0;
	virtual void GetToken(char *buff, bool newline) =0;
	virtual bool Seek(int offset, int origin)=0;
	virtual uint GetPos() const =0;
	virtual uint GetSize() const =0;
	virtual const char * GetFileName() const= 0;
};

//====================================================================================
//Not all of these really need to be here, move them to ComUtil ?
//Only FindFileExtension is REALLY needed to be exported.

namespace FileUtil
{
	FILESYSTEM_API bool FindFileExtension(char * ext, int extlen, const char * path);
	FILESYSTEM_API void ConfirmDir(char* dir);
	FILESYSTEM_API bool PathExists(const char * path);
}

//====================================================================================
/*
==========================================
FileBuffer Reader Class
==========================================
*/

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
	bool isOpen() const;
	
	//Read "count" number of items of "size" into buffer
	uint Read(void *buf,uint size, uint count);
	
	//Return the current byte, advance current position
	int GetChar();
	void GetToken(char *buff, bool newline);
	//Seek to given offset starting from given origin
	bool Seek(int offset, int origin);

	uint GetPos() const;
	uint GetSize()const;
	const char * GetFileName() const;
	byte * GetData() const;

private:

	char *  m_filename;
	uint	m_size;			//File Size
	uint	m_curpos;		//File Position
	
	//Change this so its allocated from a static pool later on
	byte *  m_buffer;		//File data is copied into this buffer in Buffered mode
	uint	m_buffersize;	//Size of Buffer
};


//====================================================================================
//Declaration to reduce dependencies.
class CArchive;

/*
==========================================
FileStream Reader class
==========================================
*/

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
	bool isOpen() const;

	//Read "count" number of items of "size" into buffer
	uint Read(void *buf,uint size, uint count);
	//Return the current byte, advance current position
	int GetChar();
	void GetToken(char *buff, bool newline);
	//Seek to given offset starting from given origin
	bool Seek(int offset,  int origin);

	uint GetPos() const;
	uint GetSize()const;
	const char * GetFileName() const;

private:

	uint	m_size;			//File Size
	char *  m_filename;

	FILE *  m_fp;			//if its a real file, then use a filestream
	int     m_filehandle;	//handle to filestream in an archive
	CArchive * m_archive;   //Arhive containing the given file
};


//====================================================================================
//C interface to file streams

extern "C"
{
	FILESYSTEM_API uint FS_Open(char *name);
	FILESYSTEM_API void FS_Close(uint handle);
	FILESYSTEM_API int  FS_Read(void *buffer,int size,uint handle);
	FILESYSTEM_API void FS_Seek(uint handle,int offset, signed char mode);
	FILESYSTEM_API int  FS_Tell(uint handle);
}



#endif