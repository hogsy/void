#ifndef VOID_GAME_ENTITIES
#define VOID_GAME_ENTITIES

#include "Com_defs.h"
#include "Com_keys.h"
#include "3dmath.h"

/*
======================================
SERVER ENTITIES
======================================
*/

const int ENT_MAXCLASSNAME = 32;
const int ENT_MAXSTRING = 128;
const int ENT_MAXMESSAGE = 256;

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
	Entity(const char * ename)
	{	
		strcpy(classname, ename);
		origin.x = origin.y = origin.z = 0.0f;
		angles.x = angles.y = angles.z = 0.0f;
	}
	
	int			num;
	char		classname[ENT_MAXCLASSNAME];
	vector_t	origin;
	vector_t	angles;
};

/*
======================================
WorldSpawn
======================================
*/
struct EntWorldSpawn : public Entity
{
	EntWorldSpawn() : Entity("worldspawn")
	{
		memset(message,0,ENT_MAXMESSAGE);
		memset(music,0, ENT_MAXSTRING);
		gravity = 800;
	}

	char	message[ENT_MAXMESSAGE];
	char    music[ENT_MAXSTRING];
	int		gravity;
};

/*
======================================
Speaker
======================================
*/
struct EntSpeaker : public Entity
{
	enum	//attenuation etc 
	{	MAX_SOUNDFILENAME = 64
	};
	
	EntSpeaker(): Entity("ent_speaker")
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
	EntClient() : Entity("client")
	{	
		memset(name,0,32); 
		inUse = false; 
	}
	
	bool inUse;
	char name[32];

	vector_t mins;
	vector_t maxs;
};

//======================================================================================
//======================================================================================

typedef std::vector<KeyField> EntFields;

class CEntityMaker
{
public:
	
	//This is what the main spawning routine will use
	static Entity * CreateEnt(const char * classname, CBuffer &parms);

	virtual ~CEntityMaker() {}
	
protected:

	//Derived classes should use this
	CEntityMaker(std::string classname);

	//Every subclass should implement this
	virtual Entity * MakeEntity(const char * classname, CBuffer &parms) const;

	static void ParseKey(Entity * ent, const char * key, CBuffer &parms);

private:

	//Constructor is private
	CEntityMaker();

	static CEntityMaker mainEntMaker;
	static EntFields	entFields;

	static std::map<std::string, CEntityMaker *> makerRegistry;
	static std::map<std::string, CEntityMaker *>::iterator itRegistry;
};

#endif