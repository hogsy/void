#ifndef REN_CACHE_H
#define REN_CACHE_H

#include "3dmath.h"

typedef struct cpoly_s
{
	poly_t	poly;
	struct	cpoly_s *next;
} cpoly_t;


void cache_add_poly(cpoly_t *poly);
void cache_destroy(void);
void cache_purge(void);
cpoly_t* get_poly(void);
void return_poly(cpoly_t *p);

#endif

