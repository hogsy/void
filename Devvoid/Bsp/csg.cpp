/*
============================================
	clips all brushes so there is no overlap
	otherwise we might get infinite loops in
	the bsp partition stage.
============================================
*/
#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "../Std_lib.h"
#include "bsp.h"


//============================================
//			intersect testing funcs
//============================================

/*
==============
quick_test_intersect - check bounding boxs
==============
*/
bool quick_test_intersect(bsp_brush_t *b1, bsp_brush_t *b2)
{
	if ((b1->maxs[0] > b2->mins[0]) &&
		(b1->maxs[1] > b2->mins[1]) &&
		(b1->maxs[2] > b2->mins[2]) &&
		(b1->mins[0] < b2->maxs[0]) &&
		(b1->mins[1] < b2->maxs[1]) &&
		(b1->mins[2] < b2->maxs[2]))
		return true;

	return false;
}

/*
==============
slow_test_intersect - test all points to all planes
==============

bool slow_test_intersect(bsp_brush_t *b1, bsp_brush_t *b2)
{
	bsp_brush_side_t *s, *s1;
	for (s=b1->sides; s; s=s->next)
	{
		for (int v=0; v<s->num_verts; v++)
		{
			bool inside = true;
			for (s1=b2->sides; s1; s1=s1->next)
			{
				if ((dot(s->verts[v], planes[s1->plane].norm) - planes[s1->plane].d) >-0.01f)
				{
					inside = false;
					break;
				}
			}

			if (inside)
				return true;
		}
	}

	// now try the other way cause one could be completely inside the other
	for (s=b2->sides; s; s=s->next)
	{
		for (int v=0; v<s->num_verts; v++)
		{
			bool inside = true;
			for (s1=b1->sides; s1; s1=s1->next)
			{
				if ((dot(s->verts[v], planes[s1->plane].norm) - planes[s1->plane].d) > 0.01f)
				{
					inside = false;
					break;
				}
			}

			if (inside)
				return true;
		}
	}

	return false;
}
*/


/*
===========
brush_volume
===========
*/
float side_area(bsp_brush_side_t *si);
float brush_volume(bsp_brush_t *b)
{
	float vol = 0;
	vector_t corner = b->sides->verts[0];

	bsp_brush_side_t *s;
	for (s=b->sides; s; s=s->next)
	{
		float d = dot(planes[s->plane].norm, corner) - planes[s->plane].d;
		vol += d * side_area(s);
	}

	return vol/(-3);
}


/*
===========
brush_subtract : return = b1-b2
===========
*/
bsp_brush_t* brush_subtract(bsp_brush_t *b1, bsp_brush_t *b2)
{
	bsp_brush_t *front, *back;
	bsp_brush_t *split;	// the brush that's going to be split (backsides)
	bsp_brush_t *main;	// brush that's going to be returned (frontsides)

	split = copy_brush(b1);
	main = NULL;

	for (bsp_brush_side_t *s=b2->sides; s&&split; s=s->next)
	{

		// see if we are clipping to one of our own planes
		bool skip = false;
		for (bsp_brush_side_t *s2=split->sides; s2; s2=s2->next)
		{
			if (s2->plane == s->plane)
			{
				skip = true;
				break;
			}
		}

		if (skip)
			continue;


		bsp_brush_split(split, &front, &back, s->plane);
		if ((split != back) && (split != front))
			free_bsp_brush(split);

		split = back;

		// save frontside
		if (front)
		{
			front->next = main;
			main = front;
		}

	}


	return main;
}


/*
===========
brush_intersect - return the intersection of b1,b2
===========
*/
bsp_brush_t* brush_intersect(bsp_brush_t* b1, bsp_brush_t *b2)
{
	if (!b1->sides || !b2->sides)
		return NULL;

	bsp_brush_t *inter = copy_brush(b1);
	inter->next = NULL;
	bsp_brush_side_t *s, *s1;

// add all b2's sides to the intersection
	for (s=b2->sides; s; s=s->next)
	{
		// make sure we dont already have this plane
		bool skip = false;
		for (s1 = inter->sides; s1; s1=s1->next)
		{
			if (s1->plane == s->plane)
			{
				skip = true;
				break;
			}

			// planes are opposing - there really isn't any intersection
			else if (s1->plane == (s->plane^1))
			{
				// free the intersection
				for (bsp_brush_t *tmp=inter; tmp; )
				{
					inter = inter->next;
					free_bsp_brush(tmp);
					tmp = inter;
				}
				return NULL;
			}
		}

		if (skip)
			continue;

		bsp_brush_side_t *nside = new_bsp_brush_side();
		memcpy(nside, s, sizeof(bsp_brush_side_t));

		nside->next = inter->sides;
		inter->sides = nside;
	}

	// clip the brush to all it's own planes
	for (s=inter->sides; s; s=s->next)
	{
		for (s1=inter->sides; s1; s1=s1->next)
		{
			if (s->plane == s1->plane)
				continue;

			clip_side(s, s1->plane);
		}
	}

	// remove any planes that where clipped out
	bsp_brush_side_t *prev = NULL, *remove;
	for (s=inter->sides; s; )
	{
		if (s->num_verts == 0)
		{
			if (prev)
				prev->next = s->next;
			else
				inter->sides = s->next;

			remove = s;
			s = s->next;
			free_bsp_brush_side(remove);
		}

		else
		{
			prev = s;
			s = s->next;
		}
	}

	if (!inter->sides)
	{
		free_bsp_brush(inter);
		return NULL;
	}

	calc_brush_bounds(inter);
	inter->contents = b1->contents | b2->contents;

	// make sure there is a volume to it
	if (brush_volume(inter) < 0.5f)
	{
		free_bsp_brush(inter);
		return NULL;
	}

	return inter;
}


//============================================


/*
===============
run csg on all brushes and
return a new list
===============
*/
bsp_brush_t* run_csg(bsp_brush_t *head)
{
	if (!head)
		return NULL;

	int num_brushes = 0;
	for (bsp_brush_t *c=head; c; c=c->next)
		num_brushes++;
	ComPrintf("%d ->", num_brushes);

	bsp_brush_t *b1, *b2, *tmp, *tmp2, *tmp3;

	bsp_brush_t *tail = head;
	while (tail->next)
		tail = tail->next;

	bool more = true;
	while (more)
	{
		more = false;
		bsp_brush_t *end = tail;

		for (b1=head; b1!=end->next; b1=b1->next)
		{
			if (!b1->sides)
				continue;

			for (b2=b1->next; b2!=end->next; b2=b2->next)
			{
				if (!b2->sides)
					continue;

				if (quick_test_intersect(b1, b2))
				{
					tmp = brush_intersect(b1, b2);
					if (tmp)
					{
						// we know b1 and b2 intersect, tmp is the intersection
						more = true;

						// insert all subtracted brushes into the list
						tmp2 = brush_subtract(b1, tmp);
//						if (tmp2)
//						{
							tail->next = tmp2;
							while (tail->next)
								tail = tail->next;
//						}

						tmp3 = brush_subtract(b2, tmp);
//						if (tmp3)
//						{
							tail->next = tmp3;
							while (tail->next)
								tail = tail->next;
//						}

						// add the intersection to the end
						tail->next = tmp;
						tail = tail->next;
						tail->next = NULL;

						// b1 and b2 must be removed - remove the sides - they will be removed before next pass
						bsp_brush_side_t *s;
						if (tmp2)
						{
							for (s=b1->sides; s; )
							{
								bsp_brush_side_t *ts = s;
								s = s->next;
								free_bsp_brush_side(ts);
							}
						}

						if (tmp3)
						{
							for (s=b2->sides; s; )
							{
								bsp_brush_side_t *ts = s;
								s = s->next;
								free_bsp_brush_side(ts);
							}
						}

						b1->sides = b2->sides = NULL;
						break;
					}
				}
			}
		}

		// remove any brushes that have no sides
		b2 = NULL; // prev
		for (b1=head; b1; )
		{
			if (!b1->sides)
			{
				if (b2)
				{
					b1 = b1->next;
					free_bsp_brush(b2->next);
					b2->next = b1;
				}

				else
				{
					b1 = b1->next;
					free_bsp_brush(head);
					head = b1;
				}
			}
			else
			{
				b2 = b1;
				b1 = b1->next;
			}
		}
	}


	// count how many brushes we have now
	num_brushes = 0;
	for (c=head; c; c=c->next)
		num_brushes++;
	ComPrintf(" %d.  ", num_brushes);

	return head;
}
