/*
======================================
Client Command Handling

-maintain a list of all the valid commands
-maintain a command buffer that gets executed every frame
-provide means to register new commands
-provide means to bind keys to commands
======================================
*/
#include "Cl_main.h"
#include "Cl_cmds.h"
#include "Sys_cons.h"


//============================================================================
//============================================================================

/*
======================================
Add a command to the buffer
======================================
*/
static void AddToCommandBuffer(ClientKey * const keycommand)
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_commandbuffer[i] == keycommand)
			return;

		if(m_commandbuffer[i] == 0)
		{
			m_commandbuffer[i] = keycommand;
			return;
		}
	}
}

/*
======================================
Remove command from the buffer
======================================
*/
static void RemoveFromCommandBuffer(const ClientKey * keycommand)
{
	for(int i=0;i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_commandbuffer[i] == keycommand)
			m_commandbuffer[i] = 0;
	}
}



void CClient::HandleKeyEvent	(const KeyEvent_t &kevent)
{
	//check if there is a command bound to that key
	if(m_clientkeys[(kevent.id)].szCommand)
	{
		//if the command is supposed to be executed everyframe
		//until its is released

		if(m_clientkeys[(kevent.id)].szCommand[0] == '+')
		{
			//if its a keydown event
			if(kevent.state == BUTTONDOWN)
			{
				//add to command buffer
				AddToCommandBuffer(&m_clientkeys[(kevent.id)]);
			}
			else if(kevent.state == BUTTONUP)
			{
				//otherwise remove from buffer
				RemoveFromCommandBuffer(&m_clientkeys[(kevent.id)]);
			}
		}
		//if its regular function and if its a keydown event,
		else if(kevent.state == BUTTONDOWN)
		{
			//Send over to the console for execution
			g_pConsole->ExecString(m_clientkeys[(kevent.id)].szCommand);
		}
	
/*		if(m_clientkeys[(kevent.id)].bBuffered)
		{
			//if its a keydown event
			if(kevent.state == BUTTONDOWN)
			{
				//add to command buffer
				AddToCommandBuffer(&m_clientkeys[(kevent.id)]);
			}
			else if(kevent.state == BUTTONUP)
			{
				//otherwise remove from buffer
				RemoveFromCommandBuffer(&m_clientkeys[(kevent.id)]);
			}
		}
		//if its regular function and if its a keydown event,
		else 
		if(kevent.state == BUTTONDOWN)
		{
			//Send over to the console for execution
			g_pConsole->ExecString(m_clientkeys[(kevent.id)].szCommand);
		}
*/
	}
}


void CClient::HandleCursorEvent(const float &ix,
				   const float &iy,
				   const float &iz)
{
	RotateRight(ix);
	RotateUp(iy);
}


//============================================================================

/*
======================================
Handle mouse input
called whenever the mouse moves if event based
or every frame if otherwise
======================================
*/
void CClient::SetInputState(bool on)
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

/*
======================================
Runs all the commands in the 
command buffer for the frame

Buffered commands cannot take arguments
======================================
*/
void CClient::RunCommands()
{
	for(int i=0; i<CL_CMDBUFFERSIZE;i++)
	{
		if(m_commandbuffer[i]) 
			m_commandbuffer[i]->pCmd->handler->HandleCommand(m_commandbuffer[i]->pCmd->id, 1, 0);
	}
}



/*
======================================
Console command 
Bind another command to a key
First validate the key,
Then validate the command being bound
Then bind the func to the key with flags
======================================
*/
void CClient::BindFuncToKey(int argc, char** argv)
{
	//no arguments
	if(argc==1)
	{
		ComPrintf("bind usage - <bind> <command> ...\n");
		return;
	}

	unsigned char keynum= 0;

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
			ComPrintf("bind - error %s is not a valid key -%d\n",argv[1],keynum);
			return;
		}
		keynum = argv[1][0];
	}
		
	//Only two args, just show binding for the key and return
	if(argc == 2)
	{
		if(m_clientkeys[keynum].szCommand && m_clientkeys[keynum].pCmd)
			ComPrintf("\"%s\" = \"%s\"\n", argv[1], m_clientkeys[keynum].szCommand);
		else
			ComPrintf("\"%s\" = \"\"\n", argv[1]);
		return;
	}

	CCommand * pCmd = g_pConsole->GetCommandByName(argv[2]);
	if(!pCmd)
	{
		ComPrintf("%s is not a valid command\n",argv[2]);
		return;
	}
	m_clientkeys[keynum].pCmd = pCmd;

/*	if(argc == 2)
	{
		if(m_clientkeys[keynum].szCommand && m_clientkeys[keynum].pFunc)
			ComPrintf("\"%s\" = \"%s\"\n",argv[1],m_clientkeys[keynum].szCommand);
		else
			ComPrintf("\"%s\" = \"\"\n",argv[1]);
		return;
	}

	//Get the requested function, and bind it to the key
	m_clientkeys[keynum].pFunc = g_pConsole->GetFuncByName(argv[2]);
	if(!m_clientkeys[keynum].pFunc)
	{
		ComPrintf("%s is not a valid command\n",argv[2]);
		return;
	}
*/	
	//If there are more arguments then just the functions name
	//then they will be used as function paramters
	
	//Get their length and allocate space
	int parmlen = 1;

	for(int i=2;i<argc;i++)
		parmlen += strlen(argv[i]);

	if(m_clientkeys[keynum].szCommand)
		delete [] m_clientkeys[keynum].szCommand;
	m_clientkeys[keynum].szCommand = new char[parmlen];

	//Copy the parms into ONE string
	char *p= m_clientkeys[keynum].szCommand;
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

	//Set buffered flag if the command starts with a +/-
/*	
	if((m_clientkeys[keynum].szCommand[0] == '+') || 
	   (m_clientkeys[keynum].szCommand[0] == '-'))
		m_clientkeys[keynum].bBuffered = true;
*/
	ComPrintf("\"%s\"(%d) = \"%s\"\n",argv[1],keynum, m_clientkeys[keynum].szCommand);
}	

/*
======================================
lists all our current bindings
======================================
*/
void CClient::BindList(int argc, char** argv)
{
	ComPrintf(" Client Bindings \n");
	ComPrintf("=================\n");
	
	for(unsigned int i=0;i<256;i++)
	{
//		if(m_clientkeys[i].pFunc && m_clientkeys[i].szCommand)
		if(m_clientkeys[i].szCommand && m_clientkeys[i].pCmd)
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
			ComPrintf("\"%s\" = \"%s\"\n",keyname, m_clientkeys[i].szCommand);
		}
	}
}

/*
======================================
unbind all our bindings
======================================
*/
void CClient::Unbindall(int argc, char** argv)
{
	for(int i=0;i<256;i++)
	{
//		if(m_clientkeys[i].pFunc && m_clientkeys[i].szCommand)
		if(m_clientkeys[i].szCommand && m_clientkeys[i].pCmd)
		{
			delete [] m_clientkeys[i].szCommand;
			m_clientkeys[i].szCommand=0;
			m_clientkeys[i].pCmd = 0;
//			m_clientkeys[i].pFunc = 0;
		}
	}
	ComPrintf("CClient::Unbindall OK\n");

}

/*
======================================
unbind all our bindings
======================================
*/

void CClient::Unbind(int argc, char** argv)
{
	if(argc <2)
	{
		ComPrintf("Usage - <unbind> <keyname>\n");
		return;
	}

	unsigned char keynum= 0;

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
			ComPrintf("bind - error %s is not a valid key -%d\n",argv[1],keynum);
			return;
		}
	}
		
//	if(m_clientkeys[keynum].pFunc && m_clientkeys[keynum].szCommand)
	if(m_clientkeys[keynum].szCommand && m_clientkeys[keynum].pCmd)
	{
		m_clientkeys[keynum].pCmd = 0;
		delete [] m_clientkeys[keynum].szCommand;
		m_clientkeys[keynum].szCommand=0;
	}
	ComPrintf("\"%s\" = \"\"\n",argv[1]);
}



/*
======================================
Register the EXE Client Commands
======================================
*/

#define CMD_MOVE_FORWARD	0		
#define CMD_MOVE_BACKWARD	1
#define CMD_MOVE_LEFT		2
#define CMD_MOVE_RIGHT		3
#define CMD_ROTATE_LEFT		4
#define CMD_ROTATE_RIGHT	5
#define CMD_ROTATE_UP		6
#define CMD_ROTATE_DOWN		7
#define CMD_BIND			8
#define CMD_BINDLIST		9
#define CMD_UNBIND			10
#define CMD_UNBINDALL		11
#define CMD_CAM				12

void CClient::RegCommands()
{
	Sys_GetConsole()->RegisterCommand("+forward",CMD_MOVE_FORWARD,this);
	Sys_GetConsole()->RegisterCommand("+back",CMD_MOVE_BACKWARD,this);
	Sys_GetConsole()->RegisterCommand("+moveleft",CMD_MOVE_LEFT,this);
	Sys_GetConsole()->RegisterCommand("+moveright",CMD_MOVE_RIGHT,this);
	Sys_GetConsole()->RegisterCommand("+right",CMD_ROTATE_RIGHT,this);
	Sys_GetConsole()->RegisterCommand("+left",CMD_ROTATE_LEFT,this);
	Sys_GetConsole()->RegisterCommand("+lookup",CMD_ROTATE_UP,this);
	Sys_GetConsole()->RegisterCommand("+lookdown",CMD_ROTATE_DOWN,this);
	Sys_GetConsole()->RegisterCommand("bind",CMD_BIND,this);
	Sys_GetConsole()->RegisterCommand("bindlist",CMD_BINDLIST,this);
	Sys_GetConsole()->RegisterCommand("cam",CMD_CAM,this);
	Sys_GetConsole()->RegisterCommand("unbind",CMD_UNBIND,this);
	Sys_GetConsole()->RegisterCommand("unbindall",CMD_UNBINDALL,this);
}

void CClient::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case CMD_MOVE_FORWARD:
		MoveForward(numArgs,szArgs);
		break;
	case CMD_MOVE_BACKWARD:
		MoveBackward(numArgs,szArgs);
		break;
	case CMD_MOVE_LEFT:
		MoveLeft(numArgs,szArgs);
		break;
	case CMD_MOVE_RIGHT:
		MoveRight(numArgs,szArgs);
		break;
	case CMD_ROTATE_LEFT:
		KRotateLeft(numArgs,szArgs);
		break;
	case CMD_ROTATE_RIGHT:
		KRotateRight(numArgs,szArgs);
		break;
	case CMD_ROTATE_UP:
		KRotateUp(numArgs,szArgs);
		break;
	case CMD_ROTATE_DOWN:
		KRotateDown(numArgs,szArgs);
		break;
	case CMD_BIND:
		BindFuncToKey(numArgs,szArgs);
		break;
	case CMD_BINDLIST:
		BindList(numArgs,szArgs);
		break;
	case CMD_UNBIND:
		Unbind(numArgs,szArgs);
		break;
	case CMD_UNBINDALL:
		Unbindall(numArgs,szArgs);
		break;
	case CMD_CAM:
		CamPath(numArgs,szArgs);
		break;
	}
}

/*
======================================
Writes the current Bind table to a config file
called from the Console Shutdown func
======================================
*/

void CClient::WriteBindTable(FILE *fp)
{
	for(unsigned int i=0;i<256;i++)
	{
		//there is a binding for this key
//		if(m_clientkeys[i].pFunc && m_clientkeys[i].szCommand)
		if(m_clientkeys[i].szCommand && m_clientkeys[i].pCmd)
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
			strcat(line, m_clientkeys[i].szCommand);
			strcat(line,"\n");
			fputs(line,fp);
		}
	}
}


/*
===========
follow a camera path
===========
*/
extern world_t		 *g_pWorld;
void CClient::CamPath(int argc,char **argv)
{
	// find the head path node
	for (int ent=0; ent<g_pWorld->nentities; ent++)
	{
		if (strcmp(key_get_value(g_pWorld, ent, "classname"), "misc_camera_path_head") == 0)
		{
			g_pClient->m_campath = ent;
			g_pClient->m_camtime = g_fcurTime;

			vector_t origin;
			key_get_vector(g_pWorld, ent, "origin", origin);
			VectorCopy(origin, g_pClient->eye.origin); // move to first point of path

			return;
		}
	}
}


