#include "Cl_main.h"
#include "Cl_cmds.h"
#include "Sys_cons.h"

//======================================================================================
//======================================================================================

CClientCmdHandler::CClientCmdHandler(CClient * pclient)
{
	m_pClient = pclient;

	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
		m_cmdBuffer[i] = 0;
}

CClientCmdHandler::~CClientCmdHandler()
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
		m_cmdBuffer[i] = 0;
}


void CClientCmdHandler::SetListenerState(bool on)
{
	if(on = true)
	{
		GetInputFocusManager()->SetCursorListener(this);
		GetInputFocusManager()->SetKeyListener(this,false);
	}
	else
	{
		GetInputFocusManager()->SetCursorListener(0);
		GetInputFocusManager()->SetKeyListener(0);
	}
}


void CClientCmdHandler::RunCommands()
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_cmdBuffer[i])
			g_pConsole->ExecCommand(m_cmdBuffer[i]->pCmd, m_cmdBuffer[i]->szCommand);
	}
}


void CClientCmdHandler::HandleKeyEvent(const KeyEvent_t &kevent)
{
	//check if there is a command bound to that key
	if(m_cmdKeys[(kevent.id)].szCommand)
	{
		//if the command is supposed to be executed everyframe
		//until its is released
		if(m_cmdKeys[(kevent.id)].szCommand[0] == '+')
		{
			//if its a keydown event
			if(kevent.state == BUTTONDOWN)
			{
				//add to command buffer
				AddToCmdBuffer(&m_cmdKeys[(kevent.id)]);
			}
			else if(kevent.state == BUTTONUP)
			{
				//otherwise remove from buffer
				RemoveFromCmdBuffer(&m_cmdKeys[(kevent.id)]);
			}
		}
		//if its regular function and if its a keydown event,
		else if(kevent.state == BUTTONDOWN)
		{
			//Send over to the console for execution
			g_pConsole->ExecString(m_cmdKeys[(kevent.id)].szCommand);
		}
	}
}


void CClientCmdHandler::HandleCursorEvent(const float &ix,
										  const float &iy,
										  const float &iz)
{
	m_pClient->RotateRight(ix);
	m_pClient->RotateUp(iy);
}


void CClientCmdHandler::BindFuncToKey(int argc, char** argv)
{
	//no arguments
	if(argc==1)
	{
		ComPrintf("Usage : bind <key> <command>\n");
		return;
	}

	byte keynum= 0;

	//Check the name of the key being bound to
	if(strlen(argv[1]) > 1)
	{
		//Find Keyname in table if its bigger than a char
		for(int x=0;keytable[x].key;x++)
		{
			if(!stricmp(keytable[x].key,argv[1]))
			{
				keynum = keytable[x].val;
				break;
			}
		}
	}
	else
	{
		//make sure that the keynum is valid
		if((argv[1][0] <= 0) || (argv[1][0] > 255))
		{
			ComPrintf("Bind : Error %s(%d) is not a valid key.\n",argv[1],keynum);
			return;
		}
		keynum = argv[1][0];
	}
		
	//Only two args, just show binding for the key and return
	if(argc == 2)
	{
		if(m_cmdKeys[keynum].szCommand && m_cmdKeys[keynum].pCmd)
			ComPrintf("\"%s\" = \"%s\"\n", argv[1], m_cmdKeys[keynum].szCommand);
		else
			ComPrintf("\"%s\" = \"\"\n", argv[1]);
		return;
	}

	m_cmdKeys[keynum].pCmd =g_pConsole->GetCommandByName(argv[2]);
	if(m_cmdKeys[keynum].pCmd < 0)
	{
		ComPrintf("Bind : %s is not a valid command\n",argv[2]);
		return;
	}
	
	//If there are more arguments then just the functions name
	//then they will be used as function paramters
	
	//Get their length and allocate space
	int parmlen = 1;

	for(int i=2;i<argc;i++)
		parmlen += strlen(argv[i]);

	if(m_cmdKeys[keynum].szCommand)
		delete [] m_cmdKeys[keynum].szCommand;
	m_cmdKeys[keynum].szCommand = new char[parmlen];

	//Copy the parms into ONE string
	char *p= m_cmdKeys[keynum].szCommand;
	for(int x=2;x<=argc;x++)
	{
		char *c = argv[x];
		while(*c && * c!='\0')
		{
			*p=*c;
			c++;
			p++;
		}
		//no more strings, break
		if((x+1)>=argc)
		{
			*p = '\0';
			break;
		}
		//add a space
		*p = ' ';
		p++;
	}

	ComPrintf("\"%s\"(%d) = \"%s\"\n",argv[1],keynum, m_cmdKeys[keynum].szCommand);
}


void CClientCmdHandler::Unbind(int argc, char** argv)
{
	if(argc <2)
	{
		ComPrintf("Usage - <unbind> <keyname>\n");
		return;
	}

	byte keynum= 0;

	//not a special KEY
	if(strlen(argv[1]) > 1)
	{
		for(int x=0;keytable[x].key;x++)
		{
			if(!stricmp(keytable[x].key,argv[1]))
			{
				keynum = keytable[x].val;
				break;
			}
		}
	}
	else
	{
		keynum = argv[1][0];
		//make sure that the key is valid
		if((keynum == 0) || (keynum > 255))
		{
			ComPrintf("Unbind : Error %s(%d) is not a valid key\n",argv[1],keynum);
			return;
		}
	}
		
	if(m_cmdKeys[keynum].szCommand && m_cmdKeys[keynum].pCmd)
	{
		m_cmdKeys[keynum].pCmd = 0;
		delete [] m_cmdKeys[keynum].szCommand;
		m_cmdKeys[keynum].szCommand=0;
	}
	ComPrintf("\"%s\" = \"\"\n",argv[1]);
}


void CClientCmdHandler::BindList() const
{
	ComPrintf(" Client Bindings \n");
	ComPrintf("=================\n");
	
	for(unsigned int i=0;i<256;i++)
	{
		if(m_cmdKeys[i].szCommand && m_cmdKeys[i].pCmd)
		{
			char keyname[16];
			bool hit=false;
			
			for(int x=0;keytable[x].key;x++)
			{
				if(keytable[x].val == i)
				{
					strcpy(keyname,keytable[x].key);
					hit = true;
					break;
				}
			}
			if(!hit)
			{
				keyname[0] = i;
				keyname[1] = '\0';
			}
			ComPrintf("\"%s\" = \"%s\"\n",keyname, m_cmdKeys[i].szCommand);
		}
	}
}

void CClientCmdHandler::Unbindall()
{
	for(int i=0;i<256;i++)
	{
		if(m_cmdKeys[i].szCommand && m_cmdKeys[i].pCmd)
		{
			delete [] m_cmdKeys[i].szCommand;
			m_cmdKeys[i].szCommand=0;
			m_cmdKeys[i].pCmd = 0;
		}
	}
	ComPrintf("Unbound all keys.\n");
}


void CClientCmdHandler::AddToCmdBuffer(ClientKey * const pcommand)
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_cmdBuffer[i] == pcommand)
			return;

		if(m_cmdBuffer[i] == 0)
		{
			m_cmdBuffer[i] = pcommand;
			return;
		}
	}
	ComPrintf("Command Buffer is FULL");
}

void CClientCmdHandler::RemoveFromCmdBuffer(const ClientKey * pcommand)
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_cmdBuffer[i] == pcommand)
		{
			m_cmdBuffer[i] = 0;
			return;
		}
	}
}


void CClientCmdHandler::WriteBindTable(FILE *fp)
{
	for(unsigned int i=0;i<256;i++)
	{
		//there is a binding for this key
		if(m_cmdKeys[i].szCommand && m_cmdKeys[i].pCmd)
		{
			char line[80];
			bool hit=false;
			
			strcpy(line,"bind ");
			for(unsigned int x=0;keytable[x].val;x++)
			{
				if(keytable[x].val == i)
				{
					strcat(line,keytable[x].key);
					hit = true;
					break;
				}
			}
			if(!hit)
			{
				char p[2];
				p[0] = i;
				p[1] = '\0';
				strcat(line,p);
			}
			
			strcat(line," ");
			strcat(line, m_cmdKeys[i].szCommand);
			strcat(line,"\n");
			fputs(line,fp);
		}
	}
}