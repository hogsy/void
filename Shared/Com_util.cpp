#include "Com_util.h"

//======================================================================================
//======================================================================================

namespace Util
{

/*
=======================================
Returns the extension of the filename 
passed to it
MAX of 8 chars for an extension
=======================================
*/
void GetExtension(const char *filename, char *ext)
{
	while (*filename && *filename != '.')
		filename++;
	if (!*filename)
		return;
	filename++;
	for (int i=0 ; i<7 && *filename ; i++,filename++)
		ext[i] = *filename;
	ext[i] = 0;
}

/*
=======================================
Removes the file extension of a given file
=======================================
*/
void RemoveExtension (const char *in, char *out)
{
	while (*in && *in != '.')
		*out++ = *in++;
	*out = 0;
}


/*
=======================================
Get File Path
=======================================
*/
void GetFilePath(const char *file, char *path)
{
	const char *s;
	
	//point to end of file
	s = file + strlen(file) - 1;
	
	//go back until we get to a /
	while (s != file && *s != '/')
		s--;
	//copy everything before the / to path
	strncpy (path,file, s-file);
	path[s-file] = 0;
}

/*
=======================================
Looks in the directory for file starting 
with the passed filename and returns the 
extension of the file if found
=======================================
*/
void  FindExtension(const char*filename, char *out)
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch;
	char	temp[256];
	char	ext[4];
			
	// Start searching for file
	// the filename SHOULD include the path before the file
	strcpy(temp,filename);
	strcat(temp,".*");
	ComPrintf("Searching for file: %s\n",temp);
	
	hSearch = FindFirstFile(temp, &FileData); 
		
	if (hSearch == INVALID_HANDLE_VALUE) 
		return;
	
	ComPrintf("Found : %s\n",FileData.cFileName);
	FindClose(hSearch);

	GetExtension(FileData.cFileName,ext);
	strcat(out,ext);
	return;
}


/*
==========================================
Make sure the file has the given extension
==========================================
*/
void DefaultExtension (char *path, const char *extension)
{
	char    *src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;
	while (*src != '/' && src != path)
	{
		if (*src == '.')	// it has an extension
		{
			char ext[4];
			GetExtension(path,ext);
			if(strcmp(ext,extension))
			{	
				RemoveExtension(path,path);
				DefaultExtension(path,extension);
			}
			return;                 
		}
		src--;
	}
	strcat (path, extension);
}


/*
=======================================
prints HR Error message
=======================================
*/
void PrintErrorMessage( HRESULT hr, const char* str)

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
void ErrorMessageBox(HRESULT hr, const char* str)
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


}
