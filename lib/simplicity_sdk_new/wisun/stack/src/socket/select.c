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
#include "sl_assert.h"
#include <string.h>
#include <cmsis_os2.h>

#include "socket/select.h"
#include "socket/socket.h"
#include "socket/sl_select_util.h"

#include "errno.h"
#include "sl_wisun_common.h"

#define READ_SOCK_FDS_CHECK_FLAG      0X00000001U
#define WRITE_SOCK_FDS_CHECK_FLAG     0X00000002U

static osEventFlagsId_t sock_fds_check_event_id = NULL;

// STATIC FUNCTIONS
/*******************************************************************************************************/
/**
 * @brief Checks if a socket is readable.
 *
 * This function uses the `SO_READABLE` non standard socket option to check if the socket is readable.
 *
 * @param fd The file descriptor of the socket.
 *
 * @return true if readable, false otherwise.
 */
static bool is_socket_readable(int fd)
{
  int32_t data_bytes = 0;
  socklen_t opt_size = sizeof(int);
  int ret = getsockopt(fd, SOL_SOCKET, SO_READABLE, &data_bytes, &opt_size);
  return ((ret == 0) ? (data_bytes > 0) : false);
}

/**
 * @brief Checks if a socket is writable.
 *
 * This function uses the `SO_WRITABLE` non standard socket option to check if the socket
 * is writable
 *
 * @param fd The file descriptor of the socket.
 *
 * @return true if writable, false otherwise.
 */
static bool is_socket_writable(int fd)
{
  int32_t sendq_space = 0;
  socklen_t opt_size = sizeof(int);
  int ret = getsockopt(fd, SOL_SOCKET, SO_WRITABLE, &sendq_space, &opt_size);
  return ((ret == 0) ? (sendq_space > 0) : false);
}

/**
 * @brief Checks the state of file descriptors and optionally updates their status.
 *
 * @param nfds The highest-numbered file descriptor in any of the three sets, plus 1.
 * @param readfds Pointer to an object of type fd_set that contains the file descriptors to be checked for being
 *                ready to read.
 * @param writefds Pointer to an object of type fd_set that contains the file descriptors to be checked for being
 *                 ready to write.
 * @param update_fds Boolean flag indicating whether to update the file descriptor sets.
 *
 * @return The number of ready file descriptors.
 *
 * This function checks each file descriptor from 0 to nfds-1 to see if it's ready for reading or writing.
 * If a file descriptor is ready for reading (i.e., data is available to be read), the function increments the count of
 * ready file descriptors.
 * If a file descriptor is ready for writing (i.e., data can be written without blocking), the function also increments
 * the count of ready file descriptors.
 * If update_fds is true and a file descriptor is not ready for reading or writing, the function removes the file
 * descriptor from the respective set.
 */
static int check_fds_state(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, bool update_fds)
{
  int ready_fd_cnt = 0;

  //Check each file descriptor
  for (int fd = 0; fd < nfds; fd++) {
    //Check if the file descriptor is ready for reading
    if (readfds && FD_ISSET(fd, readfds)) {
      if (is_socket_readable(fd)) {
        ready_fd_cnt++;
      } else if (update_fds) {
        FD_CLR(fd, readfds);
      }
    }

    if (writefds && FD_ISSET(fd, writefds)) {
      // If the file descriptor is ready, increment the count
      if (is_socket_writable(fd)) {
        ready_fd_cnt++;
      } else if (update_fds) {
        FD_CLR(fd, readfds);
      }
    }
  }

  return ready_fd_cnt;
}

static bool are_fd_valid(int nfds, fd_set *restrict readfds, fd_set *restrict writefds)
{
  int retval = -1;
  sockaddr_in6_t sock_name = {0};
  socklen_t addr_len = sizeof(sockaddr_in6_t);

  for (int fd = 0; fd < nfds; fd++) {
    // check readfds
    if (readfds && FD_ISSET(fd, readfds)) {
      retval = getsockname(fd, (struct sockaddr *) &sock_name, &addr_len);
      if (retval == -1) {
        break;
      }
    }
    // check writefds
    if (writefds && FD_ISSET(fd, writefds)) {
      retval = getsockname(fd, (struct sockaddr *) &sock_name, &addr_len);
      if (retval == -1) {
        break;
      }
    }
  }
  return (retval == 0);
}

/*******************************************************************************************************/

int select(int nfds, fd_set *restrict readfds,
           fd_set *restrict writefds,
           fd_set *restrict exceptfds,
           struct timeval *restrict timeout)
{
  (void) exceptfds;
  bool is_zero_timeout  = false;
  uint64_t timeout_us   = 0;
  uint64_t ticks        = 0;
  uint32_t flags        = 0x00000000U;
  uint32_t ready_fd_cnt = 0;

  if (nfds < 0 || nfds > FD_SETSIZE) {
    errno = EINVAL;
    return -1;
  }

  if(!are_fd_valid(nfds, readfds, writefds)) {
    errno = EBADF;
    return -1;
  }

  if(!timeout) {
    // timeout is specified as NULL, select() blocks indefinitely waiting
    // for a file descriptor to become ready.
    ticks = osWaitForever;
  } else if (timeout->tv_sec != 0 || timeout->tv_usec != 0) {
    timeout_us = (timeout->tv_sec * 1000000) + timeout->tv_usec;
    ticks      = (timeout_us * osKernelGetTickFreq()) / 1000000;
  } else {
    // both fields of the timeval structure are zero, then select() returns
    // immediately.
    is_zero_timeout = true;
  }

  if(!is_zero_timeout) {
    // check if there is already ready fds without clearing user fd sets
    ready_fd_cnt = check_fds_state(nfds, readfds, writefds, false);

    // If no file descriptors were ready, set the flags and wait for timeout
    while (ready_fd_cnt == 0) {
      flags = osEventFlagsWait(sock_fds_check_event_id,
                              READ_SOCK_FDS_CHECK_FLAG | WRITE_SOCK_FDS_CHECK_FLAG,
                              osFlagsWaitAny,
                              (uint32_t) ticks);
      if (flags == osFlagsErrorParameter) {
        return -1;
      } else if (flags == osFlagsErrorTimeout) {
        break;
      }

      // check if target fds are ready without clearing user fd sets
      ready_fd_cnt = check_fds_state(nfds, readfds, writefds, false);
    }
  }

  // Update user fds and exit
  ready_fd_cnt = check_fds_state(nfds, readfds, writefds, true);

  // Return the number of ready file descriptors
  return ready_fd_cnt;
}

void sl_wisun_select_init(void)
{
  const osEventFlagsAttr_t select_flags_attr = {
    "select event flag",
    0,
    NULL,
    0
  };
  sock_fds_check_event_id = osEventFlagsNew(&select_flags_attr);
  EFM_ASSERT(sock_fds_check_event_id != NULL);
}

void sl_wisun_check_read_sockfd_set(void)
{
  EFM_ASSERT((osEventFlagsSet(sock_fds_check_event_id, READ_SOCK_FDS_CHECK_FLAG) & CMSIS_RTOS_ERROR_MASK) == 0);
}

void sl_wisun_check_write_sockfd_set(void)
{
  EFM_ASSERT((osEventFlagsSet(sock_fds_check_event_id, WRITE_SOCK_FDS_CHECK_FLAG) & CMSIS_RTOS_ERROR_MASK) == 0);
}
