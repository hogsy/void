#ifndef INC_CONSOLE_INTERFACE
#define INC_CONSOLE_INTERFACE

//Forward -declarations
class CVarBase;
class CParms;

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

	virtual void Unlatch() = 0;
	virtual void Reset() = 0;

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
struct I_Console : public I_ConHandler
{
	//Cvar Registration
	virtual void RegisterCVar(CVarBase * var,				 //The Cvar being registered
							  I_ConHandler * handler=0)=0;	//Optional handler

	//Con Command Registration
	virtual void RegisterCommand(const char *cmdname,		//Command Name
							  HCMD id,						//ID in the registering class
							  I_ConHandler * handler)=0;	//the class registering the command

	//Print Functions
	virtual void ComPrint(const char* text)=0;

	//pass a string to be exec'ed
	virtual bool ExecString(const char *string)=0;
};

#endif


#if 0
#ifndef INC_CONSOLE_INTERFACE
#define INC_CONSOLE_INTERFACE

/*
================================================
This header should be included by any class
wanting to use Console Commands or Variables
================================================
*/

//Forward -declarations
class CVar;
class CParms;

struct I_ConHandler
{
	//Handle the Command identified by cmdId. using the given Parms
	virtual void HandleCommand(int cmdId, const CParms &parms)=0;

	//Are the given parms valid for the given cvar ?
	virtual bool ValidateCVar(const CVar * cvar, const CParms &parms)=0;

	//The CVar has changed to a new value. Do whats needed
	virtual void HandleCVar(const CVar * cvar)=0;
};


//==========================================================================
//==========================================================================

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


class CVar
{
public:

	~CVar();
	
	//Access funcs. Make sure that a CVar only gets modified by
	//the console class to avoid memory problems
	inline const char * Name()  const { return name; }
	inline const char * String()const { return string; }
	inline int   Int()   const { return ival; }
	inline float Bool()  const { return bval; }
	inline float Float() const { return fval; }
	inline int   Flags() const { return flags;}
	
private:

	friend class CConsole;

	CVar() : name(0), string(0), latched_string(0), default_string(0), handler(0)

	enum
	{	CVAR_MAXSTRINGLEN =	512
	};
	
	union
	{
		float fval;
		int	  ival;
		bool  bval;
	};

	char * name;
	char * string;
	int	   flags;
	
	char * latched_string;
	char * default_string;

	CVarType		type;
	I_ConHandler * handler;
};

/*
==========================================
Console Interface
==========================================
*/
struct I_Console : public I_ConHandler
{
	//Cvar Registration
	//Returns a const Cvar. Handler is optional
	virtual const CVar * CVar_Create(I_ConHandler * handler=0)=0;	
	
	virtual void CVar_Destroy(const CVar * pVar)=0;	
	virtual void CVar_ForceSet(const CVar * pVar, const char *varval)=0;
	virtual void CVar_ForceSet(const CVar * pVar, float val)=0;
	virtual void CVar_ForceSet(const CVar * pVar, int   val)=0;
	virtual void CVar_Set(const CVar * pVar,const char *varval)=0;
	virtual void CVar_Set(const CVar * pVar,float val)=0;
	virtual void CVar_Set(const CVar * pVar,int val)=0;
	virtual void CVar_Unlatch(const CVar * pVar) = 0;
	virtual void CVar_Reset(const CVar * pVar) = 0;

	//Con Command Registration
	virtual void RegisterCommand(const char *cmdname,		//Command Name
							     HCMD id,					//ID in the registering class
							     I_ConHandler * handler)=0;	//the class registering the command
	//Print Functions
	virtual void ComPrint(const char* text)=0;

	//pass a string to be exec'ed
	virtual bool ExecString(const char *string)=0;
};

#endif

#endif
