#include "Sys_hdr.h"
#include "Sys_main.h"
#include "resources.h"
#include "Com_util.h"
#include "Com_registry.h"
#include <direct.h>
#include <mmsystem.h>

// FIXME - where should this really be??
#include "Com_fastmath.h"	// needed for build_sqrt_table()

//Private Info
static HWND			m_hWnd;
static HINSTANCE	m_hInst;
static char		    m_exePath[COM_MAXPATH];

static bool RegisterWindow(HINSTANCE hInst);
static void UnRegisterWindow(HINSTANCE hInst);
static bool ChangeToVoidDir();

//The game
CVoid		* g_pVoid=0;		

/*
================================================
Global Access funcs
================================================
*/
namespace System
{
	const char* GetExePath()  { return m_exePath; } 
	HINSTANCE	GetHInstance(){ return m_hInst; }
	HWND		GetHwnd()	  { return m_hWnd;  }
}

/*
==========================================
Windows Entry Point
==========================================
*/
int WINAPI WinMain(HINSTANCE hInst, 
				   HINSTANCE hPrevInst,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	// must be very first thing
	build_sqrt_table();


	m_hInst = hInst;
	if(!RegisterWindow(hInst))
	{
		MessageBox(0,"Error Registering Window\n","Error",MB_OK);
		return -1;
	}

	//Find Void key in registry and get path info 
	if(!ChangeToVoidDir())
		return -1;

	//Strip quotes off the commandline
	char cmdLine[COM_MAXPATH];
	memset(cmdLine,0,COM_MAXPATH);
	if(lpCmdLine[0])
	{
		//strip "" if present
		if(lpCmdLine[0] != '"')
			strcpy(cmdLine,lpCmdLine);
		else
		{
			strcpy(cmdLine, lpCmdLine+1);
			int len = strlen(lpCmdLine) - 2;
			cmdLine[len] = '\0';
		}

		//Validata CommandLine. Should always be in the Void dir
		if(_strnicmp(m_exePath,cmdLine,strlen(m_exePath)))
		{
			char msg[256];
			sprintf("Cannot load map : %s\nMaps can only be loaded from the Void directory\n", cmdLine);
			MessageBox(0,msg,"Void", MB_OK);
			memset(cmdLine,0,COM_MAXPATH);
		}
	}
	
	//Create the Void object
	g_pVoid = new CVoid(m_exePath,cmdLine);
	if(!g_pVoid->Init()) 
	{
		System::FatalError("Error Initializing Subsystems\n");
		return -1;
	}

	//Start the window loop
	MSG msg;
	while (1)
	{
		//Process any window messages
		while(PeekMessage(&msg,m_hWnd,NULL, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Run the game frame
		g_pVoid->RunFrame();
	}

	//Will never get executed
	delete g_pVoid;  
	UnRegisterWindow(m_hInst);
	return -1;
}


/*
==========================================
Main Window Proc
==========================================
*/
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, 
				    	     WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_MOVE:
		{
			g_pVoid->OnMove((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam));
			break;
		}
	case WM_DISPLAYCHANGE:
		{
			g_pVoid->OnDisplayChange(wParam, LOWORD(lParam), HIWORD(lParam));
			break;
		}
	case WM_SIZE:
		{
			//Check to see if we are losing our window
			if(wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED)
			{
				g_pVoid->OnResize(false,0,0,0,0);
				break;
			}

			RECT rect;
			::GetClientRect(hWnd,&rect);
			g_pVoid->OnResize(true,rect.left,rect.top, rect.right, rect.bottom);
			break;
		}
	case WM_MOUSEACTIVATE:
		{
			return MA_ACTIVATEANDEAT;
		}
	case WM_ACTIVATE:
		{
			if (wParam == WA_INACTIVE)
				g_pVoid->OnActivate(false);
			else if ((wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE))
				g_pVoid->OnActivate(true);
			break;
		}
	case WM_ENTERSIZEMOVE:
	case WM_ENTERMENULOOP:
		{
			return 0;
		}
	case WM_SETFOCUS:
		{
			g_pVoid->OnFocus();
			break;
		}
	case MM_MCINOTIFY :
		{
			g_pVoid->OnMultiMedia(wParam,lParam);
			break;
		}
	case WM_KILLFOCUS:
		{
			g_pVoid->OnLostFocus();
			break;
		}
	case WM_NCDESTROY:
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		{
			//Cleanup
			delete g_pVoid;
			UnRegisterWindow(m_hInst);
			exit(0);
			break; 
		}
	}
	return(DefWindowProc(hWnd, msg, wParam, lParam));
}

/*
==========================================
Register the Window class
==========================================
*/
static bool RegisterWindow(HINSTANCE hInst)
{
	WNDCLASSEX wcl;
	
	wcl.cbSize = sizeof(WNDCLASSEX);
	wcl.style = CS_OWNDC | CS_HREDRAW|CS_VREDRAW;
	wcl.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPLICATION));
	wcl.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPLICATION));
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hInst;
	wcl.lpszClassName = VOID_MAINWINDOWCLASS;
	wcl.lpfnWndProc = MainWndProc;
	wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!RegisterClassEx(&wcl))
		return false;
	return true;
}

/*
==========================================
Unregister the window class
==========================================
*/
static void UnRegisterWindow(HINSTANCE hInst)
{
	UnregisterClass(VOID_MAINWINDOWCLASS,
					hInst);
}


/*
================================================
Change to the proper directory
================================================
*/
static bool ChangeToVoidDir()
{
	char bufPath[COM_MAXPATH];
	char message[512];

	if(!VoidReg::DoesKeyExist("Software\\Devvoid\\Void"))
	{
		MessageBox(0,"Void has not been installed on this system.\r"
					 "Please make sure you run the setup program first.",
					 "Void", MB_OK);
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
	if(!SetCurrentDirectory(m_exePath))
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

/*
==========================================
Window Creation func
==========================================
*/
bool CVoid::CreateVoidWindow()
{
	m_hWnd = CreateWindow(VOID_MAINWINDOWCLASS, 
						  VOID_MAINWINDOWTITLE,
						  WS_BORDER | WS_DLGFRAME | WS_POPUP, 
						  CW_USEDEFAULT, 
						  CW_USEDEFAULT, 
						  640,
						  480,
						  HWND_DESKTOP, 
						  NULL, 
						  m_hInst,
						  NULL);
	if(!m_hWnd)
		return false;
	return true;
}


/*
======================================
Handle out of memeoty
======================================
*/
int HandleOutOfMemory(size_t size)
{
	delete g_pVoid;
	UnRegisterWindow(m_hInst);
	exit(0);
	return 0;
}
