
#ifndef MDL_CACHE_H
#define MDL_CACHE_H


#include "Standard.h"

typedef struct drawmodel_s
{
	vector_t	origin;
	vector_t	angles;
	CacheType	cache;
	int			index;
	int			skin;
	float		frame;

	drawmodel_s *next;
} drawmodel_t;

/*
void model_cache_purge(void);
void model_cache_add(void);
void model_cache_init(void);
void model_cache_destroy(void);
*/
#endif




