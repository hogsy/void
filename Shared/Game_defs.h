#ifndef VOID_GAME_DEFS
#define VOID_GAME_DEFS

/*
==========================================================================
Basic "game" definitions.
==========================================================================
*/
const int	GAME_VERSION    = 1;
const float	GAME_FRAMETIME  = 0.1f;

const int	GAME_MAXMODELS	= 256;
const int	GAME_MAXSOUNDS	= 256;
const int	GAME_MAXIMAGES	= 256;
const int	GAME_MAXENTITIES= 1024;
const int	GAME_MAXCLIENTS = 16;
const char	GAME_WORLDSDIR[]= "Worlds/";

const char  CL_MAXNAME		= 32;
const char  CL_MAXMODELNAME = 32;
const char  CL_MAXCHARNAME  = 64;

const vector_t VEC_CLIENT_MINS(-16, -16, -24);
const vector_t VEC_CLIENT_MAXS(16, 16, 32);

/*
================================================
Basic client side definitions There are needed 
by the client resource managers like the 
Renderer and Sound system etc
================================================
*/
const int CACHE_NUMCACHES = 3;

enum CacheType
{
	CACHE_LOCAL = 0,	//Persistant through out the game, Client is reponsible for loading these.
	CACHE_GAME	= 1,	//Map specific, should be unloaded on map change
	CACHE_TEMP	= 2,	//Temp object,  should release once it has been used
};

/*
================================================
Different sound channel types
================================================
*/
enum SndChannelType
{
	CHAN_AUTO   = 0,		//first free
	CHAN_UI		= 1,		//UI sound, no coordinates
	CHAN_WORLD  = 2,		//ambient, world sounds etc
	CHAN_CLIENT = 3,		//sounds from the player
	CHAN_MONSTER= 4,		//Monster and player share channels
	CHAN_PLAYER = 4,
	CHAN_ITEM   = 5,		//item noises, pickups etc
	CHAN_WEAPON	= 6,		//weapon noises
	
	CHAN_LOOPING = 128		//flagged
};

/*
================================================
Model skin rendereing flags
Or'ed with skinNum
================================================
*/
enum
{
	MODEL_SKIN_BOUND = 0,
	MODEL_SKIN_UNBOUND_GAME  = 0X80000000,
	MODEL_SKIN_UNBOUND_LOCAL = 0X40000000
};


/*
==========================================================================
Move Stuff
==========================================================================
*/
enum EMoveType
{
	MOVETYPE_NOCLIP,
	MOVETYPE_BBOX,		//just a static bbox
	MOVETYPE_MISSLE,	
	MOVETYPE_TRAJECTORY,
	MOVETYPE_STEP
};

struct I_World;
struct BaseEntity;

namespace EntMove {

void NoClipMove(BaseEntity *ent, vector_t &dir, float time);
int  ClientMove(BaseEntity *ent, float time);
void SetWorld(I_World * pWorld);

}

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
		skinNum = 0;
		volume = attenuation = 0;
		moveType = MOVETYPE_NOCLIP;
	}

	virtual ~BaseEntity() =0 {}

	int		num;

	int		mdlIndex, 
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
==========================================================================
Client sends this to the server as frequently as possible
==========================================================================
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
		CROUCH = 32,	//move modifiers
		WALK = 64,		//move modifiers
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

/*
================================================
Applied to SV_UPDATE and SV_CLUPDATE messages
to indicate type of incoming data. 
FIXME : this doesn't fit in here
================================================
*/
enum EClUpdateFlags
{
	SVU_DEFAULT = 0,
	SVU_GRAVITY = 1,
	SVU_FRICTION = 2,
	SVU_MAXSPEED = 4,
};

/*
================================================
Maintain frame animation state
================================================
*/
struct AnimState
{
	AnimState() 
		: frameBegin(0), frameEnd(0), totalFrames(0),  currentFrame (0), frac(0) {}

	AnimState(const AnimState &anim)
		: frameBegin(anim.frameBegin), frameEnd(anim.frameEnd), totalFrames(anim.totalFrames),
		  currentFrame (0), frac (0)	{}
	
	AnimState & operator = (const AnimState &anim)
	{
		frameBegin = anim.frameBegin;
		frameEnd = anim.frameEnd;
		totalFrames = anim.totalFrames;
		currentFrame = 0;
		frac = 0;
		return *this;
	}

	void Set(int begin, int end, int totFrames=0) 
	{
		frameBegin = begin;
		frameEnd = end;
		totalFrames = totFrames;
		currentFrame=0;
		frac = 0;
	}

	int frameBegin;
	int frameEnd;
	
	int	currentFrame;
	int totalFrames;

	float frac;	// should never be touched outside of here or renderer
};

#endif