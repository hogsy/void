#include "Game_ents.h"

//======================================================================================
//======================================================================================

EntMakerMap			 CEntityMaker::makerRegistry;
EntMakerMap::iterator CEntityMaker::itRegistry;


/*



class CEntClientMaker: public CEntityMaker
{
public:
	CEntClientMaker() : CEntityMaker("client")	{}

	Entity * MakeEntity(CBuffer &parms) const
	{	return 0;
	}

	static const CEntClientMaker registerThis;
};
*/


class CEntSpeakerMaker: public CEntityMaker
{
public:
	CEntSpeakerMaker() : CEntityMaker("ent_speaker")	{}

	Entity * MakeEntity(CBuffer &parms) const
	{	return 0;
	}

	static const CEntSpeakerMaker registerThis;
};

/*
class CEntPlayerStartMaker: public CEntityMaker
{
public:
	CEntPlayerStartMaker() : CEntityMaker("ent_player_start") {}

	Entity * MakeEntity(CBuffer &parms) const
	{	

		return 0;
	}

	static const CEntPlayerStartMaker registerThis;
};
*/


