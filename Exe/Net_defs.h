#ifndef VOID_NET_DEFS
#define VOID_NET_DEFS

namespace VoidNet
{

//Connnectionless client to Server messages
const char C2S_PING[]			= "ping";
const char C2S_STATUS[]			= "status";
const char C2S_CONNECT[]		= "connect";
const char C2S_GETCHALLENGE[]	= "getchallenge";

//Connectionless server to client messages


}

#endif





#if 0

#ifndef _NET_DEFS
#define _NET_DEFS


//#include <winsock2.h>

/*
================================
Void Network protocol definition
================================
*/

#define PROTOCOL_VERSION	1		//network protocol version
#define GAME_VERSION		1		//game version

#define SERVER_PORT	36666
#define CLIENT_PORT	36667

//timeout values
#define CON_TIMEOUT		5.0
#define GAME_TIMEOUT	8.0

//max string/buffer sizes
#define MAX_ADDRLEN		16
#define MAX_MESSAGELEN	4096
#define MAX_DATAGRAM	512	

//maximum connections the servers listener socket will handle
#define MAX_CONNECTQ	8			

#define INFOSTRING_DELIM '\\'		//delimiter character used for infostrings

/*
=====================================
Client to server messages types
=====================================
*/

#define	CL_BAD			0			//bad message, notify the server
#define CL_DISCONNECT	1			//client is disconnecting
#define CL_NOP			2			//nothing happened, im alive
#define CL_CMD			3			//move dir, view, buttons
#define CL_STRING		4			//string command (talk etc)
#define CL_SVCMDS		5			//special commands



/*
=====================================
Server to client message types
=====================================
*/


#define SV_BAD			0			//bad message
#define SV_DISCONNECT	1			//disconnect
#define SV_NOP			2			//no-op
#define SV_INFO			3			//server info
#define SV_PRINT		4			//print in the console / hud
#define SV_CONSTEXT		5			//enter in the console
#define SV_CLIENT		7			//client update (movement,velocity)
#define SV_CLUPDATE		8			//other client angles and movement
#define SV_ENTUPDATE	9			//entity update
#define SV_SOUND		10			//sound
#define SV_MUSIC		11			//music




/*
GAME PROTOCOL

once a connection is negotiated
another nonblocking socket is created for that client 
and the server sends status updates and receives button states 
and movement velocities from the client

4 bytes - packetnum		(reliable if high bit is 1)
1 byte  - packet type


*/









//Server listener ports will follow


/*
CONNECTION PROTOCOL

The FIRST socket is a dedicated listener
and only handles connection requests and 
server/player info requests

-"getstatus"
-"getinfo"
-"getchallenge"
*/









/*
=====================================
Client Request Packet formats
=====================================
*/

#define VREQ_CONNECT		0x01
#define VREQ_SVINFO			0x02
#define VREQ_SVSTATUS		0x03
#define VREP_PING			0x04


// VREQ_CONNECT
//		string	game_name				"VOID"
//		byte	net_protocol_version	NET_PROTOCOL_VERSION
//		byte	game_protocol_version	PROTOCOL_VERSION
//

// VREQ_SVINFO
//		string	game_name				"VOID"
//		byte	net_protocol_version	NET_PROTOCOL_VERSION
//		byte	game_protocol_version	PROTOCOL_VERSION

// VREQ_SVSTATUS


// VREQ_PING
//		byte	data[64]


//server replies

#define VREP_CHALLENGE		0x05
#define VREP_ACCEPT			0x06
#define VREP_REJECT			0x07
#define VREP_SVINFO			0x08
#define VREP_SVSTATUS		0x09



// This is the network info/connection protocol.  It is used to find Quake
// servers, get info about them, and connect to them.  Once connected, the
// Quake game protocol (documented elsewhere) is used.
//
//
// General notes:
//	game_name is currently always "QUAKE", but is there so this same protocol
//		can be used for future games as well; can you say Quake2?
//

//
// CCREQ_PLAYER_INFO
//		byte	player_number
//
// CCREQ_RULE_INFO
//		string	rule
//
// CCREQ_RULE_NEXT_INFO
//		string	previous_rule
//
// CCREQ_PING
//		
//
//
//
// CCREP_ACCEPT
//		long	port
//
// CCREP_REJECT
//		string	reason
//
// CCREP_SERVER_INFO
//		byte	protocol_version	NET_PROTOCOL_VERSION
//		byte	address_form		;0 short form, 1 long form
//			long	port
//		OR
//			string	server_address
//		string	host_name
//		string	level_name
//		byte	current_players
//		byte	max_players
//
// CCREP_PLAYER_INFO
//		byte	player_number
//		string	name
//		long	colors
//		long	frags
//		long	connect_time
//		string	address
//
// CCREP_RULE_INFO
//		string	rule
//		string	value
//
// CCREP_PING
//		byte	data[64]

//	note:
//		There are two address forms used above.  The short form is just a
//		port number.  The address that goes along with the port is defined as
//		"whatever address you receive this reponse from".  This lets us use
//		the host OS to solve the problem of multiple host addresses (possibly
//		with no routing between them); the host will use the right address
//		when we reply to the inbound connection request.  The long from is
//		a full address and port in a string.  It is used for returning the
//		address of a server that is not running locally.

#define CCREQ_CONNECT		0x01
#define CCREQ_SERVER_INFO	0x02
#define CCREQ_PLAYER_INFO	0x03
#define CCREQ_RULE_INFO		0x04
#define CCREQ_PING			0x05

#define CCREP_ACCEPT		0x81
#define CCREP_REJECT		0x82
#define CCREP_SERVER_INFO	0x83
#define CCREP_PLAYER_INFO	0x84
#define CCREP_RULE_INFO		0x85
#define CCREP_PING			0x86









// protocol.h -- communications protocols

//#define	PROTOCOL_VERSION	15

// if the high bit of the servercmd is set, the low bits are fast update flags:
#define	U_MOREBITS	(1<<0)
#define	U_ORIGIN1	(1<<1)
#define	U_ORIGIN2	(1<<2)
#define	U_ORIGIN3	(1<<3)
#define	U_ANGLE2	(1<<4)
#define	U_NOLERP	(1<<5)		// don't interpolate movement
#define	U_FRAME		(1<<6)
#define U_SIGNAL	(1<<7)		// just differentiates from other updates

// svc_update can pass all of the fast update bits, plus more
#define	U_ANGLE1	(1<<8)
#define	U_ANGLE3	(1<<9)
#define	U_MODEL		(1<<10)
#define	U_COLORMAP	(1<<11)
#define	U_SKIN		(1<<12)
#define	U_EFFECTS	(1<<13)
#define	U_LONGENTITY	(1<<14)


#define	SU_VIEWHEIGHT	(1<<0)
#define	SU_IDEALPITCH	(1<<1)
#define	SU_PUNCH1		(1<<2)
#define	SU_PUNCH2		(1<<3)
#define	SU_PUNCH3		(1<<4)
#define	SU_VELOCITY1	(1<<5)
#define	SU_VELOCITY2	(1<<6)
#define	SU_VELOCITY3	(1<<7)
//define	SU_AIMENT		(1<<8)  AVAILABLE BIT
#define	SU_ITEMS		(1<<9)
#define	SU_ONGROUND		(1<<10)		// no data follows, the bit is it
#define	SU_INWATER		(1<<11)		// no data follows, the bit is it
#define	SU_WEAPONFRAME	(1<<12)
#define	SU_ARMOR		(1<<13)
#define	SU_WEAPON		(1<<14)

// a sound with no channel is a local only sound
#define	SND_VOLUME		(1<<0)		// a byte
#define	SND_ATTENUATION	(1<<1)		// a byte
#define	SND_LOOPING		(1<<2)		// a long


// defaults for clientinfo messages
#define	DEFAULT_VIEWHEIGHT	22
#define	DEFAULT_ITEMS		16385



//==================
// note that there are some defs.qc that mirror to these numbers
// also related to svc_strings[] in cl_parse
//==================

//
// server to client
//
#define	svc_bad				0
#define	svc_nop				1
#define	svc_disconnect		2
#define	svc_updatestat		3	// [byte] [long]
#define	svc_version			4	// [long] server version
#define	svc_setview			5	// [short] entity number
#define	svc_sound			6	// <see code>
#define	svc_time			7	// [float] server time
#define	svc_print			8	// [string] null terminated string
#define	svc_stufftext		9	// [string] stuffed into client's console buffer
								// the string should be \n terminated
#define	svc_setangle		10	// [angle3] set the view angle to this absolute value
	
#define	svc_serverinfo		11	// [long] version
						// [string] signon string
						// [string]..[0]model cache
						// [string]...[0]sounds cache
#define	svc_lightstyle		12	// [byte] [string]
#define	svc_updatename		13	// [byte] [string]
#define	svc_updatefrags		14	// [byte] [short]
#define	svc_clientdata		15	// <shortbits + data>
#define	svc_stopsound		16	// <see code>
#define	svc_updatecolors	17	// [byte] [byte]
#define	svc_particle		18	// [vec3] <variable>
#define	svc_damage			19
	
#define	svc_spawnstatic		20
//	svc_spawnbinary		21
#define	svc_spawnbaseline	22
	
#define	svc_temp_entity		23

#define	svc_setpause		24	// [byte] on / off
#define	svc_signonnum		25	// [byte]  used for the signon sequence

#define	svc_centerprint		26	// [string] to put in center of the screen

#define	svc_spawnstaticsound	29	// [coord3] [byte] samp [byte] vol [byte] aten


#endif


#endif