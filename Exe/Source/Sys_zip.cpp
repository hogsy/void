/*
   This is a very simplistic example of how to load and make a call into the
   dll. This has been compiled and tested for a 32-bit console version, but
   not under 16-bit windows. However, the #ifdef's have been left in for the
   16-bit code, simply as an example.

 */



#ifndef WIN32   /* this code is currently only tested for 32-bit console */
#  define WIN32
#endif

#if defined(__WIN32__) && !defined(WIN32)
#  define WIN32
#endif

//Void Standard Header
#include "Sys_hdr.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "Sys_zip.h"


#define UNZ_DLL_VERSION "5.4\0"
#define COMPANY_NAME "Info-ZIP\0"


#include <winver.h>



#define UNZ_DLL_NAME "UNZIP32.DLL\0"


#define DLL_WARNING "Cannot find %s."\
            " The Dll must be in the application directory, the path, "\
            "the Windows directory or the Windows System directory."
#define DLL_VERSION_WARNING "%s has the wrong version number."\
            " Insure that you have the correct dll's installed, and that "\
            "an older dll is not in your path or Windows System directory."

int hFile;              /* file handle */

LPUSERFUNCTIONS lpUserFunctions;
HANDLE hUF = (HANDLE)NULL;
LPDCL lpDCL = NULL;
HANDLE hDCL = (HANDLE)NULL;
HINSTANCE hUnzipDll;
HANDLE hZCL = (HANDLE)NULL;
DWORD dwPlatformId = 0xFFFFFFFF;


/* Forward References */
int WINAPI DisplayBuf(LPSTR, unsigned long);
int WINAPI GetReplaceDlgRetVal(char *);
int WINAPI password(char *, int, const char *, const char *);
void WINAPI ReceiveDllMessage(unsigned long, unsigned long, unsigned,
    unsigned, unsigned, unsigned, unsigned, unsigned,
    char, LPSTR, LPSTR, unsigned long, char);

_DLL_UNZIP Wiz_SingleEntryUnzip;
_USER_FUNCTIONS Wiz_Init;

void FreeUpMemory(void);
#ifdef WIN32
BOOL IsNT(VOID);
#endif


bool InitZip()
{
	DWORD dwVerInfoSize;		//Version Info size
	DWORD dwVerHnd;				//Version info
	char szFullPath[PATH_MAX];	//full path
	char *ptr;					
	HANDLE  hMem;         /* handle to mem alloc'ed */


	hDCL = GlobalAlloc( GPTR, (DWORD)sizeof(DCL));
	if (!hDCL)
	{
		return false;
	}
	lpDCL = (LPDCL)GlobalLock(hDCL);
	if (!lpDCL)
	{
		GlobalFree(hDCL);
		return false;
	}

	hUF = GlobalAlloc( GPTR, (DWORD)sizeof(USERFUNCTIONS));
	if (!hUF)
	{	
		GlobalUnlock(hDCL);
		GlobalFree(hDCL);
		return false;
	}
	lpUserFunctions = (LPUSERFUNCTIONS)GlobalLock(hUF);

	if (!lpUserFunctions)
	{
		GlobalUnlock(hDCL);
		GlobalFree(hDCL);
		GlobalFree(hUF);
		return false;
	}

	lpUserFunctions->password = password;
	lpUserFunctions->print = DisplayBuf;
	lpUserFunctions->sound = NULL;
	lpUserFunctions->replace = GetReplaceDlgRetVal;
	lpUserFunctions->SendApplicationMessage = ReceiveDllMessage;

	/* First we go look for the unzip dll */
	if (SearchPath(
					NULL,               /* address of search path               */
					UNZ_DLL_NAME,       /* address of filename                  */
					NULL,               /* address of extension                 */
					PATH_MAX,           /* size, in characters, of buffer       */
					szFullPath,         /* address of buffer for found filename */
					&ptr                /* address of pointer to file component */
					) == 0)
   {
		char str[256];
		wsprintf (str, DLL_WARNING, UNZ_DLL_NAME);
		ComPrintf("%s\n", str);
		FreeUpMemory();
		return false;
   }


/* Now we'll check the unzip dll version information. Note that this is
   not the same information as is returned from a call to UzpVersion()
 */
	dwVerInfoSize =   GetFileVersionInfoSize(szFullPath, &dwVerHnd);

	if (dwVerInfoSize)
	{
		BOOL  fRet, fRetName;
		char str[256];
		LPVOID lpstrVffInfo; /* Pointer to block to hold info */
		LPVOID lszVer = NULL;
		LPVOID lszVerName = NULL;

		UINT  cchVer = 0;

		/* Get a block big enough to hold the version information */
		hMem          = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo  = (char *)GlobalLock(hMem);

		/* Get the version information */
		if (GetFileVersionInfo(szFullPath, 0L, dwVerInfoSize, lpstrVffInfo))
		{
			fRet = VerQueryValue(lpstrVffInfo,
								TEXT("\\StringFileInfo\\040904E4\\FileVersion"),
								&lszVer,
								&cchVer);
			fRetName = VerQueryValue(lpstrVffInfo,
									TEXT("\\StringFileInfo\\040904E4\\CompanyName"),
									&lszVerName,
									&cchVer);
			if (!fRet || !fRetName ||
				(lstrcmpi((char *)lszVer, UNZ_DLL_VERSION) != 0) ||
				(lstrcmpi((char *)lszVerName, COMPANY_NAME) != 0))
			{
				wsprintf (str, DLL_VERSION_WARNING, UNZ_DLL_NAME);
				ComPrintf("%s\n", str);
				FreeUpMemory();
				GlobalUnlock(hMem);
				GlobalFree(hMem);
				return false;
			}
      }
		/* free memory */
		GlobalUnlock(hMem);
		GlobalFree(hMem);
   }
   else
   {
		char str[256];
		wsprintf (str, DLL_VERSION_WARNING, UNZ_DLL_NAME);
		ComPrintf("%s\n", str);
		FreeUpMemory();
		return false;
   }

   
/* Okay, now we know that the dll exists, and has the proper version
 * information in it. We can go ahead and load it.
 */
	hUnzipDll = LoadLibrary(UNZ_DLL_NAME);
    if (hUnzipDll != NULL)
	{
		(_DLL_UNZIP)Wiz_SingleEntryUnzip =	(_DLL_UNZIP)GetProcAddress(hUnzipDll, "Wiz_SingleEntryUnzip");
	}
	else
	{
		char str[256];
		wsprintf (str, "Could not load %s", UNZ_DLL_NAME);
		ComPrintf("%s\n", str);
		FreeUpMemory();
		return false;
   }

	ComPrintf("InitZip - ok\n");
	return true;
}


bool ShutdownZip()
{
	if(hUnzipDll)
		FreeLibrary(hUnzipDll);
	FreeUpMemory();
	return true;
}




void CUnzipfile(int argc, char** argv)
{
	int retcode;				//return code
#if 0
	int exfc, infc;
	char **exfv, **infv;
	char *x_opt;

	/*
   Here is where the actual extraction process begins. First we set up the
   flags to be passed into the dll.
 */
	lpDCL->ncflag = 0; /* Write to stdout if true */
	lpDCL->fQuiet = 0; /* We want all messages.
						  1 = fewer messages,
						  2 = no messages */
	lpDCL->ntflag = 0; /* test zip file if true */
	lpDCL->nvflag = 1; /* give a verbose listing if true */
	lpDCL->nUflag = 0; /* Do not extract only newer */
	lpDCL->nzflag = 0; /* display a zip file comment if true */
	lpDCL->ndflag = 1; /* Recreate directories if true */
	lpDCL->noflag = 1; /* Over-write all files if true */
	lpDCL->naflag = 0; /* Do not convert CR to CRLF */
	lpDCL->lpszZipFN = argv[1]; /* The archive name */
	lpDCL->lpszExtractDir = NULL; /*The directory to extract to. This is set
									to NULL if you are extracting to the
									current directory.			*/
#endif

   lpDCL->ncflag = 0;
   lpDCL->fQuiet = 0; 
   lpDCL->ntflag = 0;
   lpDCL->nvflag = 1;//(int)(!uf.fFormatLong ? 1 : 2);
   lpDCL->nUflag = 1;
   lpDCL->nzflag = 0;
   lpDCL->ndflag = 0;
   lpDCL->noflag = 0;
   lpDCL->naflag = 0;
   lpDCL->lpszZipFN = argv[1]; //lpumb->szFileName;
/*   argc   = 0;
   argv = NULL;
*/
   lpDCL->lpszExtractDir = NULL;
   retcode = (*Wiz_SingleEntryUnzip)(0, 0, 0, 0, lpDCL,lpUserFunctions);


/*
   As this is a quite short example, intended primarily to show how to
   load and call in to the dll, the command-line parameters are only
   parsed in a very simplistic way:
   We assume that the command-line parameters after the zip archive
   make up a list of file patterns:
   " [file_i1] [file_i2] ... [file_iN] [-x file_x1 [file_x2] ...]".
   We scan for an argument "-x"; all arguments in front are
   "include file patterns", all arguments after are "exclude file patterns".
   If no more arguments are given, we extract ALL files.

   In summary, the example program should be run like:
   example <archive.name> [files to include] [-x files to exclude]
   ("<...> denotes mandatory arguments, "[...]" optional arguments)
 */
#if 0
   x_opt = NULL;
	if (argc > 2) 
	{
		infv = &argv[2];
		for (infc = 0; infc < argc-2; infc++)
			if (!strcmp("-x", infv[infc])) 
			{
				x_opt = infv[infc];
				infv[infc] = NULL;
				break;
			}
		exfc = argc - infc - 3;
		if (exfc > 0)
			exfv = &argv[infc+3];
		else 
		{
			exfc = 0;
			exfv = NULL;
		}
	} 
	else 
	{
		infc = exfc = 0;
		infv = exfv = NULL;
	}
	
	retcode = (*Wiz_SingleEntryUnzip)(infc, infv, exfc, exfv, lpDCL,
										lpUserFunctions);
	if (x_opt) 
	{
		infv[infc] = x_opt;
		x_opt = NULL;
	}
#endif

	if (retcode != 0)
		ComPrintf("Error unzipping...\n");
	else
		ComPrintf("SUCCESS - unzipped %s\n",argv[1]);
}



int WINAPI GetReplaceDlgRetVal(char *filename)
{
/* This is where you will decide if you want to replace, rename etc existing
   files.
 */
return 1;
}

void FreeUpMemory(void)
{
	if (hDCL)
	{
		GlobalUnlock(hDCL);
		GlobalFree(hDCL);
	}
	if (hUF)
	{
		GlobalUnlock(hUF);
		GlobalFree(hUF);
	}
}

/* This simply determines if we are running on NT or Windows 95 */

BOOL IsNT(VOID)
{
	if(dwPlatformId != 0xFFFFFFFF)
		return dwPlatformId;
	else
	/* note: GetVersionEx() doesn't exist on WinNT 3.1 */
	{
		if(GetVersion() < 0x80000000)
		{
			//(BOOL)
			dwPlatformId = TRUE;
		}
		else
		{
			//(BOOL)
			dwPlatformId = FALSE;
		}
	}
	return dwPlatformId;
}


/* This is a very stripped down version of what is done in Wiz. Essentially
   what this function is for is to do a listing of an archive contents. It
   is actually never called in this example, but a dummy procedure had to
   be put in, so this was used.
 */
void WINAPI ReceiveDllMessage(unsigned long ucsize,  //uncompressed size
							  unsigned long csiz,    //compressed size
							  unsigned cfactor,      //compression factor
							  unsigned mo, unsigned dy, unsigned yr, unsigned hh, unsigned mm,	//time
							  char c,	
							  LPSTR filename, 
							  LPSTR methbuf, 
							  unsigned long crc, 
							  char fCrypt)
{

	char psLBEntry[PATH_MAX];
	char LongHdrStats[] =  "%7lu  %7lu %4s  %02u-%02u-%02u  %02u:%02u  %c%s";
	char CompFactorStr[] = "%c%d%%";
	char CompFactor100[] = "100%%";
	char szCompFactor[10];
	char sgn;

	if (csiz > ucsize)
		sgn = '-';
	else
		sgn = ' ';
	
	if (cfactor == 100)
		lstrcpy(szCompFactor, CompFactor100);
	else
		sprintf(szCompFactor, CompFactorStr, sgn, cfactor);
	
	wsprintf(psLBEntry, LongHdrStats, ucsize, csiz, szCompFactor, mo, dy, yr, hh, mm, c, filename);

	ComPrintf("%s\n", psLBEntry);
}


/* Password entry routine - see password.c in the wiz directory for how
   this is actually implemented in WiZ. If you have an encrypted file,
   this will probably give you great pain.
 */
int WINAPI password(char *p, int n, const char *m, const char *name)
{
return 1;
}

/* Dummy "print" routine that simply outputs what is sent from the dll */

int WINAPI DisplayBuf(LPSTR buf, unsigned long size)
{
ComPrintf("Buf::%s", (char *)buf);
return (unsigned int) size;
}
