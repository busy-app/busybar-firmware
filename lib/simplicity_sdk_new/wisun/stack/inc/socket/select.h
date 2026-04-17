/***************************************************************************//**
 * @file select.h
 * @brief select implementation example
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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



#ifdef __cplusplus
extern "C" {
// C++ doesn't provide restrict
#ifdef __GNUC__
#define restrict __restrict
#else
#define restrict
#endif
#endif

#if defined __IAR_SYSTEMS_ICC__
/// Timeval structure for IAR
struct timeval {
  /// Seconds
  int tv_sec;
  /// Microseconds
  long tv_usec;
};
#else
#include <sys/time.h>
#endif

/**************************************************************************//**
 * @addtogroup SL_WISUN_SELECT_API Select API
 * @ingroup SL_WISUN_API
 *
 * Select API provides an implementation of select() function, modeled
 * after POSIX.1-2024 specification.
 *
 * In order to utilize the function, the application must utilize
 * \a wisun_select component, and call the componen callbacks
 * sl_wisun_check_read_sockfd_set() an sl_wisun_check_write_sockfd_set()
 * from its event handler.
 *
 * @{
 *****************************************************************************/

/// The number of bits in an fd_mask.
#define NFDBITS (8 * (int) sizeof (fd_mask))

/// Maximum number of file descriptors in `fd_set'.
#define FD_SETSIZE 64

/// A type representing a set of file descriptors.
typedef long int fd_mask;

 /// A structure representing a set of file descriptors for select operations.
typedef struct {
  /// An array of fd_mask, each bit representing a file descriptor.
  fd_mask fds_bits[FD_SETSIZE / (8 * (int) sizeof (fd_mask))];
} fd_set;

/* FD set operations */

/**************************************************************************//**
 * Calculate the index in the fds_bits array for the file descriptor d.
 *
 * @param[in] d The file descriptor.
 * @return The index in the fds_bits array.
 *****************************************************************************/
#define FD_ELT(d) ((d) / NFDBITS)

/**************************************************************************//**
 * Create a mask for the bit position within an fd_mask for the file descriptor d.
 *
 * @param[in] d The file descriptor.
 * @return The mask for the bit position.
 *****************************************************************************/
#define FD_MASK(d) ((fd_mask) 1 << ((d) % NFDBITS))

/**************************************************************************//**
 * Clear the bit for the file descriptor d in the fd_set.
 *
 * @param[in] d The file descriptor.
 * @param[in,out] set The fd_set.
 *****************************************************************************/
#define FD_CLR(d, set) ((void) ((set)->fds_bits[FD_ELT(d)] &= ~FD_MASK (d)))

/**************************************************************************//**
 * Set the bit for the file descriptor d in the fd_set.
 *
 * @param[in] d The file descriptor.
 * @param[in,out] set The fd_set.
 *****************************************************************************/
#define FD_SET(d, set) ((void) ((set)->fds_bits[FD_ELT(d)] |= FD_MASK(d)))

/**************************************************************************//**
 * Clear all bits in the fd_set.
 *
 * @param[in,out] set The fd_set.
 *****************************************************************************/
#define FD_ZERO(set) ((void) memset((void *) (set), 0, sizeof (fd_set)))

/**************************************************************************//**
 * Check whether the bit for the file descriptor d is set in the fd_set.
 *
 * @param[in] d The file descriptor.
 * @param[in] set The fd_set.
 * @return Nonzero if the bit is set, and zero otherwise.
 *****************************************************************************/
#define FD_ISSET(d, set) ((set)->fds_bits[FD_ELT(d)] & FD_MASK(d))

/**************************************************************************//**
 * Monitor multiple file descriptors for readiness.
 *
 * @param[in] nfds The highest-numbered file descriptor in any of the three
 *                 sets, plus 1.
 * @param[in,out] readfds An optional pointer to a set of file descriptors
 *                        to be checked for readability.
 * @param[in,out] writefds An optional pointer to a set of file descriptors
 *                         to be checked for writability.
 * @param[in,out] exceptfds An optional pointer to a set of file descriptors
 *                          to be checked for exceptions.
 * @param[in] timeout An optional timeout. If NULL, select() blocks indefinitely.
 *                    To specify a non-blocking poll, set the timeout to zero.
 * @return On success, returns the total number of bits set in readfds, writefds.
 *         If the time limit expires, select() returns zero. On error,
 *         -1 is returned, and errno is set appropriately.
 *
 * This function monitors multiple file descriptors, waiting until one or more
 * of the file descriptors are ready for an I/O operation.
 *
 * @note This implementation does not handle exceptfds.
 *****************************************************************************/
int select (int nfds, fd_set *restrict readfds,
            fd_set *restrict writefds,
            fd_set *restrict exceptfds,
            struct timeval *restrict timeout);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* socket/select.h */

