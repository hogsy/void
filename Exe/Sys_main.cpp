#include "Sys_hdr.h"
#include "In_main.h"
#include "Mus_main.h"
#include "Sv_main.h"
#include "I_renderer.h"
#include "I_filesystem.h"
#include "Sys_exp.h"
#include "Cl_main.h"
#include "Snd_main.h"

#include "Sys_main.h"
#include "Com_hunk.h"
#include "Com_util.h"

#include <objbase.h>
#include <direct.h>

/*
==========================================
Memory stuff
==========================================
*/
const char MEM_SZLOGFILE [] = "mem_exe.log";
CHunkMem   m_HunkManager;

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
		CMD_TOGGLECONS = 1
	};
}

//======================================================================================
//======================================================================================

extern CVoid*	g_pVoid;

/*
==========================================
Constructor
==========================================
*/
CVoid::CVoid(const char * cmdLine)
{
	//Current Working directory
	_getcwd(m_exePath,COM_MAXPATH);

	//Hack. 
	//Some constructors need to access the System:: funcs, and those depends on the 
	//g_pVoid pointer.but that doesnt get set until this constructor returns
	g_pVoid = this;


	//Add CommandLine
	if(cmdLine && Util::CompareExts(cmdLine,VOID_DEFAULTMAPEXT))
	{
		char map[COM_MAXPATH];
		char parm[COM_MAXPATH];
		Util::ParseFileName(map,COM_MAXPATH,cmdLine);

		sprintf(parm, "map %s", map);
		m_Console.AddCmdLineParm(parm);
	}

	m_Console.LoadConfig("vvars.cfg");

	//Export structure
	m_pExport = new VoidExport();
	m_pExport->console    = (I_Console*)&m_Console;
	m_pExport->hunkManager= g_pHunkManager; 
	
	//================================
	//Create and initialize the file system
	m_pFileSystem = FILESYSTEM_Create(m_pExport, 
									  m_exePath,
									  VOID_DEFAULTGAMEDIR);
	//Create the input system
	m_pInput= new CInput();					
	
	//Create the Renderer
	m_pRender = RENDERER_Create(m_pExport); 
	m_pRParms = RENDERER_GetParms();

	//Register these commands before the client is create so it can bind to them
	System::GetConsole()->RegisterCommand("quit",CMD_QUIT,this);			
	System::GetConsole()->RegisterCommand("exit",CMD_QUIT,this);			
	System::GetConsole()->RegisterCommand("contoggle", CMD_TOGGLECONS,this);

	//Sound
	m_pSound = new CSoundManager();

#ifdef INCLUDE_MUSIC
	//Music
	m_pMusic = new CMusic();
#endif

	//Network Sys
	m_pServer = new CServer();
	
	//Create the client
	m_pClient = new CClient(m_pRender, m_pSound, m_pMusic);		

	//Set game state - full screen console - not connected
	m_gameState = INCONSOLE;
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

	//================================
	//Create the window
	if(!CreateVoidWindow())
	{
		System::FatalError("CVoid::Init:Error Creating Window");
		return false;
	}
	m_pRParms->hWnd = System::GetHwnd();

	//================================
	//Init COM librarry
	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		System::FatalError("CVoid::Init:Error Initializing COM library");
		return false;
	}

	if(!m_pFileSystem->IsActive())
	{
		System::FatalError("CVoid::Init:Error Initializing File System");
		return false;
	}

	//================================
	//Initialize Console
	m_Console.SetConsoleRenderer(m_pRender->GetConsole());


	//================================
	//Initialize the Renderer
	if(!m_pRender->InitRenderer())
	{
		System::FatalError("Void::Init:Error Intializing Renderer");	
		return false;
	}

	//================================
	//Update and Show window
	ShowWindow(System::GetHwnd(), SW_NORMAL); 
	UpdateWindow(System::GetHwnd());

	//================================
	//Timer
	m_Time.Init();

	//================================
	//Input
	if(!m_pInput->Init()) 
	{
		System::FatalError("CVoid::Init: Could not Initialize Input");
		return false;
	}

	//================================
	//Server
	if(!CServer::InitNetwork())
	{
		System::FatalError("CVoid::Init: Could not initalize Winsock");
		return false;
	}

	//================================
	//Sound 
	if(!m_pSound->Init())
	{
		System::FatalError("CVoid::Init: Could not initalize Sound System");
		return false;
	}

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

/*	//================================
	//Client, create the client last
	if(!m_pClient->InitNet())
	{
		System::FatalError("CVoid::Init: Couldnt not init client socket");
		return false;
	}
*/

	//Start timer
	m_Time.Reset();

	//Set focus to console
	System::GetInputFocusManager()->SetKeyListener(&m_Console,true);

	//Exec any autoexec files and the commandLine
	m_Console.ExecCmdLine();
	m_Console.ExecConfig("autoexec.cfg");
	return true;
}

/*
==========================================
Destructor
==========================================
*/
CVoid::~CVoid() 
{
	//console
	m_Console.WriteCVars("vvars.cfg");

	if(m_pClient)
		delete m_pClient;
	
	if(m_pServer)	
		delete m_pServer;	

	CServer::ShutdownNetwork();
	
	if(m_pSound)	
		delete m_pSound;

#ifdef INCLUDE_MUSIC
	if(m_pMusic)
	{
		m_pMusic->Shutdown();
		delete m_pMusic;
	}
#endif
	
	//Shutdown, and free the Renderer Interface
	if(m_pRender)
		m_pRender->Shutdown();
	m_Console.SetConsoleRenderer(0);

	if(m_pInput)
	{
		m_pInput->Shutdown();
		delete m_pInput;
	}

	//Free the file system
	FILESYSTEM_Free();	
	RENDERER_Free();

	if(m_pExport)
		delete m_pExport;

	m_HunkManager.PrintStats();
	
	CoUninitialize();
}

/*
==========================================
The Game Loop
==========================================
*/
void CVoid::RunFrame()
{
	m_Time.Update();
	
	//Run Input frame
	m_pInput->UpdateCursor();
	m_pInput->UpdateKeys();

	m_pSound->RunFrame();

	m_pServer->RunFrame();

	//Client will handle drawing as well.
	//Can be in the console , in the UI, or in the game.
	m_pClient->RunFrame();
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
	if(m_pRender)	
	{
//ComPrintf("WINDOW MOVE\n");
		m_pRender->MoveWindow(x,y);
	}
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

	//If changing to fullscreem, /./then make sure the input is exclusive
	if(m_pRParms->rflags & RFLAG_FULLSCREEN)
	{
//ComPrintf("SET INPUT EX\n");
		m_pInput->SetExclusive(true);
	}
	else if(!m_pInput->GetExclusiveVar())
	{
//ComPrintf("SET INPUT NON-EX\n");
		m_pInput->SetExclusive(false);
	}
		
	//Change the size of the rendering window
	if (m_pRender && !(m_pRParms->rflags & RFLAG_FULLSCREEN))
		m_pRender->Resize();

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
//ComPrintf("Win: Activate");
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

		if (m_pRender && (m_pRParms->rflags & RFLAG_FULLSCREEN))
			m_pRender->Resize();
	}
}

/*
==========================================
Get Focus Event
==========================================
*/
void CVoid::OnFocus()
{
//ComPrintf("Focus: Activate");
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


//======================================================================================
//Console loopback functions
//======================================================================================

/*
===============================================
quit game - 
disconnect client + shutdown server + exit game
===============================================
*/
void CVoid::Quit()
{
	ComPrintf("CVoid::Quit\n");

	//Win32 func
	PostMessage(System::GetHwnd(),	// handle of destination window 
				WM_QUIT,			// message to post 
				0,					// first message parameter 
				0);					// second message parameter 
}

/*
==========================================
Toggle Console
==========================================
*/
void CVoid::ToggleConsole()
{
	if(System::GetGameState() == INGAMECONSOLE)
		System::SetGameState(INGAME);
	else if(System::GetGameState() == INGAME)
		System::SetGameState(INGAMECONSOLE);
}

/*
=====================================
Write the configuration file
=====================================
*/
void CVoid::WriteConfig(const char *config)
{
}

/*
==========================================
Handle Commands
==========================================
*/
void CVoid::HandleCommand(HCMD cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_QUIT:
		{
			Quit();
			break;
		}
	case CMD_TOGGLECONS:
		{
			ToggleConsole();
			break;
		}
	}
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
	I_Console * GetConsole()	{ return &(g_pVoid->m_Console); }
	I_InputFocusManager * GetInputFocusManager(){ return g_pVoid->m_pInput->GetFocusManager(); }

	const float & GetCurTime() { return g_pVoid->m_Time.GetCurrentTime(); }
	const float & GetFrameTime()   { return g_pVoid->m_Time.GetFrameTime(); }


	void SetGameState(eGameState state) 
	{
		if(state == g_pVoid->m_gameState)
			return;

		g_pVoid->m_gameState = state; 
		if(state == INCONSOLE)
		{
			g_pVoid->m_Console.SetFullscreen(true);
			g_pVoid->m_Console.SetVisible(true);
		}
		else if(state == INGAMECONSOLE)
		{
			g_pVoid->m_Console.SetFullscreen(false);
			g_pVoid->m_Console.SetVisible(true);
		}
		else if(state == INGAME)
		{
			g_pVoid->m_Console.SetFullscreen(false);
			g_pVoid->m_Console.SetVisible(false);

			g_pVoid->m_pClient->SetInputState(true);
		}
	}

	void FatalError(const char *error)
	{
		Util::ShowMessageBox(error);
		delete g_pVoid;
		exit(1);
	}
}


/*
===============================================
print a string to debugging window 
and handle any arguments
===============================================
*/
void ComPrintf(const char* text, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, text);
	vsprintf(textBuffer, text, args);
	va_end(args);
		
	g_pVoid->m_Console.ComPrint(textBuffer);
}