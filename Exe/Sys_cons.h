#ifndef VOID_CONSOLE_CLASS
#define VOID_CONSOLE_CLASS

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
struct I_ConsoleRenderer;

class CConsole: public I_Console,		//Console interface exported to other modules
				public I_InKeyListener	//Key Event listener interface	
				//public I_ConHandler
{
public:

	CConsole();
	~CConsole();

	//==============================================================
	//I_Console Interface

	void RegisterCVar(CVarBase * var,I_ConHandler * handler=0);
	void RegisterCommand(const char *cmdname,HCMD id,I_ConHandler * handler);
	void ComPrint(const char* text);
	bool ExecString(const char *string);

	//==============================================================
	//Key Listener interface
	void HandleKeyEvent(const KeyEvent &kevent);

	//==============================================================
	//Command Handler
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms) { return false; } 

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

	//looks through config file to see if any parms match the given token
	//set parm to that token if found
	bool GetTokenParms(const char * token, CParms * parms);
	int  ReadConfigParm(char *buf, int bufsize, FILE * fp);
	bool IsCmdLineParm(const char * token, int tokenLen);

	//==============================================================
	typedef std::list<CCommand>	 CmdList;
	typedef std::list<CVarBase*> CVarList;
	
	//List of registered commands
	CmdList		m_lCmds;		
	
	//List of registered cvars
	CVarList	m_lCVars;		

	//Hold parsed parms of commandString enterered
	CParms		m_parms;		
	
	//Current String in Console
	std::string	m_conString;	

	//Fixed List of previously entered strings
	StringList	m_cmdBuffer;	
	//iterator to keep track of the Command buffer
	StringList::iterator	m_itCmd;

	StringList  m_configFileParms;
	StringList  m_cmdLineParms;
	
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