#include "I_file.h"
#include "I_filesystem.h"

extern CFileSystem * g_pFileSystem;

namespace FileUtil
{

bool FindFileExtension(char * ext, int extlen, const char * path)
{
	char filename[COM_MAXPATH];
	if(g_pFileSystem->FindFileName(filename,COM_MAXPATH,path))
	{
		ParseExtension(ext,extlen,filename);
		return true;
	}
	return false;
}

/*
=======================================
Returns the extension of the filename  passed to it
=======================================
*/
void ParseExtension(char *ext, int bufsize, const char *filename)
{
	const char * p = filename;
	while (*p && *p != '.')
		p++;
	if (!*p)
		return;
	p++;
	for (int i=0 ; i<bufsize && *p ; i++,p++)
		ext[i] = *p;
	ext[i] = 0;
}


/*
=======================================
Removes the file extension of a given file
=======================================
*/
void RemoveExtension (char *out, int bufsize, const char *in)
{
	int i=0;
	while (*in && *in != '.' && i<bufsize)
	{
		*out++ = *in++;
		i++;
	}
	*out = 0;
}

/*
=======================================
Get File Path
=======================================
*/
void ParseFilePath(char *path, int pathlen,const char *file)
{
	const char *s;

	//point to end of file
	s = file + strlen(file) - 1;
	
	//go back until we get to a /
	while (s != file && *s != '/')
		s--;

	int i = s-file;
	if(pathlen < s-file)
		i = pathlen;
	
	//copy everything before the / to path
	strncpy (path,file, i);
	path[i] = 0;
}

/*
==========================================
Make sure the file has the given extension
==========================================
*/
void SetDefaultExtension (char *filename, const char *extension)
{
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	char * src = filename + strlen(filename) - 1;
	while (*src != '/' && src != filename)
	{
		if (*src == '.')	// it has an extension
		{
			char ext[4];
			ParseExtension(ext,4,filename);
			if(strcmp(ext,extension))
			{	
				RemoveExtension(filename,strlen(filename),filename);
				SetDefaultExtension(filename,extension);
			}
			return;                 
		}
		src--;
	}
	strcat (filename, extension);
}

/*
======================================
make sure the dir is there. create it if not
======================================
*/
void ConfirmDir(char* dir)
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
==========================================
Shared utility funcs
Compare file Extensions
return true if equa
==========================================
*/
bool CompareExts(const char *file, const char *ext)		
{
	const char *p = file;
	while(*p && *p!='.' && *p!='\0')
		p++;

	if(*p=='.')
	{
		if(!_stricmp(++p,ext))
			return true;
	}
	return false;
}


/*
===========================================
Check if given directoy exists
===========================================
*/
bool PathExists(const char *path)
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