#ifndef VOID_SYS_MAIN
#define VOID_SYS_MAIN

#define INCLUDE_MUSIC
#define INCLUDE_SOUND

#include "Sys_hdr.h"
#include "Sys_time.h"
#include "In_main.h"
#include "Snd_main.h"
#include "Mus_main.h"
#include "I_renderer.h"
#include "I_filesystem.h"
#include "Sys_exp.h"

//========================================================================================

#define VOID_MAINWINDOWCLASS "Void"
#define VOID_MAINWINDOWTITLE "Void"

#define VOID_DEFAULTGAMEDIR	"Game"

//========================================================================================

class CVoid : public I_CmdHandler
{
public:
		
	CVoid(const char * cmdLine);			//Constructor
	~CVoid();								//Destructor
	
	bool Init();							//Init subsystems
	bool Shutdown();						//Shutdown everything

	void RunFrame();						//Game Loop

	bool LoadWorld(char *worldname);
	bool UnloadWorld();

	bool InitServer(char *map);				//Init game
	bool ShutdownServer();

	void Error(char *error, ...);

	//Application Events
	void Move(int x, int y);
	void Resize( bool focus, int x, int y, int w, int h);
	void Activate(bool focus);
	void OnFocus();
	void LostFocus();

	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);

private:

	//Give these friend access
	friend const char * System::GetExePath();
	friend const char * System::GetCurrentPath();
	friend eGameState   System::GetGameState();
	friend void	System::SetGameState(eGameState state);
	friend I_InputFocusManager * System::GetInputFocusManager();

	char		   m_exePath[COM_MAXPATH];
	eGameState	   m_gameState;

	CInput		 * m_pInput;		//Input 
	CTime		 * m_pTime;			//Timer Class
	CFileSystem  * m_pFileSystem;	//FileSystem

#ifdef INCLUDE_MUSIC
	CMusic		 * m_pMusic;
#endif

#ifdef INCLUDE_SOUND
	CSoundManager* m_pSound;		//Sound Subsystem
#endif

	RenderInfo_t * m_pRParms;		//Current Renderering info
	VoidExport   * m_pExport;		//Exported Data

	bool CreateVoidWindow();
	void ParseCmdLine(const char * cmdLine);	//Parse Cmd Line

	//Write configuration file
	void WriteConfig(char *config);

	void CFuncQuit(int argc, char** argv);		//quit game
	void CFuncMap(int argc, char** argv);		//start local server with map + connect to it

};

#endif