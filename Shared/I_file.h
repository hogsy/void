#ifndef INC_FILE_INTERFACE
#define INC_FILE_INTERFACE

#ifdef FILESYSTEM_EXPORTS
#define FILESYSTEM_API __declspec(dllexport)
#else
#define FILESYSTEM_API __declspec(dllimport)
#endif

#include "Com_defs.h"

//====================================================================================
//Declarations to reduce dependencies.
class CArchive;

//====================================================================================

struct I_FileReader
{
	virtual bool Open(const char * filename)=0;
	virtual void Close()=0;
	virtual bool isOpen()const =0;
	virtual uint Read(void * buf, uint size, uint count)=0;
	virtual int  GetChar()=0;
	virtual bool Seek(uint offset, int origin)=0;
	virtual uint GetPos() const =0;
	virtual uint GetSize() const =0;
};

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
	//Seek to given offset starting from given origin
	bool Seek(uint offset,  int origin);

	uint GetPos() const;
	uint GetSize()const;
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
	//Seek to given offset starting from given origin
	bool Seek(uint offset,  int origin);

	uint GetPos() const;
	uint GetSize()const;

private:

	uint	m_size;			//File Size
	char *  m_filename;

	FILE *  m_fp;			//if its a real file, then use a filestream
	int     m_filehandle;	//handle to filestream in an archive
	CArchive * m_archive;   //Arhive containing the given file
};


#endif