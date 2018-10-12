#ifndef __XPLSOCKADDR_H__
#define __XPLSOCKADDR_H__

// <BEGIN RANT>
// Sockaddrs are a wart of past mistakes.  They should have been defined
// like this:
// struct sockaddr {
//   sa_family_t family;
//   union {
//     struct {
//       u_int16_t port;
//       struct in_addr addr;
//     } sin;
//     struct {
//       u_int16_t port;
//       u_int32_t flowinfo;
//       struct in6_addr addr;
//     } sin6;
//     struct {
//       char data[0];
//       __ss_aligntype __ss_align;
//      char __ss_padding[_SS_PADSIZE];
//     };
//   }
// }
// <END RANT>
//
// It is too late to change past mistakes, but at least we can do this and
// put a stop to the ridiculous casting all over the place that most programs
// do.

#define MAX_IP_STRING ( ( 8 * 4 ) + 7 ) // in long format it could be up to 8 4-diget hex numbers separated by 7 periods

typedef union {
	struct sockaddr sa;			/* "generic" but not big enough for IPv6 */
	struct sockaddr_in sa4;		/* IPv4 */
	struct sockaddr_in sin;		/* IPv4 */
	struct sockaddr_in6 sa6;	/* IPv6 */
	struct sockaddr_in6 sin6;	/* IPv6 */
	struct sockaddr_storage stg; /* paranoia - a sockaddr_in6 is this big */
	struct sockaddr_storage storage; /* paranoia - a sockaddr_in6 is this big */
} XplSockAddr;

EXPORT sa_family_t XplSockAddrFamily(XplSockAddr *sa);
EXPORT uint16 XplSockAddrGetPort(XplSockAddr *sa);
EXPORT XplBool XplSockAddrSetPort(XplSockAddr *sa, uint16 port);
EXPORT XplBool XplSockAddrString(XplSockAddr *sa, char *buf, size_t buflen);
EXPORT char *XplSockAddrToString(XplSockAddr *sa, char *buf, size_t buflen);
EXPORT XplBool XplProvisionSockAddr(XplSockAddr *sa, char *address);
EXPORT void XplCopySockAddr(XplSockAddr *dst, XplSockAddr *src);
EXPORT XplBool XplEquivalentSockAddr(XplSockAddr *sa1, XplSockAddr *sa2);
EXPORT XplBool XplSockAddrIsLocal(XplSockAddr *sa);
#endif
