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

	//Constructor
	CVar(const char *varname,  const char *varval, 
		 CVarType vartype,	int varflags)
	{
		name = new char[strlen(varname)+1];
		strcpy(name,varname);
		
		type = vartype;
		flags = varflags;

		fval = 0.0f;

		string = 0;
		latched_string = 0;
		default_string = 0;
		handler = 0;

//		CVar::
			ForceSet(varval);
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
		//Reset current info
		fval = 0;
		if(string)
		{
			delete [] string;
			string = 0;
		}
		
		switch(type)
		{
		case CVAR_INT:
			{
				if(!sscanf(varval,"%d",&ival))
				{
					ival = 0;
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
		case CVAR_FLOAT:
			{
				if(!sscanf(varval,"%f",&fval))
				{
					fval = 0;
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
				if(!sscanf(varval,"%d",&ival))
					ival = 0;
				
				string = new char[2];
				if(ival)
				{
					string[0] = '1';
					bval = true;
				}
				else
				{
					string[0] = '0';
					bval = false;
				}
				string[1] = '\0';
				break;
			}
		case CVAR_STRING:
		default:
			{
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
		char buffer[16];
		sprintf(buffer,"%.2f",val);
		ForceSet(buffer);
	}
	void ForceSet(int val)
	{
		char buffer[16];
		sprintf(buffer,"%d",val);
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
		{
			if(latched_string)
				delete [] latched_string;
			latched_string = new char [strlen(varval) +1];
			strcpy(latched_string, varval);
			return;
		}

		//Read only funcs can only be set once
		if((flags & CVAR_READONLY) && default_string)
			return;
		//CVar::
			ForceSet(varval);
	}

	void Set(float val)
	{
		//Return if its a latched var
		if(flags & CVAR_LATCH)	
		{
			if(latched_string)
				delete [] latched_string;
			char buffer[16];
			sprintf(buffer,"%.2f",val);
			latched_string = new char [strlen(buffer) +1];
			strcpy(latched_string, buffer);
			return;
		}

		//Read only funcs can only be set once
		if((flags & CVAR_READONLY) && default_string)
			return;
		//CVar::
			ForceSet(val);
	}

	void Set(int val)
	{
		//Return if its a latched var
		if(flags & CVAR_LATCH)	
		{
			if(latched_string)
				delete [] latched_string;
			char buffer[16];
			sprintf(buffer,"%d",val);
			latched_string = new char [strlen(buffer) +1];
			strcpy(latched_string, buffer);
			return;
		}

		//Read only funcs can only be set once
		if((flags & CVAR_READONLY) && default_string)
			return;
		//CVar::
			ForceSet(val);
	}

	/*
	======================================
	Unlatch the cvar
	======================================
	*/
	void Unlatch()
	{	
		if(latched_string)
		{
			//CVar::
				ForceSet(latched_string);
			delete [] latched_string;
			latched_string = 0;
		}
	}

	/*
	======================================
	Reset to default value
	======================================
	*/
	void Reset()
	{	
		//CVar::
			ForceSet(default_string);
		if(latched_string)
		{
			delete [] latched_string;
			latched_string = 0;
		}
	}
};

#endif