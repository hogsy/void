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
	ClCmd() { Reset(); }
	void Reset()
	{ 
		flags = 0;
		time = angles[0] = angles[1] = angles[2] = 0.0f;
		forwardmove = rightmove = upmove = 0; 
	}

	float	time;			//Frame Time
	float	angles[3];		//Current View angels
	short	forwardmove, 
			rightmove, 
			upmove;
	byte	flags;
	
	//add buttons and what not
};


/*
================================================
defined in Game_move.cpp
================================================
*/

struct I_World;

namespace EntMove {

void NoClipMove(BaseEntity *ent, vector_t &dir, float time);
void ClientMove(BaseEntity *ent, vector_t &dir, float time);
void SetWorld(I_World * pWorld);

}



#endif