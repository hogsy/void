#include "Game_main.h"
#include "Game_ents.h"

/*
======================================
An Entity Fields list which can be 
associated with the distinct entities
======================================
*/
struct EntFields
{
	EntFields() { field = 0; next = 0; }
	~EntFields(){ if(field) delete field; if(next) delete next; }

	KeyField * field;
	EntFields * next;
};

/*
============================================================================
The Basic Entity Maker
Entity Makers cannot be held responsible for deleting their entity Fields since
that data is static, and other classes can inherit from them
so they just keep a pointer to their appropriate fields and somoene else manages
deletion for them
============================================================================
*/
class BaseEntMaker
{
public:
	BaseEntMaker(EntFields * fields) { spawnFields = fields;}
	virtual ~BaseEntMaker() {}

	void AddField(const char *ikey, int ioffset, KeyType itype)
	{
		EntFields  * iterator = spawnFields;
		while(iterator->next)
			iterator = iterator->next;
		iterator->field = new KeyField(ikey, ioffset, itype);
		iterator->next = new EntFields();		
	}

	//subclasses should override this if they need to create correponding types
	virtual Entity * MakeEntity(const char * classname, CBuffer &parms) const
	{
		Entity * ent = new Entity(classname);
		char * key = parms.ReadString();
		while(key && *key != 0)
		{
			ParseKey(ent,key,parms);
			key = parms.ReadString();
		};
		return ent; 
	}
	
protected:
	//Derived classes forced to use this constructor, so that they
	//cannot modify the spawnFields
	BaseEntMaker() {}
	
	static void ParseKey(Entity * ent, const char * key, CBuffer &parms)
	{
		//Handle resource Names differently
		if(strcmp(key,"model")==0)
			ent->modelIndex = g_pImports->RegisterModel(parms.ReadString());
		else if(strcmp(key,"noise")==0)
			ent->soundIndex = g_pImports->RegisterSound(parms.ReadString());
		else
		{	
			EntFields  * iterator = spawnFields;
			while(iterator && iterator->next)
			{
				if(strcmp(iterator->field->name, key) == 0)
				{	KeyField::ReadField(*(iterator->field),parms, reinterpret_cast<byte*>(ent));
					return;
				}
				iterator = iterator->next;
			}
		}
	}

private:
	//Make sure only one copy of the Fields is created
	static EntFields * spawnFields;
};

/*
======================================
Template Maker class for derived entities
======================================
*/
template <class T_Ent, class T_BaseMakerEnt= BaseEntMaker>
class EntMaker: public T_BaseMakerEnt
{
public:

	EntMaker(EntFields * fields) { spawnFields = fields;}
	virtual ~EntMaker() {}

	void AddField(const char *ikey, int ioffset, KeyType itype)
	{
		EntFields  * iterator = spawnFields;
		while(iterator->next)
			iterator = iterator->next;
		iterator->field = new KeyField(ikey, ioffset, itype);
		iterator->next = new EntFields();		
	}

	//Maker func
	virtual Entity * MakeEntity(const char * classname, CBuffer &parms) const
	{
		T_Ent * ent = new T_Ent();
		char * key = parms.ReadString();
		while(key && *key != 0)
		{
			ParseKey(ent,key,parms);
			key = parms.ReadString();
		};
		return ent;
	}

protected:
	//Derived classes forced to use this constructor, so that they
	//cannot modify the spawnFields
	EntMaker() {}

	static void ParseKey(T_Ent * ent, const char * key, CBuffer &parms)
	{
		EntFields  * iterator = spawnFields;
		while(iterator && iterator->next)
		{	
			if(!strcmp(iterator->field->name, key) == 0)
			{	
				KeyField::ReadField(*(iterator->field),parms, reinterpret_cast<byte*>(ent));
				return;
			}
			iterator = iterator->next;
		}
		//call parent if no match
		T_BaseMakerEnt::ParseKey(ent,key,parms);
	}

private:
	//extra fields defined by this entity
	static EntFields * spawnFields;
};

/*
======================================================================================
Add Spawnfields for different Entity types here
======================================================================================
*/
EntFields *  BaseEntMaker::spawnFields;
EntFields *  EntMaker<EntWorldSpawn>::spawnFields;

/*
======================================================================================
keeps track of registered EntityMakers and their Fields
======================================================================================
*/
struct MakerEntry
{
	MakerEntry()
	{	classname = 0;
		pMaker = 0;	pFields = 0; next = 0;
	}

	~MakerEntry()
	{
		if(pMaker)
			delete pMaker;
		if(pFields)
			delete pFields;
		if(classname)
			delete [] classname;
		//recursive delete of all entries
		if(next)	
		{	delete next;
			next =0;
		}
	}

	char * classname;
	BaseEntMaker * pMaker;
	EntFields     * pFields;

	MakerEntry   * next;
};
static MakerEntry  * makerRegistry=0;

void AddMaker(const char * classname, BaseEntMaker * pMaker, EntFields * pFields)
{
	MakerEntry  * iterator = makerRegistry;
	
	while(iterator->next)
		iterator = iterator->next;
	iterator->classname = new char [strlen(classname)+1];
	strcpy(iterator->classname, classname);
	iterator->pMaker = pMaker;
	iterator->pFields = pFields;
	iterator->next = new MakerEntry();
}


/*
======================================================================================
The Main EntitySpawner Interface, provides
-A place to register Makers for Entity Types and their corresponding fields
-A place to unregister and destroy all makers
-A means to create new entity from fields and parameters
======================================================================================
*/
namespace EntSpawner {
/*
======================================
Register Makers here
======================================
*/
void RegisterMakers()
{
	//create the registry
	makerRegistry = new MakerEntry();

	//Automatically add the BasicEntMaker to the registry
	Entity * ent = 0;
	EntFields * baseEntFields = new EntFields;
	BaseEntMaker * baseEntMaker = new BaseEntMaker(baseEntFields);
	baseEntMaker->AddField("origin",(int)&(ent->origin), KEY_VECTOR);
	baseEntMaker->AddField("angles",(int)&(ent->angles), KEY_VECTOR);
	baseEntMaker->AddField("volume",(int)&(ent->volume), KEY_INT);
	baseEntMaker->AddField("attenuation",(int)&(ent->attenuation), KEY_INT);
	AddMaker(" ", baseEntMaker, baseEntFields);

	EntWorldSpawn * worldspawn = 0;
	EntFields * worldSpawnFields = new EntFields;
	EntMaker <EntWorldSpawn> * worldSpawnMaker = new EntMaker<EntWorldSpawn>(worldSpawnFields);
	worldSpawnMaker->AddField("name",(int)&(worldspawn->message), KEY_STRING);
	worldSpawnMaker->AddField("music",(int)&(worldspawn->music), KEY_STRING);
	worldSpawnMaker->AddField("gravity",(int)&(worldspawn->gravity), KEY_INT);
	AddMaker("worldspawn", worldSpawnMaker, worldSpawnFields);
}

/*
======================================
Destroy maker Regsitry
======================================
*/
void DestroyMakers()
{	if(makerRegistry) delete makerRegistry;
}

/*
======================================
Create new entity
======================================
*/
Entity * CreateEntity(const char * classname, CBuffer &parms)
{
	if(parms.BadRead())
		return 0;

	MakerEntry  * iterator = makerRegistry;
	while(iterator->next)
	{
		if(strcmp(iterator->classname, classname)==0)
			return iterator->pMaker->MakeEntity(classname,parms);
		iterator = iterator->next;
	}
	//didnt find any proper maker, default to first
	return makerRegistry->pMaker->MakeEntity(classname,parms);
}

}

//======================================================================================
//======================================================================================

bool CGame::SpawnEntity(CBuffer &buf)
{
	buf.BeginRead();
	char * classname = buf.ReadString();

	Entity * ent = EntSpawner::CreateEntity(classname,buf);
	if(ent)
	{
ComPrintf("%s at { %.2f %.2f %.2f }\n", ent->classname, ent->origin.x, ent->origin.y, ent->origin.z);
		entities[numEnts] = ent;
		entities[numEnts]->num = numEnts;
		numEnts++;
		return true;
	}
	return false;
}
