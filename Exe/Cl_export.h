#ifndef VOID_CLIENT_EXPORTS
#define VOID_CLIENT_EXPORTS

#include "Cl_hdr.h"


class CClientExports : public I_ClientGameInterface
{
public:

	CClientExports(CClient & refClient) : m_refClient(refClient) {}
	~CClientExports() {}

	//Models
	inline int  RegisterModel(const char *model, CacheType cache,int index=-1)
	{	return m_refClient.m_pClRen->LoadModel(model,cache,index);
	}
	inline void UnregisterModel(CacheType cache,int index)
	{	m_refClient.m_pClRen->UnloadModel(cache,index);
	}
	inline void UnregisterModelCache(CacheType cache)
	{	m_refClient.m_pClRen->UnloadModelCache(cache);
	}

	//Images
	inline int  RegisterImage(const char *image, CacheType cache,int index=-1)
	{	return m_refClient.m_pClRen->LoadImage(image,cache,index);
	}
	inline void UnregisterImage(CacheType cache,int index)
	{	m_refClient.m_pClRen->UnloadImage(cache,index);
	}
	inline void UnregisterImageCache(CacheType cache)
	{	m_refClient.m_pClRen->UnloadImageCache(cache);
	}

	//Hud
	inline void HudPrint(int x, int y, float time, const char *msg)
	{	m_refClient.m_pHud->Printf(x,y,time,msg);
	}

	//Sound
	inline int  RegisterSound(const char *path, CacheType cache,int index = -1)
	{	return m_refClient.m_pSound->RegisterSound(path,cache,index);
	}
	inline void UnregisterSound(CacheType cache,int index)
	{	m_refClient.m_pSound->UnregisterSound(cache,index);
	}
	inline void UnregisterSoundCache(CacheType cache)
	{	m_refClient.m_pSound->UnregisterCache(cache);
	}

	inline void AddSoundSource(const ClEntity * ent)
	{	m_refClient.m_pSound->AddStaticSource(ent);
	}
	inline void RemoveSoundSource(const ClEntity * ent)
	{	m_refClient.m_pSound->RemoveStaticSource(ent);
	}
	inline void UpdateSoundSource(const ClEntity * ent)
	{	m_refClient.m_pSound->UpdateStaticSource(ent);
	}

	inline void PlaySnd3d(const ClEntity * ent,
						  int index, CacheType cache,
						  int volume = 10, 
						  int attenuation =5,
						  int chantype = CHAN_AUTO)
	{	m_refClient.m_pSound->PlaySnd3d(ent,index,cache,volume,attenuation,chantype);
	}
	
	inline void PlaySnd2d(int index, CacheType cache, int volume = 10,
				   int chantype = CHAN_AUTO)
	{	m_refClient.m_pSound->PlaySnd2d(index, cache, volume, chantype);
	}

	//Client
	inline void SetClientState(EClState state)
	{	m_refClient.SetClientState(static_cast<int>(state));
	}
	
	inline void SetNetworkRate(int rate)
	{	m_refClient.m_pNetCl->SetRate(rate);
	}
	
	inline bool LoadWorld(const char *worldname)
	{	return m_refClient.LoadWorld(worldname);
	}
	
	inline void UnloadWorld()
	{	m_refClient.UnloadWorld();
	}

	//Networking
	inline const NetChanState & GetNetChanState() const
	{	return m_refClient.m_pNetCl->GetChanState();
	}
	
	inline CBuffer & GetSendBuffer()
	{	return m_refClient.m_pNetCl->GetSendBuffer();
	}
	
	inline CBuffer & GetReliableSendBuffer()
	{	return m_refClient.m_pNetCl->GetReliableBuffer();
	}

private:
	CClient & m_refClient;
};


#endif