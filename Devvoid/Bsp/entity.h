#ifndef ENTITY_H
#define ENTITY_H

#include "bsp.h"
#include "map_file.h"


typedef struct vkey_s
{
	key_t	k;
	struct vkey_s	*next;
} vkey_t;



typedef struct entity_s
{
	bsp_node_t		*root;
	vkey_t			*key;

	int		map_ent;	// reference to entity # from .map file
	struct	entity_s *next;
} entity_t;


entity_t* entity_process(int e);
char* key_get_value(entity_t *ent, char *name);
void key_get_vector(entity_t *ent, char *name, vector_t &v);
float key_get_float(entity_t *ent, char *name);
vkey_t* get_key(void);

#endif


