#include "I_filesystem.h"
#include "Com_util.h"

extern CFileSystem * g_pFileSystem;

namespace FileUtil
{

FILESYSTEM_API bool FindFileExtension(char * ext, int extlen, const char * path)
{
	char filename[COM_MAXPATH];
	if(g_pFileSystem->FindFileName(filename,COM_MAXPATH,path))
	{
		Util::ParseExtension(ext,extlen,filename);
		return true;
	}
	return false;
}

/*
======================================
make sure the dir is there. create it if not
======================================
*/
FILESYSTEM_API void ConfirmDir(char* dir)
{
	//try creating each dir - nothing will change if it already exists
	char *c = dir;
	while (*c)
	{
		if ((*c)== '\\')
		{
			*c = NULL;
			::CreateDirectory(dir, NULL);
			*c = '\\';
		}
		c++;
	}
}

/*
===========================================
Check if given directoy exists
===========================================
*/
FILESYSTEM_API bool PathExists(const char *path)
{
	WIN32_FIND_DATA	finddata;
	HANDLE hsearch = FindFirstFile(path,&finddata);
	if(hsearch == INVALID_HANDLE_VALUE)
		return false;

	if(FindClose(hsearch) == FALSE)
		ComPrintf("CFileSystem::PathExists:Unable to close search handle\n");
	return true;
}

}