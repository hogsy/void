#include <stdio.h> 
#include "Standard.h"


void Util_GetExtension(const char *filename, char *ext)
{
	int		i;
	const char * p = filename;

	while (*p && *p != '.')
		p++;
	if (!*p)
		return;
	p++;
	for (i=0 ; i<7 && *p ; i++,p++)
		ext[i] = *p;
	ext[i] = 0;
}

/*
======================================
make sure the dir is there.  create it if not
======================================
*/
void ConfirmDir(char* dir)
{
// try creating each dir - nothing will change if it already exists
	char *c = dir;
	while (*c)
	{
		if ((*c)== '\\')
		{
			*c = NULL;
			CreateDirectory(dir, NULL);
			*c = '\\';
		}
		c++;
	}
}


/**********************************************
make sure a filename has the extension
**********************************************/
void DefaultExtension (char *path, char *extension)
{
	char    *src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;                 // it has an extension
		src--;
	}

	strcat (path, extension);
}



/*
=======================================
local Error and FATAL error funcs
=======================================
*/
// fatal error - MUST quit
void FError(char *error, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, error);
	vsprintf(textBuffer, error, args);
	va_end(args);
	
	MessageBox(NULL, textBuffer, "Error", MB_OK);
	
	//Win32 func
	PostMessage(rInfo->hWnd,	// handle of destination window 
				WM_QUIT,			// message to post 
				0,					// first message parameter 
				0);					// second message parameter 
}

// just a small booboo. let us know and keep going
void Error(char *error, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, error);
	vsprintf(textBuffer, error, args);
	va_end(args);
	MessageBox(NULL, textBuffer, "Error", MB_OK);
}


