#ifndef VOID_GAME_DEFS
#define VOID_GAME_DEFS

struct I_World;

/*
======================================
Basic "game" definitions.
======================================
*/

const int	GAME_MAXMODELS	= 256;
const int	GAME_MAXSOUNDS	= 256;
const int	GAME_MAXIMAGES	= 256;
const int	GAME_MAXENTITIES= 1024;
const int	GAME_MAXCLIENTS = 16;
const char	GAME_WORLDSDIR[]= "Worlds/";


//======================================================================================
//======================================================================================

enum EMoveType
{
	MOVETYPE_NOCLIP,
	MOVETYPE_BBOX,		//just a static bbox
	MOVETYPE_MISSLE,	
	MOVETYPE_TRAJECTORY,
	MOVETYPE_STEP
};

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
		mdlIndex = -1;
		frameNum = nextFrame = skinNum = 0;
		sndIndex = -1;
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
Client sends this to the server as frequently as possible
======================================
*/
struct ClCmd
{
	byte	time;			//Frame Time
	int		angles[3];		//Current View angels
	short	forwardmove, 
			rightmove, 
			upmove;
	//add buttons and what not
};


/*
================================================
defined in Game_move.cpp
================================================
*/
namespace EntMove {

void NoClipMove(BaseEntity *ent, vector_t &dir, float time);
void ClientMove(BaseEntity *ent, vector_t &dir, float time);
void SetWorld(I_World * pWorld);

}


#endif