#include "Com_fastmath.h"

using namespace FM;

uint  CFastMath::fastSqrtTable[0x10000];
CFastMath CFastMath::gfastMath;


CFastMath::CFastMath()
{	BuildSquareRootTable();
}

void  CFastMath::BuildSquareRootTable()
{
	union FastSqrtUnion
	{
		float f;
		unsigned int i;
	};
  
	FastSqrtUnion s;

	for (uint i = 0; i <= 0x7FFF; i++)
	{
		// Build a float with the bit pattern i as mantissa
		//  and an exponent of 0, stored as 127

		s.i = (i << 8) | (0x7F << 23);
		s.f = (float)sqrt(s.f);

		// Take the square root then strip the first 7 bits of
		//  the mantissa into the table

		fastSqrtTable[i + 0x8000] = (s.i & 0x7FFFFF);

		// Repeat the process, this time with an exponent of 1, 
		//  stored as 128

		s.i = (i << 8) | (0x80 << 23);
		s.f = (float)sqrt(s.f);

		fastSqrtTable[i] = (s.i & 0x7FFFFF);
	}
}



//Old stuff
#if 0

//float two = 2.0f;
//unsigned int fast_sqrt_table[0x10000];  // declare table of square roots 
/*
void  build_sqrt_table()
{
  unsigned int i;
  FastSqrtUnion s;
  
  for (i = 0; i <= 0x7FFF; i++)
  {
    
    // Build a float with the bit pattern i as mantissa
    //  and an exponent of 0, stored as 127
    
    s.i = (i << 8) | (0x7F << 23);
    s.f = (float)sqrt(s.f);
    
    // Take the square root then strip the first 7 bits of
    //  the mantissa into the table
    
    fast_sqrt_table[i + 0x8000] = (s.i & 0x7FFFFF);
    
    // Repeat the process, this time with an exponent of 1, 
    //  stored as 128
    
    s.i = (i << 8) | (0x80 << 23);
    s.f = (float)sqrt(s.f);
    
    fast_sqrt_table[i] = (s.i & 0x7FFFFF);
  }
}
*/



// r = 1/p
/*#define FP_INV(r,p)                                                          \
{                                                                            \
    int _i = 2 * FP_ONE_BITS - *(int *)&(p);                                 \
    r = *(float *)&_i;                                                       \
    r = r * (2.0f - (p) * r);                                                \
}
*/

/*
================================================
The following comes from Vincent Van Eeckhout
Thanks for sending us the code!
It's the same thing in assembly but without this C-needed line:
    r = *(float *)&_i;
================================================
*/
//extern float two;

/*
#define FP_INV2(r,p)                     \
{                                        \
	__asm { mov		eax,0x7F000000	  }; \
	__asm { sub		eax,dword ptr [p] }; \
	__asm { mov		dword ptr [r],eax }; \
	__asm { fld		dword ptr [p]     }; \
	__asm { fmul	dword ptr [r]     }; \
	__asm { fsubr	[two]             }; \
	__asm { fmul	dword ptr [r]     }; \
	__asm { fstp	dword ptr [r]     }; \
}
*/

inline float FP_INV(float &p)
{
    int _i = 2 * FP_ONE_BITS - *(int *)&(p);
    float r = *(float *)&_i;
    r = r * (2.0f - (p) * r);                   
	return r;
}

inline void FP_INV(float &r, float &p)
{
	static float two = 2.0f;

	__asm { mov		eax,0x7F000000	  }; 
	__asm { sub		eax,dword ptr [p] }; 
	__asm { mov		dword ptr [r],eax }; 
	__asm { fld		dword ptr [p]     }; 
	__asm { fmul	dword ptr [r]     }; 
	__asm { fsubr	[two]             }; 
	__asm { fmul	dword ptr [r]     }; 
	__asm { fstp	dword ptr [r]     }; 
}

/////////////////////////////////////////////////


#define FP_EXP(e,p)                                                          \
{                                                                            \
    int _i;                                                                  \
    e = -1.44269504f * (float)0x00800000 * (p);                              \
    _i = (int)e + 0x3F800000;                                                \
    e = *(float *)&_i;                                                       \
}

#define FP_NORM_TO_BYTE(i,p)                                                 \
{                                                                            \
    float _n = (p) + 1.0f;                                                   \
    i = *(int *)&_n;                                                         \
    if (i >= 0x40000000)     i = 0xFF;                                       \
    else if (i <=0x3F800000) i = 0;                                          \
    else i = ((i) >> 15) & 0xFF;                                             \
}


 inline ulong FP_NORM_TO_BYTE2(float p)                                                 
{                                                                            
	float fpTmp = p + 1.0f;                                                      
	return ((*(unsigned *)&fpTmp) >> 15) & 0xFF;  
}

inline ulong FP_NORM_TO_BYTE3(float p)     
{
	float ftmp = p + 12582912.0f;                                                      
	return ((*(unsigned long *)&ftmp) & 0xFF);
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

//extern unsigned int fast_sqrt_table[0x10000];  // declare table of square roots 
//void  build_sqrt_table();

inline float fastsqrt(float n)
{
	if (FP_BITS(n) == 0)
		return 0.0;                 // check for square root of 0
  
	FP_BITS(n) = CFastMath::fastSqrtTable[(FP_BITS(n) >> 8) & 0xFFFF] | ((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);
	return n;
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

#endif