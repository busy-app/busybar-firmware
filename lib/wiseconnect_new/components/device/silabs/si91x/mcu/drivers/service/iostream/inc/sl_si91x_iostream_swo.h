/***************************************************************************/ /**
 * @file
 * @brief IO Stream SWO Component.
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
#ifndef SL_SI91X_IOSTREAM_SWO_H
#define SL_SI91X_IOSTREAM_SWO_H

#include "sl_iostream.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/ /**
 * @addtogroup iostream_si91x
 * @{
 ******************************************************************************/

/***************************************************************************/ /**
 * @addtogroup iostream_swo_si91x I/O Stream SWO SI91X
 * @brief I/O Stream SWO SI91X
 * @details
 * ## Overview
 *
 *   Serial Wire Output (SWO) is an output stream only, which means it is impossible
 *   to receive data. SWO has a dedicated pin allowing the CPU to output data in
 *   real-time.
 *
 *   IOStream-SWO uses channel 0 for its own purpose.The module also set the SWO output
 *   frequency around 863 kHz and uses the UART encoding .
 *
 * ## Initialization
 *
 *   The stream sets itself as the default stream at the end of the initialization
 *   function.You must reconfigure the default interface if you have multiple streams
 *   in your project else the last stream initialized will be set as the system default
 *   stream.
 *
 * ## Communication channel connection
 *
 *   For connecting to the SWO channel you can use the tools provided by Segger or you
 *   can open a telnet session and connect to the port 4091 using your host IP
 *   address when the debugger is connected using USB and using J-Link debugger IP address
 *   when your debugger is connected over ethernet.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 *************************** GLOBAL VARIABLES   *******************************
 ******************************************************************************/

extern sl_iostream_t *sl_si91x_iostream_swo_handle;                     ///< sl_si91x_iostream_swo_handle.
extern sl_iostream_instance_info_t sl_si91x_iostream_instance_swo_info; ///< sl_si91x_iostream_instance_swo_info.

/*******************************************************************************
****************************  DEFINES / MACROS   ********************************
*******************************************************************************/

#define SL_SI91X_IOSTREAM_TYPE_SWO 8 ///< IOSTREAM Type Si91x SWO.

/*******************************************************************************
****************************  FUNCTION PROTOTYPES  ********************************
*******************************************************************************/

/***************************************************************************/
/**
 * @brief To initialize IOSTREAM SWO module.
 * 
 * @details This API will initialize IOSTREAM SWO module by:
 *          - Initializing ITM with Trace Enable Register for channel 0
 *          - Sets SWO module as default IOSTREAM
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Success.
 *         - SK_STATUS_FAIL               - fail
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_iostream_swo_init(void);

/**
 * @brief To de-initialize IOSTREAM SWO module.
 * 
 * @details This API will initialize IOSTREAM SWO module by:
 *          - De-Initializing ITM with Trace Enable Register for channel 0
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Success.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_iostream_swo_deinit(void);

/** @} (end addtogroup iostream_si91x) */
/** @} (end addtogroup iostream_swo_si91x) */

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_IOSTREAM_SWO_H
