#include "Sys_main.h"
#include "Sys_cons.h"
#include "Com_hunk.h"
#include "Com_util.h"

#include <objbase.h>
#include <direct.h>

/*
==========================================
Memory Managers
==========================================
*/
CMemManager		g_memManager("mem_exe.log");
CHunkMem		m_HunkManager;

/*
==========================================
Subsystems
==========================================
*/
I_Renderer  *	g_pRender  =0;	//Renderer
CConsole	*	g_pConsole =0;	//Console

world_t		*	g_pWorld=0;		//The World
extern CVoid*	g_pVoid;

/*
==========================================
Private Definitions
==========================================
*/
namespace
{
	enum
	{
		CMD_QUIT = 0,
		CMD_MAP  = 1
	};
}

//======================================================================================
//======================================================================================

/*
==========================================
Constructor
==========================================
*/
CVoid::CVoid(const char * cmdLine)
{
	//Hack. 
	//Some constructors need to access the System:: funcs, and those depends on the 
	//g_pVoid pointer.but that doesnt get set until this constructor returns
	g_pVoid = this;

	//Current Working directory
	_getcwd(m_exePath,COM_MAXPATH);		

	//Create timer
	m_pTime = new CTime();						

	//Create the game console
	g_pConsole = new CConsole();

	//Export structure
	m_pExport = new VoidExport();
	m_pExport->console    = (I_Console*)g_pConsole;
	m_pExport->hunkManager= g_pHunkManager; 
	
	//Create the file system
	m_pFileSystem = FILESYSTEM_Create(m_pExport);
	
	//Create the input system
	m_pInput= new CInput();					
	
	//Create the Renderer
	g_pRender = RENDERER_Create(m_pExport); 
	m_pRParms = RENDERER_GetParms();
	
	//Create the client
	m_pClient = new CClient();		

	//Network Sys
	m_pServer = new CServer();

#ifdef INCLUDE_SOUND
	//Sound
	m_pSound = new CSoundManager();
#endif

#ifdef INCLUDE_MUSIC
	//Music
	m_pMusic = new CMusic();
#endif

	//Set game state - full screen console - not connected
	m_gameState = INCONSOLE;

	System::GetConsole()->RegisterCommand("quit",CMD_QUIT,this);			
	System::GetConsole()->RegisterCommand("exit",CMD_QUIT,this);			
	System::GetConsole()->RegisterCommand("map",CMD_MAP,this );
	System::GetConsole()->RegisterCommand("connect",CMD_MAP,this);
}

/*
==========================================
Destructor
==========================================
*/
CVoid::~CVoid() 
{
	if(m_pServer)	delete m_pServer;	
	if(m_pClient) 	delete m_pClient;
	
#ifdef INCLUDE_SOUND
	if(m_pSound)	delete m_pSound;
#endif

#ifdef INCLUDE_MUSIC
	if(m_pMusic)	delete m_pMusic;
#endif
	
	if(m_pInput)	delete m_pInput;
	
	if(m_pTime)		delete m_pTime;

	RENDERER_Free();	//Free the Renderer Interface
	FILESYSTEM_Free();	//Free the file system

	if(m_pExport)	delete m_pExport;

	m_HunkManager.PrintStats();

	if(g_pConsole)	delete g_pConsole;
}

/*
==========================================
Initialize all the subsystems
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
	if(!CreateVoidWindow())
	{
		Error("CVoid::Init:Error Creating Window\n");
		return false;
	}
	m_pRParms->hWnd = System::GetHwnd();


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
	if(!m_pFileSystem->Init(m_exePath,VOID_DEFAULTGAMEDIR))
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
	//Update and Show window
	ShowWindow(System::GetHwnd(), SW_NORMAL); 
	UpdateWindow(System::GetHwnd());

	//================================
	//Timer
	m_pTime->Init();

	//================================
	//Input
	if(!m_pInput->Init()) 
	{
		Error("CVoid::Init: Could not Initialize Input");
		return false;
	}

	//================================
	//Server
	if(!VoidNet::InitNetwork())
	{
		Error("CVoid::Init: Could not initalize Winsock");
		return false;
	}
	if(!m_pServer->Init())
	{
		Error("CVoid::Init: Could not Initialize server");
		return false;
	}

	//================================
	//Sound 
#ifdef INCLUDE_SOUND
	if(!m_pSound->Init())
	{
		ComPrintf("CVoid::Init: couldnt init sound system\n");
		delete m_pSound;
		m_pSound = 0;
	}
#endif

	//================================
	//Music
#ifdef INCLUDE_MUSIC
	if(!m_pMusic->Init())
	{
		ComPrintf("CVoid::Init: couldnt init music system\n");
		delete m_pMusic;
		m_pMusic = 0;
	}
#endif

#ifndef __VOIDALPHA
	//================================
	//Client, create the client last
	if(!m_pClient->InitNet())
	{
		Error("CVoid::Init: Couldnt not init client socket");
		return false;
	}
#endif

	//Start timer
	m_pTime->Reset();

	//Set focus to console
	System::GetInputFocusManager()->SetKeyListener(g_pConsole,true);

	//Exec any autoexec file
	g_pConsole->ExecConfig("autoexec.cfg");
	return true;
}

/*
==========================================
Shutdown all the subsystems
==========================================
*/
bool CVoid::Shutdown()
{
	//Subsystem shutdown proceedures
	//=============================

	if(m_pClient && m_pClient->m_ingame)
		m_pClient->Disconnect();
	
	ShutdownServer();

	VoidNet::ShutdownNetwork();

	if(m_pServer)
		m_pServer->Shutdown();

#ifdef INCLUDE_SOUND
	//Sound
	if(m_pSound)
		m_pSound->Shutdown();
#endif

#ifdef INCLUDE_MUSIC
	//music
	if(m_pMusic)
		m_pMusic->Shutdown();
#endif

	//input
	if(m_pInput)
		m_pInput->Shutdown();

	//console
	char configname[128];
	sprintf(configname,"%s\\void.cfg",m_exePath);
	WriteConfig(configname);

	//Renderer
	if(g_pRender)
		g_pRender->Shutdown();

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
	m_pTime->Update();
	
	//Run Input frame
	m_pInput->UpdateCursor();
	m_pInput->UpdateKeys();


#ifdef INCLUDE_SOUND
	m_pSound->RunFrame();
#endif

	m_pServer->RunFrame();

	//Run Client frame
	if(m_pClient->m_ingame)
	//if(m_pClient->m_active)
	{
		m_pClient->RunFrame();
		//draw the scene
		g_pRender->DrawFrame(&m_pClient->eye.origin,&m_pClient->eye.angles);
	}
	else
	{
		//draw the console or menues etc
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
/*	if(m_pClient->m_connected && !m_pClient->Disconnect())
	{
		ComPrintf("CVoid::InitServer:Unable to disconnect Client\n");
		return false;
	}
*/

	char worldname[128];
	char mapname[128];
	
	strcpy(mapname, map);
	Util::SetDefaultExtension(mapname,".bsp");
	
//	sprintf(worldname,"%s\\%s\\worlds\\%s",g_exedir,g_gamedir,mapname);
	
	strcpy(worldname,"Worlds/");
	strcat(worldname,mapname);
	
	if(g_pWorld != 0)
	{
		m_pClient->UnloadWorld();
		UnloadWorld();
	}

	g_pWorld = 0;
	g_pWorld = world_create(worldname);

	if(!g_pWorld)
	{
		ComPrintf("CVoid::InitGame: couldnt load map\n");
		return false;
	}

	m_pClient->LoadWorld(g_pWorld);
	m_pClient->m_connected = true;
	m_pClient->m_ingame = true;
	m_pClient->SetInputState(true);
	

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

	m_pClient->UnloadWorld();
	UnloadWorld();

/*	//destroy the world
	if(g_pWorld)
		world_destroy(g_pWorld);
	g_pWorld = 0;
*/
	m_gameState = INCONSOLE;
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
		m_pRParms->active = false;
		return;
	}

	m_pRParms->active = true;

	//Change the size of the rendering window
	if (g_pRender && !(m_pRParms->rflags & RFLAG_FULLSCREEN))
		g_pRender->Resize();

	//Set Window extents for input if full screen
	if(m_pInput)
		m_pInput->Resize(x,y,w,h);
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
		m_pRParms->active = false;
//		if(m_pInput)
//			m_pInput->UnAcquire();
	}
	else 
	{
		m_pRParms->active = true;
		if(m_pInput)
			m_pInput->Acquire();

		if (g_pRender && (m_pRParms->rflags & RFLAG_FULLSCREEN))
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
	if (m_pRParms)
		m_pRParms->active = true;
	
	//Input Focus
	if(m_pInput)
		m_pInput->Acquire();
}


/*
==========================================
Lose Focus Event
==========================================
*/
void CVoid::LostFocus()
{
	//Input loses Focus
	if(m_pInput)
		m_pInput->UnAcquire();
	
	//stop rendering
	if (m_pRParms)
		m_pRParms->active = false;
}

/*
==========================================

==========================================
*/
void CVoid::HandleMM(WPARAM wParam, LPARAM lParam)
{
#ifdef INCLUDE_MUSIC
	if(m_pMusic)
		m_pMusic->HandleMCIMsg(wParam,lParam);
#endif
}

//======================================================================================
//======================================================================================

/*
==========================================
Parse Command line, update any CVar values
==========================================
*/
void CVoid::ParseCmdLine(const char * lpCmdLine)
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
	Util::ShowMessageBox(textBuffer);

	//Win32
	PostMessage(System::GetHwnd(),	// handle of destination window 
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
			if(m_pClient)
				m_pClient->WriteBindTable(fp);
			fclose(fp);
		}
	}
}

//======================================================================================
//Console loopback functions
//======================================================================================

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
			CFuncQuit();
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
===============================================
quit game - 
disconnect client + shutdown server + exit game
===============================================
*/
void CVoid::CFuncQuit()
{
	ComPrintf("CVoid::Quit\n");

	//Win32 func
	PostMessage(System::GetHwnd(),	// handle of destination window 
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
			m_pClient->ConnectTo("loopback",36666);
		}
		else
			ComPrintf("CVoid::Map: couldnt start server\n");
*/
		return;
	}
	ComPrintf("CVoid::Map: invalid arguments\n");
}

//======================================================================================
//======================================================================================

/*
==========================================
Global Access funcs
==========================================
*/
namespace System
{
	const char* GetExePath()    { return g_pVoid->m_exePath; } 
	const char* GetCurrentPath(){ return g_pVoid->m_pFileSystem->GetCurrentPath(); }
	eGameState  GetGameState()  { return g_pVoid->m_gameState;  }
	void SetGameState(eGameState state) { g_pVoid->m_gameState = state; }
	I_InputFocusManager * GetInputFocusManager(){ return g_pVoid->m_pInput->GetFocusManager(); }

	void ToggleConsole()
	{
		if(GetGameState() == INGAMECONSOLE)
		{
			SetGameState(INGAME);
			g_pConsole->Toggle(false);
			g_pVoid->m_pClient->SetInputState(true);
		}
		else if(GetGameState() == INGAME)
		{
			GetInputFocusManager()->SetCursorListener(0);
			GetInputFocusManager()->SetKeyListener(g_pConsole,true);

			SetGameState(INGAMECONSOLE);
			g_pConsole->Toggle(true);
		}
	}
}
