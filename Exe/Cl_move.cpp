#include "Cl_main.h"
#include "Com_world.h"

const float CL_ROTATION_SENS = 0.05f;


void calc_cam_path(int &ent, float t, vector_t *origin, vector_t *dir, float &time);

// FIXME - this should be an entitiy move, not a client move
void CClient::Move(vector_t &dir, float time)
{
	// figure out what dir we want to go if we're folling a path
//	if (m_campath != -1)
//		calc_cam_path(m_campath, System::GetCurrentTime() - m_camtime, &m_pClient->origin, &dir, time);

	//Clipping should be on by default
	if (m_cvClip.ival)
		EntMove::ClientMove(m_pClient, dir, time);
	else
		EntMove::NoClipMove(m_pClient, dir, time);
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
			m_camtime = System::GetCurTime();

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

