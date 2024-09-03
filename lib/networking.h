/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NETWORKING_H
#define NETWORKING_H

#include "pstypes.h"

#if defined(WIN32)
//Windows includes
#define NOMINMAX //why do I have to include windows.h in half the project..
#include <winsock.h>

//helper macros for working with SOCKADDR_IN to make it look nicer between windows and Linux
inline void INADDR_SET_SUN_SADDR(struct in_addr* st,unsigned int value)
{
	st->S_un.S_addr = value;
}
inline void INADDR_GET_SUN_SADDR(struct in_addr* st,unsigned int *value)
{
	*value = st->S_un.S_addr;
}
inline void INADDR_SET_SUN_SUNW(struct in_addr* st,unsigned short s_w1,unsigned short s_w2)
{
	st->S_un.S_un_w.s_w1 = s_w1;
	st->S_un.S_un_w.s_w2 = s_w2;
}
inline void INADDR_GET_SUN_SUNW(struct in_addr* st,unsigned short *s_w1,unsigned short *s_w2)
{
	*s_w1 = st->S_un.S_un_w.s_w1;
	*s_w2 = st->S_un.S_un_w.s_w2;
}
inline void INADDR_SET_SUN_SUNB(struct in_addr* st,unsigned char s_b1,unsigned char s_b2,unsigned char s_b3,unsigned char s_b4)
{
	st->S_un.S_un_b.s_b1 = s_b1;
	st->S_un.S_un_b.s_b2 = s_b2;
	st->S_un.S_un_b.s_b3 = s_b3;
	st->S_un.S_un_b.s_b4 = s_b4;
}
inline void INADDR_GET_SUN_SUNB(struct in_addr* st,unsigned char *s_b1,unsigned char *s_b2,unsigned char *s_b3,unsigned char *s_b4)
{
	*s_b1 = st->S_un.S_un_b.s_b1;
	*s_b2 = st->S_un.S_un_b.s_b2;
	*s_b3 = st->S_un.S_un_b.s_b3;
	*s_b4 = st->S_un.S_un_b.s_b4;
}

#elif defined(__LINUX__)
//Linux includes/defines

#if __SUPPORT_IPX
#include <linux/types.h>
#include <linux/ipx.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

//rcg06212000 my SDL adds.
#include "SDL.h"
#include "SDL_thread.h"

#include "linux/linux_fix.h"

#ifndef SOCKET
#define SOCKET int
#endif

#define BOOL bool
#define SOCKADDR_IN sockaddr_in
#define SOCKADDR_IPX sockaddr_ipx
#define SOCKADDR sockaddr
#define INVALID_SOCKET -1
#define NSPROTO_IPX AF_IPX

#ifdef TRUE
#undef TRUE
#endif
#define TRUE true

#ifdef FALSE
#undef FALSE
#endif
#define FALSE false
#define HOSTENT struct hostent
#define SOCKET_ERROR -1

//Winsock = sockets error translation
#define WSAEWOULDBLOCK	EWOULDBLOCK
#define WSAEINVAL			EINVAL
#define WSAENOPROTOOPT	ENOPROTOOPT

inline int WSAGetLastError(){return errno;}
extern bool Use_DirectPlay;

//helper macros for working with SOCKADDR_IN to make it look nicer between windows and Linux
inline void INADDR_SET_SUN_SADDR(struct in_addr* st,unsigned int value)
{
	st->s_addr = value;
}
inline void INADDR_GET_SUN_SADDR(struct in_addr* st,unsigned int *value)
{
	*value = st->s_addr;
}
inline void INADDR_SET_SUN_SUNW(struct in_addr* st,unsigned short s_w1,unsigned short s_w2)
{
	union{
		struct{ unsigned char s_b1,s_b2,s_b3,s_b4;}S_un_b;
		struct{	unsigned short s_w1,s_w2;} S_un_w;
		unsigned int S_addr;
	}S_un;

	S_un.S_un_w.s_w1 = s_w1;
	S_un.S_un_w.s_w2 = s_w2;
	st->s_addr = S_un.S_addr;
}
inline void INADDR_GET_SUN_SUNW(struct in_addr* st,unsigned short *s_w1,unsigned short *s_w2)
{
	union{
		struct{ unsigned char s_b1,s_b2,s_b3,s_b4;}S_un_b;
		struct{	unsigned short s_w1,s_w2;} S_un_w;
		unsigned int S_addr;
	}S_un;

	S_un.S_addr = st->s_addr;
	*s_w1 = S_un.S_un_w.s_w1;
	*s_w2 = S_un.S_un_w.s_w2;
}
inline void INADDR_SET_SUN_SUNB(struct in_addr* st,unsigned char s_b1,unsigned char s_b2,unsigned char s_b3,unsigned char s_b4)
{
	union{
		struct{ unsigned char s_b1,s_b2,s_b3,s_b4;}S_un_b;
		struct{	unsigned short s_w1,s_w2;} S_un_w;
		unsigned int S_addr;
	}S_un;

	S_un.S_un_b.s_b1 = s_b1;
	S_un.S_un_b.s_b2 = s_b2;
	S_un.S_un_b.s_b3 = s_b3;
	S_un.S_un_b.s_b4 = s_b4;
	st->s_addr = S_un.S_addr;
}
inline void INADDR_GET_SUN_SUNB(struct in_addr* st,unsigned char *s_b1,unsigned char *s_b2,unsigned char *s_b3,unsigned char *s_b4)
{
	union{
		struct{ unsigned char s_b1,s_b2,s_b3,s_b4;}S_un_b;
		struct{	unsigned short s_w1,s_w2;} S_un_w;
		unsigned int S_addr;
	}S_un;

	S_un.S_addr = st->s_addr;
	*s_b1 = S_un.S_un_b.s_b1;
	*s_b2 = S_un.S_un_b.s_b2;
	*s_b3 = S_un.S_un_b.s_b3;
	*s_b4 = S_un.S_un_b.s_b4;
}
#elif defined(MACINTOSH)

#include <OpenTransport.h>
#include <OpenTptXti.h>
#include <OpenTptInternet.h>
#include "otsockets.h"

#include "macsock.h"

#define BOOL bool
#define SOCKET unsigned int
#define SOCKADDR_IN sockaddr_in
//#define SOCKADDR_IPX sockaddr_ipx
#define SOCKADDR sockaddr
//#define INVALID_SOCKET -1
//#define NSPROTO_IPX AF_IPX
#define TRUE true
#define FALSE false
#define HOSTENT struct hostent
//#define SOCKET_ERROR -1

//Winsock = sockets error translation
//#define WSAEWOULDBLOCK	EWOULDBLOCK
//#define WSAEINVAL			EINVAL
//#define WSAENOPROTOOPT	ENOPROTOOPT

extern bool Use_DirectPlay;

//#ifdef FIXED
//inline int WSAGetLastError(){return errno;}

//helper macros for working with SOCKADDR_IN to make it look nicer between windows and Linux
inline void INADDR_SET_SUN_SADDR(struct in_addr* st,unsigned int value)
{
	st->S_un.S_addr = value;
}
inline void INADDR_GET_SUN_SADDR(struct in_addr* st,unsigned int *value)
{
	*value = st->S_un.S_addr;
}
inline void INADDR_SET_SUN_SUNW(struct in_addr* st,unsigned short s_w1,unsigned short s_w2)
{
	st->S_un.S_un_w.s_w1 = s_w1;
	st->S_un.S_un_w.s_w2 = s_w2;
}
inline void INADDR_GET_SUN_SUNW(struct in_addr* st,unsigned short *s_w1,unsigned short *s_w2)
{
	*s_w1 = st->S_un.S_un_w.s_w1;
	*s_w2 = st->S_un.S_un_w.s_w2;
}
inline void INADDR_SET_SUN_SUNB(struct in_addr* st,unsigned char s_b1,unsigned char s_b2,unsigned char s_b3,unsigned char s_b4)
{
	st->S_un.S_un_b.s_b1 = s_b1;
	st->S_un.S_un_b.s_b2 = s_b2;
	st->S_un.S_un_b.s_b3 = s_b3;
	st->S_un.S_un_b.s_b4 = s_b4;
}
inline void INADDR_GET_SUN_SUNB(struct in_addr* st,unsigned char *s_b1,unsigned char *s_b2,unsigned char *s_b3,unsigned char *s_b4)
{
	*s_b1 = st->S_un.S_un_b.s_b1;
	*s_b2 = st->S_un.S_un_b.s_b2;
	*s_b3 = st->S_un.S_un_b.s_b3;
	*s_b4 = st->S_un.S_un_b.s_b4;
}
//#endif // FIXED
#endif	// OS


#define NWT_UNRELIABLE	1
#define NWT_RELIABLE	2


// This is the max size of a packet - DO NOT INCREASE THIS NUMBER ABOVE 512!
#define MAX_PACKET_SIZE	512
#if 1 //ndef DEMO
#define DEFAULT_GAME_PORT		D3_DEFAULT_PORT
#else
#define DEFAULT_GAME_PORT		6250
#endif
// Network flags
#define NF_CHECKSUM	1
#define NF_NOSEQINC	2

enum network_protocol
{
	NP_NONE,
	NP_TCP,
	NP_IPX,
	NP_DIRECTPLAY
};

struct network_address
{
	ubyte address[6];
	ushort port;
	ubyte net_id[4];
	network_protocol connection_type;			// IPX, IP, modem, etc.
};

extern BOOL DP_active;	
extern BOOL TCP_active;
extern BOOL IPX_active;
//Get the info from RAS
unsigned int psnet_ras_status();

// function to shutdown and close the given socket.  It takes a couple of things into consideration
// when closing, such as possibly reiniting reliable sockets if they are closed here.
void nw_CloseSocket( SOCKET *sockp );

// Inits the sockets layer to activity
void nw_InitNetworking (int iReadBufSizeOverride = -1);

// called by psnet_init to initialize the listen socket used by a host/server
int nw_InitReliableSocket();

// function which checks the Listen_socket for possibly incoming requests to be connected.
// returns 0 on error or nothing waiting.  1 if we should try to accept
int nw_CheckListenSocket(network_address *from_addr);

// Inits the sockets that the application will be using
void nw_InitSockets(ushort port);

// Connects a client to a server
void nw_ConnectToServer(SOCKET *socket, network_address *server_addr);

// Returns internet address format from string address format...ie "204.243.217.14"
// turns into 1414829242
unsigned long nw_GetHostAddressFromNumbers (char *str);

// Fills in the string with the string address from the internet address
void nw_GetNumbersFromHostAddress(network_address * address,char *str);

// returns the ip address of this computer
unsigned int nw_GetThisIP();

// function which checks the Listen_socket for possibly incoming requests to be connected.
// returns 0 on error or nothing waiting.  1 if we should try to accept
int nw_CheckListenSocket(network_address *from_addr);


// Calculates a unique ushort checksum for a stream of data
ushort nw_CalculateChecksum( void * vptr, int len );

// Sends data on an unreliable socket
int nw_Send( network_address * who_to, void * data, int len, int flags );


// nw_ReceiveFromSocket will get data out of the socket and stuff it into the packet_buffers
// nw_Receive now calls this function, then determines which of the packet buffers
// to package up and use
void nw_ReceiveFromSocket();


// routine to "free" a packet buffer
void nw_FreePacket( int id );

// nw_Recieve will call the above function to read data out of the socket.  It will then determine
// which of the buffers we should use and pass to the routine which called us
int nw_Receive( void * data, network_address *from_addr );

// nw_SendReliable sends the given data through the given reliable socket.
int nw_SendReliable(unsigned int socketid, ubyte *data, int length,bool urgent=false );

// function which reads data off of a reliable socket.  recv() should read the totaly amount of data
// available I believe.  (i.e. we shouldn't read only part of a message with one call....I may be wrong
// and this may be a source of bugs).
int nw_ReceiveReliable(SOCKET socket, ubyte *buffer, int max_len);

// Returns the current protocol in use
int nw_GetProtocolType ();

// Copies my address into the passed argument
void nw_GetMyAddress (network_address *addr);

//Sends a packet to the game tracker
int nw_SendGameTrackerPacker(void *packet);

//Checks for an incoming game tracker packet.
int nw_ReceiveGameTracker(void *packet);

//Send a packet to the pilot tracker
int nw_SendPilotTrackerPacket(void *packet);

//Checks for an incoming pilot tracker packet.
int nw_ReceivePilotTracker(void *packet);

int nw_PingCompare( const void *arg1, const void *arg2 );

// initialize the buffering system
void nw_psnet_buffer_init();

// buffer a packet (maintain order!)
void nw_psnet_buffer_packet(ubyte *data, int length, network_address *from);

// get the index of the next packet in order!
int nw_psnet_buffer_get_next(ubyte *data, int *length, network_address *from);

// get the index of the next packet in order!
int nw_psnet_buffer_get_next_by_dpid(ubyte *data, int *length, unsigned long dpid);

//This is all the reliable UDP stuff...
#define MAXNETBUFFERS			150		//Maximum network buffers (For between network and upper level functions, which is 
										//required in case of out of order packets
#define NETRETRYTIME				.75		//Time after sending before we resend
#define MIN_NET_RETRYTIME		.2
#define NETTIMEOUT				300		//Time after receiving the last packet before we drop that user
#define NETHEARTBEATTIME		10		//How often to send a heartbeat
#define MAXRELIABLESOCKETS		40		//Max reliable sockets to open at once...
#define NETBUFFERSIZE			600	//Max size of a network packet

//Network Types
#define RNT_ACK				1		//ACK Packet
#define RNT_DATA				2		//Data Packet
#define RNT_DATA_COMP		3		//Compressed Data Packet
#define RNT_REQ_CONN			4		//Requesting a connection
#define RNT_DISCONNECT		5		//Disconnecting a connection
#define RNT_HEARTBEAT		6		//Heartbeat -- send every NETHEARTBEATTIME
#define RNT_I_AM_HERE		7

//Reliable socket states
#define RNF_UNUSED			0		//Completely clean socket..
#define RNF_CONNECTED		1		//Connected and running fine
#define RNF_BROKEN			2		//Broken - disconnected abnormally
#define RNF_DISCONNECTED	3		//Disconnected cleanly
#define RNF_CONNECTING		4		//We received the connecting message, but haven't told the game yet.
#define RNF_LIMBO				5		//between connecting and connected

void nw_SendReliableAck(SOCKADDR *raddr,unsigned int sig, network_protocol link_type,float time_sent);
void nw_WorkReliable(ubyte * data,int len,network_address *naddr);
int nw_Compress(void *srcdata,void *destdata,int count);
int nw_Uncompress(void *compdata,void *uncompdata,int count);

#define NW_AGHBN_CANCEL		1
#define NW_AGHBN_LOOKUP		2
#define NW_AGHBN_READ		3

struct async_dns_lookup
{
	unsigned int ip;	//resolved host. Write only to worker thread.
	char * host;//host name to resolve. read only to worker thread
	bool done;	//write only to the worker thread. Signals that the operation is complete
	bool error; //write only to worker thread. Thread sets this if the name doesn't resolve
	bool abort;	//read only to worker thread. If this is set, don't fill in the struct.

        // rcg06212000 added to let us join the thread at completion...
    #ifdef __LINUX__
        SDL_Thread *threadId;
    #endif
};

#ifdef WIN32
#define CDECLCALL	__cdecl
#else
#define CDECLCALL
#endif
#ifdef __LINUX__
// rcg06192000 used to return void *.
int CDECLCALL gethostbynameworker(void *parm);
#else
void CDECLCALL gethostbynameworker(void *parm);
#endif

int nw_Asyncgethostbyname(unsigned int *ip,int command, char *hostname);
int nw_ReccomendPPS();
void nw_DoNetworkIdle(void);

typedef void * ( * NetworkReceiveCallback ) (ubyte *data,int len, network_address *from);
int nw_RegisterCallback(NetworkReceiveCallback nfp, ubyte id);
NetworkReceiveCallback nw_UnRegisterCallback(ubyte id);
int nw_SendWithID(ubyte id,ubyte *data,int len,network_address *who_to);
int nw_DoReceiveCallbacks(void);
void nw_HandleConnectResponse(ubyte *data,int len,network_address *server_addr);
int nw_RegisterCallback(NetworkReceiveCallback nfp, ubyte id);
void nw_HandleUnreliableData(ubyte *data,int len,network_address *from_addr);
void nw_ReliableResend(void);
int nw_CheckReliableSocket(int socknum);

struct tNetworkStatus
{
	// TCP/IP Status lines
	int udp_total_packets_sent;              // total number of packets sent out on the network (unreliable)
	int udp_total_packets_rec;               // total number of packets recieved on the network (unrealiable)
	int udp_total_bytes_sent;                // total number of bytes sent (unreliable)
	int udp_total_bytes_rec;                 // total number of bytes recieved (unreliable)
	
	int tcp_total_packets_sent;              // total number of packets sent out on the network (reliable)
	int tcp_total_packets_rec;               // total number of packets recieved on the network (reliable)
	int tcp_total_bytes_sent;                // total number of bytes sent (reliable)
	int tcp_total_bytes_rec;                 // total number of bytes recieved (reliable)
	int tcp_total_packets_resent;            // total number of packets resent (reliable)
	int tcp_total_bytes_resent;              // total number of bytes resent (reliable)

	// IPX Status lines
	int ipx_total_packets_sent;              // total number of packets sent out on the network (unreliable)
	int ipx_total_packets_rec;               // total number of packets recieved on the network (unrealiable)
	int ipx_total_bytes_sent;                // total number of bytes sent (unreliable)
	int ipx_total_bytes_rec;                 // total number of bytes recieved (unreliable)
	
	int spx_total_packets_sent;              // total number of packets sent out on the network (reliable)
	int spx_total_packets_rec;               // total number of packets recieved on the network (reliable)
	int spx_total_bytes_sent;                // total number of bytes sent (reliable)
	int spx_total_bytes_rec;                 // total number of bytes recieved (reliable)
	int spx_total_packets_resent;            // total number of packets resent (reliable)
	int spx_total_bytes_resent;              // total number of bytes resent (reliable)
};	// network status information

// fills in the buffer with network stats
// pass NULL to reset the stats
void nw_GetNetworkStats(tNetworkStatus *stats);

#endif
