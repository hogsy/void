#define __VOIDALPHA	1
#define INCLUDE_MUSIC	1

#include "Sys_main.h"
#include "Sv_main.h"
#include "Cl_main.h"
#include "In_main.h"
#include "Snd_main.h"
#include "Mus_main.h"
#include "I_renderer.h"

//Global vars 
//========================================================================================
HWND		g_hWnd;
HINSTANCE	g_hInst;
char		g_exedir[COM_MAXPATH];
char		g_gamedir[COM_MAXPATH];
eGameState	g_gameState;

//pointers to subsystems
//========================================================================================
CConsole	* g_pCons =0;		//Console
CInput		* g_pInput=0;		//Input 
CClient		* g_pClient=0;		//Client and UI

#ifdef INCLUDE_SOUND
CSound		* g_pSound=0;		//Sound Subsystem
#endif

#ifdef INCLUDE_MUSIC
CMusic		* g_pMusic=0;		//Music Subsystem
#endif

#ifndef __VOIDALPHA
CServer		* g_pServer=0;		//Network Server
#endif

//Rendering info, and API
//========================================================================================
I_Renderer   * g_pRender=0;

static RenderInfo_t * g_pRinfo =0;		//holds current renderering info
static VoidExport_t * g_pExport=0;

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
CVoid::Void(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		   LPSTR lpCmdLine, int nCmdShow)
Constructor
==========================================
*/

CVoid::CVoid(HINSTANCE hInstance, 
			 HINSTANCE hPrevInstance, 
		     LPSTR lpCmdLine)
{
	g_hInst = hInstance;

	_getcwd(g_exedir,COM_MAXPATH);		//Current Working directory
	strcpy(g_gamedir,"game");			//Set default game dir

	//Create timer
	g_pTime = new CTime();						

	//Create the game console
	g_pCons = new CConsole();
	
	//Create the file system
	g_pFileSystem = CreateFileSystem(g_pCons);
	
	//Create the input system
	g_pInput= new CInput();					
	
	//Export structure
	g_pExport= new VoidExport_t(g_exedir,g_gamedir,
					&g_fcurTime, &g_fframeTime);
	g_pExport->vconsole = (I_ExeConsole*)g_pCons;
	
	//Create the Renderer
	g_pRender= RENDERER_Create(g_pExport); 
	g_pRinfo = RENDERER_GetParms();
	
	//Create the client
	g_pClient = new CClient();		

#ifndef __VOIDALPHA
	//Network Sys
	g_pServer = new CServer();
#endif
	
#ifdef INCLUDE_SOUND
	//Sound
	g_pSound = new CSound();
#endif

#ifdef INCLUDE_MUSIC
	//Music
	g_pMusic = new CMusic();
#endif

	//Set game state - full screen console - not connected
	g_gameState = INCONSOLE;

	g_pCons->RegisterCFunc("quit",&CFuncQuit);			
	g_pCons->RegisterCFunc("exit",&CFuncQuit);			
	g_pCons->RegisterCFunc("map", &CFuncMap);
	g_pCons->RegisterCFunc("connect",&CFuncConnect);
//	g_pCons->RegisterCFunc("disconnect", &CFuncDisconnect);
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
	{	delete g_pServer;	
		g_pServer = 0;
	}
#endif
	
	if(g_pClient)
	{	delete g_pClient;
		g_pClient = 0;
	}

#ifdef INCLUDE_SOUND
	if(g_pSound)
	{	delete g_pSound;
		g_pSound = 0;
	}

#endif
#ifdef INCLUDE_MUSIC
	if(g_pMusic)
	{	delete g_pMusic;
		g_pMusic = 0;
	}
#endif
	
	if(g_pInput)
	{	delete g_pInput;
		g_pInput = 0;
	}

	if(g_pTime)
	{	delete g_pTime;
		g_pTime = 0;
	}

	//Free the Renderer Interface
	RENDERER_Free();

	if(g_pExport)
	{
		delete g_pExport;
		g_pExport = 0;
	}


	DestroyFileSystem();

	if(g_pCons)
	{		delete g_pCons;
		g_pCons = 0;
	}
}

/*
==========================================
CVoid::Init
Intializes all the subsystems
==========================================
*/

bool CVoid::Init()
{
	HRESULT hr;

	//================================
	//Init COM librarry
	hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		Error("CVoid::Init:Error Initializing COM library\n");
		return false;
	}

	//================================
	//Initialize FileSystem
	if(!g_pFileSystem->Init(g_exedir,"Game"))
	{
		Error("CVoid::Init:Error Initializing File System\n");
		return false;
	}

	//================================
	//Initialize Renderer
	

	//================================
	//We parse the command line and exec configs now since
	//the renderer has been activated and its cvars have been registered

	//parse Command line
//	ParseCmdLine(lpCmdLine);

	g_pCons->ExecConfig("default.cfg");
	g_pCons->ExecConfig("void.cfg");

	if(!g_pCons->Init(g_pRender->GetConsole()))
	{
		Error("CVoid::Init: Could not Intialize the console");
		return false;
	}

	//create the window
	g_hWnd = CreateWindow(MAINWINDOWCLASS, 
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
	if(!g_hWnd)
	{
		Error("CVoid::Init:Error Creating Window\n");
		return false;
	}
	g_pRinfo->hWnd = g_hWnd;

	if(!g_pRender->InitRenderer())
	{
		Error("Void::Init:Error Intializing Renderer\n");	
		return false;
	}

	//timer
	g_pTime->Init();

	//Input
	if(!g_pInput->Init()) 
	{
		Error("CVoid::Init: Could not Initialize Input");
		return false;
	}

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

#ifdef INCLUDE_SOUND
	//Sound 
	if(!g_pSound->Init())
	{
		ComPrintf("CVoid::Init: couldnt init sound system\n");
		delete g_pSound;
		g_pSound = 0;
	}
#endif

#ifdef INCLUDE_MUSIC
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

	GetInputFocusManager()->SetKeyListener(g_pCons,true);

	g_pTime->Reset();
	
	g_pCons->ExecConfig("autoexec.cfg");

	ShowWindow(g_hWnd, SW_NORMAL); 
	UpdateWindow(g_hWnd);
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

#ifdef INCLUDE_SOUND
	//Sound
	if(g_pSound)
		g_pSound->Shutdown();
#endif


#ifdef INCLUDE_MUSIC
	//music
	if(g_pMusic)
		g_pMusic->Shutdown();
#endif

	//input
	if(g_pInput)
		g_pInput->Shutdown();

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
	g_pInput->UpdateCursor();
	g_pInput->UpdateKeys();
/*	if(g_fcurTime > (lastinput + 0.05f))
	{
		g_pInput->UpdateKeys();
		lastinput = g_fcurTime;
	}
*/
	//Run Server
#ifndef __VOIDALPHA
	if(g_pServer->m_active)
		g_pServer->RunFrame();
#endif


	//Run Client frame
	if(g_pClient->m_ingame)
	//if(g_pClient->m_active)
	{
//		g_pTime->Update();
		
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
	
	strcpy(worldname,"Worlds/");
	strcat(worldname,mapname);
	
	if(g_pWorld != 0)
	{
		g_pClient->UnloadWorld();
		world_destroy(g_pWorld);
	}

	g_pWorld = 0;
	g_pWorld = world_create(worldname);

	if(!g_pWorld)
	{
		ComPrintf("CVoid::InitGame: couldnt load map\n");
		return false;
	}

	g_pClient->LoadWorld(g_pWorld);
	g_pClient->m_connected = true;
	g_pClient->m_ingame = true;
	g_pClient->SetInputState(true);
	

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
			g_pCons->WriteCVars(fp);
			if(g_pClient)
				g_pClient->WriteBindTable(fp);
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
	//List Current Search Paths
//	g_pFileSystem->ListSearchPaths();

	//Print out the list of files in added archives
//	g_pFileSystem->ListArchiveFiles();
/*
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
*/
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
		g_pClient->SetInputState(true);
	}
	else if(g_gameState == INGAME)
	{
		//g_pInput->SetCursorHandler(0);
		//g_pInput->SetKeyHandler(&ConsoleHandleKey);
		//In_SetCursorHandler(0);
		
		GetInputFocusManager()->SetCursorListener(0);

		//In_SetKeyHandler(&ConsoleHandleKey,true);
		GetInputFocusManager()->SetKeyListener(g_pCons,true);

		g_gameState = INGAMECONSOLE;
		g_pCons->Toggle(true);
	}
}










void CVoid::Move(int x, int y)
{
	if(g_pRender)
		g_pRender->MoveWindow(x,y);
}

//void CVoid::Resize(bool focus)
void CVoid::Resize(bool focus, int x, int y, int w, int h)
{
	if(focus==false)
	{
		g_pRinfo->active = false;
		return;
	}

	g_pRinfo->active = true;

	//Change the size of the rendering window
	if (g_pRender && !(g_pRinfo->rflags & RFLAG_FULLSCREEN))
		g_pRender->Resize();

	//Set Window extents for input if full screen
	if(g_pInput)
		g_pInput->Resize(x,y,w,h);
}

void CVoid::Activate(bool focus)
{
	if (focus == false)
	{
		g_pRinfo->active = false;
//		if(g_pInput)
//			g_pInput->UnAcquire();
	}
	else 
	{
		g_pRinfo->active = true;
		if(g_pInput)
			g_pInput->Acquire();

		if (g_pRender && (g_pRinfo->rflags & RFLAG_FULLSCREEN))
			g_pRender->Resize();
	}
}

void CVoid::OnFocus()
{
	if (g_pRinfo)
		g_pRinfo->active = true;
	
	//Input Focus
	if(g_pInput)
		g_pInput->Acquire();
}


void CVoid::LostFocus()
{
	//Input loses Focus
	if(g_pInput)
		g_pInput->UnAcquire();
	
	//stop rendering
	if (g_pRinfo)
		g_pRinfo->active = false;
}


