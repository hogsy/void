#if 0
#include "World.h"


#define MAX_CLASSNAME 32


/*
======================================
the base entitiy class
every entity in the game will be derived from this
======================================
*/


class CEntBase
{
public:
	char classname[MAX_CLASSNAME];

	unsigned int sector_index;		// which sector the entitiy is in
//	sector_t *sector;

	vector_t origin;
	vector_t angles;

//	virtual void Think();			//every server frame
};


/*
======================================
Speaker - play sounds
======================================
*/

class CEnt_Speaker : public CEntBase
{
public:
	char	sound[16];
	int		type;
	int		attenuation;
};

#endif

