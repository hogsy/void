#ifndef VOID_SYS_MAIN
#define VOID_SYS_MAIN

#define INCLUDE_MUSIC
#define INCLUDE_SOUND

#include "Sys_hdr.h"
#include "Sys_time.h"
#include "In_main.h"
#include "Snd_main.h"
#include "Mus_main.h"
#include "Sv_main.h"
#include "I_renderer.h"
#include "I_filesystem.h"
#include "Sys_exp.h"
#include "Cl_main.h"
#include "Sys_cons.h"

//========================================================================================

#define VOID_MAINWINDOWCLASS "Void"
#define VOID_MAINWINDOWTITLE "Void"

#define VOID_DEFAULTGAMEDIR	"Game"

//========================================================================================

class CVoid : public I_ConHandler
{
public:
		
	CVoid(const char * cmdLine);			//Constructor
	~CVoid();								//Destructor
	
	bool Init();							//Init subsystems
	void RunFrame();						//Game Loop

	//Console Handler
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms) { return false; } 

	void Error(char *error, ...);

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
	friend const char * System::GetExePath();
	friend const char * System::GetCurrentPath();
	friend I_Console  *	System::GetConsole();
	friend eGameState   System::GetGameState();
	friend void	System::SetGameState(eGameState state);
	friend I_InputFocusManager * System::GetInputFocusManager();
	friend I_SoundManager * System::GetSoundManager();
	
	friend void ComPrintf(char* text, ...);
	
	//Subsystems
	//=========================================================
	
	CConsole	   m_Console;		//Console
	
	I_Renderer   * m_pRender;
	RenderInfo_t * m_pRParms;		//Current Renderering info

	VoidExport   * m_pExport;		//Exported Data
	
	CInput		 * m_pInput;		//Input 
	CTime		 * m_pTime;			//Timer
	CFileSystem  * m_pFileSystem;	//FileSystem

	CServer		 * m_pServer;		//Server
	CClient		 * m_pClient;		//Client and UI

#ifdef INCLUDE_MUSIC
	CMusic		 * m_pMusic;		//Music subsystem
#endif
#ifdef INCLUDE_SOUND
	CSoundManager* m_pSound;		//Sound subsystem
#endif

	//=========================================================

	char		   m_exePath[COM_MAXPATH];
	eGameState	   m_gameState;

	bool CreateVoidWindow();
	void ParseCmdLine(const char * cmdLine);	//Parse Cmd Line
	void WriteConfig(const char *config);		//Write configuration file
	void ToggleConsole();
	void Quit();				
};

#endif