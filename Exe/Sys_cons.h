#ifndef VOID_CONSOLE_CLASS
#define VOID_CONSOLE_CLASS

#include "In_defs.h"

struct I_ConsoleRenderer;
class  CVarImpl;

/*
==========================================
Console Command
==========================================
*/
struct CCommand
{
	CCommand(const char * iname, int iid, I_ConHandler * pHandler)
	{
		name = new char[strlen(iname)+1];
		strcpy(name,iname);
		handler = pHandler;
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

	int	id;
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

	CConsole(const char * curPath);
	~CConsole();

	//==============================================================
	//I_Console Interface
	CVar * RegisterCVar(const char * varName,
						const char *varval, 
						CVarType vartype,	
						int varflags,
						I_ConHandler * pHandler);

	void UnregisterHandler(I_ConHandler * pHandler);
	void RegisterCommand(const char *cmdname, int id, I_ConHandler * pHandler);
	void ComPrint(const char* text);
	bool ExecString(const char *string);

	//==============================================================
	//Key Listener interface
	void HandleKeyEvent(const KeyEvent &kevent);

	//==============================================================
	//Command Handler
	void HandleCommand(int cmdId, const CParms &parms);
	bool HandleCVar(const CVar * cvar, const CStringVal &strVal) { return false; } 

	//==============================================================

	void SetConsoleRenderer(I_ConsoleRenderer * prcons);

	//Configs, commandline Parms etc
	void AddCmdLineParm(const char * cmdLine);
	void ExecCmdLine();

	void LoadConfig(const char * szFilename);
	void ExecConfig(const char * szFilename);
	void WriteCVars(const char * szFilename);

	//Client comand binding
	CCommand * GetCommandByName(const char * cmdString);
	
	//Console viewing
    void SetFullscreen(bool full);
    void SetVisible(bool down);

private:

	enum
	{	MAX_CONSOLE_BUFFER = 1024
	};

	void UpdateCVarFromArchive(CVarImpl * pCvar);
	void WriteCVarToArchive(CVarImpl * pCvar);

	//Helper funcs
	int  ReadConfigParm(char *buf, int bufsize, FILE * fp);
	bool IsCmdLineParm(const char * token, int tokenLen);

	//==============================================================
	typedef std::list<CCommand>	 CmdList;
	typedef std::list<CVarImpl*> CVarList;
	typedef std::list<CCommand>::iterator  CmdListIt;
	typedef std::list<CVarImpl*>::iterator CVarListIt;
	
	CmdList		m_lCmds;		//Currently registered commands
	CVarList	m_lCVars;		//Currently registered cvars

	std::string	m_conString;	//Current String in Console

	StrList		m_cmdBuffer;	//Commands entered in the console
	StrListIt	m_itCmd;		//iterator to keep track of the Commands buffer

	//Parsed parms of string enterered
	CParms		m_parms;
	char		m_szParmBuffer[MAX_CONSOLE_BUFFER];

	//List of Cvar Strings. 
	//Loaded from configs on startup.
	//Updated when handlers are unregistered
	//Written to config file on exit
	StrList		m_cvarStrings;	
	//List of command line parms 
	StrList		m_cmdLineParms;
	
	HANDLE		m_hLogFile;

	//The Console Renderer
	I_ConsoleRenderer	*	m_prCons;

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