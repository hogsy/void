#include "Sys_hdr.h"
#include "Cl_main.h"
#include "Cl_export.h"



struct I_ClientGameInterface
{
	//Models
	virtual int  RegisterModel(const char *model, int index=-1)=0;
	virtual void UnregisterModel(int index)=0;
	virtual void UnregisterAllModels()=0;

	//Images
	virtual int  RegisterImage(const char *image, int index=-1)=0;
	virtual void UnregisterImage(int index)=0;
	virtual void UnregisterAllImages()=0;

	//Hud
	virtual void HudPrint(int x, int y, float time, const char *msg)=0;

	//Sound
	virtual int  RegisterSound(const char *path, int index = -1)=0;
	virtual void UnregisterSound(int index)=0;
	virtual void UnregisterAllSounds()=0;

	virtual void AddSoundSource(const ClEntity * ent)=0;
	virtual void RemoveSoundSource(const ClEntity * ent)=0;
	virtual void UpdateSoundSource(const ClEntity * ent)=0;

	virtual void PlaySnd3d(const ClEntity * ent,
				   int index, int volume = DEFAULT_VOLUME, 
				   int attenuation =DEFAULT_ATTENUATION,
				   int chantype = CHAN_AUTO)=0;
	
	virtual void PlaySnd2d(int index, int volume = DEFAULT_VOLUME,
				   int chantype = CHAN_AUTO)=0;

	//Client
	virtual void SetClientState(int state)=0;
	virtual void SetNetworkRate(int rate)=0;
	
	virtual bool LoadWorld(const char *worldname)=0;
	virtual void UnloadWorld()=0;

	//Networking
	virtual const NetChanState & GetNetChanState() const =0;
	virtual CBuffer & GetSendBuffer()=0;
	virtual CBuffer & GetReliableSendBuffer()=0;

};