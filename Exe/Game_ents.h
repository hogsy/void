#ifndef VOID_GAME_ENTITIES
#define VOID_GAME_ENTITIES

#include "Com_defs.h"
#include "Com_buffer.h"
#include "3dmath.h"

//======================================================================================
//======================================================================================
const int ENT_MAXCLASSNAME = 32;


enum EntType
{
	ENT_ITEM,
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
struct Entity
{
	Entity(const char * ename, EntType etype) : type(etype)
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
Speaker
======================================
*/
struct EntSpeaker : public Entity
{
	enum	//attenuation etc 
	{	
		MAX_SOUNDFILENAME = 64
	};
	
	EntSpeaker(): Entity("ent_speaker", ENT_WORLD)
	{
	}

	int	 volume;
	int  attenuation;
	char soundName[MAX_SOUNDFILENAME];
};


/*
======================================
Client Entity on the server
======================================
*/
struct EntClient : public Entity
{
	EntClient() : Entity("client", ENT_CLIENT)
	{	
		memset(name,0,32); 
		inUse = false; 
	}
	
	bool inUse;
	char name[32];

	vector_t mins;
	vector_t maxs;
};


/*
======================================================================================
To support creation of a new entity type
Derive class from CBaseEntityMaker
create an object of that class
======================================================================================
*/
class CEntityMaker
{
public:

	static Entity * CreateEnt(CBuffer &parms)
	{
		char * classname = parms.ReadString();
		if(!parms.BadRead())
		{
			CEntityMaker * maker = (*makerRegistry.find(std::string(classname))).second;
			if(maker)
				return maker->MakeEntity(parms);
		}
		return 0; 
	}
	
protected:

	typedef std::map<std::string, CEntityMaker *> EntMakerMap;
	
	CEntityMaker(std::string classname)
	{	makerRegistry.insert( std::make_pair(classname,this));
	}

	virtual ~CEntityMaker() {}
	//Every subclass has to implement this
	virtual Entity * MakeEntity(CBuffer &parms) const = 0;

private:
	static EntMakerMap			 makerRegistry;
	static EntMakerMap::iterator itRegistry;
};

#endif