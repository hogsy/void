#ifndef VOID_VECTOR_H
#define VOID_VECTOR_H

#include <math.h>

#ifndef PI
#define PI    3.14159265358979323846f
#endif

#define DEG2RAD( x ) ( (x) * PI / 180.0f )
#define RAD2DEG( x ) ( (x) * 180.0f / PI )

#define ROLL	z	// around z axis
#define PITCH	x	// around x axis
#define YAW		y	// around y axis


typedef struct
{
	float x, y, z;
}vector_t;


typedef struct
{
	vector_t	norm;	// normal to plane
	float		d;		// distance from origin
} plane_t;


//new replacement 3d funcs
namespace Void3d
{

inline float DotProduct(const vector_t &a, const vector_t &b)
{	return ( a.x*b.x + a.y*b.y + a.z*b.z );
}

inline void VectorSet(vector_t &dest, const vector_t &a)
{	dest.x = a.x; 
	dest.y = a.y; 
	dest.z = a.z;
}

inline void VectorSet(vector_t &dest, float x, float y, float z)
{	dest.x = x;
	dest.y = y;
	dest.z = z;
}

inline float VectorLength(const vector_t &v)
{	return (float)sqrt((v.x*v.x)+(v.y * v.y)+(v.z * v.z));
}

inline void VectorSub(const vector_t &a, const vector_t &b, vector_t &dest)
{
	dest.x = a.x - b.x;
	dest.y = a.y - b.y;
	dest.z = a.z - b.z;
}

inline float VectorDistance(const vector_t &a, const vector_t &b)
{
	vector_t dist;
	VectorSub(a,b,dist);
	return VectorLength(dist);
}

inline void	 VectorScale(vector_t &out, const vector_t &in, float scale)
{	out.x = in.x * scale;
	out.y = in.y * scale;
	out.z = in.z * scale;
}


/*

inline void VectorAdd(vector_t &dest, const vector_t &a)
{	dest.x +=a.x;
	dest.y +=a.y;
	dest.z +=a.z;
}

inline void VectorAdd(vector_t &dest, vector_t &a, vector_t &b)
{	dest.x += (a.x + b.x);
	dest.y += (a.y + b.y);
	dest.z += (a.z + b.z);
}

inline void VectorSub(vector_t &dest, const vector_t &a, const vector_t &b)
{
	dest.x=a.x-b.x; 
	dest.y=a.y-b.y; 
	dest.z=a.z-b.z;
}
inline void VectorInv(vector_t &dest)
{	dest.x = -dest.x;
	dest.y = -dest.y;
	dest.z = -dest.z;
}

inline void VectorInv(vector_t &src, vector_t &dest)
{	dest.x = -src.x;
	dest.y = -src.y;
	dest.z = -src.z;
}
*/

}


// math prototypes
int		VectorCompare (const vector_t *v1, const vector_t *v2);
float	VectorNormalize2 (vector_t *v, vector_t *out);
void	VectorMA (const vector_t *veca, float scale, const vector_t *vecb, vector_t *vecc);
void	_CrossProduct(const vector_t *a, const vector_t *b, vector_t *normal);
float	VectorLength(const vector_t *v);
float	VectorNormalize(vector_t *v);
void	VectorScale (const vector_t *in, float scale, vector_t *out);
void	AngleToVector (const vector_t *angles, vector_t *forward, vector_t *right, vector_t *up);
int		VectorCompare2 (const vector_t *v1, const vector_t *v2, float thresh);
void	MakeVectorPlanar(vector_t *in, vector_t *out, vector_t *norm);
void	VectorSet(vector_t *a, float x, float y, float z);


/*
Blah, macros suck ass. i can't even define my own funcs in a diff namespace cause 
these mess up the names.
*/

#define dot(a, b) ( a.x*b.x + a.y*b.y + a.z*b.z )
#define VectorCopy(a, b)    { b.x = a.x; b.y = a.y; b.z = a.z; }

#define VectorAdd(a, b, c)  { c.x=a.x+b.x; c.y=a.y+b.y; c.z=a.z+b.z; }
#define VectorAdd2(a, b)	{ a.x+=b.x; a.y+=b.y; a.z+=b.z; }

#define VectorSub(a, b, c)  { c.x=a.x-b.x; c.y=a.y-b.y; c.z=a.z-b.z; }
#define VectorInv(a)	    { a.x=-a.x; a.y=-a.y; a.z=-a.z; }
#define VectorInv2(a, b)	    { b.x=-a.x; b.y=-a.y; b.z=-a.z; }

void RotatePointAroundVector(vector_t *dst, vector_t *dir, vector_t *point, float rad);

#endif


