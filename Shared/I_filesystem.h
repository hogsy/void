#ifndef INC_FILESYSTEM_INTERFACE
#define INC_FILESYSTEM_INTERFACE

#ifdef FILESYSTEM_EXPORTS
#define FILESYSTEM_API __declspec(dllexport)
#else
#define FILESYSTEM_API __declspec(dllimport)
#endif

#include "I_file.h"

/*
================================================
FileSystem interface
================================================
*/
struct I_FileSystem
{
	virtual I_FileReader * CreateReader(EFileMode mode)=0;

	//Check after creating it the first time
	virtual bool IsActive() const =0;

	//Set current "Game" directory for over-ridable content
	virtual bool AddGameDir(const char *dir)=0;

	//Remove any additional game dirs. reset to basedir
	virtual void ResetGameDir()=0;

	//List Current Search Paths
	virtual void ListSearchPaths()=0;

	//Print out the list of files in added archives
	virtual void ListArchiveFiles()=0;
	virtual void ListFiles(const char *path, const char *ext)=0;

	//Finds the full file name and returns it
	virtual bool FindFileName(char * buf, int buflen, const char * path)=0;

	//Returns current path EXE+Game
	virtual const char * GetCurrentPath() const=0;
};

typedef void (*PrintFunc)(const char * msg,...);
typedef void (*ErrorFunc)(const char * msg);

FILESYSTEM_API I_FileSystem * FILESYSTEM_Create(PrintFunc pPrint, ErrorFunc pError,
										const char * exeDir,const char * baseGameDir);
FILESYSTEM_API void FILESYSTEM_Free();

#endif
