#include "Cl_main.h"
#include "I_renderer.h"
#include "I_hud.h"
#include "Snd_main.h"
#include "Net_defs.h"
#include "Net_protocol.h"

/*
======================================
Process game message
======================================
*/
void CClient::HandleGameMsg(CBuffer &buffer)
{
	byte msgId = 0;
	while(msgId != 255)
	{
		msgId= buffer.ReadByte();
		
		//bad message
		if(msgId == 255)
			break;

		switch(msgId)
		{
		case SV_TALK:
			{
				char name[32];
				strcpy(name,buffer.ReadString());
				m_pSound->PlaySnd(m_hsTalk, CACHE_LOCAL);
				ComPrintf("%s: %s\n",name,buffer.ReadString());
				break;
			}
		case SV_DISCONNECT:
			{
				m_pSound->PlaySnd(m_hsMessage, CACHE_LOCAL);
				ComPrintf("Server quit\n");
				m_pNetCl->Disconnect(true);
				break;
			}
		case SV_PRINT:	//just a print message
			{
				m_pSound->PlaySnd(m_hsMessage, CACHE_LOCAL);
				ComPrintf("%s\n",buffer.ReadString());
				break;
			}
		case SV_RECONNECT:
			{
				m_pNetCl->Reconnect();
				break;
			}
		case SV_CLIENTINFO:
			{
				int num = buffer.ReadShort();
				m_clients[num].Reset();
				strcpy(m_clients[num].name, buffer.ReadString());

				int mindex = buffer.ReadShort();
				char model[64];
				strcpy(model,buffer.ReadString());

				int sindex = buffer.ReadShort();
				char path[COM_MAXPATH];

				sprintf(path,"Players/%s/%s", model, buffer.ReadString());

				m_clients[num].cache = CACHE_GAME;
				m_clients[num].skinnum = m_pImage->LoadImage(path, sindex, CACHE_GAME);
				m_clients[num].skinnum |= MODEL_SKIN_UNBOUND_GAME;

				sprintf(path,"Players/%s/tris.md2", model);
				m_clients[num].index = m_pModel->LoadModel(path, mindex, CACHE_GAME);
				m_clients[num].cache = CACHE_GAME;

				m_clients[num].inUse = true;

				break;
			}
		case SV_CLUPDATE:
			{
				//buffer.Reset();
				int num = buffer.ReadShort();

				if(m_clients[num].inUse)
				{
					m_clients[num].origin.x = buffer.ReadCoord();
					m_clients[num].origin.y = buffer.ReadCoord();
					m_clients[num].origin.z = buffer.ReadCoord();
					m_clients[num].angle.x = buffer.ReadAngle();
					m_clients[num].angle.y = buffer.ReadAngle();
					m_clients[num].angle.z = buffer.ReadAngle();
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
void CClient::HandleSpawnMsg(const byte &msgId, CBuffer &buffer)
{
	switch(msgId)
	{
	case SVC_GAMEINFO:
		{
			char * game = buffer.ReadString();
//ComPrintf("CL: Game: %s\n", game);
			char * map = buffer.ReadString();
//ComPrintf("CL: Map: %s\n", map);
			if(!LoadWorld(map))
				m_pNetCl->Disconnect(false);
			break;
		}
	case SVC_MODELLIST:
		{
			char modelName[32];
			int  modelId=0;

			int numModels = buffer.ReadInt();
			ComPrintf("CL: ModelList :%d models\n", numModels);

			for(int i=0; i<numModels;i++)
			{
				modelId = buffer.ReadShort();
				buffer.ReadString(modelName,32);

				if(modelId == -1 || !modelName[0])
				{
					continue;
				}
//				m_pModel->LoadModel(modelName,modelId,CACHE_GAME);
			}
			break;
		}
	case SVC_SOUNDLIST:
		{
			char soundName[32];
			int  soundId=0;

			int numSounds = buffer.ReadInt();
			ComPrintf("CL: SoundList :%d models\n", numSounds);

			for(int i=0; i<numSounds;i++)
			{
				soundId = buffer.ReadShort();
				buffer.ReadString(soundName,32);

				if(soundId == -1 || !soundName[0])
				{
					continue;
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
				m_entities[id].origin.x = buffer.ReadCoord();
				m_entities[id].origin.y = buffer.ReadCoord();
				m_entities[id].origin.z = buffer.ReadCoord();

				m_entities[id].angle.x = buffer.ReadAngle();
				m_entities[id].angle.y = buffer.ReadAngle();
				m_entities[id].angle.z = buffer.ReadAngle();

				type = buffer.ReadChar();
							
				if(type == 'm')
				{
//					m_entities[id].index = buffer.ReadShort();
					m_entities[id].skinnum = buffer.ReadShort();
					m_entities[id].frame = buffer.ReadShort();
					m_entities[id].nextframe = m_entities[id].frame;
					m_entities[id].frac = 0;
					m_entities[id].cache = CACHE_GAME;
				}
				else
				{
					m_entities[id].soundIndex = buffer.ReadShort();
					m_entities[id].volume = buffer.ReadShort();
					m_entities[id].attenuation = buffer.ReadShort();
				}

				if(buffer.BadRead())
				{
					ComPrintf("Error reading Ent %d\n", id);
					m_entities[id].Reset();
					break;
				}

//				m_entities[id].inUse = true;
				m_numEnts ++;
				id = buffer.ReadShort();
			}

			ComPrintf("CL: Parsed %d entities\n", m_numEnts);
			break;
		}
	case SVC_BEGIN:
		{
			HandleGameMsg(buffer);
			BeginGame();
			break;
		}
	}
}

/*
======================================
Handle disconnect from server
======================================
*/
void CClient::HandleDisconnect(bool listenserver)
{
//	ComPrintf("CL: KILLING LOCAL SERVER\n");

	//Kill server if local
	if(listenserver)
	{
		ComPrintf("CL: KILLING LOCAL SERVER\n");
		System::GetConsole()->ExecString("killserver");
	}
	UnloadWorld();
}

/*
======================================
Write UserInfo to buffer
======================================
*/
void CClient::WriteUserInfo(CBuffer &buffer)
{
	buffer.Write(m_clname.string);
	buffer.Write(m_clmodel.string);
	buffer.Write(m_clskin.string);
	buffer.Write(m_clrate.ival);
}


/*
======================================
Print a message 
======================================
*/
void CClient::Print(const char * msg, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, msg);
	vsprintf(textBuffer, msg, args);
	va_end(args);
	System::GetConsole()->ComPrint(textBuffer);
}

/*
======================================
Say something
======================================
*/
void CClient::Talk(const char * string)
{
	if(!m_ingame)
		return;

	//parse to right after "say"
	const char * msg = string + 4;
	while(*msg && *msg == ' ')
		msg++;

	if(!*msg || *msg == '\0')
		return;

	ComPrintf("%s: %s\n", m_clname.string, msg);
	m_pSound->PlaySnd(m_hsTalk, CACHE_LOCAL);

	//Send this reliably ?
	m_pNetCl->GetReliableBuffer().Write(CL_TALK);
	m_pNetCl->GetReliableBuffer().Write(msg);
}

/*
======================================
Validate name locally before asking 
the server to update it
======================================
*/
bool CClient::ValidateName(const CParms &parms)
{
	char name[24];
	parms.StringTok(1,name,24);

	if(!name[0])
	{
		ComPrintf("Name = \"%s\"\n", m_clname.string);
		return false;
	}
	if(!m_ingame)
		return true;

	m_pNetCl->GetReliableBuffer().Write(CL_UPDATEINFO);
	m_pNetCl->GetReliableBuffer().Write('n');
	m_pNetCl->GetReliableBuffer().Write(name);
	return true;
}

/*
======================================
Validate Rate before updating it 
on the server
======================================
*/
bool CClient::ValidateRate(const CParms &parms)
{
	int rate = parms.IntTok(1);
	if(rate == -1)
	{
		ComPrintf("Rate = \"%d\"\n", m_clrate.ival);
		return false;
	}

	if(rate < 1000 || rate > 30000)
	{
		ComPrintf("Rate is out of range\n");
		return false;
	}

	m_pNetCl->SetRate(rate);
	if(!m_ingame)
		return true;

	CBuffer &buffer = m_pNetCl->GetReliableBuffer();
	buffer.Write(CL_UPDATEINFO);
	buffer.Write('r');
	buffer.Write(rate);
	return true;
}
