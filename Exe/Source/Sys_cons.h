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

class CConsole: public I_Console,	//Console interface exported to other modules
				public I_InKeyListener	//Key Event listener interface	
{
public:

	//==============================================================
	//I_ExeConsole Interface

	//Print Function
	void dprint(char* text);

	//CVar Registration
/*	
	void RegisterCVar(CVar **cvar, 
					  const char *varname, 
					  const char *varval,		//scanned to sting/float/int 
					  CVar::CVarType vartype,	//var type - can be float/int/char * etc
					  int varflags,				//extra parm, locked vars etc
					  CVAR_FUNC varfunc=0);		//validation func
*/
	CVar * RegisterCVar(const char *varname, 
					    const char *varval,		//scanned to sting/float/int 
					    CVar::CVarType vartype,	//var type - can be float/int/char * etc
					    int varflags,			//extra parm, locked vars etc
					    CVAR_FUNC varfunc=0);	//validation func

	void RegisterCFunc(const char *funcname,
					  CFUNC pfunc);

	void CVarSet(CVar **cvar, const char *varval);
	void CVarForceSet(CVar **cvar, const char *varval);
	void CVarSet(CVar **cvar, float val);
	void CVarForceSet(CVar **cvar, float val);

	//==============================================================
	//Key Listener interface
	void HandleKeyEvent(const KeyEvent_t &kevent);


	//==============================================================

	CConsole();
	~CConsole();

	bool Init(I_ConsoleRenderer * prcons);
	bool Shutdown();

	//Message Boxes
	void MsgBox(char *caption, unsigned long boxType, char *msg, ...);
	void MsgBox(char *msg, ...);

	void ExecConfig(const char *filename);
	void WriteCVars(FILE *fp);

	//just pass a string to be parsed and exec'ed
	void ExecString(const char *string);

	CFUNC GetFuncByName(const char * fname);

	//Console funcs
    void ToggleFullscreen(bool full);
    void Toggle(bool down);

private:

	void HandleInput(const int &c);

	//see if name matches, if it does, exec func
	bool Exec(int argc, char ** argv);

	friend void CVarlist(int argc, char** argv);
	friend void CFunclist(int argc, char** argv);

	CPtrList<CVar>  *m_pcList;		//List of Cvars
	CPtrList<CFunc> *m_pfList;		//List of Cfuncs

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