#ifndef VOID_COM_FILE_H
#define VOID_COM_FILE_H

/*
================================================
W32 lowlevel file wrapper
Use these instead of C std io FILEs they
end up calling these anyways.
================================================
*/
class VFile
{
public:

	enum EOpenMode
	{                        
	  OPEN_READ,		// Open for reading
	  OPEN_WRITE,		// Open for writing
	  OPEN_READ_WRITE   // Open for reading and writing
	};

	enum EOpenType
	{                
	  E_CREATE_NEW = 1,      // Fails if file already exists
	  E_CREATE_ALWAYS=2,     // Never fails, always creates and truncates to 0
	  E_OPEN_EXISTING=3,     // Fails if file doesn't exist, keeps contents
	  E_OPEN_ALWAYS=4,       // Never fails, creates if doesn't exist, keeps contents
	  E_TRUNCATE_EXISTING=5  // Fails if file doesn't exist, truncates to 0
	};

	enum ESharing
	{
	  SHARE_NONE   = 0x0000, // No sharing
	  SHARE_READ   = 0x0001, // Allow sharing for reading
	  SHARE_WRITE  = 0x0002  // Allow sharing for writing
	};

public:

	VFile();
	virtual ~VFile();

	bool Open(const char* szPath,
			 int openMode = OPEN_READ, 
			 EOpenType openType = E_OPEN_EXISTING, 
			 int shareMode= SHARE_READ);

	void Close();

	bool IsOpen() const;
	int  GetPosition() const;
	int  GetLength()   const;
	const char * GetFilePath() const;

	bool Seek(int offset, int origin);
	bool SeekToStart();
	bool SeekToEnd();

	//Will return 0 if fail
	int   ReadChar();
	ulong Read ( void* buffer, ulong numBytes);
	ulong Write( void* buffer, ulong numBytes);
	bool  Flush();
   
public:

	//extensions will include the "."
	static char * ParseExtension(char *ext,  int bufsize, const char *filename);
	//includes drive name
	static char * ParseFilePath (char *path, int pathlen, const char *filename);
	static char * ParseFileName (char *name, int namelen, const char *path);
	//extension should include the "."
	static char * ForceExtension(char *szPath, const char *extension);
	
	static void RemoveExtension(char *filename);
	static bool CompareExtension(const char *szPath, const char *ext);		
	static void ConfirmDir(const char * szPath);
	
	static bool FileExists(const char * szPath);
	static bool FileDelete(const char * szPath);
	static bool FileRename(const char* szOldPath, const char* szNewPath);
	static bool FileCopy(const char* szSourcePath, const char* szDestPath, bool bOverwrite = true);

	//Add more

protected:

	char      m_szPath[COM_MAXPATH]; // Full path to file
	bool      m_bOpen;               // Whether or not the file is open
	HANDLE	  m_hFile;
};

#endif