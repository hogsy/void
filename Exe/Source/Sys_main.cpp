#include "Sys_main.h"
#include "Sys_cons.h"
#include "Sv_main.h"
#include "Cl_main.h"
#include "In_main.h"
#include "Snd_main.h"
#include "Mus_main.h"

//========================================================================================
HWND		g_hWnd;
HINSTANCE	g_hInst;
char		g_exedir[COM_MAXPATH];
eGameState	g_gameState;

//========================================================================================
CVoid		* g_pVoid =0;		//The game
world_t		* g_pWorld=0;		//The World

//Subsystems
//======================================================================================

I_Renderer  * g_pRender  =0;	//Renderer
CConsole	* g_pConsole =0;	//Console
CInput		* g_pInput   =0;	//Input 
CClient		* g_pClient  =0;	//Client and UI

#ifdef INCLUDE_SOUND
CSound		* g_pSound=0;		//Sound Subsystem
#endif
#ifdef INCLUDE_MUSIC
CMusic		* g_pMusic=0;		//Music Subsystem
#endif
#ifndef __VOIDALPHA
CServer		* g_pServer=0;		//Network Server
#endif

//======================================================================================
//Global access functions for private data
//======================================================================================

HINSTANCE	Sys_GetHInstance(){ return g_hInst; }
HWND		Sys_GetHwnd()	  { return g_hWnd;  }
const char* Sys_GetExeDir()	  { return g_exedir;}


//======================================================================================
//Console loopback func
//======================================================================================

#define CMD_QUIT	0
#define CMD_MAP		1

static void CFuncDisconnect(int argc, char** argv);	//disconnect from server + shutdown if local 
static void CFuncConnect(int argc, char ** argv);	//connect to a server
void CToggleConsole(int argc, char** argv);			//this should be non-static so the console can access it


/*
==========================================
Constructor
==========================================
*/
CVoid::CVoid(HINSTANCE hInstance, 
			 HINSTANCE hPrevInstance, 
		     LPSTR lpCmdLine)
{
	g_hInst = hInstance;

	_getcwd(g_exedir,COM_MAXPATH);		//Current Working directory

	//Create timer
	g_pTime = new CTime();						

	//Create the game console
	g_pConsole = new CConsole();
	
	//Create the file system
	g_pFileSystem = FILESYSTEM_Create(g_pConsole);
	
	//Create the input system
	g_pInput= new CInput();					
	
	//Export structure
	g_pExport= new VoidExport_t(&g_fcurTime, &g_fframeTime);
	g_pExport->vconsole = (I_Console*)g_pConsole;
	
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

	Sys_GetConsole()->RegisterCommand("quit",CMD_QUIT,this);			
	Sys_GetConsole()->RegisterCommand("exit",CMD_QUIT,this);			
	Sys_GetConsole()->RegisterCommand("map",CMD_MAP,this );
	Sys_GetConsole()->RegisterCommand("connect",CMD_MAP,this);

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

	FILESYSTEM_Free();

	if(g_pConsole)
	{		delete g_pConsole;
		g_pConsole = 0;
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

	//================================
	//We parse the command line and exec configs now
	//once all the subsystems have registers their vars

	//parse Command line
//	ParseCmdLine(lpCmdLine);

	g_pConsole->ExecConfig("default.cfg");
	g_pConsole->ExecConfig("void.cfg");


	//================================
	//Create the window
	g_hWnd = CreateWindow(VOID_MAINWINDOWCLASS, 
						  VOID_MAINWINDOWTITLE,
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


	//================================
	//Init COM librarry
	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		Error("CVoid::Init:Error Initializing COM library\n");
		return false;
	}

	//================================
	//Initialize FileSystem
	if(!g_pFileSystem->Init(g_exedir,VOID_DEFAULTGAMEDIR))
	{
		Error("CVoid::Init:Error Initializing File System\n");
		return false;
	}

	//================================
	//Initialize Console
	if(!g_pConsole->Init(g_pRender->GetConsole()))
	{
		Error("CVoid::Init: Could not Intialize the console");
		return false;
	}

	//================================
	//Initialize the Renderer
	if(!g_pRender->InitRenderer())
	{
		Error("Void::Init:Error Intializing Renderer\n");	
		return false;
	}

	//================================
	//Timer
	g_pTime->Init();


	//================================
	//Update and Show window
	ShowWindow(g_hWnd, SW_NORMAL); 
	UpdateWindow(g_hWnd);

	//================================
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
	//================================
	//Server
	if(!g_pServer->Init())
	{
		Error("CVoid::Init: Could not Initialize Winsock");
		return false;
	}
#endif

#ifdef INCLUDE_SOUND
	//================================
	//Sound 
	if(!g_pSound->Init())
	{
		ComPrintf("CVoid::Init: couldnt init sound system\n");
		delete g_pSound;
		g_pSound = 0;
	}
#endif

#ifdef INCLUDE_MUSIC
	//================================
	//Music
	if(!g_pMusic->Init())
	{
		ComPrintf("CVoid::Init: couldnt music system\n");
		delete g_pMusic;
		g_pMusic = 0;
	}
#endif


#ifndef __VOIDALPHA
	//================================
	//Client, create the client last
	if(!g_pClient->InitNet())
	{
		Error("CVoid::Init: Couldnt not init client socket");
		return false;
	}
#endif

	//Start timer
	g_pTime->Reset();

	//Set focus to console
	GetInputFocusManager()->SetKeyListener(g_pConsole,true);

	//Exec any autoexec file
	g_pConsole->ExecConfig("autoexec.cfg");
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
	
	ShutdownServer();

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

	//Renderer
	if(g_pRender)
		g_pRender->Shutdown();

	//console
	char configname[128];
	sprintf(configname,"%s\\void.cfg",g_exedir);
	WriteConfig(configname);

	if(g_pConsole)
		g_pConsole->Shutdown(); 

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
==========================================
Handle Commands
==========================================
*/
void CVoid::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case CMD_QUIT:
		{
			CFuncQuit(numArgs,szArgs);
			break;
		}
	case CMD_MAP:
		{
			CFuncMap(numArgs,szArgs);
			break;
		}
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
		UnloadWorld();
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

//======================================================================================
//Window Event Handlers
//======================================================================================

/*
==========================================
Move Window Event
==========================================
*/
void CVoid::Move(int x, int y)
{
	if(g_pRender)	
		g_pRender->MoveWindow(x,y);
}

/*
==========================================
Resize Window Event
==========================================
*/
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

/*
==========================================
Activiate window Event
==========================================
*/
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

/*
==========================================
Get Focus Event
==========================================
*/
void CVoid::OnFocus()
{
	if (g_pRinfo)
		g_pRinfo->active = true;
	
	//Input Focus
	if(g_pInput)
		g_pInput->Acquire();
}


/*
==========================================
Lose Focus Event
==========================================
*/
void CVoid::LostFocus()
{
	//Input loses Focus
	if(g_pInput)
		g_pInput->UnAcquire();
	
	//stop rendering
	if (g_pRinfo)
		g_pRinfo->active = false;
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
//	ComPrintf("CVoid::Current Working Directory :%s\n", g_exedir);

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
			g_pConsole->WriteCVars(fp);
			if(g_pClient)
				g_pClient->WriteBindTable(fp);
			fclose(fp);
		}
	}
}

//======================================================================================
//Console loopback functions
//======================================================================================
/*
===============================================
quit game - 
disconnect client + shutdown server + exit game
===============================================
*/
void CVoid::CFuncQuit(int argc, char** argv)
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
void CVoid::CFuncMap(int argc, char** argv)
{
	if(argc >= 2)
	{
		InitServer(argv[1]);
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
		g_pConsole->Toggle(false);
		g_pClient->SetInputState(true);
	}
	else if(g_gameState == INGAME)
	{
		GetInputFocusManager()->SetCursorListener(0);
		GetInputFocusManager()->SetKeyListener(g_pConsole,true);

		g_gameState = INGAMECONSOLE;
		g_pConsole->Toggle(true);
	}
}