#ifndef COM_VOID_CLIENT_CAMERA
#define COM_VOID_CLIENT_CAMERA

#include "Com_vector.h"

/*
======================================
The camera class.
keeps refs to client angle/position data. 
Client creates this locally when starting into a game
======================================
*/
class CCamera
{
public:

	CCamera(vector_t & rorigin,
			vector_t & rangles,
			vector_t & rblend
//			,vector_t & rforward,
//			vector_t & rright,
			//vector_t & rup
			): origin(rorigin), angles(rangles), blend(rblend)
							 //,forward(rforward), right(rright), up(rup)
	{}
	
	~CCamera() {} 

	vector_t & origin;
	vector_t & angles;
	vector_t & blend;
	
/*	vector_t & forward;
	vector_t & right;
	vector_t & up;
*/
};

#endif
