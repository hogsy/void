#ifndef VOID_SOUND_LISTENER
#define VOID_SOUND_LISTENER

#include "Snd_hdr.h"
#include "3dmath.h"

namespace VoidSound {

/*
======================================
3d listener
======================================
*/
class C3DListener
{
public:

	C3DListener(IDirectSound3DListener * listener) : m_pListener(listener)	{}
	~C3DListener()	{ m_pListener->Release(); m_pListener =0;	}

	
	bool SetRollOffFactor(float factor)
	{
		hr = m_pListener->SetRolloffFactor(factor, DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"C3DListener::SetRollOffFactor");
			return false;
		}
		return true;
	}
	
	bool SetDopplerFactor(float factor)
	{
		hr = m_pListener->SetDopplerFactor(factor, DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"C3DListener::SetDopplerFactor");
			return false;
		}
		return true;
	}
	
	bool SetDistanceFactor(float factor)
	{
		hr = m_pListener->SetDistanceFactor(factor, DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"C3DListener::SetDistanceFactor");
			return false;
		}
		return true;
	}

	bool CommitSettings()
	{
		hr = m_pListener->CommitDeferredSettings();
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"C3DListener::CommitSettings");
			return false;
		}
		return true;
	}


	void UpdateState(const vector_t &position,
					 const vector_t &velocity,
					 const vector_t &forward,
					 const vector_t &up)
	{

		m_pListener->SetPosition(position.x,position.y, position.z, DS3D_DEFERRED);
		m_pListener->SetVelocity(velocity.x,velocity.y, velocity.z, DS3D_DEFERRED);
		m_pListener->SetOrientation(forward.x,forward.y, forward.z,
									up.x,up.y, up.z, DS3D_DEFERRED);

		hr = m_pListener->CommitDeferredSettings();
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"C3DListener::UpdateState");
		}
	}

private:

	HRESULT	hr;
	IDirectSound3DListener * m_pListener;

};

}

#endif