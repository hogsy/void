#include "Sv_main.h"

/*
============================================================================
Main Entity Maker class
============================================================================
*/
std::map<std::string, CEntityMaker *>		    CEntityMaker::makerRegistry;
std::map<std::string, CEntityMaker *>::iterator CEntityMaker::itRegistry;

EntFields	 CEntityMaker::entFields;
CEntityMaker CEntityMaker::mainEntMaker;

/*
======================================
Constructors
======================================
*/
//Protected constructor
CEntityMaker::CEntityMaker(std::string classname)
{	makerRegistry.insert( std::make_pair(classname,this));
}

//Private constructor
CEntityMaker::CEntityMaker()
{
	//register fields
	Entity * ent = 0;
	entFields.push_back(KeyField("origin",(int)&(ent->origin), KEY_VECTOR));
	entFields.push_back(KeyField("angles",(int)&(ent->angles), KEY_VECTOR));
	entFields.push_back(KeyField("volume",(int)&(ent->volume), KEY_INT));
	entFields.push_back(KeyField("attenuation",(int)&(ent->attenuation), KEY_INT));
}

//ntFields.push_back(KeyField("sound",(int)&(ent->soundName), KEY_STRING));

/*
======================================
This is whats exposed as public
======================================
*/
Entity * CEntityMaker::CreateEnt(const char * classname, CBuffer &parms)
{
	itRegistry = makerRegistry.find(std::string(classname));
	if(itRegistry != makerRegistry.end())
		return (*itRegistry).second->MakeEntity(classname,parms);
	return mainEntMaker.MakeEntity(classname,parms);  //
}

/*
======================================
Protected funcs used by derived classes
======================================
*/
void CEntityMaker::ParseKey(Entity * ent, const char * key, CBuffer &parms)
{
	for(EntFields::iterator it = entFields.begin(); it != entFields.end(); it++)
	{	
		//Matched. then parse
		if(strcmp(it->name, key) == 0)	
		{
			KeyField::ReadField(*it,parms, reinterpret_cast<byte*>(ent));
			return;
		}
	}
}

/*
======================================
Every subclass should implement this
======================================
*/
Entity * CEntityMaker::MakeEntity(const char * classname, CBuffer &parms) const 
{ 
	Entity * ent = new Entity(classname);
	char * key = parms.ReadString();
	while(key && *key != 0)
	{
//Handle resource Names differently
		if(strcmp(key,"model")==0)
			ent->modelIndex = g_pServer->RegisterModel(parms.ReadString());
		else if(strcmp(key,"noise")==0)
			ent->soundIndex = g_pServer->RegisterSound(parms.ReadString());
		else
			ParseKey(ent,key,parms);
		key = parms.ReadString();
	};
	return ent; 
};

/*
======================================================================================
Template class for EntityMakers
======================================================================================
*/
template <class T_Ent, class T_BaseMakerEnt= CEntityMaker>
class EntMaker: public T_BaseMakerEnt
{
public:

	EntMaker(const char * classname) : T_BaseMakerEnt(classname){}
	virtual ~EntMaker() {}

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

	static AddField(const KeyField &field)
	{	entFields.push_back(field);
	}

protected:

	static void ParseKey(T_Ent * ent, const char * key, CBuffer &parms)
	{
		for(EntFields::iterator it = entFields.begin(); it != entFields.end(); it++)
		{	
			//Matched. then parse
			if(strcmp(it->name, key) == 0)	
			{
				KeyField::ReadField(*it,parms, reinterpret_cast<byte*>(ent));
				return;
			}
		}
		//call parent if no match
		T_BaseMakerEnt::ParseKey(ent,key,parms);
	}
private:
	//extra fields defined by this entity
	static EntFields entFields;
};


//======================================================================================
//======================================================================================


//Worldspawn spawner
EntMaker <EntWorldSpawn> worldSpawnMaker("worldspawn");
EntFields EntMaker<EntWorldSpawn,CEntityMaker>::entFields;

/*
EntMaker <EntSpeaker> speakerMaker("target_speaker");
EntFields EntMaker<EntSpeaker>::entFields;

EntMaker <EntWorldModel> miscModelMaker("misc_model");
EntFields EntMaker<EntWorldModel>::entFields;
*/


void CServer::InitGame()
{
	EntWorldSpawn * worldspawn = 0;
	EntMaker<EntWorldSpawn>::AddField(KeyField("name",(int)&(worldspawn->message), KEY_STRING));
	EntMaker<EntWorldSpawn>::AddField(KeyField("music",(int)&(worldspawn->music), KEY_STRING));
	EntMaker<EntWorldSpawn>::AddField(KeyField("gravity",(int)&(worldspawn->gravity), KEY_INT));

/*	EntSpeaker * speaker = 0;
	EntMaker<EntSpeaker>::AddField(KeyField("volume",(int)&(speaker->volume), KEY_INT));
	EntMaker<EntSpeaker>::AddField(KeyField("attenuation",(int)&(speaker->attenuation), KEY_INT));
	EntMaker<EntSpeaker>::AddField(KeyField("sound",(int)&(speaker->soundName), KEY_STRING));

	EntWorldModel * miscModel = 0;
	EntMaker<EntWorldModel>::AddField(KeyField("model",(int)&(miscModel->modelName), KEY_STRING));
*/
}

/*
void EntSpeaker::Initialize()
{	soundIndex = g_pServer->RegisterSound(soundName);
}

void EntWorldModel::Initialize()
{	modelIndex = g_pServer->RegisterModel(modelName);
}
*/

/*
void Entity::Initialize()
{
}
*/








#if 0

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

/*	virtual void Write(CBuffer &buf) const
	{
		Entity::Write(buf);
		buf.Write(soundIndex);
	}
*/
//	int	 volume;
//	int  attenuation;
//	int  soundIndex;
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

/*	virtual void Write(CBuffer &buf) const
	{
		Entity::Write(buf);
		buf.Write(modelIndex);
	}
*/

//	int  modelIndex;
	char modelName[ENT_MAXRESNAME];
//	virtual void Initialize();
};

#endif









