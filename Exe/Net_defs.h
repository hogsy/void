#ifndef VOID_NET_DEFS
#define VOID_NET_DEFS

namespace VoidNet
{
enum
{	CL_FREE = 0,		//nothing doing. can be used
	CL_INUSE = 1,		//trying to connect
	CL_CONNECTED = 2,	//havent spawned yet
	CL_SPAWNED = 4		//in game
};

const char szWORLDDIR[]     = "Worlds/";

const int  SV_DEFAULT_PORT = 20010;

const int  PACKET_HEADER	  =	8;
const int  MAX_DATAGRAM_SIZE  =	1450;
const int  MAX_BUFFER_SIZE	  = 2900;

const int  VOID_PROTOCOL_VERSION = 1;

//======================================================================================

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

//======================================================================================
//Connection spawning protocol
/*
When the server sends client the S2C_ACCEPT message, it follows up
with the following messages in order, until all the spawn data is send
the client just echos the messages which prompt the server to keep sending data
*/
const int SVC_INITCONNECTION= 1;	//Send the server vars, map info
const int SVC_MODELLIST		= 2;	//Sequenced list of models in use
const int SVC_SOUNDLIST		= 3;	//Sequenced list of sounds in use 
const int SVC_IMAGELIST		= 4;	//Sequenced list of images in use 
const int SVC_BASELINES		= 5;	//Static entity baselines data
const int SVC_SPAWN			= 6;	//Dynamic Spawning info. client is assumed to be spawned after this

//======================================================================================
//Game Protocol

//===============================
//Client to server

const int CL_BAD			= 0;	//Drop me
const int CL_NOP			= 1;	//Nothing doing. Keep alive
const int CL_MOVE			= 2;	//angles/velocity
const int CL_STRING			= 3;	//Command string
const int CL_DELTA			= 4;	//update a specific field only ?

//===============================
//Server to Client
const int SVC_BAD			= 0;
const int SVC_NOP			= 1;
const int SVC_DISCONNECT	= 2;	// Server going down
const int SVC_UPDATESTAT	= 3;	
const int SVC_STUFFCMD		= 4;	// Client will execute this locally
const int SVC_HUDPRINT		= 5;    // Client will print this locally

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

}

#endif

