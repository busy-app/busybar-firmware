/***************************************************************************/ /**
 * @file
 * @brief IO Stream SWO ITM SI91X.
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
#ifndef SLI_SI91X_IOSTREAM_SWO_ITM_8_H
#define SLI_SI91X_IOSTREAM_SWO_ITM_8_H

#include "sl_iostream.h"
#include "sl_status.h"
#include "sl_si91x_iostream_swo_itm_8.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
****************************  FUNCTION PROTOTYPES   ********************************
*******************************************************************************/

/***************************************************************************/
/**
 * @brief To initialize ITM Channle 8.
 * 
 * @details This API will initialize swo stimulus 8 by writing to Trace Enable Register
 * 
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
void *sli_si91x_iostream_swo_itm_8_init(void);

/***************************************************************************/
/**
 * @brief Writes data to vuart interface
 * 
 * @details This API write data to vuart interface by encapsulating data in a frame and writing it to SWO
 * @param[in] Context , context reserved for future use
 * @param[in] buffer , buffer that contains the send data
 * @param[in] buffer_length , length of the buffer
 * @param[in] type ,message type that needs to be sent through vuart
 * @param[in] seq_nbr ,sequence number that is part of the vuart frame
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Success.
 *         - SL_STATUS_NOT_INITIALIZED    - fail ITM trace not enabled
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sli_si91x_iostream_swo_itm_8_write(void *context,
                                               const void *buffer,
                                               size_t buffer_length,
                                               sl_si91x_iostream_swo_itm_8_msg_type_t type,
                                               uint8_t seq_nbr);

#ifdef __cplusplus
}
#endif

#endif /* SLI_SI91X_IOSTREAM_SWO_ITM_8_H */
