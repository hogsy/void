#ifndef VOID_SV_ENTITIES
#define VOID_SV_ENTITIES

#include "Com_defs.h"
#include "3dmath.h"

/*
======================================
The base game entitiy. 
can be subclasses for more specific stuff
======================================
*/
const int SV_MAXCLASSNAME = 32;
enum SVEntType
{
	ENT_ITEM,
	ENT_WEAPON,
	ENT_NPC,
	ENT_WORLD,
	ENT_CLIENT
};

class CEntity
{
public:
	CEntity(const char * ename, SVEntType etype) : type(etype)
	{	
		strcpy(classname, ename);
		origin.x = origin.y = origin.z = 0.0f;
	}
	
	int	 num;
	char classname[SV_MAXCLASSNAME];
	
	SVEntType	type;
	vector_t	origin;
};


/*
======================================
Client Entity on the server
======================================
*/
enum SVClientState
{
};

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
};

#endif

