#ifndef VOID_SYS_MAIN
#define VOID_SYS_MAIN

#define INCLUDE_MUSIC

#include "Sys_cons.h"
#include "Sys_time.h"

//========================================================================================

const char VOID_MAINWINDOWCLASS[]	= "Void";
const char VOID_MAINWINDOWTITLE[]	= "Void";
const char VOID_DEFAULTBINARYNAME[]	= "Void.exe";


struct	VoidExport;
struct  I_Renderer;
struct  I_FileSystem;
struct 	RenderInfo_t;

class	CInput;
class	CFileSystem;
class	CClient;
class	CSoundManager;
class	CMusic;

//========================================================================================

class CVoid : public I_ConHandler
{
public:
		
	//Constructor
	CVoid(const char * curDir, 
		  const char * cmdLine);			
	~CVoid();								//Destructor
	
	bool Init();							//Init subsystems
	void RunFrame();						//Game Loop

	//Console Handler
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms) { return false; } 

	//Application Events
	void Move(int x, int y);
	void Resize( bool focus, int x, int y, int w, int h);
	void Activate(bool focus);
	void OnFocus();
	void LostFocus();
	
	//hack
	void HandleMM(uint wParam, long lParam);	//MCI Multimedia event

private:

	//Give this friend access
	friend const float& System::GetCurTime();
	friend const float& System::GetFrameTime();
	friend const char * System::GetExePath();
	friend const char * System::GetCurGamePath();
	friend I_Console  *	System::GetConsole();
	friend eGameState   System::GetGameState();
	
	friend I_InputFocusManager * System::GetInputFocusManager();
	
	friend void System::FatalError(const char *error);
	friend void	System::SetGameState(eGameState state);
	
	friend void ComPrintf(const char* text, ...);
	
	//Subsystems
	//=========================================================
	
	CConsole	   m_Console;		//Console
	CTime		   m_Time;

	VoidExport   * m_pExport;		//Exported Stuff
	
	I_FileSystem * m_pFileSystem;	//FileSystem
	
	I_Renderer   * m_pRender;		//Renderer
	RenderInfo_t * m_pRParms;		//Current Renderering info

	CInput		 * m_pInput;		//Input 
	CSoundManager* m_pSound;		//Sound subsystem
	CMusic		 * m_pMusic;		//Music subsystem

	CClient		 * m_pClient;		//Client and UI

	//=========================================================

	eGameState	   m_gameState;

	bool CreateVoidWindow();
	void ToggleConsole();
	void Quit();				
};

#endif