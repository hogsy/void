#include "Standard.h"
#include "Ren_main.h"
#include "Ren_beam.h"
#include "Ren_cache.h"
#include "Tex_hdr.h"
#include "Con_main.h"
#include "Client.h"


extern CRConsole * g_prCons;
extern CVar		 * g_pVidSynch;
const  CCamera   * camera=0;

vector_t	forward, right, up;	// FIXME - move into eyepoint_t ?
int			eye_leaf;

//extern model_cache_t *tmodel;	// where model info is put so it can be rendered - FIXME
plane_t frust[5];	// 4 sides + near-z

extern	vector_t *fullblend;

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
//	glClearColor(.1, .1, .1, 1);

	/* set viewing projection */
	g_pRast->ProjectionMode(VRAST_PERSPECTIVE);

/*
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, tex->base_names[0]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DITHER);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
*/

	// reset last r_vidsynch
	if (g_rInfo.rflags & RFLAG_SWAP_CONTROL)
	{

		if (g_pVidSynch->ival)
			g_pRast->SetVidSynch(1);
		else
			g_pRast->SetVidSynch(0);
	}

	g_rInfo.ready = true;
}



//===========================================================
//	draw node - draws this node, then what's behind it
//===========================================================
void r_draw_leaf(int l)
{
	// leaf 0 is always the outside leaf
	if (l==0)
		return;

	// pvs cull
	if (world->leafvis_size > 0)
	{
		if (!(*(world->leafvis + world->leafs[eye_leaf].vis + (l>>3)) & (1<<(l&7))))
			return;
	}

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

	int endb = world->leafs[l].first_brush + world->leafs[l].num_brushes;
	for (int b=world->leafs[l].first_brush; b < endb; b++)
		beam_insert(&world->brushes[b], world->leafs[l].contents);
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
	float d = dot(world->planes[world->nodes[n].plane].norm, camera->origin) - world->planes[world->nodes[n].plane].d;

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
// build a plane using 3 arbitrary points
void build_plane3(vector_t &a, vector_t &b, vector_t &c, plane_t &p)
{
	vector_t u, v;
	VectorSub(c, a, u);
	VectorSub(b, a, v);

	_CrossProduct(&u, &v, &p.norm);
	VectorNormalize (&p.norm);
	p.d = dot(p.norm, a);
}


void build_frust(void)
{
	vector_t a, b, c, d, center;
	float x, z;

	x = (float) tan(g_pFov->ival * 0.5f * PI/180);
	z = x * 0.75f;			// always render in a 3:4 aspect ratio

	VectorAdd(camera->origin, forward, center);

	VectorMA(&center, -x, &right, &a);
	VectorMA(&a, z, &up, &a);

	VectorMA(&center, x, &right, &b);
	VectorMA(&b, z, &up, &b);

	VectorMA(&center, x, &right, &c);
	VectorMA(&c, -z, &up, &c);

	VectorMA(&center, -x, &right, &d);
	VectorMA(&d, -z, &up, &d);

// 4 sides
	build_plane3(camera->origin, b, a, frust[0]);
	build_plane3(camera->origin, c, b, frust[1]);
	build_plane3(camera->origin, d, c, frust[2]);
	build_plane3(camera->origin, a, d, frust[3]);

// near-z
	VectorCopy(forward, frust[4].norm);
	frust[4].d = dot(frust[4].norm, camera->origin);
}


/***********************
build and draw the world
***********************/
void r_draw_world()
{
	build_frust();
	beam_reset();
	r_draw_node(0);
	cache_purge();
}


/***********************
Draw the current frame
************************/
void r_drawframe(const CCamera * pcamera)
{
	camera = pcamera;

	AngleToVector (&camera->angles, &forward, &right, &up);
	
	fullblend  =  &camera->blend;

	VectorNormalize(&forward);
	VectorNormalize(&right);
	VectorNormalize(&up);

	// find eye leaf for pvs tests
	eye_leaf = get_leaf_for_point(camera->origin);

	g_pRast->ClearBuffers(/*VRAST_COLOR_BUFFER |*/ VRAST_DEPTH_BUFFER);

// set up the view transformation
	g_pRast->ProjectionMode(VRAST_PERSPECTIVE);

	g_pRast->MatrixReset();

	g_pRast->MatrixRotateZ( camera->angles.ROLL  * 180/PI);
	g_pRast->MatrixRotateX(-camera->angles.PITCH * 180/PI);
	g_pRast->MatrixRotateY( camera->angles.YAW   * 180/PI);

	g_pRast->MatrixTranslate(camera->origin);


	r_draw_world();

	// display any messages
	g_pClient->DrawHud();

// draw the console if we need to
	g_prCons->Draw();

	g_pRast->FrameEnd();
}


/*
======================================
Just draw the console
======================================
*/

void r_drawcons()
{
	g_pRast->ClearBuffers(/*VRAST_COLOR_BUFFER |*/ VRAST_DEPTH_BUFFER);
	g_prCons->Draw();
	g_pRast->FrameEnd();
}