#include "Sys_hdr.h"
#include "Sys_cvar.h"
#include "Sys_cons.h"
#include "I_renderer.h"
#include "Com_util.h"
#include <direct.h>

//Private stuff
namespace
{
	enum
	{
		CMD_CVARLIST = 1,
		CMD_CMDLIST  = 2,
		CMD_TEST	 = 3
	};
	const int  CON_MAXARGSIZE  = 80;
	const int  CON_MAXCONFIGLINES = 256;
	const int  CON_MAXBUFFEREDCMDS = 32;
}

I_Console * I_Console::GetConsole()
{	return System::GetConsole();
}

/*
======================================
Constructor
======================================
*/ 
CConsole::CConsole(const char * curPath) : m_parms(CON_MAXARGSIZE)
{
	m_itCmd = 0;
	m_prCons = 0;

	//open logfile
	char debugfilename[COM_MAXPATH];
	strcpy(debugfilename,curPath);
	strcat(debugfilename,"debug.log");

	m_hLogFile = ::CreateFile(debugfilename,GENERIC_WRITE, FILE_SHARE_READ, 0,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);

#ifdef DOSCONS
	::AllocConsole();
	::SetConsoleTitle("Void Debug");
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
	if(m_hLogFile != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hLogFile);

#ifdef DOSCONS
	ComPrintf("CConsole::Shutting down Console\n");
	::CloseHandle(m_hIn);
	::CloseHandle(m_hOut);
	::FreeConsole();
#endif

	m_prCons = 0;
}

/*
==========================================
Set the Renderer
==========================================
*/
void CConsole::SetConsoleRenderer(I_ConsoleRenderer * pRCons)
{	m_prCons = pRCons;
}

/*
===============================================
print a string to debugging window	
===============================================
*/
void CConsole::ComPrint(const char* text)
{
	if (!text)
		return;

#ifdef DOSCONS
	DWORD num;
	WriteConsole(m_hOut, text, strlen(text), &num, NULL);
#endif
	
	//write to log
	ulong bytesWritten =0;
	if(m_hLogFile != INVALID_HANDLE_VALUE)
		WriteFile(m_hLogFile,text, strlen(text), &bytesWritten, 0);

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
				//if it wasnt a blank line, enter it into the command buffer
				if(!m_conString.length())
					ComPrintf("]\n");
				else
				{
					if(m_cmdBuffer.size() == CON_MAXBUFFEREDCMDS)
						m_cmdBuffer.pop_front();
					m_cmdBuffer.push_back(std::string(m_conString));
					m_itCmd = m_cmdBuffer.end();

					ComPrintf("]%s\n",m_conString.c_str());

					//Exec the string
					if(!ExecString(m_conString.c_str()))
					{
						if(System::GetGameState() == INGAMECONSOLE)
						{
							m_conString.insert(0,"say ");
							ExecString(m_conString.c_str());
						}
						else
							ComPrintf("Unknown command \"%s\"\n",m_conString.c_str());
					}
					//reset the buffer
					m_conString.erase();
				}
				break;
			}
			case INKEY_TAB:
			{
				//Print all the CVars and cmds that match the given string
				if(!m_conString.length())
					return;

				StrList	matchingNames;
				int	len	= m_conString.size();

				for(CVarListIt itVar= m_lCVars.begin(); itVar != m_lCVars.end(); itVar++)
				{
					if(strncmp(m_conString.c_str(),(*itVar)->name,len) == 0)
						matchingNames.push_back(std::string((*itVar)->name));
				}

				for(CmdListIt itCmd = m_lCmds.begin(); itCmd != m_lCmds.end(); itCmd ++)
				{
					if(strncmp(m_conString.c_str(),itCmd->name,len) == 0)
						matchingNames.push_back(std::string(itCmd->name));
				}
				
				//Sort and print all the matched entries
				int numMatches = matchingNames.size();
				if(!numMatches)
					ComPrintf("No Matches\n");
				else if(numMatches == 1)
					m_conString.assign(matchingNames.front());
				else
				{
					matchingNames.sort();
					for(StrListIt it = matchingNames.begin(); it != matchingNames.end(); it++)
						ComPrintf("%s\n", it->c_str());
				}
				ComPrintf("==========================\n");
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
				m_prCons->MoveCurrentLine(I_ConsoleRenderer::LINE_UP);
				break;
			}
			case INKEY_PGDN:
			{
				m_prCons->MoveCurrentLine(I_ConsoleRenderer::LINE_DOWN);
				break;
			}
			case INKEY_HOME:
			{
				m_prCons->MoveCurrentLine(I_ConsoleRenderer::TOP);
				break;
			}
			case INKEY_END:
			{
				m_prCons->MoveCurrentLine(I_ConsoleRenderer::BOTTOM);
				break;
			}
			default:	
			{
				//ascii char entered, add to line
				if(kevent.id >= 0 && kevent.id <= 127)	
					m_conString.append(1,kevent.id);
				break;
			}
		}
		m_prCons->SetStatusline(m_conString.c_str(),m_conString.size()+1);
	}
}

/*
=======================================
Try to execute a string in the console
=======================================
*/
bool CConsole::ExecString(const char *string)
{
	m_parms = string;
	const char * szfirstArg = m_parms.StringTok(0,m_szParmBuffer,MAX_CONSOLE_BUFFER);

	//Try matching with registered commands
	for(CmdListIt itcmd = m_lCmds.begin(); itcmd != m_lCmds.end(); itcmd ++)
	{
		if(strcmp(itcmd->name, szfirstArg)==0)
		{	
			itcmd->handler->HandleCommand(itcmd->id,m_parms);
			return true;
		}
	}

	//Try matching with registered cvars
	for (CVarListIt it = m_lCVars.begin(); it != m_lCVars.end(); it++)
	{
		if(strcmp((*it)->name,szfirstArg)==0)
		{
			CVarImpl * pVar = *it;

			if(pVar->flags & CVAR_READONLY)
			{
				if(m_parms.NumTokens() > 1)
					ComPrintf("%s is a read-only variable\n",  pVar->name);
				ComPrintf("%s = \"%s\"\n",pVar->name,pVar->string);
				return true;
			}

			if(m_parms.NumTokens() > 1)
			{
				const char * argVal = m_parms.StringTok(1,m_szParmBuffer,MAX_CONSOLE_BUFFER);
				
				if((!pVar->handler) || pVar->handler->HandleCVar(pVar,CStringVal(argVal)))
				{
					pVar->Set(argVal);
					if(pVar->flags & CVAR_LATCH)
					{
						ComPrintf("%s will be changed on restart\n",pVar->name);
						return true;
					}
				}
			}
			ComPrintf("%s = \"%s\" : default = \"%s\"\n",pVar->name,pVar->string, pVar->default_string);
			return true;
		}
	}
	return false;
}

/*
==========================================
Proxy functions 
==========================================
*/
void CConsole::SetFullscreen(bool full)
{	m_prCons->ToggleFullscreen(full);
}

void CConsole::SetVisible(bool down)
{	
	System::GetInputFocusManager()->SetCursorListener(0);
	System::GetInputFocusManager()->SetKeyListener(this,true);
	m_prCons->Toggle(down);
}

/*
==========================================
Register CVar
Initialize CVar and alphabetically add to list
==========================================
*/
//void CConsole::RegisterCVar(CVarBase * var,	I_ConHandler * handler)
CVar * CConsole::RegisterCVar(const char * varName,
					const char *varVal, 
					CVarType varType,	
					int varFlags,
					I_ConHandler * pHandler)
{
	int ret = 0;
	
	//Insert in the proper place alphabetically
	for(CVarListIt it = m_lCVars.begin(); it != m_lCVars.end(); it++)
	{
		ret = strcmp((*it)->name, varName);
		if(ret > 0)
			break;
		
		//found a cvar using the same name. just return that.
		if(ret == 0)
		{
			ComPrintf("CConsole::RegisterCVar: WARNING, %s was re-registered\n", varName);
			return *it;
		}
		
	}

	CVarImpl * pVar = new CVarImpl(varName,varVal,varType,varFlags);
	if(pHandler)
		pVar->handler = pHandler;

	//Find archived keyval and set it to that.if its not readonly
	if(!(pVar->flags & CVAR_READONLY))
		UpdateCVarFromArchive(pVar);
	
	//Insert into the list
	m_lCVars.insert(it,pVar);
	return pVar;
}


/*
================================================
Unregister a handler func
================================================
*/
void CConsole::UnregisterHandler(I_ConHandler * pHandler)
{
	if(!pHandler || pHandler == this)
		return;

	for(CVarListIt it = m_lCVars.begin(); it != m_lCVars.end();)
	{
		if((*it)->handler == pHandler)
		{
			CVarImpl * pVar = *it;
			WriteCVarToArchive(pVar);
			delete pVar;
			m_lCVars.erase(it++);
		}
		else
			it++;
	}

	for(CmdListIt itCmd = m_lCmds.begin(); itCmd != m_lCmds.end();)
	{
		if(itCmd->handler == pHandler)
			m_lCmds.erase(itCmd++);
		else
			itCmd++;
	}
}

/*
==========================================
Register CFunc
==========================================
*/
void CConsole::RegisterCommand(const char *cmdname,   int id,   I_ConHandler * pHandler)
{
	for(CmdListIt it = m_lCmds.begin(); it != m_lCmds.end(); it++)
	{
		if(strcmp(it->name, cmdname) > 0)
			break;
	}
	
	m_lCmds.insert(it,CCommand(cmdname,id,pHandler));
}

/*
======================================
Only needed by the client
======================================
*/
CCommand * CConsole::GetCommandByName(const char * cmdString)
{
	for(CmdListIt it = m_lCmds.begin(); it != m_lCmds.end(); it ++)
	{
		if(!strcmp(cmdString,it->name))
			return &(*it);
	}
	return 0;
}

//======================================================================================
//Console Commands
//======================================================================================

/*
==========================================
Handle Console command
==========================================
*/
void CConsole::HandleCommand(int cmdId, const CParms &parms)
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
======================================
Prints out all the CVars and their values
======================================
*/
void CConsole::CVarlist(const CParms &parms)
{
	ComPrintf("Console Variable Listing\n");
	ComPrintf("========================\n");

	CVarListIt it;
	
	if(parms.NumTokens() >=2)
	{
		const char * arg = parms.StringTok(1,m_szParmBuffer,MAX_CONSOLE_BUFFER);
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

	CmdListIt it;

	if(parms.NumTokens() >=2)
	{
		const char * arg = parms.StringTok(1,m_szParmBuffer,MAX_CONSOLE_BUFFER);
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
	ComPrintf("test");
	ComPrintf("blah\n\nBlah2\n");
}



//======================================================================================
//Command line
//======================================================================================

void CConsole::AddCmdLineParm(const char * cmdLine)
{	m_cmdLineParms.push_back(std::string(cmdLine));
}

bool CConsole::IsCmdLineParm(const char * token, int tokenLen)
{
	for(StrListIt it = m_cmdLineParms.begin(); it != m_cmdLineParms.end(); it++)
	{
		if(strncmp(it->c_str(), token, tokenLen)==0)
			return true;
	}
	return false;
}

void CConsole::ExecCmdLine()
{
	for(StrListIt itVal = m_cmdLineParms.begin(); itVal != m_cmdLineParms.end(); itVal++)
	{
		if(!ExecString(itVal->c_str()))
			ComPrintf("Unknown command \"%s\"\n",itVal->c_str());
	}
}

//======================================================================================
//Config files etc
//======================================================================================

/*
======================================
Looks through config file parms to see
if anything matches the given token.
will set parms to the matching token
it finds LAST
======================================
*/
void CConsole::UpdateCVarFromArchive(CVarImpl * pVar)
{
	CParms archivedParms(CON_MAXARGSIZE);
	const char * archivedString = 0;

	for(StrListIt itVal = m_cvarStrings.begin(); itVal != m_cvarStrings.end(); itVal++)
	{
		archivedParms = itVal->c_str();
		if(archivedParms.NumTokens() > 1)
		{
			//match the name
			if(strcmp(pVar->name,archivedParms.StringTok(0,m_szParmBuffer,MAX_CONSOLE_BUFFER))==0)
				archivedString = itVal->c_str();
		}
	}

	//Did we find a string ?
	if(archivedString)
	{
		archivedParms = archivedString;
		pVar->ForceSet(archivedParms.StringTok(1,m_szParmBuffer,MAX_CONSOLE_BUFFER));
	}
}

/*
================================================
Makes sure that the cvars value has been archived
================================================
*/
void CConsole::WriteCVarToArchive(CVarImpl * pCvar)
{
	CParms archivedParms(CON_MAXARGSIZE);
	StrListIt itMatched = m_cvarStrings.begin();

	for(StrListIt itVal = m_cvarStrings.begin(); itVal != m_cvarStrings.end(); itVal++)
	{
		archivedParms = itVal->c_str();
		if(archivedParms.NumTokens() > 1)
		{
			//match the name
			if(strcmp(pCvar->name,archivedParms.StringTok(0,m_szParmBuffer,MAX_CONSOLE_BUFFER))==0)
				itMatched = itVal;
		}
	}

	char line[80];
	sprintf(line,"%s \"%s\"\n", pCvar->name, pCvar->string);

	//We did find a match. Now we need to modify this string
	if(itMatched != m_cvarStrings.begin())
		*itMatched = line;
	else	//Didnt find a match. Need to add the string
		m_cvarStrings.push_back(std::string(line));
}

/*
======================================
Fill buffer with a parm from the config file
======================================
*/
int CConsole::ReadConfigParm(char * buf, int bufsize, FILE * fp)
{
	int len = 0;
	char c = fgetc(fp);

	if(c == EOF || c == '\0')
		return false;

	while(c && (c != '\n') && (c != EOF) && (len < bufsize))
	{
		buf[len++] = c;
		c = fgetc(fp);
	}
	buf[len] = '\0';
	return len;
}

/*
======================================
Load cvar data from a config file
======================================
*/
void CConsole::LoadConfig(const char * szFilename)
{
	char file[COM_MAXPATH];
	sprintf(file,"%s/%s",System::GetExePath(),szFilename);

	FILE * fpcfg = fopen(file,"r");
	if(fpcfg == NULL)
	{
		ComPrintf("CConsole::ExecConfig:Error opening %s\n",file);
		return;
	}

	m_cvarStrings.empty();

	int  lines=0;
	char line[CON_MAXARGSIZE];
	
	//Configs files are linited to 256 lines, 80 chars each
	do
	{
		if(!ReadConfigParm(line,CON_MAXARGSIZE,fpcfg))
			break;

		m_cvarStrings.push_back(std::string(line));
		memset(line,0,CON_MAXARGSIZE);
		lines ++;

	}while(lines < CON_MAXCONFIGLINES);

	fclose(fpcfg);
	ComPrintf("CConsole::Loaded %s\n",file);
}

/*
======================================
Executes a Config file
======================================
*/
void CConsole::ExecConfig(const char *szFilename)
{
	char file[128];
	sprintf(file,"%s/%s",System::GetExePath(),szFilename);

	FILE * fpcfg=fopen(file,"r");
	if(fpcfg == NULL)
	{
		ComPrintf("CConsole::ExecConfig:Error opening %s\n",file);
		return;
	}

	//Configs files are linited to 256 lines, 80 chars each
	int  lines=0;
	CParms parm(80);
	const char * firstParm;
	int firstParmLen;
	char line[CON_MAXARGSIZE];
	memset(line,0,CON_MAXARGSIZE);
	
	do
	{
		if(!ReadConfigParm(line,CON_MAXARGSIZE,fpcfg))
			break;
		
		parm.Set(line);
		firstParm = parm.StringTok(0,m_szParmBuffer,MAX_CONSOLE_BUFFER);
		firstParmLen = strlen(firstParm);

		//Ignore if the parm was specified in the commandline 
		//as that takes higher precedence
		if(IsCmdLineParm(firstParm,firstParmLen))
			continue;
		
		if(!ExecString(line))
			ComPrintf("Unknown command \"%s\"\n",line);

		memset(line,0,CON_MAXARGSIZE);
		lines ++;

	}while(lines < CON_MAXCONFIGLINES);

	fclose(fpcfg);
	ComPrintf("CConsole::Exec'ed %s\n",file);
}

/*
=====================================
Write CVars to given file
=====================================
*/
void CConsole::WriteCVars(const char * szFilename)
{
	char file[COM_MAXPATH];
	sprintf(file,"%s/%s",System::GetExePath(),szFilename);

	FILE * fp = fopen(file,"w");
	if(!fp)
	{
		ComPrintf("CConsole::WriteCVars: Error writing config file\n");
		return;
	}

	//write all the archive flaged vars in the config file
	for (CVarListIt it = m_lCVars.begin(); it != m_lCVars.end(); it++)
	{
		if((*it)->flags & CVAR_ARCHIVE)
		{
			char line[80];
			(*it)->Unlatch();
			sprintf(line,"%s \"%s\"\n", (*it)->name, (*it)->string);
			fputs(line,fp);
		}
	}
	fclose(fp);
}

