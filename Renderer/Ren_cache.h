#ifndef REN_CACHE_H
#define REN_CACHE_H

#define CACHE_PASS_SKY			0
#define	CACHE_PASS_ZFILL		1
#define CACHE_PASS_ZBUFFER		2
#define	CACHE_PASS_ALPHABLEND	3
#define CACHE_PASS_NUM			4


struct cpoly_t
{
	int			num_vertices;
	vector_t	vertices[32];

	int			texdef;
	int			lightdef;

	cpoly_t *next;
};

void cache_add_poly(cpoly_t *poly, int cache);
void cache_destroy(void);
void cache_purge(void);
cpoly_t* get_poly(void);
void return_poly(cpoly_t *p);

#endif

