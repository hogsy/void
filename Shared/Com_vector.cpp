#include "Com_defs.h"
#include "Com_vector.h"


// doesn't keep the length the same! - just projects it onto the plane
void MakeVectorPlanar(vector_t *in, vector_t *out, vector_t *norm)
{
	float d = dot((*in), (*norm));
	VectorMA(in, -d, norm, out);
}

void VectorSet(vector_t *a, float x, float y, float z)
{ 
	a->x = x;
	a->y = y;
	a->z = z;
}


int VectorCompare (const vector_t *v1, const vector_t *v2)
{
	if (v1->x != v2->x || v1->y != v2->y || v1->z != v2->z)
			return 0;

	return 1;
}


int VectorCompare2 (const vector_t *v1, const vector_t *v2, float thresh)
{
	if ((v1->x-v2->x < -thresh) || (v1->x-v2->x > thresh) ||
		(v1->y-v2->y < -thresh) || (v1->y-v2->y > thresh) ||
		(v1->z-v2->z < -thresh) || (v1->z-v2->z > thresh))
		return 0;
	return 1;
}



float VectorNormalize2 (vector_t *v, vector_t *out)
{
	float	length, ilength;

	length = v->x*v->x + v->y*v->y + v->z*v->z;
	length = (float)sqrt (length);

	if (length)
	{
		ilength = 1/length;
		out->x = v->x*ilength;
		out->y = v->y*ilength;
		out->z = v->z*ilength;
	}

	return length;
}

void VectorMA (const vector_t *veca, float scale, const vector_t *vecb, vector_t *vecc)
{
	vecc->x = veca->x + scale*vecb->x;
	vecc->y = veca->y + scale*vecb->y;
	vecc->z = veca->z + scale*vecb->z;
}


void _CrossProduct(const vector_t *a,const vector_t *b, vector_t *normal)
{
	normal->x = (a->y*b->z - a->z*b->y);
	normal->y = (a->z*b->x - a->x*b->z);
	normal->z = (a->x*b->y - a->y*b->x);
}


float VectorLength(const vector_t *v)
{
	return (float)sqrt((v->x*v->x)+(v->y * v->y)+(v->z * v->z));
/*
	float	length;

	length = (v->x*v->x) + (v->y * v->y) + (v->z * v->z);
	length = (float)sqrt (length);

	return length;
*/
}



float VectorNormalize(vector_t *v)
{
	float mag;
	mag = VectorLength (v);
	v->x /= mag;
	v->y /= mag;
	v->z /= mag;
	return mag;
}


void VectorScale (const vector_t *in, float scale, vector_t *out)
{
	out->x = in->x*scale;
	out->y = in->y*scale;
	out->z = in->z*scale;
}


void AngleToVector (const vector_t *angles, vector_t *forward, vector_t *right, vector_t *up)
{

	float		angle;
	static float		sr, sp, sy, cr, cp, cy;

	angle = -angles->YAW + PI/2;
	sy = (float)sin(angle);
	cy = (float)cos(angle);
	angle = -angles->PITCH;
	sp = (float)sin(angle);
	cp = (float)cos(angle);
	angle = -angles->ROLL;
	sr = (float)sin(angle);
	cr = (float)cos(angle);

	if (forward)
	{
		forward->x = cp*cy;
		forward->y = cp*sy;
		forward->z = -sp;
	}
	if (right)
	{
		right->x = (-1*sr*sp*cy+-1*cr*-sy);
		right->y = (-1*sr*sp*sy+-1*cr*cy);
		right->z = -1*sr*cp;
	}

	if (up)
	{
		up->x = (cr*sp*cy+-sr*-sy);
		up->y = (cr*sp*sy+-sr*cy);
		up->z = cr*cp;
	}
}


/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +	in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +	in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +	in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +	in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +	in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +	in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +	in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +	in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +	in1[2][2] * in2[2][2];
}



/*
** assumes "src" is normalized
*/
void PerpendicularVector(vector_t *dst, vector_t *src)
{
	int	pos;
	float minelem = 1.0F;
	vector_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	pos = 0;
	if (fabs(src->x) < minelem)
	{
		pos = 0;
		minelem = (float)fabs(src->x);
	}

	if (fabs(src->y) < minelem)
	{
		pos = 1;
		minelem = (float)fabs(src->y);
	}

	if (fabs(src->z) < minelem)
	{
		pos = 2;
		minelem = (float)fabs(src->z);
	}

	VectorSet(&tempvec, 0, 0, 0);

	if (pos == 0)
		tempvec.x = 1;

	else if (pos == 1)
		tempvec.y = 1;

	else
		tempvec.z = 1;

	MakeVectorPlanar(&tempvec, dst, src);
	VectorNormalize(dst);
}


#ifdef _WIN32
#pragma optimize( "", off )
#endif


void RotatePointAroundVector(vector_t *dst, vector_t *dir, vector_t *point, float rad)
{
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	vector_t vr, vup, vf;

	VectorCopy((*dir), vf);

	PerpendicularVector(&vr, dir);
	_CrossProduct(&vr, &vf, &vup);

	m[0][0] = vr.x;
	m[1][0] = vr.y;
	m[2][0] = vr.z;

	m[0][1] = vup.x;
	m[1][1] = vup.y;
	m[2][1] = vup.z;

	m[0][2] = vf.x;
	m[1][2] = vf.y;
	m[2][2] = vf.z;

	memcpy(im, m, sizeof(im));

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];


	memset(zrot, 0, sizeof(zrot));
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	zrot[0][0] =  (float)cos(rad);
	zrot[0][1] =  (float)sin(rad);
	zrot[1][0] = -(float)sin(rad);
	zrot[1][1] =  (float)cos(rad);



	R_ConcatRotations( m, zrot, tmpmat );
	R_ConcatRotations( tmpmat, im, rot );

	dst->x = rot[0][0] * point->x + rot[0][1] * point->y + rot[0][2] * point->z;
	dst->y = rot[1][0] * point->x + rot[1][1] * point->y + rot[1][2] * point->z;
	dst->z = rot[2][0] * point->x + rot[2][1] * point->y + rot[2][2] * point->z;
}


#ifdef _WIN32
#pragma optimize( "", on )
#endif

