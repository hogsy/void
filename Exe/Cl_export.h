#ifndef VOID_CLIENT_EXPORTS
#define VOID_CLIENT_EXPORTS


class CClientExports : public I_ClientGame
{
public:

	CClientExports(CClient & refClient) : m_refClient(refClient) {}
	~CClientExports() {}

	inline float GetCurTime()
	{	return m_refClient.GetCurTime();
	}

	//Models
	inline void DrawModel(ClEntity &state)
	{	m_refClient.m_pClRen->DrawModel(state);
	}
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
	inline void HudPrintf(int x, int y, float time, const char *msg, ...)
	{	
		va_list args;
		va_start(args, msg);
		vsprintf(m_hudBuffer, msg, args);
		va_end(args);
		m_refClient.m_pHud->Printf(x,y,time,m_hudBuffer);
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

	//Music
	inline void PlayMusicTrack(const char * path)
	{	m_refClient.m_pMusic->PlayMp3(path);
	}

	inline void StopMusicTrack()
	{	m_refClient.m_pMusic->StopMp3();
	}

	//Client
	inline void ForwardNetworkEvent(EClEvent event)
	{	m_refClient.ForwardNetworkEvent(static_cast<int>(event));
	}
	
	inline void SetNetworkRate(int rate)
	{	m_refClient.m_pNetCl->SetRate(rate);
	}
	
	inline CWorld * LoadWorld(const char * worldName)
	{	return m_refClient.LoadWorld(worldName);
	}
	
	inline void UnloadWorld()
	{	m_refClient.UnloadWorld();
	}


	//Networking
	inline CBuffer & GetSendBuffer()
	{	return m_refClient.m_pNetCl->GetSendBuffer();
	}
	
	inline CBuffer & GetReliableSendBuffer()
	{	return m_refClient.m_pNetCl->GetReliableBuffer();
	}

private:
	char	  m_hudBuffer[512];
	CClient & m_refClient;
};


#endif