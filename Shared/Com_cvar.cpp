#include "Com_cvar.h"

/*
=====================================
CFunc Implementation
=====================================
*/


CFunc::CFunc(const char *inname,CFUNC infunc)
{
	name = (char *)MALLOC(strlen(inname) + 1);
	strcpy(name,inname);
	func = infunc;
}

CFunc::~CFunc()
{	func = 0;
	free(name);
}


/*
================================================================================
CVar Class implemetation
================================================================================
*/
CVar::CVar(const char *varname,
		   const char *varval,
		   CVarType vartype,
		   int varinfo,
		   CVAR_FUNC infunc) :	flags(varinfo), 
								type (vartype), 
								func(infunc)
{
	string =0;
	default_string =0;
	latched_string =0;

	name = (char *)MALLOC(strlen(varname)+1);
	strcpy(name,varname);
	
	ForceSet(varval);
}


/*
===================
Destructor
===================
*/
CVar::~CVar()
{	
	func = 0;
	if(name)
		free(name);
	name = 0;
	if(string)
		free(string);
	string = 0;
	if(default_string)
		free(default_string);
	default_string  = 0;
	if(latched_string)
		free(latched_string);
	latched_string = 0;
}

/*
===================
Standard Set function
===================
*/
void CVar::Set(const char *varstring)
{
	//Return if its a latched var
	if(flags & CVAR_LATCH)		
		return;
	
	//Read only funcs can only be set once
	if((flags & CVAR_READONLY) && default_string)
		return;
	ForceSet(varstring);
}

void CVar::Set(float val)
{
	//Return if its a latched var
	if(flags & CVAR_LATCH)		
		return;

	//Read only funcs can only be set once
	if((flags & CVAR_READONLY) && default_string)
		return;
	ForceSet(val);
}

/*
===================
Standard Set function
===================
*/
void CVar::ForceSet(const char *varstring)
{
	if(!varstring)
		return;

	int len = strlen(varstring) + 1;
	if(len > MAX_CVARSTRING_LEN)
		len = MAX_CVARSTRING_LEN;

	if(string)
	{
		free(string);
		string = 0;
	}
	
	switch(type)
	{
	case CVAR_INT:
	case CVAR_FLOAT:
		if(!sscanf(varstring,"%f",&value))
		{
			value =0;
			string = (char *)MALLOC(strlen("\" \"") + 1);
			strcpy(string,"\" \"");
		}
		else
		{
			string = (char *)MALLOC(len);
			strcpy(string,varstring);
		}
		break;
	case CVAR_BOOL:
		if(!sscanf(varstring,"%f",&value))
			value = 0;
		string = (char *)MALLOC(strlen("false") + 1);
		if(!value)
			strcpy(string,"false");
		else
			strcpy(string,"true");
		break;
	case CVAR_STRING:
	default:
		value = 0;
		string = (char *)MALLOC(len);
		strcpy(string,varstring);
		break;
	}

	//Add a default value, if this is the first time
	//we are setting the cvar
	if(!default_string)
	{
		default_string = (char *)MALLOC(strlen(string) + 1);
		strcpy(default_string,string);
	}
}


void CVar::ForceSet(float fval)
{
	static char buffer[8];
	memset(buffer,0,8);
	sprintf(buffer,"%f",fval);
	ForceSet(buffer);
}