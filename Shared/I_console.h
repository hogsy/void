#ifndef INC_INTERFACE_CONSOLE
#define INC_INTERFACE_CONSOLE

#include "Com_cvar.h"

/*
==========================================
Exe Console Interface
==========================================
*/
struct I_ExeConsole
{
	virtual void RegisterCVar( CVar **cvar, 
							  const char *varname, 
							  const char *varval,		//scanned to sting/float/int 
							  CVar::CVarType vartype,	//var type - can be float/int/char * etc
							  int varflags,				//extra parm, locked vars etc
							  CVAR_FUNC varfunc=0)=0;	//validation func
	
	virtual void RegisterCFunc(const char *funcname,
							  CFUNC pfunc)=0;
	//Print Functions
	virtual void dprint(char* text)=0;
};


/*
==========================================
Renderer Console Interface
==========================================
*/
struct I_RConsole
{
	virtual void Toggle(bool down) = 0;
	virtual void ToggleFullscreen(bool full) = 0;
	virtual void Lineup() = 0;
	virtual void Linedown() = 0;
	virtual void Statusline(const char  *status_line, const int &len) = 0;
	virtual void AddLine(char *line, int color=0, int size=0) = 0;
};


#endif