
#ifndef TRACE_H
#define TRACE_H


#include "Com_vector.h"


typedef struct
{
	vector_t	endpos;		// where the trace ended
	float		fraction;	// fraction of trace completed
	plane_t		*plane;
} trace_t;


trace_t trace(vector_t &start, vector_t &end);

#endif

