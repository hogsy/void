#ifndef INC_FILE_INTERFACE
#define INC_FILE_INTERFACE

/*
================================================
Interface to fileReader
Creation function is exported by the Dll to be
statically linked by the exe. The exe provides
the fileCreation func to other modules as needed
================================================
*/

enum EFileMode
{
	FILE_BUFFERED,
	FILE_STREAM
};

struct I_FileReader
{
	//Open the file at the given path
	virtual bool Open(const char * szFilename)=0;
	//Close the currently opened file
	virtual void Close()=0;
	//Destroy/Delete the reader
	virtual void Destroy()=0;
	
	//Read "count" number of items of "size" into buffer
	virtual uint Read(void *buf,uint size, uint count)=0;
	
	//Return the current byte, advance current position
	virtual int  GetChar(bool perror)=0;
	virtual void GetToken(char *buff, bool newline)=0;
	
	//Seek to given offset starting from given origin
	virtual bool Seek(int offset, int origin)=0;

	//Misc access funcs
	virtual uint GetPos() const=0;
	virtual uint GetSize()const=0;
	virtual const char * GetFileName() const=0;
	virtual byte * GetBuffer() const=0;
};

#endif


