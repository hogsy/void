
#ifndef PORTAL_H
#define PORTAL_H

#include "bsp.h"

#define MAX_MAP_PORTALS		65536


typedef struct portal_s
{
	// two sides
	bsp_node_t			*nodes[2];
	struct portal_s		*next[2];	// next portal in chains for each side
	bsp_brush_side_t	*side;	// the area that is touching - faces nodes[0]
} portal_t;


extern portal_t portals[MAX_MAP_PORTALS];
extern int num_portals;

void portal_run(bsp_node_t *head);
void portal_file(char *name);

#endif
