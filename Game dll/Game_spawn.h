#ifndef VOID_GAME_ENT_SPAWNERS
#define VOID_GAME_ENT_SPAWNERS

/*
============================================================================
Subclassed game entities
============================================================================
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

//======================================================================================
//======================================================================================

/*
======================================
Entity Spawning class
All Entity Makers should register with it
======================================
*/
namespace EntSpawner {

void RegisterMakers();
void DestroyMakers();
Entity * CreateEntity(const char * classname, CBuffer &parms);

}

#endif