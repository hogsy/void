#ifndef VOID_GAME_ENTITIES
#define VOID_GAME_ENTITIES

#include "Com_defs.h"
#include "3dmath.h"

//======================================================================================
//======================================================================================

const int ENT_MAXCLASSNAME = 32;

enum EntType
{
	ENT_ITEM,
	ENT_WEAPON,
	ENT_NPC,
	ENT_WORLD,
	ENT_CLIENT
};

/*
======================================
The base game entitiy. 
can be subclasses for more specific stuff
======================================
*/

class CEntity
{
public:
	CEntity(const char * ename, EntType etype) : type(etype)
	{	strcpy(classname, ename);
		
		origin.x = origin.y = origin.z = 0.0f;
		angles.x = angles.y = angles.z = 0.0f;
	}
	
	int	 num;
	char classname[ENT_MAXCLASSNAME];
	
	EntType		type;
	vector_t	origin;
	vector_t	angles;
};

/*
======================================
Client Entity on the server
======================================
*/

class CEntClient : public CEntity
{
public:
	CEntClient() : CEntity("client", ENT_CLIENT)
	{	
		memset(name,0,32); 
		inUse = false; 
	}
	
	bool inUse;
	char name[32];

	vector_t mins;
	vector_t maxs;
};

#endif