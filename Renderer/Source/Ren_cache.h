#ifndef REN_CACHE_H
#define REN_CACHE_H

#include "3dmath.h"

#define	CACHE_PASS_ZFILL		0
#define CACHE_PASS_ZBUFFER		1
#define	CACHE_PASS_ALPHABLEND	2
#define CACHE_PASS_NUM			3


struct cpoly_t
{
	poly_t	poly;
	cpoly_t *next;
};

void cache_add_poly(cpoly_t *poly, int cache);
void cache_destroy(void);
void cache_purge(void);
cpoly_t* get_poly(void);
void return_poly(cpoly_t *p);

#endif

