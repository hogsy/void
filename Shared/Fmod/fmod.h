//==========================================================================================
// FMOD Main header file. Copyright (c), FireLight Multimedia 1999.
//==========================================================================================

#ifndef _FMOD_H_
#define _FMOD_H_

//===============================================================================================
//= DEFINITIONS
//===============================================================================================

#define F_API _stdcall

#ifdef DLL_EXPORTS
	#define DLL_API __declspec(dllexport)
#else
	#ifdef LCCWIN32
		#define DLL_API F_API
	#else
		#define DLL_API __declspec(dllimport)
	#endif // LCCWIN32
#endif //DLL_EXPORTS


// fmod defined types
typedef struct FSOUND_SAMPLE	FSOUND_SAMPLE;
typedef struct FSOUND_STREAM	FSOUND_STREAM;
typedef struct FSOUND_DSPUNIT	FSOUND_DSPUNIT;
typedef struct FMUSIC_MODULE	FMUSIC_MODULE;
typedef struct FSOUND_MATERIAL	FSOUND_MATERIAL;
typedef struct FSOUND_GEOMLIST	FSOUND_GEOMLIST;

// callback types
typedef void (*FSOUND_STREAMCALLBACK)(FSOUND_STREAM *stream, void *buff, long len);
typedef void *(*FSOUND_DSPCALLBACK)(void *originalbuffer, void *newbuffer, long length, long param);
typedef void (*FMUSIC_ZXXCALLBACK)(FMUSIC_MODULE *mod, unsigned char param);

/*
[ENUM]
[
	[DESCRIPTION]	
	On failure of commands in FMOD, use FSOUND_GetError to attain what happened.
	
	[SEE_ALSO]		
	FSOUND_GetError
]
*/
enum FMOD_ERRORS 
{
	FMOD_ERR_NONE,			   // No errors
	FMOD_ERR_BUSY,             // Cannot call this command after FSOUND_Init.  Call FSOUND_Close first.
	FMOD_ERR_UNINITIALIZED,	   // This command failed because FSOUND_Init was not called
	FMOD_ERR_INIT,			   // Error initializing output device.
	FMOD_ERR_ALLOCATED,		   // Error initializing output device, but more specifically, the output device is already in use and cannot be reused.
	FMOD_ERR_PLAY,			   // Playing the sound failed.
	FMOD_ERR_OUTPUT_FORMAT,	   // Soundcard does not support the features needed for this soundsystem (16bit stereo output)
	FMOD_ERR_COOPERATIVELEVEL, // Error setting cooperative level for hardware.
	FMOD_ERR_CREATEBUFFER,	   // Error creating hardware sound buffer.
	FMOD_ERR_FILE_NOTFOUND,	   // File not found
	FMOD_ERR_FILE_FORMAT,	   // Unknown file format
	FMOD_ERR_FILE_BAD,		   // Error loading file
	FMOD_ERR_MEMORY,           // Not enough memory 
	FMOD_ERR_VERSION,		   // The version number of this file format is not supported
	FMOD_ERR_INVALID_MIXER,    // Incorrect mixer selected
	FMOD_ERR_INVALID_PARAM,	   // An invalid parameter was passed to this function
	FMOD_ERR_NO_A3D,		   // Tried to use a3d and not an a3d hardware card, or dll didnt exist, try another output type.
	FMOD_ERR_NO_EAX,		   // Tried to use an EAX command on a non EAX enabled channel or output.
	FMOD_ERR_CHANNEL_ALLOC,	   // Failed to allocate a new channel
};

/*
[ENUM]
[
	[DESCRIPTION]	
	These output types are used with FSOUND_SetOutput, to choose which output driver to use.
	
	FSOUND_OUTPUT_A3D will cause FSOUND_Init to FAIL if you have not got a vortex 
	based A3D card.  The suggestion for this is to immediately try and reinitialize FMOD with
	FSOUND_OUTPUT_DSOUND, and if this fails, try initializing FMOD with	FSOUND_OUTPUT_WAVEOUT.
	
	FSOUND_OUTPUT_DSOUND will not support hardware 3d acceleration if the sound card driver 
	does not support DirectX 6 Voice Manager Extensions.

	[SEE_ALSO]		
	FSOUND_SetOutput
	FSOUND_GetOutput
]
*/
enum FSOUND_OUTPUTTYPES
{
	FSOUND_OUTPUT_NOSOUND,    // NoSound driver, all calls to this succeed but do nothing.
	FSOUND_OUTPUT_WINMM,      // Windows Multimedia driver.
	FSOUND_OUTPUT_DSOUND,     // DirectSound driver.  You need this to get EAX support.
	FSOUND_OUTPUT_A3D,        // A3D driver.
};


/*
[ENUM]
[
	[DESCRIPTION]	
	These mixer types are used with FSOUND_SetMixer, to choose which mixer to use, or to act 
	upon for other reasons using FSOUND_GetMixer.

	[SEE_ALSO]		
	FSOUND_SetMixer
	FSOUND_GetMixer
]
*/
enum FSOUND_MIXERTYPES
{
	FSOUND_MIXER_AUTODETECT,	// This enables autodetection of the fastest mixer based on your cpu.
	FSOUND_MIXER_BLENDMODE,		// This enables the standard non mmx, blendmode mixer.
	FSOUND_MIXER_MMXP5,			// This enables the mmx, pentium optimized blendmode mixer.
	FSOUND_MIXER_MMXP6,			// This enables the mmx, ppro/p2/p3 optimized mixer.

	FSOUND_MIXER_QUALITY_AUTODETECT,// This enables autodetection of the fastest quality mixer based on your cpu.
	FSOUND_MIXER_QUALITY_FPU,	// This enables the interpolating FPU mixer. 
	FSOUND_MIXER_QUALITY_MMXP5,	// This enables the interpolating p5 MMX mixer. 
	FSOUND_MIXER_QUALITY_MMXP6,	// This enables the interpolating ppro/p2/p3 MMX mixer. 
};

/*
[ENUM]
[
	[DESCRIPTION]	
	These definitions describe the type of song being played.

	[SEE_ALSO]		
	FMUSIC_GetType	
]
*/
enum FMUSIC_TYPES
{
	FMUSIC_TYPE_NONE,		
	FMUSIC_TYPE_MOD,		// Protracker / Fasttracker
	FMUSIC_TYPE_S3M,		// ScreamTracker 3
	FMUSIC_TYPE_XM,			// FastTracker 2
	FMUSIC_TYPE_IT,			// Impulse Tracker.
};


/*
[DEFINE_START] 
[
 	[NAME] 
	FSOUND_DSP_PRIORITIES

	[DESCRIPTION]	
	These default priorities are 

	[SEE_ALSO]		
	FSOUND_DSP_Create
	FSOUND_DSP_SetPriority
]
*/
#define FSOUND_DSP_DEFAULTPRIORITY_CLEARUNIT		0	 // DSP CLEAR unit - done first
#define FSOUND_DSP_DEFAULTPRIORITY_SFXUNIT			100	 // DSP SFX unit - done second
#define FSOUND_DSP_DEFAULTPRIORITY_MUSICUNIT		200	 // DSP MUSIC unit - done third
#define FSOUND_DSP_DEFAULTPRIORITY_USER				300	 // User priority, use this as reference
#define FSOUND_DSP_DEFAULTPRIORITY_CLIPANDCOPYUNIT	1000 // DSP CLIP AND COPY unit - last
// [DEFINE_END]


/*
[DEFINE_START] 
[
 	[NAME] 
	FSOUND_CAPS

	[DESCRIPTION]	
	Driver description bitfields.  Use FSOUND_Driver_GetCaps to determine if a driver enumerated
	has the settings you are after.  The enumerated driver depends on the output mode, see
	FSOUND_OUTPUTTYPES

	[SEE_ALSO]
	FSOUND_GetDriverCaps
	FSOUND_OUTPUTTYPES
]
*/
#define FSOUND_CAPS_HARDWARE				0x1		// This driver supports hardware accelerated 3d sound.
#define FSOUND_CAPS_EAX						0x2		// This driver supports EAX reverb
#define FSOUND_CAPS_GEOMETRY_OCCLUSIONS		0x4		// This driver supports (A3D) geometry occlusions
#define FSOUND_CAPS_GEOMETRY_REFLECTIONS	0x8		// This driver supports (A3D) geometry reflections
// [DEFINE_END]


/*
[DEFINE_START] 
[
 	[NAME] 
	FSOUND_MODES
	
	[DESCRIPTION]	
	Sample description bitfields, OR them together for loading and describing samples.
]
*/
#define FSOUND_NORMAL		0x0			// Default sample type.  Loop off, 8bit mono, signed, not hardware accelerated.
#define FSOUND_LOOP_OFF		0x01		// For non looping samples.
#define FSOUND_LOOP_NORMAL	0x02		// For forward looping samples.
#define FSOUND_LOOP_BIDI	0x04		// For bidirectional looping samples.  (no effect if in hardware).
#define FSOUND_8BITS		0x08		// For 8 bit samples.
#define FSOUND_16BITS		0x10		// For 16 bit samples.
#define FSOUND_MONO			0x20		// For mono samples.
#define FSOUND_STEREO		0x40		// For stereo samples.
#define FSOUND_UNSIGNED		0x80		// For source data containing unsigned samples.
#define FSOUND_SIGNED		0x100		// For source data containing signed data.
#define FSOUND_DELTA		0x200		// For source data stored as delta values.
#define FSOUND_IT214		0x400		// For source data stored using IT214 compression.
#define FSOUND_IT215		0x800		// For source data stored using IT215 compression.
#define FSOUND_HW3D			0x1000		// Attempts to make samples use 3d hardware acceleration. (if the card supports it)
#define FSOUND_2D			0x2000		// Ignores any 3d processing.  overrides FSOUND_HW3D.  Located in software.
#define FSOUND_STREAMABLE	0x4000		// For realtime streamable samples.  If you dont supply this sound may come out corrupted.
// [DEFINE_END]


/*
[DEFINE_START] 
[
 	[NAME] 
	FSOUND_CDPLAYMODES
	
	[DESCRIPTION]	
	Playback method for a CD Audio track, using FSOUND_CD_Play

	[SEE_ALSO]		
	FSOUND_CD_Play
]
*/
#define FSOUND_CD_PLAYCONTINUOUS	0	// Starts from the current track and plays to end of CD.
#define FSOUND_CD_PLAYONCE			1	// Plays the specified track then stops.
#define FSOUND_CD_PLAYLOOPED		2	// Plays the specified track looped, forever until stopped manually.
#define FSOUND_CD_PLAYRANDOM		3	// Plays tracks in random order
// [DEFINE_END]


/*
[DEFINE_START] 
[
 	[NAME] 
	FSOUND_SAMPLEMODE
	
	[DESCRIPTION]	

	[SEE_ALSO]		
	FSOUND_CD_Play
]
*/
#define FSOUND_FREE					-1	// definition for dynamically allocated channel or sample
#define FSOUND_UNMANAGED			-2	// definition for allocating a sample that is NOT managed by fsound
#define FSOUND_STEREOPAN			-1	// definition for full middle stereo volume on both channels
// [DEFINE_END]


/*
[ENUM]
[
	[DESCRIPTION]	
	These are environment types defined for use with EAX

	[SEE_ALSO]		
	FSOUND_EAX_SetEnvironment
]
*/
enum FSOUND_EAX_ENVIRONMENTS
{
    EAX_ENVIRONMENT_GENERIC,           // factory default
    EAX_ENVIRONMENT_PADDEDCELL,
    EAX_ENVIRONMENT_ROOM,              // standard environments
    EAX_ENVIRONMENT_BATHROOM,
    EAX_ENVIRONMENT_LIVINGROOM,
    EAX_ENVIRONMENT_STONEROOM,
    EAX_ENVIRONMENT_AUDITORIUM,
    EAX_ENVIRONMENT_CONCERTHALL,
    EAX_ENVIRONMENT_CAVE,
    EAX_ENVIRONMENT_ARENA,
    EAX_ENVIRONMENT_HANGAR,
    EAX_ENVIRONMENT_CARPETEDHALLWAY,
    EAX_ENVIRONMENT_HALLWAY,
    EAX_ENVIRONMENT_STONECORRIDOR,
    EAX_ENVIRONMENT_ALLEY,
    EAX_ENVIRONMENT_FOREST,
    EAX_ENVIRONMENT_CITY,
    EAX_ENVIRONMENT_MOUNTAINS,
    EAX_ENVIRONMENT_QUARRY,
    EAX_ENVIRONMENT_PLAIN,
    EAX_ENVIRONMENT_PARKINGLOT,
    EAX_ENVIRONMENT_SEWERPIPE,
    EAX_ENVIRONMENT_UNDERWATER,
    EAX_ENVIRONMENT_DRUGGED,
    EAX_ENVIRONMENT_DIZZY,
    EAX_ENVIRONMENT_PSYCHOTIC,

    EAX_ENVIRONMENT_COUNT           // total number of environments
};

#define EAX_MAX_ENVIRONMENT (EAX_ENVIRONMENT_COUNT - 1)
#define EAX_REVERBMIX_USEDISTANCE	-1.0f


/*
[DEFINE_START] 
[
 	[NAME] 
	FSOUND_EAX_PRESETS
	
	[DESCRIPTION]	
	A set of predefined environments with their parameters, created by Creative Labs

	[SEE_ALSO]
	FSOUND_EAX_SetEnvironment
]
*/
#define EAX_PRESET_GENERIC         EAX_ENVIRONMENT_GENERIC,0.5F,1.493F,0.5F
#define EAX_PRESET_PADDEDCELL      EAX_ENVIRONMENT_PADDEDCELL,0.25F,0.1F,0.0F
#define EAX_PRESET_ROOM            EAX_ENVIRONMENT_ROOM,0.417F,0.4F,0.666F
#define EAX_PRESET_BATHROOM        EAX_ENVIRONMENT_BATHROOM,0.653F,1.499F,0.166F
#define EAX_PRESET_LIVINGROOM      EAX_ENVIRONMENT_LIVINGROOM,0.208F,0.478F,0.0F
#define EAX_PRESET_STONEROOM       EAX_ENVIRONMENT_STONEROOM,0.5F,2.309F,0.888F
#define EAX_PRESET_AUDITORIUM      EAX_ENVIRONMENT_AUDITORIUM,0.403F,4.279F,0.5F
#define EAX_PRESET_CONCERTHALL     EAX_ENVIRONMENT_CONCERTHALL,0.5F,3.961F,0.5F
#define EAX_PRESET_CAVE            EAX_ENVIRONMENT_CAVE,0.5F,2.886F,1.304F
#define EAX_PRESET_ARENA           EAX_ENVIRONMENT_ARENA,0.361F,7.284F,0.332F
#define EAX_PRESET_HANGAR          EAX_ENVIRONMENT_HANGAR,0.5F,10.0F,0.3F
#define EAX_PRESET_CARPETEDHALLWAY EAX_ENVIRONMENT_CARPETEDHALLWAY,0.153F,0.259F,2.0F
#define EAX_PRESET_HALLWAY         EAX_ENVIRONMENT_HALLWAY,0.361F,1.493F,0.0F
#define EAX_PRESET_STONECORRIDOR   EAX_ENVIRONMENT_STONECORRIDOR,0.444F,2.697F,0.638F
#define EAX_PRESET_ALLEY           EAX_ENVIRONMENT_ALLEY,0.25F,1.752F,0.776F
#define EAX_PRESET_FOREST          EAX_ENVIRONMENT_FOREST,0.111F,3.145F,0.472F
#define EAX_PRESET_CITY            EAX_ENVIRONMENT_CITY,0.111F,2.767F,0.224F
#define EAX_PRESET_MOUNTAINS       EAX_ENVIRONMENT_MOUNTAINS,0.194F,7.841F,0.472F
#define EAX_PRESET_QUARRY          EAX_ENVIRONMENT_QUARRY,1.0F,1.499F,0.5F
#define EAX_PRESET_PLAIN           EAX_ENVIRONMENT_PLAIN,0.097F,2.767F,0.224F
#define EAX_PRESET_PARKINGLOT      EAX_ENVIRONMENT_PARKINGLOT,0.208F,1.652F,1.5F
#define EAX_PRESET_SEWERPIPE       EAX_ENVIRONMENT_SEWERPIPE,0.652F,2.886F,0.25F
#define EAX_PRESET_UNDERWATER      EAX_ENVIRONMENT_UNDERWATER,1.0F,1.499F,0.0F
#define EAX_PRESET_DRUGGED         EAX_ENVIRONMENT_DRUGGED,0.875F,8.392F,1.388F
#define EAX_PRESET_DIZZY           EAX_ENVIRONMENT_DIZZY,0.139F,17.234F,0.666F
#define EAX_PRESET_PSYCHOTIC       EAX_ENVIRONMENT_PSYCHOTIC,0.486F,7.563F,0.806F
// [DEFINE_END]


/*
[DEFINE_START] 
[
 	[NAME] 
	FSOUND_GEOMETRY_MODES
	
	[DESCRIPTION]	
	geometry flags,
]
*/
#define FSOUND_GEOMETRY_NORMAL				0x0		// Default geometry type.  Occluding polygon
#define FSOUND_GEOMETRY_REFLECTIVE			0x01	// This polygon is reflective
#define FSOUND_GEOMETRY_OPENING				0x02	// Overlays a transparency over the previous polygon.  The 'openingfactor' value supplied is copied internally.
#define FSOUND_GEOMETRY_OPENING_REFERENCE	0x04	// Overlays a transparency over the previous polygon.  The 'openingfactor' supplied is pointed to (for access when building a list)
// [DEFINE_END]





//===============================================================================================
//= FUNCTION PROTOTYPES
//===============================================================================================

#ifdef __cplusplus
extern "C" {
#endif

// ==================================
// Initialization / Global functions.
// ==================================

// Pre FSOUND_Init functions. These can't be called after FSOUND_Init is called (they will fail)
DLL_API signed char		F_API FSOUND_SetOutput(long outputtype);
DLL_API signed char		F_API FSOUND_SetDriver(long driver);
DLL_API signed char		F_API FSOUND_SetMixer(long mixer);
DLL_API signed char		F_API FSOUND_SetBufferSize(long len_ms);
DLL_API signed char		F_API FSOUND_SetHWND(void *hwnd);

// Main initialization / closedown functions
DLL_API signed char		F_API FSOUND_Init(long mixrate, long maxchannels, long vcmmode);
DLL_API void			F_API FSOUND_Close();

// Runtime 
DLL_API void			F_API FSOUND_SetSFXMasterVolume(long volume);
DLL_API void			F_API FSOUND_SetPanSeperation(float pansep);

// Error functions
DLL_API long			F_API FSOUND_GetError();

// ===================================
// Sample management / load functions.
// ===================================

// File functions
DLL_API FSOUND_SAMPLE *	F_API FSOUND_Sample_LoadWav(long index, char *filename, unsigned long mode);
DLL_API FSOUND_SAMPLE *	F_API FSOUND_Sample_LoadMpeg(long index, char *filename, unsigned long mode);
DLL_API FSOUND_SAMPLE *	F_API FSOUND_Sample_LoadRaw(long index, char *filename, unsigned long mode);
DLL_API FSOUND_SAMPLE * F_API FSOUND_Sample_LoadWavMemory(long index, void *data, unsigned long mode, long length);
DLL_API FSOUND_SAMPLE * F_API FSOUND_Sample_LoadMpegMemory(long index, void *data, unsigned long mode, long length);

// Sample management functions
DLL_API FSOUND_SAMPLE *	F_API FSOUND_Sample_Alloc(long index, long length, unsigned long mode, long deffreq, long defvol, long defpan, long defpri);
DLL_API void			F_API FSOUND_Sample_Free(FSOUND_SAMPLE *sptr);
DLL_API signed char		F_API FSOUND_Sample_Upload(FSOUND_SAMPLE *sptr, void *srcdata, unsigned long mode);
DLL_API signed char		F_API FSOUND_Sample_Lock(FSOUND_SAMPLE *sptr, long offset, long length, void **ptr1, void **ptr2, unsigned long *len1, unsigned long *len2);
DLL_API signed char		F_API FSOUND_Sample_Unlock(FSOUND_SAMPLE *sptr, void *ptr1, void *ptr2, unsigned long len1, unsigned long len2);

// Sample control functions
DLL_API signed char		F_API FSOUND_Sample_SetLoopMode(FSOUND_SAMPLE *sptr, unsigned long loopmode);
DLL_API signed char		F_API FSOUND_Sample_SetLoopPoints(FSOUND_SAMPLE *sptr, long loopstart, long loopend);
DLL_API signed char		F_API FSOUND_Sample_SetDefaults(FSOUND_SAMPLE *sptr, long deffreq, long defvol, long defpan, long defpri);
DLL_API signed char		F_API FSOUND_Sample_SetMaxAudible(FSOUND_SAMPLE *sptr, long max);
DLL_API void			F_API FSOUND_Sample_UseMulaw(signed char mulaw);
DLL_API signed char		F_API FSOUND_Sample_SetMinMaxDistance(FSOUND_SAMPLE *sptr, float min, float max);
  
// ============================
// Channel control functions.
// ============================

// Playing and stopping sounds.
DLL_API long			F_API FSOUND_PlaySound(long channel, FSOUND_SAMPLE *sptr);
DLL_API long			F_API FSOUND_PlaySoundAttrib(long channel, FSOUND_SAMPLE *sptr, long freq, long vol, long pan);
DLL_API long			F_API FSOUND_3D_PlaySound(long channel, FSOUND_SAMPLE *sptr, float *pos, float *vel);
DLL_API signed char		F_API FSOUND_StopSound(long channel);
DLL_API void			F_API FSOUND_StopAllChannels();
 
// Functions to control playback of a channel.
DLL_API signed char		F_API FSOUND_SetFrequency(long channel, long freq);
DLL_API signed char		F_API FSOUND_SetVolume(long channel, long vol);
DLL_API signed char 	F_API FSOUND_SetVolumeAbsolute(long channel, long vol);
DLL_API signed char		F_API FSOUND_SetPan(long channel, long pan);
DLL_API signed char		F_API FSOUND_SetSurround(long channel, signed char surround);
DLL_API signed char		F_API FSOUND_SetMute(long channel, signed char mute);
DLL_API signed char		F_API FSOUND_SetPriority(long channel, long priority);
DLL_API signed char		F_API FSOUND_SetReserved(long channel, signed char reserved);
DLL_API signed char		F_API FSOUND_SetPaused(long channel, signed char paused);
DLL_API signed char		F_API FSOUND_MixBuffers(void *destbuffer, void *srcbuffer, long len, long freq, long vol, long pan, unsigned long mode);

// ================================
// Information retrieval functions.
// ================================

// System information
DLL_API long			F_API FSOUND_GetOutput();
DLL_API long			F_API FSOUND_GetDriver();
DLL_API long			F_API FSOUND_GetMixer();
DLL_API long			F_API FSOUND_GetNumDrivers();
DLL_API signed char *	F_API FSOUND_GetDriverName(long id);
DLL_API signed char 	F_API FSOUND_GetDriverCaps(long id, unsigned long *caps);

DLL_API long			F_API FSOUND_GetOutputRate();
DLL_API long			F_API FSOUND_GetMaxChannels();
DLL_API long			F_API FSOUND_GetMaxSamples();
DLL_API long			F_API FSOUND_GetSFXMasterVolume();
DLL_API long			F_API FSOUND_GetNumHardwareChannels();
DLL_API long			F_API FSOUND_GetChannelsPlaying();
DLL_API float			F_API FSOUND_GetCPUUsage();

// Channel information
DLL_API signed char		F_API FSOUND_IsPlaying(long channel);
DLL_API long			F_API FSOUND_GetFrequency(long channel);
DLL_API long			F_API FSOUND_GetVolume(long channel);
DLL_API long			F_API FSOUND_GetPan(long channel);
DLL_API signed char		F_API FSOUND_GetSurround(long channel);
DLL_API signed char		F_API FSOUND_GetMute(long channel);
DLL_API long			F_API FSOUND_GetPriority(long channel);
DLL_API signed char		F_API FSOUND_GetReserved(long channel);
DLL_API signed char		F_API FSOUND_GetPaused(long channel);
DLL_API unsigned long	F_API FSOUND_GetCurrentPosition(long channel);
DLL_API FSOUND_SAMPLE *	F_API FSOUND_GetCurrentSample(long channel);
DLL_API float			F_API FSOUND_GetCurrentVU(long channel);

// Sample information 
DLL_API FSOUND_SAMPLE * F_API FSOUND_Sample_Get(long sampno);
DLL_API unsigned long	F_API FSOUND_Sample_GetLength(FSOUND_SAMPLE *sptr);
DLL_API signed char		F_API FSOUND_Sample_GetLoopPoints(FSOUND_SAMPLE *sptr, long *loopstart, long *loopend);
DLL_API signed char		F_API FSOUND_Sample_GetDefaults(FSOUND_SAMPLE *sptr, long *deffreq, long *defvol, long *defpan, long *defpri);
DLL_API unsigned long	F_API FSOUND_Sample_GetMode(FSOUND_SAMPLE *sptr);
DLL_API signed char		F_API FSOUND_Sample_IsUsingMulaw();
 
// ===================
// 3D sound functions.
// ===================
// see also FSOUND_3D_PlaySound (above)
// see also FSOUND_Sample_SetMinMaxDistance (above)
DLL_API void			F_API FSOUND_3D_Update();
DLL_API signed char		F_API FSOUND_3D_SetAttributes(long channel, float *pos, float *vel);
DLL_API signed char		F_API FSOUND_3D_GetAttributes(long channel, float *pos, float *vel);
DLL_API void			F_API FSOUND_3D_Listener_SetAttributes(float *pos, float *vel, float fx, float fy, float fz, float tx, float ty, float tz);
DLL_API void			F_API FSOUND_3D_Listener_GetAttributes(float *pos, float *vel, float *fx, float *fy, float *fz, float *tx, float *ty, float *tz);
DLL_API void			F_API FSOUND_3D_Listener_SetDopplerFactor(float scale);
DLL_API void			F_API FSOUND_3D_Listener_SetDistanceFactor(float scale);
DLL_API void			F_API FSOUND_3D_Listener_SetRolloffFactor(float scale);

// ===================
// Geometry functions.
// ===================

// scene/polygon functions
DLL_API signed char		F_API FSOUND_Geometry_AddPolygon(float *p1, float *p2, float *p3, float *p4, float *normal, unsigned long mode, float *openingfactor);
DLL_API long			F_API FSOUND_Geometry_AddList(FSOUND_GEOMLIST *geomlist);

// polygon list functions
DLL_API FSOUND_GEOMLIST * F_API FSOUND_Geometry_List_Create(signed char boundingvolume);
DLL_API signed char		F_API FSOUND_Geometry_List_Free(FSOUND_GEOMLIST *geomlist);
DLL_API signed char		F_API FSOUND_Geometry_List_Begin(FSOUND_GEOMLIST *geomlist);
DLL_API signed char		F_API FSOUND_Geometry_List_End(FSOUND_GEOMLIST *geomlist);
DLL_API signed char		F_API FSOUND_Geometry_List_Add(FSOUND_GEOMLIST *geomlist);

// material functions
DLL_API FSOUND_MATERIAL * F_API FSOUND_Geometry_Material_Create();
DLL_API signed char		F_API FSOUND_Geometry_Material_Free(FSOUND_MATERIAL *material);
DLL_API signed char		F_API FSOUND_Geometry_Material_SetAttributes(FSOUND_MATERIAL *material, float reflectancegain, float reflectancefreq, float transmittancegain, float transmittancefreq);
DLL_API signed char		F_API FSOUND_Geometry_Material_GetAttributes(FSOUND_MATERIAL *material, float *reflectancegain, float *reflectancefreq, float *transmittancegain, float *transmittancefreq);
DLL_API signed char		F_API FSOUND_Geometry_Material_Set(FSOUND_MATERIAL *material);

// =============
// EAX functions.
// ==============
DLL_API signed char		F_API FSOUND_EAX_SetEnvironment(long env, float vol, float decay, float damp);
DLL_API signed char		F_API FSOUND_EAX_GetEnvironment(long *env, float *vol, float *decay, float *damp);
DLL_API signed char		F_API FSOUND_EAX_SetMix(long channel, float mix);
DLL_API signed char		F_API FSOUND_EAX_GetMix(long channel, float *mix);
 
// =========================
// File Streaming functions.
// =========================


DLL_API FSOUND_STREAM *	F_API FSOUND_Stream_Create(FSOUND_STREAMCALLBACK callback, long length, unsigned long mode, long samplerate);
DLL_API FSOUND_STREAM *	F_API FSOUND_Stream_Open(char *filename, unsigned long mode, long samplerate);
DLL_API FSOUND_STREAM *	F_API FSOUND_Stream_OpenWav(char *filename, unsigned long mode);
DLL_API FSOUND_STREAM *	F_API FSOUND_Stream_OpenMpeg(char *filename, unsigned long mode);
DLL_API long 			F_API FSOUND_Stream_Play(long channel, FSOUND_STREAM *stream);
DLL_API signed char		F_API FSOUND_Stream_Stop(FSOUND_STREAM *stream);
DLL_API signed char		F_API FSOUND_Stream_Close(FSOUND_STREAM *stream);

DLL_API signed char		F_API FSOUND_Stream_SetPaused(FSOUND_STREAM *stream, signed char paused);
DLL_API signed char		F_API FSOUND_Stream_GetPaused(FSOUND_STREAM *stream);
DLL_API signed char		F_API FSOUND_Stream_SetPosition(FSOUND_STREAM *stream, long position);
DLL_API long			F_API FSOUND_Stream_GetPosition(FSOUND_STREAM *stream);
DLL_API long			F_API FSOUND_Stream_GetTime(FSOUND_STREAM *stream);
DLL_API long			F_API FSOUND_Stream_GetLength(FSOUND_STREAM *stream);

// ===================
// CD audio functions.
// ===================
DLL_API signed char		F_API FSOUND_CD_Play(long track);
DLL_API void			F_API FSOUND_CD_SetPlayMode(signed char mode);
DLL_API signed char		F_API FSOUND_CD_Stop();
DLL_API signed char		F_API FSOUND_CD_SetPaused(signed char paused);
DLL_API signed char		F_API FSOUND_CD_GetPaused();
DLL_API long			F_API FSOUND_CD_GetTrack();
DLL_API long			F_API FSOUND_CD_GetNumTracks();
DLL_API signed char		F_API FSOUND_CD_Eject();


// ==============
// DSP functions.
// ==============

// DSP Unit control and information functions.
DLL_API FSOUND_DSPUNIT *F_API FSOUND_DSP_Create(FSOUND_DSPCALLBACK callback, long priority, long param);
DLL_API void			F_API FSOUND_DSP_Free(FSOUND_DSPUNIT *unit);

DLL_API void			F_API FSOUND_DSP_SetPriority(FSOUND_DSPUNIT *unit, long priority);
DLL_API long			F_API FSOUND_DSP_GetPriority(FSOUND_DSPUNIT *unit);

DLL_API void			F_API FSOUND_DSP_SetActive(FSOUND_DSPUNIT *unit, signed char active);
DLL_API signed char		F_API FSOUND_DSP_GetActive(FSOUND_DSPUNIT *unit);

// Functions to get hold of FSOUND 'system DSP unit' handles.
DLL_API FSOUND_DSPUNIT *F_API FSOUND_DSP_GetClearUnit();
DLL_API FSOUND_DSPUNIT *F_API FSOUND_DSP_GetSFXUnit();
DLL_API FSOUND_DSPUNIT *F_API FSOUND_DSP_GetMusicUnit();
DLL_API FSOUND_DSPUNIT *F_API FSOUND_DSP_GetClipAndCopyUnit();

DLL_API void			F_API FSOUND_DSP_ClearMixBuffer();
DLL_API long			F_API FSOUND_DSP_GetBufferLength();

// =========================
// File system override API.
// =========================

DLL_API void F_API FSOUND_File_SetCallbacks(unsigned long (*OpenCallback)(char *name),
                                            void          (*CloseCallback)(unsigned long handle),
                                            long          (*ReadCallback)(void *buffer, long size, unsigned long handle),
                                            void		  (*SeekCallback)(unsigned long handle, long pos, signed char mode),
                                            long          (*TellCallback)(unsigned long handle));

// =============================================================================================
// FMUSIC API
// =============================================================================================

// Song management / playback functions.
// =====================================
DLL_API FMUSIC_MODULE * F_API FMUSIC_LoadSong(char *name);
DLL_API FMUSIC_MODULE * F_API FMUSIC_LoadSongMemory(void *data, long length);
DLL_API signed char		F_API FMUSIC_FreeSong(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_PlaySong(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_StopSong(FMUSIC_MODULE *mod);
DLL_API void			F_API FMUSIC_StopAllSongs();
DLL_API void			F_API FMUSIC_SetZxxCallback(FMUSIC_MODULE *mod, FMUSIC_ZXXCALLBACK callback);
DLL_API signed char		F_API FMUSIC_OptimizeChannels(FMUSIC_MODULE *mod, long maxchannels, long minvolume);


// Runtime song functions.
// =======================
DLL_API signed char		F_API FMUSIC_SetOrder(FMUSIC_MODULE *mod, long order);
DLL_API signed char		F_API FMUSIC_SetPaused(FMUSIC_MODULE *mod, signed char pause);
DLL_API signed char		F_API FMUSIC_SetMasterVolume(FMUSIC_MODULE *mod, long volume);
DLL_API signed char		F_API FMUSIC_SetPanSeperation(FMUSIC_MODULE *mod, float pansep);
 
// Static song information functions.
// ==================================
DLL_API char *			F_API FMUSIC_GetName(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_GetType(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_IsFinished(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_IsPlaying(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_UsesLinearFrequencies(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_UsesMulaw(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetNumOrders(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetNumPatterns(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetNumInstruments(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetNumSamples(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetNumChannels(FMUSIC_MODULE *mod);
DLL_API FSOUND_SAMPLE * F_API FMUSIC_GetSample(FMUSIC_MODULE *mod, long sampno);
 
// Runtime song information.
// =========================
DLL_API long			F_API FMUSIC_GetMasterVolume(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetGlobalVolume(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetOrder(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetPattern(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetSpeed(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetBPM(FMUSIC_MODULE *mod);
DLL_API long			F_API FMUSIC_GetRow(FMUSIC_MODULE *mod);
DLL_API signed char		F_API FMUSIC_GetPaused(FMUSIC_MODULE *mod);
DLL_API unsigned long	F_API FMUSIC_GetTime(FMUSIC_MODULE *mod);
  
#ifdef __cplusplus
}
#endif

#endif
