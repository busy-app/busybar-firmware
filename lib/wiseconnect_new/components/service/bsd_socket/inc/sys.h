/***************************************************************************/ /**
 * @file sys.h
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

#ifndef _SYS_H
#define _SYS_H

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
#ifndef size_t
typedef unsigned int size_t;
#endif
#endif /* _SIZE_T_DEFINED */

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED

/* Check if ssize_t is already defined */
#if !defined(__ssize_t_defined) && !defined(_SSIZE_T_DECLARED)
#define __ssize_t_defined
#define _SSIZE_T_DECLARED
typedef int ssize_t;
#endif /* _SSIZE_T_DEFINED */
#endif

#ifndef _SYS__SIGSET_H_
#define	_SYS__SIGSET_H_
#ifndef __sigset_t_defined
#define __sigset_t_defined
typedef unsigned long __sigset_t;
#endif /* __sigset_t_defined */
#endif /* !_SYS__SIGSET_H_ */

#ifndef _SUSECONDS_T_DECLARED
#define	_SUSECONDS_T_DECLARED
typedef	long suseconds_t;
#endif


#if !defined(_TIME_T_DEFINED) && !defined(_TIME_T_DECLARED)
typedef	long int time_t;
#define	_TIME_T_DEFINED
#define	_TIME_T_DECLARED
#endif

/* This define is also used outside of Newlib, e.g. in MinGW-w64 */
#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
#if !defined(__USE_POSIX) && !defined(_STRUCT_TIMEVAL)
#define _STRUCT_TIMEVAL
/* Check if struct timeval is already defined */
#ifndef __struct_timeval_defined
#define __struct_timeval_defined
/*
 * Structure returned by gettimeofday(2) system call, and used in other calls.
 */
struct timeval {
	time_t		tv_sec;		/* seconds */
	suseconds_t	tv_usec;	/* and microseconds */
};
#endif /* _TIMEVAL_DEFINED */
#endif /* __struct_timeval_defined */
#endif 

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
#if !defined(__USE_POSIX) && !defined(_STRUCT_TIMESPEC)
#define _STRUCT_TIMEVAL
/* Check if struct timespec is already defined */
#ifndef __struct_timespec_defined 
#define __struct_timespec_defined 
/** Represents a time. */
struct timespec {
  /** Number of seconds. */
  time_t tv_sec;
  /** Number of nanoseconds. Must be less than 1,000,000,000. */
  long tv_nsec;
};
#endif /* __struct_timespec_defined */
#endif
#endif 

#endif //_SYS_H