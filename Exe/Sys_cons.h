#ifndef VOID_CONSOLE_CLASS
#define VOID_CONSOLE_CLASS

#include "I_console.h"
#include "I_renderer.h"
#include "In_defs.h"

//#define VOID_DOS_CONSOLE	1

/*
==========================================
Console Command
==========================================
*/
struct CCommand
{
	CCommand(const char * iname, HCMD iid, I_ConHandler * ihandler)
	{
		name = new char[strlen(iname)+1];
		strcpy(name,iname);
		handler = ihandler;
		id = iid;
	}

	CCommand(const CCommand &cmd)
	{
		name = new char[strlen(cmd.name)+1];
		strcpy(name,cmd.name);
		handler = cmd.handler;
		id = cmd.id;
	}

	~CCommand()
	{	delete [] name;
		handler = 0;
	}

	HCMD	id;
	char *	name;
	I_ConHandler  * handler;
};


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
				public I_ConHandler
{
public:

	CConsole();
	~CConsole();

	//==============================================================
	//I_Console Interface

	void RegisterCVar(	CVarBase * var,
						I_ConHandler * handler=0);
	void RegisterCommand(const char *cmdname,
						HCMD id,
						I_ConHandler * handler);

	void UnlatchCVars(I_ConHandler * handler);

	void ComPrint(char* text);

	//just pass a string to be parsed and exec'ed
	void ExecString(const char *string);

	//==============================================================
	//Key Listener interface
	void HandleKeyEvent(const KeyEvent &kevent);

	//==============================================================
	//Command Handler
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms) { return false; } 

	//==============================================================

	void SetConsoleRenderer(I_ConsoleRenderer * prcons);

	void ExecConfig(const char *filename);
	void WriteCVars(FILE *fp);

	//Client comand binding
	CCommand * GetCommandByName(const char * cmdString);
	
	//Console funcs
    void SetFullscreen(bool full);
    void SetVisible(bool down);

private:

	enum
	{	MAX_OLDCMDS = 32
	};

	//==============================================================
	typedef list<CCommand>	 CmdList;
	typedef list<CVarBase*> CVarList;
	
	CmdList		m_lCmds;		//List of registered commands
	CVarList	m_lCVars;

	CParms		m_parms;
	
	string		m_conString;	//Current String in Console
	StringList	m_cmdBuffer;	//Fixed List of previously entered strings
	
	//iterator to keep track of the Command buffer
	StringList::iterator	m_itCmd;		

	//The Console Renderer
	I_ConsoleRenderer	*	m_prCons;

	HANDLE		m_hLogFile;	

#ifdef VOID_DOS_CONSOLE
	HANDLE		m_hOut;			//handle to console output		
	HANDLE		m_hIn;			//handle to console in
#endif

	//==============================================================
	void CVarlist (const CParms &parms);
	void CCmdList (const CParms &parms);
	void CFunctest(const CParms &parms);

};

#endif