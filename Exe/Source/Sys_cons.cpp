#include "Sys_hdr.h"

static void HandleBool   (CVar *var, int argc,  char** argv);
static void HandleInt    (CVar *var, int argc,  char** argv);
static void HandleString (CVar *var, int argc,  char** argv);
static void HandleFloat  (CVar *var, int argc,  char** argv);

static void CFunctest(int argc,  char** argv);
void CToggleConsole(int argc, char** argv);			//Console toggle hack

/*
======================================
Constructor
======================================
*/
CConsole::CConsole()
{
	m_pflog = NULL;
	m_prCons = NULL;

	m_pcList = new CPtrList<CVar>;
	m_pfList = new CPtrList<CFunc>;

	m_CmdBuffer = new CQueue<char>;

	m_szargv = new char * [BMAX_ARGS];
	for(int i=0;i<BMAX_ARGS;i++)
	{
		m_szargv[i] = new char[CON_MAXARGSIZE];;
	}

	RegisterCFunc("cvarlist",&CVarlist);
	RegisterCFunc("cfunclist",&CFunclist);
	RegisterCFunc("ctest",&CFunctest);
	RegisterCFunc("contoggle", &CToggleConsole);

	//open logfile
	char debugfilename[128];
	strcpy(debugfilename,g_exedir);
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
bool CConsole::Init(I_RConsole * prcons)
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
void CConsole::dprint(char* text)
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
	if(g_pCons)
	{
		static char textBuffer[1024];
		va_list args;
		va_start(args, text);
		vsprintf(textBuffer, text, args);
		va_end(args);
		
		g_pCons->dprint(textBuffer);
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

			//Allow this nonconstant priviledges
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
			CPtrList<CVar> * pcvar = g_pCons->m_pcList;
			CPtrList<CFunc> * pcfunc = g_pCons->m_pfList;

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
Prints the current line every frame
used to render the status line
======================================
*/
/*void CConsole::UpdateBuffer()
{	m_prCons->Statusline(m_szCBuffer.GetString(), m_szCBuffer.GetSize());
}
*/

/*
======================================
Exec a command line in the console
======================================
*/
bool CConsole::Exec(int argc, char ** argv) 
{
	CFunc * pfunc = 0;
	for(CPtrList<CFunc> *temp2=m_pfList; temp2->item; temp2=temp2->next)
	{
		if(!strcmp(argv[0],temp2->item->name))
		{
			temp2->item->func(argc,argv);
			return true;
		}
	}

	//is it a cvar ?
	CVar * pcvar = 0;
	for (CPtrList<CVar> *temp= m_pcList;temp->item; temp=temp->next)
	{
		pcvar = temp->item;
		if(!strcmp(argv[0],pcvar->name))
		{
			//only exec'ed IF function returns true
			if((!pcvar->func) || pcvar->func(pcvar,argc,argv))
			{
				switch(pcvar->type)
				{
				case CVar::CVAR_INT:
					HandleInt(pcvar,argc,argv);
					break;
				case CVar::CVAR_FLOAT:
					HandleFloat(pcvar,argc,argv);
					break;
				case CVar::CVAR_STRING:
					HandleString(pcvar,argc,argv);
					break;
				case CVar::CVAR_BOOL:
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
	sprintf(file,"%s/%s",g_exedir,filename);

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



/*
=====================================
Write CVars to given file
=====================================
*/
void CConsole::WriteCVars(FILE * fp)
{
	//write all the archive flaged vars in the config file
	CVar * var = 0;
	for (CPtrList<CVar> *temp=m_pcList; temp->item; temp=temp->next)
	{
		var = temp->item;
		if(var->flags & CVar::CVAR_ARCHIVE)
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
void CVarlist(int argc,  char** argv)
{
	ComPrintf("Console Variable Listing\n");
	ComPrintf("========================\n");

	if(argc==2)
	{
		int len= strlen(argv[1]);
		for (CPtrList<CVar> *temp= g_pCons->m_pcList ; temp->item ; temp=temp->next)
		{
			if(strncmp(temp->item->name,argv[1], len)==0)
				ComPrintf("\"%s\" is \"%s\"\n",temp->item->name,temp->item->string);
		}
	}
	else
	{	
		for (CPtrList<CVar> *temp= g_pCons->m_pcList ; temp->item ; temp=temp->next)
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
void CFunclist(int argc,  char** argv)
{
	ComPrintf("Console Functions Listing\n");
	ComPrintf("========================\n");

	if(argc==2)
	{
		int len= strlen(argv[1]);
		for (CPtrList<CFunc> *temp= g_pCons->m_pfList ; temp->item ; temp=temp->next)
		{
			if(strncmp(temp->item->name,argv[1], len)==0)
				ComPrintf("%s\n",temp->item->name);
		}
	}
	else
	{	
		for (CPtrList<CFunc> *temp= g_pCons->m_pfList ; temp->item ; temp=temp->next)
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
void HandleString (CVar *var, int argc,  char** argv)
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
		g_pCons->CVarSet(&var,newstr);
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}

/*
=======================================
Handle arguments to the CVar containing 
an int value and assign new values
=======================================
*/
void  HandleInt (CVar *var, int argc,  char** argv)
{
//FIXME - add parser for special chars like "" and =
	if(argc >=2 && argv[1])
	{
		int temp=0;
		if(sscanf(argv[1],"%d",&temp))
			g_pCons->CVarSet(&var,(float)temp);
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}

/*
=======================================
Handle arguments to the CVar containing 
a float value and assign new values
=======================================
*/
void HandleFloat (CVar *var, int argc,  char** argv)
{
//FIXME - add parser for special chars like "" and =
	if(argc>=2 && argv[1])
	{
		float temp=0;
		if(sscanf(argv[1],"%f",&temp))
			g_pCons->CVarSet(&var,temp);
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}

/*
=======================================
Handle arguments to the CVar containing 
a bool value and assign new values
=======================================
*/
void HandleBool (CVar *var , int argc,  char** argv)
{
//FIXME - add parser for special chars like "" and =
	if(argc>=2 && argv[1])
	{
		float temp=0;
		if(sscanf(argv[1],"%f",&temp))
			g_pCons->CVarSet(&var,temp);
		else if(!strcmp(argv[1],"true"))
			g_pCons->CVarSet(&var,1.0f);
		else
			g_pCons->CVarSet(&var,0.0f);
	}
	ComPrintf("%s = \"%s\"\n",var->name,var->string);
}




/*
=====================================
Util test func
=====================================
*/
//void CConsole::CFunctest(int argc,  char** argv)
void CFunctest(int argc,  char** argv)
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