#ifndef VOID_CVARS_AND_CMDS
#define VOID_CVARS_AND_CMDS

#include "I_console.h"

/*
==========================================
Each module has its a implementation of CVarBase
This gets rid of nasty memory problems as the CVars
are dynamicallys modified throughout a session
==========================================
*/

class CVar : public CVarBase
{
public:

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

		CVar::ForceSet(varval);
	}

	//Destructor
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
		sprintf(m_buffer,"%.2f",val);
		CVar::ForceSet(m_buffer);
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
		CVar::ForceSet(varval);
	}

	void Set(float val)
	{
		//Return if its a latched var
		if(flags & CVAR_LATCH)		
			return;

		//Read only funcs can only be set once
		if((flags & CVAR_READONLY) && default_string)
			return;
		CVar::ForceSet(val);
	}

private:
	char m_buffer[16];
};

#endif