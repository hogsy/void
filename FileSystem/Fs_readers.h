#ifndef VOID_FS_FILEREADER_H
#define VOID_FS_FILEREADER_H

#include "I_file.h"

/*
================================================
Buffered Reader
================================================
*/
class CFileBuffer : public I_FileReader
{
	friend class CFileSystem;

public:
	
	CFileBuffer();
	virtual ~CFileBuffer();

	//Open the file at the given path
	bool Open(const char * ifilename);
	
	//Close the currently opened file
	void Close();

	void Destroy()
	{	delete this;
	}
	
	//Do we have a file open right now ?
	bool isOpen() const;
	
	//Read "count" number of items of "size" into buffer
	uint Read(void *buf,uint size, uint count);
	
	//Return the current byte, advance current position
	int GetChar(bool perror);
	void GetToken(char *buff, bool newline);
	
	//Seek to given offset starting from given origin
	bool Seek(int offset, int origin);

	uint GetPos() const;
	uint GetSize()const;
	byte * GetBuffer() const;
	const char * GetFileName() const;
	
private:

	char *  m_filename;
	uint	m_size;			//File Size
	uint	m_curpos;		//File Position
	
	//Change this so its allocated from a static pool later on
	byte *  m_buffer;		//File data is copied into this buffer in Buffered mode
	uint	m_buffersize;	//Size of Buffer
};



/*
==========================================
FileStream Reader class
==========================================
*/
class CArchive;

class CFileStream : public I_FileReader
{
	friend class CFileSystem;

public:
	
	CFileStream();
	virtual ~CFileStream();

	//Open the file at the given path
	bool Open(const char * ifilename);
	//Close the currently opened file
	void Close();
	//Do we have a file open right now ?
	bool isOpen() const;

	void Destroy()
	{	delete this;
	}

	//Read "count" number of items of "size" into buffer
	uint Read(void *buf,uint size, uint count);
	//Return the current byte, advance current position
	int GetChar(bool perror);
	void GetToken(char *buff, bool newline);
	//Seek to given offset starting from given origin
	bool Seek(int offset,  int origin);

	uint GetPos() const;
	uint GetSize()const;
	byte * GetBuffer() const { return 0; }
	const char * GetFileName() const;
	
private:

	uint	m_size;			//File Size
	char *  m_filename;

	FILE *  m_fp;			//if its a real file, then use a filestream
	int     m_filehandle;	//handle to filestream in an archive
	CArchive * m_archive;   //Arhive containing the given file
};


#endif