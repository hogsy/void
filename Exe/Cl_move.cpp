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

const int	CL_VIEWHEIGHT = 22;

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
		m_cmd.moveFlags |= ClCmd::JUMP;
}

void CGameClient::Crouch()
{	m_cmd.moveFlags |= ClCmd::CROUCH;
}

void CGameClient::Walk()
{	
ComPrintf("WALKING");
	m_cmd.moveFlags |= ClCmd::WALK;
}

/*
================================================
Rotate camera in the appropriate dir
================================================
*/
void CGameClient::RotateRight(const float &val)
{	m_vecDesiredAngles.y -= (val*m_fFrameTime);
}

void CGameClient:: RotateLeft(const float &val)
{	m_vecDesiredAngles.y += (val*m_fFrameTime);
}

void CGameClient::RotateUp(const float &val)
{	m_vecDesiredAngles.x += (val*m_fFrameTime);
}

void CGameClient:: RotateDown(const float &val)
{	m_vecDesiredAngles.x -= (val*m_fFrameTime);
}

/*
================================================
Update angles. called onces per frame
================================================
*/
void CGameClient::UpdateViewAngles()
{
//	m_vecDesiredAngles.YAW *= ;  
//	m_vecDesiredAngles.PITCH *= m_fFrameTime;  


//	m_pGameClient->angles.YAW += (m_vecDesiredAngles.YAW * m_fFrameTime);  
	m_pGameClient->angles.YAW += m_vecDesiredAngles.YAW;
	if (m_pGameClient->angles.YAW > PI)
		m_pGameClient->angles.YAW -= 2*PI;

//	m_pGameClient->angles.PITCH +=  (m_vecDesiredAngles.PITCH * m_fFrameTime);
	m_pGameClient->angles.PITCH +=  m_vecDesiredAngles.PITCH;
	if (m_pGameClient->angles.PITCH <= (-PI * 0.4f))
		m_pGameClient->angles.PITCH = (-PI * 0.4f);
	if (m_pGameClient->angles.PITCH > PI/2)
		m_pGameClient->angles.PITCH = PI/2;

	//Apply tilt
	if((m_cmd.moveFlags & ClCmd::MOVERIGHT) ||
	   (m_cmd.moveFlags & ClCmd::MOVELEFT))
	{
		if(m_cmd.moveFlags & ClCmd::MOVERIGHT)
		{
			m_pGameClient->angles.ROLL += (CL_TILT_SPEED * m_fFrameTime);
			if(m_pGameClient->angles.ROLL > m_cvViewTilt->fval)
				m_pGameClient->angles.ROLL = m_cvViewTilt->fval;
		}

		if(m_cmd.moveFlags & ClCmd::MOVELEFT)
		{
			m_pGameClient->angles.ROLL -= (CL_TILT_SPEED * m_fFrameTime);
			if(m_pGameClient->angles.ROLL < -m_cvViewTilt->fval)
				m_pGameClient->angles.ROLL = -m_cvViewTilt->fval;
		}
	}
	else
	{
		//Step back to normal view
		if(m_pGameClient->angles.ROLL >= 0)
		{	
			m_pGameClient->angles.ROLL -= (CL_TILT_RESTORE_SPEED * m_fFrameTime);
			if(m_pGameClient->angles.ROLL < 0)
				m_pGameClient->angles.ROLL = 0;
		}
		else
		{
			m_pGameClient->angles.ROLL += (CL_TILT_RESTORE_SPEED * m_fFrameTime);
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
void CGameClient::UpdatePosition()
{
/*	if(!m_cvClip.bval)
	{
		EntMove::NoClipMove(m_pGameClient, m_pGameClient->velocity, m_fFrameTime);
		return;
	}
*/

	EntMove::ClientMove(m_pGameClient, m_fFrameTime);

	TraceInfo traceInfo;
	vector_t  end(m_pGameClient->origin);
	end.z -= 0.1f;

	m_pWorld->Trace(traceInfo,m_pGameClient->origin, end, m_pGameClient->mins,m_pGameClient->maxs);
	if(traceInfo.fraction != 1)
		m_bOnGround = true;
	else
		m_bOnGround = false;

	m_pCamera->origin = m_pGameClient->origin;
	
	//Need a better transition
	if(!(m_cmd.moveFlags & ClCmd::CROUCH))
		m_pCamera->origin.z += CL_VIEWHEIGHT;


/*	
	if(EntMove::ClientMove(m_pGameClient, time) == 2)
	{
ComPrintf("Hit step!");

		bool stepped = false;

		//move up a bit and try once more
		vector_t vecOldPos(m_pGameClient->origin); 
		vector_t vecOldVelocity(m_pGameClient->velocity);

		//can we move up ?
		m_pGameClient->velocity.x = 0;
		m_pGameClient->velocity.y = 0;
		m_pGameClient->velocity.z = 200 * time;

		if(EntMove::ClientMove(m_pGameClient,time) == 1)
		{
ComPrintf("Moved up!");
			m_pGameClient->velocity = vecOldVelocity;
			m_pGameClient->velocity.z = 0;

			//Can we move forward now ?
			if(EntMove::ClientMove(m_pGameClient, time) == 1)
			{
ComPrintf("Stepped up !");
				stepped = true;
			}
		}

		//Go back to original pos
		if(!stepped)
		{
			m_pGameClient->velocity = vecOldVelocity;
			m_pGameClient->origin = vecOldPos;
		}
	}
*/
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

