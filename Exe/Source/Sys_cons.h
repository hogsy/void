#ifndef __VOID_CONSOLE__
#define __VOID_CONSOLE__

#include "I_console.h"
#include "Com_defs.h"
#include "Com_buffer.h"
#include "Com_queue.h"
#include "Com_list.h"
#include "In_defs.h"

//#define VOID_DOS_CONSOLE	1

#define CON_MAXARGSIZE 80

/*
==========================================
The Console class
Handles 
Cvars/Commands, 
console input, 
config files
==========================================
*/

class CConsole : public I_ExeConsole
{
public:

	CConsole();
	~CConsole();

	bool Init(I_RConsole * prcons);
	bool Shutdown();

	//CVar Registration
	void RegisterCVar(CVar **cvar, 
					  const char *varname, 
					  const char *varval,		//scanned to sting/float/int 
					  CVar::CVarType vartype,	//var type - can be float/int/char * etc
					  int varflags,				//extra parm, locked vars etc
					  CVAR_FUNC varfunc=0);		//validation func

	void RegisterCFunc(const char *funcname,
					  CFUNC pfunc);

	//Print Function
	void dprint(char* text);

	//Message Boxes
	void MsgBox(char *caption, unsigned long boxType, char *msg, ...);
	void MsgBox(char *msg, ...);

	void ExecConfig(const char *filename);
	void WriteCVars(FILE *fp);

	//just pass a string to be parsed and exec'ed
	void ExecString(const char *string);
	
	//Console funcs
    void ToggleFullscreen(bool full);
    void Toggle(bool down);

	friend void  ConsoleHandleKey(const KeyEvent_t *kevent);

private:

	void UpdateBuffer();
	void HandleInput(const int &c);

	CPtrList<CVar>  *m_pcList;	//List of Cvars
	CPtrList<CFunc> *m_pfList;	//List of Cfuncs

	friend void CVarlist(int argc,  char** argv);
	friend void CFunclist(int argc,  char** argv);

	//see if name matches, if it does, exec func
	bool Exec(int argc, char ** argv);
	
	//Args
	char	m_cwd[COM_MAXPATH];	//current working dir
	char **	m_szargv;			//console arguments

#ifdef VOID_DOS_CONSOLE
	HANDLE	m_hOut;				//handle to console output		
	HANDLE	m_hIn;				//handle to console in
#endif

	CPtrList<CVar>  *m_pcItor;	//Cvar iterator
	CPtrList<CFunc> *m_pfItor;	//Cfunc iterator

	I_RConsole	    *m_prCons;

	FILE			*m_pflog;	//log file

	CSBuffer		 m_szCBuffer;	//Static buffer used to console line
	CQueue<char>* 	 m_CmdBuffer;	//Command Buffer for doskey type functionality
};


extern CConsole *g_pCons;

#endif