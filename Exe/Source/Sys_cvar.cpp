#include "Sys_cons.h"



/*
==========================================
Register CVar

Initialize CVar and alphabetically add to list
==========================================
*/

void CConsole::RegisterCVar(CVar * var,
							I_CVarHandler * handler)
{
	if(handler)
		var->handler = handler;

	//Add Item to CvarList
	CPRefList<CVar>* i1 = m_pcList;
	CPRefList<CVar>* i2 = 0;
	
	//Loop till there are no more items in list, or the variable name is 
	//bigger than the item in the list
	while(i1->next && i1->item && (strcmp(var->name, i1->item->name) > 0))
	{
		i2 = i1;
		i1 = i1->next;
	}

	//didnt loop
	if(i2 == 0)
	{
		//New item comes before the first item in the list
		if(m_pcList->item)
		{
			CPRefList<CVar> * newentry = new CPRefList <CVar>;
			newentry->item = var;
			newentry->next = i1;
			m_pcList = newentry;
		}
		//List is empty, add to it
		else
		{
			i1->item = var;
			i1->next = new CPRefList<CVar>;
		}
	}
	//Item comes after the item in list pointer to by i2, and before i1
	else
	{
		CPRefList<CVar> * newentry = new CPRefList <CVar>;
		newentry->item = var;
		i2->next = newentry;
		newentry->next = i1;
	}
}

/*
==========================================
Register CFunc
==========================================
*/

void CConsole::RegisterCommand(const char *cmdname,
							   HCMD id,
							   I_CmdHandler * handler)
{
	CCommand * newfunc = new CCommand(cmdname,id,handler);
	
	//Add Item to CvarList
	CPtrList<CCommand>* i1 = m_pfList;
	CPtrList<CCommand>* i2 = 0;
	
	//Loop till there are no more items in list, or the variable name is 
	//bigger than the item in the list
	while(i1->next && i1->item && (strcmp(newfunc->name, i1->item->name) > 0))
	{
		i2 = i1;
		i1 = i1->next;
	}

	//didnt loop
	if(i2 == 0)
	{
		//New item comes before the first item in the list
		if(m_pfList->item)
		{
			CPtrList<CCommand> * newentry = new CPtrList <CCommand>;
			newentry->item = newfunc;
			newentry->next = i1;
			m_pfList = newentry;
		}
		//List is empty, add to it
		else
		{
			i1->item = newfunc;
			i1->next = new CPtrList<CCommand>;
		}
	}
	//Item comes after the item in list pointer to by i2, and before i1
	else
	{
		CPtrList<CCommand> * newentry = new CPtrList <CCommand>;
		newentry->item = newfunc;
		i2->next = newentry;
		newentry->next = i1;
	}
}


CCommand * CConsole::GetCommandByName(const char * cmdString)
{
	CPtrList<CCommand>* iterator = m_pfList;
	while(iterator->next && iterator->item)
	{
		if(strcmp(cmdString,iterator->item->name) ==0)
		{
			//*phandler = iterator->item->handler;
			return iterator->item; //->id;
		}
		iterator = iterator->next;
	}
	return 0;
}


#if 0

/*
===================
Standard Set functions
===================
*/

void CConsole::CVarSet(CVar **cvar, const char *varval)
{
	//Return if its a latched var
	if((*cvar)->flags & CVar::CVAR_LATCH)		
		return;
	
	//Read only funcs can only be set once
	if(((*cvar)->flags & CVar::CVAR_READONLY) && (*cvar)->default_string)
		return;
	CVarForceSet(cvar,varval);
}

void CConsole::CVarSet(CVar **cvar, float val)
{
	//Return if its a latched var
	if((*cvar)->flags & CVar::CVAR_LATCH)		
		return;
	
	//Read only funcs can only be set once
	if(((*cvar)->flags & CVar::CVAR_READONLY) && (*cvar)->default_string)
		return;
	
	CVarForceSet(cvar,val);
}


/*
===================
Forceset functions
===================
*/
void CConsole::CVarForceSet(CVar **cvar, const char *varval)
{
	if(!varval)
		return;

	int len = strlen(varval) + 1;
	if(len > CVar::CVAR_MAXSTRINGLEN)
		len = CVar::CVAR_MAXSTRINGLEN;

	if((*cvar)->string)
	{
		delete [] (*cvar)->string;
		(*cvar)->string = 0;
	}
	
	switch((*cvar)->type)
	{
	case CVar::CVAR_INT:
	case CVar::CVAR_FLOAT:
		if(!sscanf(varval,"%f",&((*cvar)->value)))
		{
			(*cvar)->value =0;
			(*cvar)->string = new char[strlen("\" \"") + 1];
			strcpy((*cvar)->string,"\" \"");
		}
		else
		{
			(*cvar)->string = new char[len];
			strcpy((*cvar)->string,varval);
		}
		break;
	case CVar::CVAR_BOOL:
		if(!sscanf(varval,"%f",&((*cvar)->value)))
			(*cvar)->value = 0;
		(*cvar)->string = new char[strlen("false") + 1];
		if(!(*cvar)->value)
			strcpy((*cvar)->string,"false");
		else
			strcpy((*cvar)->string,"true");
		break;
	case CVar::CVAR_STRING:
	default:
		(*cvar)->value = 0;
		(*cvar)->string = new char[len];
		strcpy((*cvar)->string,varval);
		break;
	}

	//Add a default value, if this is the first time
	//we are setting the cvar
	if(!(*cvar)->default_string)
	{
		(*cvar)->default_string = new char[strlen((*cvar)->string) + 1];
		strcpy((*cvar)->default_string,(*cvar)->string);
	}
}


void CConsole::CVarForceSet(CVar **cvar, float val)
{
	static char buffer[8];
	memset(buffer,0,8);
	sprintf(buffer,"%f",val);
	CVarForceSet(cvar,buffer);
}

#endif


