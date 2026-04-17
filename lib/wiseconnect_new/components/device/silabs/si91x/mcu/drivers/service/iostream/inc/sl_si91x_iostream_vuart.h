/***************************************************************************/ /**
 * @file
 * @brief IO Stream VUART SI91X.
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
#ifndef SL_SI91X_IOSTREAM_VUART_H
#define SL_SI91X_IOSTREAM_VUART_H

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
 * @addtogroup iostream_vuart_si91x I/O Stream VUART SI91X
 * @brief IO Stream VUART SI91X
 * @details
 * ## Overview
 *
 *   Virtual UART (VUART) is a special interface where RTT is used as input channel
 *   and the Serial Wire Output(SWO) is used as output channel.
 *
 *   On the receive side, a RTT buffer with a dedicated name is allocated for
 *   the reception.
 *
 *   IOStream-VUART uses SWO-stimulus 8 as output channel. Note that the transmit
 *   channel also implements a small protocol, so the data is encapsulated in a
 *   frame which must be decoded on the host side.
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
 *   For connecting to the vuart console you need to use the WPK and you must be
 *   connected using the ethernet link. Then you can open a telnet session on port
 *   4900 using the WPK IP address. Note that the WPK firmware decodes the
 *   received frame and it outputs only the payload into vuart console.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
****************************  DEFINES / MACROS   ********************************
*******************************************************************************/

#define SL_SI91X_IOSTREAM_TYPE_VUART 9 ///<Iostream Type Si9ax Vuart

/*******************************************************************************
****************************  GLOBAL VARIABLES   ********************************
*******************************************************************************/

extern sl_iostream_t *sl_si91x_iostream_vuart_handle;                     ///< sl_si91x_iostream_vuart_handle
extern sl_iostream_instance_info_t sl_si91x_iostream_instance_vuart_info; ///< sl_si91x_iostream_instance_vuart_info

/*******************************************************************************
****************************  FUNCTION PROTOTYPES   ********************************
*******************************************************************************/

/***************************************************************************/
/**
 * @brief To initialize IOSTREAM Vuart module.
 * 
 * @details This API will initialize IOSTREAM Vuart module by:
 *          - Initializing swo itm channel 8
 *          - Configures RTT down buffer
 *          - Sets vuart_write as iostream vuart write
 *          - Sets vuart_read as iostream vuart read
 *          - Sets vuart module as default IOSTREAM
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Success.
 *         - SK_STATUS_FAIL               - fail
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_iostream_vuart_init(void);

/** @} (end addtogroup iostream_vuart_si91x) */
/** @} (end addtogroup iostream_si91x) */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_IOSTREAM_VUART_H */
