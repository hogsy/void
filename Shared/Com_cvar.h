#ifndef VOID_CVARS_AND_CMDS
#define VOID_CVARS_AND_CMDS

#include "Com_defs.h"

//==============================================================

#define MAX_CVARSTRING_LEN	512

struct CVar;

//validation func, cvar is only updated if this returns true
typedef bool (*CVAR_FUNC)(const CVar *, int,  char**);	
typedef void (*CFUNC)(int,  char**);

//==============================================================

struct CVar
{
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

	char * name;
	char * string;
	char * latched_string;
	char * default_string;

	float		value;
	int			flags;		//CVar characteristics
	CVarType	type;
	CVAR_FUNC	func;		//Functions pointer to Custom Cvar func


	CVar()
	{
		name = string = default_string = latched_string = 0;
		value = 0.0f;
		flags = 0;
		type = CVAR_UNDEFINED;
		func = 0;
	}

	~CVar()
	{	func = 0;
		if(name)   delete [] name; name = 0;
		if(string) delete [] string; string = 0;
		if(default_string) delete [] default_string; default_string = 0;
		if(latched_string) delete [] latched_string; latched_string = 0;
	}
};


//==============================================================

struct CFunc
{
	CFunc()
	{	name = 0;
		func = 0;
	}
	
	~CFunc()
	{	if(name) delete [] name;name = 0;
		func = 0;
	}

	char *	name;
	CFUNC	func;
};

#endif