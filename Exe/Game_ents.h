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

const int GAME_MAXCLIENTS = 16;
const int GAME_MAXENTITES = 1024;

/*
============================================================================
The basic game entitiy. 
This data will be propagated to connected clients
============================================================================
*/
struct Entity
{
	Entity(const char * ename)
	{	
		strcpy(classname, ename);
		origin.x = origin.y = origin.z = 0.0f;
		angles.x = angles.y = angles.z = 0.0f;

		modelIndex = -1;
		soundIndex = -1;

		memset(modelName,0,64);
		memset(skinName,0,64);

		skinNum = frameNum = 0;
		volume = attenuation = 0;
	}
	
	virtual ~Entity() {}

	int			num;

	char		classname[ENT_MAXCLASSNAME];
	vector_t	origin;
	vector_t	angles;

	int		    modelIndex;
	char		modelName[64];
	char		skinName[64];

	int			skinNum;
	int			frameNum;

	int			soundIndex;
	int			volume;
	int			attenuation;

	virtual bool WriteBaseline(CBuffer &buf) const
	{
		//we only write a baseline if the entity
		//is using a model or soundIndex
		if(modelIndex >= 0 || soundIndex >= 0)
		{
			buf.Write((short)num);
			buf.WriteCoord(origin.x);
			buf.WriteCoord(origin.y);
			buf.WriteCoord(origin.z);
			buf.WriteAngle(angles.x);
			buf.WriteAngle(angles.y);
			buf.WriteAngle(angles.z);

			if(modelIndex >=0)
			{
				//set the highbit if its a modelindex
				buf.Write('m');
				buf.Write((short)modelIndex);
				buf.Write((short)skinNum);
				buf.Write((short)frameNum);
			}
			else
			{
				buf.Write('s');
				buf.Write((short)soundIndex);
				buf.Write((short)volume);
				buf.Write((short)attenuation);
			}
			return true;
		}
		return false;
	}
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

	virtual ~EntWorldSpawn() { }

	char	message[ENT_MAXMESSAGE];
	char    music[ENT_MAXSTRING];
	int		gravity;
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
		spawned = false;
	}

	virtual ~EntClient() { }

	virtual bool WriteBaseline(CBuffer &buf) const
	{
		return Entity::WriteBaseline(buf);
	}
	
	bool inUse;
	bool spawned;

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