/*
	This algorithm does NOT produce an exact PVS.  This is because if a portal is AT ALL visible,
	it assumes that it is entirely visible.  To be removed, the portal must be entirely occluded.
	It was not always this way, but it would take about a week to vis a level with perfect visiblity.
*/


#include "../Std_lib.h"
#include "Com_world.h"
#include "vis.h"


#define	MAX_MAP_VISIBILITY	0x1000000
#define	MAX_PLANES			(65536*4)
#define MAX_MAP_LEAFS		65536
#define MAX_MAP_PORTALS		65536		// same as in bsp



unsigned char lvis[MAX_MAP_VISIBILITY];
int	lvis_size;

// vars already defined in bsp or light
extern CWorld *world;

vportal_t	vportals[MAX_MAP_PORTALS];
int			num_vportals;

vportal_t* leaf_vportals[MAX_MAP_LEAFS];

plane_t		vplanes[MAX_PLANES];
int			num_vplanes;

int max_planes = 0;

/*
========
load_portals - doesnt do much error checking
========
*/
bool load_portals(const char *name)
{
	char token[1024];

	num_vportals = 0;

	// set so all leafs have no vportals
	memset(leaf_vportals, 0, sizeof(vportal_t*)*MAX_MAP_LEAFS);

	ComPrintf("Loading portal file: ");


	I_FileReader * pFile = CreateFileReader(FILE_BUFFERED);

	if(!pFile->Open(name))
	{
		ComPrintf("load_portals(): Could not open %s\n",name);
		pFile->Destroy();
		return false;
	}


	pFile->GetToken(token, true);
	if (strcmp(token, "VoidPortalFile"))
	{
		ComPrintf("load_portals(): Not a valid portal file!");
		pFile->Destroy();
		return false;
	}


	while (1)
	{
		pFile->GetToken(token, true);
		if (!token[0])
			break;

		// nodes
		vportals[num_vportals].nodes[0] = atoi(token);

		pFile->GetToken(token, false);
		vportals[num_vportals].nodes[1] = atoi(token);

		// planes - node 0 is pointed into by the original plane
		pFile->GetToken(token, false);
		vportals[num_vportals].planes[1] = &world->planes[atoi(token)];

		pFile->GetToken(token, false);
		vportals[num_vportals].planes[0] = &world->planes[atoi(token)];

		// num verts
		pFile->GetToken(token, false);
		vportals[num_vportals].num_verts = atoi(token);

		// vert data
		for (int v=0; v<vportals[num_vportals].num_verts; v++)
		{
			pFile->GetToken(token, false);	// (

			pFile->GetToken(token, false);
			vportals[num_vportals].verts[0][v].x = atof(token);
			vportals[num_vportals].verts[1][vportals[num_vportals].num_verts - v - 1].x = vportals[num_vportals].verts[0][v].x;

			pFile->GetToken(token, false);
			vportals[num_vportals].verts[0][v].y = atof(token);
			vportals[num_vportals].verts[1][vportals[num_vportals].num_verts - v - 1].y = vportals[num_vportals].verts[0][v].y;

			pFile->GetToken(token, false);
			vportals[num_vportals].verts[0][v].z = atof(token);
			vportals[num_vportals].verts[1][vportals[num_vportals].num_verts - v - 1].z = vportals[num_vportals].verts[0][v].z;

			pFile->GetToken(token, false);	// )

		}

		// add the portal to it's 2 leafs
		vportals[num_vportals].next[0] = leaf_vportals[vportals[num_vportals].nodes[0]];
		vportals[num_vportals].next[1] = leaf_vportals[vportals[num_vportals].nodes[1]];

		leaf_vportals[vportals[num_vportals].nodes[0]] = &vportals[num_vportals];
		leaf_vportals[vportals[num_vportals].nodes[1]] = &vportals[num_vportals];

		num_vportals++;
	}

	pFile->Destroy();
	ComPrintf("%d portals loaded\n", num_vportals);
	return true;
}

/*
=============
clip_side
=============
*/
#define PLANE_ON	0
#define PLANE_FRONT	1
#define PLANE_BACK	-1
#define CLIP_EPSILON 0.001
int vis_clip(vector_t *source, int snum, vector_t *dest, int &dnum, plane_t *p)
{
	int			sides[33];
	float		dists[33];

	bool allin = true;
	bool allout= true;

	dnum = 0;

	for (int i=0; i<snum; i++)
	{
		dists[i] = DotProduct(p->norm, source[i]) - p->d;

		if (dists[i] > CLIP_EPSILON)
		{
			sides[i] = PLANE_FRONT;
			allin = false;
		}
		else if (dists[i] < -CLIP_EPSILON)
		{
			sides[i] = PLANE_BACK;
			allout = false;
		}
		else
			sides[i] = PLANE_ON;
	}

	// coplanar or all behind the plane
	if (allin)
		return PLANE_BACK;

	if (allout)
	{
		dnum = snum;
		memcpy(dest, source, sizeof(vector_t) * snum);
		return PLANE_FRONT;
	}

	dists[i] = dists[0];
	sides[i] = sides[0];

	for (i=0; i<snum; i++)
	{
		if (sides[i] == PLANE_ON)
		{
			dest[dnum] = source[i];
			dnum++;
			continue;
		}

		if (sides[i] == PLANE_FRONT)
		{
			dest[dnum] = source[i];
			dnum++;
		}

		if ((sides[i+1] == PLANE_ON) || (sides[i] == sides[i+1]))
			continue;

		vector_t *nextvert = &source[(i+1)%snum];
		double frac = dists[i] / (dists[i]-dists[i+1]);
		dest[dnum].x = (float)(source[i].x + frac*(nextvert->x - source[i].x));
		dest[dnum].y = (float)(source[i].y + frac*(nextvert->y - source[i].y));
		dest[dnum].z = (float)(source[i].z + frac*(nextvert->z - source[i].z));
		dnum++;
	}

	if (dnum > 32)
		Error("clipped face to too many verts");

	return PLANE_ON;
}

// determine which side of the plane the verts are on
int vis_sides(vector_t *pts, int num, plane_t *p)
{
	bool allin = true;
	bool allout= true;

	for (int i=0; i<num; i++)
	{
		float d;
		d = DotProduct(p->norm, pts[i]) - p->d;

		if (d > CLIP_EPSILON)
		{
			if (!allout)
				return PLANE_ON;
			allin = false;
		}
		else if (d < -CLIP_EPSILON)
		{
			if (!allin)
				return PLANE_ON;
			allout = false;
		}
	}



	// coplanar returns split (no good)
	if (allin && allout)
		return PLANE_ON;


	if (allin)
		return PLANE_BACK;

//	if (allout)
		return PLANE_FRONT;
}

// build a plane using 3 arbitrary points
void build_plane3(const vector_t &a, const vector_t &b, const vector_t &c, plane_t &p)
{
	vector_t u = c - a;
	vector_t v = b - a;

	CrossProduct(u, v, p.norm);
	p.norm.Normalize();
	p.d = dot(p.norm, a);
}


/*
========
vis_add_planes
========
*/
void vis_add_planes(vector_t *src, int nsrc, vector_t *dst, int ndst)
{
	// create planes such that src is completely on back side and dst is completely on front side
	int v1, nv1, v2;
	int side1, side2;

	// 2 points from src, 1 from dst
	for (v1=0; v1<nsrc; v1++)
	{
		nv1 = (v1+1) % nsrc;

		for (v2=0; v2<ndst; v2++)
		{
			if (num_vplanes == MAX_PLANES)
				Error("too many vis planes");

			build_plane3(src[v1], src[nv1], dst[v2], vplanes[num_vplanes]);

			// cant split either portal
			side1 = vis_sides(src, nsrc, &vplanes[num_vplanes]);
			if (side1 == PLANE_ON)
				continue;

			side2 = vis_sides(dst, ndst, &vplanes[num_vplanes]);
			if (side2 == PLANE_ON)
				continue;

			// must be on opposite sides
			if (side1 == side2)
				continue;

			// flip the plane if we have to
			if (side1 == PLANE_FRONT)
			{
				vplanes[num_vplanes].norm.Inverse();
				vplanes[num_vplanes].d = -vplanes[num_vplanes].d;
			}

			// we got a good plane
			num_vplanes++;
		}
	}

	// 2 points from dst, 1 from src
	for (v1=0; v1<ndst; v1++)
	{
		nv1 = (v1+1) % ndst;

		for (v2=0; v2<nsrc; v2++)
		{
			if (num_vplanes == MAX_PLANES)
				Error("too many vis planes");

			build_plane3(dst[v1], dst[nv1], src[v2], vplanes[num_vplanes]);

			// cant split either portal
			side1 = vis_sides(src, nsrc, &vplanes[num_vplanes]);
			if (side1 == PLANE_ON)
				continue;

			side2 = vis_sides(dst, ndst, &vplanes[num_vplanes]);
			if (side2 == PLANE_ON)
				continue;

			// must be on opposite sides
			if (side1 == side2)
				continue;

			// flip the plane if we have to
			if (side1 == PLANE_FRONT)
			{
				vplanes[num_vplanes].norm.Inverse();
				vplanes[num_vplanes].d = -vplanes[num_vplanes].d;
			}

			// we got a good plane
			num_vplanes++;
		}
	}

	if (num_vplanes > max_planes)
		max_planes = num_vplanes;
}


/*
========
vis_r
========
*/
int vis_r(int leaf, int into, vector_t *sportal, int sportalnum, int planestart)
{
	int visible = 0;
	if (!(*(lvis + world->leafs[leaf].vis + (into>>3)) & (1 << (into&7))))
		visible++;

	// into is visible
	*(lvis + world->leafs[leaf].vis + (into>>3)) |= 1 << (into&7);

	// cant go through solids
	if (world->leafs[into].contents & CONTENTS_SOLID)
		return visible;


	// go to each portal that leads out of into
	vportal_t *next;
	for (vportal_t *p=leaf_vportals[into]; p; p=next)
	{
		int side = (into == p->nodes[0]) ? 0 : 1;
		next = p->next[side];

		// never go back into leaf
		if (p->nodes[1-side] == leaf)
			continue;

		// new origin winding
		vector_t newsportal[32];
		int newsportalnum;

		vis_clip(sportal, sportalnum, newsportal, newsportalnum, p->planes[1-side]);
		if (newsportalnum < 3)
			continue;

		vector_t dest[32];
		vector_t tmp[32];
		int		tmpnum;
		int		destnum;
		memcpy(tmp, p->verts[side], sizeof(vector_t)*p->num_verts);
		tmpnum = p->num_verts;

		// clip dest portal to all umbra planes
		for (int cplane=planestart; cplane<num_vplanes; cplane++)
		{
			vis_clip(tmp, tmpnum, dest, destnum, &vplanes[cplane]);
			if (destnum < 3)
				break;	// dest portal completely obscured

			memcpy(tmp, dest, sizeof(vector_t)*destnum);
			tmpnum = destnum;
		}

		// must have broken early
		if (cplane != num_vplanes)
			continue;

		// if we got here, there is part of the portal visible
		// add new umbra planes and go through it
		int newplanestart = num_vplanes;
		vis_add_planes(newsportal, newsportalnum, dest, destnum);

		visible += vis_r(leaf, p->nodes[1-side], newsportal, newsportalnum, newplanestart);
		num_vplanes = newplanestart;	// remove the planes we added
	}

	return visible;
}


/*
========
vis_leaf
========
*/
void vis_leaf(int leaf)
{
	ComPrintf("vising leaf %d\n", leaf);

	int visible = 1;

	// we can always see ourself
	*(lvis + world->leafs[leaf].vis + (leaf>>3)) |= 1 << (leaf&7);

	// go to every portal out of this leaf
	vportal_t *next;
	for (vportal_t *p=leaf_vportals[leaf]; p; p=next)
	{
		int side = (leaf == p->nodes[0]) ? 0 : 1;
		next = p->next[side];

		vplanes[0] = *p->planes[side];
		num_vplanes = 1;

		visible += vis_r(leaf, p->nodes[1-side], p->verts[side], p->num_verts, 0);
	}

	ComPrintf("%d has %d visible\n", leaf, visible);
}


/*
========
vis_node - vis everything
========
*/
void vis_all(void)
{

	lvis_size = (world->nleafs >> 3) + 1;

	// clear - nothing can see anything
	memset(lvis, 0, MAX_MAP_VISIBILITY);


	// this will also vis other entities but that shouldn't
	// matter because they wont have portals going in/out of them
	for (int l=0; l<world->nleafs; l++)
	{
		// store vis offset for this leaf
		world->leafs[l].vis = lvis_size * l;

		// vis this leaf - solids see everything
		if (world->leafs[l].contents & CONTENTS_SOLID)
			memset(lvis+world->leafs[l].vis, 0xff, lvis_size);
		else
			vis_leaf(l);
	}

	ComPrintf("max planes - %d\n", max_planes);
}


/*
========
vis_write - modify the old bsp file to contain vis info
========
*/
void vis_write(const char * szPath)
{
	ComPrintf("Writing world with vis info: ");

	// write vis data to the world
	world->leafvis = lvis;
	world->leafvis_size = lvis_size;

	world->WriteToFile(szPath);

	// have to set leafvis to NULL so it doesnt get freed in DestroyWorld()
	world->leafvis = NULL;

	ComPrintf("OK\n");
}


/*
========
vis_run
========
*/
void vis_run(const char * szPath, const char * szFileName)
{
	char worldfile[260];
	strcpy(worldfile, szFileName);
	strcpy(&worldfile[strlen(worldfile)-3], "wld");

	// load world data
	world = CWorld::CreateWorld(worldfile);
	if(!world)
	{
		ComPrintf("Unable to open world file: %s\n", worldfile);
		return;
	}

	world->DestroyVisData();

	// load portals
	if (!load_portals(szFileName))
	{
		CWorld::DestroyWorld(world);
		return;
	}

	vis_all();

	vis_write(szPath);

	CWorld::DestroyWorld(world);
}




















