#ifndef REN_CACHE_H
#define REN_CACHE_H

#include "3dmath.h"

/*
typedef struct cpoly_s
{
	poly_t	poly;
	struct	cpoly_s *next;
} cpoly_t;
*/

struct cpoly_t
{
	cpoly_t() { next = 0; }

	poly_t	poly;
	cpoly_t *next;
};

void cache_add_poly(cpoly_t *poly);
void cache_destroy(void);
void cache_purge(void);
cpoly_t* get_poly(void);
void return_poly(cpoly_t *p);

#endif

