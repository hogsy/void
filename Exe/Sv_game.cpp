#include "Sys_hdr.h"
#include "Sv_main.h"
#include "Sv_defs.h"

#include "Com_util.h"
#include "Com_world.h"



I_World * CServer::GetWorld()
{	return m_pWorld;
}

/*
======================================
Util funcs
======================================
*/
void CServer::AddServerCmd(const char * cmd)
{	m_svCmds.push_back(std::string(cmd));
}

void CServer::DebugPrint(const char * msg)
{	System::GetConsole()->ComPrint(msg);
}

/*
======================================
Game messed up. kill server
======================================
*/
void CServer::FatalError(const char * msg)
{
	for(int i=0; i< m_svState.numClients; i++)
		m_net.SendDisconnect(i,DR_SVERROR);
	ComPrintf("Server Error : %s\n", msg);
	Shutdown();
}


/*
======================================
Print a broadcast message
======================================
*/
void CServer::BroadcastPrintf(const char * msg,...)
{
	va_list args;
	va_start(args, msg);
	vsprintf(m_printBuffer, msg, args);
	va_end(args);
	m_net.BroadcastPrintf(m_printBuffer);
}

/*
======================================
Print a message to a given client
======================================
*/
void CServer::ClientPrintf(int clNum, const char * msg,...)
{
	va_list args;
	va_start(args, msg);
	vsprintf(m_printBuffer, msg, args);
	va_end(args);
	m_net.ClientPrintf(clNum,msg);
}




//==========================================================================
//==========================================================================



NetChanWriter & CServer::GetNetChanWriter()
{	return (reinterpret_cast<NetChanWriter &>(m_net));
}





/*
======================================

======================================
*/
void CServer::GetMultiCastSet(MultiCastSet &set, MultiCastType type, int clId)
{
	if((type == MULTICAST_ALL) || (type == MULTICAST_ALL_X))
	{
		set.Reset();
		for(int i=0;i<m_svState.numClients;i++)
		{
			if(m_clients[i] && m_clients[i]->spawned)
					set.dest[i] = true;
		}
		if(type == MULTICAST_ALL_X)
			set.dest[clId] = false;
	}
	else if((type == MULTICAST_PVS) || (type == MULTICAST_PVS_X))
	{
	}
	else if((type == MULTICAST_PHS) || (type == MULTICAST_PHS_X))
	{
	}
}

void CServer::GetMultiCastSet(MultiCastSet &set, MultiCastType type, const vector_t &source)
{
}









/*
======================================
Send sound message to clients in range
======================================
*/
void CServer::PlaySnd(const Entity &ent, int index, int channel, float vol, float atten)
{
}

void CServer::PlaySnd(vector_t &origin,  int index, int channel, float vol, float atten)
{
}



//==========================================================================
//==========================================================================



/*
======================================
Return an id for the given model
======================================
*/
int CServer::RegisterModel(const char * model)
{
	if(m_numModels == GAME_MAXMODELS)
		return -1;

	//Check to see whether we already have a model Id by that name
	for(int i=0; i<GAME_MAXMODELS; i++)
	{
		//we reached the end, no more models after this
		if(!m_modelList[i].name)
			break;
		if(strcmp(m_modelList[i].name, model) == 0)
		    return i;
	}
	m_numModels ++;
	m_modelList[i].name = new char[strlen(model)+1];
	strcpy(m_modelList[i].name,model);
	return i;
}

/*
======================================
Return an id for the given sound
======================================
*/
int CServer::RegisterSound(const char * sound)
{
	if(m_numSounds == GAME_MAXSOUNDS)
		return -1;

	//Check to see whether we already have a model Id by that name
	for(int i=0; i<GAME_MAXSOUNDS; i++)
	{
		//we reached the end, no more models after this
		if(!m_soundList[i].name)
			break;
		if(strcmp(m_soundList[i].name, sound) == 0)
		    return i;
	}
	m_numSounds ++;
	m_soundList[i].name = new char[strlen(sound)+1];
	strcpy(m_soundList[i].name,sound);
	return i;
}

/*
======================================
Return an id for the given image
======================================
*/
int CServer::RegisterImage(const char * image)
{
	if(m_numImages == GAME_MAXIMAGES)
		return -1;

	//Check to see whether we already have a model Id by that name
	for(int i=0; i<GAME_MAXIMAGES; i++)
	{
		//we reached the end, no more models after this
		if(!m_imageList[i].name)
			break;
		if(strcmp(m_imageList[i].name, image) == 0)
		    return i;
	}
	m_numImages ++;
	m_imageList[i].name = new char[strlen(image)+1];
	strcpy(m_imageList[i].name,image);
	return i;
}