#ifndef VOID_GAME_BASE_ENT
#define VOID_GAME_BASE_ENT

#include "Game_defs.h"

/*
============================================================================
Only contains data needed by routines shared between client and server code
Both client and server subclass this adding stuff they need

This data will be propagated to all connected clients
============================================================================
*/
struct BaseEntity
{
	BaseEntity()
	{
		num = -1;
		mdlIndex = sndIndex = -1;
		frameNum = nextFrame = skinNum = 0;
		volume = attenuation = 0;
		moveType = MOVETYPE_NOCLIP;
	}

	virtual ~BaseEntity() =0 {}

	int		num;
	int		mdlIndex, 
			frameNum,	
			nextFrame, 
			skinNum;
	int		sndIndex, 
			volume,	
			attenuation;
	
	EMoveType	moveType;
	
	vector_t	origin;
	vector_t	angles;
	vector_t	velocity;
	vector_t	mins;
	vector_t	maxs;
};

/*
======================================
Client sends this to the server as 
frequently as possible
======================================
*/
struct ClCmd
{
	enum
	{
		MOVEFORWARD = 1,
		MOVEBACK = 2,
		MOVELEFT = 4,
		MOVERIGHT = 8,
		JUMP = 16,
		CROUCH = 32
	};

	enum
	{
		NONE = 0,
		UPDATED = 1,
		SENT = 2
	};

	ClCmd() { Reset(); }

	void UpdateCmd(const ClCmd & cmd)
	{
		time = cmd.time;
		angles = cmd.angles;
		moveFlags |= cmd.moveFlags;
		svFlags = UPDATED;
	}
	
	void Reset()
	{ 
		moveFlags = 0;
		svFlags = NONE;
		time = 0.0f;
		angles.Set(0,0,0);
	}

	vector_t angles;	//Clients view angles
	float	 time;		//Clients frame Time
	byte	 moveFlags;	//Move flags
	byte	 svFlags;	//Server flags
	
	//add buttons and what not
};

const vector_t VEC_CLIENT_MINS(-16, -16, -24);
const vector_t VEC_CLIENT_MAXS(16, 16, 32);

/*
================================================
Applied to SV_UPDATE and SV_CLUPDATE messages
to indicate type of incoming data. not sure if
this should be here
================================================
*/
enum EClUpdateFlags
{
	SVU_DEFAULT = 0,
	SVU_GRAVITY = 1,
	SVU_FRICTION = 2,
	SVU_MAXSPEED = 4
};

/*
================================================
defined in Game_move.cpp
================================================
*/

struct I_World;

namespace EntMove {

void NoClipMove(BaseEntity *ent, vector_t &dir, float time);
void ClientMove(BaseEntity *ent, float time);
void SetWorld(I_World * pWorld);

}



#endif