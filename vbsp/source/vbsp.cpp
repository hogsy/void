/***************************************************

	build a bsp file out of a .map file

***************************************************/

#include <string.h>
#include <time.h>
#include "std_lib.h"
#include "vbsp.h"
#include "map_file.h"
#include "bsp.h"
#include "entity.h"
#include "cam_path.h"
#include "portal.h"


// required for filesystem
#include "Com_defs.h"
#include "I_file.h"
#include "I_filesystem.h"


// fast mode doesn't remove any non-visible faces, and doesn't calculate portals
bool fast = false;

void write_bsp(entity_t *ents, char *file);


void print_brushes(bsp_brush_t *b)
{
	for (int i=0; b; b=b->next, i++)
	{
		v_printf("\nbrush %d\n", i);

		v_printf("mins(%4d,%4d,%4d)\n", b->mins[0], b->mins[1], b->mins[2]);
		v_printf("maxs(%4d,%4d,%4d)\n", b->maxs[0], b->maxs[1], b->maxs[2]);

		for (bsp_brush_side_t *s=b->sides; s; s=s->next)
		{
			v_printf("plane %2d\t", s->plane);

			for (int v=0; v<s->num_verts; v++)
				v_printf("(%4f,%4f,%4f) ", s->verts[v].x, s->verts[v].y, s->verts[v].z);

			v_printf("\n");
		}
	}
}


/*
============
main
============
*/
int main (int argc, char **argv)
{
	FILESYSTEM_Create(&v_printf, &VFSError, "E:\\Void", "Game");
//	g_pShaders = new CShaderManager();

	printf ("======== vbsp ========\n");

	long start_time;
	long end_time;
	time(&start_time);

	for (int i=1; i<argc; i++)
	{
		if (strcmp(argv[i], "-v") == 0)
			verbose = true;
		else if (strcmp(argv[i], "-fast") == 0)
			fast = true;
		else if (!strcmp(argv[i], "-log"))
		{
			i++;
			flog = fopen(argv[i], "w");
			if (!flog)
				Error("couldn't open log file!");
		}
		else if (argv[i][0] == '-')
			Error ("Unknown command line option \"%s\"", argv[i]);
		else
			break;
	}


	if (i != argc - 1)
		Error ("usage: vbsp [options] mapfile");


	if (!load_map(argv[i]))
		Error("couldn't find map file %s!", argv[i]);


	reset_bsp_brush();

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
		Error("no worldspawn found!!");

	cam_calc_paths(ents);


	// write the bsp file
	strcpy(&(argv[i])[strlen(argv[i])-3], "wld");
	write_bsp(ents, argv[i]);

	// write the portal file
	strcpy(&(argv[i])[strlen(argv[i])-3], "prt");
	portal_file(argv[i]);


	// log time
	time(&end_time);
	long seconds = end_time-start_time;
	long minutes = seconds/60;
	seconds %= 60;
	long hours = minutes / 60;
	minutes %= 60;

	v_printf("time elapsed - %2d:%2d:%2d (%d seconds)\n", hours, minutes, seconds, end_time-start_time);


	// close up log file
	if (flog)
		fclose(flog);

//	delete g_pShaders;
	FILESYSTEM_Free();


	return 0;
}





