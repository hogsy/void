#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "../Std_lib.h"
#include "bsp.h"
#include "portal.h"


portal_t portals[MAX_MAP_PORTALS];
int num_portals=0;


/*
============
get_portal
============
*/
portal_t* get_portal(void)
{
	if (num_portals == MAX_MAP_PORTALS)
		Error("too many portals!");

	num_portals++;
	return &portals[num_portals-1];
}


/*
============
portal_remove_nodes
============
*/
void portal_remove_nodes(portal_t *portal)
{
	portal_t *next, *p;
	portal_t *prev = NULL;
	bsp_node_t *n1 = portal->nodes[0];
	bsp_node_t *n2 = portal->nodes[1];

	for (p=n1->portals; p; prev=p, p=next)
	{
		if (n1 == p->nodes[0])
			next = p->next[0];
		else
			next = p->next[1];


		if (p == portal)
		{
			// remove this portal
			if (!prev)
				n1->portals = next;
			else
			{
				if (n1 == prev->nodes[0])
					prev->next[0] = next;
				else
					prev->next[1] = next;
			}

			break;
		}
	}

	if (!p)
		Error("portal_remove_nodes: Node not liked to portal");

	prev = NULL;
	for (p=n2->portals; p; prev=p, p=next)
	{
		if (n2 == p->nodes[0])
			next = p->next[0];
		else
			next = p->next[1];


		if (p == portal)
		{
			// remove this portal
			if (!prev)
				n2->portals = next;
			else
			{
				if (n2 == prev->nodes[0])
					prev->next[0] = next;
				else
					prev->next[1] = next;
			}

			break;
		}
	}

	if (!p)
		Error("portal_remove_nodes: Node not liked to portal");

	portal->nodes[0] = NULL;
	portal->nodes[1] = NULL;
}

/*
============
portal_add_nodes
============
*/
float side_area(bsp_brush_side_t *si);
void portal_add_nodes(portal_t *portal, bsp_node_t *n1, bsp_node_t *n2)
{
	// only add if the portal side isnt 'tiny'
	if (side_area(portal->side) < 1.0f)
	{
		ComPrintf("removing portal\n");
		return;
	}


	portal->nodes[0] = n1;
	portal->next[0] = n1->portals;
	n1->portals = portal;

	portal->nodes[1] = n2;
	portal->next[1] = n2->portals;
	n2->portals = portal;
}


/*
===========
next_leaf - find the next leaf from the current node/leaf
===========
*/
bsp_node_t* next_leaf(bsp_node_t *n)
{
	if (n->plane != -1)
	{
		while(n->plane != -1)
			n = n->children[0];
		return n;
	}

// walk up till we can cross over
	while (1)
	{
		if (!n->parent)
			return NULL;

		if (n == n->parent->children[0])
		{
			n = n->parent->children[1];
			break;
		}
		
		n = n->parent;
	}


// follow down towards the front

	while(n->plane != -1)
		n = n->children[0];

	return n;
}



/*
===========
portal_parent_clip - clip portal to all parents
===========
*/
void portal_parent_clip(bsp_node_t *n, bsp_brush_side_t *s)
{
	if (!n->parent)
		return;

	// clip away the part on the other side of the node
	if (n == n->parent->children[0])
		clip_side(s, n->parent->plane^1);
	else
		clip_side(s, n->parent->plane);

	portal_parent_clip(n->parent, s);
}


/*
===========
portal_split_node
===========
*/
void portal_split_node(bsp_node_t *n)
{

	// split all portals that link here

	bsp_node_t *otherside;
	bool		other_is_back;
	bsp_brush_side_t *front, *back;

	portal_t *next, *p;
	for (p=n->portals; p; p=next)
	{
		if (n == p->nodes[0])
		{
			otherside = p->nodes[1];
			other_is_back = true;
			next = p->next[0];
		}
		else
		{
			otherside = p->nodes[0];
			other_is_back = false;
			next = p->next[1];
		}


		portal_remove_nodes(p);

		// create 2 new portals with out node plane

		front = p->side;
		back  = new_bsp_brush_side();
		memcpy(back, front, sizeof(bsp_brush_side_t));

		clip_side(front, n->plane^1);
		clip_side(back,  n->plane);


		// portal all on backside
		if (front->num_verts < 3)
		{
			free_bsp_brush_side(front);
			p->side = back;
			if (other_is_back)
				portal_add_nodes(p, n->children[1], otherside);
			else
				portal_add_nodes(p, otherside, n->children[1]);
			continue;
		}

		// portal all  on frontside
		if (back->num_verts < 3)
		{
			free_bsp_brush_side(back);
			p->side = front;
			if (other_is_back)
				portal_add_nodes(p, n->children[0], otherside);
			else
				portal_add_nodes(p, otherside, n->children[0]);
			continue;
		}

		// else it was split in two

		portal_t *p2 = get_portal();

		p->side = front;
		p2->side = back;

		if (other_is_back)
		{
			portal_add_nodes(p,  n->children[0], otherside);
			portal_add_nodes(p2, n->children[1], otherside);
		}
		else
		{
			portal_add_nodes(p,  otherside, n->children[0]);
			portal_add_nodes(p2, otherside, n->children[1]);
		}
	}

	n->portals = NULL;
}


/*
===========
portal_create_node - create all portals for this node
===========
*/
void portal_create_node(bsp_node_t *node)
{
	portal_t *baseportal = get_portal();
	baseportal->side = new_bsp_brush_side();
	baseportal->side->plane = node->plane;
	make_base_side(baseportal->side);

	portal_add_nodes(baseportal, node->children[0], node->children[1]);

	// clip to all parent nodes
	portal_parent_clip(node, baseportal->side);
	if (baseportal->side->num_verts < 3)
		Error("base < 3\n");

	// split all portals that link to here
	// after this, no portals will be able to link here

	portal_split_node(node);
}


/*
===========
portal_create_r - creates all portals connecting leafs
===========
*/
void portal_create_r(bsp_node_t *head)
{
	if (head->plane == -1)
		return;

	portal_create_node(head);


	portal_create_r(head->children[0]);
	portal_create_r(head->children[1]);
}


/*
==========
portal_mark_outside_leafs - mark "big" volumes
==========
*/
void portal_mark_outside_leafs(bsp_node_t *head)
{
	if (head->plane == -1)
	{
		for (int i=0; i<3; i++)
		{
			if ((head->volume->mins[i] < -9000) ||
				(head->volume->maxs[i] >  9000))
			{
				head->outside = true;
				head->contents = CONTENTS_SOLID;
				break;
			}
		}

		if (i==3)
			head = head;
	}

	else
	{
		portal_mark_outside_leafs(head->children[0]);
		portal_mark_outside_leafs(head->children[1]);
	}
}


/*
==========
portal_flood_outside
==========
*/
void portal_flood_outside_r(bsp_node_t *leaf)
{
	portal_t *p;
	int s;	// side of this portal our leaf is on
	for (p=leaf->portals; p; p=p->next[s])
	{
		if (leaf == p->nodes[0])
			s=0;
		else
			s=1;

		// if the other side isn't solid, we can flood into it
		// skip anything that's already marked outside
		if ((p->nodes[1-s]->contents & CONTENTS_SOLID) ||
			(p->nodes[1-s]->outside))
			continue;

		p->nodes[1-s]->outside = true;
		p->nodes[1-s]->contents = CONTENTS_SOLID;
		portal_flood_outside_r(p->nodes[1-s]);
	}
}


void portal_flood_outside(bsp_node_t *head)
{
	if (head->plane == -1)
	{
		if (head->outside)
			portal_flood_outside_r(head);
	}
	else
	{
		portal_flood_outside(head->children[0]);
		portal_flood_outside(head->children[1]);
	}
}


/*
==========
portal_mark_visible_sides
==========
*/
void portal_mark_visible_sides(bsp_node_t *head, int &vis, int &num)
{

	if (head->plane == -1)
	{
		if (!head->brushes)
			return;

		// set all brush sides to not visible
		bsp_brush_side_t *s;
		for (s=head->brushes->sides; s; s=s->next)
		{
			num++;
			s->visible = false;
		}

		int side;	// side of this portal our leaf is on
		portal_t *p;
		for (p=head->portals; p; p=p->next[side])
		{
			if (head == p->nodes[0])
				side=0;
			else
				side=1;

			if (p->nodes[1-side]->outside)
				continue;
			if (p->nodes[1-side]->contents&CONTENTS_SOLID)
				continue;

			// we can see through - find the side the portal lies on and mark visible
			for (s=head->brushes->sides; s; s=s->next)
			{
				if (s->visible)
					continue;
				if ((s->plane|1) != (p->side->plane|1))
					continue;

				vis++;
				s->visible = true;
				break;
			}
		}
	}

	else
	{
		portal_mark_visible_sides(head->children[0], vis, num);
		portal_mark_visible_sides(head->children[1], vis, num);
	}
}


/*
==========
portal_remove_invis - remove invisible sides
==========
*/
void portal_remove_invis(bsp_node_t *head)
{
	if (head->plane == -1)
	{
		if (!head->brushes)
			return;


		bsp_brush_side_t *s, *prev=NULL;
		for (s=head->brushes->sides; s; )
		{
			if (s->visible)
			{
				prev = s;
				s=s->next;
				continue;
			}

			// remove this side
			if (!prev)
			{
				head->brushes->sides = head->brushes->sides->next;
				free_bsp_brush_side(s);
				s = head->brushes->sides;
			}

			else
			{
				s = s->next;
				free_bsp_brush_side(prev->next);
				prev->next = s;
			}
		}

		if (!head->brushes->sides)
		{
			free_bsp_brush(head->brushes);
			head->brushes = NULL;
		}
	}

	else
	{
		portal_remove_invis(head->children[0]);
		portal_remove_invis(head->children[1]);
	}
}


/*
==========
portal_file
==========
*/
void portal_file(char *name)
{
	int num=0;

	FILE *f = fopen(name, "w");
	fprintf(f, "%s\n", "VoidPortalFile");
	for (int p=0; p<num_portals; p++)
	{
		// dont print any that link to outside leafs or are between two solids
		if (portals[p].nodes[0]->outside || portals[p].nodes[1]->outside)
			continue;

		if ((portals[p].nodes[0]->contents&CONTENTS_SOLID) &&
			(portals[p].nodes[1]->contents&CONTENTS_SOLID))
			continue;

		if (!portals[p].nodes[0] || !portals[p].nodes[1])
			continue;


		num++;

		fprintf(f, "%d %d %d %d %d",	portals[p].nodes[0]->fleaf,
										portals[p].nodes[1]->fleaf,
										portals[p].side->plane,
										portals[p].side->plane^1,
										portals[p].side->num_verts);

		for (int v=0; v<portals[p].side->num_verts; v++)
			fprintf(f, " ( %f %f %f)",	portals[p].side->verts[v].x,
										portals[p].side->verts[v].y,
										portals[p].side->verts[v].z);

		fprintf(f, "\n");

	}

	fclose(f);

	ComPrintf("%d portals filed - OK\n", num);
}


/*
==========
portal_run - do all portal related processing
==========
*/
void portal_run(bsp_node_t *head)
{
	portal_create_r(head);
	ComPrintf("%d portals\n", num_portals);

	// find and get rid of all outside nodes
	portal_mark_outside_leafs(head);
	portal_flood_outside(head);


	int out=0, in=0, solid=0;
	bsp_node_t *leaf;
	for (leaf=next_leaf(head); leaf; leaf=next_leaf(leaf))
	{
		if (leaf->outside)
			out++;
		else if (leaf->contents&CONTENTS_SOLID)
			solid++;
		else
			in++;
	}
	ComPrintf("%d outside, %d inside, %d solid\n", out, in, solid);


	// only remove invisible sides if there is a leak
	if (in)
	{
		int vis, num;
		vis = num = 0;
		portal_mark_visible_sides(head, vis, num);
		ComPrintf("%d of %d sides visible\n", vis, num);
		portal_remove_invis(head);		// need all portals here
	}

//	portal_print_list(head);		// find the portals to print, cant until we have leafs numbered (when bsp written)
}








