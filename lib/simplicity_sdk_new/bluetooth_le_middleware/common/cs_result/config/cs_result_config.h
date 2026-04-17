/***************************************************************************//**
 * @file
 * @brief CS result config.
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
#ifndef CS_RESULT_CONFIG_H
#define CS_RESULT_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Header of CS result configuration

// <e CS_RESULT_LOG> Enable result log
// <i> Enable CS result component logger
// <i> Default: 1
#ifndef CS_RESULT_LOG
#define CS_RESULT_LOG                              (1)
#endif

// <q CS_RESULT_DEBUG_LOG> Enable debug log
// <i> Enable CS result component debug logs
// <i> Default: 0
#ifndef CS_RESULT_DEBUG_LOG
#define CS_RESULT_DEBUG_LOG                        (0)
#endif

// <s CS_RESULT_LOG_PREFIX> Log prefix
// <i> Default: "[result]"
#ifndef CS_RESULT_LOG_PREFIX
#define CS_RESULT_LOG_PREFIX                       "[result]"
#endif

// </e>

// <o CS_RESULT_MAX_BUFFER_SIZE> Specify maximum size of result data <32..255>
// <i> Default: 32
#ifndef CS_RESULT_MAX_BUFFER_SIZE
#define CS_RESULT_MAX_BUFFER_SIZE                  (128)
#endif

// <q CS_INITIATOR_BT_ADDR_LOG> Enable Bluetooth address log
// <i> Default: 1
// <i> Enable or disable Bluetooth address logging.
#ifndef CS_INITIATOR_BT_ADDR_LOG
#define CS_INITIATOR_BT_ADDR_LOG               1
#endif

// <q CS_INITIATOR_DISTANCE_LOG> Enable distance log
// <i> Default: 1
// <i> Enable or disable distance logging.
#ifndef CS_INITIATOR_DISTANCE_LOG
#define CS_INITIATOR_DISTANCE_LOG              1
#endif

// <q CS_INITIATOR_RAW_DISTANCE_LOG> Enable Raw distance log
// <i> Default: 0
// <i> Enable or disable RAW distance logging.
#ifndef CS_INITIATOR_RAW_DISTANCE_LOG
#define CS_INITIATOR_RAW_DISTANCE_LOG          0
#endif

// <q CS_INITIATOR_RSSI_DISTANCE_LOG> Enable RSSI distance log
// <i> Default: 0
// <i> Enable or disable RSSI distance logging.
#ifndef CS_INITIATOR_RSSI_DISTANCE_LOG
#define CS_INITIATOR_RSSI_DISTANCE_LOG         0
#endif

// <q CS_INITIATOR_BER_LOG> Enable Bit error rate log
// <i> Default: 0
// <i> Enable or disable Bit error rate logging.
#ifndef CS_INITIATOR_BER_LOG
#define CS_INITIATOR_BER_LOG                   0
#endif

// <q CS_INITIATOR_LIKELINESS_LOG> Enable likeliness log
// <i> Default: 1
// <i> Enable or disable likeliness logging.
#ifndef CS_INITIATOR_LIKELINESS_LOG
#define CS_INITIATOR_LIKELINESS_LOG            1
#endif

// <q CS_INITIATOR_VELOCITY_LOG> Enable velocity log
// <i> Default: 1
// <i> Enable or disable velocity logging.
#ifndef CS_INITIATOR_VELOCITY_LOG
#define CS_INITIATOR_VELOCITY_LOG              1
#endif

// <o CS_INITIATOR_HEADER_LOG> Header log of measurements results <1..20>
// <i> Sets how many measurements are written between header logs in SoC Initiator example
// <i> Only kept for backward compatibility, moved to app_config.h
// <i> Default: 5
#ifndef CS_INITIATOR_HEADER_LOG
#define CS_INITIATOR_HEADER_LOG                5
#endif

// <o CS_HOST_HEADER_LOG> Header log of measurements results <1..20>
// <i> Sets how many measurements are written between header logs in NCP host example
// <i> Only kept for backward compatibility, moved to app_config.h
// <i> Default: 5
#ifndef CS_HOST_HEADER_LOG
#define CS_HOST_HEADER_LOG                5
#endif

// </h>

// <<< end of configuration section >>>

#endif // CS_RESULT_CONFIG_H
