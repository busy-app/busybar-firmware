/***************************************************************************/ /**
 * @file
 * @brief SI91x IO Stream debug Component header.
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
#ifndef SL_SI91X_IOSTREAM_DEBUG_H
#define SL_SI91X_IOSTREAM_DEBUG_H

#include "sl_iostream.h"
#include "sl_status.h"
#include "sl_si91x_iostream_swo_itm_8.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
****************************  DEFINES / MACROS   ********************************
*******************************************************************************/

#define SL_SI91X_IOSTREAM_TYPE_DEBUG_OUTPUT 10 //IOstream Type Si91x Debug

/*******************************************************************************
 *************************** GLOBAL VARIABLES   *******************************
 ******************************************************************************/

extern sl_iostream_t *sl_si91x_iostream_debug_handle;                     ///< sl_si91x_iostream_debug_handle
extern sl_iostream_instance_info_t sl_si91x_iostream_instance_debug_info; ///< sl_si91x_iostream_instance_debug_info

/*******************************************************************************
 *************************** FUNCTION PROTOTYPES  *******************************
 ******************************************************************************/

/***************************************************************************/
/**
 * @brief To initialize IOSTREAM Debug module.
 * 
 * @details This API will initialize IOSTREAM Debug module by:
 *          - Initializing swo itm 
 *          - sets debug_write as iostream write
 *          - Sets debug module as default IOSTREAM
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Success.
 *         - SK_STATUS_FAIL               - fail
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_iostream_debug_init(void);

/***************************************************************************/
/**
 * @brief Sets the Iostream debug type.
 * 
 * @details This API sets the parameter passed as the type for Iostream Debug
 * @param[in] type that need to be set for iostream debug
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
void sl_si91x_iostream_set_debug_type(sl_si91x_iostream_swo_itm_8_msg_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_IOSTREAM_debug_H */
