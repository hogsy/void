#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "../Std_lib.h"
#include "Bsp.h"
#include "map_file.h"
#include "bsp.h"
#include "entity.h"
#include "portal.h"


bool	g_bFastBSP=false;


void cam_calc_paths(entity_t *ent);
void write_bsp(entity_t *ents, char *file);

//==========================================================================
//==========================================================================
/*
void print_brushes(bsp_brush_t *b)
{
	for (int i=0; b; b=b->next, i++)
	{
		ComPrintf("\nbrush %d\n", i);

		ComPrintf("mins(%4d,%4d,%4d)\n", b->mins[0], b->mins[1], b->mins[2]);
		ComPrintf("maxs(%4d,%4d,%4d)\n", b->maxs[0], b->maxs[1], b->maxs[2]);

		for (bsp_brush_side_t *s=b->sides; s; s=s->next)
		{
			ComPrintf("plane %2d\t", s->plane);

			for (int v=0; v<s->num_verts; v++)
				ComPrintf("(%4f,%4f,%4f) ", s->verts[v].x, s->verts[v].y, s->verts[v].z);

			ComPrintf("\n");
		}
	}
}
*/

//==========================================================================
//==========================================================================

void CompileBsp(const char * szFileName)
{
	ComPrintf ("\n\n======== vbsp ========\n");

	reset_bsp_brush();

	if (!load_map(szFileName))
	{
		Error("CompileBsp: Couldn't find map file %s!", szFileName);
		return;
	}

	// process all entities, including the world
	entity_t *nent, *ents = NULL;
	for (int e=0; e<num_map_entities; e++)
	{
		nent = entity_process(e);
		nent->next = ents;
		ents = nent;
	}

	// make sure that worldspawn is the first entity in the list
	entity_t *prev = NULL, *walk = ents;
	while (walk)
	{
		char *val = key_get_value(walk, "classname");
		if (strcmp(val, "worldspawn") == 0)
		{
			if (prev)
			{
				prev->next = walk->next;
				walk->next = ents;
				ents = walk;
			}
			break;
		}

		prev = walk;
		walk = walk->next;
	}
	
	if (!walk)
	{
		Error("CompileBsp: no worldspawn found!!");
		return;
	}

	cam_calc_paths(ents);

	// write the bsp file
	char newFileName[_MAX_PATH];
	strcpy(newFileName, szFileName);

	strcpy(&(newFileName)[strlen(newFileName)-3], "wld");
	write_bsp(ents, newFileName);

	// write the portal file
	strcpy(&(newFileName)[strlen(newFileName)-3], "prt");
	portal_file(newFileName);
}





