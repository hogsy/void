

#include "std_lib.h"
#include "entity.h"
#include "string.h"
#include "csg.h"
#include "portal.h"
#include "vbsp.h"


entity_t *ent = NULL;

/*
==============
allocate an entity
==============
*/
entity_t fents[MAX_MAP_ENTITIES];
int		 num_fents = 0;
entity_t* get_ent(void)
{
	if (num_fents == MAX_MAP_ENTITIES)
		Error("too many entities!");

	num_fents++;
	return &fents[num_fents-1];
}



/*
==============
allocate a key
==============
*/
vkey_t fkeys[MAX_MAP_KEYS];
int		 num_fkeys = 0;
vkey_t* get_key(void)
{
	if (num_fkeys == MAX_MAP_KEYS)
		Error("too many keys!");

	num_fkeys++;
	return &fkeys[num_fkeys-1];
}



/*
================
entity_process
================
*/
void build_bsp_brushes(entity_t *ent);
entity_t* entity_process(int e)
{

	entity_t *ent = get_ent();

	ent->map_ent = e;
	ent->next = NULL;
	ent->key = NULL;
	ent->root = NULL;

	// copy all our keys
	for (int k=map_entities[e].first_key; k<(map_entities[e].first_key+map_entities[e].num_keys); k++)
	{
		vkey_t *nkey = get_key();
		strcpy(nkey->k.name, keys[k].name);
		strcpy(nkey->k.value, keys[k].value);

		nkey->next = ent->key;
		ent->key = nkey;
	}


	build_bsp_brushes(ent);

	v_printf("running csg: ");
	ent->root->brushes = run_csg(ent->root->brushes);
	v_printf("OK\n");

	v_printf("partitioning: ");
	bsp_partition(ent->root);
	v_printf("OK\n");

	// only create portals for the map
	if (!fast && (strcmp(key_get_value(ent, "classname"), "worldspawn") == 0))
	{
		v_printf("creating portals: ");
		portal_run(ent->root);
		v_printf("OK\n");
	}

	return ent;
}



//===========================================================================================
// functions for accessing key/value pairs
//===========================================================================================

char* key_get_value(entity_t *ent, char *name)
{

	for (vkey_t *key=ent->key; key; key=key->next)
	{
		if (strcmp(key->k.name, name) == 0)
			return key->k.value;
	}

	return "";
}


float key_get_float(entity_t *ent, char *name)
{
	char *val = key_get_value(ent, name);
	return (float)atof(val);
}

void key_get_vector(entity_t *ent, char *name, vector_t &v)
{
	char *val = key_get_value(ent, name);
	sscanf(val, "%f %f %f", &v.x, &v.y, &v.z);
}
