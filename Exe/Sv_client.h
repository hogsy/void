#if 0


#ifndef _V_GAMECLIENT
#define _V_GAMECLIENT

#include "Net_defs.h"
#include "Net_util.h"
#include "Net_sock.h"
//#include "World.h"
#include "Sv_ents.h"


//This is what the server runs
#define MAX_CLASSNAME 32

class CEntBase
{
public:
	char classname[MAX_CLASSNAME];

	unsigned int sector_index;		// which sector the entitiy is in
//	sector_t *sector;

	vector_t origin;
	vector_t angles;

//	virtual void Think();			//every server frame
};

class CSVClient:public CEntBase
{
public:

	CSVClient();
	~CSVClient();

	bool SV_InitClient(SOCKADDR_IN laddr, int port);				//init the client socket and bind to this info
	
	bool SV_Connect(SOCKADDR_IN raddr, int rport, int numclient);	//connect to this info
	bool SV_Disconnect();											//disconnect cleanup the client

	void WriteToClient(int type,unsigned char *msg);				//append this to the clients outgoing buffer

	void Run();

	//Client states
	bool		m_active;
	bool		m_ingame;
	bool		m_connected;

	
	//Net info vars
	int			m_port;				//this will be the clients port
	float		m_lastmessagetime;	//reliable messages must be sent
	int			m_retries;			//message retries
	float		m_rate;
	

	CNBuffer	 m_recvBuf;			//message out, added to outgoing socket buffer
	CNBuffer	 m_sendBuf;			//this gets updated with data coming in the socket

	CSocket	 	 m_sock;
	
	unsigned int m_sendseq;
	unsigned int m_recvseq;

	
	//Game specific
	//these could be moved to a seperate struct or class
	//which could be shared by the game dlls
	
	int			num;
	char		name[32];
	char		ipaddr[16];

	eyepoint_t  eye;
	vector_t	desired_movement;
};



#endif



















#if 0




#if 0
	usercmd_t		cmd;				// movement
	vec3_t			wishdir;			// intended motion calced from cmd

	sizebuf_t		message;			// can be added to at any time,
										// copied and clear once per frame
	byte			msgbuf[MAX_MSGLEN];
	edict_t			*edict;				// EDICT_NUM(clientnum+1)
	char			name[32];			// for printing to other people
	int				colors;
	float			ping_times[NUM_PING_TIMES];
	int				num_pings;			// ping_times[num_pings%NUM_PING_TIMES]

// spawn parms are carried from level to level
	float			spawn_parms[NUM_SPAWN_PARMS];

// client known data for deltas	
	int				old_frags;
#endif









// server.h

typedef struct
{
	int			maxclients;
	int			maxclientslimit;
	struct client_s	*clients;		// [maxclients]
	int			serverflags;		// episode completion information
	qboolean	changelevel_issued;	// cleared when at SV_SpawnServer
} server_static_t;

//=============================================================================

typedef enum {ss_loading, ss_active} server_state_t;

typedef struct
{
	qboolean	active;				// false if only a net client

	qboolean	paused;
	qboolean	loadgame;			// handle connections specially

	double		time;
	
	int			lastcheck;			// used by PF_checkclient
	double		lastchecktime;
	
	char		name[64];			// map name
	char		modelname[64];		// maps/<name>.bsp, for model_precache[0]
	struct model_s 	*worldmodel;
	char		*model_precache[MAX_MODELS];	// NULL terminated
	struct model_s	*models[MAX_MODELS];
	char		*sound_precache[MAX_SOUNDS];	// NULL terminated
	char		*lightstyles[MAX_LIGHTSTYLES];
	int			num_edicts;
	int			max_edicts;
	edict_t		*edicts;			// can NOT be array indexed, because
									// edict_t is variable sized, but can
									// be used to reference the world ent
	server_state_t	state;			// some actions are only valid during load

	sizebuf_t	datagram;
	byte		datagram_buf[MAX_DATAGRAM];

	sizebuf_t	reliable_datagram;	// copied to all clients at end of frame
	byte		reliable_datagram_buf[MAX_DATAGRAM];

	sizebuf_t	signon;
	byte		signon_buf[8192];
} server_t;


#define	NUM_PING_TIMES		16
#define	NUM_SPAWN_PARMS		16

typedef struct client_s
{
	qboolean		active;				// false = client is free
	qboolean		spawned;			// false = don't send datagrams
	qboolean		dropasap;			// has been told to go to another level
	qboolean		privileged;			// can execute any host command
	qboolean		sendsignon;			// only valid before spawned

	double			last_message;		// reliable messages must be sent
										// periodically

	struct qsocket_s *netconnection;	// communications handle

	usercmd_t		cmd;				// movement
	vec3_t			wishdir;			// intended motion calced from cmd

	sizebuf_t		message;			// can be added to at any time,
										// copied and clear once per frame
	byte			msgbuf[MAX_MSGLEN];
	edict_t			*edict;				// EDICT_NUM(clientnum+1)
	char			name[32];			// for printing to other people
	int				colors;
		
	float			ping_times[NUM_PING_TIMES];
	int				num_pings;			// ping_times[num_pings%NUM_PING_TIMES]

// spawn parms are carried from level to level
	float			spawn_parms[NUM_SPAWN_PARMS];

// client known data for deltas	
	int				old_frags;
} client_t;

#endif

#endif