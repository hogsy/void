#include "Sys_hdr.h"

#include "Com_vector.h"
#include "Com_world.h"
#include "Com_camera.h"

#include "Cl_base.h"
#include "Cl_hdr.h"
#include "Net_client.h"
#include "Cl_game.h"
#include "I_file.h"

#include "Game_anims.h"

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
				m_pClGame->ForwardNetworkEvent(CLIENT_SV_DISCONNECTED);
				break;
			}
		case SV_RECONNECT:
			{
				m_pClGame->ForwardNetworkEvent(CLIENT_SV_RECONNECTING);
				break;
			}
		case SV_PRINT:	//just a print message
			{
				m_pClGame->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
				ComPrintf("%s\n",buffer.ReadString());
				break;
			}
		case SV_CLFULLINFO:
			{
				ReadClientInfo(buffer);
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
				
				//Unregister model/skin
				//m_pClGame->UnregisterModel(CACHE_LOCAL, m_clients[num].mdlIndex);
				
				m_clients[num].Reset();
				break;
			}

		case SV_UPDATE:
			{
				byte  b = buffer.ReadByte();
				float x = buffer.ReadFloat();
				float y = buffer.ReadFloat();
				float z = buffer.ReadFloat();

				if(buffer.BadRead())
				{
ComPrintf("CL: Update is corrupt. Ignoring\n");
					return;
				}

				if(b)
				{
					//Order matters
					if(b & SVU_GRAVITY)
					{
						m_pGameClient->gravity = buffer.ReadFloat();
ComPrintf("CL: Grav changed to %f\n", m_pGameClient->gravity);
					}
					if(b & SVU_FRICTION)
						m_pGameClient->friction = buffer.ReadFloat();
					if(b & SVU_MAXSPEED)
						m_pGameClient->maxSpeed = buffer.ReadFloat();
				}

/*				//Ignoring position until movement is redesigned.
				if(!m_cvLocalMove.bval)
				{
					vector_t pos(x,y,z);
					vector_t oldvelocity = m_pGameClient->velocity;

					m_pGameClient->velocity += (pos - m_pGameClient->origin);
					EntMove::ClientMove(m_pGameClient, GAME_FRAMETIME);
					m_pGameClient->velocity = oldvelocity;
				}
*/
				break;
			}

		case SV_CLUPDATE:
			{
				int num = buffer.ReadByte();
				if(m_clients[num].inUse && (num != m_clNum))
				{
					m_clients[num].origin.x = buffer.ReadCoord();
					m_clients[num].origin.y = buffer.ReadCoord();
					m_clients[num].origin.z = buffer.ReadCoord();

					m_clients[num].angles.x = buffer.ReadCoord();
					m_clients[num].angles.y = buffer.ReadCoord();
					m_clients[num].angles.z = buffer.ReadCoord();

					int anim = buffer.ReadByte();
					if(m_clients[num].clAnim != anim)
					{
						m_clients[num].clAnim = anim;
						ClAnim::SetAnim(m_clients[num].animInfo,(EPlayerAnim)anim);
						if(anim == PLAYER_STAND)
							ComPrintf("Client Stand");
						else if(anim == PLAYER_RUN)
							ComPrintf("Client Run");
						else if(anim == PLAYER_JUMP)
							ComPrintf("Client Jump");

						ComPrintf(": Frames : %d to %d\n", m_clients[num].animInfo.frameBegin, 
							m_clients[num].animInfo.frameEnd);
					}
m_pClGame->HudPrintf(0,200,0,"OTHER ORIGIN: %s", m_clients[num].origin.ToString());
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

			//All this should be moved into a special worldspawn msg
			
			char * map = buffer.ReadString();
			ComPrintf("CL: Map: %s\n", map);

			CWorld * pWorld = m_pClGame->LoadWorld(map);
			if(pWorld)
				LoadWorld(pWorld);

			char music[64];
			strcpy(music,"Zurich_Switzerland.mp3");
			m_pClGame->PlayMusicTrack(music);

			break;
		}
	case SVC_MODELLIST:
		{
			char modelName[128];
			int  modelId=0;

			int numModels = buffer.ReadShort();
			ComPrintf("CL: ModelList :%d models: %d bytes\n", numModels, buffer.GetSize());

			for(int i=0; i<numModels;i++)
			{
				modelId = buffer.ReadShort();
				buffer.ReadString(modelName,128);

				if(modelId == -1 || !modelName[0])
					continue;
				m_pClGame->RegisterModel(modelName,CACHE_GAME,modelId);
			}
			break;
		}
	case SVC_SOUNDLIST:
		{
			char soundName[128];
			int  soundId=0;

			int numSounds = buffer.ReadShort();
			ComPrintf("CL: SoundList :%d sounds: %d bytes\n", numSounds, buffer.GetSize());

			for(int i=0; i<numSounds;i++)
			{
				soundId = buffer.ReadShort();
				buffer.ReadString(soundName,128);

				if(soundId == -1 || !soundName[0])
						continue;
				m_pClGame->RegisterSound(soundName,CACHE_GAME, soundId);
			}
			break;
		}
	case SVC_IMAGELIST:
		{
			char imgName[128];
			int imgId=0;
			int numImages = buffer.ReadShort();

			ComPrintf("CL: ImageList :%d images: %d bytes\n", numImages, buffer.GetSize());
			
			for(int i=0; i<numImages;i++)
			{
				imgId = buffer.ReadShort();
				buffer.ReadString(imgName,128);

				if(imgId == -1 || !imgName[0])
						continue;
				m_pClGame->RegisterImage(imgName,CACHE_GAME, imgId);
			}
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
							buffer.ReadShort();
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
			int numClients = buffer.ReadByte();
			for(int i=0;i<numClients;i++)
			{
				if(buffer.ReadByte() != SV_CLFULLINFO)
					break;
				ReadClientInfo(buffer);
			}
			break;
		}
	}
}

/*
================================================
Read Client info
================================================
*/
void CGameClient::ReadClientInfo(CBuffer &buffer)
{
	int num = buffer.ReadByte();

ComPrintf("CL: REMOTE : GETTING CLIENT %d\n", num);

	m_clients[num].Reset();
	strcpy(m_clients[num].name, buffer.ReadString());

	char szCharacter[CL_MAXCHARNAME];
	strcpy(szCharacter,buffer.ReadString());

	char * skin = strchr(szCharacter,'/');
	
	//See if a skin name was given
	if(!skin)
	{
		ComPrintf("CL: Unable to get skinname: %s: Defaulting to %s\n", szCharacter, m_cvDefaultChar.string);
		strcpy(szCharacter,m_cvDefaultChar.string);
		//reset to default character
		skin = strchr(szCharacter,'/');
	}

	//Load skin
	char path[COM_MAXPATH];
	sprintf(path,"Models/Player/%s", szCharacter);
ComPrintf("CL: REMOTE : Loading player skin: %s\n", path);
	
	m_clients[num].skinNum = m_pClGame->RegisterImage(path, CACHE_LOCAL);
	m_clients[num].skinNum |= MODEL_SKIN_UNBOUND_LOCAL;

	//Load Model
	strncpy(m_clients[num].model, szCharacter, skin-szCharacter);
	sprintf(path,"Models/Player/%s/tris.md2", m_clients[num].model);
ComPrintf("CL: REMOTE: Loading player model: %s\n", path);

	m_clients[num].mdlIndex = m_pClGame->RegisterModel(path, CACHE_LOCAL);
	m_clients[num].mdlCache = CACHE_LOCAL;

	//Setup bounding box. gravity,. friction etc here as well
	m_clients[num].moveType = MOVETYPE_STEP;
	m_clients[num].angles.Set(0.0f,0.0f,0.0f);
	m_clients[num].velocity.Set(0,0,0);
	m_clients[num].origin.Set(0.0f,0.0f,64.0f);
	m_clients[num].mins = VEC_CLIENT_MINS;
	m_clients[num].maxs = VEC_CLIENT_MAXS;
	m_clients[num].inUse = true;

	ComPrintf("CL: %s entered at slot %d\n", m_clients[num].name,num);
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
bool CGameClient::ValidateName(const CStringVal &stringval)
{
	if(!m_ingame)
		return true;

	m_pClGame->GetReliableSendBuffer().WriteByte(CL_INFOCHANGE);
	m_pClGame->GetReliableSendBuffer().WriteChar('n');
	m_pClGame->GetReliableSendBuffer().WriteString(stringval.String());
	return true;
}

/*
======================================
Validate Rate before updating it 
on the server
======================================
*/
bool CGameClient::ValidateRate(const CStringVal &stringval)
{

	int rate = stringval.IntVal();
	if(rate < 100 || rate > 10000)
	{
		ComPrintf("Rate needs to be in (1000-10000): %d\n", rate);
		return false;
	}

	if(!m_ingame)
		return true;

	CBuffer &buffer = m_pClGame->GetReliableSendBuffer();
	buffer.WriteByte(CL_INFOCHANGE);
	buffer.WriteChar('r');
	buffer.WriteInt(rate);
	return true;
}


/*
================================================
Validate Model/Skin change. make sure we
locally have it
================================================
*/
bool CGameClient::ValidateCharacter(const CStringVal &stringval)
{
	char modelName[CL_MAXMODELNAME];
	const char * szCharacter = stringval.String();
	char * pSkin = strchr(szCharacter,'/');

	if(pSkin)
		strncpy(modelName,szCharacter,pSkin-szCharacter);
	else
		strncpy(modelName,szCharacter,CL_MAXMODELNAME);

	//Try to load the given model
	char path[COM_MAXPATH];
	I_FileReader * pFile = System::CreateFileReader(FILE_BUFFERED);

	//Validate model
	sprintf(path,"Models/Player/%s/tris.md2",modelName);

	if(!pFile->Open(path))
	{
		ComPrintf("CL: Unable to load model : %s\n", path);
		pFile->Destroy();
		return false;
	}

	pFile->Close();
	
	if(pSkin)
		sprintf(path,"Models/Player/%s/%s.tga",modelName, pSkin);
	else
		sprintf(path,"Models/Player/%s/%s.tga",modelName, modelName);

	//We dont have the given skin
	if(!pFile->Open(path))
	{
		ComPrintf("CL: Unable to load skin : %s\n", path);
		pFile->Destroy();
		return false;
	}
	pFile->Destroy();
	
	//Everything okay. override the CVar IF the skinName was not supplied
	if(!pSkin)
		sprintf(path,"%s/%s", modelName,modelName);
	else
		sprintf(path,"%s/%s", modelName,pSkin);

	m_cvCharacter.ForceSet(path);

	//Now unload the current stuff, and load the new stuff
	if(m_ingame)
	{
		m_pClGame->UnregisterModel(CACHE_LOCAL, m_pGameClient->mdlIndex);
		m_pClGame->UnregisterImage(CACHE_LOCAL, (m_pGameClient->skinNum & ~MODEL_SKIN_UNBOUND_LOCAL));

		//Load Skin
		m_pGameClient->skinNum = m_pClGame->RegisterImage(path, CACHE_LOCAL);
		m_pGameClient->skinNum |= MODEL_SKIN_UNBOUND_LOCAL;

		sprintf(path,"Models/Player/%s/tris.md2",modelName);

		m_pGameClient->mdlCache = CACHE_LOCAL;
		m_pGameClient->mdlIndex = m_pClGame->RegisterModel(path, CACHE_LOCAL);
		strcpy(m_pGameClient->model,modelName);

		//Notify server
		CBuffer &buffer = m_pClGame->GetReliableSendBuffer();
		buffer.WriteByte(CL_INFOCHANGE);
		buffer.WriteChar('c');
		buffer.WriteString(m_cvCharacter.string);
	}
	return false;
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
	m_clNum = clNum;
	m_pGameClient = &m_clients[clNum];

ComPrintf("CL: LOCAL: CLIENT NUM %d\n", clNum);
	
	m_pGameClient->Reset();
	strcpy(m_pGameClient->name, m_cvName.string);
	
	//Load model Resources
	char path[COM_MAXPATH];
	char * skin = strchr(m_cvCharacter.string,'/');
	
	if(skin)
	{
		strncpy(m_pGameClient->model, m_cvCharacter.string, skin - m_cvCharacter.string);
		skin++;
		sprintf(path,"Models/Player/%s/%s", m_pGameClient->model,skin);
	}
	else
	{
		strcpy(m_pGameClient->model, m_cvCharacter.string);
		sprintf(path,"Models/Player/%s/%s", m_pGameClient->model,m_pGameClient->model);
	}

ComPrintf("CL: LOCAL: Loading player skin: %s\n", path);
	m_pGameClient->skinNum = m_pClGame->RegisterImage(path, CACHE_LOCAL);
	m_pGameClient->skinNum |= MODEL_SKIN_UNBOUND_LOCAL;

	sprintf(path,"Models/Player/%s/tris.md2",m_pGameClient->model);
ComPrintf("CL: LOCAL: Loading player model: %s\n", path);
	m_pGameClient->mdlIndex = m_pClGame->RegisterModel(path, CACHE_LOCAL);
	m_pGameClient->mdlCache = CACHE_LOCAL;
	
	//Setup Game client info
	m_pGameClient->moveType = MOVETYPE_STEP;
	m_pGameClient->angles.Set(0.0f,0.0f,0.0f);
	m_pGameClient->velocity.Set(0,0,0);
	m_pGameClient->origin.Set(0.0f,0.0f,64.0f);
	m_pGameClient->mins = VEC_CLIENT_MINS;
	m_pGameClient->maxs = VEC_CLIENT_MAXS;

	HandleGameMsg(buffer);

	//Register static sound sources with SoundManager
	for(int i=0; i< GAME_MAXENTITIES; i++)
	{
		if((m_entities[i].inUse) && (m_entities[i].sndIndex > -1))
		{
			m_entities[i].sndCache = CACHE_GAME;
			m_entities[i].volume = 10;
			m_entities[i].attenuation = 5;
//ComPrintf("CL: Added Sound Source Index : %d\n", m_entities[i].sndIndex);
			m_pClGame->AddSoundSource(&m_entities[i]);
		}
	}

	m_campath = -1;
	
	//Setup camera
	m_pCamera = new CCamera(m_pGameClient->angles,
							m_vecForward, m_vecRight, m_vecUp,	
							m_pGameClient->velocity);

	m_pClGame->ForwardNetworkEvent(CLIENT_BEGINGAME);

	m_pGameClient->inUse = true;
	m_ingame = true;
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
======================================
Write UserInfo to buffer
======================================
*/
void CGameClient::WriteUserInfo(CBuffer &buffer)
{
	buffer.WriteString(m_cvName.string);
	buffer.WriteString(m_cvCharacter.string);
	buffer.WriteInt(m_cvInRate.ival);
}