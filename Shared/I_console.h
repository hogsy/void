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
	//Cvar Registrations
	virtual void RegisterCVar(	CVar * var,					//The Cvar being registered
								I_CVarHandler * handler=0)=0;//Optional handler

	virtual void RegisterCommand(const char *cmdname,		//Command Name
								HCMD id,					//ID in the registering class
								I_CmdHandler * handler)=0;	//the class registering the command
	//Print Functions
	virtual void ComPrint(char* text)=0;
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