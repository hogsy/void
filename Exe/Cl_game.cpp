#include "Sys_hdr.h"

#include "Com_util.h"
#include "Com_world.h"
#include "Com_camera.h"

#include "I_clientRenderer.h"
#include "I_renderer.h"
#include "I_hud.h"
#include "Snd_main.h"
#include "Mus_main.h"

#include "Net_defs.h"
#include "Net_protocol.h"

#include "Cl_main.h"
#include "Cl_cmds.h"

#include "Cl_game.h"



CGameClient::	CGameClient(CClient & rClient, I_ClientRenderer	   * pRenderer, I_HudRenderer * pHud,
				 CSoundManager * pSound,
				CMusic		   * pMusic) : 
			m_refClient(rClient), m_pRenderer(pRenderer), m_pHud(pHud), m_pSound(pSound), m_pMusic(pMusic),
			m_cvKbSpeed("cl_kbspeed","0.6", CVAR_FLOAT, CVAR_ARCHIVE),
							m_cvClip("cl_clip","1",     CVAR_BOOL,0),
					m_cvRate("cl_rate","2500",	CVAR_INT,	CVAR_ARCHIVE),
					m_cvName("cl_name","Player",CVAR_STRING,CVAR_ARCHIVE),
					m_cvModel("cl_model", "Ratamahatta", CVAR_STRING, CVAR_ARCHIVE),
					m_cvSkin("cl_skin", "Ratamahatta", CVAR_STRING, CVAR_ARCHIVE),
					m_cvNetStats("cl_netstats","1", CVAR_BOOL, CVAR_ARCHIVE)
{

	m_pCmdHandler = new CClientGameInput();

	m_ingame = false;

	m_pGameClient = 0;

	m_numEnts = 0;
	

	m_hsTalk = 0;
	m_hsMessage = 0;

	m_pCamera = 0;
	
	m_pWorld = 0;

	System::GetConsole()->RegisterCVar(&m_cvKbSpeed,this);
	System::GetConsole()->RegisterCVar(&m_cvClip);
	System::GetConsole()->RegisterCVar(&m_cvNetStats);
		
	System::GetConsole()->RegisterCVar(&m_cvRate,this);
	System::GetConsole()->RegisterCVar(&m_cvName,this);
	System::GetConsole()->RegisterCVar(&m_cvModel,this);
	System::GetConsole()->RegisterCVar(&m_cvSkin,this);

	

	System::GetConsole()->RegisterCommand("+forward",CMD_MOVE_FORWARD,this);
	System::GetConsole()->RegisterCommand("+back",CMD_MOVE_BACKWARD,this);
	System::GetConsole()->RegisterCommand("+moveleft",CMD_MOVE_LEFT,this);
	System::GetConsole()->RegisterCommand("+moveright",CMD_MOVE_RIGHT,this);
	System::GetConsole()->RegisterCommand("+right",CMD_ROTATE_RIGHT,this);
	System::GetConsole()->RegisterCommand("+left",CMD_ROTATE_LEFT,this);
	System::GetConsole()->RegisterCommand("+lookup",CMD_ROTATE_UP,this);
	System::GetConsole()->RegisterCommand("+lookdown",CMD_ROTATE_DOWN,this);

	System::GetConsole()->RegisterCommand("say", CMD_TALK, this);
	System::GetConsole()->RegisterCommand("bind",CMD_BIND,this);
	System::GetConsole()->RegisterCommand("bindlist",CMD_BINDLIST,this);
	System::GetConsole()->RegisterCommand("cam",CMD_CAM,this);
	System::GetConsole()->RegisterCommand("unbind",CMD_UNBIND,this);
	System::GetConsole()->RegisterCommand("unbindall",CMD_UNBINDALL,this);

	m_pCmdHandler->IntializeBinds();

}

CGameClient::~CGameClient()
{
	m_pCmdHandler->WriteBinds("vbinds.cfg");

	if(m_pCamera)
		delete m_pCamera;
	m_pWorld = 0;

	m_pHud=0;
	m_pSound=0;
	m_pMusic=0;
	m_pRenderer = 0;

	delete m_pCmdHandler;

	m_pGameClient = 0;
}


void CGameClient::Spawn(vector_t	*origin, vector_t *angles)
{
}

void CGameClient::RunFrame(float frameTime)
{
	if(m_pCmdHandler->CursorChanged())
	{
		m_pCmdHandler->UpdateCursorPos(m_moveAngles.x, m_moveAngles.y, m_moveAngles.z);
		RotateRight(m_moveAngles.x);
		RotateUp(m_moveAngles.y);
	}

	m_pCmdHandler->RunCommands();

	VectorNormalize(&desired_movement);
	Move(desired_movement, frameTime * m_maxvelocity);
	desired_movement.Set(0,0,0);

	m_pHud->Printf(0, 50,0, "%.2f, %.2f, %.2f", 
				m_pGameClient->origin.x,  m_pGameClient->origin.y, m_pGameClient->origin.z);
}


/*
======================================

======================================
*/
void CGameClient::WriteCmdUpdate(CBuffer &buf)
{
	buf.WriteShort(m_cmd.forwardmove);
	buf.WriteShort(m_cmd.rightmove);
	buf.WriteShort(m_cmd.upmove);

	buf.WriteInt(m_cmd.angles[0]);
	buf.WriteInt(m_cmd.angles[1]);
	buf.WriteInt(m_cmd.angles[2]);
}


/*
======================================

======================================
*/
void CGameClient::UpdateView()
{
	// FIXME - put this in game dll
	int contents = m_pWorld->PointContents(m_pGameClient->origin);
	if(contents & CONTENTS_SOLID)
		VectorSet(&m_screenBlend, 0.4f, 0.4f, 0.4f);
	else if(contents & CONTENTS_WATER)
		VectorSet(&m_screenBlend, 0, 1, 1);
	else if(contents & CONTENTS_LAVA)
		VectorSet(&m_screenBlend, 1, 0, 0);
	else
		VectorSet(&m_screenBlend, 1, 1, 1);
}


//spawn for the first time.
void CGameClient::BeginGame()
{
	m_campath = -1;
	m_maxvelocity =  200.0f;
	
	m_pGameClient->moveType = MOVETYPE_STEP;
	VectorSet(&m_pGameClient->angles, 0.0f,0.0f,0.0f);
	VectorSet(&m_pGameClient->origin, 0.0f,0.0f,48.0f);
	VectorSet(&m_pGameClient->mins, -10.0f, -10.0f, -40.0f);
	VectorSet(&m_pGameClient->maxs, 10.0f, 10.0f, 10.0f);
	VectorSet(&m_screenBlend,0.0f,0.0f,0.0f);

	VectorSet(&desired_movement, 0, 0, 0);

	//Register static sound sources with SoundManager
	for(int i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse && m_entities[i].sndIndex > -1)
		{
			m_entities[i].sndCache = CACHE_GAME;
			m_entities[i].volume = 10;
			m_entities[i].attenuation = 5;
			m_pSound->AddStaticSource(&m_entities[i]);
		}
	}
	
	
	m_pCamera = new CCamera(m_pGameClient->origin, m_pGameClient->angles, m_screenBlend);

	m_ingame = true;

	Spawn(0,0);
}


bool CGameClient::LoadWorld(CWorld * pWorld)
{
	//setup
	m_pWorld = pWorld;
	EntMove::SetWorld(m_pWorld);

	m_hsTalk    = m_pSound->RegisterSound("sounds/talk.wav", CACHE_LOCAL);
	m_hsMessage = m_pSound->RegisterSound("sounds/message.wav", CACHE_LOCAL);

	ComPrintf("CGameClient::Load World: OK\n");
	return true;

}

void CGameClient::UnloadWorld()
{
	if(!m_ingame)
		return;

	EntMove::SetWorld(0);

	delete m_pCamera;
	m_pCamera = 0;

	m_pGameClient = 0;

	int i;
	for(i=0; i< GAME_MAXCLIENTS; i++)
		if(m_clients[i].inUse) 
			m_clients[i].Reset();

	for(i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse)
		{
			if(m_entities[i].sndIndex > -1)
				m_pSound->RemoveStaticSource(&m_entities[i]);
			m_entities[i].Reset();
		}
	}

	m_pWorld = 0;

	m_ingame = false;
}




/*
==========================================
Handle Registered Commands
==========================================
*/

void CGameClient::HandleCommand(HCMD cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_MOVE_FORWARD:
		MoveForward();
		break;
	case CMD_MOVE_BACKWARD:
		MoveBackward();
		break;
	case CMD_MOVE_LEFT:
		MoveLeft();
		break;
	case CMD_MOVE_RIGHT:
		MoveRight();
		break;
	case CMD_ROTATE_LEFT:
		RotateLeft(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_RIGHT:
		RotateRight(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_UP:
		RotateUp(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_DOWN:
		RotateDown(m_cvKbSpeed.fval);
		break;
	case CMD_BIND:
		m_pCmdHandler->BindFuncToKey(parms);
		break;
	case CMD_BINDLIST:
		m_pCmdHandler->BindList();
		break;
	case CMD_UNBIND:
		m_pCmdHandler->Unbind(parms);
		break;
	case CMD_UNBINDALL:
		m_pCmdHandler->Unbindall();
		break;
	case CMD_TALK:
		Talk(parms.String());
		break;
	}
}

/*
==========================================
Validate/Handle any CVAR changes
==========================================
*/
bool CGameClient::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == reinterpret_cast<CVarBase *>(&m_cvKbSpeed))
	{
		float val = parms.FloatTok(1);
		if(val <= 0.0 || val >= 1.0)
		{
			ComPrintf("Out of range. Should be between 0.0 and 1.0\n");
			return false;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvRate))
		return ValidateRate(parms);
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvName))
		return ValidateName(parms);
	return false;
}




/*
======================================
Say something
======================================
*/
void CGameClient::Talk(const char * string)
{
	if(!m_ingame)
		return;

	//parse to right after "say"
	const char * msg = string + 4;
	while(*msg && *msg == ' ')
		msg++;

	if(!*msg || *msg == '\0')
		return;

	ComPrintf("%s: %s\n", m_cvName.string, msg);
	m_pSound->PlaySnd2d(m_hsTalk, CACHE_LOCAL);

	//Send this reliably ?
	m_refClient.GetReliableSendBuffer().WriteByte(CL_TALK);
	m_refClient.GetReliableSendBuffer().WriteString(msg);
}

/*
======================================
Validate name locally before asking 
the server to update it
======================================
*/
bool CGameClient::ValidateName(const CParms &parms)
{
	char name[24];
	parms.StringTok(1,name,24);

	if(!name[0])
	{
		ComPrintf("Name = \"%s\"\n", m_cvName.string);
		return false;
	}
	if(!m_ingame)
		return true;

	m_refClient.GetReliableSendBuffer().WriteByte(CL_INFOCHANGE);
	m_refClient.GetReliableSendBuffer().WriteChar('n');
	m_refClient.GetReliableSendBuffer().WriteString(name);
	return true;
}

/*
======================================
Validate Rate before updating it 
on the server
======================================
*/
bool CGameClient::ValidateRate(const CParms &parms)
{
	int rate = parms.IntTok(1);
	if(rate == -1)
	{
		ComPrintf("Rate = \"%d\"\n", m_cvRate.ival);
		return false;
	}

	if(rate < 1000 || rate > 10000)
	{
		ComPrintf("Rate is out of range\n");
		return false;
	}

	m_refClient.SetNetworkRate(rate);

	if(!m_ingame)
		return true;

	CBuffer &buffer = m_refClient.GetReliableSendBuffer();
	buffer.WriteByte(CL_INFOCHANGE);
	buffer.WriteChar('r');
	buffer.WriteInt(rate);
	return true;
}












/*
======================================
Process game message
======================================
*/
void CGameClient::HandleGameMsg(CBuffer &buffer)
{
	byte msgId = 0;
	while(msgId != 255)
	{
		msgId= buffer.ReadByte();
		//bad message
		if(msgId == 255)
		{	break;
		}

		switch(msgId)
		{
		case SV_TALK:
			{
				int clNum = buffer.ReadByte();
				m_pSound->PlaySnd2d(m_hsTalk, CACHE_LOCAL);
				ComPrintf("%s: %s\n", m_clients[clNum].name ,buffer.ReadString());
				break;
			}
		case SV_DISCONNECT:
			{
				m_pSound->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
				ComPrintf("Server quit\n");
				//m_pNetCl->Disconnect(true);
				m_refClient.SetState(CClient::CL_DISCONNECTED);
				break;
			}
		case SV_PRINT:	//just a print message
			{
				m_pSound->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
				ComPrintf("%s\n",buffer.ReadString());
				break;
			}
		case SV_RECONNECT:
			{
				//m_pNetCl->Reconnect(true);
				m_refClient.SetState(CClient::CL_RECONNECTING);
				break;
			}
		case SV_CLFULLINFO:
			{
				int num = buffer.ReadByte();
				m_clients[num].Reset();
				strcpy(m_clients[num].name, buffer.ReadString());

				int mindex = buffer.ReadShort();
				char model[64];
				strcpy(model,buffer.ReadString());

				int sindex = buffer.ReadShort();
				char path[COM_MAXPATH];

				sprintf(path,"Players/%s/%s", model, buffer.ReadString());

				m_clients[num].mdlCache = CACHE_GAME;
				m_clients[num].skinNum = m_pRenderer->LoadImage(path, CACHE_GAME, sindex);
				m_pRenderer->LoadImage(path, CACHE_GAME, sindex);
				m_clients[num].skinNum |= MODEL_SKIN_UNBOUND_GAME;

				sprintf(path,"Players/%s/tris.md2", model);
				m_clients[num].mdlIndex = m_pRenderer->LoadModel(path, CACHE_GAME,mindex);
				m_clients[num].mdlCache = CACHE_GAME;

				m_clients[num].inUse = true;

				break;
			}
		case SV_CLINFOCHANGE:
			{
				int num = buffer.ReadByte();
				char field = buffer.ReadChar();
				if(field == 'n')
				{
					char * newName = buffer.ReadString();
					m_pSound->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
					ComPrintf("%s renamed to %s\n", m_clients[num].name, newName);
					strcpy(m_clients[num].name, newName);
				}
				break;
			}
		case SV_CLDISCONNECT:
			{
				int  num = buffer.ReadByte();
				m_pSound->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
				ComPrintf("%s %s\n", m_clients[num].name, buffer.ReadString());
				m_clients[num].Reset();
				break;
			}
		case SV_CLUPDATE:
			{
				int num = buffer.ReadShort();
				if(m_clients[num].inUse)
				{
					m_clients[num].origin.x = buffer.ReadCoord();
					m_clients[num].origin.y = buffer.ReadCoord();
					m_clients[num].origin.z = buffer.ReadCoord();
					m_clients[num].angles.x = buffer.ReadAngle();
					m_clients[num].angles.y = buffer.ReadAngle();
					m_clients[num].angles.z = buffer.ReadAngle();
				}
				break;
			}
		default:
			{
				buffer.Reset();
				break;
			}
		}
	}
}

/*
======================================
Process Spawn message
======================================
*/
void CGameClient::HandleSpawnMsg(byte msgId, CBuffer &buffer)
{
	switch(msgId)
	{
	case SVC_GAMEINFO:
		{

			char * game = buffer.ReadString();
ComPrintf("CL: Game: %s\n", game);
			char * map = buffer.ReadString();
ComPrintf("CL: Map: %s\n", map);

			m_refClient.LoadWorld(map);

//			if(!LoadWorld(map))
//				m_pNetCl->Disconnect(false);

			break;
		}
	case SVC_MODELLIST:
		{
			char modelName[32];
			int  modelId=0;

			int numModels = buffer.ReadShort();
			ComPrintf("CL: ModelList :%d models\n", numModels);

			for(int i=0; i<numModels;i++)
			{
				modelId = buffer.ReadShort();
				buffer.ReadString(modelName,32);

				if(modelId == -1 || !modelName[0])
				{	continue;
				}
				m_pRenderer->LoadModel(modelName,CACHE_GAME,modelId);
			}
			break;
		}
	case SVC_SOUNDLIST:
		{
			char soundName[32];
			int  soundId=0;

			int numSounds = buffer.ReadShort();
			ComPrintf("CL: SoundList :%d models\n", numSounds);

			for(int i=0; i<numSounds;i++)
			{
				soundId = buffer.ReadShort();
				buffer.ReadString(soundName,32);

				if(soundId == -1 || !soundName[0])
				{		continue;
				}
				m_pSound->RegisterSound(soundName,CACHE_GAME, soundId);
			}
//			ComPrintf("CL: SoundList :%d\n", buffer.GetSize());
			break;
		}
	case SVC_IMAGELIST:
		{
			ComPrintf("CL: ImageList :%d\n", buffer.GetSize());
			break;
		}
	case SVC_BASELINES:
		{
			char  type = 0;
			m_numEnts = 0;
			int id = buffer.ReadShort();

			while(id != -1)
			{
				m_entities[id].Reset();

				m_entities[id].moveType = (EMoveType)buffer.ReadByte();

				m_entities[id].origin.x = buffer.ReadCoord();
				m_entities[id].origin.y = buffer.ReadCoord();
				m_entities[id].origin.z = buffer.ReadCoord();

				m_entities[id].angles.x = buffer.ReadAngle();
				m_entities[id].angles.y = buffer.ReadAngle();
				m_entities[id].angles.z = buffer.ReadAngle();

				type = buffer.ReadChar();
				while(type != 0)
				{
					switch(type)
					{
					case 'm':
						{
							m_entities[id].mdlIndex = buffer.ReadShort();
							m_entities[id].skinNum = buffer.ReadShort();
							m_entities[id].frameNum = buffer.ReadShort();
							m_entities[id].nextFrame = m_entities[id].frameNum;
							m_entities[id].frac = 0;
							m_entities[id].mdlCache = CACHE_GAME;
							break;
						}
					case 's':
						{
							m_entities[id].sndCache = CACHE_GAME;
							m_entities[id].sndIndex = buffer.ReadShort();
							m_entities[id].volume = buffer.ReadShort();
							m_entities[id].attenuation = buffer.ReadShort();
							break;
						}
					}
					type = buffer.ReadChar();
				}
					
				if(buffer.BadRead())
				{
					ComPrintf("Error reading Ent %d\n", id);
					m_entities[id].Reset();
					break;
				}
				m_entities[id].inUse = true;
				m_numEnts ++;
				id = buffer.ReadShort();
			}
			ComPrintf("CL: Parsed %d entities\n", m_numEnts);
			break;
		}
	case SVC_CLIENTINFO:
		{
			HandleGameMsg(buffer);
			break;
		}
	}
}







void CGameClient::BeginGame(int clNum, CBuffer &buffer)
{
	//Initialize local Client
	m_pGameClient = &m_clients[clNum];
	m_pGameClient->Reset();
//	strcpy(m_pGameClient->name, m_cvName.string);
	m_pGameClient->inUse = true;

	HandleGameMsg(buffer);
	BeginGame();

	m_refClient.SetState(CClient::CL_INGAME);
}





/*
======================================
Write UserInfo to buffer
======================================
*/
void CGameClient::WriteUserInfo(CBuffer &buffer)
{
	buffer.WriteString(m_cvName.string);
	buffer.WriteString(m_cvModel.string);
	buffer.WriteString(m_cvSkin.string);
	buffer.WriteInt(m_cvRate.ival);
}






/*
======================================
Handle disconnect from server
======================================
*/
void CGameClient::HandleDisconnect(bool listenserver)
{
//	m_refClient.HandleDisconnect(listenserver);
	
//HACK	
	if(listenserver)
	{
//		ComPrintf("CL: KILLING LOCAL SERVER\n");
		System::GetConsole()->ExecString("killserver");
	}
	m_refClient.UnloadWorld();

	/*
//	ComPrintf("CL: KILLING LOCAL SERVER\n");

	//Kill server if local
	if(listenserver)
	{
//		ComPrintf("CL: KILLING LOCAL SERVER\n");
		System::GetConsole()->ExecString("killserver");
	}
	UnloadWorld();
*/
}

/*
======================================
Print a message 
======================================
*/
void CGameClient::Print(const char * msg, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, msg);
	vsprintf(textBuffer, msg, args);
	va_end(args);

	System::GetConsole()->ComPrint(textBuffer);
}














