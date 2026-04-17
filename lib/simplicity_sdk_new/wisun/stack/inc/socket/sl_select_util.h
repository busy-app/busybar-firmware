/***************************************************************************//**
 * @file sl_select_util.h
 * @brief select utility functions
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
#ifndef _SYS_SL_SELECT_UTIL_H
#define _SYS_SL_SELECT_UTIL_H

/**************************************************************************//**
 * @addtogroup SL_WISUN_SELECT_API Select API
 * @ingroup SL_WISUN_API
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * Initialize the select system.
 *
 * This function initializes the select system by creating a new event flag with
 * the specified attributes. The event flag is used to indicate when a file
 * descriptor set is ready to be checked for I/O operations.
 *****************************************************************************/
void sl_wisun_select_init(void);

/**************************************************************************//**
 * Check the read socket file descriptor set.
 *
 * This function notifies select to check the read socket file descriptor
 * set. The function should be called in response to the reception of
 * #SL_WISUN_MSG_SOCKET_DATA_AVAILABLE_IND_ID. It notifies select to check
 * the read socket file descriptor set for any ready socket to read.
 *
 * Usage:
 * @code
 *    // Application indication handler
 *    void sl_wisun_on_event(void) {
 *      // Application code here...
 *      switch (evt->header.id) {
 *        // When SL_WISUN_MSG_SOCKET_DATA_AVAILABLE_IND_ID is received
 *        case SL_WISUN_MSG_SOCKET_DATA_AVAILABLE_IND_ID:
 *          // Call the function to check the read socket file descriptor set
 *          sl_wisun_check_read_sockfd_set();
 *          break;
 *      }
 *      // Application code here...
 *    }
 * @endcode
 *****************************************************************************/
void sl_wisun_check_read_sockfd_set(void);

/**************************************************************************//**
 * Check the write socket file descriptor set.
 *
 * This function notifies select to check the write socket file descriptor
 * set. The function should be called in response to the reception of
 * #SL_WISUN_MSG_SOCKET_DATA_SENT_IND_ID. It notifies select to check the
 * write socket file descriptor set for any ready socket to write.
 *
 * Usage:
 * @code
 *    void sl_wisun_on_event(void) {
 *      // Your code here...
 *      switch (evt->header.id) {
 *        // When SL_WISUN_MSG_SOCKET_DATA_SENT_IND_ID is received
 *        case SL_WISUN_MSG_SOCKET_DATA_SENT_IND_ID:
 *          // Call the function to check the write socket file descriptor set
 *          sl_wisun_check_write_sockfd_set();
 *          break;
 *      }
 *      // Your code here...
 *    }
 * @endcode
 *****************************************************************************/
void sl_wisun_check_write_sockfd_set(void);

/** @} */

#endif

