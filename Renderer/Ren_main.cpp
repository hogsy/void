#include "Standard.h"
#include "Ren_main.h"
#include "ShaderManager.h"
#include "Shader.h"
#include "Client.h"


extern CVar		 g_varVidSynch;
const  CCamera   * camera=0;

vector_t	forward, right, up;	// FIXME - move into eyepoint_t ?
int			eye_leaf;

//extern model_cache_t *tmodel;	// where model info is put so it can be rendered - FIXME
plane_t frust[5];	// 4 sides + near-z

const vector_t *fullblend;

//====================================================================================

/*
==============
get_node_for_vert
==============
*/
int get_leaf_for_point(const vector_t &v)
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
	/* set viewing projection */
	g_pRast->ProjectionMode(VRAST_PERSPECTIVE);

	// reset last r_vidsynch
	if (g_rInfo.rflags & RFLAG_SWAP_CONTROL)
	{

		if (g_varVidSynch.ival)
			g_pRast->SetVidSynch(1);
		else
			g_pRast->SetVidSynch(0);
	}

	g_rInfo.ready = true;
}


/***********************
r_getsky - make a set of polys from the sky
***********************/
cpoly_t* r_getsky(bspf_side_t *side)
{
	int		 num_clipverts;
	int		 sides[33];		// for clipping
	float	 dists[33];		// 
	vector_t verts[33];		// verts used in creating planes and clipping
	vector_t planes[33];	// dont need dists cause they all go through the origin
	cpoly_t *ret = NULL;	// poly list to be returned

	// create list of planes to clip sky polys to
	int v, nv;
	for (v=0; v<side->num_verts; v++)
		verts[v] = world->verts[world->iverts[v+side->first_vert]] - camera->origin;

	for (v=0; v<side->num_verts; v++)
	{
		nv = (v+1) % side->num_verts;

		CrossProduct(verts[v], verts[nv], planes[v]);
		planes[v].Normalize();
	}

	// add every sky poly to the list and clip to all planes
	// sky brushes are always brush 0
	int endside = world->brushes[0].first_side + world->brushes[0].num_sides;
	for (int s=world->brushes[0].first_side; s<endside; s++)
	{
		cpoly_t *npoly = g_pShaders->GetPoly();
		npoly->num_vertices = world->sides[s].num_verts;
		npoly->texdef            = world->sides[s].texdef;
		npoly->lightdef          = world->sides[s].lightdef;

		for (v=0; v<npoly->num_vertices; v++)
			npoly->vertices[v] = world->verts[world->iverts[world->sides[s].first_vert+v]];

		for (int p=0; p<side->num_verts; p++)
		{

			bool allfront = true;
			bool allback = true;

			for (int i=0; i<npoly->num_vertices; i++)
			{
				dists[i] = dot(planes[p], npoly->vertices[i]);
                                
				if (dists[i] > 0.01f)
				{
					sides[i] = 1;
					allback = false;
				}
				else if (dists[i] < -0.01f)
				{
					sides[i] = -1;
					allfront = false;
				}
				else
					sides[i] = 0;
			}

			// side not clipped by this plane
			if (allfront)
				continue;
			// side completely clipped out by this plane
			if (allback)
			{
				npoly->num_vertices = 0;
				break;
			}

			num_clipverts = 0;

			dists[i] = dists[0];
			sides[i] = sides[0];

			for (i=0; i<npoly->num_vertices; i++)
			{
				if (sides[i] == 0)
				{
					verts[num_clipverts] = npoly->vertices[i];
					verts[num_clipverts] = npoly->vertices[i];
					num_clipverts++;
					continue;
				}

				if (sides[i] == 1)
				{
					verts[num_clipverts] = npoly->vertices[i];
					num_clipverts++;
				}

				if ((sides[i+1] == 0) || (sides[i] == sides[i+1]))
					continue;

				vector_t *nextvert = &npoly->vertices[(i+1)%npoly->num_vertices];
				double frac = dists[i] / (dists[i]-dists[i+1]);
				verts[num_clipverts].x = (float)(npoly->vertices[i].x + frac*(nextvert->x - npoly->vertices[i].x));
				verts[num_clipverts].y = (float)(npoly->vertices[i].y + frac*(nextvert->y - npoly->vertices[i].y));
				verts[num_clipverts].z = (float)(npoly->vertices[i].z + frac*(nextvert->z - npoly->vertices[i].z));
				num_clipverts++;
			}

			// copy everything back
			if (num_clipverts > 32)
					num_clipverts = 32;
			memcpy(npoly->vertices, verts, sizeof(vector_t) * num_clipverts);
			npoly->num_vertices = num_clipverts;

		}

		if (npoly->num_vertices < 3)
		{
			npoly->next = NULL;
			g_pShaders->ReturnPoly(npoly);
		}
		else
		{
			// translate so it follows the view origin
			// add to list
			for (v=0; v<npoly->num_vertices; v++)
				npoly->vertices[v] += camera->origin;
			npoly->next = ret;
			ret = npoly;
		}
	}

	return ret;
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

// just push everything right through to the cache
	int endb = world->leafs[l].first_brush + world->leafs[l].num_brushes;
	for (int b=world->leafs[l].first_brush; b < endb; b++)
	{
		int ends = world->brushes[b].first_side + world->brushes[b].num_sides;
		for (int s=world->brushes[b].first_side; s<ends; s++)
		{
			bspf_side_t *side = &world->sides[s];

			if ((DotProduct(world->planes[side->plane].norm, camera->origin) - world->planes[side->plane].d) < 0)
				continue;

			// get polys from sky leaf if we need to
			if (g_pShaders->GetShader(g_pShaders->mWorldBin, world->texdefs[side->texdef].texture)->GetContentFlags() & CONTENTS_SKYVIEW)
			{
				cpoly_t *p = r_getsky(side);
				while (p)
				{
					cpoly_t *add = p;
					p = p->next;
					g_pShaders->CacheAdd(add);
				}
			}

			else
			{

				cpoly_t *p = g_pShaders->GetPoly();

				p->lightdef = side->lightdef;
				p->next = NULL;
				p->num_vertices = side->num_verts;
				p->texdef = side->texdef;

				for (int v=0; v<p->num_vertices; v++)
					p->vertices[v] = world->verts[world->iverts[side->first_vert+v]];

				g_pShaders->CacheAdd(p);
			}
		}
	}
}



void r_draw_node(int n, bool testfrust)
{
	//
	//frustum cull - check this nodes bounding box to the frustum
	//
	if (testfrust)
	{
		// only have to be outside one plane to require testing
		testfrust = false;


		vector_t in, out;
		for (int p=0; p<5; p++)
		{
			// we really only need to test 2 corners the 2 farthest from the plane
			if (frust[p].norm.x > 0)
			{
				in.x = world->nodes[n].maxs[0];
				out.x= world->nodes[n].mins[0];
			}
			else
			{
				in.x = world->nodes[n].mins[0];
				out.x= world->nodes[n].maxs[0];
			}

			if (frust[p].norm.y > 0)
			{
				in.y = world->nodes[n].maxs[1];
				out.y= world->nodes[n].mins[1];
			}
			else
			{
				in.y = world->nodes[n].mins[1];
				out.y= world->nodes[n].maxs[1];
			}

			if (frust[p].norm.z > 0)
			{
				in.z = world->nodes[n].maxs[2];
				out.z= world->nodes[n].mins[2];
			}
			else
			{
				in.z = world->nodes[n].mins[2];
				out.z= world->nodes[n].maxs[2];
			}


			// if the one closest to the inside is outside, the box is completely out
			if ((dot(in, frust[p].norm) - frust[p].d) < 0)
				return;

			if ((dot(out, frust[p].norm) - frust[p].d) < 0)
				testfrust = true;
		}
	}

	int nn;
	float d = dot(world->planes[world->nodes[n].plane].norm, camera->origin) - world->planes[world->nodes[n].plane].d;

	// front to back traversal
	if (d>=0)
	{
		nn = world->nodes[n].children[0];
		(nn>0) ? r_draw_node(nn, testfrust) : r_draw_leaf(-nn);

		nn = world->nodes[n].children[1];
		(nn>0) ? r_draw_node(nn, testfrust) : r_draw_leaf(-nn);
	}
	else
	{
		nn = world->nodes[n].children[1];
		(nn>0) ? r_draw_node(nn, testfrust) : r_draw_leaf(-nn);

		nn = world->nodes[n].children[0];
		(nn>0) ? r_draw_node(nn, testfrust) : r_draw_leaf(-nn);
	}
}


/*************************************************
build initial view frustrum
*************************************************/
// build a plane using 3 arbitrary points
void build_plane3(const vector_t &a, const vector_t &b, const vector_t &c, plane_t &p)
{
	vector_t u = c - a;
	vector_t v = b - a;

	CrossProduct(u, v, p.norm);
	p.norm.Normalize();
	p.d = dot(p.norm, a);
}


void build_frust(void)
{
	vector_t a, b, c, d, center;
	float x, z;

	x = (float) tan(g_varFov.ival * 0.5f * PI/180);
	z = x * 0.75f;			// always render in a 3:4 aspect ratio

	center = camera->origin + forward;

	a.VectorMA(center, -x, right);
	a.VectorMA(a, z, up);

	b.VectorMA(center,  x, right);
	b.VectorMA(b, z, up);

	c.VectorMA(center,  x, right);
	c.VectorMA(c,-z, up);

	d.VectorMA(center, -x, right);
	d.VectorMA(d,-z, up);


// 4 sides
	build_plane3(camera->origin, b, a, frust[0]);
	build_plane3(camera->origin, c, b, frust[1]);
	build_plane3(camera->origin, d, c, frust[2]);
	build_plane3(camera->origin, a, d, frust[3]);

// near-z
	frust[4].norm = forward;
	frust[4].d = dot(frust[4].norm, camera->origin);
}


/***********************
build and draw the world
***********************/
void r_draw_world()
{
	build_frust();
//	beam_reset();
	r_draw_node(0, true);
	g_pShaders->CachePurge();
	g_pClient->Purge();
}


/***********************
Draw the current frame
************************/
void r_drawframe(const CCamera * pcamera)
{
	camera = pcamera;
	camera->angles.AngleToVector(&forward, &right, &up);
	
	fullblend  =  &camera->blend;

	forward.Normalize();
	right.Normalize();
	up.Normalize();


	// find eye leaf for pvs tests
	eye_leaf = get_leaf_for_point(camera->origin);

//	g_pRast->ClearBuffers(/*VRAST_COLOR_BUFFER |*/ VRAST_DEPTH_BUFFER);

// set up the view transformation
	g_pRast->ProjectionMode(VRAST_PERSPECTIVE);

	g_pRast->MatrixReset();

	// switch to +y = north, +z = up coordinate system
	g_pRast->MatrixRotateX(-90);
	g_pRast->MatrixRotateY(-camera->angles.ROLL * 180/PI);
	g_pRast->MatrixRotateX(-camera->angles.PITCH * 180/PI);
	g_pRast->MatrixRotateZ(-(camera->angles.YAW - PI/2)  * 180/PI);
	g_pRast->MatrixTranslate(-camera->origin.x, -camera->origin.y, -camera->origin.z);

	r_draw_world();

	// display any messageseee
//	g_pClient->DrawHud();

// draw the console if we need to
//	g_prCons->Draw();

//	g_pRast->FrameEnd();
}


