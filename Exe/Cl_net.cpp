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
				m_pSound->PlaySnd2d(m_hsTalk, CACHE_LOCAL);
				ComPrintf("%s: %s\n",name,buffer.ReadString());
				break;
			}
		case SV_DISCONNECT:
			{
				m_pSound->PlaySnd2d(m_hsMessage, CACHE_LOCAL);
				ComPrintf("Server quit\n");
				m_pNetCl->Disconnect(true);
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
				m_pNetCl->Reconnect(true);
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

				m_clients[num].mdlCache = CACHE_GAME;
				m_clients[num].skinNum = m_pClRen->LoadImage(path, CACHE_GAME, sindex);
				m_clients[num].skinNum |= MODEL_SKIN_UNBOUND_GAME;

				sprintf(path,"Players/%s/tris.md2", model);
				m_clients[num].mdlIndex = m_pClRen->LoadModel(path, CACHE_GAME,mindex);
				m_clients[num].mdlCache = CACHE_GAME;

				m_clients[num].inUse = true;

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
void CClient::HandleSpawnMsg(byte msgId, CBuffer &buffer)
{
	switch(msgId)
	{
	case SVC_GAMEINFO:
		{
//			int slotNum = buffer.ReadInt();

			char * game = buffer.ReadString();
ComPrintf("CL: Game: %s\n", game);
			char * map = buffer.ReadString();
ComPrintf("CL: Map: %s\n", map);
			if(!LoadWorld(map))
				m_pNetCl->Disconnect(false);
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
				{
					continue;
				}
				m_pClRen->LoadModel(modelName,CACHE_GAME,modelId);
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
				while(type != 0)
				{
					switch(type)
					{
					case 'm':
						{
							m_entities[id].mdlIndex = buffer.ReadShort();
							m_entities[id].skinNum = buffer.ReadShort();
							m_entities[id].frame = buffer.ReadShort();
							m_entities[id].nextFrame = m_entities[id].frame;
							m_entities[id].frac = 0;
							m_entities[id].mdlCache = CACHE_GAME;
							break;
						}
					case 's':
						{
							m_entities[id].sndCache = CACHE_GAME;
							m_entities[id].soundIndex = buffer.ReadShort();
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
	buffer.WriteString(m_cvName.string);
	buffer.WriteString(m_cvModel.string);
	buffer.WriteString(m_cvSkin.string);
	buffer.WriteInt(m_cvRate.ival);
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

	ComPrintf("%s: %s\n", m_cvName.string, msg);
	m_pSound->PlaySnd2d(m_hsTalk, CACHE_LOCAL);

	//Send this reliably ?
	m_pNetCl->GetReliableBuffer().WriteByte(CL_TALK);
	m_pNetCl->GetReliableBuffer().WriteString(msg);
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
		ComPrintf("Name = \"%s\"\n", m_cvName.string);
		return false;
	}
	if(!m_ingame)
		return true;

	m_pNetCl->GetReliableBuffer().WriteByte(CL_UPDATEINFO);
	m_pNetCl->GetReliableBuffer().WriteChar('n');
	m_pNetCl->GetReliableBuffer().WriteString(name);
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
		ComPrintf("Rate = \"%d\"\n", m_cvRate.ival);
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
	buffer.WriteByte(CL_UPDATEINFO);
	buffer.WriteChar('r');
	buffer.WriteInt(rate);
	return true;
}


/*
======================================

======================================
*/
void CClient::ShowNetStats()
{
	//Print Networking stats
	const NetChanState & chanState = m_pNetCl->GetChanState();

	m_pClRen->HudPrintf(0,390,0, "Latency %.2f", chanState.latency * 1000);
	m_pClRen->HudPrintf(0,400,0, "Drop stats %d/%d. Choked %d", chanState.dropCount, 
						chanState.dropCount + chanState.goodCount, chanState.numChokes);
	m_pClRen->HudPrintf(0,410,0, "In      %d", chanState.inMsgId);
	m_pClRen->HudPrintf(0,420,0, "In  Ack %d", chanState.inAckedId);
	m_pClRen->HudPrintf(0,430,0, "Out     %d", chanState.outMsgId);
	m_pClRen->HudPrintf(0,440,0, "Out Ack %d", chanState.lastOutReliableId);

}
