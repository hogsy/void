#ifndef VOID_COM_3DSTRUCTS
#define VOID_COM_3DSTRUCTS

#include "Com_vector.h"

typedef float matrix_t[4][4];


typedef struct poly_t
{

	int			num_vertices;
	vector_t	vertices[32];

	int			texdef;
	int			lightdef;

} poly_t;


#endif