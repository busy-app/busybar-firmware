/***************************************************************************//**
 * @file sl_wisun_rtt_logger.h
 * @brief Wi-SUN RTT Logger
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

#ifndef SL_WISUN_RTT_LOGGER_H
#define SL_WISUN_RTT_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// Thread loop definition
#ifndef SL_WISUN_RTT_LOGGER_THREAD_LOOP
  #define SL_WISUN_RTT_LOGGER_THREAD_LOOP while (1)
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Initialize the Wi-SUN RTT Logger module.
 * @details This function initializes the Wi-SUN RTT Logger module.
 *****************************************************************************/
void sl_wisun_rtt_logger_init(void);

/**************************************************************************//**
 * @brief Start RTT report.
 * @details This function starts the RTT report timer.
 *****************************************************************************/
void sl_wisun_rtt_logger_start_report(void);

/**************************************************************************//**
 * @brief Stop RTT report.
 * @details This function stops the RTT report timer.
 *****************************************************************************/
void sl_wisun_rtt_logger_stop_report(void);

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_RTT_LOGGER_H
