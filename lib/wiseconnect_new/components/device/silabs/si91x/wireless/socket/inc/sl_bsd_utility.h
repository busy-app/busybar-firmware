/***************************************************************************/ /**
 * @file
 * @brief sl_bsd_utility.h
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

#include <stdint.h>

/**
 * @addtogroup SI91X_SOCKET_FUNCTIONS
 * @{ 
 */

/***************************************************************************/ /**
 * @brief
 *  Retrieves the Maximum Segment Size (MSS) for a specific socket. The MSS is the largest amount of data, specified in bytes, that a computer or communications device can handle in a single, unfragmented piece.
 * @param[in] socketIndex
 *  The index of the socket for which the MSS is to be retrieved. This is an integer value that uniquely identifies the socket.
 * @return
 *  Returns an int16_t value representing the MSS of the specified socket in bytes.
 ******************************************************************************/
int16_t sl_si91x_get_socket_mss(int32_t socketIndex);

/** @} */
