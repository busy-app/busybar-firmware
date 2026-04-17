/***************************************************************************/ /**
 * @file
 * @brief SI91x IO Stream rtt Component header.
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
#ifndef SL_SI91X_IOSTREAM_RTT_H
#define SL_SI91X_IOSTREAM_RTT_H

#include "sl_iostream.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
  *************************** GLOBAL VARIABLES  *******************************
  ******************************************************************************/

extern sl_iostream_t *sl_si91x_iostream_rtt_handle;                     ///< sl_si91x_iostream_rtt_handle
extern sl_iostream_instance_info_t sl_si91x_iostream_instance_rtt_info; ///< sl_si91x_iostream_instance_rtt_info

/*******************************************************************************
  *************************** FUNCTION PROTOTYPES  *******************************
  ******************************************************************************/

/***************************************************************************/
/**
  * @brief To initialize IOSTREAM RTT module.
  * 
  * @details This API will initialize IOSTREAM Rtt module
  * 
  * @return sl_status_t Status code indicating the result:
  *         - SL_STATUS_OK                 - Success.
  *         - SK_STATUS_FAIL               - fail
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  ******************************************************************************/
sl_status_t sl_si91x_iostream_rtt_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_IOSTREAM_debug_H */
