#ifndef VOID_COM_FILE
#define VOID_COM_FILE

#include "Com_defs.h"

/*
==========================================
The Resource File class
Used for Buffered Binary Input 
==========================================
*/

class CFile
{
public:

	~CFile();

	//Was the file opened
	bool isOpen();

	//Read num items of size into buf
	ulong Read(void *buf, 
			   const ulong &size, 
			   const ulong &num);
	
	//Return the current byte, advances pointer
	inline int GetChar();

	/*
	More Read Funcs here ?
	GetInt()
	GetFloat()
	*/
	

	//Seek
	bool Seek(const ulong &offset, 
			  const int   &origin);

	//Info
	ulong GetPos() const { return m_curpos; }
	ulong GetSize()const { return m_size;   } 

protected:
	
	friend class CFileManager;

	//A file can ONLY be created,opened or closed via the FileManager
	CFile();

	ulong	m_size;		//File Size
	ulong	m_curpos;	//File Position

	byte *  m_buffer;	//Buffer can hold file data
	char *	m_filename;
};

#endif