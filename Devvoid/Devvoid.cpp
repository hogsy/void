// Devvoid.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Devvoid.h"
#include "DevvoidDlg.h"

#include "Std_lib.h"
#include "Com_util.h"
#include "Com_registry.h"
#include "I_fileSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDevvoidDlg * g_pDlg=0;
FILE	* g_fLog = 0;
I_FileSystem * g_pFileSystem=0;

void FError(const char *err, ...);
void FileSysError(const char *err);
void ComPrintf(const char *err, ...);


//==========================================================================
//==========================================================================


/////////////////////////////////////////////////////////////////////////////
// CDevvoidApp

BEGIN_MESSAGE_MAP(CDevvoidApp, CWinApp)
	//{{AFX_MSG_MAP(CDevvoidApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDevvoidApp construction

CDevvoidApp::CDevvoidApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDevvoidApp object

CDevvoidApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDevvoidApp initialization

BOOL CDevvoidApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	//=========================================
	//Change to proper Void Game dir
	if(!ChangeToVoidDir())
	{	return FALSE;
	}

	g_fLog = fopen("Devvoid.log","w");


	//=========================================
	//Initialize File System
	g_pFileSystem = FILESYSTEM_Create(&ComPrintf, &FileSysError, m_exePath, "Game");
	if(!g_pFileSystem)
	{
		AfxMessageBox("Unable to create Void FileSystem");
		return FALSE;
	}

	CDevvoidDlg dlg;
	
	m_pMainWnd = &dlg;
	g_pDlg	= &dlg;

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	FILESYSTEM_Free();

	if(g_fLog)
		fclose(g_fLog);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



/*
================================================
Change to the proper directory
================================================
*/
bool CDevvoidApp::ChangeToVoidDir()
{
	char bufPath[_MAX_PATH];
	char message[512];

	if(!VoidReg::DoesKeyExist("Software\\Devvoid\\Void"))
	{
		MessageBox(0,"Void has not been installed in this profile. Please install Void first", "Error",
			MB_OK);
		return false;
	}

	if(!VoidReg::GetKeyValue("Software\\Devvoid\\Void","Path", bufPath, MAX_PATH))
	{
		MessageBox(0,"Error reading registry info, please reinstall Void", "Void", MB_OK);
		return false;
	}

	if(bufPath[0] == '\"')
	{
		strcpy(m_exePath,bufPath+1);
		m_exePath[strlen(m_exePath)-1] = 0;
	}
	else
		strcpy(m_exePath,bufPath);

	//Read path info, and change to proper dir
	if(!::SetCurrentDirectory(m_exePath))
	{
		char errMsg[256];

		if(Util::GetWin32ErrorMessage(GetLastError(),errMsg,256))
			sprintf(message,"Can't change directory to %s \n\rError: %s", m_exePath, errMsg);
		else
			sprintf(message,"Unknown error while changing directory to %s", m_exePath);
		MessageBox(0,message,"Void", MB_OK);
		return false;
	}
	return true;
}



//==========================================================================
//==========================================================================

void Error(const char *err, ...)
{
	static char buff[1024];
	static CCriticalSection	criticalSel;

	criticalSel.Lock();

	va_list args;
	va_start(args, err);
	vsprintf(buff, err, args);
	va_end(args);

	ComPrintf(buff);

	criticalSel.Lock();

	MessageBox(0,buff,"Error",MB_OK);

	if(g_pDlg)
		g_pDlg->EndDialog(IDOK);
}

void ComPrintf(const char *msg, ...)
{
	static char buff[1024];
	static CCriticalSection	criticalSel;

	criticalSel.Lock();

	va_list args;
	va_start(args, msg);
	vsprintf(buff, msg, args);
	va_end(args);

	int lastChar = strlen(buff)-1;
	if(buff[lastChar] == '\n')
		buff[lastChar] = '\0';

	if(g_pDlg && lastChar > 1)
		g_pDlg->m_wndOutput.AddLine(CString(buff));
	if (g_fLog)
		fprintf(g_fLog, "%s\n", buff);

	criticalSel.Unlock();
}


//Needed by Renderer
void FError(const char *err, ...)
{
	static char buff[1024];
	static CCriticalSection	criticalSel;

	criticalSel.Lock();

	va_list args;
	va_start(args, err);
	vsprintf(buff, err, args);
	va_end(args);

	ComPrintf(buff);
	
	criticalSel.Unlock();

	MessageBox(0,buff,"Error",MB_OK);

	if(g_pDlg)
		g_pDlg->EndDialog(IDOK);
}


void FileSysError(const char *err)
{	Error(err);
}

const char * GetVoidPath()
{	return ((CDevvoidApp*)AfxGetApp())->m_exePath;
}

I_FileReader * CreateFileReader(EFileMode mode)
{	return g_pFileSystem->CreateReader(mode);
}
