#include "Com_world.h"
#include "Game_defs.h"


const int MAX_CLIP_PLANES =5;
//#define STOP_EPSILON 0.3f


CWorld * CMoveType::m_pWorld=0;


void CMoveType::SetWorld(CWorld * pWorld)
{	m_pWorld = pWorld;
}



void CMoveType::NoClipMove(BaseEntity &ent, vector_t &dir, float time)
{	VectorMA(&ent.origin, time, &dir, &ent.origin);
}


void CMoveType::ClientMove(BaseEntity &ent, vector_t &dir, float time)
{
	// all the normals of the planes we've hit
	vector_t	hitplanes[MAX_CLIP_PLANES];	
	int			hits=0;				// number of planes we've hit, and number we're touching
	vector_t	end;				// where we want to end up
	vector_t	primal_dir;			// dir we originally wanted
	TraceInfo	tr;
	float d;

	dir.Scale(time);
	primal_dir = dir;
//	Void3d::VectorScale(dir,dir, time);
//	Void3d::VectorSet(primal_dir, dir);
	
	for(int bumps=0; bumps<MAX_CLIP_PLANES; bumps++)
	{
		VectorAdd(ent.origin, dir, end);

		m_pWorld->Trace(tr, ent.origin, end, ent.mins, ent.maxs);
		if (tr.fraction > 0)
		{
			//Void3d::VectorSet(ent.origin, tr.endpos);
			ent.origin = tr.endpos;
			hits = 0;
		}

		// we're done moving
		if ((!tr.plane) || (hits==2))	// full move or this is our 3rd plane
			break;

		//Void3d::VectorSet(hitplanes[hits],tr.plane->norm);
		hitplanes[hits] =tr.plane->norm;
		hits++;

		// we're only touching 1 plane - project velocity onto it
		if (hits==1)
			MakeVectorPlanar(&dir, &dir, &hitplanes[0]);
		// we have to set velocity along crease
		else
		{
			vector_t tmp;
			_CrossProduct(&hitplanes[0], &hitplanes[1], &tmp);
			d = DotProduct(tmp,dir);
			dir.Scale(tmp,d);
			//Void3d::VectorScale(dir,tmp, d);
		}

		// make sure we're still going forward
		if (DotProduct(dir,primal_dir) <= 0)
		{
			dir.Set(0, 0, 0);
			break;
		}
	}
}