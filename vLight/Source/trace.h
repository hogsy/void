
#ifndef TRACE_H
#define TRACE_H


#include "3dmath.h"


typedef struct
{
	vector_t	endpos;		// where the trace ended
	float		fraction;	// fraction of trace completed
} trace_t;


trace_t trace(vector_t *start, vector_t *end);

#endif

