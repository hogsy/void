#ifndef VOID_CVARS_AND_CMDS
#define VOID_CVARS_AND_CMDS

#include "Com_defs.h"

//======================================================================================
//======================================================================================

struct CVar;
struct I_CVarHandler
{	virtual bool HandleCVar(const CVar * cvar, int numArgs, char ** szArgs)=0;
};

//======================================================================================
//======================================================================================

struct CVar
{
	/*
	==========================================
	Constants
	==========================================
	*/
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

	/*
	==========================================
	Public vars
	==========================================
	*/
	char * name;
	char * string;
	char * latched_string;
	char * default_string;

	float		value;
	int			flags;		//CVar characteristics
	CVarType	type;
	I_CVarHandler * handler;

	/*
	==========================================
	Constructor
	==========================================
	*/
	CVar(const char *varname, 
		 const char *varval,
		 CVarType vartype,	
		 int varflags)
	{
		name = new char[strlen(varname)+1];
		strcpy(name,varname);
		
		type = vartype;
		flags = varflags;

		string = 0;
		latched_string = 0;
		default_string = 0;
		value = 0.0f;
		handler = 0;

		ForceSet(varval);
	}

	/*
	==========================================
	Desctructor
	==========================================
	*/
	~CVar()
	{	
		handler = 0;
		if(name)   delete [] name; name = 0;
		if(string) delete [] string; string = 0;
		if(default_string) delete [] default_string; default_string = 0;
		if(latched_string) delete [] latched_string; latched_string = 0;
	}

	/*
	==========================================
	ForceSet CVar to the given val. regardless of flags
	==========================================
	*/
	void ForceSet(const char *varval)
	{
		if(string)
		{
			delete [] string;
			string = 0;
		}
	
		switch(type)
		{
		case CVAR_INT:
		case CVAR_FLOAT:
			{
				if(!sscanf(varval,"%f",&value))
				{
					value =0;
					string = new char[strlen("\" \"") + 1];
					strcpy(string,"\" \"");
				}
				else
				{
					string = new char[strlen(varval) + 1];
					strcpy(string,varval);
				}
				break;
			}
		case CVAR_BOOL:
			{
				if(!sscanf(varval,"%f",&value))
					value = 0;
				string = new char[strlen("false") + 1];
				if(!value)
					strcpy(string,"false");
				else
					strcpy(string,"true");
				break;
			}
		case CVAR_STRING:
		default:
			{
				value = 0;
				string = new char[strlen(varval) + 1];
				strcpy(string,varval);
				break;
			}
		}

		//Add a default value, if this is the first time
		//we are setting the cvar
		if(!default_string)
		{
			default_string = new char[strlen(string) + 1];
			strcpy(default_string,string);
		}
	}


	void ForceSet(float val)
	{
		char buffer[8];
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"%f",val);
		
		ForceSet(buffer);
	}

	/*
	==========================================
	Set Cvar to the given value
	==========================================
	*/
	void Set(const char *varval)
	{
		//Return if its a latched var
		if(flags & CVAR_LATCH)		
			return;
	
		//Read only funcs can only be set once
		if((flags & CVAR_READONLY) && default_string)
			return;
		ForceSet(varval);
	}

	void Set(float val)
	{
		//Return if its a latched var
		if(flags & CVAR_LATCH)		
			return;
	
		//Read only funcs can only be set once
		if((flags & CVAR_READONLY) && default_string)
			return;
		ForceSet(val);
	}
};


//======================================================================================
//======================================================================================

typedef int HCMD;

struct I_CmdHandler
{	virtual void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)=0;
};

//======================================================================================
//======================================================================================

struct CCommand
{
	CCommand(const char * iname,
			 HCMD iid, 
			 I_CmdHandler * ihandler) : handler(ihandler), id(iid)
	{
		name = new char[strlen(iname)+1];
		strcpy(name,iname);
	}

	~CCommand()
	{
		delete [] name;
		handler = 0;
	}

	HCMD	id;
	char *	name;
	I_CmdHandler * handler;
};

#endif