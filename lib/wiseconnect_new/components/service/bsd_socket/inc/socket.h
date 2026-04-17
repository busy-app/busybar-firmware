/*	$OpenBSD: socket.h,v 1.105 2022/09/03 21:13:48 mbuhl Exp $	*/
/*	$NetBSD: socket.h,v 1.14 1996/02/09 18:25:36 christos Exp $	*/

/*
 * Copyright (c) 1982, 1985, 1986, 1988, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)socket.h	8.4 (Berkeley) 2/21/94
 */

#ifndef COMPONENTS_BSD_SOCKET_INC_SOCKET_H_
#define	COMPONENTS_BSD_SOCKET_INC_SOCKET_H_

/* get the definitions for struct iovec, size_t, ssize_t, and <sys/cdefs.h> */
#include "uio.h"
#include "netinet_in.h"

#ifdef __cplusplus
extern "C" {	
#endif

#ifndef	_SOCKLEN_T_DEFINED_
#define	_SOCKLEN_T_DEFINED_
typedef	__socklen_t	socklen_t;	/* length type for network syscalls */
#endif

#ifndef	_SA_FAMILY_T_DEFINED_
#define	_SA_FAMILY_T_DEFINED_
typedef	__sa_family_t	sa_family_t;	/* sockaddr address family type */
#endif

/*
 * Definitions related to sockets: types, address families, options.
 */

/*
 * Types
 */
/**
 * @addtogroup BSD_SOCKET_TYPES Socket Type
 * @ingroup BSD_SOCKET_FUNCTIONS
 * @{ 
 */
#define	SOCK_STREAM	1		///< Stream socket. Provides sequenced, reliable, two-way, connection-based byte streams.
#define	SOCK_DGRAM	2		///< Datagram socket. Supports datagrams (connectionless, unreliable messages of a fixed maximum length).
#define	SOCK_RAW	3		///< Raw socket. Provides raw network protocol access. This feature is not supported in the current release.
#define	SOCK_RDM	4		///< Reliably-delivered message. Provides a reliable datagram layer that does not guarantee ordering. This feature is not supported in the current release.
#define	SOCK_SEQPACKET	5   ///< Sequenced packet stream. Provides a sequenced, reliable, two-way, connection-based data transmission path for datagrams of fixed maximum length. This feature is not supported in the current release.
#ifdef _KERNEL
#define	SOCK_TYPE_MASK	0x000F		///< Mask that covers all the above socket types. This feature is not supported in the current release.
#endif
/** @} */

/*
 * Option flags per-socket.
 */
/**
 * @addtogroup BSD_SOCKET_OPTION_NAME Socket Option Name
 * @ingroup BSD_SOCKET_FUNCTIONS
 * @{ 
 */
#define	SO_DEBUG	0x0001		///< Enables recording of debugging information. This option is not supported in the current release.
#define	SO_ACCEPTCONN	0x0002		///< Indicates that the socket has had listen(). This option is not supported in the current release.
#define	SO_REUSEADDR	0x0004		///< Allows local address reuse. This option is not supported in the current release.
#define	SO_KEEPALIVE	0x0008		///< Keeps connections alive.
#define	SO_DONTROUTE	0x0010		///< Uses interface addresses only. This option is not supported in the current release.
#define	SO_BROADCAST	0x0020		///< Permits sending of broadcast messages. This option is not supported in the current release.
#define	SO_USELOOPBACK	0x0040		///< Bypasses hardware when possible. This option is not supported in the current release.
#define	SO_LINGER	0x0080		///< Lingers on close if data is present. This option is not supported in the current release.
#define	SO_OOBINLINE	0x0100		///< Leaves received out-of-band data in line. This option is not supported in the current release.
#define	SO_REUSEPORT	0x0200		///< Allows local address and port reuse. This option is not supported in the current release.
#define SO_TIMESTAMP	0x0800		///< Timestamps received datagram traffic. This option is not supported in the current release.
#define SO_BINDANY	0x1000		///< Allows binding to any address. This option is not supported in the current release.
#define SO_ZEROIZE	0x2000		///< Zeroes out all mbufs sent over the socket. This option is not supported in the current release.
#define SO_MAX_RETRANSMISSION_TIMEOUT_VALUE 0x3012 ///< Configures max retransmission timeout value. The option value associated with this option name should be a power of 2 between 1 and 128.
/*
 * Additional options, not kept in so_options.
 */
#define	SO_SNDBUF	0x1001		///< Sets send buffer size.
#define	SO_RCVBUF	0x1002		///< Sets receive buffer size.
#define	SO_SNDLOWAT	0x1003		///< Sets send low-water mark. This option is not supported in the current release.
#define	SO_RCVLOWAT	0x1004		///< Sets receive low-water mark. This option is not supported in the current release.
#define	SO_SNDTIMEO	0x1005		///< Sets send timeout. This option is not supported in the current release.
#define	SO_RCVTIMEO	0x1006		///< Sets receive timeout. 
#define	SO_ERROR	0x1007		///< Gets error status and clears.
#define	SO_TYPE		0x1008		///< Gets socket type.
#define	SO_NETPROC	0x1020		///< Multiplexes; network processing. This option is not supported in the current release.
#define	SO_RTABLE	0x1021		///< Sets routing table to be used. This option is not supported in the current release.
#define	SO_PEERCRED	0x1022		///< Gets connect-time credentials. This option is not supported in the current release.
#define	SO_SPLICE	0x1023		///< Splices data to another socket. This option is not supported in the current release.
#define	SO_DOMAIN	0x1024		///< Gets socket domain.
#define	SO_PROTOCOL	0x1025		///< Gets socket protocol.

// From Linux include/uapi/linux/tcp.h
// See https://www.kernel.org/doc/Documentation/networking/tls.txt for more details.
#define TCP_ULP	0x001f		///< Attach a ULP (Upper Layer Protocol) to a TCP connection.

/*
 * si91x socket options
 */
#define	SL_SO_CERT_INDEX				0x1026	///< Sets certificate index for SSL socket.
#define	SL_SO_HIGH_PERFORMANCE_SOCKET	0x1027	///< Enables high-performance socket.
#define SL_SO_TLS_SNI                  0x1028  ///< Passes SNI extension for SSL socket.
#define SL_SO_TLS_ALPN                 0x1029  ///< Passes ALPN extension for SSL socket.
#define SL_SO_MSS                      0x102A  ///< Sets the Maximum Segment Size (MSS) for a socket.
#define SL_SO_SOCK_VAP_ID              0x102B  ///< Sets the VAP ID for a socket.
#define SL_SO_MAXRETRY                 0x102C  ///< Sets the maximum number of retries for a socket.
#define SL_SO_VERIFY_DOMAIN_NAME       0x102D  ///< Sets expected domain name for TLS certificate verification.
/** @} */

/*
 * Option values per-socket.
 */
/**
 * @addtogroup BSD_TLS_OPTION_VALUE TLS Option Value
 * @ingroup BSD_SOCKET_FUNCTIONS
 * @{ 
 */
#define TLS		"tls"		///< Option value for default TLS version.
#define TLS_1_0	"tls_1_0"	///< Option value for TLS 1.0.
#define TLS_1_1	"tls_1_1" 	///< Option value for TLS 1.1.
#define TLS_1_2	"tls_1_2"   ///< Option value for TLS 1.2.
#define TLS_1_3 "tls_1_3"	///< Option value for TLS 1.3.
/** @} */

/**
 * @addtogroup BSD_SOCKET_OPTION_LEVEL Socket Option Level
 * @ingroup BSD_SOCKET_FUNCTIONS
 * @{ 
 */
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		///< This is used to denote that the options are applicable at the socket level.

//From Linux include/linux/socket.h
/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SOL_TCP 0x0006  ///< This is used to denote that the options are applicable at the TCP level.
/** @} */

/*
 * Address families.
 */
/**
 * @addtogroup BSD_SOCKET_FAMILIY Socket Address Family
 * @ingroup BSD_SOCKET_FUNCTIONS
 * @{ 
 */
#define	AF_UNSPEC	0		///< Unspecified address family.
#define	AF_UNIX		1		///< Local to host (UNIX domain) address family. This domain is not supported in the current release.
#define	AF_LOCAL	AF_UNIX		///< POSIX compatibility alias for AF_UNIX. This domain is not supported in the current release.
#define	AF_INET		2		///< Internet IP Protocol address family.
#define	AF_IMPLINK	3		///< ARPANET IMP address family. This domain is not supported in the current release.
#define	AF_PUP		4		///< PUP protocols (for example, BSP) address family. This domain is not supported in the current release.
#define	AF_CHAOS	5		///< MIT CHAOS protocols address family. This domain is not supported in the current release.
#define	AF_NS		6		///< XEROX NS protocols address family. This domain is not supported in the current release.
#define	AF_ISO		7		///< ISO protocols address family. This domain is not supported in the current release.
#define	AF_OSI		AF_ISO  ///< OSI protocol family. This domain is not supported in the current release.
#define	AF_ECMA		8		///< European Computer Manufacturers Association (ECMA) protocols address family. This domain is not supported in the current release.
#define	AF_DATAKIT	9		///< Datakit protocols address family. This domain is not supported in the current release.
#define	AF_CCITT	10		///< CCITT protocols (for example, X.25) address family. This domain is not supported in the current release.
#define	AF_SNA		11		///< IBM SNA address family. This domain is not supported in the current release.
#define AF_DECnet	12		///< DECnet address family. This domain is not supported in the current release.
#define AF_DLI		13		///< DEC Direct data link interface address family. This domain is not supported in the current release.
#define AF_LAT		14		///< LAT address family. This domain is not supported in the current release.
#define	AF_HYLINK	15		///< NSC Hyperchannel address family. This domain is not supported in the current release.
#define	AF_APPLETALK	16		///< AppleTalk address family. This domain is not supported in the current release.
#define	AF_ROUTE	17		///< Internal Routing Protocol address family. This domain is not supported in the current release.
#define	AF_LINK		18		///< Link layer interface address family. This domain is not supported in the current release.
#define	pseudo_AF_XTP	19		///< eXpress Transfer Protocol (no AF). This domain is not supported in the current release.
#define	AF_COIP		20		///< Connection-oriented IP, aka ST II address family. This domain is not supported in the current release.
#define	AF_CNT		21		///< Computer Network Technology address family. This domain is not supported in the current release.
#define pseudo_AF_RTIP	22		///< Identifier for RTIP packets. This domain is not supported in the current release.
#define	AF_IPX		23		///< Novell Internet Protocol address family. This domain is not supported in the current release.
#define	AF_INET6	24		///< IPv6 address family.
#define pseudo_AF_PIP	25		///< Identifier for PIP packets. This domain is not supported in the current release.
#define AF_ISDN		26		///< Integrated Services Digital Network address family. This domain is not supported in the current release.
#define AF_E164		AF_ISDN		///< CCITT E.164 recommendation address family. This domain is not supported in the current release.
#define AF_NATM		27		///< Native ATM access address family. This domain is not supported in the current release.
#define	AF_ENCAP	28      ///< Encapsulation address family. This domain is not supported in the current release.
#define	AF_SIP		29		///< Simple Internet Protocol address family. This domain is not supported in the current release.
#define AF_KEY		30      ///< Key management protocol address family (originally developed for usage with IPsec).
#define pseudo_AF_HDRCMPLT 31		///< Used by BPF to not rewrite headers in interface output routine. This domain is not supported in the current release.
#define	AF_BLUETOOTH	32		///< Bluetooth address family. This domain is not supported in the current release.
#define AF_MPLS         33              ///< MPLS address family. This domain is not supported in the current release.
#define pseudo_AF_PFLOW 34		///< Identifier for pflow packets. This domain is not supported in the current release.
#define pseudo_AF_PIPEX 35		///< Identifier for PIPEX packets. This domain is not supported in the current release.
#define AF_MAX          36      ///< Maximum address family value. This domain is not supported in the current release.
/** @} */

/*
 * Structure used by kernel to store most
 * addresses.
 */
/**
 * @addtogroup BSD_SOCKET_FUNCTIONS
 * @{ 
 */
/**
 * @struct sockaddr
 * @brief 
 *     The structure stores generic socket address.
 * 
 * @details
 *     The @ref sockaddr structure represents addresses in a protocol-independent way. It includes the address family, and the address data. The actual format of the address data is determined by the address family specified in `sa_family`.
 * 
 */
struct sockaddr {
	uint8_t    sa_len;		///< Total length of the structure.
	sa_family_t sa_family;		///< Address family (for example, AF_INET).
	char	    sa_data[14];	///< Socket address data. The actual format is determined by the address family.
};
/** @} */

/**
 * Sockaddr type which can hold any sockaddr type available
 * in the system.
 *
 * Note: __ss_{len,family} is defined in RFC2553.  During RFC2553 discussion
 * the field name went back and forth between ss_len and __ss_len,
 * and RFC2553 specifies it to be __ss_len.  openbsd picked ss_len.
 * For maximum portability, userland programmer would need to
 * (1) make the code never touch ss_len portion (cast it into sockaddr and
 * touch sa_len), or (2) add "-Dss_len=__ss_len" into CFLAGS to unify all
 * occurrences (including header file) to __ss_len.
 */
struct sockaddr_storage {
	uint8_t	ss_len;		///< Total length. 
	sa_family_t	ss_family;	///< Address family.
	unsigned char	__ss_pad1[6];	///< Align to quad. 
	uint64_t	__ss_pad2;	///< Force alignment for stupid compilers. 
	unsigned char	__ss_pad3[240];	///< Pad to a total of 256 bytes. 
};

/* __BEGIN_DECLS */

/**
 * @addtogroup BSD_SOCKET_FUNCTIONS
 * @{ 
 */

/***************************************************************************/ 
/**
 * @brief
 *   Create an endpoint for communication.
 * 
 * @details
 *   The `socket()` function creates an endpoint for communication and returns a file descriptor that refers to that endpoint. 
 *   The file descriptor returned by a successful call would be the lowest-numbered file descriptor not currently open for the process.
 * 
 * @param[in] domain
 *   Specifies a communication domain, selecting the protocol family to be used for communication. 
 *   Must be one of the values from @ref BSD_SOCKET_FAMILIY.
 * 
 * @param[in] type
 *   Specifies the semantics of communication for the socket. 
 *   Must be one of the values from BSD_SOCKET_TYPES. 
 *   Currently defined types are SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, SOCK_RDM, and SOCK_SEQPACKET.
 * 
 * @param[in] protocol
 *   Specifies a particular protocol to be used with the socket. Normally, only a single protocol exists to support a particular socket type 
 *   within a given protocol family. However, if multiple protocols exist, a specific protocol must be specified. 
 *   The protocol number to use is specific to the communication domain. Must be one of the values from @ref BSD_SOCKET_PROTOCOL.
 * 
 * @return 
 *   Returns the socket ID or file descriptor that references the socket upon success.
 *   Returns -1 if an error occurs.
 * 
 * @note
 * 	The \p type supports SOCK_STREAM and SOCK_DGRAM only.
 ******************************************************************************/
int socket(int domain, int type, int protocol);

/***************************************************************************/ 
/**
 * @brief
 *   Bind a name to a socket.
 * 
 * @details
 *   When a socket is created with `socket()`, it exists in a namespace (address family) but, has no address assigned to it. 
 *   The `bind()` function assigns the address specified by `addr` to the socket referred to by the file descriptor `socket_id`.  
 *   Traditionally, this operation is called "assigning a name to a socket".
 * 
 *   It is normally necessary to assign a local address using `bind()` before a `SOCK_STREAM` socket may receive connections.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[in] addr 
 *   Pointer to a @ref sockaddr structure containing the address to be assigned to the socket.
 * 
 * @param[in] addr_len 
 *   The `addr_len` parameter specifies the size, in bytes, of the address structure pointed to by `addr`.
 * 
 * @return
 *   Returns 0 if successful. Otherwise, returns -1 and sets the global variable `errno` to indicate the error.
 ******************************************************************************/
int bind(int socket_id, const struct sockaddr *addr, socklen_t addr_len);

/***************************************************************************/ 
/**
 * @brief
 *   Listen for connections on a socket.
 * 
 * @details
 *   The `listen()` function marks the socket referred to by `socket_id` as a passive socket, 
 *   that is, as a socket that would be used to accept incoming connection requests using `accept()`.
 *   This function applies only to sockets of type `SOCK_STREAM` or `SOCK_SEQPACKET`.
 * 
 * @pre
 *   To accept connections, a socket must first be created with @ref socket(). 
 *   A willingness to accept incoming connections and a queue limit for incoming connections 
 *   must be specified with @ref listen(), and then the connections can be accepted with @ref accept().
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[in] backlog
 *   The backlog argument defines the maximum number of pending connections that can be queued.
 * 
 * @return
 *   Returns 0 if successful. Otherwise, returns -1 and sets the global variable `errno` to indicate the error.
 ******************************************************************************/
int listen(int socket_id, int backlog);

/***************************************************************************/ 
/**
 * @brief
 *   Accept a connection on a socket.
 * 
 * @details
 *   The `accept()` system call is used with connection-based socket types (for example, `SOCK_STREAM`, `SOCK_SEQPACKET`). 
 *   It extracts the first connection request on the queue of pending connections for the listening socket (`socket_id`), 
 *   creates a new connected socket, and returns a new file descriptor referring to that socket. 
 *   The newly created socket is not in the listening state. The original socket (`socket_id`) is unaffected by this call.
 * 
 *   If no pending connections are present on the queue and the original socket is not marked as non-blocking, 
 *   this function blocks the caller until a connection is present. If the original socket is marked non-blocking and 
 *   no pending connections are present on the queue, @ref accept() returns an error. The accepted socket may not be 
 *   used to accept more connections. The original socket (`socket_id`) remains open.
 * 
 * @param[in] socket_id
 *   Socket identification number of the socket to accept. This socket must have been created with @ref socket(), 
 *   bound to an address with @ref bind(), and set to listen for connections with @ref listen().
 * 
 * @param[out] addr 
 *   Pointer to a `sockaddr` structure that will be filled with the address of the connecting entity, as known
 *   to the communications layer. The exact format of the `addr` argument is determined by the domain in which the 
 *   communication is occurring. A null pointer may be specified for `addr` if the address information is not desired; 
 *   in this case, `addr_len` is not used and should also be null.
 * 
 * @param[in, out] addr_len 
 *   Pointer to a `socklen_t` variable. Initially, it should contain the amount of space pointed to by `addr`. 
 *   On return, it will contain the actual length (in bytes) of the address returned. This call is used with 
 *   connection-based socket types, currently with `SOCK_STREAM`.
 * 
 * @return
 *   Returns a non-negative integer that is a descriptor for the accepted socket if successful. 
 *   Returns -1 on error and sets the global variable `errno` to indicate the error.
 * 
 * @note 
 *   The @ref accept() system call only supports blocking mode.
 ******************************************************************************/
int accept(int socket_id, struct sockaddr *addr, socklen_t *addr_len);

/***************************************************************************/ 
/**
 * @brief
 *   Initiate a connection on a socket.
 * 
 * @details
 *   The @ref connect() system call connects the socket referred to by the file descriptor `socket_id` to the address specified by `addr`.  
 *   The `addr_len` argument specifies the size of `addr`. The format of the address in `addr` is determined by 
 *   the address space of the socket `socket_id`; see @ref socket() for further details.
 * 
 *   If `socket_id` is of type `SOCK_DGRAM`, the @ref connect() system call specifies the peer with which the socket is to be associated.
 *   If the socket is of type `SOCK_STREAM`, the @ref connect() system call attempts to make a connection to another socket. The other 
 *   socket is specified by `addr`, which is an address in the communications space of the socket. Each communications space interprets
 *   the `addr` argument in its own way. Generally, stream sockets may successfully @ref connect() only once; datagram sockets can use 
 *   @ref connect() multiple times to change their association.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[in] addr 
 *   The addr argument of type @ref sockaddr is the address is that to which datagrams are to be sent.
 * 
 * @param[in] addr_len 
 *   The addr_len argument of type @ref socklen_t indicates the amount of space pointed to by address, in bytes.
 * 
 * @return
 *   Returns 0 if successful. Otherwise, returns -1 and sets the global variable `errno` to indicate the error.
 * 
 * @note
 *   Connecting to an invalid address, such as a null address, will result in an `EFAULT` error.
 ******************************************************************************/
int connect(int socket_id, const struct sockaddr *addr, socklen_t addr_len);

/***************************************************************************/ 
/**
 * @brief
 *   Receive a message from a socket.
 *  
 * @details
 *   The @ref recv() function is normally used only on a connected socket. It receives messages from a socket and returns the length of the message on successful completion. 
 *   If a message is too long to fit in the supplied buffer, excess bytes may be discarded depending on the type of socket the message is received from.
 *   By default, @ref recv() is a blocking API. To use it in a non-blocking manner, you can set the socket to non-blocking mode using `setsockopt()`.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[out] buf
 *   Pointer to the buffer that receives the data.
 * 
 * @param[in] buf_len
 *   Length of the buffer pointed to by the `buf` parameter.
 * 
 * @param[in] flags
 *   Controls the reception of the data. Currently, no flags are supported and this parameter should be set to 0.
 * 
 * @return
 *   ssize_t
 * 
 * @note 
 *   The @ref recv() system call does not support any flags.
 ******************************************************************************/
ssize_t recv(int socket_id, void *buf, size_t buf_len, int flags);

/***************************************************************************/ 
/**
 * @brief
 *   Receive a message from a socket.
 * 
 * @details
 *   The @ref recvfrom() system call is used to receive messages from a socket and can be used to receive data on a socket 
 *   whether or not it is connection-oriented. If `from_addr` is not a null pointer and the socket is not connection-oriented, 
 *   the source address of the message is filled in.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[out] buf
 *   Pointer to the buffer that receives the data.
 * 
 * @param[in] buf_len
 *   The length of the buffer pointed to by the `buf` parameter, in bytes.
 * 
 * @param[in] flags
 *   Controls the reception of the data. Currently, no flags are supported and this parameter should be set to 0.
 * 
 * @param[out] from_addr 
 *   Pointer to a socket address structure of type @ref sockaddr that will be filled with the source address of the received message. 
 *   If the source address is not required, this parameter can be NULL.
 * 
 * @param[in, out] from_addr_len 
 *   A value-result argument of type @ref socklen_t. Initially, it should contain the size of the buffer associated with `from_addr`. 
 *   On return, it will be modified to indicate the actual size of the address stored there. If `from_addr` is NULL, this parameter is ignored.
 * 
 * @return
 *   Returns the length of the message on successful completion. Returns -1 on error and sets the global variable `errno` to indicate the error.
 * 
 * @note 
 *   The @ref recvfrom() system call does not support any flags.
 ******************************************************************************/
ssize_t recvfrom(int socket_id, void *buf, size_t buf_len, int flags, struct sockaddr *from_addr, socklen_t *from_addr_len);

/***************************************************************************/ 
/**
 * @brief
 *   Send a message on a socket.
 * 
 * @details
 *   The @ref send() function is used to transmit a message on a socket. This function can only be used when the socket is in a connected state. 
 *   If the socket is connection-oriented, the protocol must support implied connect or the socket must be explicitly connected before use. 
 *   No indication of failure to deliver is implicit in a @ref send(). Locally detected errors are indicated by a return value of -1. 
 *   If no message space is available at the socket to hold the message to be transmitted, then @ref send() normally blocks, unless the socket 
 *   has been placed in non-blocking I/O mode.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[in] buf
 *   Pointer to the buffer containing the message to transmit.
 * 
 * @param[in] buf_len
 *   The length of the message in bytes.
 * 
 * @param[in] flags
 *   Controls the transmission of the data. Currently, no flags are supported and this parameter should be set to 0.
 * 
 * @return
 *   Returns the number of bytes sent on success. Returns -1 on error and sets the global variable `errno` to indicate the error.
 * 
 * @note 
 *   - The @ref send() system call currently supports only blocking mode.
 *   - The @ref send() system call does not guarantee that the packets are transmitted to the remote node; they are enqueued in the local queue.
 *   - The @ref send() system call does not support any flags.
 *   - The @ref send() system call can send a maximum of 1460 bytes in the case of plain TCP/UDP. For TLS, the maximum buffer length is 1370 bytes.
 ******************************************************************************/
ssize_t send(int socket_id, const void *buf, size_t buf_len, int flags);

/***************************************************************************/ 
/**
 * @brief
 *   Send a message on a socket.
 * 
 * @details
 *   The @ref sendto() function is used to transmit a message on a socket. This function can be used at any time if the socket is in connectionless mode. 
 *   If the socket is in connection mode, the protocol must support implied connect or the socket must be in a connected state before use. 
 *   The address of the target is specified by `to_addr` with `to_addr_len` indicating its size. If the socket is in a connected state, 
 *   the target address passed to @ref sendto() is ignored. If the message is too long to pass atomically through the protocol, 
 *   the error `EMSGSIZE` is returned, and the message is not transmitted.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[in] buf
 *   Pointer to the buffer containing the message to transmit.
 * 
 * @param[in] buf_len
 *   The length of the message in bytes.
 * 
 * @param[in] flags
 *   Controls the transmission of the data. Due to firmware limitations, @ref sendto() does not support any flags and this parameter should be set to 0.
 * 
 * @param[in] to_addr 
 *   Pointer to a `sockaddr` structure containing the address of the target.
 * 
 * @param[in] to_addr_len 
 *   Size of the address structure pointed to by `to_addr`, in bytes.
 * 
 * @return
 *   The number of octets sent. If an error occurred, a value of -1 is returned.
 * 
 * @note 
 *   - The @ref sendto() system call can only send a maximum of 1460 bytes in the case of plain TCP/UDP. For TLS, the maximum buffer length is 1370 bytes.
 *   - The @ref sendto() system call does not support any flags.
 ******************************************************************************/
ssize_t sendto(int socket_id, const void *buf, size_t buf_len, int flags, const struct sockaddr *to_addr, socklen_t to_addr_len);

/***************************************************************************/ 
/**
 * @brief
 *   Set options on a socket.
 * 
 * @details
 *   The @ref setsockopt() system call manipulates the options associated with a socket. Options may exist at multiple protocol levels; 
 *   they are always present at the uppermost "socket" level. When manipulating socket options, the level at which the option resides 
 *   and the name of the option must be specified. To manipulate options at the socket level, `option_level` is specified as `SOL_SOCKET`. 
 *   To manipulate options at any other level, the protocol number of the appropriate protocol controlling the option is supplied. 
 *   For example, to indicate that an option is to be interpreted by the TCP protocol, `option_level` should be set to the protocol number of TCP. 
 *   The `option_value` and `option_length` arguments are used to access option values for @ref setsockopt().
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[in] option_level
 *   The level at which the option is being set. One of the values from @ref BSD_SOCKET_OPTION_LEVEL.
 * 
 * @param[in] option_name
 *   The option to be configured. Accepts values from @ref BSD_SOCKET_OPTION_NAME. 
 *   Currently, following options are supported:
 *   - @ref SO_RCVTIMEO
 *   - @ref SO_KEEPALIVE
 *   - @ref SO_MAX_RETRANSMISSION_TIMEOUT_VALUE
 *   - @ref TCP_ULP
 *   - IP_TOS
 *   - @ref SL_SO_CERT_INDEX
 *   - @ref SL_SO_HIGH_PERFORMANCE_SOCKET
 *   - @ref SL_SO_TLS_SNI
 *   - @ref SL_SO_TLS_ALPN
 *   - @ref SL_SO_VERIFY_DOMAIN_NAME
 *  
 * @param[in] option_value
 *   A pointer to the buffer containing the value for the option. Most socket-level options utilize an `int` argument for `option_value`. 
 *   For boolean options, set option_value to a non-zero value to enable the option or to zero to disable it. For non-boolean options, option_value should contain the specific data required for the option.
 *   The value of the parameter.
 *   | option_name                                       | option_value                         |  description                                                                                                               |
 *   |---------------------------------------------------|--------------------------------------|----------------------------------------------------------------------------------------------------------------------------|
 *   | @ref SO_RCVTIMEO                                  | sl_si91x_time_value                  | Socket Receive timeout. sl_si91x_time_value structure is used to represent time in two parts: seconds and microseconds.    |
 *   | @ref SO_KEEPALIVE                                 | uint16_t                             | Set TCP keepalive in seconds                                                                                               |
 *   | @ref SO_MAX_RETRANSMISSION_TIMEOUT_VALUE          | uint8_t                              | Maximum retransmission timeout value that should be in the power of 2 for TCP in seconds                                   |
 *   | @ref TCP_ULP                                      | uint8_t                              | Supported one of the values from @ref BSD_TLS_OPTION_VALUE                                                       |
 *   | IP_TOS                                            | uint16_t                             | Supported one of the values from @ref BSD_SOCKET_TOS_DEFINES (Values 0-7 are deprecated)                                              |
 *   | @ref SL_SO_CERT_INDEX                             | uint8_t                              | Supported values for certificate index range from 0 - 3                                                                    |
 *   | @ref SL_SO_HIGH_PERFORMANCE_SOCKET                | BIT(7)                               | Set high performance socket                                                                                                |
 *   | @ref SL_SO_TLS_SNI                                | sl_si91x_socket_type_length_value_t  | Server name indication for the socket                                                                                      |
 *   | @ref SL_SO_TLS_ALPN                               | sl_si91x_socket_type_length_value_t  | Application layer protocol negotiation for the socket                                                                      |
 *   | @ref SL_SO_VERIFY_DOMAIN_NAME                     | uint8_t *                            | Expected domain name for TLS certificate verification; firmware uses this for server certificate CN/SAN check.             |
 * 
 * @param[in] option_length
 *   The length of the option data, in bytes, pointed to by `option_value`.
 * 
 * @return
 *   Returns 0 on successful completion. Returns -1 on error and sets the global variable `errno` to indicate the error.
 * 
 * @note 
 *   The options `SL_SO_CERT_INDEX`, `SL_SO_HIGH_PERFORMANCE_SOCKET`, `SL_SO_TLS_SNI`, `SL_SO_TLS_ALPN`, and `SL_SO_VERIFY_DOMAIN_NAME` are Silicon Labs specific options.
 * 	 This function is used before the socket is connected.
 ******************************************************************************/
int setsockopt(int socket_id, int option_level, int option_name, const void *option_value, socklen_t option_length);

/***************************************************************************/ 
/**
 * @brief
 *   Get options on a socket.
 * 
 * @details
 *   The @ref getsockopt() system call retrieves the options associated with a socket. Options may exist at multiple protocol levels; 
 *   they are always present at the uppermost "socket" level. When retrieving socket options, the level at which the option resides 
 *   and the name of the option must be specified. To retrieve options at the socket level, `option_level` is specified as `SOL_SOCKET`. 
 *   To retrieve options at any other level, the protocol number of the appropriate protocol controlling the option is supplied. 
 *   For example, to indicate that an option is to be interpreted by the TCP protocol, `option_level` should be set to the protocol number of TCP. 
 *   The `option_value` and `option_length` arguments identify a buffer in which the value for the requested options are to be returned.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[in] option_level
 *   The level at which the option is defined. One of the values from @ref BSD_SOCKET_OPTION_LEVEL.
 * 
 * @param[in] option_name
 *   The name of the option to retrieve. One of the values from @ref BSD_SOCKET_OPTION_NAME. The `option_name` argument and any specified options 
 *   are passed uninterpreted to the appropriate protocol module for interpretation.
 * 
 * @param[out] option_value
 *   Pointer to a buffer where the retrieved option value will be stored.
 * 
 * @param[in, out] option_length
 *   A value-result argument of type @ref socklen_t. Initially, it should contain the size of the buffer pointed to by `option_value`. 
 *   On return, it will be modified to indicate the actual size of the value returned. If no option value is to be supplied or returned, 
 *   `option_value` may be NULL.
 * 
 * @return
 *   Returns 0 on successful completion. Returns -1 on error and sets the global variable `errno` to indicate the error.
 * 
 * @note 
 *   The following options are currently supported:
 *   - `SO_RCVTIMEO`: Receive timeout.
 *   - `SO_KEEPALIVE`: Keep connections alive.
 *   - `TCP_ULP`: TCP upper layer protocol.
 *   - `SO_MAX_RETRANSMISSION_TIMEOUT_VALUE`: Maximum retransmission timeout value.
 *   - `IP_TOS`: Type of service.
 ******************************************************************************/
int getsockopt(int socket_id, int option_level, int option_name, void *option_value, socklen_t *option_length);

/***************************************************************************/ 
/**
 * @brief
 *   Get the name of the connected peer socket.
 * 
 * @details
 *   The @ref getpeername() system call returns the address of the peer connected to the socket specified by `socket_id`. 
 *   The address is returned in the buffer pointed to by `name`, and the actual size of the address is returned in the variable 
 *   pointed to by `name_len`. If the buffer provided is too small, the address is truncated.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[out] name 
 *   Pointer to a `sockaddr` structure that will be filled with the address of the peer socket.
 * 
 * @param[in, out] name_len
 *   A value-result argument of type @ref socklen_t. Initially, it should contain the size of the buffer pointed to by `name`. 
 *   On return, it will be modified to indicate the actual size of the address returned (in bytes).
 * 
 * @return
 *   Returns 0 on success. Returns -1 on error and sets the global variable `errno` to indicate the error.
 ******************************************************************************/
int getpeername(int socket_id, struct sockaddr *name, socklen_t *name_len);

/***************************************************************************/ 
/**
 * @brief
 *   Get the current address assigned to a socket.
 * 
 * @details
 *   The @ref getsockname() system call returns the current address to which the socket `socket_id` is bound. 
 *   The address is returned in the buffer pointed to by `name`, and the actual size of the address is returned in the variable 
 *   pointed to by `name_len`. If the buffer provided is too small, the address is truncated.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @param[out] name 
 *   Pointer to a `sockaddr` structure that will be filled with the address of the socket.
 * 
 * @param[in, out] name_len
 *   A value-result argument of type @ref socklen_t. Initially, it should contain the size of the buffer pointed to by `name`. 
 *   On return, it will be modified to indicate the actual size of the address returned (in bytes).
 * 
 * @return
 *   Returns 0 on success. Returns -1 on error and sets the global variable `errno` to indicate the error.
 ******************************************************************************/
int getsockname(int socket_id, struct sockaddr *name, socklen_t *name_len);

/***************************************************************************/ 
/**
 * @brief 
 *   Close a socket.
 * 
 * @details
 *   The @ref close() function closes a socket identified by `socket_id`, so that it no longer refers to any file and can be reused. 
 *   This function releases the resources associated with the socket and makes the socket descriptor available for reuse.
 * 
 * @param[in] socket_id
 *   The socket ID or file descriptor for the specified socket.
 * 
 * @return
 *   Returns 0 on success. Returns -1 on error and sets the global variable `errno` to indicate the error.
 * 
 * @note
 *   - Calling @ref close() on a server socket or the first client socket will close all associated sockets.
 *   - If @ref close() is called on a non-existing socket, the error `EBADF` (Bad file descriptor) will be set in `errno`.
 ******************************************************************************/
int close(int socket_id);

/** @} */
/* __END_DECLS */

#ifdef __cplusplus
}	
#endif

#endif /* !COMPONENTS_BSD_SOCKET_INC_SOCKET_H_ */
