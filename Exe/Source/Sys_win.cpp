#include "Sys_main.h"
#include "Util_sys.h"
#include "resources.h"	


static bool RegisterWindow();

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
	InitMemReporting();

	if(!RegisterWindow())
	{
		MessageBox(NULL,"Error Registering Window\n","Error",MB_OK);
		return -1;
	}

	g_pVoid = new CVoid(hInst, hPrevInst, lpCmdLine);
	if(!g_pVoid->Init()) 
	{
		g_pVoid->Error("Error Initializing Subsystems\n");
		g_pVoid->Shutdown();
		delete g_pVoid;
		EndMemReporting();
		return -1;
	}

	MSG msg;
	while (1)
	{
		if(PeekMessage(&msg,g_hWnd,NULL, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		g_pVoid->RunFrame();	// Game loop function
	}

/*	while(1)
	{
		if(PeekMessage(&msg,g_hWnd,0,0, PM_NOREMOVE))
		{
			do
			{
				if(!GetMessage(&msg,g_hWnd,0,0))
				{
					g_pVoid->Shutdown();
					delete g_pVoid;

					EndMemReporting();					

					return 0;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);

			}while (PeekMessage(&msg,g_hWnd,0,0, PM_NOREMOVE));
		}
		else
		{
			g_pVoid->RunFrame();	// Game loop function
		}
	}
*/
	//Will never get executed
	g_pVoid->Shutdown();
	delete g_pVoid;  
	EndMemReporting();
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
	case WM_SETFOCUS:
//	case WM_MOUSEACTIVATE:
		{
			g_pVoid->OnFocus();
			break;
		}
	case WM_ENTERSIZEMOVE:
	case WM_ENTERMENULOOP:
		{
//			if(g_pInput)
//				g_pInput->UnAcquire();
			break;
		}
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
			g_pVoid->Shutdown();
			delete g_pVoid;
			EndMemReporting();
			exit(0);
			break; 
		}
	}
	return(DefWindowProc(hWnd, msg, wParam, lParam));
}

/*
==========================================
Register the Window
==========================================
*/

static bool RegisterWindow()
{
	WNDCLASSEX wcl;
	
	wcl.cbSize = sizeof(WNDCLASSEX);
	wcl.style = CS_OWNDC | CS_HREDRAW|CS_VREDRAW;
	wcl.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_APPLICATION));
	wcl.hIconSm = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_APPLICATION));
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = g_hInst;
	wcl.lpszClassName = MAINWINDOWCLASS;
	wcl.lpfnWndProc = MainWndProc;
	wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!RegisterClassEx(&wcl))
		return false;
	return true;
}

