#ifndef VOID_CONSOLE_INTERFACE
#define VOID_CONSOLE_INTERFACE

/*
================================================
CVar, Console, and Console Handler interfaces
================================================
*/
struct I_Console;
struct I_ConHandler;

class  CParms;
class  CStringVal;

enum
{
	CVAR_ARCHIVE = 1,
	CVAR_LATCH = 2,
	CVAR_READONLY = 4,
	
	CVAR_MAXSTRINGLEN =	512
};

enum CVarType
{
	CVAR_UNDEFINED,
	CVAR_INT,
	CVAR_FLOAT,
	CVAR_STRING,
	CVAR_BOOL
};

class CVar
{
protected:
	CVar() {}
	virtual ~CVar() {}

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
	char * latched_string;
	char * default_string;
	
	int	     flags;
	CVarType type;

	I_ConHandler * handler;

	virtual void ForceSet(const char *varval)=0;
	virtual void ForceSet(float val)=0;
	virtual void ForceSet(int   val)=0;

	virtual void Set(const char *varval)=0;
	virtual void Set(float val)=0;
	virtual void Set(int val)=0;

	virtual void Unlatch() = 0;
	virtual void Reset() = 0;
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
	static  I_Console * GetConsole();

	//Cvar Registration
	virtual CVar * RegisterCVar(const char * varName,
								const char *varval, 
								CVarType vartype,	
								int varflags,
								I_ConHandler * pHandler)=0;

	//Con Command Registration
	virtual void RegisterCommand(const char * cmdname, int id,	
								 I_ConHandler * pHandler)=0;

	//Every ConHandler needs to call this on destruction so that 
	//its vars and commands are removed and archived.
	virtual void UnregisterHandler(I_ConHandler * pHandler)=0;

	//Print Functions
	virtual void ComPrint(const char* text)=0;

	virtual void AddToCmdBuffer(const char * string)=0;

	//pass a string to be exec'ed
	virtual bool ExecString(const char *string)=0;
};


/*
================================================
Interface to handle console variables and commands
Auto-unregisters itself on destruction
================================================
*/
struct I_ConHandler
{
	//Auto unregisters variables
	virtual ~I_ConHandler() { I_Console::GetConsole()->UnregisterHandler(this); }

	//I can implement RegisterCVar and RegisterCommand here if adding "this" 
	//as the last parameter in each call gets to be annoying

	//Called everytime a the given command it entered
	virtual void HandleCommand(int cmdId, const CParms &parms) = 0;

	//Return true if proposed changes are accepted
	virtual bool HandleCVar(const CVar * pVar, const CStringVal &val) = 0;
};

#endif