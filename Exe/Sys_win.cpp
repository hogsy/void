#include "Sys_hdr.h"
#include "Sys_main.h"
#include "resources.h"
#include "Com_util.h"
#include <direct.h>
#include <mmsystem.h>

//======================================================================================
//======================================================================================

//Private Info
static HWND			m_hWnd;
static HINSTANCE	m_hInst;
static bool RegisterWindow(HINSTANCE hInst);
static void UnRegisterWindow(HINSTANCE hInst);

CVoid		* g_pVoid=0;		//The game

namespace System
{

	HINSTANCE	GetHInstance(){ return m_hInst; }
	HWND		GetHwnd()	  { return m_hWnd;  }
}

//======================================================================================
//======================================================================================

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
	m_hInst = hInst;
	if(!RegisterWindow(hInst))
	{
		MessageBox(NULL,"Error Registering Window\n","Error",MB_OK);
		return -1;
	}

	//Check if CmdLine contains a map, if so, then go down the directory
	//tree until we find a void binary to load it with and change to that dir
	char cmdLine[COM_MAXPATH];
	memset(cmdLine,0,COM_MAXPATH);
	if(lpCmdLine)
	{
		//strip ""
		if(lpCmdLine[0] == '"')
		{
			strcpy(cmdLine, lpCmdLine+1);
			int len = strlen(lpCmdLine) - 2;
			cmdLine[len] = '\0';
		}
	}

	if(Util::CompareExts(cmdLine,VOID_DEFAULTMAPEXT))
	{
		WIN32_FIND_DATA finddata;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		char nextPath[COM_MAXPATH];
		char curPath[COM_MAXPATH];
		bool foundPath = false;
	
		strcpy(curPath,cmdLine);
		do
		{
			memset(&finddata,0, sizeof(WIN32_FIND_DATA));
			memset(nextPath,0,COM_MAXPATH);

			Util::ParseFilePath(nextPath,COM_MAXPATH,curPath);
			if(!strlen(nextPath))
				break;

			hFind = ::FindFirstFile(VOID_DEFAULTBINARYNAME,&finddata);
			if(hFind != INVALID_HANDLE_VALUE)
			{
				foundPath = true;
				break;
			}

			if(_chdir(nextPath) == -1)
				break;
			strcpy(curPath,nextPath);

		}while(!foundPath);

		if(hFind != INVALID_HANDLE_VALUE)
			::FindClose(hFind);

		if(!foundPath)
		{
			Util::ShowMessageBox("Unable to find Void executable in the current directory tree",
								 "Void Error");
			return -1;
		}
	}

	//Create the Void object
	g_pVoid = new CVoid(cmdLine);
	if(!g_pVoid->Init()) 
	{
		System::FatalError("Error Initializing Subsystems\n");
		return -1;
	}

	//Start the windowloop
	MSG msg;
	while (1)
	{
		if(PeekMessage(&msg,m_hWnd,NULL, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		g_pVoid->RunFrame();	// Game loop function
	}

/*	while(1)
	{
		if(PeekMessage(&msg,m_hWnd,0,0, PM_NOREMOVE))
		{
			do
			{
				if(!GetMessage(&msg,m_hWnd,0,0))
				{
					g_pVoid->Shutdown();
					delete g_pVoid;

					EndMemReporting();					

					return 0;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);

			}while (PeekMessage(&msg,m_hWnd,0,0, PM_NOREMOVE));
		}
		else
		{
			g_pVoid->RunFrame();	// Game loop function
		}
	}
*/
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
			g_pVoid->Move((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam));
			break;
		}
	case WM_SIZE:
		{
			//Check to see if we are losing our window
			if(wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED)
			{
				g_pVoid->Resize(false,0,0,0,0);
				break;
			}

			RECT rect;
			::GetClientRect(hWnd,&rect);
			g_pVoid->Resize(true,rect.left,rect.top, rect.right, rect.bottom);
			break;
		}
//	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE:
		{
			if (wParam == WA_INACTIVE)
				g_pVoid->Activate(false);
			else if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)  ////if (wParam == WA_ACTIVE)
				g_pVoid->Activate(true);
			break;
		}
	case WM_ENTERSIZEMOVE:
	case WM_ENTERMENULOOP:
	case WM_SETFOCUS:
//	case WM_MOUSEACTIVATE:
		{
			g_pVoid->OnFocus();
			break;
		}
//FIXME. hack to get MCI notifications routed properly
	case MM_MCINOTIFY :
		{
			g_pVoid->HandleMM(wParam,lParam);
			break;
		}
/*	case WM_ENTERSIZEMOVE:
	case WM_ENTERMENULOOP:
		{
//			if(g_pInput)
//				g_pInput->UnAcquire();
			break;
		}
*/
	case WM_KILLFOCUS:
		{
			g_pVoid->LostFocus();
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
static  void UnRegisterWindow(HINSTANCE hInst)
{
	UnregisterClass(VOID_MAINWINDOWCLASS,
					hInst);
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
