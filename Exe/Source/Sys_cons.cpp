#include "Sys_hdr.h"
#include "Sys_cons.h"

#define CMD_CVARLIST	0
#define CMD_CMDLIST		1
#define CMD_TOGGLECONS	2
#define CMD_TEST		3


void CToggleConsole(int argc, char** argv);			//Console toggle hack

namespace System
{

I_Console * GetConsole()
{	return g_pConsole;
}

}

/*
======================================
Constructor
======================================
*/
CConsole::CConsole()
{
	m_pflog = NULL;
	m_prCons = NULL;

	m_pcList = new CPRefList<CVarBase>;
	m_pfList = new CPtrList<CCommand>;

	m_CmdBuffer = new CQueue<char>;

	m_szargv = new char * [BMAX_ARGS];
	for(int i=0;i<CVarBase::CVAR_MAXARGS;i++)
	{
		m_szargv[i] = new char[CON_MAXARGSIZE];;
	}

	RegisterCommand("cvarlist",CMD_CVARLIST, this);
	RegisterCommand("cfunclist", CMD_CMDLIST,this);
	RegisterCommand("ctest", CMD_TEST, this);
	RegisterCommand("contoggle", CMD_TOGGLECONS,this);

	//open logfile
	char debugfilename[128];
	strcpy(debugfilename,System::GetExePath());
	strcat(debugfilename,"//vdebug.log");
	m_pflog = fopen(debugfilename, "w");
}

/*
==========================================
Destructor
==========================================
*/
CConsole::~CConsole()
{
	//close log file
	if(m_pflog)
		fclose(m_pflog);

	for(int i=0;i<BMAX_ARGS;i++)
			delete [] m_szargv[i]; 
	delete [] m_szargv;

	delete m_CmdBuffer;
	m_prCons = 0;

	//Delete the CVar Lists
	delete m_pcList;
	delete m_pfList;

}

/*
==========================================
Initialize the Console
==========================================
*/
bool CConsole::Init(I_ConsoleRenderer * prcons)
{
	// Create a debugging window
#ifdef DOSCONS
	AllocConsole();
	SetConsoleTitle("Void Debug");
	m_hIn  = ::GetStdHandle(STD_INPUT_HANDLE);
	m_hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	
	m_prCons = prcons;
	if(!m_prCons)
		return false;
	return true;
}

/*
==========================================
Shutdown the console
==========================================
*/
bool CConsole::Shutdown()
{

#ifdef DOSCONS
	ComPrintf("CConsole::Shutting down Console\n");
	CloseHandle(m_hIn);
	CloseHandle(m_hOut);
	FreeConsole();
#endif
	m_prCons=0;
	return true;
}

/*
===============================================
print a string to debugging window	
===============================================
*/
void CConsole::ComPrint(char* text)
{
	if (!text)
		return;

#ifdef DOSCONS
	DWORD num;
	WriteConsole(m_hOut, text, strlen(text), &num, NULL);
#endif
	
	//write to log
	if(m_pflog)
	{
		fputs(text,m_pflog);
		fflush(m_pflog);
	}
	
	//pass it the renderer
	if (m_prCons)
		m_prCons->AddLine(text);
}

/*
===============================================
print a string to debugging window 
and handle any arguments
===============================================
*/
void ComPrintf(char* text, ...)
{
	if(g_pConsole)
	{
		static char textBuffer[1024];
		va_list args;
		va_start(args, text);
		vsprintf(textBuffer, text, args);
		va_end(args);
		
		g_pConsole->ComPrint(textBuffer);
	}
}

/*
======================================
Handle input, 1 character at a time
======================================
*/
void CConsole::HandleKeyEvent(const KeyEvent_t &kevent)
{
	if((kevent.state == BUTTONDOWN) ||
		(kevent.state == BUTTONHELD))
	{
			HandleInput(kevent.id);
			m_prCons->Statusline(m_szCBuffer.GetString(), m_szCBuffer.GetSize());
	}
}


void CConsole::HandleInput(const int &i)
{
	switch(i)
	{
		case '`':
		{
			m_szCBuffer.Reset();
			CToggleConsole(1,0);
			break;
		}
		case INKEY_ENTER:
		{
			int nargc =0;

			//if it wasnt a blank line, enter it into the command buffer
			if(m_szCBuffer.GetSize())
			{
				m_CmdBuffer->Add(m_szCBuffer.GetString(), 
								 m_szCBuffer.GetSize()+1);
			}

			//print out the line
			ComPrintf("%s\n",m_szCBuffer.GetString());	


//			Exec(m_szCBuffer.GetString());

			nargc = m_szCBuffer.Parse(m_szargv);

			//parse the string and try to exec it
			
			if(Exec(nargc,m_szargv))
			{
			}
		
			//reset the buffer
			m_szCBuffer.Reset();
			break;
		}
		case INKEY_BACKSPACE:
		{	
			m_szCBuffer.Pop(1);
			break;
		}
		case INKEY_UPARROW:					
		{
			//change it to the previous command
			m_szCBuffer.Set(m_CmdBuffer->GetPrev());
			break;
		}
		case INKEY_DOWNARROW:				
		{
			//change it to the next command
			m_szCBuffer.Set(m_CmdBuffer->GetNext());
			break;
		}
		case INKEY_TAB:
		{
			int len = m_szCBuffer.GetSize();
			if(!len)
				return;
			//Print all the CVars and cmds that match the given string
			const char * p = m_szCBuffer.GetString();
			char * lastmatch=0;
			CPRefList<CVarBase> * pcvar = g_pConsole->m_pcList;
			CPtrList<CCommand> * pcfunc = g_pConsole->m_pfList;

			ComPrintf("\n");
			while(pcvar->item || pcfunc->item)
			{
				if(pcvar->item)
				{
					if(!strncmp(pcvar->item->name,p, len))
					{
						ComPrintf("%s\n",pcvar->item->name);
						lastmatch = pcvar->item->name;
					}
					pcvar = pcvar->next;
				}
				if(pcfunc->item)
				{
					if(!strncmp(pcfunc->item->name,p, len))
					{
						ComPrintf("%s\n",pcfunc->item->name);
						lastmatch = pcfunc->item->name;
					}
					pcfunc = pcfunc->next;
				}
				
			}
			//change current buffer to the one that closest matches.
			if(lastmatch)
				m_szCBuffer.Set(lastmatch);

			break;
		}
		case INKEY_PGUP:
		{
			m_prCons->Lineup();
			break;
		}
		case INKEY_PGDN:
		{
			m_prCons->Linedown();
			break;
		}
		default:
		{
			//ascii char entered, add to line
			if(i >= 0 && i <= 127)					
				m_szCBuffer.Append(i);
			break;
		}
	}
}

/*
======================================
Exec a command line in the console
======================================
*/
void CConsole::ExecCommand(CCommand * cmd, const char * cmdString)
{
	m_szCBuffer.Set(cmdString);	//set to string
	int nargc=m_szCBuffer.Parse(m_szargv);
	cmd->handler->HandleCommand(cmd->id, nargc, m_szargv);
	m_szCBuffer.Reset();
}

bool CConsole::Exec(int argc, char** argv) 
{

	CCommand * pfunc = 0;
	for(CPtrList<CCommand> *temp2=m_pfList; temp2->item; temp2=temp2->next)
	{
		if(!strcmp(argv[0],temp2->item->name))
		{
			temp2->item->handler->HandleCommand(temp2->item->id, argc, argv);
			return true;
		}
	}

	//is it a cvar ?
	CVarBase * pcvar = 0;
	for (CPRefList<CVarBase> *temp= m_pcList;temp->item; temp=temp->next)
	{
		pcvar = temp->item;
		if(!strcmp(argv[0],pcvar->name))
		{
			//only exec'ed IF function returns true
			//if((!pcvar->func) || pcvar->func(pcvar,argc,argv))
			if((!pcvar->handler) || pcvar->handler->HandleCVar(pcvar,argc,argv))
			{
				switch(pcvar->type)
				{
				case CVarBase::CVAR_INT:
					HandleInt(pcvar,argc,argv);
					break;
				case CVarBase::CVAR_FLOAT:
					HandleFloat(pcvar,argc,argv);
					break;
				case CVarBase::CVAR_STRING:
					HandleString(pcvar,argc,argv);
					break;
				case CVarBase::CVAR_BOOL:
					HandleBool(pcvar,argc,argv);
					break;
				}
			}
			return true;
		}
	}
	return false;
}


/*
=======================================

=======================================
*/
void CConsole::ExecString(const char *string) //const
{
	m_szCBuffer.Set(string);	//set to string
	int nargc=m_szCBuffer.Parse(m_szargv);

	if((nargc) && !Exec(nargc,m_szargv))
	{
		ComPrintf("%s\n",string);
	}
	m_szCBuffer.Reset();
}

/*
======================================
Runs a Config file
======================================
*/
void CConsole::ExecConfig(const char *filename)
{
	char file[128];
	sprintf(file,"%s/%s",System::GetExePath(),filename);

	FILE * fpcfg=fopen(file,"r");
	if(fpcfg == NULL)
	{
		ComPrintf("CConsole::ExecConfig:Error opening %s\n",file);
		return;
	}

	char c, lastc;
	int  lines=0;
	int  chars=0;
	char line[80];
	
	//Configs files are linited to 256 lines, 80 chars each
	memset(line,0,80);
	while((c=fgetc(fpcfg)) != EOF)
	{
		if(lines > 256)
			break;

		if((c == '\n') || !(chars < 80)	//Line finished, parse the line and process it
			|| (c == '/' && lastc == '/'))		//Comments
		{
			if(chars > 3)
				ExecString(line);
			lines++;
			chars=0;
			memset(line,'\0',80);
		}
		else
		{
			line[chars] = c;
			lastc = c;
			chars++;
		}
	}
	fclose(fpcfg);
	ComPrintf("CConsole::Exec'ed %s\n",file);
}



void CConsole::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case CMD_CVARLIST:
		CVarlist(numArgs,szArgs);
		break;
	case CMD_CMDLIST:
		CCmdList(numArgs,szArgs);
		break;
	case CMD_TOGGLECONS:
		CToggleConsole(numArgs,szArgs);
		break;
	case CMD_TEST:
		CFunctest(numArgs,szArgs);
		break;
	}
}



/*
=====================================
Write CVars to given file
=====================================
*/
void CConsole::WriteCVars(FILE * fp)
{
	//write all the archive flaged vars in the config file
	CVarBase * var = 0;
	for (CPRefList<CVarBase> *temp=m_pcList; temp->item; temp=temp->next)
	{
		var = temp->item;
		if(var->flags & CVarBase::CVAR_ARCHIVE)
		{
			char line[80];
			strcpy(line,var->name);
			strcat(line," ");
			strcat(line,var->string);
			strcat(line,"\n");
			fputs(line,fp);
		}
	}

}

/*
==========================================
Proxy functions 
==========================================
*/

void CConsole::ToggleFullscreen(bool full)
{	
	m_prCons->ToggleFullscreen(full);
}

void CConsole::Toggle(bool down)
{	
	m_prCons->Toggle(down);
}


/*
===============================================
Throw a message box
===============================================
*/
void CConsole::MsgBox(char *msg, ...)
{
	static char textBuffer[2048];
	va_list args;
	va_start(args, msg);
	vsprintf(textBuffer, msg, args);
	va_end(args);
	MessageBox(NULL, textBuffer, "Error", MB_OK);
}

void CConsole::MsgBox(char *caption, unsigned long boxType, char *msg, ...)
{
	static char textBuffer[2048];
	va_list args;
	va_start(args, msg);
	vsprintf(textBuffer, msg, args);
	va_end(args);
	MessageBox(NULL, textBuffer, caption, boxType);
}



/*
======================================
Prints out all the CVars and their values
======================================
*/
void CConsole::CVarlist(int argc,  char** argv)
{
	ComPrintf("Console Variable Listing\n");
	ComPrintf("========================\n");

	if(argc==2)
	{
		int len= strlen(argv[1]);
		for (CPRefList<CVarBase> *temp= g_pConsole->m_pcList ; temp->item ; temp=temp->next)
		{
			if(strncmp(temp->item->name,argv[1], len)==0)
				ComPrintf("\"%s\" is \"%s\"\n",temp->item->name,temp->item->string);
		}
	}
	else
	{	
		for (CPRefList<CVarBase> *temp= g_pConsole->m_pcList ; temp->item ; temp=temp->next)
		{
			ComPrintf("\"%s\" is \"%s\"\n",temp->item->name,temp->item->string);
		}
	}

}

/*
======================================
Prints out all the CVars and their values
======================================
*/
void CConsole::CCmdList(int argc,  char** argv)
{
	ComPrintf("Console Command Listing\n");
	ComPrintf("========================\n");

	if(argc==2)
	{
		int len= strlen(argv[1]);
		for (CPtrList<CCommand> *temp= g_pConsole->m_pfList ; temp->item ; temp=temp->next)
		{
			if(strncmp(temp->item->name,argv[1], len)==0)
				ComPrintf("%s\n",temp->item->name);
		}
	}
	else
	{	
		for (CPtrList<CCommand> *temp= g_pConsole->m_pfList ; temp->item ; temp=temp->next)
		{	ComPrintf("%s\n",temp->item->name);
		}
	}

}


/*
=======================================
Handle arguments to the CVar containing 
a string and assign new values
=======================================
*/
void CConsole::HandleString (CVarBase *var, int argc,  char** argv)
{
	if(argc >= 2 && argv[1])
	{
		char newstr[80];
		
		strcpy(newstr,argv[1]);
		
		for(int i=2,len=0;i<argc;i++)
		{
			len += strlen(argv[i]+1);
			if(len >= 80)
				break;
			strcat(newstr," ");
			strcat(newstr,argv[i]);
		}
		//CVarSet(&var,newstr);
		var->Set(newstr);
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}

/*
=======================================
Handle arguments to the CVar containing 
an int value and assign new values
=======================================
*/
void  CConsole::HandleInt (CVarBase *var, int argc,  char** argv)
{
//FIXME - add parser for special chars like "" and =
	if(argc >=2 && argv[1])
	{
		var->Set(argv[1]);
/*		int temp=0;
		if(sscanf(argv[1],"%d",&temp))
			//CVarSet(&var,(float)temp);
			var->Set((float)temp);
*/
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}

/*
=======================================
Handle arguments to the CVar containing 
a float value and assign new values
=======================================
*/
void CConsole::HandleFloat (CVarBase *var, int argc,  char** argv)
{
//FIXME - add parser for special chars like "" and =
	if(argc>=2 && argv[1])
	{
		var->Set(argv[1]);
/*		float temp=0;
		if(sscanf(argv[1],"%f",&temp))
			//CVarSet(&var,temp);
			var->Set(temp);
*/
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}

/*
=======================================
Handle arguments to the CVar containing 
a bool value and assign new values
=======================================
*/
void CConsole::HandleBool (CVarBase *var , int argc,  char** argv)
{
//FIXME - add parser for special chars like "" and =
	if(argc>=2 && argv[1])
	{
		var->Set(argv[1]);
/*		float temp=0;
		if(sscanf(argv[1],"%f",&temp))
			//CVarSet(&var,temp);
			var->Set(temp);
		else if(!strcmp(argv[1],"true"))
			//CVarSet(&var,1.0f);
			var->Set(1.0f);
		else
			//CVarSet(&var,0.0f);
			var->Set(1.0f);
*/
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}




/*
=====================================
Util test func
=====================================
*/
//void CConsole::CFunctest(int argc,  char** argv)
void CConsole::CFunctest(int argc, char** argv)
{
/*	CQueue<char> q(3);
//	CList<char *> l(3);
	
	char * test[] = {"abc","def","ghi"};

	for(int x=0;x<3;x++)
	{
//		l.Add(&test[x]);
		q.Add(test[x]);
	}
//	l.PrintAll();
//	q.PrintAll();
//	q;
*/
	/*
char * buffer = NULL;
if ( OpenClipboard() ) {
	HANDLE hData = GetClipboardData( CF_TEXT );
	buffer = (char*)GlobalLock( hData );
	GlobalUnlock( hData );
	CloseClipboard();
}
*/
}



//======================================================================================
//======================================================================================


/*
==========================================
Register CVar

Initialize CVar and alphabetically add to list
==========================================
*/

void CConsole::RegisterCVar(CVarBase * var,
							I_CVarHandler * handler)
{
	if(handler)
		var->handler = handler;

	//Add Item to CvarList
	CPRefList<CVarBase>* i1 = m_pcList;
	CPRefList<CVarBase>* i2 = 0;
	
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
			CPRefList<CVarBase> * newentry = new CPRefList <CVarBase>;
			newentry->item = var;
			newentry->next = i1;
			m_pcList = newentry;
		}
		//List is empty, add to it
		else
		{
			i1->item = var;
			i1->next = new CPRefList<CVarBase>;
		}
	}
	//Item comes after the item in list pointer to by i2, and before i1
	else
	{
		CPRefList<CVarBase> * newentry = new CPRefList <CVarBase>;
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


