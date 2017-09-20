/*****************************************************************************
 * SRT - Secure, Reliable, Transport
 * Copyright (c) 2017 Haivision Systems Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; If not, see <http://www.gnu.org/licenses/>
 * 
 *****************************************************************************/

/*****************************************************************************
written by
   Haivision Systems Inc.
 *****************************************************************************/

#ifndef INC__SRTC_H
#define INC__SRTC_H

#include "platform_sys.h"

#include <string.h>
#include <stdlib.h>

#include "srt4udt.h"
#include "logging_api.h"

////////////////////////////////////////////////////////////////////////////////

//if compiling on VC6.0 or pre-WindowsXP systems
//use -DLEGACY_WIN32

//if compiling with MinGW, it only works on XP or above
//use -D_WIN32_WINNT=0x0501


#ifdef WIN32
   #ifndef __MINGW__
      // Explicitly define 32-bit and 64-bit numbers
      typedef __int32 int32_t;
      typedef __int64 int64_t;
      typedef unsigned __int32 uint32_t;
      #ifndef LEGACY_WIN32
         typedef unsigned __int64 uint64_t;
      #else
         // VC 6.0 does not support unsigned __int64: may cause potential problems.
         typedef __int64 uint64_t;
      #endif

	#ifdef UDT_DYNAMIC
      #ifdef UDT_EXPORTS
         #define SRT_API __declspec(dllexport)
      #else
         #define SRT_API __declspec(dllimport)
      #endif
	#else
		#define SRT_API
	#endif
   #else
      #define SRT_API
   #endif
#else
   #define SRT_API __attribute__ ((visibility("default")))
#endif

#define NO_BUSY_WAITING

// For feature tests if you need.
// You can use these constants with SRTO_MINVERSION option.
#define SRT_VERSION_FEAT_HSv5 0x010300


// To construct version value
#define SRT_MAKE_VERSION(major, minor, patch) ((patch)+((minor)*0x100)+((major)*0x10000))

#ifdef __GNUG__
#define SRT_ATR_UNUSED __attribute__((unused))
#define SRT_ATR_DEPRECATED __attribute__((deprecated))
#else
#define SRT_ATR_UNUSED
#define SRT_ATR_DEPRECATED
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int SRTSOCKET; // SRTSOCKET is a typedef to int anyway, and it's not even in UDT namespace :)

#ifdef WIN32
   #ifndef __MINGW__
      typedef SOCKET SYSSOCKET;
   #else
      typedef int SYSSOCKET;
   #endif
#else
   typedef int SYSSOCKET;
#endif

typedef SYSSOCKET UDPSOCKET;


// Values returned by srt_getsockstate()
typedef enum SRT_SOCKSTATUS {
	SRTS_INIT = 1,
	SRTS_OPENED,
	SRTS_LISTENING,
	SRTS_CONNECTING,
	SRTS_CONNECTED,
	SRTS_BROKEN,
	SRTS_CLOSING,
	SRTS_CLOSED,
	SRTS_NONEXIST
} SRT_SOCKSTATUS;

// This is a duplicate enum. Must be kept in sync with the original UDT enum for
// backward compatibility until all compat is destroyed.
typedef enum SRT_SOCKOPT {

	SRTO_MSS,             // the Maximum Transfer Unit
	SRTO_SNDSYN,          // if sending is blocking
	SRTO_RCVSYN,          // if receiving is blocking
	SRTO_CC,              // custom congestion control algorithm
	SRTO_FC,              // Flight flag size (window size)
	SRTO_SNDBUF,          // maximum buffer in sending queue
	SRTO_RCVBUF,          // UDT receiving buffer size
	SRTO_LINGER,          // waiting for unsent data when closing
	SRTO_UDP_SNDBUF,      // UDP sending buffer size
	SRTO_UDP_RCVBUF,      // UDP receiving buffer size
	SRTO_MAXMSG,          // maximum datagram message size
	SRTO_MSGTTL,          // time-to-live of a datagram message
	SRTO_RENDEZVOUS,      // rendezvous connection mode
	SRTO_SNDTIMEO,        // send() timeout
	SRTO_RCVTIMEO,        // recv() timeout
	SRTO_REUSEADDR,       // reuse an existing port or create a new one
	SRTO_MAXBW,           // maximum bandwidth (bytes per second) that the connection can use
	SRTO_STATE,           // current socket state, see UDTSTATUS, read only
	SRTO_EVENT,           // current available events associated with the socket
	SRTO_SNDDATA,         // size of data in the sending buffer
	SRTO_RCVDATA,         // size of data available for recv
	SRTO_SENDER = 21,     // Sender mode (independent of conn mode), for encryption, tsbpd handshake.
	SRTO_TSBPDMODE = 22,  // Enable/Disable TsbPd. Enable -> Tx set origin timestamp, Rx deliver packet at origin time + delay
    SRTO_LATENCY = 23,    // DEPRECATED. SET: to both SRTO_RCVLATENCY and SRTO_PEERLATENCY. GET: same as SRTO_RCVLATENCY.
	SRTO_TSBPDDELAY = 23, // ALIAS: SRTO_LATENCY
	SRTO_INPUTBW = 24,    // Estimated input stream rate.
	SRTO_OHEADBW,         // MaxBW ceiling based on % over input stream rate. Applies when UDT_MAXBW=0 (auto).
	SRTO_PASSPHRASE = 26, // Crypto PBKDF2 Passphrase size[0,10..64] 0:disable crypto
	SRTO_PBKEYLEN,        // Crypto key len in bytes {16,24,32} Default: 16 (128-bit)
	SRTO_KMSTATE,         // Key Material exchange status (UDT_SRTKmState)
	SRTO_IPTTL = 29,      // IP Time To Live (passthru for system sockopt IPPROTO_IP/IP_TTL)
	SRTO_IPTOS,           // IP Type of Service (passthru for system sockopt IPPROTO_IP/IP_TOS)
	SRTO_TLPKTDROP = 31,  // Enable receiver pkt drop
    // deprecated: SRTO_TSBPDMAXLAG (@c below)
	SRTO_NAKREPORT = 33,  // Enable receiver to send periodic NAK reports
	SRTO_VERSION = 34,    // Local SRT Version
	SRTO_PEERVERSION,     // Peer SRT Version (from SRT Handshake)
	SRTO_CONNTIMEO = 36,   // Connect timeout in msec. Ccaller default: 3000, rendezvous (x 10)
    // deprecated: SRTO_TWOWAYDATA (@c below)
    SRTO_SNDPBKEYLEN = 38, // (DEPRECATED: use SRTO_PBKEYLEN)
    SRTO_RCVPBKEYLEN,      // (DEPRECATED: use SRTO_PBKEYLEN)
    SRTO_SNDPEERKMSTATE,  // (GET) the current state of the encryption at the peer side
    SRTO_RCVKMSTATE,      // (GET) the current state of the encryption at the agent side
    SRTO_LOSSMAXTTL,      // Maximum possible packet reorder tolerance (number of packets to receive after loss to send lossreport)
    SRTO_RCVLATENCY,      // TsbPd receiver delay (mSec) to absorb burst of missed packet retransmission
    SRTO_PEERLATENCY,     // Minimum value of the TsbPd receiver delay (mSec) for the opposite side (peer)
    SRTO_MINVERSION,      // Minimum SRT version needed for the peer (peers with less version will get connection reject)
    SRTO_STREAMID,         // A string set to a socket and passed to the listener's accepted socket
    SRTO_SMOOTHER,         // Smoother selection (congestion control algorithm)
    SRTO_MESSAGEAPI,
    SRTO_PAYLOADSIZE,
    SRTO_TRANSTYPE         // Transmission type (set of options required for given transmission type)
} SRT_SOCKOPT;

// SRTO_TWOWAYDATA: not to be used. SRT connection is always bidirectional if
// both clients support HSv5 - that is, since version 1.3.0
static const SRT_SOCKOPT SRTO_TWOWAYDATA SRT_ATR_DEPRECATED = (SRT_SOCKOPT)37;

// This has been deprecated a long time ago, treat this as never implemented.
static const SRT_SOCKOPT SRTO_TSBPDMAXLAG SRT_ATR_DEPRECATED = (SRT_SOCKOPT)32;


typedef enum SRT_TRANSTYPE
{
    SRTT_LIVE,
    SRTT_VOD,
    SRTT_INVALID
} SRT_TRANSTYPE;


struct CBytePerfMon
{
   // global measurements
   int64_t msTimeStamp;                 // time since the UDT entity is started, in milliseconds
   int64_t pktSentTotal;                // total number of sent data packets, including retransmissions
   int64_t pktRecvTotal;                // total number of received packets
   int pktSndLossTotal;                 // total number of lost packets (sender side)
   int pktRcvLossTotal;                 // total number of lost packets (receiver side)
   int pktRetransTotal;                 // total number of retransmitted packets
   int pktSentACKTotal;                 // total number of sent ACK packets
   int pktRecvACKTotal;                 // total number of received ACK packets
   int pktSentNAKTotal;                 // total number of sent NAK packets
   int pktRecvNAKTotal;                 // total number of received NAK packets
   int64_t usSndDurationTotal;		// total time duration when UDT is sending data (idle time exclusive)
   //>new
   int pktSndDropTotal;                 // number of too-late-to-send dropped packets
   int pktRcvDropTotal;                 // number of too-late-to play missing packets
   int pktRcvUndecryptTotal;            // number of undecrypted packets
   uint64_t byteSentTotal;              // total number of sent data bytes, including retransmissions
   uint64_t byteRecvTotal;              // total number of received bytes
#ifdef SRT_ENABLE_LOSTBYTESCOUNT
   uint64_t byteRcvLossTotal;           // total number of lost bytes
#endif
   uint64_t byteRetransTotal;           // total number of retransmitted bytes
   uint64_t byteSndDropTotal;           // number of too-late-to-send dropped bytes
   uint64_t byteRcvDropTotal;           // number of too-late-to play missing bytes (estimate based on average packet size)
   uint64_t byteRcvUndecryptTotal;      // number of undecrypted bytes
   //<

   // local measurements
   int64_t pktSent;                     // number of sent data packets, including retransmissions
   int64_t pktRecv;                     // number of received packets
   int pktSndLoss;                      // number of lost packets (sender side)
   int pktRcvLoss;                      // number of lost packets (receiver side)
   int pktRetrans;                      // number of retransmitted packets
   int pktRcvRetrans;                   // number of retransmitted packets received
   int pktSentACK;                      // number of sent ACK packets
   int pktRecvACK;                      // number of received ACK packets
   int pktSentNAK;                      // number of sent NAK packets
   int pktRecvNAK;                      // number of received NAK packets
   double mbpsSendRate;                 // sending rate in Mb/s
   double mbpsRecvRate;                 // receiving rate in Mb/s
   int64_t usSndDuration;		// busy sending time (i.e., idle time exclusive)
   int pktReorderDistance;              // size of order discrepancy in received sequences
   double pktRcvAvgBelatedTime;             // average time of packet delay for belated packets (packets with sequence past the ACK)
   int64_t pktRcvBelated;              // number of received AND IGNORED packets due to having come too late
   //>new
   int pktSndDrop;                      // number of too-late-to-send dropped packets
   int pktRcvDrop;                      // number of too-late-to play missing packets
   int pktRcvUndecrypt;                 // number of undecrypted packets
   uint64_t byteSent;                   // number of sent data bytes, including retransmissions
   uint64_t byteRecv;                   // number of received bytes
#ifdef SRT_ENABLE_LOSTBYTESCOUNT
   uint64_t byteRcvLoss;                // number of retransmitted bytes
#endif
   uint64_t byteRetrans;                // number of retransmitted bytes
   uint64_t byteSndDrop;                // number of too-late-to-send dropped bytes
   uint64_t byteRcvDrop;                // number of too-late-to play missing bytes (estimate based on average packet size)
   uint64_t byteRcvUndecrypt;           // number of undecrypted bytes
   //<

   // instant measurements
   double usPktSndPeriod;               // packet sending period, in microseconds
   int pktFlowWindow;                   // flow window size, in number of packets
   int pktCongestionWindow;             // congestion window size, in number of packets
   int pktFlightSize;                   // number of packets on flight
   double msRTT;                        // RTT, in milliseconds
   double mbpsBandwidth;                // estimated bandwidth, in Mb/s
   int byteAvailSndBuf;                 // available UDT sender buffer size
   int byteAvailRcvBuf;                 // available UDT receiver buffer size
   //>new
   double  mbpsMaxBW;                   // Transmit Bandwidth ceiling (Mbps)
   int     byteMSS;                     // MTU

   int     pktSndBuf;                   // UnACKed packets in UDT sender
   int     byteSndBuf;                  // UnACKed bytes in UDT sender
   int     msSndBuf;                    // UnACKed timespan (msec) of UDT sender
   int     msSndTsbPdDelay;             // Timestamp-based Packet Delivery Delay

   int     pktRcvBuf;                   // Undelivered packets in UDT receiver
   int     byteRcvBuf;                  // Undelivered bytes of UDT receiver
   int     msRcvBuf;                    // Undelivered timespan (msec) of UDT receiver
   int     msRcvTsbPdDelay;             // Timestamp-based Packet Delivery Delay
   //<
};

////////////////////////////////////////////////////////////////////////////////

// Error codes - define outside the CUDTException class
// because otherwise you'd have to use CUDTException::MJ_SUCCESS etc.
// in all throw CUDTException expressions.
enum CodeMajor
{
    MJ_UNKNOWN = -1,
    MJ_SUCCESS = 0,
    MJ_SETUP = 1,
    MJ_CONNECTION = 2,
    MJ_SYSTEMRES = 3,
    MJ_FILESYSTEM = 4,
    MJ_NOTSUP = 5,
    MJ_AGAIN = 6,
    MJ_PEERERROR = 7
};

enum CodeMinor
{
    // These are "minor" error codes from various "major" categories
    // MJ_SETUP
    MN_NONE = 0,
    MN_TIMEOUT = 1,
    MN_REJECTED = 2,
    MN_NORES = 3,
    MN_SECURITY = 4,
    // MJ_CONNECTION
    MN_CONNLOST = 1,
    MN_NOCONN = 2,
    // MJ_SYSTEMRES
    MN_THREAD = 1,
    MN_MEMORY = 2,
    // MJ_FILESYSTEM
    MN_SEEKGFAIL = 1,
    MN_READFAIL = 2,
    MN_SEEKPFAIL = 3,
    MN_WRITEFAIL = 4,
    // MJ_NOTSUP
    MN_ISBOUND = 1,
    MN_ISCONNECTED = 2,
    MN_INVAL = 3,
    MN_SIDINVAL = 4,
    MN_ISUNBOUND = 5,
    MN_NOLISTEN = 6,
    MN_ISRENDEZVOUS = 7,
    MN_ISRENDUNBOUND = 8,
    MN_INVALMSGAPI = 9,
    MN_INVALBUFFERAPI = 10,
    MN_BUSY = 11,
    MN_XSIZE = 12,
    MN_EIDINVAL = 13,
    // MJ_AGAIN
    MN_WRAVAIL = 1,
    MN_RDAVAIL = 2,
    MN_XMTIMEOUT = 3,
    MN_CONGESTION = 4
};

static const enum CodeMinor MN_ISSTREAM SRT_ATR_DEPRECATED = 9;
static const enum CodeMinor MN_ISDGRAM SRT_ATR_DEPRECATED = 10;

// Stupid, but effective. This will be #undefined, so don't worry.
#define MJ(major) (1000*MJ_##major)
#define MN(major, minor) (1000*MJ_##major + MN_##minor)

// Some better way to define it, and better for C language.
typedef enum SRT_ERRNO
{
    SRT_EUNKNOWN = -1,
    SRT_SUCCESS = MJ_SUCCESS,

    SRT_ECONNSETUP = MJ(SETUP),
    SRT_ENOSERVER  = MN(SETUP, TIMEOUT),
    SRT_ECONNREJ   = MN(SETUP, REJECTED),
    SRT_ESOCKFAIL  = MN(SETUP, NORES),
    SRT_ESECFAIL   = MN(SETUP, SECURITY),

    SRT_ECONNFAIL =  MJ(CONNECTION),
    SRT_ECONNLOST =  MN(CONNECTION, CONNLOST),
    SRT_ENOCONN =    MN(CONNECTION, NOCONN),

    SRT_ERESOURCE =  MJ(SYSTEMRES),
    SRT_ETHREAD =    MN(SYSTEMRES, THREAD),
    SRT_ENOBUF =     MN(SYSTEMRES, MEMORY),

    SRT_EFILE =      MJ(FILESYSTEM),
    SRT_EINVRDOFF =  MN(FILESYSTEM, SEEKGFAIL),
    SRT_ERDPERM =    MN(FILESYSTEM, READFAIL),
    SRT_EINVWROFF =  MN(FILESYSTEM, SEEKPFAIL),
    SRT_EWRPERM =    MN(FILESYSTEM, WRITEFAIL),

    SRT_EINVOP =       MJ(NOTSUP),
    SRT_EBOUNDSOCK =   MN(NOTSUP, ISBOUND),
    SRT_ECONNSOCK =    MN(NOTSUP, ISCONNECTED),
    SRT_EINVPARAM =    MN(NOTSUP, INVAL),
    SRT_EINVSOCK =     MN(NOTSUP, SIDINVAL),
    SRT_EUNBOUNDSOCK = MN(NOTSUP, ISUNBOUND),
    SRT_ENOLISTEN =    MN(NOTSUP, NOLISTEN),
    SRT_ERDVNOSERV =   MN(NOTSUP, ISRENDEZVOUS),
    SRT_ERDVUNBOUND =  MN(NOTSUP, ISRENDUNBOUND),
    SRT_EINVALMSGAPI =   MN(NOTSUP, INVALMSGAPI),
    SRT_EINVALBUFFERAPI = MN(NOTSUP, INVALBUFFERAPI),
    SRT_EDUPLISTEN =   MN(NOTSUP, BUSY),
    SRT_ELARGEMSG =    MN(NOTSUP, XSIZE),
    SRT_EINVPOLLID =   MN(NOTSUP, EIDINVAL),

    SRT_EASYNCFAIL = MJ(AGAIN),
    SRT_EASYNCSND =  MN(AGAIN, WRAVAIL),
    SRT_EASYNCRCV =  MN(AGAIN, RDAVAIL),
    SRT_ETIMEOUT =   MN(AGAIN, XMTIMEOUT),
    SRT_ECONGEST =   MN(AGAIN, CONGESTION),

    SRT_EPEERERR = MJ(PEERERROR)
} SRT_ERRNO;

static const SRT_ERRNO SRT_EISSTREAM SRT_ATR_DEPRECATED = MN(NOTSUP, INVALMSGAPI);
static const SRT_ERRNO SRT_EISDGRAM = MN(NOTSUP, INVALBUFFERAPI),

#undef MJ
#undef MN

// Logging API - specialization for SRT.

// Define logging functional areas for log selection.
// Use values greater than 0. Value 0 is reserved for LOGFA_GENERAL,
// which is considered always enabled.

// Logger Functional Areas
// Note that 0 is "general".

// Made by #define so that it's available also for C API.
#define SRT_LOGFA_GENERAL 0
#define SRT_LOGFA_BSTATS 1
#define SRT_LOGFA_CONTROL 2
#define SRT_LOGFA_DATA 3
#define SRT_LOGFA_TSBPD 4
#define SRT_LOGFA_REXMIT 5

#define SRT_LOGFA_LASTNONE 99

enum SRT_KM_STATE
{
    SRT_KM_S_UNSECURED = 0,      //No encryption
    SRT_KM_S_SECURING  = 1,      //Stream encrypted, exchanging Keying Material
    SRT_KM_S_SECURED   = 2,      //Stream encrypted, keying Material exchanged, decrypting ok.
    SRT_KM_S_NOSECRET  = 3,      //Stream encrypted and no secret to decrypt Keying Material
    SRT_KM_S_BADSECRET = 4       //Stream encrypted and wrong secret, cannot decrypt Keying Material
};

enum SRT_EPOLL_OPT
{
   // this values are defined same as linux epoll.h
   // so that if system values are used by mistake, they should have the same effect
   SRT_EPOLL_IN = 0x1,
   SRT_EPOLL_OUT = 0x4,
   SRT_EPOLL_ERR = 0x8
};

#ifdef __cplusplus
// In C++ these enums cannot be treated as int and glued by operator |.
// Unless this operator is defined.
inline SRT_EPOLL_OPT operator|(SRT_EPOLL_OPT a1, SRT_EPOLL_OPT a2)
{
    return SRT_EPOLL_OPT( (int)a1 | (int)a2 );
}
#endif



typedef struct CPerfMon SRT_TRACEINFO;
typedef struct CBytePerfMon SRT_TRACEBSTATS;

// This structure is only a kind-of wannabe. The only use of it is currently
// the 'srctime', however the functionality of application-supplied timestamps
// also doesn't work properly. Left for future until the problems are solved.
// This may prove useful as currently there's no way to tell the application
// that TLPKTDROP facility has dropped some data in favor of timely delivery.
typedef struct SRT_MsgCtrl_ {
   int flags;
   int boundary;                        //0:mid pkt, 1(01b):end of frame, 2(11b):complete frame, 3(10b): start of frame
   uint64_t srctime;                    //source timestamp (usec), 0LL: use internal time     
} SRT_MSGCTRL;

static const SRTSOCKET SRT_INVALID_SOCK = -1;
static const int SRT_ERROR = -1;

// library initialization
SRT_API extern int srt_startup(void);
SRT_API extern int srt_cleanup(void);

// socket operations
SRT_API extern SRTSOCKET srt_socket(int af, int type, int protocol);
SRT_API extern int srt_bind(SRTSOCKET u, const struct sockaddr* name, int namelen);
SRT_API extern int srt_bind_peerof(SRTSOCKET u, UDPSOCKET udpsock);
SRT_API extern int srt_listen(SRTSOCKET u, int backlog);
SRT_API extern SRTSOCKET srt_accept(SRTSOCKET u, struct sockaddr* addr, int* addrlen);
SRT_API extern int srt_connect(SRTSOCKET u, const struct sockaddr* name, int namelen);
SRT_API extern int srt_rendezvous(SRTSOCKET u, const struct sockaddr* local_name, int local_namelen,
        const struct sockaddr* remote_name, int remote_namelen);
SRT_API extern int srt_close(SRTSOCKET u);
SRT_API extern int srt_getpeername(SRTSOCKET u, struct sockaddr* name, int* namelen);
SRT_API extern int srt_getsockname(SRTSOCKET u, struct sockaddr* name, int* namelen);
SRT_API extern int srt_getsockopt(SRTSOCKET u, int level /*ignored*/, SRT_SOCKOPT optname, void* optval, int* optlen);
SRT_API extern int srt_setsockopt(SRTSOCKET u, int level /*ignored*/, SRT_SOCKOPT optname, const void* optval, int optlen);
SRT_API extern int srt_getsockflag(SRTSOCKET u, SRT_SOCKOPT opt, void* optval, int* optlen);
SRT_API extern int srt_setsockflag(SRTSOCKET u, SRT_SOCKOPT opt, const void* optval, int optlen);
SRT_API extern int srt_send(SRTSOCKET u, const char* buf, int len, int flags);
SRT_API extern int srt_recv(SRTSOCKET u, char* buf, int len, int flags);

// The sendmsg/recvmsg and their 2 counterpart require MAXIMUM the size of SRT payload size (1316).
// Any data over that size will be ignored.
SRT_API extern int srt_sendmsg(SRTSOCKET u, const char* buf, int len, int ttl/* = -1*/, int inorder/* = false*/);
SRT_API extern int srt_recvmsg(SRTSOCKET u, char* buf, int len);
SRT_API extern int srt_sendmsg2(SRTSOCKET u, const char* buf, int len, SRT_MSGCTRL *mctrl);
SRT_API extern int srt_recvmsg2(SRTSOCKET u, char *buf, int len, SRT_MSGCTRL *mctrl);

// last error detection
SRT_API extern const char* srt_getlasterror_str(void);
SRT_API extern int srt_getlasterror(int* errno_loc);
SRT_API extern const char* srt_strerror(int code, int errnoval);
SRT_API extern void srt_clearlasterror(void);

// performance track
// srt_perfmon is deprecated - use srt_bstats, which provides the same stats plus more.
SRT_API extern int srt_perfmon(SRTSOCKET u, SRT_TRACEINFO * perf, int clear) SRT_ATR_DEPRECATED;
SRT_API extern int srt_bstats(SRTSOCKET u, SRT_TRACEBSTATS * perf, int clear);

// Socket Status (for problem tracking)
SRT_API extern SRT_SOCKSTATUS srt_getsockstate(SRTSOCKET u);

SRT_API extern int srt_epoll_create(void);
SRT_API extern int srt_epoll_add_usock(int eid, SRTSOCKET u, const int* events);
SRT_API extern int srt_epoll_add_ssock(int eid, SYSSOCKET s, const int* events);
SRT_API extern int srt_epoll_remove_usock(int eid, SRTSOCKET u);
SRT_API extern int srt_epoll_remove_ssock(int eid, SYSSOCKET s);
SRT_API extern int srt_epoll_update_usock(int eid, SRTSOCKET u, const int* events);
SRT_API extern int srt_epoll_update_ssock(int eid, SYSSOCKET s, const int* events);
///SRT_API extern int srt_epoll_wait(int eid, std::set<SRTSOCKET>* readfds, std::set<SRTSOCKET>* writefds, int64_t msTimeOut,
///                       std::set<SYSSOCKET>* lrfds = NULL, std::set<SYSSOCKET>* wrfds = NULL);
SRT_API extern int srt_epoll_wait(int eid, SRTSOCKET* readfds, int* rnum, SRTSOCKET* writefds, int* wnum, int64_t msTimeOut,
                        SYSSOCKET* lrfds, int* lrnum, SYSSOCKET* lwfds, int* lwnum);
SRT_API extern int srt_epoll_release(int eid);

// Logging control

SRT_API void srt_setloglevel(int ll);
SRT_API void srt_addlogfa(int fa);
SRT_API void srt_dellogfa(int fa);
SRT_API void srt_resetlogfa(const int* fara, size_t fara_size);
// This isn't predicted, will be only available in SRT C++ API.
// For the time being, until this API is ready, use UDT::setlogstream.
// SRT_API void srt_setlogstream(std::ostream& stream);
SRT_API void srt_setloghandler(void* opaque, SRT_LOG_HANDLER_FN* handler);
SRT_API void srt_setlogflags(int flags);


SRT_API int srt_getsndbuffer(SRTSOCKET sock, size_t* blocks, size_t* bytes);

#ifdef __cplusplus
}
#endif

#endif
