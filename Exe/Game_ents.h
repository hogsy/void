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
const int ENT_MAXRESNAME = 64;
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

add all common parms we want to be baselined here
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
	virtual ~Entity() {}

	
	int			num;
	char		classname[ENT_MAXCLASSNAME];
	vector_t	origin;
	vector_t	angles;

	virtual void Write(CBuffer &buf) const
	{
		buf.Write(num);
		buf.Write(classname);
		buf.WriteCoord(origin.x);
		buf.WriteCoord(origin.y);
		buf.WriteCoord(origin.z);
		buf.WriteAngle(angles.x);
		buf.WriteAngle(angles.y);
		buf.WriteAngle(angles.z);
	}

	//Register resources here
	virtual void Initialize(){}
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

	virtual void Write(CBuffer &buf) const {}

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
	EntSpeaker(): Entity("target_speaker")
	{
		volume = 0;
		attenuation = 0;

		soundIndex = 0;
		memset(soundName,0,ENT_MAXRESNAME);
	}

	virtual void Write(CBuffer &buf) const
	{
//		Entity::Write(buf);
//		buf.Write(soundIndex);
	}

	int	 volume;
	int  attenuation;
	int  soundIndex;
	char soundName[ENT_MAXRESNAME];
	virtual void Initialize();

};


/*
======================================
EntWorldModel
======================================
*/
struct EntWorldModel : public Entity
{
	EntWorldModel(): Entity("misc_model")
	{	
		modelIndex = 0;
		memset(modelName,0,ENT_MAXRESNAME);
	}

	virtual void Write(CBuffer &buf) const
	{
		Entity::Write(buf);
		buf.Write(modelIndex);
	}

	int  modelIndex;
	char modelName[ENT_MAXRESNAME];
	virtual void Initialize();
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