
#include <memory.h>
#include <string.h>

#include "std_lib.h"
#include "vlight.h"
#include "light.h"
#include "world.h"
#include "trace.h"


bspf_texdef_t	lightdefs[16384];	// MAX_MAP_TEXINFOS
int				nlightdefs;

world_t		*world;
int			samples=1;	// samples per lumel

lightmap_t	*lightmaps[65536];	// max brush sides
int			num_lightmaps;

light_t		lights[1024];
int			num_lights;

unsigned char	ambient[3] = { 0, 0, 0 };



/*
========
light_fill_defs
========
*/
vector_t baseaxis[6] =
{
	{0, 1, 0},  {0, 0, 1},
	{1, 0, 0},  {0, 0, 1},
	{1, 0, 0},  {0, 1, 0}
};

int	comps[6] = 
{
	1, 2,
	0, 2,
	0, 1
};

void light_fill_defs(void)
{
	v_printf("creating lightdefs: ");
	vector_t br;	// top-right and bottom-left
	float	 cosv, sinv, len;
	vector_t dir;

	for (int l=0; l<num_lightmaps; l++)
	{
		lightdefs[l].texture = l;

		VectorMA(&lightmaps[l]->origin, lightmaps[l]->lwidth,  &lightmaps[l]->right, &br);
		VectorMA(&br, lightmaps[l]->lheight, &lightmaps[l]->down,  &br);


		// create our transformation

		int	axis;	// base axis for side

		if (fabs(world->planes[world->sides[lightmaps[l]->side].plane].norm.y) > 
			fabs(world->planes[world->sides[lightmaps[l]->side].plane].norm.x))
		{
			if (fabs(world->planes[world->sides[lightmaps[l]->side].plane].norm.z) >
				fabs(world->planes[world->sides[lightmaps[l]->side].plane].norm.y))
				axis = 2;
			else
				axis = 1;
		}

		else
		{
			if (fabs(world->planes[world->sides[lightmaps[l]->side].plane].norm.z) >
				fabs(world->planes[world->sides[lightmaps[l]->side].plane].norm.x))
				axis = 2;
			else
				axis = 0;
		}


		for (int i=0; i<2; i++)
		{
			if (i==0)
			{
				VectorCopy(lightmaps[l]->right, dir);
			}
			else
			{
				VectorCopy(lightmaps[l]->down, dir);
			}

			// project dir onto base plane
			((float*)&dir)[axis] = 0;
			len = VectorNormalize(&dir);

			if (i==0)
				len *= lightmaps[l]->lwidth;
			else
				len *= lightmaps[l]->lheight;

			cosv = dot(baseaxis[axis*2], dir);
			if (cosv >= 0.99f)
			{
				cosv = 1;
				sinv = 0;
			}
			else if (cosv < -0.99f)
			{
				cosv = -1;
				sinv = 0;
			}
			else if (fabs(cosv) < 0.01f)
			{
				cosv = 0;
				if (dot(baseaxis[axis*2+1], dir) > 0)
					sinv = 1;
				else
					sinv = -1;
			}
			else
			{
				// cosv stays the same, find sinv
				sinv = (float)sin(acos(cosv));
				if (dot(baseaxis[axis*2+1], dir) < 0)
					sinv *= -1;
			}


			lightdefs[l].vecs[i][axis] = 0;
			lightdefs[l].vecs[i][comps[axis*2  ]] = cosv / len;
			lightdefs[l].vecs[i][comps[axis*2+1]] = sinv / len;
			lightdefs[l].vecs[i][3] = -dot (dir, lightmaps[l]->origin);
			lightdefs[l].vecs[i][3] /= len;

		}
	}

	nlightdefs = num_lightmaps;

	v_printf("OK\n");
}


/*
========
light_assemble - assemble all the lightmap data into one chunk - also free's other lightmaps
========
*/
int light_assemble(unsigned char **data)
{
	*data = (unsigned char*)malloc((32*32*3+2) * num_lightmaps + 2);
	if (!*data)
		Error("not enough mem to assemble lightmaps");


	unsigned char *p = *data;
	for (int l=0; l<num_lightmaps; l++)
	{
		*p = (unsigned char)lightmaps[l]->width;
		p++;

		*p = (unsigned char)lightmaps[l]->height;
		p++;

		memcpy(p, &lightmaps[l]->data, lightmaps[l]->width*lightmaps[l]->height*3);
		p += lightmaps[l]->width*lightmaps[l]->height*3;

		free(lightmaps[l]);
	}

	// terminate the string
	*p = 0;
	p++;
	*p = 0;
	p++;

	num_lightmaps = 0;

	v_printf("%d bytes light data\n", p - *data);
	return (p - *data);
}


/*
========
light_write - rewrite the bsp file with light data
========
*/
void light_write(char *file)
{
	v_printf("Writing bsp with light info: ");

	FILE *f = fopen(file, "r+b");
	if (!f)
		Error("couldn't open %s for light info!", file);

	light_fill_defs();

	bspf_header_t header;
	fread(&header, 1, sizeof(bspf_header_t), f);


	// assemble all the lightmap data
	unsigned char *data;
	header.lumps[LUMP_LIGHTMAP].length = light_assemble(&data);

	// write lightmap data at the end
	fseek(f, 0, SEEK_END);
	header.lumps[LUMP_LIGHTMAP].offset = ftell(f);
	fwrite(data, 1, header.lumps[LUMP_LIGHTMAP].length, f);

	// write lightdef data
	header.lumps[LUMP_LIGHTDEF].offset = ftell(f);
	header.lumps[LUMP_LIGHTDEF].length = nlightdefs * sizeof(bspf_texdef_t);
	fwrite(lightdefs, nlightdefs, sizeof(bspf_texdef_t), f);

	// rewrite sides with lightdef indices
	fseek(f, header.lumps[LUMP_SIDES].offset, SEEK_SET);
	fwrite(world->sides, 1, header.lumps[LUMP_SIDES].length, f);


	// rewrite the header with light offset info
	fseek(f, 0, SEEK_SET);
	fwrite(&header, 1, sizeof(bspf_header_t), f);
	fclose(f);

	free (data);

	v_printf("OK\n");
}


/*
========
ColorNormalize
========
*/
void ColorNormalize(vector_t *v)
{
	float max;
	max = (v->x > v->y) ? v->x : v->y;
	if (v->z > max)
		max = v->z;

	if (max <= 0)
	{
		VectorSet(v, 1, 1, 1);
		return;
	}
	VectorScale(v, 1/max, v);
}


/*
========
light_find - find all the lights we'll use
========
*/
void light_find(void)
{
	num_lights = 0;

	for (int e=0; e<world->nentities; e++)
	{
		if (strcmp(key_get_value(world, e, "classname"), "light") != 0)
			continue;

		key_get_vector(world, e, "origin", lights[num_lights].origin);
		key_get_vector(world, e, "_color", lights[num_lights].color);
		ColorNormalize(&lights[num_lights].color);

		lights[num_lights].intensity = key_get_float(world, e, "light");
		if (!lights[num_lights].intensity)
			lights[num_lights].intensity = key_get_float(world, e, "_light");
		if (!lights[num_lights].intensity)
			lights[num_lights].intensity = 300;

		num_lights++;
	}

}


/*
========
leaf_for_vector
========
*/
int leaf_for_vector(vector_t &v)
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


/*
========
point_in_side
========
*/
bool point_in_side(vector_t &vert, int s)
{
	plane_t plane;
	int v, nv;
	vector_t tmp;
	bspf_side_t *side = &world->sides[s];

	for (v=0; v<side->num_verts; v++)
	{
		nv = (v+1)%side->num_verts;

		// build a plane that points inward
		VectorSub(world->verts[world->iverts[nv+side->first_vert]], world->verts[world->iverts[v+side->first_vert]], tmp);
		_CrossProduct(&tmp, &world->planes[side->plane].norm, &plane.norm);
		VectorNormalize(&plane.norm);
		plane.d = dot(plane.norm, world->verts[world->iverts[v+side->first_vert]]);

		// only need to be outside of one plane
		if ((dot(plane.norm, vert) - plane.d) < -1)
			return false;
	}

	return true;
}


/*
========
light_do - light the given lightmap with the given light
========
*/
void light_do(int map, int l)
{
	// if we have vis info, see if we can skip
	if (world->leafvis_size > 0)
	{
		int light_leaf = leaf_for_vector(lights[l].origin);
		int map_leaf   = lightmaps[map]->leaf;

		if (*(world->leafvis + world->leafs[light_leaf].vis + (map_leaf>>3)) & (1<<(map_leaf&7)))
			return;
	}

	light_t *light = &lights[l];
	lightmap_t *lmap = lightmaps[map];

	float tw, th;	// lumel width/height
	tw = lmap->lwidth  / lmap->width;
	th = lmap->lheight / lmap->height;
	
	float sw, sh;	// sample width/height
	sw = tw / (samples+1);
	sh = th / (samples+1);

	float intensity = light->intensity * light->intensity;

	vector_t tl;	// top-left of current texel
	vector_t test;	// where we're tracing to
	vector_t accum;
	vector_t tmp;
	float	 len;	// length of trace
	int traces;
	int add;
	
	for (int h=0; h<lmap->width; h++)
	{
		for (int v=0; v<lmap->height; v++)
		{
			traces=0;

			VectorMA(&lmap->origin, h*tw, &lmap->right, &tl);
			VectorMA(&tl, v*th, &lmap->down, &tl);

			VectorSet(&accum, 0, 0, 0);

			for (int s=1; s<=samples; s++)
			{
				VectorMA(&tl,   s*sw, &lmap->right, &test);
				VectorMA(&test, s*sh, &lmap->down,  &test);

				VectorSub(test, light->origin, tmp);
				len = VectorLength(&tmp);
				if (len > light->intensity)
					continue;

				if (!point_in_side(test, lmap->side))
					continue;

				trace_t tr = trace(light->origin, test);
				if (tr.fraction < 1)
					continue;

				// add this trace to the lumel
				VectorMA(&accum, light->intensity - len, &light->color, &accum);
				traces++;
			}

			// average our traces
			if (traces > 0)
			{
				VectorScale(&accum, 1.0f/traces, &accum);

				// round and clamp and add to total
				for (int i=0; i<3; i++)
				{
					add = lmap->data[(v*lmap->width+h)*3 + i];
					add += (int)floor(((float*)&accum.x)[i] + 0.5);

					if (add > 255)
						add = 255;
					else if (add < 0)
						add = 0;

					// store it off
					lmap->data[(v*lmap->width+h)*3 + i] = (unsigned char)add;
				}
			}
		}
	}
}


/*
========
light_node
========
*/
void light_all(void)
{
	for (int lm=0; lm<num_lightmaps; lm++)
	{
		lightmaps[lm];
		for (int li=0; li<num_lights; li++)
		{
			light_do(lm, li);
		}

		v_printf("%4d of %4d\n", lm, num_lightmaps);
	}
}


/*
========
light_run - light the given file
========
*/
void light_run(char *file)
{
	world = world_create(file);
	if (!world)
		Error("Couldn't load %s", file);
	if (world->light_size)
	{
		v_printf("%s already contains light data.");
		world_destroy(world);
		return;
	}


	num_lightmaps = 0;
	lightmap_build(0);
	if (num_lightmaps == 0)
	{
		world_destroy(world);
		Error("no lightmaps!");
	}

	light_find();
	if (num_lights == 0)
	{
		for (int l=0; l<num_lightmaps; l++)
			free(lightmaps[l]);
		world_destroy(world);
		Error("no lights!");
	}

	light_all();

	light_write(file);
	world_destroy(world);
}

