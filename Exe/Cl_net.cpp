#include "Sys_hdr.h"

#include "Com_vector.h"
#include "Com_world.h"
#include "Com_camera.h"

#include "Cl_base.h"
#include "Cl_hdr.h"
#include "Net_client.h"
#include "Cl_game.h"

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
				m_pClGame->PlaySnd2d(m_hsTalk, CACHE_LOCAL);
				ComPrintf("%s: %s\n", m_clients[clNum].name ,buffer.ReadString());
				break;
			}
		case SV_DISCONNECT:
			{
				m_pClGame->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
				ComPrintf("Server quit\n");
				m_pClGame->HandleNetEvent(CLIENT_SV_DISCONNECTED);
				break;
			}
		case SV_PRINT:	//just a print message
			{
				m_pClGame->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
				ComPrintf("%s\n",buffer.ReadString());
				break;
			}
		case SV_RECONNECT:
			{
				m_pClGame->HandleNetEvent(CLIENT_SV_RECONNECTING);
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
				m_clients[num].skinNum = m_pClGame->RegisterImage(path, CACHE_GAME, sindex);
				m_clients[num].skinNum |= MODEL_SKIN_UNBOUND_GAME;

				sprintf(path,"Players/%s/tris.md2", model);
				m_clients[num].mdlIndex = m_pClGame->RegisterModel(path, CACHE_GAME,mindex);
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
					m_pClGame->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
					ComPrintf("%s renamed to %s\n", m_clients[num].name, newName);
					strcpy(m_clients[num].name, newName);
				}
				break;
			}
		case SV_CLDISCONNECT:
			{
				int  num = buffer.ReadByte();
				m_pClGame->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
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

			CWorld * pWorld = m_pClGame->LoadWorld(map);
			if(pWorld)
				LoadWorld(pWorld);
			break;
		}
	case SVC_MODELLIST:
		{
			char modelName[32];
			int  modelId=0;

			int numModels = buffer.ReadShort();
			ComPrintf("CL: ModelList :%d models: %d bytes\n", numModels, buffer.GetSize());

			for(int i=0; i<numModels;i++)
			{
				modelId = buffer.ReadShort();
				buffer.ReadString(modelName,32);

				if(modelId == -1 || !modelName[0])
				{	continue;
				}
				m_pClGame->RegisterModel(modelName,CACHE_GAME,modelId);
			}
			break;
		}
	case SVC_SOUNDLIST:
		{
			char soundName[32];
			int  soundId=0;

			int numSounds = buffer.ReadShort();
			ComPrintf("CL: SoundList :%d sounds: %d bytes\n", numSounds, buffer.GetSize());

			for(int i=0; i<numSounds;i++)
			{
				soundId = buffer.ReadShort();
				buffer.ReadString(soundName,32);

				if(soundId == -1 || !soundName[0])
				{		continue;
				}
				m_pClGame->RegisterSound(soundName,CACHE_GAME, soundId);
			}
			break;
		}
	case SVC_IMAGELIST:
		{
			int numImages = buffer.ReadShort();
			ComPrintf("CL: ImageList :%d images: %d bytes\n", numImages, buffer.GetSize());
			break;
		}
	case SVC_BASELINES:
		{
			char  type = 0;
			m_numEnts = 0;

			ComPrintf("CL: Ent Baselines :%d bytes\n", buffer.GetSize());

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

//==========================================================================
//==========================================================================

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
	m_pClGame->PlaySnd2d(m_hsTalk, CACHE_LOCAL);

	//Send this reliably ?
	m_pClGame->GetReliableSendBuffer().WriteByte(CL_TALK);
	m_pClGame->GetReliableSendBuffer().WriteString(msg);
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

	m_pClGame->GetReliableSendBuffer().WriteByte(CL_INFOCHANGE);
	m_pClGame->GetReliableSendBuffer().WriteChar('n');
	m_pClGame->GetReliableSendBuffer().WriteString(name);
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
	
	if(parms.NumTokens() < 2)
	{
		ComPrintf("Rate = \"%d\"\n", m_cvRate.ival);
		return false;
	}

	int rate = parms.IntTok(1);
	if(rate < 1000 || rate > 10000)
	{
		ComPrintf("Rate needs to be in (1000-10000): %d\n", rate);
		return false;
	}

	m_pClGame->SetNetworkRate(rate);

	if(!m_ingame)
		return true;

	CBuffer &buffer = m_pClGame->GetReliableSendBuffer();
	buffer.WriteByte(CL_INFOCHANGE);
	buffer.WriteChar('r');
	buffer.WriteInt(rate);
	return true;
}



//==========================================================================
//==========================================================================

/*
================================================
Enter the game for the first time
================================================
*/
void CGameClient::BeginGame(int clNum, CBuffer &buffer)
{
	//Initialize local Client
	m_pGameClient = &m_clients[clNum];
	m_pGameClient->Reset();
	strcpy(m_pGameClient->name, m_cvName.string);
	m_pGameClient->inUse = true;

	HandleGameMsg(buffer);

	m_campath = -1;
	m_maxvelocity =  200.0f;
	
	m_pGameClient->moveType = MOVETYPE_STEP;

	VectorSet(&m_pGameClient->angles, 0.0f,0.0f,0.0f);
	VectorSet(&m_pGameClient->origin, 0.0f,0.0f,48.0f);
	VectorSet(&m_pGameClient->mins, -10.0f, -10.0f, -40.0f);
	VectorSet(&m_pGameClient->maxs, 10.0f, 10.0f, 10.0f);
	
	VectorSet(&m_vecBlend,0.0f,0.0f,0.0f);
	VectorSet(&m_vecDesiredMove, 0, 0, 0);

	//Register static sound sources with SoundManager
	for(int i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse && m_entities[i].sndIndex > -1)
		{
			m_entities[i].sndCache = CACHE_GAME;
			m_entities[i].volume = 10;
			m_entities[i].attenuation = 5;
			m_pClGame->AddSoundSource(&m_entities[i]);
		}
	}
	
	
	m_pCamera = new CCamera(m_pGameClient->origin, m_pGameClient->angles, m_vecBlend,
							m_vecForward, m_vecRight, m_vecUp,	m_vecVelocity);

	m_ingame = true;
	m_pClGame->HandleNetEvent(CLIENT_BEGINGAME);
	Spawn(0,0);
}


/*
======================================
Handle disconnect from server
======================================
*/
void CGameClient::HandleDisconnect()
{
	UnloadWorld();
	m_pClGame->UnloadWorld();
}
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
