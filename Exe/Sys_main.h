#ifndef VOID_SYS_MAIN
#define VOID_SYS_MAIN

#include "Sys_time.h"

//========================================================================================

const char VOID_MAINWINDOWCLASS[]	= "Void";
const char VOID_MAINWINDOWTITLE[]	= "Void";

struct  I_Renderer;
struct  I_FileSystem;
struct  I_InputFocusManager;

struct  VoidExports;
struct 	RenderInfo_t;

class   CConsole;
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
		  const char * cmdLine,
		  CConsole * pConsole);			
	~CVoid();								//Destructor
	
	bool Init();							//Init subsystems
	void RunFrame();						//Game Loop

	//Console Handler
	void HandleCommand(int cmdId, const CParms &parms);
	bool HandleCVar(const CVar * cvar, const CStringVal &strVal) 
	{ return false; 
	}

	//Application Events
	void OnMove(int x, int y);
	void OnResize( bool focus, int x, int y, int w, int h);
	void OnActivate(bool focus);
	void OnDisplayChange(int bpp, int width, int height);
	void OnFocus();
	void OnLostFocus();
	void OnMultiMedia(uint wParam, long lParam);	//MCI Multimedia event

private:

	//Give this friend access
	friend float System::GetCurTime();
	friend float System::GetFrameTime();
	friend const char * System::GetCurGamePath();
	friend eGameState   System::GetGameState();
	
	friend I_InputFocusManager * System::GetInputFocusManager();
	friend I_FileReader *  System::CreateFileReader(EFileMode mode);
	
	friend void	System::SetGameState(eGameState state);
	
	//Subsystems
	//=========================================================
	
	CTime		   m_Time;

	CConsole	 * m_pConsole;		//Console
	I_FileSystem * m_pFileSystem;	//FileSystem

	VoidExports	 * m_pExport;		//Exported Stuff
	
	I_Renderer   * m_pRender;		//Renderer
	RenderInfo_t * m_pRParms;		//Current Renderering info

	CInput		 * m_pInput;		//Input 
	CSoundManager* m_pSound;		//Sound subsystem
	CMusic		 * m_pMusic;		//Music subsystem

	CClient		 * m_pClient;		//Client and UI

	//=========================================================
	CVar *		   m_varTimeStamp;
	CVar *		   m_varVersion;

	eGameState	   m_gameState;

	bool CreateVoidWindow();
	void ToggleConsole();
	void Quit();				
};

#endif