#ifndef _EXAMPLE_H
#define _EXAMPLE_H

#include <windows.h>
#ifdef __RSXNT__
#  include "win32/rsxntwin.h"
#endif
#include <assert.h>    /* required for all Windows applications */
#include <stdlib.h>
#include <stdio.h>
#include <commdlg.h>
#ifndef __RSXNT__
#  include <dlgs.h>
#endif
#include <windowsx.h>


//Struct definations


#ifndef Far
#  define Far far
#endif

/* Porting definitions between Win 3.1x and Win32 */
#ifdef WIN32
#  define far
#  define _far
#  define __far
#  define near
#  define _near
#  define __near
#  ifndef FAR
#    define FAR
#  endif
#endif

#ifndef PATH_MAX
#  define PATH_MAX 260            /* max total file or directory name path */
#endif

#ifndef DEFINED_ONCE
#define DEFINED_ONCE

typedef int (WINAPI DLLPRNT) (LPSTR, unsigned long);
typedef int (WINAPI DLLPASSWORD) (LPSTR, int, LPCSTR, LPCSTR);
typedef int (WINAPI DLLSERVICE) (LPCSTR, unsigned long);
#endif

typedef void (WINAPI DLLSND) (void);
typedef int (WINAPI DLLREPLACE)(LPSTR);
typedef void (WINAPI DLLMESSAGE)(unsigned long, unsigned long, unsigned,
								unsigned, unsigned, unsigned, unsigned, unsigned,
								char, LPSTR, LPSTR, unsigned long, char);

typedef struct 
{
DLLPRNT *print;						//print routine
DLLSND *sound;						//sound routine
DLLREPLACE *replace;				//replace prompt
DLLPASSWORD *password;				//password prompt
DLLMESSAGE *SendApplicationMessage;	//file listing routine
DLLSERVICE *ServCallBk;				//windows messages etc
unsigned long TotalSizeComp;		//total size compressed, filled in by dll
unsigned long TotalSize;			//total size uncompressed
int CompFactor;						//compression factor
unsigned int NumMembers;			//number of files in archive
WORD cchComment;					//comment
} USERFUNCTIONS, far * LPUSERFUNCTIONS;


typedef struct 
{
int ExtractOnlyNewer;   //true if you are to extract only newer
int SpaceToUnderscore;  //true if convert space to underscore
int PromptToOverwrite;  //true if prompt to overwrite is wanted
int fQuiet;             //quiet flag. 1 = few messages, 2 = no messages, 0 = all messages
int ncflag;             //write to stdout if true
int ntflag;             //test zip file
int nvflag;             //verbose listing
int nUflag;             //"update" (extract only newer/new files)
int nzflag;             //display zip file comment
int ndflag;             //all args are files/dir to be extracted
int noflag;             //true if you are to always over-write files, false if not
int naflag;             //do end-of-line translation
int nZIflag;            //get zip info if true
int C_flag;             //be case insensitive if TRUE
int fPrivilege;         // => restore Acl's, 2 => Use privileges
LPSTR lpszZipFN;        //zip file name
LPSTR lpszExtractDir;   //Directory to extract to. This should be NULL if you
                        //are extracting to the current directory.
} DCL, far * LPDCL;


//end struct definations



/* Defines */
#ifndef MSWIN
#define MSWIN
#endif

typedef int (WINAPI * _DLL_UNZIP)(int, char **, int, char **,
                                  LPDCL, LPUSERFUNCTIONS);
typedef int (WINAPI * _USER_FUNCTIONS)(LPUSERFUNCTIONS);

/* Global variables */

extern LPUSERFUNCTIONS lpUserFunctions;
extern LPDCL lpDCL;

extern HINSTANCE hUnzipDll;

extern int hFile;                 /* file handle             */

/* Global functions */

extern _DLL_UNZIP Wiz_SingleEntryUnzip;
extern _USER_FUNCTIONS Wiz_Init;
int WINAPI DisplayBuf(LPSTR, unsigned long);

/* Procedure Calls */
void WINAPI ReceiveDllMessage(unsigned long, unsigned long, unsigned,
   unsigned, unsigned, unsigned, unsigned, unsigned,
   char, LPSTR, LPSTR, unsigned long, char);
#endif /* _EXAMPLE_H */
