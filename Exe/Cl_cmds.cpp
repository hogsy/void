#include "Sys_hdr.h"
#include "Cl_cmds.h"

/*
==========================================
Constructor/Destructor
==========================================
*/
CClientGameInput::CClientGameInput() 
		: m_fXpos(0.0f), m_fYpos (0.0f), m_fZpos(0.0f),
		  m_bCursorChanged(false)
{
	memset(m_cmdKeys,  0,sizeof(char*)*IN_NUMKEYS);
	memset(m_cmdBuffer,0,sizeof(char*)*CL_CMDBUFFERSIZE);
}

CClientGameInput::~CClientGameInput()
{
	for(int i=0;i<IN_NUMKEYS;i++)
	{
		if(m_cmdKeys[i])
		{
			delete [] m_cmdKeys[i];
			m_cmdKeys[i] = 0;
		}
	}
	memset(m_cmdBuffer,0,sizeof(char*)*CL_CMDBUFFERSIZE);
}


/*
================================================
Return the updated cursor position
================================================
*/
void CClientGameInput::UpdateCursorPos(float &ix, float &iy, float &iz)
{
	m_bCursorChanged = false;
	ix = m_fXpos;
	iy = m_fYpos;
	iz = m_fZpos;
}

/*
==========================================
Run all the commands added into the buffer
==========================================
*/
void CClientGameInput::RunCommands()
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_cmdBuffer[i])
			I_Console::GetConsole()->AddToCmdBuffer(m_cmdBuffer[i]);
	}
}


/*
==========================================
Handle Key Event
==========================================
*/
void CClientGameInput::HandleKeyEvent(const KeyEvent &kevent)
{
	//check if there is a command bound to that key
	if(m_cmdKeys[(kevent.id)])
	{
		//if its a keydown event
		if(kevent.state == BUTTONDOWN)
		{
			if(m_cmdKeys[(kevent.id)][0] == '+')
				AddToCmdBuffer(m_cmdKeys[(kevent.id)]);
			else
				I_Console::GetConsole()->AddToCmdBuffer(m_cmdKeys[(kevent.id)]);
		}
		else if(kevent.state == BUTTONUP)
		{
			//otherwise remove from buffer
			if(m_cmdKeys[(kevent.id)][0] == '+')
				RemoveFromCmdBuffer(m_cmdKeys[(kevent.id)]);
		}
	}
}

/*
==========================================
Handle Cursor Move Event
==========================================
*/
void CClientGameInput::HandleCursorEvent(const float &ix, const float &iy, const float &iz)
{
	m_bCursorChanged = true;
	m_fXpos = ix;
	m_fYpos = iy;
	m_fZpos = iz;
}


/*
==========================================
Bind a command to a key
==========================================
*/
void CClientGameInput::BindFuncToKey(const CParms &parms, bool bPrint)
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
		if(m_cmdKeys[keynum])
			ComPrintf("\"%s\" = \"%s\"\n", keyName, m_cmdKeys[keynum]);
		else
			ComPrintf("\"%s\" = \" \"\n", keyName);
		return;
	}

	char cmdName[80];
	parms.StringTok(2, cmdName, 80);

	if(m_cmdKeys[keynum])
		delete [] m_cmdKeys[keynum];

	m_cmdKeys[keynum] = new char[strlen(cmdName)+1];
	strcpy(m_cmdKeys[keynum], cmdName);
	
	if(bPrint)
		ComPrintf("\"%s\"(%d) = \"%s\"\n", keyName, keynum, m_cmdKeys[keynum]);
}


/*
======================================
Default key bindings
Yes, I know this looks horrible =)
======================================
*/
void CClientGameInput::IntializeBinds()
{
	CParms parms(80);

	parms = "bind ` contoggle";
	BindFuncToKey(parms, false);
	parms = "bind o +forward";
	BindFuncToKey(parms, false);
	parms = "bind l +back";
	BindFuncToKey(parms, false);
	parms = "bind k +moveleft";
	BindFuncToKey(parms, false);
	parms = "bind ; +moveright";
	BindFuncToKey(parms, false);
	parms = "bind a +lookup";
	BindFuncToKey(parms, false);
	parms = "bind z +lookdown";
	BindFuncToKey(parms, false);
	parms = "bind q quit";
	BindFuncToKey(parms, false);
	parms = "bind UPARROW +forward";
	BindFuncToKey(parms, false);
	parms = "bind DOWNARROW +back";
	BindFuncToKey(parms, false);
	parms = "bind LEFTARROW +left";
	BindFuncToKey(parms, false);
	parms = "bind RIGHTARROW +right";
	BindFuncToKey(parms, false);
	parms = "bind SPACE jump";
	BindFuncToKey(parms, false);
	parms = "bind MOUSE2 +crouch";
	BindFuncToKey(parms, false);
//	parms = "bind RSHIFT +walk";
//	BindFuncToKey(parms, false);

	//Exec binds file
	ExecBindsFile("vbinds.cfg");
}

/*
==========================================
Unbind a key
==========================================
*/
void CClientGameInput::Unbind(const CParms &parms)
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

	if(m_cmdKeys[keynum])
	{
		delete [] m_cmdKeys[keynum];
		m_cmdKeys[keynum] = 0;
	}
		
	ComPrintf("\"%s\" = \"\"\n",keyName);
}

/*
==========================================
Print out a list of current binds
==========================================
*/
void CClientGameInput::BindList() const
{
	ComPrintf("Client Bindings\n");
	ComPrintf("=================\n");
	
	for(unsigned int i=0;i<256;i++)
	{
		if(m_cmdKeys[i])
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
			ComPrintf("\"%s\" = \"%s\"\n",keyname, m_cmdKeys[i]);
		}
	}
}

/*
==========================================
Unbind all the keys
==========================================
*/
void CClientGameInput::Unbindall()
{
	for(int i=0;i<256;i++)
	{
		if(m_cmdKeys[i])
		{
			delete [] m_cmdKeys[i];
			m_cmdKeys[i] = 0;
		}
	}
	ComPrintf("Unbound all keys.\n");
}

/*
==========================================
Add command to execute buffer
==========================================
*/
void CClientGameInput::AddToCmdBuffer(const char * pcommand)
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
void CClientGameInput::RemoveFromCmdBuffer(const char * pcommand)
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
void CClientGameInput::WriteBinds(const char * szBindsfile)
{
	char file[COM_MAXPATH];
	sprintf(file,"%s/%s",System::GetExePath(),szBindsfile);

	FILE * fp = fopen(file,"w");
	if(!fp)
	{
		ComPrintf("CClientGameInput::WriteBinds\n");
		return;
	}

	for(int i=0;i<256;i++)
	{
		//there is a binding for this key
		if(m_cmdKeys[i])
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
			strcat(line, m_cmdKeys[i]);
			strcat(line,"\n");
			fputs(line,fp);
		}
	}
	fclose(fp);
}

/*
================================================
Exec the binds file
================================================
*/
void CClientGameInput::ExecBindsFile(const char * szBindsfile)
{
	char file[128];
	sprintf(file,"%s/%s",System::GetExePath(),szBindsfile);

	FILE * fpcfg=fopen(file,"r");
	if(fpcfg == NULL)
	{
		ComPrintf("CConsole::ExecConfig:Error opening %s\n",file);
		return;
	}

	//Configs files are linited to 256 lines, 80 chars each
	int  lines=0;
	int len = 0;
	char c = 0;

	CParms parm(80);
	char line[80];
	
	memset(line,0,80);
	
	do
	{
		len = 0;
		c = fgetc(fpcfg);

		if(c == EOF || c == '\0')
			break;

		while(c && (c != '\n') && (c != EOF) && (len < 80))
		{
			line[len++] = c;
			c = fgetc(fpcfg);
		}
		line[len] = '\0';
		
		if(!len)
			break;
		
		parm = line;

		parm.StringTok(0,line,80);
		if(strcmp(line,"bind")==0)
			BindFuncToKey(parm,false);
		
		memset(line,0,80);
		lines ++;

	}while(lines < 256);

	fclose(fpcfg);
	ComPrintf("CConsole::Exec'ed %s\n",file);
}