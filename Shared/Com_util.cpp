#include "Com_util.h"

//======================================================================================
//======================================================================================

namespace Util
{

/*
=======================================
Returns the extension of the filename  passed to it
=======================================
*/
void ParseExtension(char *ext, int bufsize, const char *filename)
{
	const char * p = filename + strlen(filename) -1;
	while (*p && *p != '.' && *p != '/')
		p--;
	if (*p != '.')
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
======================================
Get file name out of a path string
======================================
*/
void ParseFileName(char *name, int namelen, const char *path)
{
	int pathlen = strlen(path);
	const char * s = path + pathlen - 1;

	//go back until we get to a /
	while (s != path && *s != '/' && *s != '\\')
		s--;
	int i = s-path + 1;
	strcpy(name,path+i);
}

/*
=======================================
Get File Path
=======================================
*/
void ParseFilePath(char *path, int pathlen,const char *file)
{
	//point to end of file
	const char *s = file + strlen(file) - 1;
	
	//go back until we get to a /
	while (s != file && *s != '/' && *s != '\\')
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
==========================================
Shared utility funcs
Compare file Extensions
return true if equa
==========================================
*/
bool CompareExts(const char *file, const char *ext)		
{
	const char *p = file + (strlen(file) - 1);
	while(*p && *p!='.' && *p!='\0')
		p--;
	if(*p=='.')
	{
		if(!_stricmp(++p,ext))
			return true;
	}
	return false;
}




//======================================================================================
//======================================================================================

/*
=======================================
prints HR message
=======================================
*/
void HRPrint( HRESULT hr, const char* str)
{
	void* pMsgBuf ;
 
	::FormatMessage( 
		 FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		 NULL,
		 hr,
		 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		 (LPTSTR) &pMsgBuf,
		 0,
		 NULL 
	) ;

	// Display the string.
	if(str)
		ComPrintf("%s - Error %d - %s\n",str,hr,pMsgBuf);
	else
		ComPrintf("::Error %d - %s\n",hr,pMsgBuf);

	// Free the buffer.
	LocalFree( pMsgBuf ) ;
}

/*
==========================================
Print HR error message box
==========================================
*/
void HRShowMessageBox(HRESULT hr, const char* str)
{
	void* pMsgBuf ;
	 
	::FormatMessage( 
		 FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		 NULL,
		 hr,
		 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		 (LPTSTR) &pMsgBuf,
		 0,
		 NULL 
	) ;

	if(str)
	{
		char msg[512];
		strcpy(msg,str);
		strcat(msg," : ");
		strcat(msg,(char *)pMsgBuf);
		MessageBox(NULL, msg, "Error", MB_OK);
	}
	else
		MessageBox(NULL, (char *)pMsgBuf, "Error", MB_OK);

	// Free the buffer.
	LocalFree( pMsgBuf ) ;
}

/*
==========================================
Throw a messagebox
==========================================
*/
void ShowMessageBox(const char * str, const char *title)
{
	if(!title)
		MessageBox(0,str,"Error", MB_OK);
	else
		MessageBox(0,str,title, MB_OK);
}

}
