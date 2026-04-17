/***************************************************************************//**
 * @file errno_error_codes.h
 * @brief errno error codes header file
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

#ifndef ERRNO_ERROR_CODES_H
#define ERRNO_ERROR_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

// If not using the errno component, include the next errno.h file.
#if !defined(SL_CATALOG_ERRNO_PRESENT)
#if defined(__ICCARM__)  // IAR compiler
// Force system errno for IAR
    #pragma system_include
    #include <errno.h>
#elif defined(__GNUC__)  // GCC
    #include_next <errno.h>
#else
    #pragma error "Unsupported compiler"
#endif
#endif

/*******************************************************************************
 * @addtogroup errno_error_codes Errno Error Codes
 * @details Errno Error Codes contains the defines for the errno error codes.
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Errno Define Values
#ifndef EPERM
#define EPERM           1   ///< Not owner
#endif
#ifndef ENOENT
#define ENOENT          2   ///< No such file or directory
#endif
#ifndef ESRCH
#define ESRCH           3   ///< No such process
#endif
#ifndef EINTR
#define EINTR           4   ///< Interrupted system call
#endif
#ifndef EIO
#define EIO             5   ///< I/O error
#endif
#ifndef ENXIO
#define ENXIO           6   ///< No such device or address
#endif
#ifndef E2BIG
#define E2BIG           7   ///< Arguments list too long
#endif
#ifndef ENOEXEC
#define ENOEXEC         8   ///< Exec format error
#endif
#ifndef EBADF
#define EBADF           9   ///< Bad file number
#endif
#ifndef ECHILD
#define ECHILD          10  ///< No children
#endif
#ifndef EAGAIN
#define EAGAIN          11  ///< No more processes
#endif
#ifndef ENOMEM
#define ENOMEM          12  ///< Not enough space
#endif
#ifndef EACCES
#define EACCES          13  ///< Permission denied
#endif
#ifndef EFAULT
#define EFAULT          14  ///< Bad address
#endif
#ifndef ENOTBLK
#define ENOTBLK         15  ///< Block device required
#endif
#ifndef EBUSY
#define EBUSY           16  ///< Device or resource busy
#endif
#ifndef EEXIST
#define EEXIST          17  ///< File exists
#endif
#ifndef EXDEV
#define EXDEV           18  ///< Cross-device link
#endif
#ifndef ENODEV
#define ENODEV          19  ///< No such device
#endif
#ifndef ENOTDIR
#define ENOTDIR         20  ///< Not a directory
#endif
#ifndef EISDIR
#define EISDIR          21  ///< Is a directory
#endif
#ifndef EINVAL
#define EINVAL          22  ///< Invalid argument
#endif
#ifndef ENFILE
#define ENFILE          23  ///< Too many open files in system
#endif
#ifndef EMFILE
#define EMFILE          24  ///< File descriptor value too large
#endif
#ifndef ENOTTY
#define ENOTTY          25  ///< Not a character device
#endif
#ifndef ETXTBSY
#define ETXTBSY         26  ///< Text file busy
#endif
#ifndef EFBIG
#define EFBIG           27  ///< File too large
#endif
#ifndef ENOSPC
#define ENOSPC          28  ///< No space left on device
#endif
#ifndef ESPIPE
#define ESPIPE          29  ///< Illegal seek
#endif
#ifndef EROFS
#define EROFS           30  ///< Read-only file system
#endif
#ifndef EMLINK
#define EMLINK          31  ///< Too many links
#endif
#ifndef EPIPE
#define EPIPE           32  ///< Broken pipe
#endif
#ifndef EDOM
#define EDOM            33  ///< Mathematics argument out of domain of function
#endif
#ifndef ERANGE
#define ERANGE          34  ///< Result too large
#endif
#ifndef ENOMSG
#define ENOMSG          35  ///< No message of desired type
#endif
#ifndef EIDRM
#define EIDRM           36  ///< Identifier removed
#endif
#ifndef EDEADLK
#define EDEADLK         45  ///< Deadlock
#endif
#ifndef ENOLCK
#define ENOLCK          46  ///< No lock
#endif

#ifndef ENOSTR
#define ENOSTR          60  ///< Not a stream
#endif
#ifndef ENODATA
#define ENODATA         61  ///< No data (for no delay io)
#endif
#ifndef ETIME
#define ETIME           62  ///< Stream ioctl timeout
#endif
#ifndef ENOSR
#define ENOSR           63  ///< No stream resources
#endif
#ifndef ENOLINK
#define ENOLINK         67  ///< Virtual circuit is gone
#endif
#ifndef EPROTO
#define EPROTO          71  ///< Protocol error
#endif
#ifndef EMULTIHOP
#define EMULTIHOP       74  ///< Multihop attempted
#endif
#ifndef EBADMSG
#define EBADMSG         77  ///< Bad message
#endif
#ifndef EFTYPE
#define EFTYPE          79  ///< Inappropriate file type or format
#endif
#ifndef ENOSYS
#define ENOSYS          88  ///< Function not implemented
#endif
#ifndef ENOTEMPTY
#define ENOTEMPTY       90  ///< Directory not empty
#endif
#ifndef ENAMETOOLONG
#define ENAMETOOLONG    91  ///< File or path name too long
#endif
#ifndef ELOOP
#define ELOOP           92  ///< Too many symbolic links
#endif
#ifndef EOPNOTSUPP
#define EOPNOTSUPP      95  ///< Operation not supported on socket
#endif
#ifndef EPFNOSUPPORT
#define EPFNOSUPPORT    96  ///< Protocol family not supported
#endif
#ifndef ECONNRESET
#define ECONNRESET      104 ///< Connection reset by peer
#endif
#ifndef ENOBUFS
#define ENOBUFS         105 ///< No buffer space available
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT    106 ///< Address family not supported by protocol family
#endif
#ifndef EPROTOTYPE
#define EPROTOTYPE      107 ///< Protocol wrong type for socket
#endif
#ifndef ENOTSOCK
#define ENOTSOCK        108 ///< Socket operation on non-socket
#endif
#ifndef ENOPROTOOPT
#define ENOPROTOOPT     109 ///< Protocol not available
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN       110 ///< Can't send after socket shutdown
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED    111 ///< Connection refused
#endif
#ifndef EADDRINUSE
#define EADDRINUSE      112 ///< Address already in use
#endif
#ifndef ECONNABORTED
#define ECONNABORTED    113 ///< Software caused connection abort
#endif
#ifndef ENETUNREACH
#define ENETUNREACH     114 ///< Network is unreachable
#endif
#ifndef ENETDOWN
#define ENETDOWN        115 ///< Network interface is not configured
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT       116 ///< Connection timed out
#endif
#ifndef EHOSTDOWN
#define EHOSTDOWN       117 ///< Host is down
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH    118 ///< Host is unreachable
#endif
#ifndef EINPROGRESS
#define EINPROGRESS     119 ///< Connection already in progress
#endif
#ifndef EALREADY
#define EALREADY        120 ///< Socket already connected
#endif
#ifndef EDESTADDRREQ
#define EDESTADDRREQ    121 ///< Destination address required
#endif
#ifndef EMSGSIZE
#define EMSGSIZE        122 ///< Message too long
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT 123 ///< Unknown protocol
#endif
#ifndef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT 124 ///< Socket type not supported
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL   125 ///< Address not available
#endif
#ifndef ENETRESET
#define ENETRESET       126 ///< Connection aborted by network
#endif
#ifndef EISCONN
#define EISCONN         127 ///< Socket is already connected
#endif
#ifndef ENOTCONN
#define ENOTCONN        128 ///< Socket is not connected
#endif
#ifndef ETOOMANYREFS
#define ETOOMANYREFS    129 ///< Too many references: can't splice
#endif

#ifndef ENOTSUP
#define ENOTSUP         134 ///< Not supported
#endif

#ifndef EILSEQ
#define EILSEQ          138 ///< Illegal byte sequence
#endif
#ifndef EOVERFLOW
#define EOVERFLOW       139 ///< Value too large for defined data type
#endif
#ifndef ECANCELED
#define ECANCELED       140 ///< Operation canceled
#endif
#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE 141 ///< State not recoverable
#endif
#ifndef EOWNERDEAD
#define EOWNERDEAD      142 ///< Previous owner died
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK     EAGAIN  ///< Operation would block
#endif

/** @} (end addtogroup errno_error_codes) */

#ifdef __cplusplus
}
#endif

#endif // ERRNO_ERROR_CODES_H
