#include "Sys_cons.h"
#include "Com_util.h"

#include <direct.h>

//Private stuff
//======================================================================================

namespace
{
	enum
	{
		CMD_CVARLIST = 1,
		CMD_CMDLIST  = 2,
		CMD_TEST	 = 3
	};
	const int  CON_MAXARGSIZE  = 80;
}
char CParms::szParmBuffer[1024];

//======================================================================================

/*
======================================
Constructor
======================================
*/ 
CConsole::CConsole() : m_parms(CON_MAXARGSIZE)
{
	m_itCmd = 0;

	m_pflog = NULL;
	m_prCons = NULL;

	//open logfile
	char debugfilename[COM_MAXPATH];
	_getcwd(debugfilename,COM_MAXPATH);
	strcat(debugfilename,"/debug.log");

	m_pflog = fopen(debugfilename, "wc");

#ifdef DOSCONS
	AllocConsole();
	SetConsoleTitle("Void Debug");
	m_hIn  = ::GetStdHandle(STD_INPUT_HANDLE);
	m_hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	RegisterCommand("cvarlist",CMD_CVARLIST, this);
	RegisterCommand("cmdlist", CMD_CMDLIST,this);
	RegisterCommand("cfunclist", CMD_CMDLIST,this);
	RegisterCommand("ctest", CMD_TEST, this);
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

#ifdef DOSCONS
	ComPrintf("CConsole::Shutting down Console\n");
	CloseHandle(m_hIn);
	CloseHandle(m_hOut);
	FreeConsole();
#endif

	m_prCons = 0;
}

/*
==========================================
Set the Renderer
==========================================
*/
void CConsole::SetConsoleRenderer(I_ConsoleRenderer * prcons)
{	m_prCons = prcons;
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
//		fflush(m_pflog);
	}
	
	//pass it the renderer
	if (m_prCons)
		m_prCons->AddLine(text);
}

/*
======================================
Handle input, 1 character at a time
======================================
*/
void CConsole::HandleKeyEvent(const KeyEvent &kevent)
{
	if((kevent.state == BUTTONDOWN) ||
		(kevent.state == BUTTONHELD))
	{
		switch(kevent.id)
		{
			case '`':
			{
				m_conString.erase();
				ExecString("contoggle");
				break;
			}
			case INKEY_ENTER:
			{
				int nargc =0;

				//if it wasnt a blank line, enter it into the command buffer
				if(m_conString.length())
				{
					if(m_cmdBuffer.size() == MAX_OLDCMDS)
						m_cmdBuffer.pop_front();
					m_cmdBuffer.push_back(std::string(m_conString));
					m_itCmd = m_cmdBuffer.end();
				}
				
				//print out the line
				ExecString(m_conString.c_str());
	
				//reset the buffer
				m_conString.erase();
				break;
			}
			case INKEY_TAB:
			{
				//Print all the CVars and cmds that match the given string
				if(!m_conString.size())
					return;

				StringList	matchingNames;
				int	len	  = m_conString.size();

				for(CVarList::iterator itVar= m_lCVars.begin(); itVar != m_lCVars.end(); itVar++)
				{
					if(strncmp(m_conString.c_str(),(*itVar)->name,len) == 0)
						matchingNames.push_back(std::string((*itVar)->name));
				}

				for(CmdList::iterator itCmd = m_lCmds.begin(); itCmd != m_lCmds.end(); itCmd ++)
				{
					if(strncmp(m_conString.c_str(),itCmd->name,len) == 0)
						matchingNames.push_back(std::string(itCmd->name));
				}
				
				if(matchingNames.size())
				{
					matchingNames.sort();
					
					//Print all the matched entries
					for(StringList::iterator it = matchingNames.begin(); it != matchingNames.end(); it++)
						ComPrintf("%s\n", it->c_str());
					m_conString.assign(matchingNames.back());
				}
				break;
			}
			case INKEY_BACKSPACE:
			{	
				if(m_conString.length())
					m_conString.erase(m_conString.end()-1);
				break;
			}
			case INKEY_UPARROW:					
			{
				if((m_itCmd != 0) && (m_itCmd != m_cmdBuffer.begin()))
				{
					m_itCmd--;
					m_conString.assign(*m_itCmd);
				}
				break;
			}
			case INKEY_DOWNARROW:				
			{
				if(m_itCmd != 0)
				{
					if(++m_itCmd != m_cmdBuffer.end())
						m_conString.assign(*m_itCmd);
					else
						m_itCmd--;
				}
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
				if(kevent.id >= 0 && kevent.id <= 127)	//ascii char entered, add to line
					m_conString.append(1,kevent.id);
				break;
			}
		}
		m_prCons->Statusline(m_conString.c_str(),m_conString.size()+1);
	}
}

/*
======================================
Exec a command line in the console
======================================
*/
void CConsole::ExecCommand(CCommand * cmd, const char * cmdString)
{
	m_parms = cmdString;
	cmd->handler->HandleCommand(cmd->id,m_parms);
}

/*
==========================================
See if the first arg matches any funcs or commands.
exec func if it does.
==========================================
*/
/*bool CConsole::Exec(int argc, char** argv) 
{
	for(CmdList::iterator itcmd = m_lCmds.begin(); itcmd != m_lCmds.end(); itcmd ++)
	{
		if(strcmp(itcmd->name, argv[0])==0)
		{	itcmd->handler->HandleCommand(itcmd->id,argc,argv);
			return true;
		}
	}

	for (CVarList::iterator it = m_lCVars.begin(); it != m_lCVars.end(); it++)
	{
		if(!strcmp((*it)->name,argv[0]))
		{
			//only exec'ed IF function returns true
			if((!(*it)->handler) || (*it)->handler->HandleCVar((*it),argc,argv))
			{
				if(argc >= 2 && argv[1])
				{
					if((*it)->type != CVarBase::CVAR_STRING)
						(*it)->Set(argv[1]);
					else
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
						(*it)->Set(newstr);
					}
				}
				ComPrintf("%s = \"%s\"\n",(*it)->name,(*it)->string);
			}
			return true;
		}
	}
	return false;
}
*/

/*
=======================================

=======================================
*/
void CConsole::ExecString(const char *string) //const
{
	m_parms = string;
	const char * szfirstArg = m_parms.StringTok(0);

	for(CmdList::iterator itcmd = m_lCmds.begin(); itcmd != m_lCmds.end(); itcmd ++)
	{
		if(strcmp(itcmd->name, szfirstArg)==0)
		{	itcmd->handler->HandleCommand(itcmd->id,m_parms);
			return;
		}
	}

	for (CVarList::iterator it = m_lCVars.begin(); it != m_lCVars.end(); it++)
	{
		if(!strcmp((*it)->name,szfirstArg))
		{
			//only exec'ed IF function returns true
			if((!(*it)->handler) || (*it)->handler->HandleCVar((*it),m_parms))
			{
				int numtokens = 0;
				if(numtokens = m_parms.NumTokens() > 1)
				{
					const char * argVal = m_parms.StringTok(1);
					
					if((*it)->type != CVarBase::CVAR_STRING)
						(*it)->Set(argVal);
					else
					{
						char newstr[80];
						const char * arg=0;
						strcpy(newstr,argVal);
				
						for(int i=2,len=0;i<numtokens;i++)
						{
							arg = m_parms.StringTok(i);
							len += strlen(arg+1);
							if(len >= 80)
								break;
							strcat(newstr," ");
							strcat(newstr,arg);
						}
						(*it)->Set(newstr);
					}
				}
				ComPrintf("%s = \"%s\"\n",(*it)->name,(*it)->string);
			}
			return;
		}
	}
	ComPrintf("%s\n",string);
/*	int nargc = Util::BufParse(string, m_szargv);

	if((nargc) && !Exec(nargc,m_szargv))
	{
		ComPrintf("%s\n",string);
	}
	m_conString.erase();
*/
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

/*
==========================================
Handle Console command
==========================================
*/
void CConsole::HandleCommand(HCMD cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_CVARLIST:
		CVarlist(parms);
		break;
	case CMD_CMDLIST:
		CCmdList(parms);
		break;
	case CMD_TEST:
		CFunctest(parms);
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
	for (CVarList::iterator it = m_lCVars.begin(); it != m_lCVars.end(); it++)
	{
		if((*it)->flags & CVarBase::CVAR_ARCHIVE)
		{
			char line[80];
			strcpy(line,(*it)->name);
			strcat(line," ");
			strcat(line,(*it)->string);
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

void CConsole::SetFullscreen(bool full)
{	
	m_prCons->ToggleFullscreen(full);
}

void CConsole::SetVisible(bool down)
{	
	System::GetInputFocusManager()->SetCursorListener(0);
	System::GetInputFocusManager()->SetKeyListener(this,true);
	m_prCons->Toggle(down);
}

/*
======================================
Prints out all the CVars and their values
======================================
*/
void CConsole::CVarlist(const CParms &parms)
{
	ComPrintf("Console Variable Listing\n");
	ComPrintf("========================\n");

	CVarList::iterator it;
	
	if(parms.NumTokens() >=2)
	{
		const char * arg = parms.StringTok(1);
		int len= strlen(arg);
		for(it = m_lCVars.begin(); it != m_lCVars.end(); it++)
		{
			if(strncmp((*it)->name,arg,len)==0)
				ComPrintf("\"%s\" is \"%s\"\n",(*it)->name,(*it)->string);
		}
	}
	else
	{	
		for(it = m_lCVars.begin(); it != m_lCVars.end(); it++)
			ComPrintf("\"%s\" is \"%s\"\n",(*it)->name,(*it)->string);
	}

}

/*
======================================
Prints out all the CVars and their values
======================================
*/
void CConsole::CCmdList(const CParms &parms)
{
	ComPrintf("Console Command Listing\n");
	ComPrintf("========================\n");

	CmdList::iterator it;

	if(parms.NumTokens() >=2)
	{
		const char * arg = parms.StringTok(1);
		int len= strlen(arg);
		for(it = m_lCmds.begin(); it != m_lCmds.end(); it ++)
		{
			if(!strncmp(it->name,arg,len))
				ComPrintf("%s\n",it->name);
		}
	}
	else
	{
		for(it = m_lCmds.begin(); it != m_lCmds.end(); it ++)
			ComPrintf("%s\n",it->name);
	}
}

/*
=====================================
Util test func
=====================================
*/
void CConsole::CFunctest(const CParms &parms)
{
/*	if(argc > 1 && argv[1])
	{
		CParms blah(20);
		const char * tok=0;
		int i=0;
		blah = " bwa heh bla ";
		ComPrintf("Tokens in <%s> = %d\n", blah.string, blah.NumTokens());
		i=0; tok = 0;
		while(tok = blah.GetToken(i))
		{
			ComPrintf("%s\n",tok);
			i++;
		}
		blah = "blah heh b";
		ComPrintf("Tokens in <%s> = %d\n", blah.string, blah.NumTokens());
		i=0; tok = 0;
		while(tok = blah.GetToken(i))
		{
			ComPrintf("%s\n",tok);
			i++;
		}
		blah = "blah";
		ComPrintf("Tokens in <%s> = %d\n", blah.string, blah.NumTokens());
		i=0; tok = 0;
		while(tok = blah.GetToken(i))
		{
			ComPrintf("%s\n",tok);
			i++;
		}
	}
*/
}


/*
==========================================
Register CVar
Initialize CVar and alphabetically add to list
==========================================
*/
void CConsole::RegisterCVar(CVarBase * var,	I_CVarHandler * handler)
{
	if(handler)
		var->handler = handler;

	for(CVarList::iterator it = m_lCVars.begin(); it != m_lCVars.end(); it++)
	{
		if(strcmp((*it)->name, var->name) > 0)
			break;
	}
	m_lCVars.insert(it,var);
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
	for(CmdList::iterator it = m_lCmds.begin(); it != m_lCmds.end(); it++)
	{
		if(strcmp(it->name, cmdname) > 0)
			break;
	}
	m_lCmds.insert(it,CCommand(cmdname,id,handler));
}


CCommand * CConsole::GetCommandByName(const char * cmdString)
{
	for(CmdList::iterator it = m_lCmds.begin(); it != m_lCmds.end(); it ++)
	{
		if(!strcmp(cmdString,it->name))
			return &(*it);
	}
	return 0;
}