#ifndef INC_CONSOLE_INTERFACE
#define INC_CONSOLE_INTERFACE


//======================================================================================
//======================================================================================

class CVarBase;
struct I_CVarHandler
{	virtual bool HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs)=0;
};

//======================================================================================
//======================================================================================

class CVarBase
{
public:

	enum
	{
		CVAR_MAXSTRINGLEN =	512,
		CVAR_MAXARGS	  =	5
	};

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
	
	friend class CConsole;

	char * latched_string;
	char * default_string;

	CVarType		type;
	I_CVarHandler * handler;
};


//======================================================================================
//======================================================================================

typedef int HCMD;

struct I_CmdHandler
{	virtual void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)=0;
};


/*
==========================================
Console Interface
==========================================
*/
struct I_Console
{
	//Cvar Registrations
	virtual void RegisterCVar(	CVarBase * var,				 //The Cvar being registered
								I_CVarHandler * handler=0)=0;//Optional handler

	virtual void RegisterCommand(const char *cmdname,		 //Command Name
								HCMD id,					 //ID in the registering class
								I_CmdHandler * handler)=0;	 //the class registering the command
	//Print Functions
	virtual void ComPrint(char* text)=0;

	//pass a string to be exec'ed
	virtual void ExecString(const char *string)=0;
};

#endif