#include "Sys_hdr.h"
#include "Sys_cons.h"
#include "In_main.h"
#include "Mus_main.h"
#include "Sv_defs.h"
#include "I_renderer.h"
#include "I_file.h"
#include "I_filesystem.h"
#include "Cl_main.h"
#include "Snd_main.h"

#include "Sys_main.h"
#include "Com_hunk.h"
#include "Com_util.h"
#include "Com_release.h"

#include <objbase.h>

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
		CMD_TOGGLECONS = 1,
		CMD_LISTFILES = 2,
		CMD_LISTPATHS = 3,
		CMD_DIRPATH = 4
	};
}

extern CVoid *	g_pVoid;

/*
==========================================
Constructor
==========================================
*/
CVoid::CVoid(const char * curDir, const char * cmdLine, CConsole * pConsole) : 
						m_pConsole(pConsole),
						m_pExport(0),
						m_pRender(0),
						m_pRParms(0),
						m_pInput(0),
						m_pFileSystem(0),
						m_pClient(0),
						m_pSound(0),
						m_pMusic(0)
{
	//Some constructors need to access the System:: funcs, and those depends on the 
	//g_pVoid pointer.but that doesnt get set until this constructor returns
	g_pVoid = this;

	//================================
	//Add CommandLine
	if(cmdLine && Util::CompareExts(cmdLine,VOID_DEFAULTMAPEXT))
	{
		char map[COM_MAXPATH];
		char parm[COM_MAXPATH];
		Util::ParseFileName(map,COM_MAXPATH,cmdLine);

		sprintf(parm, "map %s", map);
		m_pConsole->AddCmdLineParm(parm);
	}

	m_pConsole->LoadConfig("vvars.cfg");

	//================================
	//Create and initialize the file system
	m_pFileSystem = FILESYSTEM_Create(ComPrintf,
									  System::FatalError,
									  curDir,
									  VOID_DEFAULTGAMEDIR);
	if(!m_pFileSystem)
	{
		System::FatalError("Unable to create the Void filesystem");
		return;
	}

	//================================
	//Export structure
	m_pExport = new VoidExports();
	m_pExport->pConsole    = (I_Console*)m_pConsole;
	m_pExport->pHunkManager= g_pHunkManager; 
	m_pExport->pfnGetCurTime = System::GetCurTime;
	m_pExport->pfnGetCurPath = System::GetCurGamePath;
	m_pExport->pfnSystemError = System::FatalError;
	m_pExport->pfnGetFrameTime = System::GetFrameTime;
	m_pExport->pfnCreateFileReader = System::CreateFileReader;

	//Create the input system
	m_pInput= new CInput();					
	
	//Create the Renderer
	m_pRender = RENDERER_Create(m_pExport); 
	m_pRParms = RENDERER_GetParms();

	//Sound
	m_pSound = new CSoundManager();

	//Music
	m_pMusic = new CMusic();

	//Network Sys
	VoidServer::Create();

	//Set game state - full screen console - not connected
	m_gameState = INCONSOLE;

	//Register these commands before the client is created so it can bind to them
	m_pConsole->RegisterCommand("quit",CMD_QUIT,this);			
	m_pConsole->RegisterCommand("exit",CMD_QUIT,this);			
	m_pConsole->RegisterCommand("contoggle", CMD_TOGGLECONS,this);
	
	m_pConsole->RegisterCommand("fs_listarchives",CMD_LISTFILES,this);
	m_pConsole->RegisterCommand("fs_path",CMD_LISTPATHS,this);
	m_pConsole->RegisterCommand("fs_dir",CMD_DIRPATH,this);

	m_varTimeStamp = m_pConsole->RegisterCVar("sys_timestamp", __TIMESTAMP__, CVAR_STRING, CVAR_READONLY,this);
	m_varVersion = m_pConsole->RegisterCVar("sys_version", VOID_VERSION, CVAR_STRING, CVAR_READONLY,this);
}

/*
==========================================
Initialize all the subsystems
==========================================
*/
bool CVoid::Init()
{
	//================================
	//Init COM librarry
	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		System::FatalError("CVoid::Init:Error Initializing COM library");
		return false;
	}

	//================================
	//Create the window
	if(!CreateVoidWindow())
	{
		System::FatalError("CVoid::Init:Error Creating Window");
		return false;
	}
	m_pRParms->hWnd = System::GetHwnd();

	//================================
	//Check whether filesystem is okay
	if(!m_pFileSystem->IsActive())
	{
		System::FatalError("CVoid::Init:Error Initializing File System");
		return false;
	}

	//================================
	//Initialize Console
	m_pConsole->SetConsoleRenderer(m_pRender->GetConsole());


	//================================
	//Initialize the Renderer
	if(!m_pRender->InitRenderer())
	{
		System::FatalError("Void::Init:Error Intializing Renderer");	
		return false;
	}

	//================================
	//Timer
	m_Time.Init();

	//================================
	//Update and Show window
	::ShowWindow(System::GetHwnd(), SW_SHOW); 
	::UpdateWindow(System::GetHwnd());
	::SetForegroundWindow(System::GetHwnd());

 	//================================
	//Input
	if(!m_pInput->Init()) 
	{
		System::FatalError("CVoid::Init: Could not Initialize Input");
		return false;
	}

	//================================
	//Server
	if(!VoidServer::InitializeNetwork())
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
	if(!m_pMusic->Init())
	{
		System::FatalError("CVoid::Init: Could not initalize Music System");
		return false;
	}

	//================================
	//Client, create the client last
	m_pClient = new CClient(m_pRender, m_pSound, m_pMusic);		

	//Start timer
	m_Time.Reset();

	//Set focus to console
	System::GetInputFocusManager()->SetKeyListener(m_pConsole,true);

	//Exec any autoexec files and the commandLine
	m_pConsole->ExecCmdLine();
	m_pConsole->ExecConfig("autoexec.cfg");
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
	m_pConsole->WriteCVars("vvars.cfg");

	if(m_pClient)
		delete m_pClient;
	
	VoidServer::Destroy();
	VoidServer::ShutdownNetwork();

	if(m_pSound)	
		delete m_pSound;

	if(m_pMusic)
		delete m_pMusic;
	
	//Shutdown, and free the Renderer Interface
	if(m_pRender)
		m_pRender->Shutdown();
	
	m_pConsole->SetConsoleRenderer(0);
	m_pConsole = 0;

	if(m_pInput)
	{
		delete m_pInput;
		m_pInput = 0;
	}

	//Free the file system
	RENDERER_Free();
	FILESYSTEM_Free();	

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

	VoidServer::RunFrame();

	//Client will handle drawing as well.
	//Can be in the console , in the UI, or in the game.
	m_pClient->RunFrame();

	//Commands are run at the end of the frame
	m_pConsole->ExecCommands();
}

//======================================================================================
//Window Event Handlers
//======================================================================================

/*
==========================================
Move Window Event
==========================================
*/
void CVoid::OnMove(int x, int y)
{
	if(m_pRender)	
		m_pRender->MoveWindow(x,y);
}

/*
==========================================
Resize Window Event
==========================================
*/
void CVoid::OnResize(bool focus, int x, int y, int w, int h)
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
================================================
Handle Display Change
================================================
*/
void CVoid::OnDisplayChange(int bpp, int width, int height)
{
	if(m_pRParms->rflags & RFLAG_FULLSCREEN)
		m_pInput->ShowMouse(false);
}


/*
==========================================
Activiate window Event
==========================================
*/
void CVoid::OnActivate(bool focus)
{
	if (focus == false)
		m_pRParms->active = false;
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
	
	if(m_pInput)
		m_pInput->Acquire();
}


/*
==========================================
Lose Focus Event
==========================================
*/
void CVoid::OnLostFocus()
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
Hackish way to handle MM messages
==========================================
*/
void CVoid::OnMultiMedia(WPARAM wParam, LPARAM lParam)
{
	if(m_pMusic)
		m_pMusic->HandleMCIMsg(wParam,lParam);
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
==========================================
Handle Commands
==========================================
*/
void CVoid::HandleCommand(int cmdId, const CParms &parms)
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
	case CMD_LISTFILES:
		{
			m_pFileSystem->ListArchiveFiles();
			break;
		}
	case CMD_LISTPATHS:
		{
			m_pFileSystem->ListSearchPaths();
			break;
		}
	case CMD_DIRPATH:
		{
			int numArgs = parms.NumTokens();
			char arg1[80];

			if(numArgs == 2)
			{
				parms.StringTok(1,arg1,80);
				m_pFileSystem->ListFiles(arg1,0);
			}
			else if(numArgs == 3)
			{
				char arg2[80];
				parms.StringTok(1,arg1,80);
				parms.StringTok(2,arg2,80);
				m_pFileSystem->ListFiles(arg1,arg2);
			}
			else
				m_pFileSystem->ListFiles(0,0);
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
	const char* GetCurGamePath(){ return g_pVoid->m_pFileSystem->GetCurrentPath(); }
	eGameState  GetGameState()  { return g_pVoid->m_gameState;  }
	I_InputFocusManager * GetInputFocusManager(){ return g_pVoid->m_pInput->GetFocusManager(); }

	float GetCurTime()	{ return g_pVoid->m_Time.GetCurrentTime(); }
	float GetFrameTime(){ return g_pVoid->m_Time.GetFrameTime(); }

	I_FileReader *  CreateFileReader(EFileMode mode)
	{	return g_pVoid->m_pFileSystem->CreateReader(mode);
	}


	void SetGameState(eGameState state) 
	{
		if(state == g_pVoid->m_gameState)
			return;

		g_pVoid->m_gameState = state; 
		if(state == INCONSOLE)
		{
			g_pVoid->m_pConsole->SetFullscreen(true);
			g_pVoid->m_pConsole->SetVisible(true);
		}
		else if(state == INGAMECONSOLE)
		{
			g_pVoid->m_pConsole->SetFullscreen(false);
			g_pVoid->m_pConsole->SetVisible(true);
		}
		else if(state == INGAME)
		{
			g_pVoid->m_pConsole->SetFullscreen(false);
			g_pVoid->m_pConsole->SetVisible(false);

			g_pVoid->m_pClient->SetInputState(true);
		}
	}
}
