#include "Cl_main.h"
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
		msgId= (int)buffer.ReadByte();
		//bad message
		if(msgId == 255)
			break;

		switch(msgId)
		{
		case SV_TALK:
			{
				char name[32];
				strcpy(name,buffer.ReadString());
				m_pSound->PlaySnd(m_hsTalk);
				ComPrintf("%s: %s\n",name,buffer.ReadString());
				break;
			}
		case SV_DISCONNECT:
			{
				m_pSound->PlaySnd(m_hsMessage);
				ComPrintf("Server quit\n");
				m_pNetCl->Disconnect(true);
				break;
			}
		case SV_PRINT:	//just a print message
			{
				m_pSound->PlaySnd(m_hsMessage);
				ComPrintf("%s\n",buffer.ReadString());
				break;
			}
		case SV_RECONNECT:
			{
				m_pNetCl->Reconnect();
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
				m_pNetCl->Disconnect();
			break;
		}
	case SVC_MODELLIST:
		{
			ComPrintf("CL: ModelList :%d\n", buffer.GetSize());
			break;
		}
	case SVC_SOUNDLIST:
		{
			ComPrintf("CL: SoundList :%d\n", buffer.GetSize());
			break;
		}
	case SVC_IMAGELIST:
		{
			ComPrintf("CL: ImageList :%d\n", buffer.GetSize());
			break;
		}
	case SVC_BASELINES:
		{
			ComPrintf("CL: Baselines :%d\n", buffer.GetSize());
			break;
		}
	case SVC_BEGIN:
		{
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
	ComPrintf("CL: KILLING LOCAL SERVER\n");

	//Kill server if local
	if(listenserver)
		System::GetConsole()->ExecString("killserver");
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
	m_pSound->PlaySnd(m_hsTalk);

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
