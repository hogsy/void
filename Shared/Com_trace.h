#ifndef COM_VOID_WORLD_TRACE
#define COM_VOID_WORLD_TRACE

#include "Com_vector.h"


// content flags
#define	CONTENTS_SOLID			0x00000001		// an eye is never valid in a solid
#define	CONTENTS_SKY			0x00000002		// is a sky brush - moves with eye
#define CONTENTS_SKYVIEW		0x00000004		// allows viewing of sky
#define CONTENTS_TRANSPARENT	0x00000008		// partially visible
#define CONTENTS_INVISIBLE		(0x00000010 | CONTENTS_TRANSPARENT)		// completely invisible



#define	CONTENTS_LAVA			0x00000020
#define	CONTENTS_WATER			0x00000040

// surface flags
#define SURF_INVISIBLE	0x00000001	// not drawn
#define SURF_SKYVIEW	0x00000002	// implies CONTENTS_SKYVIEW



/*
#define	CONTENTS_AUX			0x00000004
#define CONTENTS_SKY			0x00000004		// my sky contents is same sas q2 AUX
#define	CONTENTS_LAVA			0x00000008
#define	CONTENTS_SLIME			0x00000010
#define	CONTENTS_WATER			0x00000020
#define	CONTENTS_MIST			0x00000040
#define	CONTENTS_FOG			0x00000040
#define	LAST_VISIBLE_CONTENTS	0x00000040

// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_AREAPORTAL		0x00008000
#define	CONTENTS_PLAYERCLIP		0x00010000
#define	CONTENTS_MONSTERCLIP	0x00020000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x00040000
#define	CONTENTS_CURRENT_90		0x00080000
#define	CONTENTS_CURRENT_180	0x00100000
#define	CONTENTS_CURRENT_270	0x00200000
#define	CONTENTS_CURRENT_UP		0x00400000
#define	CONTENTS_CURRENT_DOWN	0x00800000

#define	CONTENTS_ORIGIN			0x01000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x02000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x04000000
#define	CONTENTS_DETAIL			0x08000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000
*/



/*
============================================================================
keeps info about trace operations within the world
============================================================================
*/
struct TraceInfo
{
	TraceInfo() : fraction(0.0f), plane(0) { }
	~TraceInfo() { plane = 0; }

	vector_t	endpos;		// where the trace ended
	float		fraction;	// fraction of trace completed
	plane_t	  * plane;
};


/*
================================================
The game dll and all don't need to be aware of the
world definitions. All they need is this interface to
do all the world interfaction
================================================
*/
struct I_World
{
	virtual int  PointContents(const vector_t &v)=0;
	virtual void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end)=0;
	virtual void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end, 
					   const vector_t &mins, const vector_t &maxs)=0;
};


#endif