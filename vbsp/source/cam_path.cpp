
/*
implementation of smooth camera paths
*/

#include "std_lib.h"
#include "entity.h"
#include <string.h>

#define MAX_CAM_VERTS 100
typedef float control_t[3];
typedef float eq_p[4];	// At^3 + Bt^2 + Ct + D
typedef eq_p eq_t[3];		// equations for our 3 components




/*
==========
take the determinant of a nxn matrix
subscripts are as follows:
[[0 1 2]
 [3 4 5]
 [6 7 8]]
and
[[ 0  1  2  3]
 [ 4  5  6  7]
 [ 8  9 10 11]
 [12 13 14 15]]
and so on
==========
*/
double det(int n, float *m)
{
	double ret;

	// 2x2 matrix - easy compute det
	if (n == 2)
	{
		ret = (m[0] * m[3]) -
			  (m[1] * m[2]);


	}

	// matrix is a 3x3 - calculate determinant straight forward
	else if (n == 3)
	{
		ret = (m[0] * m[4] * m[8]) +
			  (m[1] * m[5] * m[6]) +
			  (m[2] * m[3] * m[7]) -
			  (m[2] * m[4] * m[6]) -
			  (m[0] * m[5] * m[7]) -
			  (m[1] * m[3] * m[8]);
	}

	// have to do it recursively
	else
	{
		int coef = 1;
		ret = 0;

		// our n-1 x n-1 matrix
		float *mat = (float*)malloc(sizeof(float) * (n-1) * (n-1));

		// have to get a sub-determinant for each row
		for (int td=0; td<n; td++)
		{
			float *matp = mat;
			for (int r=0; r<n; r++)
			{
				// skip a row
				if (r == td)
					continue;

				// always skip the first column
				for (int c=1; c<n; c++)
				{
					*matp = m[r*n + c];
					matp++;
				}
			}

			ret += coef * m[td*n] * det(n-1, mat);
			coef *= -1;
		}

		free(mat);
	}

	return ret;
}


/*
==============
cam_get_eq
==============
*/
void cam_get_eq(eq_t eq, int e, control_t *controls, int num_controls)
{
	int p1, p2;
	control_t s1, s2;

	// which control points we need to pass through
	p1 = e;
	p2 = e+1;

	// set up slops at control points
	if (p1 == 0)
	{
		s1[0] = controls[1][0] - controls[0][0];
		s1[1] = controls[1][1] - controls[0][1];
		s1[2] = controls[1][2] - controls[0][2];
	}
	else
	{
		s1[0] = controls[p1+1][0] - controls[p1-1][0];
		s1[1] = controls[p1+1][1] - controls[p1-1][1];
		s1[2] = controls[p1+1][2] - controls[p1-1][2];
	}

	if (p2 == num_controls-1)
	{
		s2[0] = controls[num_controls-1][0] - controls[num_controls-2][0];
		s2[1] = controls[num_controls-1][1] - controls[num_controls-2][1];
		s2[2] = controls[num_controls-1][2] - controls[num_controls-2][2];
	}
	else
	{
		s2[0] = controls[p2+1][0] - controls[p2-1][0];
		s2[1] = controls[p2+1][1] - controls[p2-1][1];
		s2[2] = controls[p2+1][2] - controls[p2-1][2];
	}



	float *base = (float*)malloc(sizeof(float) * 16);
	float *num  = (float*)malloc(sizeof(float) * 16);

// the 2 points we have to pass through
// satisfy equation At^3 + Bt^2 + Ct + D
	base[0] = (float)p1*p1*p1;
	base[1] = (float)p1*p1;
	base[2] = (float)p1;
	base[3] = 1;

	base[4] = (float)p2*p2*p2;
	base[5] = (float)p2*p2;
	base[6] = (float)p2;
	base[7] = 1;

// the 2 slopes we need
// satisfy equation 3At^2 + 2Bt + C
	base[8 ] = (float)3*p1*p1;
	base[9 ] = (float)2*p1;
	base[10] = 1;
	base[11] = 0;

	base[12] = (float)3*p2*p2;
	base[13] = (float)2*p2;
	base[14] = 1;
	base[15] = 0;



	double dbase = det (4, base);
	if (dbase == 0)
	{
		free(base);
		free(num);
		Error("dbase == 0!");
	}

	// for each dim (x,y,z)
	for (int c=0; c<3; c++)
	{
		// each coefficient
		for (int d=0; d<4; d++)
		{
			memcpy(num, base, sizeof(float) * 16);
			num[ 0 + d] = controls[p1][c];
			num[ 4 + d] = controls[p2][c];
			num[ 8 + d] = s1[c];
			num[12 + d] = s2[c];

			eq[c][3-d] = (float)(det(4, num) / dbase);
		}
	}

	free(num);
	free(base);
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
	eq_t eq;
	for (int e=0; e<n-1; e++)
	{
		cam_get_eq(eq, e, (control_t*)(void*)verts, n);


		// add these keys
		char keyname[] = "e00t0";
		char value[1024];
		keyname[1] = (e/10) + '0';
		keyname[2] = (e%10) + '0';

		for (int i=0; i<4; i++)
		{
			keyname[4] = i + '0';
			sprintf(value, "%lf %lf %lf", eq[0][i], eq[1][i], eq[2][i]);

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














