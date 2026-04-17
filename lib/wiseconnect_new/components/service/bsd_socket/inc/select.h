/***************************************************************************/ /**
 * @file select.h
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

/* We don't define fd_set and friends if we are compiling POSIX
   source, or if we have included (or may include as indicated
   by __USE_W32_SOCKETS) the W32api winsock[2].h header which
   defines Windows versions of them.   Note that a program which
   includes the W32api winsock[2].h header must know what it is doing;
   it must not call the Cygwin select function.
*/
# if !(defined (_WINSOCK_H) || defined (_WINSOCKAPI_) || defined (__USE_W32_SOCKETS))

#include "sys.h"

//typedef unsigned long long int __size_t ;
#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
typedef struct timeval_t {
	long		tv_sec;		/* seconds */
	long long	tv_usec;	/* and microseconds */
}timeval;
#endif

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
typedef struct timespec_t {
	long long	tv_sec;		/* seconds */
	long	tv_nsec;	/* and nanoseconds */
}timespec;
#endif


#if !defined(_SIGSET_T_DECLARED)
#define	_SIGSET_T_DECLARED
typedef	__sigset_t	sigset_t;
#endif

#  define _SYS_TYPES_FD_SET
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	32 //64 to 32 changed
#endif

typedef unsigned int	__fd_mask;  //long to int changed
#if __BSD_VISIBLE
typedef __fd_mask	fd_mask;
#endif

#define _NFDBITS	((int)sizeof(__fd_mask) * 8) /* bits per mask */
#if __BSD_VISIBLE
#define NFDBITS		_NFDBITS
#endif

#ifndef	_howmany
#define	_howmany(x,y)	(((x) + ((y) - 1)) / (y))
#endif

typedef	struct fd_set {
	__fd_mask	__fds_bits[_howmany(FD_SETSIZE, _NFDBITS)];
} fd_set;
#if __BSD_VISIBLE
#define fds_bits	__fds_bits
#endif

#define __fdset_mask(n)	((__fd_mask)1 << ((n) % _NFDBITS))
#define FD_CLR(n, p)	((p)->__fds_bits[(n)/_NFDBITS] &= ~__fdset_mask(n))
#if __BSD_VISIBLE
#define FD_COPY(f, t)	(void)(*(t) = *(f))
#endif
#define FD_ISSET(n, p)	(((p)->__fds_bits[(n)/_NFDBITS] & __fdset_mask(n)) != 0)
#define FD_SET(n, p)	((p)->__fds_bits[(n)/_NFDBITS] |= __fdset_mask(n))
#define FD_ZERO(p) do {				\
        fd_set *_p;					\
        size_t _n;					\
							\
        _p = (p);					\
        _n = _howmany(FD_SETSIZE, _NFDBITS);		\
        while (_n > 0)					\
                _p->__fds_bits[--_n] = 0;		\
} while (0)

#if !defined (__INSIDE_CYGWIN_NET__)

/**
 * @brief
 * The select function is used to monitor multiple file descriptors for readiness to
 * perform I/O operations.  The file descriptors in the sets are monitored to
 * see if they are ready for reading, ready for writing, or have an error
 * condition pending.  The timeout parameter specifies the maximum time to
 * wait for the file descriptors to become ready.  If the timeout parameter
 * is NULL, select will block until a file descriptor becomes ready.  If the
 * timeout parameter is not NULL, select will return after the specified time
 * has elapsed, even if no file descriptors are ready.
 * @param __n
 *   The highest file descriptor number in any of the sets plus one.
 * @param __readfds
 *      The set of file descriptors to check for being ready to read.
 * @param __writefds
 *     The set of file descriptors to check for being ready to write.
 * @param __exceptfds
 *    The set of file descriptors to check for error conditions pending.
 * @param __timeout
 *   The maximum time to wait for the file descriptors to become ready.
 * @return
 *   On succcess returns total number of file descriptors contained in the three returned descriptor sets, which may be zero if the timeout expires.
 *   On error, -1 is returned, and errno is set appropriately.
 * @note
 * The select function modifies the sets passed to it, so if the function
 * is to be called again, the sets must be reinitialized.
 * The exceptfds parameter is not currently supported.
 * @note 
 * If the number of select requests is not configured, the select() API will fail and return -1, with the errno being set to EPERM (Operation not permitted).
 * @note 
 * The number of select operations the device can handle can be configured using the [SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS](../wiseconnect-api-reference-guide-si91x-driver/si91-x-extended-tcp-ip-feature-bitmap#sl-si91-x-ext-tcp-ip-total-selects).
 */
#ifndef __ZEPHYR__
int select(int __n, fd_set *__readfds, fd_set *__writefds,
		 fd_set *__exceptfds, const struct timeval *__timeout);
#else
int select(int __n, sl_si91x_fdset_t *__readfds, sl_si91x_fdset_t *__writefds,
		 sl_si91x_fdset_t *__exceptfds, const struct timeval *__timeout);
#endif                 


#endif /* !__INSIDE_CYGWIN_NET__ */

#endif /* !(_WINSOCK_H || _WINSOCKAPI_ || __USE_W32_SOCKETS) */

#endif /* sys/select.h */
