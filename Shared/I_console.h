#ifndef INC_CONSOLE_INTERFACE
#define INC_CONSOLE_INTERFACE

#include "Com_cvar.h"

/*
==========================================
Exe Console Interface
==========================================
*/
struct I_Console
{
	//Print Functions
	virtual void dprint(char* text)=0;

	//Cvar Registrations
	virtual CVar * RegisterCVar(const char *varname, 
							  const char *varval,		//scanned to sting/float/int 
							  CVar::CVarType vartype,	//var type - can be float/int/char * etc
							  int varflags,				//extra parm, locked vars etc
							  CVAR_FUNC varfunc=0)=0;	//validation func
	
	virtual void RegisterCFunc(const char *funcname,
							  CFUNC pfunc)=0;

	virtual void CVarSet(CVar **cvar, const char *varval)=0;
	virtual void CVarForceSet(CVar **cvar, const char *varval)=0;

	virtual void CVarSet(CVar **cvar, float val)=0;
	virtual void CVarForceSet(CVar **cvar, float val)=0;
};


/*
==========================================
Renderer Console Interface
==========================================
*/
struct I_ConsoleRenderer
{
	virtual void Toggle(bool down) = 0;
	virtual void ToggleFullscreen(bool full) = 0;
	virtual void Lineup() = 0;
	virtual void Linedown() = 0;
	virtual void Statusline(const char  *status_line, const int &len) = 0;
	virtual void AddLine(char *line, int color=0, int size=0) = 0;
};


#endif