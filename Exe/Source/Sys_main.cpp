#define __VOIDALPHA	1
#define __MUSIC	1

#include "Sys_main.h"
#include "Sys_time.h"
#include "Sv_main.h"
#include "Cl_main.h"
#include "In_main.h"
#include "Snd_main.h"
#include "Mus_main.h"
#include "Util_sys.h"
#include "resources.h"	
#include "I_renderer.h"

//========================================================================================

#define MAINWINDOWCLASS "Void"
#define MAINWINDOWTITLE	"Void"

//Global vars 
//========================================================================================
HWND		g_hWnd;
HINSTANCE	g_hInst;

char		g_exedir[COM_MAXPATH];
char		g_gamedir[COM_MAXPATH];

eGameState	g_gameState;
RECT		g_hRect;

//pointers to subsystems
//========================================================================================
//CMemManager * g_pMem=0;
CTime		* g_pTime =0;
CInput		* g_pInput=0;		//Input 
CConsole	* g_pCons =0;		//Console

#ifdef __SOUND
CSound		* g_pSound=0;		//Sound Subsystem
#endif

#ifdef __MUSIC
CMusic		* g_pMusic=0;		//Music Subsystem
#endif

#ifndef __VOIDALPHA
CServer		* g_pServer=0;		//Network Server
#endif

CClient		* g_pClient=0;		//Client and UI

//Rendering info, and API
//========================================================================================
RenderInfo_t * g_pRinfo =0;		//holds current renderering info
I_Renderer   * g_pRender=0;
VoidExport_t * g_pExport=0;

//========================================================================================
CVoid		 * g_pVoid =0;		//The game
world_t		 * g_pWorld=0;		//The World


//console loopback func
static void CFuncQuit(int argc, char** argv);		//quit game
static void CFuncMap(int argc, char** argv);		//start local server with map + connect to it
static void CFuncDisconnect(int argc, char** argv);	//disconnect from server + shutdown if local 
static void CFuncConnect(int argc, char ** argv);	//connect to a server
void CToggleConsole(int argc, char** argv);			//this should be non-static so the console can access it


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
			//Move the rendering window
			if(g_pRender)
				g_pRender->MoveWindow((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam));
			break;
		}
	case WM_SIZE:
		{
			//Check to see if we are losing our window...
			if(wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED)
			{
				g_pRinfo->active = false;
				break;
			}
			g_pRinfo->active = true;

			//Change the size of the rendering window
			if (g_pRender && !(g_pRinfo->rflags & RFLAG_FULLSCREEN))
				g_pRender->Resize();

			//Update rect
			::GetClientRect(hWnd,&g_hRect);

			//Set Window extents for input if full screen
			if(g_pInput)
				g_pInput->Resize();
			break;
		}
//	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE:
		{
			if (wParam == WA_INACTIVE)
			{
				g_pRinfo->active = false;

//				if(g_pInput)
//					g_pInput->UnAcquire();
			}
			else //if (wParam == WA_ACTIVE)
			{
				g_pRinfo->active = true;

				if(g_pInput)
					g_pInput->Acquire();

				if (g_pRender && (g_pRinfo->rflags & RFLAG_FULLSCREEN))
					g_pRender->Resize();

			}
			break;
		}
	case WM_SETFOCUS:
//	case WM_MOUSEACTIVATE:
		{
			//start rendering again
			if (g_pRinfo)
				g_pRinfo->active = true;
			
			//Input Focus
			if(g_pInput)
				g_pInput->Acquire();
			break;
		}
	case WM_ENTERSIZEMOVE:
	case WM_ENTERMENULOOP:
		{
			if(g_pInput)
					g_pInput->UnAcquire();
			break;
		}
	case WM_KILLFOCUS:
		{
			//Input loses Focus
			if(g_pInput)
					g_pInput->UnAcquire();
			
			//stop rendering
			if (g_pRinfo)
				g_pRinfo->active = false;
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
Windows Entry Point
==========================================
*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
				   LPSTR lpCmdLine, int nCmdShow)
{
	
	InitMemReporting();
//	::atexit(EndMemReporting);

	MSG msg;
	g_pVoid = new CVoid(hInst, hPrevInst, lpCmdLine, nCmdShow);

	if(!g_pVoid->Init()) 
	{
		g_pVoid->Error("Error Initializing Subsystems\n");
		g_pVoid->Shutdown();
		delete g_pVoid;
		EndMemReporting();
		return -1;
	}

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
Register the Window
==========================================
*/

bool CVoid::RegisterWindow()
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


/*
==========================================
CVoid::Void(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		   LPSTR lpCmdLine, int nCmdShow)
Constructor
==========================================
*/

CVoid::CVoid(HINSTANCE hInstance, 
			 HINSTANCE hPrevInstance, 
		     LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;

	if(!RegisterWindow())
	{
		Error("Error Registering Window\n");
		return;
	}

	//Current Working directory
	_getcwd(g_exedir,COM_MAXPATH);

	//Init Memory first of all
//	g_pMem = new CMemManager(COM_DEFAULTHUNKSIZE);

	//Init Game Timer
	g_pTime = new CTime;

	//Console
	g_pCons = new CConsole;

	//Input sys
	g_pInput = new CInput;

#ifndef __VOIDALPHA
	//Network Sys
	g_pServer = new CServer;
#endif
	strcpy(g_gamedir,"game");


	//Rendering Info
	g_pRinfo = new RenderInfo_t;
	
	g_pExport = new VoidExport_t;

#ifdef __SOUND
	//Sound
	g_pSound = new CSound;
#endif

#ifdef __MUSIC
	//Music
	g_pMusic = new CMusic;
#endif

	g_pClient = new CClient;

	//Set game state - full screen console - not connected
	g_gameState = INCONSOLE;

	g_pCons->RegisterCFunc("quit",&CFuncQuit);							//Quit
	g_pCons->RegisterCFunc("exit",&CFuncQuit);							//Quit
	g_pCons->RegisterCFunc("map", &CFuncMap);
//	g_pCons->RegisterCFunc("disconnect", &CFuncDisconnect);
//	g_pCons->RegisterCFunc("connect",&CFuncConnect);
//	g_pCons->RegisterCFunc("unzip", &CUnzipfile);
}


/*
==========================================
Destructor
==========================================
*/

CVoid::~CVoid() 
{
#ifndef __VOIDALPHA	
	if(g_pServer)
	{
		delete g_pServer;
		g_pServer = 0;
	}
#endif
	
	if(g_pClient)
	{
		delete g_pClient;
		g_pClient = 0;
	}

#ifdef __SOUND
	if(g_pSound)
	{
		delete g_pSound;
		g_pSound = 0;
	}

#endif
#ifdef __MUSIC
	if(g_pMusic)
	{
		delete g_pMusic;
		g_pMusic = 0;
	}
#endif
	
	if(g_pInput)
	{
		delete g_pInput;
		g_pInput = 0;
	}

	if(g_pTime)
	{
		delete g_pTime;
		g_pTime = 0;
	}

	if(g_pRinfo)
	{
		delete g_pRinfo;
		g_pRinfo = 0;
	}

	//Free the Renderer Interface
	if(g_pRender)
	{
		FREERENDERER rfree = (FREERENDERER)::GetProcAddress(hRenderer, "FreeRenderer");
		rfree(&g_pRender);
	}

	if(g_pExport)
	{
		g_pExport->basedir = 0;
		g_pExport->gamedir = 0;
		g_pExport->vconsole = 0;
		g_pExport->frametime = 0;
		g_pExport->curtime = 0;
		delete g_pExport;
		g_pExport = 0;
	}
	
	if(g_pCons)
	{
		delete g_pCons;
		g_pCons = 0;
	}

/*	if(g_pMem)
	{
		delete g_pMem;
		g_pMem= 0;
	}
*/
	//Release the renderer dll
	::FreeLibrary(hRenderer);	
}


/*
==========================================
CVoid::Init
Intializes all the subsystems
==========================================
*/

bool CVoid::Init()
{
	HRESULT			hr;
	GETRENDERERAPI	rapi;

	//subsystem startup proceedures
	//=============================

	//Renderer
	//========
	hRenderer = ::LoadLibrary("vrender.dll");
	if(hRenderer == NULL)
	{
		Error("CVoid::Init:Could not load Renderer dll");
		return false;
	}

	rapi = (GETRENDERERAPI)::GetProcAddress(hRenderer, "GetRendererAPI");
	hr = rapi(&g_pRender);

	if(FAILED(hr))
	{
		Util_ErrorMessage(hr,"Void::Init:Error Loading Renderer");
		g_pRender = NULL ; 
		return false;
	}

	g_pRinfo->hInst  = g_hInst;
	g_pRinfo->width  = 640;
	g_pRinfo->height = 480;
	g_pRinfo->bpp	 = 16;
	g_pRinfo->zdepth = 16;
	g_pRinfo->stencil= 0;
	g_pRinfo->active = false;
	g_pRinfo->ready  = false;
	g_pRinfo->fov	 = PI / 2;
	g_pRinfo->rflags = 0;
	
	g_pExport->basedir = g_exedir;
	g_pExport->gamedir = g_gamedir;
	g_pExport->curtime = &g_fcurTime;
	g_pExport->frametime = &g_fframeTime;
	g_pExport->vconsole = (I_ExeConsole*)g_pCons;

	if(!g_pRender->PreInit(g_pRinfo,&g_pExport))
	{
		Error("Void::Init:Error Intializing Renderering API\n");	
		return false;
	}

	//We parse the command line exec configs NOW
	//once the renderer has been activated and its cvars have been registered

	//parse Command line
	ParseCmdLine(g_exedir);

	g_pCons->ExecConfig("default.cfg");
	g_pCons->ExecConfig("void.cfg");

	//Init COM librarry
	hr = CoInitialize(NULL);

	if(FAILED(hr))
	{
		Error("CVoid::Init:Error Initializing COM library\n");
		return false;
	}

	//timer
	g_pTime->Init();

	//create the window
	g_pRinfo->hWnd = CreateWindow(MAINWINDOWCLASS, 
								  MAINWINDOWTITLE,
								  WS_BORDER | WS_DLGFRAME | WS_POPUP, 
								  CW_USEDEFAULT, 
								  CW_USEDEFAULT, 
								  640,
								  480,
								  HWND_DESKTOP, 
								  NULL, 
								  g_hInst,
								  NULL);
	if (!g_pRinfo->hWnd)
	{
		Error("CVoid::Init:Error Creating Window\n");
		return false;
	}
	g_pRinfo->hInst = g_hInst;

	if(!g_pCons->Init(g_pRender->GetConsole()))
	{
		Error("CVoid::Init: Could not Intialize the console");
		return false;
	}

	if(!g_pRender->InitRenderer())
	{
		Error("Void::Init:Error Intializing Renderer\n");	
		return false;
	}

	g_hWnd = g_pRinfo->hWnd;
	g_hInst = g_pRinfo->hInst;
	ShowWindow(g_hWnd, SW_NORMAL); 
	UpdateWindow(g_hWnd);

	//Input
	if(!g_pInput->Init()) 
	{
		Error("CVoid::Init: Could not Initialize Input");
		return false;
	}
	//Input Focus
	g_pInput->Acquire();


/*	if(!InitZip())
	{
		ComPrintf("CVoid::Init: couldnt load unzip library\n");
	}
*/
#ifndef __VOIDALPHA
	if(!InitWinsock())
	{
		Error("CVoid::Init: Couldnt init Winsock");
		return false;
	}

	//Server
	if(!g_pServer->Init())
	{
		Error("CVoid::Init: Could not Initialize Winsock");
		return false;
	}
#endif

#ifdef __SOUND
	//Sound 
	if(!g_pSound->Init())
	{
		ComPrintf("CVoid::Init: couldnt init sound system\n");
		delete g_pSound;
		g_pSound = 0;
	}
#endif

#ifdef __MUSIC
	//Music
	if(!g_pMusic->Init())
	{
		ComPrintf("CVoid::Init: couldnt music system\n");
		delete g_pMusic;
		g_pMusic = 0;
	}
#endif


#ifndef __VOIDALPHA
	//Client, create the client last
	if(!g_pClient->InitNet())
	{
		Error("CVoid::Init: Couldnt not init client socket");
		return false;
	}
#endif
	
	g_pInput->SetKeyHandler(&ConsoleHandleKey);
	g_pCons->ExecConfig("autoexec.cfg");
	g_pTime->Reset();

	return true;
}

/*
==========================================
Shutdown all the subsystems
==========================================
*/

bool CVoid :: Shutdown()
{
	//Subsystem shutdown proceedures
	//=============================

	if(g_pClient && g_pClient->m_ingame)
		g_pClient->Disconnect();

#ifndef __VOIDALPHA
	if(g_pServer && g_pServer->m_active)
	{
		if(!ShutdownServer())
		{
			ComPrintf("CVoid::Shutdown: couldnt shutdown game\n");
			return false;
		}
	}
#endif

#ifdef __SOUND
	//Sound
	if(g_pSound)
		g_pSound->Shutdown();
#endif


#ifdef __MUSIC
	//music
	if(g_pMusic)
		g_pMusic->Shutdown();
#endif

	//input
	if(g_pInput)
		g_pInput->Release();

//	ShutdownZip();

	//Renderer
	if(g_pRender)
		g_pRender->Shutdown();

	//console
	char configname[128];
	sprintf(configname,"%s\\void.cfg",g_exedir);
	WriteConfig(configname);

	if(g_pCons)
		g_pCons->Shutdown(); 

	//Release COM library
	CoUninitialize();
	
	return true;
}



/*
==========================================
The Game Loop
==========================================
*/
void CVoid::RunFrame()
{
	g_pTime->Update();
	
	//Run Input frame
	g_pInput->InputFrame();

	//Run Server
#ifndef __VOIDALPHA
	if(g_pServer->m_active)
		g_pServer->RunFrame();
#endif

	//Run Client frame
	if(g_pClient->m_ingame)
	//if(g_pClient->m_active)
	{
		g_pClient->RunFrame();

		//draw the scene
		g_pRender->DrawFrame(&g_pClient->eye.origin,&g_pClient->eye.angles);
	}
	else
	{
		//draw the console or whatnot
		g_pRender->DrawFrame(0,0);
	}
}


/*
======================================
Init Game
======================================
*/
bool CVoid::InitServer(char *map)
{
/*	if(g_pClient->m_connected && !g_pClient->Disconnect())
	{
		ComPrintf("CVoid::InitServer:Unable to disconnect Client\n");
		return false;
	}
*/
	char worldname[128];
	char mapname[128];
	
	strcpy(mapname, map);
	Util_DefaultExtension(mapname,".bsp");
	//	sprintf(worldname,"%s\\%s\\worlds\\%s",g_exedir,g_gamedir,mapname);
	sprintf(worldname,"%s\\game\\worlds\\%s",g_exedir,mapname);
	
	if(g_pWorld != 0)
	{
		g_pClient->UnloadWorld();
		world_destroy(g_pWorld);
	}
	g_pWorld = 0;
	g_pWorld = world_create(worldname);
	g_pClient->LoadWorld(g_pWorld);
	g_pClient->m_connected = true;
	g_pClient->m_ingame = true;
	
	g_pInput->SetCursorHandler(&ClientHandleCursor);
	g_pInput->SetKeyHandler(&ClientHandleKey);


#ifndef __VOIDALPHA
	if(g_pServer->m_active)
	{
		if(!ShutdownServer())
		{
			ComPrintf("CVoid::InitGame: already in game, couldnt shutdown\n");
			return false;
		}
	}

	if(!g_pServer->InitGame(map))
	{
		ComPrintf("CVoid::InitGame: couldnt init server\n");
		ShutdownServer();
		return false;
	}
#endif

/*	if(!LoadWorld(mapname))
	{
		ComPrintf("CVoid::InitGame: couldnt load world\n");
		ShutdownServer();
		return false;
	}

	//Init game server and load the world
	if(!g_pServer->InitGame(g_pWorld))
	{
		ComPrintf("CVoid::InitGame: couldnt init server\n");
		ShutdownServer();
		return false;
	}
*/

	ComPrintf("CVoid::InitGame: OK\n");
	return true;
}

/*
======================================
Shutdown game
======================================
*/
bool CVoid::ShutdownServer()
{
	//Network Server
#ifndef __VOIDALPHA
	if(g_pServer && g_pServer->m_active)
		g_pServer->Shutdown();
#endif

	g_pClient->UnloadWorld();
	UnloadWorld();

/*	//destroy the world
	if(g_pWorld)
		world_destroy(g_pWorld);
	g_pWorld = 0;
*/
	g_gameState = INCONSOLE;
	ComPrintf("CVoid::ShutdownGame: OK\n");
	return true;
}


/*
=======================================
Load a map
destroys current map, if its there
=======================================
*/
bool CVoid::LoadWorld(char *worldname)
{
	if(g_pWorld != 0)
		return false;

#ifndef __VOIDALPHA	
	if(!strcmp(worldname,g_pServer->m_mapname))
	{
		g_pWorld = g_pServer->GetWorld();
		return true;
	}
#endif

	g_pWorld = world_create(worldname);
	if(!g_pWorld)
	{
		ComPrintf("CVoid::LoadWorld: couldnt load %s\n",worldname);
		return false;
	}
	return true;
}

/*
=====================================
Unload a map
=====================================
*/

bool CVoid::UnloadWorld()
{
#ifndef __VOIDALPHA
	if(g_pWorld && g_pWorld == g_pServer->GetWorld())
	{
		ComPrintf("CVoid::UnloadWorld: world in use by the server\n");
		return false;
	}
#endif

	world_destroy(g_pWorld);
	g_pWorld =0;
	return true;
}



//==============================================================================================
//Other Utility Functions
//==============================================================================================

/*
==========================================
Parse Command line, update any CVar values
==========================================
*/
void CVoid::ParseCmdLine(LPSTR lpCmdLine)
{
	ComPrintf("CVoid::Command Line :%s\n",lpCmdLine);
	ComPrintf("CVoid::Current Working Directory :%s\n", g_exedir);

	//FIXME - break into arguments
	//decide on an identifier scheme
}


/*
==========================================
Fatal Error
==========================================
*/
void CVoid::Error(char *error, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, error);
	vsprintf(textBuffer, error, args);
	va_end(args);
	MessageBox(NULL, textBuffer, "Error", MB_OK);

	//Win32
	PostMessage(g_hWnd,				// handle of destination window 
				WM_QUIT,			// message to post 
				0,					// first message parameter 
				0);
}


/*
=====================================
Write the configuration file
=====================================
*/
void CVoid::WriteConfig(char *config)
{
	//Write the Config file
	if(config)
	{
		FILE * fp = fopen(config,"w");
		if(fp != NULL)
		{
			if(g_pClient)
				g_pClient->Cl_WriteBindTable(fp);
			g_pCons->WriteCVars(fp);
			fclose(fp);
		}
	}
}

//==============================================================================================
/*
===============================================
Console "Quit" loopback func
quit game - 
disconnect client + shutdown server + exit game
===============================================
*/
void CFuncQuit(int argc, char** argv)
{
	ComPrintf("CVoid::Quit\n");
	
	//Win32 func
	PostMessage(g_hWnd,				// handle of destination window 
				WM_QUIT,			// message to post 
				0,					// first message parameter 
				0);					// second message parameter 

}

/*
======================================
Load a Map
start local server with map + connect to it
======================================
*/
void CFuncMap(int argc, char** argv)
{
	if(argc >= 2)
	{
		g_pVoid->InitServer(argv[1]);
/*		if(InitServer(argv[1]))
		{
			g_pClient->ConnectTo("loopback",36666);
		}
		else
			ComPrintf("CVoid::Map: couldnt start server\n");
*/
		return;
	}
	ComPrintf("CVoid::Map: invalid arguments\n");
}


/*
======================================
Disconnect
disconnect from server + shutdown server if local 
======================================
*/
void CFuncDisconnect(int argc, char** argv)
{
	if(g_pClient->m_ingame)
		g_pClient->Disconnect();
#ifndef __VOIDALPHA
	if(g_pServer->m_active)
#endif
		g_pVoid->ShutdownServer();
}


/*
=====================================
Connect
=====================================
*/
void CFuncConnect(int argc, char ** argv)
{
	if(argc ==2)
	{
		if(ValidIP(argv[1]))
			g_pClient->ConnectTo(argv[1],36666);
		else
			ComPrintf("CVoid::CFuncConnect:INVALID IP ADDR %s\n",argv[1]);
	}
//TEMP
	else
		g_pClient->ConnectTo("24.112.133.112",36666);
}


/*
=====================================

=====================================
*/

void CToggleConsole(int argc, char** argv)
{
	if(g_gameState == INGAMECONSOLE)
	{
		g_gameState = INGAME;
		g_pCons->Toggle(false);
		g_pInput->SetCursorHandler(&ClientHandleCursor);
		g_pInput->SetKeyHandler(&ClientHandleKey);
	}
	else if(g_gameState == INGAME)
	{
		g_pInput->SetCursorHandler(0);
		g_pInput->SetKeyHandler(&ConsoleHandleKey);

		g_gameState = INGAMECONSOLE;
		g_pCons->Toggle(true);
	}
}