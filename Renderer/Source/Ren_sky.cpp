
#include "standard.h"
#include "Ren_sky.h"
#include "Ren_cache.h"
#include "clip.h"

extern poly_t *tpoly;			// temporary poly
extern eyepoint_t eye;			// where we're gonna draw from
extern frustum_t *frust;		// frustum stack

//sector_t sky_box;
//w_vertex_t sky_verts[8];


/***********************
draw skybox
***********************/
void sky_draw(int frustum, float time)	// FIXME - add an axis
{
/*
	unsigned int f, v;
	vector_t tmp, tmp2, axis;

	VectorSet(&axis, 1, 1, 1);
	VectorNormalize(&axis);

// go to each face
	for (f = 0; f < 6; f++)
	{
		tpoly->num_vertices = 4;

	// rotate/translate.
		for (v = 0; v < 4; v++)
		{
		// rotate around the axis, translate to be centered around the eye
			VectorCopy((*sky_box.faces[f].vertices[v]), tmp);
			RotatePointAroundVector(&tmp2, &axis, &tmp, time);
			VectorAdd(tmp2, eye.origin.point, tpoly->vertices[v]);

			tpoly->vertices[v].s = sky_box.faces[f].tverts[v].s;
			tpoly->vertices[v].t = sky_box.faces[f].tverts[v].t;
			tpoly->color = 0xffffffff;
		}

		clip_poly_to_frust(tpoly, &frust[frustum]);

		if (tpoly->num_vertices < 3)	// poly isn't valid anymore
			continue;

		cache_add_poly(tpoly, sky_box.faces[f].tex_index + world->header.num_textures);
	}
*/
}


/*
======================================
free all skybox mem
======================================
*/
void sky_shutdown(void)
{
/*
// skybox
	for (int f = 0; f < 6; f++)
	{
		free (sky_box.faces[f].tverts);
		free (sky_box.faces[f].vertices);
	}
	free (sky_box.faces);
*/
}



/*
======================================
allocate all sky mem, hard code the sky box sector
======================================
*/
void sky_init(void)
{
/*
//
// set up the sky box
//
	sky_verts[0].x = -128;		sky_verts[0].y = -128;		sky_verts[0].z =  128;
	sky_verts[1].x =  128;		sky_verts[1].y = -128;		sky_verts[1].z =  128;
	sky_verts[2].x =  128;		sky_verts[2].y =  128;		sky_verts[2].z =  128;
	sky_verts[3].x = -128;		sky_verts[3].y =  128;		sky_verts[3].z =  128;
	sky_verts[4].x = -128;		sky_verts[4].y = -128;		sky_verts[4].z = -128;
	sky_verts[5].x = -128;		sky_verts[5].y =  128;		sky_verts[5].z = -128;
	sky_verts[6].x =  128;		sky_verts[6].y =  128;		sky_verts[6].z = -128;
	sky_verts[7].x =  128;		sky_verts[7].y = -128;		sky_verts[7].z = -128;


	sky_box.num_faces = 6;
	sky_box.faces = (face_t*)MALLOC(sizeof(face_t)*6);
	if (sky_box.faces == NULL) FError("mem for skybox");

// BACK
	sky_box.faces[0].num_vertices = 4;
	sky_box.faces[0].vertices = (w_vertex_t**)MALLOC(sizeof(w_vertex_t*) * 4);
	if (sky_box.faces[0].vertices == NULL)  FError("mem for sky verts");
	sky_box.faces[0].tex_index = 0;
	VectorSet(&sky_box.faces[0].plane.norm, 0, 1, 0);
	sky_box.faces[0].plane.d = -128;
	sky_box.faces[0].portal = NULL;
	sky_box.faces[0].vertices[0] = &sky_verts[1];
	sky_box.faces[0].vertices[1] = &sky_verts[0];
	sky_box.faces[0].vertices[2] = &sky_verts[4];
	sky_box.faces[0].vertices[3] = &sky_verts[7];
	sky_box.faces[0].tverts = (tvert_t*)MALLOC(sizeof(tvert_t) * 4);
	if (sky_box.faces[0].tverts == NULL) FError("mem for sky tverts");
	sky_box.faces[0].tverts[0].s = 0;
	sky_box.faces[0].tverts[0].t = 0;
	sky_box.faces[0].tverts[1].s = 1;
	sky_box.faces[0].tverts[1].t = 0;
	sky_box.faces[0].tverts[2].s = 1;
	sky_box.faces[0].tverts[2].t = 1;
	sky_box.faces[0].tverts[3].s = 0;
	sky_box.faces[0].tverts[3].t = 1;

// LEFT
	sky_box.faces[1].num_vertices = 4;
	sky_box.faces[1].vertices = (w_vertex_t**)MALLOC(sizeof(w_vertex_t*) * 4);
	if (sky_box.faces[1].vertices == NULL)  FError("mem for sky verts");
	sky_box.faces[1].tex_index = 1;
	VectorSet(&sky_box.faces[1].plane.norm, 1, 0, 0);
	sky_box.faces[1].plane.d = -128;
	sky_box.faces[1].portal = NULL;
	sky_box.faces[1].vertices[0] = &sky_verts[0];
	sky_box.faces[1].vertices[1] = &sky_verts[3];
	sky_box.faces[1].vertices[2] = &sky_verts[5];
	sky_box.faces[1].vertices[3] = &sky_verts[4];
	sky_box.faces[1].tverts = (tvert_t*)MALLOC(sizeof(tvert_t) * 4);
	if (sky_box.faces[1].tverts == NULL) FError("mem for sky tverts");
	sky_box.faces[1].tverts[0].s = 0;
	sky_box.faces[1].tverts[0].t = 0;
	sky_box.faces[1].tverts[1].s = 1;
	sky_box.faces[1].tverts[1].t = 0;
	sky_box.faces[1].tverts[2].s = 1;
	sky_box.faces[1].tverts[2].t = 1;
	sky_box.faces[1].tverts[3].s = 0;
	sky_box.faces[1].tverts[3].t = 1;



// FRONT
	sky_box.faces[2].num_vertices = 4;
	sky_box.faces[2].vertices = (w_vertex_t**)MALLOC(sizeof(w_vertex_t*) * 4);
	if (sky_box.faces[2].vertices == NULL)  FError("mem for sky verts");
	sky_box.faces[2].tex_index = 2;
	VectorSet(&sky_box.faces[2].plane.norm, 0, -1, 0);
	sky_box.faces[2].plane.d = -128;
	sky_box.faces[2].portal = NULL;
	sky_box.faces[2].vertices[0] = &sky_verts[3];
	sky_box.faces[2].vertices[1] = &sky_verts[2];
	sky_box.faces[2].vertices[2] = &sky_verts[6];
	sky_box.faces[2].vertices[3] = &sky_verts[5];
	sky_box.faces[2].tverts = (tvert_t*)MALLOC(sizeof(tvert_t) * 4);
	if (sky_box.faces[2].tverts == NULL) FError("mem for sky tverts");
	sky_box.faces[2].tverts[0].s = 0;
	sky_box.faces[2].tverts[0].t = 0;
	sky_box.faces[2].tverts[1].s = 1;
	sky_box.faces[2].tverts[1].t = 0;
	sky_box.faces[2].tverts[2].s = 1;
	sky_box.faces[2].tverts[2].t = 1;
	sky_box.faces[2].tverts[3].s = 0;
	sky_box.faces[2].tverts[3].t = 1;

// RIGHT
	sky_box.faces[3].num_vertices = 4;
	sky_box.faces[3].vertices = (w_vertex_t**)MALLOC(sizeof(w_vertex_t*) * 4);
	if (sky_box.faces[3].vertices == NULL)  FError("mem for sky verts");
	sky_box.faces[3].tex_index = 3;
	VectorSet(&sky_box.faces[3].plane.norm, -1, 0, 0);
	sky_box.faces[3].plane.d = -128;
	sky_box.faces[3].portal = NULL;
	sky_box.faces[3].vertices[0] = &sky_verts[2];
	sky_box.faces[3].vertices[1] = &sky_verts[1];
	sky_box.faces[3].vertices[2] = &sky_verts[7];
	sky_box.faces[3].vertices[3] = &sky_verts[6];
	sky_box.faces[3].tverts = (tvert_t*)MALLOC(sizeof(tvert_t) * 4);
	if (sky_box.faces[3].tverts == NULL) FError("mem for sky tverts");
	sky_box.faces[3].tverts[0].s = 0;
	sky_box.faces[3].tverts[0].t = 0;
	sky_box.faces[3].tverts[1].s = 1;
	sky_box.faces[3].tverts[1].t = 0;
	sky_box.faces[3].tverts[2].s = 1;
	sky_box.faces[3].tverts[2].t = 1;
	sky_box.faces[3].tverts[3].s = 0;
	sky_box.faces[3].tverts[3].t = 1;

// UP
	sky_box.faces[4].num_vertices = 4;
	sky_box.faces[4].vertices = (w_vertex_t**)MALLOC(sizeof(w_vertex_t*) * 4);
	if (sky_box.faces[4].vertices == NULL)  FError("mem for sky verts");
	sky_box.faces[4].tex_index = 4;
	VectorSet(&sky_box.faces[4].plane.norm, 0, 0, -1);
	sky_box.faces[4].plane.d = -128;
	sky_box.faces[4].portal = NULL;
	sky_box.faces[4].vertices[0] = &sky_verts[0];
	sky_box.faces[4].vertices[1] = &sky_verts[1];
	sky_box.faces[4].vertices[2] = &sky_verts[2];
	sky_box.faces[4].vertices[3] = &sky_verts[3];
	sky_box.faces[4].tverts = (tvert_t*)MALLOC(sizeof(tvert_t) * 4);
	if (sky_box.faces[4].tverts == NULL) FError("mem for sky tverts");
	sky_box.faces[4].tverts[0].s = 0;
	sky_box.faces[4].tverts[0].t = 0;
	sky_box.faces[4].tverts[1].s = 1;
	sky_box.faces[4].tverts[1].t = 0;
	sky_box.faces[4].tverts[2].s = 1;
	sky_box.faces[4].tverts[2].t = 1;
	sky_box.faces[4].tverts[3].s = 0;
	sky_box.faces[4].tverts[3].t = 1;

// DOWN
	sky_box.faces[5].num_vertices = 4;
	sky_box.faces[5].vertices = (w_vertex_t**)MALLOC(sizeof(w_vertex_t*) * 4);
	if (sky_box.faces[5].vertices == NULL)  FError("mem for sky verts");
	sky_box.faces[5].tex_index = 5;
	VectorSet(&sky_box.faces[5].plane.norm, 0, 0, 1);
	sky_box.faces[5].plane.d = -128;
	sky_box.faces[5].portal = NULL;
	sky_box.faces[5].vertices[0] = &sky_verts[5];
	sky_box.faces[5].vertices[1] = &sky_verts[6];
	sky_box.faces[5].vertices[2] = &sky_verts[7];
	sky_box.faces[5].vertices[3] = &sky_verts[4];
	sky_box.faces[5].tverts = (tvert_t*)MALLOC(sizeof(tvert_t) * 4);
	if (sky_box.faces[5].tverts == NULL) FError("mem for sky tverts");
	sky_box.faces[5].tverts[0].s = 0;
	sky_box.faces[5].tverts[0].t = 0;
	sky_box.faces[5].tverts[1].s = 1;
	sky_box.faces[5].tverts[1].t = 0;
	sky_box.faces[5].tverts[2].s = 1;
	sky_box.faces[5].tverts[2].t = 1;
	sky_box.faces[5].tverts[3].s = 0;
	sky_box.faces[5].tverts[3].t = 1;
*/
}
