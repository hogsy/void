#include "Sys_hdr.h"
#include "Util_sys.h"

//winbase.h 

/*
=======================================
Returns the extension of the filename 
passed to it
MAX of 8 chars for an extension
=======================================
*/

void Util_GetExtension(const char *filename, char *ext)
{
//	static char exten[8];
	int		i;
//	memset(exten,0,8);

	while (*filename && *filename != '.')
		filename++;
	if (!*filename)
		return;
	filename++;
	for (i=0 ; i<7 && *filename ; i++,filename++)
		ext[i] = *filename;
	ext[i] = 0;
//	return exten;
}

/*
=======================================
Removes the file extension of a given file
=======================================
*/

void Util_RemoveExtension (const char *in, char *out)
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

void Util_GetFilePath(const char *file, char *path)
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

void   Util_FindExtension(const char*filename, char *out)
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch;
	char	temp[256];
	char	ext[4];
			
	// Start searching for file
	// the filename SHOULD include the path before the file
//	temp = new char[strlen(filename)+3];
	strcpy(temp,filename);
	strcat(temp,".*");
	g_pCons->dprintf("Searching for file: %s\n",temp);
	
	hSearch = FindFirstFile(temp, &FileData); 
		
	if (hSearch == INVALID_HANDLE_VALUE) 
		return;
	
	g_pCons->dprintf("Found : %s\n",FileData.cFileName);
	FindClose(hSearch);

	Util_GetExtension(FileData.cFileName,ext);
	strcat(out,ext);
	return;
}


/**********************************************
make sure a filename has the extension
**********************************************/
void Util_DefaultExtension (char *path, const char *extension)
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
			Util_GetExtension(path,ext);
			if(strcmp(ext,extension))
			{	
				Util_RemoveExtension(path,path);
				Util_DefaultExtension(path,extension);
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
void Util_ErrorMessage( HRESULT hr, const char* str)

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
		g_pCons->dprintf("%s - Error %d - %s\n",str,hr,pMsgBuf);
	else
		g_pCons->dprintf("::Error %d - %s\n",hr,pMsgBuf);

	// Free the buffer.
	LocalFree( pMsgBuf ) ;

}


void  Util_ErrorMessageBox(HRESULT hr, const char* str)
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

