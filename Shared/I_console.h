#ifndef INC_CONSOLE_INTERFACE
#define INC_CONSOLE_INTERFACE

#include "Com_parms.h"

//======================================================================================
//======================================================================================

//Pre-declarations
class CVarBase;
typedef int HCMD;

//Handles console variables and commands
struct I_ConHandler
{
	virtual void HandleCommand(HCMD cmdId, const CParms &parms) = 0;
	virtual bool HandleCVar(const CVarBase * cvar, const CParms &parms) = 0;
};

//======================================================================================
//======================================================================================

enum CVarFlags
{
	CVAR_ARCHIVE = 1,
	CVAR_LATCH = 2,
	CVAR_READONLY = 4
};

enum CVarType
{
	CVAR_UNDEFINED,
	CVAR_INT,
	CVAR_FLOAT,
	CVAR_STRING,
	CVAR_BOOL
};

class CVarBase
{
public:

	union
	{
		float fval;
		int	  ival;
		bool  bval;
	};

	//Public vars
	char * name;
	char * string;
	int	   flags;

	virtual void ForceSet(const char *varval)=0;
	virtual void ForceSet(float val)=0;
	virtual void ForceSet(int   val)=0;

	virtual void Set(const char *varval)=0;
	virtual void Set(float val)=0;
	virtual void Set(int val)=0;

protected:

	enum
	{	CVAR_MAXSTRINGLEN =	512
	};
	
	friend class CConsole;

	char * latched_string;
	char * default_string;

	CVarType		type;
	I_ConHandler * handler;
};


//======================================================================================
//======================================================================================

/*
==========================================
Console Interface
==========================================
*/
struct I_Console
{
	//Cvar Registration
	virtual void RegisterCVar(CVarBase * var,				 //The Cvar being registered
							  I_ConHandler * handler=0)=0;	//Optional handler

	//Con Command Registration
	virtual void RegisterCommand(const char *cmdname,		//Command Name
							  HCMD id,						//ID in the registering class
							  I_ConHandler * handler)=0;	//the class registering the command

	//Subsystem has restarted, unlatch its cvars
	virtual void UnlatchCVars(I_ConHandler * handler)=0;

	//Print Functions
	virtual void ComPrint(char* text)=0;

	//pass a string to be exec'ed
	virtual void ExecString(const char *string)=0;
};

#endif