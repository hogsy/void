#ifndef VOID_VECTOR_H
#define VOID_VECTOR_H

#include <math.h>

#ifndef PI
#define PI		3.14159265358979323846f
#endif

#define ROLL	z	// around z axis
#define PITCH	x	// around x axis
#define YAW		y	// around y axis

inline void DEG2RAD(float &x) {	x = (x * PI)/ 180.0f; }
inline void RAD2DEG(float &x) { x = (x* 180.0f)/ PI; }

/*
======================================
Vector class
======================================
*/
struct vector_t //Vec3d
{
	float x, y, z;

	inline vector_t() : x(0), y(0), z(0) {}
	inline vector_t(const vector_t &v) : x (v.x), y(v.y), z(v.z) {}
	inline ~vector_t() {}

	//Operators
	//Assignment
	inline vector_t & operator = (const vector_t &v) 
	{
		if(&v != this)
		{	x = v.x; 
			y = v.y; 
			z = v.z;	
		}
		return *this;
	}

	//Addition
	inline vector_t & operator += (const vector_t &a)
	{	
		x +=a.x;
		y +=a.y;
		z +=a.z;
		return *this;
	}

	//Substraction
	inline vector_t & operator -= (const vector_t &a)
	{	
		x -=a.x;
		y -=a.y;
		z -=a.z;
		return *this;
	}

	inline float Length() const
	{	return (float)sqrt((x*x)+(y*y)+(z*z));	
	}

	inline void Set(float ix, float iy, float iz)
	{
		x = ix;
		y = iy;
		z = iz;
	}

	inline void Scale(float scale)
	{	
		x = x*scale;
		y = y*scale;
		z = z*scale;
	}

	inline void Scale(vector_t &out,float scale)
	{	
		out.x = x*scale;
		out.y = y*scale;
		out.z = z*scale;
	}

	inline void Inverse()
	{	
		x = -x;
		y = -y;
		z = -z;
	}

	inline void Inverse(vector_t &out) const
	{	
		out.x = -x;
		out.y = -y;
		out.z = -z;
	}

	float Normalize();
	float Normalize(vector_t &out);
	void  VectorMA (const vector_t &veca, float scale, const vector_t &vecb);
	void  AngleToVector(vector_t * forward, vector_t * right, vector_t *up);

	//======================================================================================
	//Friend funcs. These are the ones that always operate on more than one vector.
	//======================================================================================

	//Addition
	inline friend vector_t operator + (const vector_t &a, const vector_t &b)
	{	
		vector_t sum(a);
		return sum += b;
	}

	//Substractor
	inline friend vector_t operator - (const vector_t &a, const vector_t &b)
	{	
		vector_t diff(a);
		return diff -= b;
	}

	friend float VectorDistance(const vector_t &a, const vector_t &b)
	{
		vector_t dist(a - b);
		return dist.Length();
	}

	friend float DotProduct(const vector_t &a, const vector_t &b)
	{	return ( a.x*b.x + a.y*b.y + a.z*b.z );
	}

	//Comparision
	friend int   operator == (const vector_t &v1, const vector_t &v2);
	friend int   VectorCompare (const vector_t &v1, const vector_t &v2, float thresh);
	
	friend void  CrossProduct(const vector_t &a, const vector_t &b, vector_t &normal);
/*	friend void  MakeVectorPlanar(const vector_t &in, vector_t &out, const vector_t &norm);
	friend void  RotatePointAroundVector(vector_t *dst, vector_t *dir, vector_t *point, float rad);
*/
};

/*
======================================

======================================
*/
struct plane_t
{
	plane_t () {}
	~plane_t () {}

	vector_t	norm;	// normal to plane
	float		d;		// distance from origin
};


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
Blah, macros suck ass. can't even define my own funcs in a diff namespace cause 
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


