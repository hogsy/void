#ifndef VOID_SV_ENTITIES
#define VOID_SV_ENTITIES

#include "Com_defs.h"
#include "3dmath.h"

//======================================================================================
//======================================================================================

const int SV_MAXCLASSNAME = 32;

enum EntityType
{
	ENT_ITEM,
	ENT_WEAPON,
	ENT_NPC,
	ENT_WORLD,
	ENT_CLIENT
};

//======================================================================================
//======================================================================================
/*
======================================
The base game entitiy. 
can be subclasses for more specific stuff
======================================
*/
class CEntity
{
public:

	int			num;
	EntityType	type;
	char		classname[SV_MAXCLASSNAME];
	
	vector_t	origin;

	virtual void Run()=0;
};


//======================================================================================
//======================================================================================

struct EntCreationFuncs
{
	CEntity * (Create)();
	const char * classname;
};

/*
======================================
Entity creator. Reads world data
and creates entities with correct parms
======================================
*/
class CEntityMaker
{
public:
	static CEntity * CreateEntity();
};


#endif

