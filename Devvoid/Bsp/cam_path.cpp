#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "../Std_lib.h"
#include "entity.h"


/*
implementation of smooth camera paths
*/



#define MAX_CAM_VERTS 256
#define MAX_EQUATIONS (MAX_CAM_VERTS-1)
typedef float control_t[3];
typedef float eq_p[4];	// At^3 + Bt^2 + Ct + D
typedef eq_p eq_t[3];		// equations for our 3 components

eq_t	eqs[MAX_EQUATIONS];


//--------
// use gaussian elimination with partial pivoting to solve a system
// n = number of equations, m=augmented matrix, put results in coef
//--------
void GaussElim(int n, float *m, float *coef)
{
	int r,c,r1;
	float *rows[MAX_EQUATIONS*4];

	for (r=0; r<n; r++)
		rows[r] = &m[r*(n+1)];


	for (r=0; r<(n-1); r++)
	{
		// partial pivoting - find the row we want to use
		int best = r;
		for (r1=r+1; r1<n; r1++)
		{
			if (fabs(rows[r1][r]) > fabs(rows[best][r]))
				best = r1;
		}

		// swap rows
		if (best != r)
		{
			float *tmp = rows[r];
			rows[r] = rows[best];
			rows[best] = tmp;
		}

		if (fabs(rows[r][r]) == 0)
			exit(5);	// close to singular

		// elimination using row r
		for (r1=r+1; r1<n; r1++)
		{
			float scale = rows[r1][r] / rows[r][r];
			for (c=(r+1); c<(n+1); c++)
				rows[r1][c] -= rows[r][c]*scale;
		}
	}


	// back substitution
	for (r=(n-1); r>=0; r--)
	{
		coef[r] = 0;
		for (c=(r+1); c<n; c++)
			coef[r] += rows[r][c]*coef[c];

		coef[r] -= rows[r][n];
		coef[r] /= -rows[r][r];
	}

}



void cam_get_eq_data(control_t *controls, int num_controls)
{
	int ncoefs = (num_controls-1)*4;
	int rowsize= ncoefs+1;

	int r,c;

	float *m = (float*)malloc(sizeof(float)*ncoefs*rowsize);
	float *coef = (float*)malloc(sizeof(float)*ncoefs);


	for (int i=0; i<3; i++)
	{
		memset(m, 0, sizeof(float)*ncoefs*rowsize);

		for (int t=0; t<num_controls; t++)
		{
			// data fitting that uses this point

			// left side equation
			if (t!=0)
			{
				r = (t-1)*4+1;
				c = (t-1)*4;

				m[r*rowsize+c+0] = (float)(t*t*t);
				m[r*rowsize+c+1] = (float)(t*t);
				m[r*rowsize+c+2] = (float)(t);
				m[r*rowsize+c+3] = (float)(1);
				m[(r+1)*rowsize-1] = controls[t][i];
			}

			// right side equation
			if (t!=(num_controls-1))
			{
				r = t*4;
				c = t*4;

				m[r*rowsize+c+0] = (float)(t*t*t);
				m[r*rowsize+c+1] = (float)(t*t);
				m[r*rowsize+c+2] = (float)(t);
				m[r*rowsize+c+3] = (float)(1);
				m[(r+1)*rowsize-1] = controls[t][i];
			}


			// derivative fitting - only for interior points
			if ((t==0) || (t == (num_controls-1)))
				continue;

			r = (t-1)*4 + 2;
			c = (t-1)*4;


			// first derivative
			m[r*rowsize+c+0] = (float)( 3*t*t);
			m[r*rowsize+c+1] = (float)( 2*t);
			m[r*rowsize+c+2] = (float)( 1);
			m[r*rowsize+c+4] = (float)(-3*t*t);
			m[r*rowsize+c+5] = (float)(-2*t);
			m[r*rowsize+c+6] = (float)(-1);

			// second derivative
			r++;
			m[r*rowsize+c+0] = (float)( 6*t);
			m[r*rowsize+c+1] = (float)( 2);
			m[r*rowsize+c+4] = (float)(-6*t);
			m[r*rowsize+c+5] = (float)(-2);
		}

		// two more equations for the end points

/*
		if (mode == 1)
		{
			// "natural" spline - second derivative at endpoints == 0
			r = ncoefs-2;
			c = 0;
			m[r*rowsize+c+0] = 0;
			m[r*rowsize+c+1] = 2;

			r++;
			c = rowsize-5;
			m[r*rowsize+c+0] = (num_controls-1) * 6;
			m[r*rowsize+c+1] = 2;
		}

		else
*/		{
			// not-a-knot / extrapolation
			r = ncoefs-2;
			c = 0;
			m[r*rowsize+c+0] = 8;
			m[r*rowsize+c+1] = 4;
			m[r*rowsize+c+2] = 2;
			m[r*rowsize+c+3] = 1;
			m[(r+1)*rowsize-1] = controls[2][i];


			r++;
			c = rowsize-5;
			m[r*rowsize+c+0] = (float)((num_controls-3)*(num_controls-3)*(num_controls-3));
			m[r*rowsize+c+1] = (float)((num_controls-3)*(num_controls-3));
			m[r*rowsize+c+2] = (float)((num_controls-3));
			m[r*rowsize+c+3] = (float)(1);
			m[(r+1)*rowsize-1] = controls[(num_controls-3)][i];
		}

		GaussElim(ncoefs, m, coef);

		// copy data into our eq structs
		for (r=0; r<ncoefs; r++)
		{
			eqs[r/4][i][3-r%4] = coef[r];
		}
	}


	free(m);
	free(coef);

}


/*
==============
cam_calc_eqs
keys are in the form:
ex1tx2 where x1 = which eq it is and x2 is the t^x2 component
are vector keys
==============
*/
void cam_calc_eqs(entity_t *head, vector_t *verts, int n)
{
	cam_get_eq_data((control_t*)(void*)verts, n);

	for (int e=0; e<n-1; e++)
	{
		// add these keys
		char keyname[] = "e00t0";
		char value[1024];
		keyname[1] = (e/10) + '0';
		keyname[2] = (e%10) + '0';

		for (int i=0; i<4; i++)
		{
			keyname[4] = i + '0';
			sprintf(value, "%lf %lf %lf", eqs[0][i], eqs[1][i], eqs[2][i]);

			vkey_t *nkey = get_key();
			strcpy(nkey->k.name, keyname);
			strcpy(nkey->k.value, value);

			nkey->next = head->key;
			head->key = nkey;
		}
	}

	vkey_t *k = get_key();
	sprintf(k->k.name, "%s", "num_eqs");
	sprintf(k->k.value, "%d", n-1);
	k->next = head->key;
	head->key = k;

// change the head classname
	char *hc = key_get_value(head, "classname");
	strcpy(hc, "misc_camera_path_head");
}


/*
==============
cam_calc_path
==============
*/
void cam_calc_paths(entity_t *ent)
{
	vector_t cam_verts[MAX_CAM_VERTS];

	for (entity_t *walk=ent; walk; walk=walk->next)
	{
		// find only entities that are the beginning of a path
		if (strcmp(key_get_value(walk, "classname"), "misc_camera_path") != 0)
			continue;

		if (strcmp(key_get_value(walk, "targetname"), "") != 0)
			continue;


		// go through the whole linkage, find all our verts

		int num_verts = 0;
		entity_t *walk2 = walk;
		while (1)
		{
			char *target = key_get_value(walk2, "target");
			if (!target[0])
				break;

			// find the target
			for (walk2=ent; walk2; walk2=walk2->next)
			{
				if (strcmp(key_get_value(walk2, "targetname"), target) == 0)
				{
					key_get_vector(walk2, "origin", cam_verts[num_verts]);
					num_verts++;
					if (num_verts > MAX_CAM_VERTS)
						Error("too many camera verts.");
					break;
				}
			}

			// terminated - at the end of the path
			if (!walk2)
				Error("Couldn't find target %s for camera path", target);
		}


		// we have all the verts, find the equations
		cam_calc_eqs(walk, cam_verts, num_verts);
	}
}














