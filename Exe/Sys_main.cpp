#include "Sys_main.h"
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
Private Definitions
==========================================
*/
namespace
{
	enum
	{
		CMD_QUIT = 0,
		CMD_TOGGLECONS = 1,
		CMD_WRITECONFIG = 2,
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
	//Hack. 
	//Some constructors need to access the System:: funcs, and those depends on the 
	//g_pVoid pointer.but that doesnt get set until this constructor returns
	g_pVoid = this;

	//Current Working directory
	_getcwd(m_exePath,COM_MAXPATH);		

	//Create timer
	m_pTime = new CTime();						

	//Export structure
	m_pExport = new VoidExport();
	m_pExport->console    = (I_Console*)&m_Console;
	m_pExport->hunkManager= g_pHunkManager; 
	
	//Create the file system
	m_pFileSystem = FILESYSTEM_Create(m_pExport);
	
	//Create the input system
	m_pInput= new CInput();					
	
	//Create the Renderer
	m_pRender = RENDERER_Create(m_pExport); 
	m_pRParms = RENDERER_GetParms();

	//Register these commands before the client is create so it can bind to them
	System::GetConsole()->RegisterCommand("quit",CMD_QUIT,this);			
	System::GetConsole()->RegisterCommand("exit",CMD_QUIT,this);			
	System::GetConsole()->RegisterCommand("contoggle", CMD_TOGGLECONS,this);
	System::GetConsole()->RegisterCommand("writeconfig",CMD_WRITECONFIG, this);
	
	//Create the client
	m_pClient = new CClient(m_pRender);		

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

//	m_Console.ExecConfig("default.cfg");
	m_Console.ExecConfig("void.cfg");


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
	m_Console.SetConsoleRenderer(m_pRender->GetConsole());


	//================================
	//Initialize the Renderer
	if(!m_pRender->InitRenderer())
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
	System::GetInputFocusManager()->SetKeyListener(&m_Console,true);

	//Exec any autoexec file
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
	char configname[128];
	sprintf(configname,"%s\\void.cfg",m_exePath);
	WriteConfig(configname);

	if(m_pServer)	
		delete m_pServer;	

	if(m_pClient)
	{
//		m_pClient->UnloadWorld();
		delete m_pClient;
	}

	VoidNet::ShutdownNetwork();
	
#ifdef INCLUDE_SOUND
	if(m_pSound)	
	{
		m_pSound->Shutdown();
		delete m_pSound;
	}
#endif

#ifdef INCLUDE_MUSIC
	if(m_pMusic)
	{
		m_pMusic->Shutdown();
		delete m_pMusic;
	}
#endif
	
	if(m_pInput)
	{
		m_pInput->Shutdown();
		delete m_pInput;
	}
	
	if(m_pTime)
		delete m_pTime;

	//Shutdown, and free the Renderer Interface
	if(m_pRender)
		m_pRender->Shutdown();

	m_HunkManager.PrintStats();

	//Free the file system
	FILESYSTEM_Free();	
	RENDERER_Free();

	if(m_pExport)
		delete m_pExport;
	
	CoUninitialize();
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
	
	//Will handle menu/UI/world/HUD drawing
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
		m_pRender->MoveWindow(x,y);
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
	//Write the Config file
	if(config)
	{
		FILE * fp = fopen(config,"w");
		if(fp != NULL)
		{
			m_Console.WriteCVars(fp);
			if(m_pClient)
				m_pClient->WriteBindTable(fp);
			fclose(fp);
		}
	}
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
	case CMD_WRITECONFIG:
		{
			WriteConfig(parms.StringTok(1));
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
	I_SoundManager * GetSoundManager() { return g_pVoid->m_pSound; }

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
}

/*
===============================================
print a string to debugging window 
and handle any arguments
===============================================
*/
void ComPrintf(char* text, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, text);
	vsprintf(textBuffer, text, args);
	va_end(args);
		
	g_pVoid->m_Console.ComPrint(textBuffer);
}