#ifndef VOID_VECTOR_H
#define VOID_VECTOR_H

#include "Com_fastmath.h"

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
	inline vector_t(float ix, float iy, float iz) : x(ix), y(iy), z(iz){}
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
	{	return fastsqrt((x*x)+(y*y)+(z*z));	
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
Plane
======================================
*/
struct plane_t
{
	plane_t () {}
	~plane_t () {}

	vector_t	norm;	// normal to plane
	float		d;		// distance from origin
};


void	MakeVectorPlanar(vector_t *in, vector_t *out, vector_t *norm);
void	RotatePointAroundVector(vector_t *dst, vector_t *dir, vector_t *point, float rad);

/*
Blah, macros suck ass. can't even define my own funcs in a diff namespace cause 
these mess up the names.
*/
#define dot(a, b) ( a.x*b.x + a.y*b.y + a.z*b.z )

#endif


