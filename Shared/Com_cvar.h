#ifndef _VOID_CVARS_AND_CMDS_
#define _VOID_CVARS_AND_CMDS_

#include "Com_defs.h"

//==============================================================

#define MAX_CVARSTRING_LEN	512

class CVar;

//validation func, cvar is only updated if this returns true
typedef bool (*CVAR_FUNC)(const CVar *, int,  char**);	
typedef void (*CFUNC)(int,  char**);

//==============================================================

class CVar
{
public:

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

	
	CVar(const char *varname, 
		 const char *varval,
		 CVarType vartype,
		 int varflags,
		 CVAR_FUNC varfunc=0);

	~CVar();

	//Utility functions, use these to change cvar data
	void Set(const char *varstring);
	void Set(const float val);
	void ForceSet(const char *varstring);
	void ForceSet(const float val);

/*
	std::string name;
	std::string str;
	std::string latched_str;
*/
	char * name;
	char * string;
	char * latched_string;
	char * default_string;

	float		value;
	int			flags;		//CVar characteristics
	CVarType	type;
	CVAR_FUNC	func;		//Functions pointer to Custom Cvar func
};


//==============================================================

class CFunc
{
public:
	CFunc(const char *inname, CFUNC infunc);
	~CFunc();

//	std::string name;
	char *	name;
	CFUNC	func;
};

#endif