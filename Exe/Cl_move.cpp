#include "Cl_main.h"
#include "Cl_collision.h"


#define MAX_CLIP_PLANES 5
#define STOP_EPSILON 0.3f

const float CL_ROTATION_SENS = 0.05f;

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
// fake gravity
//	velocity->z -= MAXVELOCITY;


	// figure out what dir we want to go if we're folling a path
	if (m_campath != -1)
		calc_cam_path(m_campath, System::g_fcurTime - m_camtime, &m_gameClient.origin, dir, time);

	// can we clip through walls?  it's simple then
	if (!m_cvClip.ival)
	{
		VectorMA(&m_gameClient.origin, time, dir, &m_gameClient.origin);
		return;
	}

/*	if (PointContents(eye.origin) & CONTENTS_SOLID)
		m_pHud->HudPrintf(0, 30, 0, "SOLID");
	else if (PointContents(eye.origin) & CONTENTS_WATER)
		m_pHud->HudPrintf(0, 30, 0, "WATER");
*/


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
		VectorAdd(m_gameClient.origin, (*dir), end);
		tr = trace(m_gameClient.origin, end, &m_gameClient.mins, &m_gameClient.maxs);


		if (tr.fraction > 0)
		{
			VectorCopy(tr.endpos, m_gameClient.origin);
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





void CClient::MoveForward()
{
	static vector_t forward;
	AngleToVector (&m_gameClient.angle, &forward, NULL, NULL);
	VectorNormalize(&forward);
	VectorAdd2(desired_movement,forward);
}

void CClient::MoveBackward()
{
	static vector_t backword;
	AngleToVector (&m_gameClient.angle, &backword, NULL, NULL);
	VectorNormalize(&backword);
	VectorMA(&desired_movement, -1, &backword, &desired_movement);
}

void CClient::MoveRight()
{
	static vector_t right;
	AngleToVector (&m_gameClient.angle, NULL, &right, NULL);
	VectorNormalize(&right);
	VectorAdd2(desired_movement,right);
}

void CClient::MoveLeft()
{
	static vector_t left;
	AngleToVector (&m_gameClient.angle, NULL, &left, NULL);
	VectorNormalize(&left);
	VectorMA(&desired_movement, -1, &left, &desired_movement);
}

void CClient::RotateRight(const float &val)
{
	m_gameClient.angle.YAW += (val * CL_ROTATION_SENS);  
	if (m_gameClient.angle.YAW > PI)
		m_gameClient.angle.YAW -= 2*PI;
}

void CClient:: RotateLeft(const float &val)
{
	m_gameClient.angle.YAW -= (val * CL_ROTATION_SENS); 
	if (m_gameClient.angle.YAW < -PI)
		m_gameClient.angle.YAW += 2*PI;
}

void CClient::RotateUp(const float &val)
{
	m_gameClient.angle.PITCH +=  (val * CL_ROTATION_SENS);
	if (m_gameClient.angle.PITCH < -PI/2)
		m_gameClient.angle.PITCH = -PI/2;
	if (m_gameClient.angle.PITCH > PI/2)
		m_gameClient.angle.PITCH = PI/2;
}

void CClient:: RotateDown(const float &val)
{
	m_gameClient.angle.PITCH -=  (val * CL_ROTATION_SENS); 
	if (m_gameClient.angle.PITCH < -PI/2)
		m_gameClient.angle.PITCH = -PI/2;
	if (m_gameClient.angle.PITCH > PI/2)
		m_gameClient.angle.PITCH = PI/2;
}



/*
===========
follow a camera path
===========
*/
void CClient::CamPath()
{
	// find the head path node
	for (int ent=0; ent<g_pWorld->nentities; ent++)
	{
		if (strcmp(key_get_value(g_pWorld, ent, "classname"), "misc_camera_path_head") == 0)
		{
			m_campath = ent;
			m_camtime = System::g_fcurTime;

			vector_t origin;
			key_get_vector(g_pWorld, ent, "origin", origin);
			VectorCopy(origin, m_gameClient.origin); // move to first point of path
			return;
		}
	}
}

