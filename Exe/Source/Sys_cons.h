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
	CCommand(const char * iname, HCMD iid, I_CmdHandler * ihandler)
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
	I_CmdHandler * handler;
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
				public I_CmdHandler
{
public:

	CConsole();
	~CConsole();

	//==============================================================
	//I_Console Interface

	void RegisterCVar(	CVarBase * var,
						I_CVarHandler * handler=0);
	void RegisterCommand(const char *cmdname,
						HCMD id,
						I_CmdHandler * handler);
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

	enum
	{	MAX_OLDCMDS = 32
	};

	//==============================================================

	typedef std::list<CCommand>	 CmdList;
	typedef std::list<CVarBase*> CVarList;
	
	CmdList		m_lCmds;		//List of registered commands
	CVarList	m_lCVars;
	
	FILE  *		m_pflog;		//log file

	char **		m_szargv;		//console arguments	
	
	std::string	m_conString;	//Current String in Console
	StringList	m_cmdBuffer;	//Fixed List of previously entered strings
	
	//iterator to keep track of the Command buffer
	StringList::iterator	m_itCmd;		

	//The Console Renderer
	I_ConsoleRenderer	*	m_prCons;

#ifdef VOID_DOS_CONSOLE
	HANDLE		m_hOut;			//handle to console output		
	HANDLE		m_hIn;			//handle to console in
#endif

	//==============================================================

	bool Exec	  (int argc, char** argv);	
	void CVarlist (int argc, char** argv);
	void CCmdList (int argc, char** argv);
	void CFunctest(int argc, char** argv);

};

extern CConsole *g_pConsole;

#endif