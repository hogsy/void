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

	CCamera(vector_t & refAngles,
			vector_t & refForward,
			vector_t & refRight,
			vector_t & refUp,
			vector_t & refVelocity):
				angles(refAngles), 
				forward(refForward), right(refRight), up(refUp),
				velocity(refVelocity)
	{}
	
	~CCamera() {} 

	//Always tied to an entity
	vector_t & angles;
	vector_t & forward;
	vector_t & right;
	vector_t & up;
	vector_t & velocity;

	vector_t origin;		//includes viewheight
	vector_t blend;
};

#endif
