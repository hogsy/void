#include "Cl_main.h"
#include "Com_world.h"

#define MAX_CLIP_PLANES 5
#define STOP_EPSILON 0.3f

const float CL_ROTATION_SENS = 0.05f;


void CClient::EntityMove(const vector_t &origin, 
						 const vector_t &mins, 
						 const vector_t &maxs,
						 vector_t &dir, 
						 float time)
{
	
	// all the normals of the planes we've hit
	vector_t	hitplanes[MAX_CLIP_PLANES];	
	int			hits=0;				// number of planes we've hit, and number we're touching
	vector_t	end;				// where we want to end up
	vector_t	primal_dir;			// dir we originally wanted
	TraceInfo	tr;
	float d;

	Void3d::VectorScale(dir,dir, time);
	Void3d::VectorSet(primal_dir, dir);
	
	for(int bumps=0; bumps<MAX_CLIP_PLANES; bumps++)
	{
		VectorAdd(origin, dir, end);

		m_pWorld->Trace(tr, origin, end, mins, maxs);
		if (tr.fraction > 0)
		{
			Void3d::VectorSet(m_pClient->origin, tr.endpos);
			hits = 0;
		}

		// we're done moving
		if ((!tr.plane) || (hits==2))	// full move or this is our 3rd plane
			break;

		Void3d::VectorSet(hitplanes[hits],tr.plane->norm);
		hits++;

		// we're only touching 1 plane - project velocity onto it
		if (hits==1)
			MakeVectorPlanar(&dir, &dir, &hitplanes[0]);
		// we have to set velocity along crease
		else
		{
			vector_t tmp;
			_CrossProduct(&hitplanes[0], &hitplanes[1], &tmp);
			d = Void3d::DotProduct(tmp,dir);
			Void3d::VectorScale(dir,tmp, d);
		}

		// make sure we're still going forward
		if (Void3d::DotProduct(dir,primal_dir) <= 0)
		{
			Void3d::VectorSet(dir, 0, 0, 0);
			break;
		}
	}

}


void calc_cam_path(int &ent, float t, vector_t *origin, vector_t *dir, float &time);

// FIXME - this should be an entitiy move, not a client move
void CClient::Move(vector_t *dir, float time)
{
	// not in a sector, so just fly through everything
// fake gravity
//	velocity->z -= MAXVELOCITY;


	// figure out what dir we want to go if we're folling a path
	if (m_campath != -1)
		calc_cam_path(m_campath, System::g_fcurTime - m_camtime, &m_pClient->origin, dir, time);

	// can we clip through walls?  it's simple then
	if (!m_cvClip.ival)
	{
		VectorMA(&m_pClient->origin, time, dir, &m_pClient->origin);
		return;
	}
	EntityMove(m_pClient->origin, m_pClient->mins, m_pClient->maxs,	*dir, time);
}





void CClient::MoveForward()
{
	static vector_t forward;
	AngleToVector (&m_pClient->angles, &forward, NULL, NULL);
	VectorNormalize(&forward);
	VectorAdd2(desired_movement,forward);
}

void CClient::MoveBackward()
{
	static vector_t backword;
	AngleToVector (&m_pClient->angles, &backword, NULL, NULL);
	VectorNormalize(&backword);
	VectorMA(&desired_movement, -1, &backword, &desired_movement);
}

void CClient::MoveRight()
{
	static vector_t right;
	AngleToVector (&m_pClient->angles, NULL, &right, NULL);
	VectorNormalize(&right);
	VectorAdd2(desired_movement,right);
}

void CClient::MoveLeft()
{
	static vector_t left;
	AngleToVector (&m_pClient->angles, NULL, &left, NULL);
	VectorNormalize(&left);
	VectorMA(&desired_movement, -1, &left, &desired_movement);
}

void CClient::RotateRight(const float &val)
{
	m_pClient->angles.YAW += (val * CL_ROTATION_SENS);  
	if (m_pClient->angles.YAW > PI)
		m_pClient->angles.YAW -= 2*PI;
}

void CClient:: RotateLeft(const float &val)
{
	m_pClient->angles.YAW -= (val * CL_ROTATION_SENS); 
	if (m_pClient->angles.YAW < -PI)
		m_pClient->angles.YAW += 2*PI;
}

void CClient::RotateUp(const float &val)
{
	m_pClient->angles.PITCH +=  (val * CL_ROTATION_SENS);
	if (m_pClient->angles.PITCH < -PI/2)
		m_pClient->angles.PITCH = -PI/2;
	if (m_pClient->angles.PITCH > PI/2)
		m_pClient->angles.PITCH = PI/2;
}

void CClient:: RotateDown(const float &val)
{
	m_pClient->angles.PITCH -=  (val * CL_ROTATION_SENS); 
	if (m_pClient->angles.PITCH < -PI/2)
		m_pClient->angles.PITCH = -PI/2;
	if (m_pClient->angles.PITCH > PI/2)
		m_pClient->angles.PITCH = PI/2;
}




/*
===========
follow a camera path
===========
*/
void CClient::CamPath()
{
	// find the head path node
	for (int ent=0; ent<m_pWorld->nentities; ent++)
	{
		if (strcmp(m_pWorld->GetKeyString(ent, "classname"), "misc_camera_path_head") == 0)
		{
			m_campath = ent;
			m_camtime = System::g_fcurTime;

			vector_t origin;
			m_pWorld->GetKeyVector(ent, "origin", origin);
			VectorCopy(origin, m_pClient->origin); // move to first point of path
			return;
		}
	}
}




//======================================================================================
//======================================================================================



void calc_cam_path(int &ent, float t, vector_t *origin, vector_t *dir, float &time)
{
/*
	t /= 2.0f;

	int eq = (int)t;
	if (eq >=  m_pWorld->GetKeyInt(ent,"num_eqs"))
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
		m_pWorld->GetKeyVector(ent, name, comp);


		p.x += powers[i] * comp.x;
		p.y += powers[i] * comp.y;
		p.z += powers[i] * comp.z;
	}

	VectorSub(p, (*origin), (*dir));
	time = VectorNormalize(dir);
*/
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


