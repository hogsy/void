
#include "std_lib.h"
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
brushes_touching
============
*/
bsp_brush_side_t* brushes_touching(bsp_brush_t *b1, bsp_brush_t *b2)
{
	bsp_brush_side_t *s1, *s2, *tmp;
	for (s1=b1->sides; s1; s1=s1->next)
	{
		for (s2=b2->sides; s2; s2=s2->next)
		{
			if (s1->plane == (s2->plane^1))
			{
				// we have an opposing plane - make sure the verts are overlapping
				// create a new side withing the plane and clip it to all other planes
				// of the brushes.  if there is anything left, it overlaps
				bsp_brush_side_t *ns = new_bsp_brush_side();
				ns->plane = s1->plane;
				make_base_side(ns);

				for (tmp=b1->sides; tmp; tmp=tmp->next)
				{
					if ((tmp->plane|1) != (ns->plane|1))
					{
						clip_side(ns, tmp->plane);
						if (ns->num_verts<3)
						{
							free_bsp_brush_side(ns);
							goto nomatch;
						}
					}
				}

				for (tmp=b2->sides; tmp; tmp=tmp->next)
				{
					if ((tmp->plane|1) != (ns->plane|1))
					{
						clip_side(ns, tmp->plane);
						if (ns->num_verts<3)
						{
							free_bsp_brush_side(ns);
							goto nomatch;
						}
					}
				}

				// we got here - there are still verts in ns
				return ns;
			}
nomatch:;

		}
	}

	return NULL;
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
portal_create - creates all portals connecting leafs
===========
*/
void portal_create(bsp_node_t *head)
{
	int num=0;
	bsp_node_t *n1, *n2;
	for (n1=next_leaf(head); n1; n1=next_leaf(n1))
	{ 
		if (n1->plane != -1)
			continue;

		for (n2=next_leaf(n1); n2; n2=next_leaf(n2))
		{
			if (n2->plane != -1)
				continue;

			// see if the volumes of these nodes touch
			bsp_brush_side_t *touch = brushes_touching(n1->volume, n2->volume);

			if (touch)
			{
				num++;
				portal_t *p = get_portal();
				p->side = touch;
				p->nodes[0] = n1;
				p->nodes[1] = n2;
				p->next[0] = n1->portals;
				p->next[1] = n2->portals;
				n1->portals = p;
				n2->portals = p;
			}
		}
	}

	v_printf("%d portals.  ", num);
}


/*
==========
portal_mark_outside_leafs - mark "big" volumes
==========
*/
void portal_mark_outside_leafs(bsp_node_t *head)
{
	for (head=next_leaf(head); head; head=next_leaf(head))
	{
		for (int i=0; i<3; i++)
		{
			if ((head->volume->mins[i] < -9000) ||
				(head->volume->maxs[i] >  9000))
			{
				head->outside = true;
				head->contents = CONTENTS_SOLID;
			}
		}
	}
}


/*
==========
portal_flood_outside
==========
*/
void portal_flood_outside_r(bsp_node_t *leaf)
{
	if (leaf->contents & CONTENTS_SOLID)
		return;

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
	bsp_node_t *leaf;
	for (leaf=next_leaf(head); leaf; leaf=next_leaf(leaf))
	{
		if (!leaf->outside)
			continue;

		// reach everything we can from this leaf
		portal_flood_outside_r(leaf);
	}
}


/*
==========
portal_mark_visible_sides
==========
*/
void portal_mark_visible_sides(bsp_node_t *head)
{
	int vis=0, num=0;

	bsp_node_t *leaf;
	for (leaf=next_leaf(head); leaf; leaf=next_leaf(leaf))
	{
		if (!leaf->brushes)
			continue;

		// set all brush sides to not visible
		bsp_brush_side_t *s;
		for (s=leaf->brushes->sides; s; s=s->next)
		{
			num++;
			s->visible = false;
		}

		int side;	// side of this portal our leaf is on
		portal_t *p;
		int iter = 0;
		for (p=leaf->portals; p; p=p->next[side])
		{
			iter++;

			if (leaf == p->nodes[0])
				side=0;
			else
				side=1;

			if (p->nodes[1-side]->outside)
				continue;
			if (p->nodes[1-side]->contents&CONTENTS_SOLID)
				continue;

			// we can see through - find the side the portal lies on and mark visible
			for (s=leaf->brushes->sides; s; s=s->next)
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
		iter;
	}

	v_printf("%d of %d sides visible\n", vis, num);
}


/*
==========
portal_remove_invis - remove invisible sides
==========
*/
void portal_remove_invis(bsp_node_t *head)
{
	bsp_node_t *leaf;
	for (leaf=next_leaf(head); leaf; leaf=next_leaf(leaf))
	{
		if (!leaf->brushes)
			continue;


		bsp_brush_side_t *s, *prev=NULL;
		for (s=leaf->brushes->sides; s; )
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
				leaf->brushes->sides = leaf->brushes->sides->next;
				free_bsp_brush_side(s);
				s = leaf->brushes->sides;
			}

			else
			{
				s = s->next;
				free_bsp_brush_side(prev->next);
				prev->next = s;
			}
		}

		if (!leaf->brushes->sides)
		{
			free_bsp_brush(leaf->brushes);
			leaf->brushes = NULL;
		}
	}
}


/*
==========
portal_remove_outside - get rid of as many outside nodes as possible
==========
*/
bsp_node_t outside_node;

void portal_remove_outside_r(bsp_node_t *node)
{
	if (node->plane == -1)
		return;

	for (int i=0; i<2; i++)
	{
		portal_remove_outside_r(node->children[i]);

	// outside leaf is always the same
	// nodes that are outside are leaked - not a big deal
		if (node->children[i]->outside)
		{
			// free brushes/volume
			if (node->children[i]->brushes)
				free_bsp_brush(node->children[i]->brushes);
			free_bsp_brush(node->children[i]->volume);

			node->children[i] = &outside_node;
		}
	}

	// if both are outside, so is parent
	if (node->children[0] == node->children[1])
	{
		node->outside = true;
		node->plane = -1;
	}
}

void portal_remove_outside(bsp_node_t *head)
{
// FIXME - make leafs without any brushes but solid outside nodes??

	outside_node.brushes		= NULL;
	outside_node.children[0]	= NULL;
	outside_node.children[1]	= NULL;
	outside_node.contents		= CONTENTS_SOLID;
	outside_node.outside		= true;
	outside_node.parent			= NULL;
	outside_node.plane			= -1;
	outside_node.portals		= NULL;
	outside_node.volume			= bsp_build_volume();

	portal_remove_outside_r(head);

	if (head->outside)
		v_printf("WARNING: everything outside\n");
}


/*
==========
portal_remove - remove the portal from it's two leaf lists.  portal is leaked - not a big deal
==========
*/
void portal_remove(portal_t *p)
{
	portal_t *walk, *prev, *next;
	bsp_node_t *leaf;

	for (int i=0; i<2; i++)
	{
		prev = NULL;
		leaf = p->nodes[i];
		walk = p;

		while (1)
		{
			// find next
			if (walk->nodes[0] == leaf)
				next = walk->next[0];
			else
				next = walk->next[1];

			// we found it
			if (walk == p)
				break;

			// move along
			prev = walk;
			walk = next;
		}

		// remove it from the list
		if (!prev)
			leaf->portals = next;
		else
		{
			if (leaf == prev->nodes[0])
				prev->next[0] = next;
			else
				prev->next[1] = next;
		}
	}

	free_bsp_brush_side(p->side);
}


/*
==========
portal_remove_unwanted - remove all portals that have a solid on both sides
==========
*/
void portal_remove_unwanted(bsp_node_t *head)
{
	bsp_node_t *leaf;
	for (leaf=next_leaf(head); leaf; leaf=next_leaf(leaf))
	{

		if (!(leaf->contents&CONTENTS_SOLID))
		{
			if (leaf->outside)
				v_printf("oops\n");
			continue;
		}

		// remove all portals to this leaf
		portal_t *p, *pnext;
		for (p=leaf->portals; p; p=pnext)
		{
			if (leaf == p->nodes[0])
				pnext = p->next[0];
			else
				pnext = p->next[1];

			if ((p->nodes[0]->contents&CONTENTS_SOLID) &&
				(p->nodes[1]->contents&CONTENTS_SOLID))
				portal_remove(p);
			else
				if ((p->nodes[0]->outside) || (p->nodes[1]->outside))
					v_printf("oops2\n");
		}
	}
}


/*
==========
portal_print_list
==========
*/
portal_t* pportals[MAX_MAP_PORTALS];
int		  npportals = 0;
void portal_add(portal_t *port)
{
	for (int p=0; p<npportals; p++)
	{
		// we're already going to print this one
		if (pportals[p] == port)
			return;
	}
	
	// add a new one to the list
	if (npportals == MAX_MAP_PORTALS)
		Error("too many portals to print");

	pportals[npportals] = port;
	npportals++;
}

void portal_print_list(bsp_node_t *head)
{
	bsp_node_t *leaf;
	for (leaf=next_leaf(head); leaf; leaf=next_leaf(leaf))
	{
		portal_t *p;
		for (p=leaf->portals; p; )
		{
			portal_add(p);
			
			if (leaf == p->nodes[0])
				p = p->next[0];
			else
				p = p->next[1];
		}
	}
}


/*
==========
portal_file
==========
*/
void portal_file(char *name)
{
	if (!npportals)
		return;

	int num=0;

	v_printf("writing portal file: ");
	FILE *f = fopen(name, "w");
	fprintf(f, "%s\n", "VoidPortalFile");
	for (int p=0; p<npportals; p++)
	{
		// dont print any that link to outside leafs or are between two solids
		if (pportals[p]->nodes[0]->outside || pportals[p]->nodes[1]->outside)
			continue;

		if ((pportals[p]->nodes[0]->contents&CONTENTS_SOLID) &&
			(pportals[p]->nodes[1]->contents&CONTENTS_SOLID))
			continue;

		num++;

		fprintf(f, "%d %d %d %d %d",	pportals[p]->nodes[0]->fleaf,
										pportals[p]->nodes[1]->fleaf,
										pportals[p]->side->plane,
										pportals[p]->side->plane^1,
										pportals[p]->side->num_verts);

		for (int v=0; v<pportals[p]->side->num_verts; v++)
			fprintf(f, " ( %f %f %f)",	pportals[p]->side->verts[v].x,
												pportals[p]->side->verts[v].y,
												pportals[p]->side->verts[v].z);

		fprintf(f, "\n");

	}

	fclose(f);

	v_printf("%d portals filed - OK\n", num);
}


/*
==========
portal_run - do all portal related processing
==========
*/
void portal_run(bsp_node_t *head)
{
	portal_create(head);
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
	v_printf("%d outside, %d inside, %d solid\n", out, in, solid);



	portal_mark_visible_sides(head);

	portal_remove_invis(head);		// need all portals here

//	portal_remove_unwanted(head);	// remove everything except what can be walked through
//	portal_remove_outside(head);	// get rid of all nodes that are outside
	portal_print_list(head);		// find the portals to print, cant until we have leafs numbered (when bsp written)
}









