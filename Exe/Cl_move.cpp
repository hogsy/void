#include "Sys_hdr.h"

#include "Com_vector.h"
#include "Com_world.h"
#include "Com_camera.h"

#include "Cl_base.h"
#include "Cl_game.h"

const float CL_ROTATION_SENS = 0.05f;

/*
================================================
Step Functions. The angle vectors have already
been normalized.
================================================
*/
void CGameClient::MoveForward()
{	m_vecDesiredMove += m_vecForward;
}

void CGameClient::MoveBackward()
{	m_vecDesiredMove -= m_vecForward;
}

void CGameClient::MoveRight()
{	m_vecDesiredMove += m_vecRight;
}

void CGameClient::MoveLeft()
{	m_vecDesiredMove -= m_vecRight;
}


/*
================================================
Rotate camera in the appropriate dir
================================================
*/
void CGameClient::RotateRight(const float &val)
{
	m_pGameClient->angles.YAW += (val * CL_ROTATION_SENS);  
	if (m_pGameClient->angles.YAW > PI)
		m_pGameClient->angles.YAW -= 2*PI;
}

void CGameClient:: RotateLeft(const float &val)
{
	m_pGameClient->angles.YAW -= (val * CL_ROTATION_SENS); 
	if (m_pGameClient->angles.YAW < -PI)
		m_pGameClient->angles.YAW += 2*PI;
}

void CGameClient::RotateUp(const float &val)
{
	m_pGameClient->angles.PITCH +=  (val * CL_ROTATION_SENS);
	if (m_pGameClient->angles.PITCH < -PI/2)
		m_pGameClient->angles.PITCH = -PI/2;
	if (m_pGameClient->angles.PITCH > PI/2)
		m_pGameClient->angles.PITCH = PI/2;
}

void CGameClient:: RotateDown(const float &val)
{
	m_pGameClient->angles.PITCH -=  (val * CL_ROTATION_SENS); 
	if (m_pGameClient->angles.PITCH < -PI/2)
		m_pGameClient->angles.PITCH = -PI/2;
	if (m_pGameClient->angles.PITCH > PI/2)
		m_pGameClient->angles.PITCH = PI/2;
}


/*
================================================
Perform the actual move
================================================
*/
//void calc_cam_path(int &ent, float t, vector_t *origin, vector_t *dir, float &time);

void CGameClient::Move(vector_t &dir, float time)
{
	// figure out what dir we want to go if we're folling a path
//	if (m_campath != -1)
//		calc_cam_path(m_campath, System::GetCurrentTime() - m_camtime, &m_pGameClient->origin, &dir, time);

	//Clipping should be on by default
//	if (m_cvClip.ival)
		EntMove::ClientMove(m_pGameClient, dir, time);
///	else
//		EntMove::NoClipMove(m_pGameClient, dir, time);
}

/*
================================================
follow a camera path
================================================
*/
void CGameClient::CamPath()
{
/*	// find the head path node
	for (int ent=0; ent<m_pWorld->nentities; ent++)
	{
		if (strcmp(m_pWorld->GetKeyString(ent, "classname"), "misc_camera_path_head") == 0)
		{
			m_campath = ent;
			m_camtime = System::GetCurTime();

			vector_t origin;
			m_pWorld->GetKeyVector(ent, "origin", origin);
			VectorCopy(origin, m_pGameClient->origin); // move to first point of path
			return;
		}
	}
*/
}


/*
void calc_cam_path(int &ent, float t, vector_t *origin, vector_t *dir, float &time)
{

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

}
*/
