#include "Sys_hdr.h"

#include "Com_vector.h"
#include "Com_world.h"
#include "Com_camera.h"

#include "Cl_base.h"
#include "Cl_game.h"
#include "Cl_hdr.h"

namespace {

const float CL_TILT_RESTORE_SPEED = 0.05f;
const float CL_TILT_SPEED = 0.1f;

}

/*
================================================
Step Functions. The angle vectors have already
been normalized.
================================================
*/
void CGameClient::MoveForward()
{	m_cmd.moveFlags |= ClCmd::MOVEFORWARD;
}

void CGameClient::MoveBackward()
{	m_cmd.moveFlags |= ClCmd::MOVEBACK;
}

void CGameClient::MoveRight()
{	m_cmd.moveFlags |= ClCmd::MOVERIGHT;
}

void CGameClient::MoveLeft()
{	m_cmd.moveFlags |= ClCmd::MOVELEFT;
}

void CGameClient::Jump()
{	
	if(m_bOnGround)
	{
//		ComPrintf("Jumped: %f\n", m_pClGame->GetCurTime());
		m_cmd.moveFlags |= ClCmd::JUMP;
	}
}

void CGameClient::Crouch()
{	m_cmd.moveFlags |= ClCmd::CROUCH;
}

/*
================================================
Rotate camera in the appropriate dir
================================================
*/
void CGameClient::RotateRight(const float &val)
{	m_vecDesiredAngles.y -= val;
}

void CGameClient:: RotateLeft(const float &val)
{	m_vecDesiredAngles.y += val;
}

void CGameClient::RotateUp(const float &val)
{	m_vecDesiredAngles.x += val;
}

void CGameClient:: RotateDown(const float &val)
{	m_vecDesiredAngles.x -= val;
}

/*
================================================
Update angles. called onces per frame
================================================
*/
void CGameClient::UpdateViewAngles(const float &time)
{
	m_pGameClient->angles.YAW += (m_vecDesiredAngles.YAW * time);  
	if (m_pGameClient->angles.YAW > PI)
		m_pGameClient->angles.YAW -= 2*PI;

	m_pGameClient->angles.PITCH +=  (m_vecDesiredAngles.PITCH * time);
	if (m_pGameClient->angles.PITCH < -PI/2)
		m_pGameClient->angles.PITCH = -PI/2;
	if (m_pGameClient->angles.PITCH > PI/2)
		m_pGameClient->angles.PITCH = PI/2;

	//Apply tilt
	if((m_cmd.moveFlags & ClCmd::MOVERIGHT) ||
	   (m_cmd.moveFlags & ClCmd::MOVELEFT))
	{
		if(m_cmd.moveFlags & ClCmd::MOVERIGHT)
		{
			m_pGameClient->angles.ROLL += (time * CL_TILT_SPEED);
			if(m_pGameClient->angles.ROLL > m_cvViewTilt.fval)
				m_pGameClient->angles.ROLL = m_cvViewTilt.fval;
		}

		if(m_cmd.moveFlags & ClCmd::MOVELEFT)
		{
			m_pGameClient->angles.ROLL -= (time * CL_TILT_SPEED);
			if(m_pGameClient->angles.ROLL < -m_cvViewTilt.fval)
				m_pGameClient->angles.ROLL = -m_cvViewTilt.fval;
		}
	}
	else
	{
		//Step back to normal view
		if(m_pGameClient->angles.ROLL >= 0)
		{	
			m_pGameClient->angles.ROLL -= (CL_TILT_RESTORE_SPEED * time);
			if(m_pGameClient->angles.ROLL < 0)
				m_pGameClient->angles.ROLL = 0;
		}
		else
		{
			m_pGameClient->angles.ROLL += (CL_TILT_RESTORE_SPEED * time);
			if(m_pGameClient->angles.ROLL >= 0)
				m_pGameClient->angles.ROLL = 0;
		}
	}
}


/*
================================================
Perform the actual move
================================================
*/
//void calc_cam_path(int &ent, float t, vector_t *origin, vector_t *dir, float &time);

void CGameClient::UpdatePosition(const float &time)
{

	// figure out what dir we want to go if we're folling a path
//	if (m_campath != -1)
//		calc_cam_path(m_campath, System::GetCurrentTime() - m_camtime, &m_pGameClient->origin, &dir, time);

	//Clipping should be on by default
//	if (m_cvClip.ival)
	EntMove::ClientMove(m_pGameClient, time);
///	else
//		EntMove::NoClipMove(m_pGameClient, dir, time);

	TraceInfo traceInfo;
	vector_t  end(m_pGameClient->origin);
	end.z -= 0.1f;

	m_pWorld->Trace(traceInfo,m_pGameClient->origin, end, m_pGameClient->mins,m_pGameClient->maxs);
	if(traceInfo.fraction != 1)
		m_bOnGround = true;
	else
	{
		m_bOnGround = false;
		m_pClGame->HudPrintf(0,160,0,"IN AIR");
	}

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

