#ifndef VOID_GAME_INTERFACE
#define VOID_GAME_INTERFACE



/*
======================================
Functions exported by the Exe
======================================
*/
struct I_GameHandler
{
	virtual void BroadcastPrint(const char * msg)=0;
	virtual void ClientPrint(const char * msg)=0;

	virtual void DebugPrint(const char * msg)=0;
	virtual void FatalError(const char * msg)=0;

	virtual void PlaySnd(const Entity &ent, hSnd index, int channel, float vol, float atten);
	virtual void PlaySnd(vector_t &origin,  hSnd index, int channel, float vol, float atten);

	virtual void ExecCommand(const char * cmd)=0;

	virtual int  RegisterModel(const char * model)=0;
	virtual int  RegisterSound(const char * image)=0;
	virtual int  RegisterImage(const char * sound)=0;
};


//implemented by the game dll
struct I_GameServer
{
	virtual void InitGame()=0;
	virtual void ShutdownGame()=0;
	virtual int  GetVersion()=0;

	virtual void RunFrame()=0;

	virtual bool SpawnEntity(CBuffer &buf)=0;

	//Client funcs
//	virtual bool ClientConnect( 
};



#if 0 
typedef struct
{
	void	(*setmodel) (edict_t *ent, char *name);
	// collision detection
	trace_t	(*trace) (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, edict_t *passent, int contentmask);
	int		(*pointcontents) (vec3_t point);
	qboolean	(*inPVS) (vec3_t p1, vec3_t p2);
	qboolean	(*inPHS) (vec3_t p1, vec3_t p2);
	void		(*SetAreaPortalState) (int portalnum, qboolean open);
	qboolean	(*AreasConnected) (int area1, int area2);

	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  If the size, position, or
	// solidity changes, it must be relinked.
	void	(*linkentity) (edict_t *ent);
	void	(*unlinkentity) (edict_t *ent);		// call before removing an interactive edict
	int		(*BoxEdicts) (vec3_t mins, vec3_t maxs, edict_t **list,	int maxcount, int areatype);
	void	(*Pmove) (pmove_t *pmove);		// player movement code common with client prediction
} game_import_t;

typedef struct
{
	int			apiversion;

	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init
	void		(*Init) (void);
	void		(*Shutdown) (void);

	// each new level entered will cause a call to SpawnEntities
	void		(*SpawnEntities) (char *mapname, char *entstring, char *spawnpoint);

	// Read/Write Game is for storing persistant cross level information
	// about the world state and the clients.
	// WriteGame is called every time a level is exited.
	// ReadGame is called on a loadgame.
	void		(*WriteGame) (char *filename);
	void		(*ReadGame) (char *filename);

	// ReadLevel is called after the default map information has been
	// loaded with SpawnEntities, so any stored client spawn spots will
	// be used when the clients reconnect.
	void		(*WriteLevel) (char *filename);
	void		(*ReadLevel) (char *filename);

	qboolean	(*ClientConnect) (edict_t *ent, char *userinfo, qboolean loadgame);
	void		(*ClientBegin) (edict_t *ent, qboolean loadgame);
	void		(*ClientUserinfoChanged) (edict_t *ent, char *userinfo);
	void		(*ClientDisconnect) (edict_t *ent);
	void		(*ClientCommand) (edict_t *ent);
	void		(*ClientThink) (edict_t *ent, usercmd_t *cmd);

	void		(*RunFrame) (void);

	//
	// global variables shared between game and server
	//

	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	// 
	// The size will be fixed when ge->Init() is called
	struct edict_s	*edicts;
	int			edict_size;
	int			num_edicts;		// current number, <= max_edicts
	int			max_edicts;
} game_export_t;

#endif


#endif