#include "Game_ents.h"

//======================================================================================
//======================================================================================







class CEntClientMaker: public CEntityMaker
{
public:
	CEntClientMaker() : CEntityMaker("client")	{}

	Entity * MakeEntity(CBuffer &parms) const
	{
		return 0;
	}

	static const CEntClientMaker registerThis;
};




class CEntSpeakerMaker: public CEntityMaker
{
public:
	CEntSpeakerMaker() : CEntityMaker("ent_speaker")	{}

	Entity * MakeEntity(CBuffer &parms) const
	{
		return 0;
	}

	static const CEntSpeakerMaker registerThis;
};

