#ifndef VOID_FAST_MATH_H
#define VOID_FAST_MATH_H

/*
================================================
Nvidias fast math routines
================================================
*/
#include "Com_defs.h"
#include <math.h>

/*
================================================
Floating point macros
================================================
*/

#define FP_BITS(fp) (*(DWORD *)&(fp))
#define FP_ABS_BITS(fp) (FP_BITS(fp)&0x7FFFFFFF)
#define FP_SIGN_BIT(fp) (FP_BITS(fp)&0x80000000)
#define FP_ONE_BITS 0x3F800000
#define FP_EXP(e,p)                                                          \
{                                                                            \
    int _i;                                                                  \
    e = -1.44269504f * (float)0x00800000 * (p);                              \
    _i = (int)e + 0x3F800000;                                                \
    e = *(float *)&_i;                                                       \
}

/*
================================================
Floating point util funcs
================================================
*/
namespace FM {

inline float INV(float p)
{
// devvoid needs more accuracy
#ifdef DEVVOID
	return (1.0f/p);
#else
	static float two = 2.0f;
	float r;

	__asm { mov		eax,0x7F000000	  }; 
	__asm { sub		eax,dword ptr [p] }; 
	__asm { mov		dword ptr [r],eax }; 
	__asm { fld		dword ptr [p]     }; 
	__asm { fmul	dword ptr [r]     }; 
	__asm { fsubr	[two]             }; 
	__asm { fmul	dword ptr [r]     }; 
	__asm { fstp	dword ptr [r]     }; 
	return r;
#endif
}

inline float INV(const float &p)
{
// devvoid needs more accuracy
#ifdef DEVVOID
	return (1.0f/p);
#else
	static float two = 2.0f;
	float r;

	__asm { mov		eax,0x7F000000	  }; 
	__asm { sub		eax,dword ptr [p] }; 
	__asm { mov		dword ptr [r],eax }; 
	__asm { fld		dword ptr [p]     }; 
	__asm { fmul	dword ptr [r]     }; 
	__asm { fsubr	[two]             }; 
	__asm { fmul	dword ptr [r]     }; 
	__asm { fstp	dword ptr [r]     }; 
	return r;
#endif
}

inline ulong NORM_TO_BYTE(float p)
{
    float _n = (p) + 1.0f;
    ulong i = *(int *)&_n; 
    if (i >= 0x40000000)     i = 0xFF;
    else if (i <=0x3F800000) i = 0;
    else i = ((i) >> 15) & 0xFF;
	return i;
}

inline ulong NORM_TO_BYTE2(float p)                                                 
{                                                                            
	float fpTmp = p + 1.0f;                                                      
	return ((*(unsigned *)&fpTmp) >> 15) & 0xFF;  
}

inline ulong NORM_TO_BYTE3(float p)     
{
	float ftmp = p + 12582912.0f;                                                      
	return ((*(unsigned long *)&ftmp) & 0xFF);
}

// At the assembly level the recommended workaround for the second FIST bug is the same for the first; 
// inserting the FRNDINT instruction immediately preceding the FIST instruction. 
__forceinline void FloatToInt(int * int_pointer, float f) 
{
	__asm	fld  f
	__asm	mov  edx,int_pointer
	__asm	FRNDINT
	__asm	fistp dword ptr [edx];
}


/*
================================================
This will guarantee the creation of the table
before any of our code executes
================================================
*/
class CFastMath
{
public:
	
	static uint  fastSqrtTable[0x10000];

private:
	
	CFastMath();
	void  BuildSquareRootTable();
	static CFastMath	gfastMath;
};


inline float SquareRoot(float n)
{
#ifdef DEVVOID
	return sqrt(n);
#else
	if (FP_BITS(n) == 0)
		return 0.0;                 // check for square root of 0
  
	FP_BITS(n) = CFastMath::fastSqrtTable[(FP_BITS(n) >> 8) & 0xFFFF] | 
				((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);
	return n;
#endif
}

inline float SquareRoot(const float &n)
{
#ifdef DEVVOID
	return sqrt(n);
#else
	if (FP_BITS(n) == 0)
		return 0.0;                 // check for square root of 0
  
	FP_BITS(n) = CFastMath::fastSqrtTable[(FP_BITS(n) >> 8) & 0xFFFF] | 
				((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);
	return n;
#endif
}

}

#endif
