#include "Cl_main.h"
#include "Cl_collision.h"


#define MAX_CLIP_PLANES 5
#define STOP_EPSILON 0.3f

extern world_t *g_pWorld;





int PointContents(vector_t &v)
{
	int n=0;
	float d;

	do
	{
		// test to this nodes plane
		d = dot(g_pWorld->planes[g_pWorld->nodes[n].plane].norm, v) - g_pWorld->planes[g_pWorld->nodes[n].plane].d;

		if (d>=0)
			n = g_pWorld->nodes[n].children[0];
		else
			n = g_pWorld->nodes[n].children[1];

		// if we found a leaf, it's what we want
		if (n<=0)
			return g_pWorld->leafs[-n].contents;

	} while (1);

	return 0;
}




void calc_cam_path(int &ent, float t, vector_t *origin, vector_t *dir, float &time)
{
	t /= 2.0f;

	int eq = (int)t;
	if (eq >= key_get_int(g_pWorld, ent, "num_eqs"))
	{
		ent = -1;	// done with the path
		VectorSet(dir, 0, 0, 0);
		return;
	}

	// find the point we wanna move to - get our equation
	float powers[4];
	powers[0] = 1;
	powers[1] = powers[0] * t;
	powers[2] = powers[1] * t;
	powers[3] = powers[2] * t;

	vector_t comp, p;
	VectorSet(&p, 0, 0, 0);
	char name[] = "e00t0";
	name[1] = eq/10 + '0';
	name[2] = eq%10 + '0';
	for (int i=0; i<4; i++)
	{
		name[4] = i + '0';
		key_get_vector(g_pWorld, ent, name, comp);


		p.x += powers[i] * comp.x;
		p.y += powers[i] * comp.y;
		p.z += powers[i] * comp.z;
	}

	VectorSub(p, (*origin), (*dir));
	time = VectorNormalize(dir);
}


// FIXME - this should be an entitiy move, not a client move
void CClient::Move(vector_t *dir, float time)
{
	// not in a sector, so just fly through everything
/*	if (!eye.origin.sector)
	{
		VectorMA(&eye.origin, time, velocity, &eye.origin);
		return;
	}
*/
// fake gravity
//	velocity->z -= MAXVELOCITY;


	// figure out what dir we want to go if we're folling a path
	if (m_campath != -1)
		calc_cam_path(m_campath, g_fcurTime - m_camtime, &eye.origin, dir, time);

	// can we clip through walls?  it's simple then
	if ((int)m_noclip->value)
	{
		VectorMA(&eye.origin, time, dir, &eye.origin);
		return;
	}

	if (PointContents(eye.origin) & CONTENTS_SOLID)
		m_rHud->HudPrintf(0, 30, 0, "SOLID");


	// regular collision
	vector_t	hitplanes[MAX_CLIP_PLANES];	// all the normals of the planes we've hit
	int			bumps, hits=0;				// number of planes we've hit, and number we're touching
	trace_t		tr;							// the current trace
	vector_t	end;						// where we want to end up
	vector_t	primal_dir;			// dir we originally wanted
	float d;

	VectorScale(dir, time, dir);
	VectorCopy((*dir), primal_dir);

	for (bumps=0; bumps<MAX_CLIP_PLANES; bumps++)
	{
		VectorAdd(eye.origin, (*dir), end);
		tr = trace(eye.origin, end, &eye.mins, &eye.maxs);


		if (tr.fraction > 0)
		{
			VectorCopy(tr.endpos, eye.origin);
			hits = 0;
		}


		// we're done moving
		if ((!tr.plane) || (hits==2))	// full move or this is our 3rd plane
			break;

		VectorCopy(tr.plane->norm, hitplanes[hits]);
		hits++;


		// we're only touching 1 plane - project velocity onto it
		if (hits==1)
			MakeVectorPlanar(dir, dir, &hitplanes[0]);

		// we have to set velocity along crease
		else
		{
			vector_t tmp;
			_CrossProduct(&hitplanes[0], &hitplanes[1], &tmp);
			d = dot(tmp, (*dir));
			VectorScale (&tmp, d, dir);
		}

		// make sure we're still going forward
		if (dot((*dir), primal_dir) <= 0)
		{
			VectorSet(dir, 0, 0, 0);
			break;
		}
	}



/*	VectorMA(&eye.origin, timeleft, velocity, &end);
	tr = trace(&eye.origin, &end, &eye.mins, &eye.maxs);

	tr;
	VectorCopy(tr.endpos, eye.origin);
	return;
*/
/*
	for (bumps = 0; bumps < MAX_CLIP_PLANES; bumps++)
	{
		VectorMA(&eye.origin, timeleft, velocity, &end);
		tr = trace(&eye.origin, &end, &eye.mins, &eye.maxs);

		// we moved - move to where the trace ended
		if (tr.fraction > 0)
		{
			VectorCopy(tr.endpos, eye.origin);
			VectorCopy((*velocity), original_velocity);
			hits = 0;
		}

		// moved the entire distance
		if (tr.fraction == 1)
			break;



		timeleft -= timeleft * tr.fraction;

		VectorCopy(tr.plane->norm, hitplanes[hits]);
		hits++;


		for (i = 0; i < hits; i++)
		{
			// make a velocity vector parallel to all hit planes
			MakeVectorPlanar(&original_velocity, &new_velocity, &hitplanes[i]);

// FIXME - how do i want to do this?
#if 0
			if ((new_velocity.x > -STOP_EPSILON) && (new_velocity.x < STOP_EPSILON)) new_velocity.x = 0;
			if ((new_velocity.y > -STOP_EPSILON) && (new_velocity.y < STOP_EPSILON)) new_velocity.y = 0;
			if ((new_velocity.z > -STOP_EPSILON) && (new_velocity.z < STOP_EPSILON)) new_velocity.z = 0;
#endif
			if ((VectorLength(&new_velocity) * timeleft) < STOP_EPSILON)
			{
				VectorSet(&new_velocity, 0, 0, 0);
				return;
			}

			for (j = 0; j < hits; j++)
			{
				if ((j != i) && !VectorCompare(&hitplanes[i], &hitplanes[j]))
				{
					if (dot(new_velocity, hitplanes[j]) < 0)
						break;	// going against a hit plane
				}
			}

			if (j == hits)
				break;	// found a good new velocity - not against any planes we've hit
		}

		if (i != hits)	// one of the velocities was good - not against any hit planes
		{
			VectorCopy(new_velocity, (*velocity));
//			ComPrintf("going away from all planes\n");
		}

		else
		{
			if (hits != 2)
			{
				// running into a corner
//				ComPrintf("num clip planes = %i\n", hits);
				return;
			}


//			ComPrintf("using cross product thingy\n");
			_CrossProduct(&hitplanes[0], &hitplanes[1], &dir);
			d = dot(dir, original_velocity);
			VectorScale (&dir, d, velocity);

		}

// FIXME - add friction to velocity here

		if (dot(original_velocity, primal_velocity) <= 0)
		{
			VectorSet(velocity, 0, 0, 0);
			ComPrintf("against original velocity\n");
			return;
		}
	}
*/
}







void MoveForward(int argc, char** argv)
{
	static vector_t forward;
	AngleToVector (&g_pClient->eye.angles, &forward, NULL, NULL);
	VectorNormalize(&forward);
//	VectorScale(&forward, g_pClient->m_acceleration, &forward);
	VectorAdd2(g_pClient->desired_movement,forward);
}


void MoveBackward(int argc, char** argv)
{
	static vector_t backword;
	AngleToVector (&g_pClient->eye.angles, &backword, NULL, NULL);
	VectorNormalize(&backword);
//	VectorScale(&backword, -g_pClient->m_acceleration, &backword);
//	VectorAdd2(g_pClient->desired_movement,backword);
	VectorMA(&g_pClient->desired_movement, -1, &backword, &g_pClient->desired_movement);
}


void MoveRight(int argc, char** argv)
{
	static vector_t right;
	AngleToVector (&g_pClient->eye.angles, NULL, &right, NULL);
	VectorNormalize(&right);
//	VectorScale(&right, g_pClient->m_acceleration, &right);
	VectorAdd2(g_pClient->desired_movement,right);
}


void MoveLeft(int argc, char** argv)
{
	static vector_t left;
	AngleToVector (&g_pClient->eye.angles, NULL, &left, NULL);
	VectorNormalize(&left);
//	VectorScale(&left, -g_pClient->m_acceleration, &left);
//	VectorAdd2(g_pClient->desired_movement,left);
	VectorMA(&g_pClient->desired_movement, -1, &left, &g_pClient->desired_movement);
}

void KRotateRight(int argc, char** argv)
{	g_pClient->RotateRight(5.0);
}

void KRotateLeft(int argc, char** argv)
{	g_pClient->RotateLeft(5.0);
}

void KRotateUp(int argc, char** argv)
{	g_pClient->RotateUp(5.0);
}

void KRotateDown(int argc, char** argv)
{	g_pClient->RotateDown(5.0);
}





void CClient::RotateRight(float val)
{
	eye.angles.YAW += g_fframeTime * val;

	if (eye.angles.YAW > PI)
		eye.angles.YAW -= 2*PI;
}


void CClient:: RotateLeft(float val)
{
	eye.angles.YAW -= g_fframeTime * val;

	if (eye.angles.YAW < -PI)
		eye.angles.YAW += 2*PI;
}







void CClient::RotateUp(float val)
{
	eye.angles.PITCH += g_fframeTime * val;

	if (eye.angles.PITCH < -PI/2)
		eye.angles.PITCH = -PI/2;
	if (eye.angles.PITCH > PI/2)
		eye.angles.PITCH = PI/2;
}


void CClient:: RotateDown(float val)
{
	eye.angles.PITCH -= g_fframeTime * val;

	if (eye.angles.PITCH < -PI/2)
		eye.angles.PITCH = -PI/2;
	if (eye.angles.PITCH > PI/2)
		eye.angles.PITCH = PI/2;
}


/*






void MoveForward(int argc, char** argv)
{
	static vector_t forward;
	AngleToVector (&g_pClient->eye.angles, &forward, NULL, NULL);
	VectorNormalize(&forward);
	VectorScale(&forward, g_pClient->m_acceleration, &forward);
	VectorAdd2(g_pClient->desired_movement,forward);
}


void MoveBackward(int argc, char** argv)
{
	static vector_t backword;
	AngleToVector (&g_pClient->eye.angles, &backword, NULL, NULL);
	VectorNormalize(&backword);
	VectorScale(&backword, -g_pClient->m_acceleration, &backword);
	VectorAdd2(g_pClient->desired_movement,backword);
}


void MoveRight(int argc, char** argv)
{
	static vector_t right;
	AngleToVector (&g_pClient->eye.angles, NULL, &right, NULL);
	VectorNormalize(&right);
	VectorScale(&right, g_pClient->m_acceleration, &right);
	VectorAdd2(g_pClient->desired_movement,right);
}


void MoveLeft(int argc, char** argv)
{
	static vector_t left;
	AngleToVector (&g_pClient->eye.angles, NULL, &left, NULL);
	VectorNormalize(&left);
	VectorScale(&left, -g_pClient->m_acceleration, &left);
	VectorAdd2(g_pClient->desired_movement,left);
}

void KRotateRight(int argc, char** argv)
{	g_pClient->RotateRight(5.0);
}

void KRotateLeft(int argc, char** argv)
{	g_pClient->RotateLeft(5.0);
}

void KRotateUp(int argc, char** argv)
{	g_pClient->RotateUp(5.0);
}

void KRotateDown(int argc, char** argv)
{	g_pClient->RotateDown(5.0);
}





void CClient::RotateRight(float val)
{
	eye.angles.YAW += g_fframeTime * val;

	if (eye.angles.YAW > PI)
		eye.angles.YAW -= 2*PI;
}


void CClient:: RotateLeft(float val)
{
	eye.angles.YAW -= g_fframeTime * val;

	if (eye.angles.YAW < -PI)
		eye.angles.YAW += 2*PI;
}


void CClient::RotateUp(float val)
{
	eye.angles.PITCH += g_fframeTime * val;

	if (eye.angles.PITCH < -PI/2)
		eye.angles.PITCH = -PI/2;
	if (eye.angles.PITCH > PI/2)
		eye.angles.PITCH = PI/2;
}


void CClient:: RotateDown(float val)
{
	eye.angles.PITCH -= g_fframeTime * val;

	if (eye.angles.PITCH < -PI/2)
		eye.angles.PITCH = -PI/2;
	if (eye.angles.PITCH > PI/2)
		eye.angles.PITCH = PI/2;
}

  */