#ifndef CL_COLLISION
#define CL_COLLISION

#include "3dmath.h"
//#include "World.h"
#include "clip.h"


/*

typedef struct
{
//	float		dist;	// distance moved
	int			side;	// side hit
	vector_t	end;	// where the trace ended (intersection or full dist 
} rayhit_t;

*/



typedef struct
{
	vector_t	endpos;		// where the trace ended
	float		fraction;	// fraction of trace completed
	plane_t		*plane;
} trace_t;



trace_t trace(vector_t &start, vector_t &end, vector_t *mins, vector_t *maxs);


#endif