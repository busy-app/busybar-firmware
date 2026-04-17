/***************************************************************************/ /**
 * @file errno.h
 * @brief errno standard header
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_SI91X_ERRNO_H
#define SL_SI91X_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * @addtogroup errno Errno
 * @details Errno contains the definition for errno and the defines for
 * the errno values.
 * @{
 ******************************************************************************/

/***************************************************************************/ /**
 * Macro for errno usage.
 * Can be used to retrieve errno value and to assign errno value since function
 * returns address of variable used to store errno value.
 ******************************************************************************/
#define errno (*sl_si91x_errno())

/***************************************************************************/ /**
 * Errno function used with macro errno.
 *
 * @return  Address of global variable used to store errno value
 ******************************************************************************/
int *sl_si91x_errno(void);

// -----------------------------------------------------------------------------
// Errno Define Values

#define EPERM   1  /* Operation not permitted */
#define ENOENT  2  /* No such file or directory */
#define ESRCH   3  /* No such process */
#define EINTR   4  /* Interrupted system call */
#define EIO     5  /* Input/output error */
#define ENXIO   6  /* Device not configured */
#define E2BIG   7  /* Argument list too long */
#define ENOEXEC 8  /* Exec format error */
#define EBADF   9  /* Bad file descriptor */
#define ECHILD  10 /* No child processes */
#define EDEADLK 11 /* Resource deadlock avoided */
                   /* 11 was EAGAIN */
#define ENOMEM 12  /* Cannot allocate memory */
#define EACCES 13  /* Permission denied */
#define EFAULT 14  /* Bad address */
#if __BSD_VISIBLE
#define ENOTBLK 15 /* Block device required */
#endif
#define EBUSY   16 /* Device busy */
#define EEXIST  17 /* File exists */
#define EXDEV   18 /* Cross-device link */
#define ENODEV  19 /* Operation not supported by device */
#define ENOTDIR 20 /* Not a directory */
#define EISDIR  21 /* Is a directory */
#define EINVAL  22 /* Invalid argument */
#define ENFILE  23 /* Too many open files in system */
#define EMFILE  24 /* Too many open files */
#define ENOTTY  25 /* Inappropriate ioctl for device */
#define ETXTBSY 26 /* Text file busy */
#define EFBIG   27 /* File too large */
#define ENOSPC  28 /* No space left on device */
#define ESPIPE  29 /* Illegal seek */
#define EROFS   30 /* Read-only file system */
#define EMLINK  31 /* Too many links */
#define EPIPE   32 /* Broken pipe */

/* math software */
#define EDOM   33 /* Numerical argument out of domain */
#define ERANGE 34 /* Result too large */

/* non-blocking and interrupt i/o */
#define EAGAIN      35     /* Resource temporarily unavailable */
#define EWOULDBLOCK EAGAIN /* Operation would block */
#define EINPROGRESS 36     /* Operation now in progress */
#define EALREADY    37     /* Operation already in progress */

/* ipc/network software -- argument errors */
#define ENOTSOCK        38 /* Socket operation on non-socket */
#define EDESTADDRREQ    39 /* Destination address required */
#define EMSGSIZE        40 /* Message too long */
#define EPROTOTYPE      41 /* Protocol wrong type for socket */
#define ENOPROTOOPT     42 /* Protocol not available */
#define EPROTONOSUPPORT 43 /* Protocol not supported */
#if __BSD_VISIBLE
#define ESOCKTNOSUPPORT 44 /* Socket type not supported */
#endif
#define EOPNOTSUPP 45 /* Operation not supported */
#if __BSD_VISIBLE
#define EPFNOSUPPORT 46 /* Protocol family not supported */
#endif
#define EAFNOSUPPORT  47 /* Address family not supported by protocol family */
#define EADDRINUSE    48 /* Address already in use */
#define EADDRNOTAVAIL 49 /* Can't assign requested address */

/* ipc/network software -- operational errors */
#define ENETDOWN     50 /* Network is down */
#define ENETUNREACH  51 /* Network is unreachable */
#define ENETRESET    52 /* Network dropped connection on reset */
#define ECONNABORTED 53 /* Software caused connection abort */
#define ECONNRESET   54 /* Connection reset by peer */
#define ENOBUFS      55 /* No buffer space available */
#define EISCONN      56 /* Socket is already connected */
#define ENOTCONN     57 /* Socket is not connected */
#if __BSD_VISIBLE
#define ESHUTDOWN    58 /* Can't send after socket shutdown */
#define ETOOMANYREFS 59 /* Too many references: can't splice */
#endif                  /* __BSD_VISIBLE */
#define ETIMEDOUT    60 /* Operation timed out */
#define ECONNREFUSED 61 /* Connection refused */

#define ELOOP        62 /* Too many levels of symbolic links */
#define ENAMETOOLONG 63 /* File name too long */

/* should be rearranged */
#if __BSD_VISIBLE
#define EHOSTDOWN 64    /* Host is down */
#endif                  /* __BSD_VISIBLE */
#define EHOSTUNREACH 65 /* No route to host */
#define ENOTEMPTY    66 /* Directory not empty */

/* quotas & mush */
#if __BSD_VISIBLE
#define EPROCLIM 67 /* Too many processes */
#define EUSERS   68 /* Too many users */
#endif              /* __BSD_VISIBLE */
#define EDQUOT 69   /* Disk quota exceeded */

/* Network File System */
#define ESTALE 70 /* Stale NFS file handle */
#if __BSD_VISIBLE
#define EREMOTE       71 /* Too many levels of remote in path */
#define EBADRPC       72 /* RPC struct is bad */
#define ERPCMISMATCH  73 /* RPC version wrong */
#define EPROGUNAVAIL  74 /* RPC program not available */
#define EPROGMISMATCH 75 /* Program version wrong */
#define EPROCUNAVAIL  76 /* Bad procedure for program */
#endif                   /* __BSD_VISIBLE */

#define ENOLCK 77 /* No locks available */
#define ENOSYS 78 /* Function not implemented */

#if __BSD_VISIBLE
#define EFTYPE    79 /* Inappropriate file type or format */
#define EAUTH     80 /* Authentication error */
#define ENEEDAUTH 81 /* Need authenticator */
#define EIPSEC    82 /* IPsec processing failure */
#define ENOATTR   83 /* Attribute not found */
#endif               /* __BSD_VISIBLE */
#define EILSEQ 84    /* Illegal byte sequence */
#if __BSD_VISIBLE
#define ENOMEDIUM   85     /* No medium found */
#define EMEDIUMTYPE 86     /* Wrong medium type */
#endif                     /* __BSD_VISIBLE */
#define EOVERFLOW       87 /* Value too large to be stored in data type */
#define ECANCELED       88 /* Operation canceled */
#define EIDRM           89 /* Identifier removed */
#define ENOMSG          90 /* No message of desired type */
#define ENOTSUP         91 /* Not supported */
#define EBADMSG         92 /* Bad message */
#define ENOTRECOVERABLE 93 /* State not recoverable */
#define EOWNERDEAD      94 /* Previous owner died */
#define EPROTO          95 /* Protocol error */
#if __BSD_VISIBLE
#define ELAST 95 /* Must be equal largest errno */
#endif           /* __BSD_VISIBLE */

#ifdef _KERNEL
/* pseudo-errors returned inside kernel to modify return to process */
#define ERESTART    -1 /* restart syscall */
#define EJUSTRETURN -2 /* don't modify regs, just return */
#endif                 /* !_KERNEL */
/** @} (end addtogroup errno) */
#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_ERRNO_H