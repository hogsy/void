#include "Sys_hdr.h"
#include "Com_vector.h"
#include "Com_camera.h"
#include "Com_world.h"
#include "Com_buffer.h"
#include "Cl_defs.h"
#include "Cl_main.h"
#include "Cl_game.h"
#include "I_hud.h"
#include "Snd_main.h"
#include "Mus_main.h"


CClientState::	CClientState(CClient & rClient, I_HudRenderer * pHud,
				 CSoundManager * pSound,
				CMusic		   * pMusic) : 
	m_rClient(rClient), m_pHud(pHud), m_pSound(pSound), m_pMusic(pMusic)
{
	m_ingame = false;

	m_pGameClient = 0;

	m_numEnts = 0;
	
	m_pCamera = 0;
	
	m_pWorld = 0;
}

CClientState::~CClientState()
{
	if(m_pCamera)
		delete m_pCamera;
	m_pWorld = 0;

	m_pHud=0;
	m_pSound=0;
	m_pMusic=0;

	m_pGameClient = 0;
}


void CClientState::Spawn(vector_t	*origin, vector_t *angles)
{
}

void CClientState::RunFrame(float frameTime)
{
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
void CClientState::WriteCmdUpdate(CBuffer &buf)
{
/*	buf.Reset();
	buf.WriteByte(CL_MOVE);
	buf.WriteFloat(m_cmd.time);
*/		
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
void CClientState::UpdateView()
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
void CClientState::BeginGame()
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


bool CClientState::LoadWorld(CWorld * pWorld)
{
	//setup
	m_pWorld = pWorld;
	EntMove::SetWorld(m_pWorld);

	ComPrintf("CClientState::Load World: OK\n");
	return true;

}

void CClientState::UnloadWorld()
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
		if(m_entities[i].inUse)
		{
			if(m_entities[i].sndIndex > -1)
				m_pSound->RemoveStaticSource(&m_entities[i]);
			m_entities[i].Reset();
		}

//	CWorld::DestroyWorld(m_pWorld);
	m_pWorld = 0;

	m_ingame = false;
}
