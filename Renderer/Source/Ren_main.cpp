
#include "Standard.h"
#include "Ren_main.h"
#include "Ren_sky.h"
#include "Ren_beam.h"

#include "Tex_hdr.h"
#include "Clip.h"
#include "Con_main.h"
#include "Mdl_main.h"
#include "Mdl_cache.h"
#include "Hud_main.h"

extern CRConsole * g_prCons;
extern CVar *	g_pVidSynch;


eyepoint_t	eye;			// where we're gonna draw from
vector_t	forward, right, up;	// FIXME - move into eyepoint_t ?
int			eye_leaf;

extern model_cache_t *tmodel;	// where model info is put so it can be rendered - FIXME
plane_t frust[5];	// 4 sides + near-z


//====================================================================================

/*
==============
get_node_for_vert
==============
*/
int get_leaf_for_point(vector_t &v)
{
	int n=0;
	float d;

	do
	{
		// test to this nodes plane
		d = dot(world->planes[world->nodes[n].plane].norm, v) - world->planes[world->nodes[n].plane].d;

		if (d>=0)
			n = world->nodes[n].children[0];
		else
			n = world->nodes[n].children[1];

		// if we found a leaf, it's what we want
		if (n<=0)
			return -n;

	} while (1);

	return 0;
}

//====================================================================================


/***********************
Renderer Initiation - set up initial render state
***********************/
void r_init(void)
{
	glClearColor(.1, .1, .1, 1);

	float x = (float) tan(g_rInfo.fov * 0.5f);
	float z = x * 0.75f;						// always render in a 3:4 aspect ratio

	/* set viewing projection */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-x, x, -z, z, 1, 10000);

	/* position viewer */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, tex->base_names[0]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DITHER);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);


	// reset last r_vidsynch
	if (g_rInfo.rflags & RFLAG_SWAP_CONTROL)
	{
		if (g_pVidSynch->ival)
			wglSwapIntervalEXT(1);
		else
			wglSwapIntervalEXT(0);
	}

	g_rInfo.ready = true;
}



//===========================================================
//	draw node - draws this node, then what's behind it
//===========================================================
/*
==============
r_draw_brush
==============

void r_draw_brush(int n)
{
	//
	// just add polys to the cache
	// this is where the beam tree / c-buffer / s-buffer / whatever insertion would be
	int endp = world->brushes[n].first_side + world->brushes[n].num_sides;
	for (int p=world->brushes[n].first_side; p<endp; p++)
	{
		// backface cull
		if (dot(forward, world->planes[world->sides[p].plane].norm) > 0)
			continue;

		cpoly_t *poly = get_poly();
		poly->poly.num_vertices = world->sides[p].num_verts;
		poly->poly.texdef = world->sides[p].texdef;
		poly->poly.lightdef = world->sides[p].lightdef;

		for (int v=0; v<poly->poly.num_vertices; v++)
		{
			VectorCopy(world->verts[world->iverts[world->sides[p].first_vert+v]], poly->poly.vertices[v]);
		}

		cache_add_poly(poly);
	}
}
*/


void r_draw_leaf(int l)
{
	//
	//frustum cull - check this nodes bounding box to the frustum
	//
	vector_t point;
	for (int p=0; p<5; p++)
	{
		// we really only need to test 1 corner
		// find that corner
		point.x = (float)((frust[p].norm.x > 0) ? (world->leafs[l].maxs[0]) : (world->leafs[l].mins[0]));
		point.y = (float)((frust[p].norm.y > 0) ? (world->leafs[l].maxs[1]) : (world->leafs[l].mins[1]));
		point.z = (float)((frust[p].norm.z > 0) ? (world->leafs[l].maxs[2]) : (world->leafs[l].mins[2]));

		if ((dot(point, frust[p].norm) - frust[p].d) < 0)
			return;
	}

	// pvs cull
	if (world->leafvis_size > 0)
	{
		if (!(*(world->leafvis + world->leafs[eye_leaf].vis + (l>>3)) & (1<<(l&7))))
			return;
	}

	int endb = world->leafs[l].first_brush + world->leafs[l].num_brushes;
	for (int b=world->leafs[l].first_brush; b < endb; b++)
		beam_insert(&world->brushes[b]);
}



void r_draw_node(int n)
{
	//
	//frustum cull - check this nodes bounding box to the frustum
	//
	vector_t point;
	for (int p=0; p<5; p++)
	{
		// we really only need to test 1 corner
		// find that corner
		point.x = (float)((frust[p].norm.x > 0) ? (world->nodes[n].maxs[0]) : (world->nodes[n].mins[0]));
		point.y = (float)((frust[p].norm.y > 0) ? (world->nodes[n].maxs[1]) : (world->nodes[n].mins[1]));
		point.z = (float)((frust[p].norm.z > 0) ? (world->nodes[n].maxs[2]) : (world->nodes[n].mins[2]));

		if ((dot(point, frust[p].norm) - frust[p].d) < 0)
			return;
	}

	int nn;
	float d = dot(world->planes[world->nodes[n].plane].norm, eye.origin) - world->planes[world->nodes[n].plane].d;

	// front to back traversal
	if (d>=0)
	{
		nn = world->nodes[n].children[0];
		(nn>0) ? r_draw_node(nn) : r_draw_leaf(-nn);

		nn = world->nodes[n].children[1];
		(nn>0) ? r_draw_node(nn) : r_draw_leaf(-nn);
	}
	else
	{
		nn = world->nodes[n].children[1];
		(nn>0) ? r_draw_node(nn) : r_draw_leaf(-nn);

		nn = world->nodes[n].children[0];
		(nn>0) ? r_draw_node(nn) : r_draw_leaf(-nn);
	}
}


/*************************************************
build initial view frustrum
*************************************************/
void build_frust(void)
{
	vector_t a, b, c, d, center;
	float x, z;

	x = (float) tan(g_rInfo.fov * 0.5f);
	z = x * 0.75f;			// always render in a 3:4 aspect ratio

	VectorAdd(eye.origin, forward, center);

	VectorMA(&center, -x, &right, &a);
	VectorMA(&a, z, &up, &a);

	VectorMA(&center, x, &right, &b);
	VectorMA(&b, z, &up, &b);

	VectorMA(&center, x, &right, &c);
	VectorMA(&c, -z, &up, &c);

	VectorMA(&center, -x, &right, &d);
	VectorMA(&d, -z, &up, &d);

// 4 sides
	clip_build_plane3(&eye.origin, &b, &a, &frust[0]);
	clip_build_plane3(&eye.origin, &c, &b, &frust[1]);
	clip_build_plane3(&eye.origin, &d, &c, &frust[2]);
	clip_build_plane3(&eye.origin, &a, &d, &frust[3]);

// near-z
	VectorCopy(forward, frust[4].norm);
	frust[4].d = dot(frust[4].norm, eye.origin);
}


/***********************
build and draw the world
***********************/
void r_draw_world(vector_t *blend)
{
	// turn on zbuffering for now
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glColor3fv(&blend->x);

	build_frust();
	beam_reset();
	r_draw_node(0);
	cache_purge();
}


/***********************
Draw the current frame
***********************/
void r_drawframe(vector_t *origin, vector_t *angles, vector_t *blend)
{
//FIXME !!!!!!!!!!!!!!!!!!!!!
	eye.origin = *origin;
	eye.angles = *angles;

	AngleToVector (&eye.angles, &forward, &right, &up);
	VectorNormalize(&forward);
	VectorNormalize(&right);
	VectorNormalize(&up);

	eye_leaf = get_leaf_for_point(eye.origin);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// set up the view transformation
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glRotatef( eye.angles.ROLL  * 180/PI, 0, 0, 1);
	glRotatef(-eye.angles.PITCH * 180/PI, 1, 0, 0);
	glRotatef( eye.angles.YAW   * 180/PI, 0, 1, 0);
	glTranslatef(-eye.origin.x, -eye.origin.z, eye.origin.y);

	r_draw_world(blend);

	glPopMatrix();

// display any messages
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	g_prHud->DrawHud();

// draw the console if we need to
	g_prCons->Draw();

	glFlush();
	_SwapBuffers(g_rInfo.hDC);
}


/*
======================================
Just draw the console
======================================
*/

void r_drawcons()
{
	glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);
	
	g_prCons->Draw();

	glFlush();
	_SwapBuffers(g_rInfo.hDC);
}