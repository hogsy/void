#include "Cl_main.h"
#include "Cl_cmds.h"
#include "Sys_cons.h"

//======================================================================================
//======================================================================================

/*
==========================================
Constructor/Destructor
==========================================
*/
CClientGameCmd::CClientGameCmd(CClient &owner) : 
							m_Parms(80), m_refClient(owner)
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
		m_cmdBuffer[i] = 0;
}

CClientGameCmd::~CClientGameCmd()
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
		m_cmdBuffer[i] = 0;
}


/*
==========================================
Activate/Deactivate Listener
==========================================
*/
void CClientGameCmd::SetListenerState(bool on)
{
	if(on = true)
	{
		System::GetInputFocusManager()->SetCursorListener(this);
		System::GetInputFocusManager()->SetKeyListener(this,false);
	}
	else
	{
		System::GetInputFocusManager()->SetCursorListener(0);
		System::GetInputFocusManager()->SetKeyListener(0);
	}
}

/*
==========================================
Run all the commands added into the buffer
==========================================
*/
void CClientGameCmd::RunCommands()
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_cmdBuffer[i])
		{
			m_Parms = m_cmdBuffer[i]->szCommand;
			m_cmdBuffer[i]->pCmd->handler->HandleCommand(m_cmdBuffer[i]->pCmd->id, m_Parms);
			if(m_cmdBuffer[i]->szCommand[0] != '+')
				m_cmdBuffer[i] = 0;
		}
	}
}

/*
==========================================
Handle Key Event
==========================================
*/
void CClientGameCmd::HandleKeyEvent(const KeyEvent &kevent)
{
	//check if there is a command bound to that key
	if(m_cmdKeys[(kevent.id)].szCommand[0])
	{
		//if its a keydown event
		if(kevent.state == BUTTONDOWN)
		{
			//add to command buffer
			AddToCmdBuffer(&m_cmdKeys[(kevent.id)]);
		}
		else if((kevent.state == BUTTONUP) && (m_cmdKeys[(kevent.id)].szCommand[0] == '+'))
		{
			//otherwise remove from buffer
			RemoveFromCmdBuffer(&m_cmdKeys[(kevent.id)]);
		}
	}
}

/*
==========================================
Handle Cursor Move Event
==========================================
*/
void CClientGameCmd::HandleCursorEvent(const float &ix,
										  const float &iy,
										  const float &iz)
{
	m_refClient.RotateRight(ix);
	m_refClient.RotateUp(iy);
}


/*
==========================================
Bind a command to a key
==========================================
*/
void CClientGameCmd::BindFuncToKey(const CParms &parms)
{
	//no arguments
	int argc = parms.NumTokens();
	if(argc==1)
	{
		ComPrintf("Usage : bind <key> <command>\n");
		return;
	}

	byte keynum= 0;

	char keyName[32];
	parms.StringTok(1,keyName,32);

	//Check the name of the key being bound to
	if(strlen(keyName) > 1)
	{
		//Find Keyname in table if its bigger than a char
		for(int x=0;keytable[x].key;x++)
		{
			if(!_stricmp(keytable[x].key,keyName))
			{
				keynum = keytable[x].val;
				break;
			}
		}
	}
	else
	{
		//make sure that the keynum is valid
		if((keyName[0] <= 0) || (keyName[0] > 255))
		{
			ComPrintf("Bind : Error %s(%d) is not a valid key.\n",keyName,keynum);
			return;
		}
		keynum = keyName[0];
	}
		
	//Only two args, just show binding for the key and return
	if(argc == 2)
	{
		ComPrintf("\"%s\" = \"%s\"\n", keyName, m_cmdKeys[keynum].szCommand);
		return;
	}

	char cmdName[32];
	parms.StringTok(2, cmdName, 32);

	m_cmdKeys[keynum].pCmd = ((CConsole*)System::GetConsole())->GetCommandByName(cmdName);
	if(m_cmdKeys[keynum].pCmd == 0)
	{
		ComPrintf("Bind : %s is not a valid command\n",cmdName);
		return;
	}

	strcpy(m_cmdKeys[keynum].szCommand, cmdName);

	//copy token 3 and up to the command
/*	for(int i=3; i< argc; i++)
	{
		strcat(m_cmdKeys[keynum].szCommand," ");
		strcat(m_cmdKeys[keynum].szCommand, parms.StringTok(i));
	}
*/
	ComPrintf("\"%s\"(%d) = \"%s\"\n", keyName, keynum, m_cmdKeys[keynum].szCommand);
}


/*
======================================
Default key bindings
======================================
*/
void CClientGameCmd::IntializeBinds()
{
	CParms parms(80);

	parms = "bind ` contoggle";
	BindFuncToKey(parms);
	parms = "bind o +forward";
	BindFuncToKey(parms);
	parms = "bind l +back";
	BindFuncToKey(parms);
	parms = "bind k +moveleft";
	BindFuncToKey(parms);
	parms = "bind ; +moveright";
	BindFuncToKey(parms);
	parms = "bind a +lookup";
	BindFuncToKey(parms);
	parms = "bind z +lookdown";
	BindFuncToKey(parms);
	parms = "bind q quit";
	BindFuncToKey(parms);
	parms = "bind UPARROW +forward";
	BindFuncToKey(parms);
	parms = "bind DOWNARROW +back";
	BindFuncToKey(parms);
	parms = "bind LEFTARROW +left";
	BindFuncToKey(parms);
	parms = "bind RIGHTARROW +right";
	BindFuncToKey(parms);

	//Exec binds file
	((CConsole*)System::GetConsole())->ExecConfig("vbinds.cfg");
}

/*
==========================================
Unbind a key
==========================================
*/
void CClientGameCmd::Unbind(const CParms &parms)
{
	if(parms.NumTokens() <2)
	{
		ComPrintf("Usage - <unbind> <keyname>\n");
		return;
	}

	byte keynum= 0;
	char keyName[32];
	parms.StringTok(1,keyName,32);

	//not a special KEY
	if(strlen(keyName) > 1)
	{
		for(int x=0;keytable[x].key;x++)
		{
			if(!_stricmp(keytable[x].key,keyName))
			{
				keynum = keytable[x].val;
				break;
			}
		}
	}
	else
	{
		keynum = keyName[0];
		//make sure that the key is valid
		if((keynum == 0) || (keynum > 255))
		{
			ComPrintf("Unbind : Error %s(%d) is not a valid key\n",keyName,keynum);
			return;
		}
	}
		
	if(m_cmdKeys[keynum].szCommand && m_cmdKeys[keynum].pCmd)
	{
		m_cmdKeys[keynum].pCmd = 0;
		m_cmdKeys[keynum].szCommand[0] = 0;
	}
	ComPrintf("\"%s\" = \"\"\n",keyName);
}

/*
==========================================
Print out a list of current binds
==========================================
*/
void CClientGameCmd::BindList() const
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

/*
==========================================
Unbind all the keys
==========================================
*/
void CClientGameCmd::Unbindall()
{
	for(int i=0;i<256;i++)
	{
		if(m_cmdKeys[i].szCommand && m_cmdKeys[i].pCmd)
		{
			m_cmdKeys[i].szCommand[0]=0;
			m_cmdKeys[i].pCmd = 0;
		}
	}
	ComPrintf("Unbound all keys.\n");
}

/*
==========================================
Add command to execute buffer
==========================================
*/
void CClientGameCmd::AddToCmdBuffer(ClientKey * const pcommand)
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

/*
==========================================
Remove from execute buffer
==========================================
*/
void CClientGameCmd::RemoveFromCmdBuffer(const ClientKey * pcommand)
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

/*
==========================================
Write all the bound commands to file
==========================================
*/
void CClientGameCmd::WriteBinds(const char * szBindsfile)
{
	char file[COM_MAXPATH];
	sprintf(file,"%s/%s",System::GetExePath(),szBindsfile);

	FILE * fp = fopen(file,"w");
	if(!fp)
	{
		ComPrintf("CClientGameCmd::WriteBinds\n");
		return;
	}

	for(int i=0;i<256;i++)
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
	fclose(fp);
}

