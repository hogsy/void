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

