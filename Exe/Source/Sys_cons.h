#ifndef VOID_CONSOLE_CLASS
#define VOID_CONSOLE_CLASS

#include "I_console.h"
#include "In_defs.h"

#include "Com_buffer.h"
#include "Com_queue.h"
#include "Com_list.h"

//#define VOID_DOS_CONSOLE	1
#define CON_MAXARGSIZE 80

/*
==========================================
The Console interface class
Handles 
Cvars/Commands
console input
config files
==========================================
*/

class CConsole: public I_Console,		//Console interface exported to other modules
				public I_InKeyListener,	//Key Event listener interface	
				public I_CmdHandler
{

public:

	CConsole();
	~CConsole();

	//==============================================================
	//I_Console Interface

	//CVar Registration
	void RegisterCVar(	CVar * var,
						I_CVarHandler * handler=0);
	void RegisterCommand(const char *cmdname,
						HCMD id,
						I_CmdHandler * handler);
	//Print Function
	void ComPrint(char* text);

	//==============================================================
	//Key Listener interface
	void HandleKeyEvent(const KeyEvent_t &kevent);

	//==============================================================
	//Command Handler
	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);

	//==============================================================

	bool Init(I_ConsoleRenderer * prcons);
	bool Shutdown();

	//Message Boxes
	void MsgBox(char *caption, unsigned long boxType, char *msg, ...);
	void MsgBox(char *msg, ...);

	void ExecConfig(const char *filename);
	
	void WriteCVars(FILE *fp);

	//just pass a string to be parsed and exec'ed
	void ExecString(const char *string);
	void ExecCommand(CCommand * cmd, const char * cmdString);

	CCommand * GetCommandByName(const char * cmdString);
	
	//Console funcs
    void ToggleFullscreen(bool full);
    void Toggle(bool down);

private:

	void HandleBool   (CVar *var, int argc,  char** argv);
	void HandleInt    (CVar *var, int argc,  char** argv);
	void HandleString (CVar *var, int argc,  char** argv);
	void HandleFloat  (CVar *var, int argc,  char** argv);

	void HandleInput(const int &c);

	//see if name matches, if it does, exec func
	bool Exec(int argc, char ** argv);

	void CVarlist(int argc, char** argv);
	void CCmdList(int argc, char** argv);
	void CFunctest(int argc, char** argv);

	CPRefList<CVar>    *m_pcList;	//List of Cvars
	CPtrList<CCommand> *m_pfList;	//List of Cfuncs

	//Args
	char **			m_szargv;		//console arguments

	I_ConsoleRenderer   *m_prCons;
	FILE				*m_pflog;		//log file

	CSBuffer		 m_szCBuffer;	//Static buffer used to console line
	CQueue<char>	*m_CmdBuffer;	//Command Buffer for doskey type functionality

#ifdef VOID_DOS_CONSOLE
	
	HANDLE	m_hOut;					//handle to console output		
	HANDLE	m_hIn;					//handle to console in

#endif
};

extern CConsole *g_pConsole;

#endif