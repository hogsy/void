#ifndef REN_CACHE_H
#define REN_CACHE_H

//#include "3dmath.h"
#include "Com_vector.h"
#include "Com_3dstructs.h"

#define CACHE_PASS_SKY			0
#define	CACHE_PASS_ZFILL		1
#define CACHE_PASS_ZBUFFER		2
#define	CACHE_PASS_ALPHABLEND	3
#define CACHE_PASS_NUM			4


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

