#ifndef NET_PROTOCOL_HEADER
#define NET_PROTOCOL_HEADER

/*
======================================================================================
Contains common definitions needed by both the Network library
and the game code.
======================================================================================
*/
const int  VOID_PROTOCOL_VERSION = 1;		//Current protcol version

//Connnectionless client to Server messages
const char C2S_GETSTATUS[]		= "getstatus";
const char C2S_CONNECT[]		= "connect";
const char C2S_GETCHALLENGE[]	= "getchallenge";

//Connectionless server to client messages
const char S2C_REJECT[]			= "reject";
const char S2C_CHALLENGE[]		= "challenge";
const char S2C_STATUS[]			= "status";
const char S2C_ACCEPT[]			= "accept";

//Common connectionless messages
const char VNET_PING[]			= "ping";


/*
======================================================================================
Connection spawning protocol
When the server sends client the S2C_ACCEPT message, the client switches to Spawn mode.
The client then requests each of the following messages from the server one by one.
These are reliablly send.
Once the client has received and acked all the messages it switches to Ingame mode and.
once the server has received all the acks it switches its netclient to Ingame mode
======================================================================================
*/
const byte SVC_INITCONNECTION= 1;	//Send the server vars, map info
const byte SVC_MODELLIST	 = 2;	//Sequenced list of models in use
const byte SVC_SOUNDLIST	 = 3;	//Sequenced list of sounds in use 
const byte SVC_IMAGELIST	 = 4;	//Sequenced list of images in use 
const byte SVC_BASELINES	 = 5;	//Static entity baselines data
const byte SVC_BEGIN		 = 6;	//ready to spawn

/*
======================================================================================
The Game Protocol
======================================================================================
*/
//Client to server
const byte CL_BAD			= 0;	//Drop me
const byte CL_NOP			= 1;	//Nothing doing. Keep alive
const byte CL_MOVE			= 2;	//angles/velocity
const byte CL_STRING		= 3;	//Command string
const byte CL_DELTA			= 4;	//update a specific field only ?
const byte CL_TALK			= 5;	//client said something
const byte CL_UPDATEINFO	= 6;	//client wants to update its info. name/rate/skin/model etc
									// n=name r=rate
const byte CL_DISCONNECT	= 15;	//client is disconnecting

//Server to Client
const byte SV_BAD			= 0;
const byte SV_NOP			= 1;
const byte SV_DISCONNECT	= 2;	// Server going down
const byte SV_UPDATESTAT	= 3;	
const byte SV_STUFFCMD		= 4;	// Client will execute this locally
const byte SV_PRINT			= 5;    // Client will print this locally
const byte SV_TALK			= 7;	// chat message
const byte SV_RECONNECT		= 10;	// Server is changing maps, tell all clients to reconnect










/*
#define	svc_setview			5	// [short] entity number
#define	svc_sound			6	// <see code>
#define	svc_serverdata		11	// [long] protocol ...
#define	svc_lightstyle		12	// [byte] [string]
//define	svc_updatename		13	// [byte] [string]
#define	svc_updatefrags		14	// [byte] [short]
//define	svc_clientdata		15	// <shortbits + data>
#define	svc_stopsound		16	// <see code>
//define	svc_updatecolors	17	// [byte] [byte] [byte]
//define	svc_particle		18	// [vec3] <variable>
#define	svc_damage			19
	
#define	svc_spawnstatic		20
//	svc_spawnbinary		21
#define	svc_spawnbaseline	22
	
#define	svc_temp_entity		23	// variable


#define	svc_killedmonster	27
#define	svc_foundsecret		28

#define	svc_spawnstaticsound	29	// [coord3] [byte] samp [byte] vol [byte] aten

#define	svc_intermission	30		// [vec3_t] origin [vec3_t] angle
#define	svc_finale			31		// [string] text

#define	svc_cdtrack			32		// [byte] track
#define svc_sellscreen		33

#define	svc_smallkick		34		// set client punchangle to 2
#define	svc_bigkick			35		// set client punchangle to 4

#define	svc_updateping		36		// [byte] [short]
#define	svc_updateentertime	37		// [byte] [float]

#define	svc_updatestatlong	38		// [byte] [long]

#define	svc_muzzleflash		39		// [short] entity

#define	svc_updateuserinfo	40		// [byte] slot [long] uid
									// [string] userinfo
#define	svc_download		41		// [short] size [size bytes]
#define	svc_playerinfo		42		// variable
#define	svc_nails			43		// [byte] num [48 bits] xyzpy 12 12 12 4 8 
#define	svc_chokecount		44		// [byte] packets choked
#define	svc_modellist		45		// [strings]
#define	svc_soundlist		46		// [strings]
#define	svc_packetentities	47		// [...]
#define	svc_deltapacketentities	48		// [...]
#define svc_maxspeed		49		// maxspeed change, for prediction
#define svc_entgravity		50		// gravity change, for prediction
#define svc_setinfo			51		// setinfo on a client
#define svc_serverinfo		52		// serverinfo
#define svc_updatepl		53		// [byte] [byte]
*/



#endif