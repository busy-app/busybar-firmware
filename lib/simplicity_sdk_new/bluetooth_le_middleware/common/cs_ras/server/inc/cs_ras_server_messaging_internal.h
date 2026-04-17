/***************************************************************************//**
 * @file
 * @brief CS RAS Server - Messaging internal header
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
#ifndef CS_RAS_SERVER_MESSAGING_INTERNAL_H
#define CS_RAS_SERVER_MESSAGING_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_bt_api.h"
#include "cs_ras_common.h"
#include "cs_ras_server_messaging.h"
#include "sl_slist.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Initialize messaging
 *****************************************************************************/
void cs_ras_server_messaging_init(void);

/**************************************************************************//**
 * Step function (called from RAS Server step function).
 *****************************************************************************/
void cs_ras_server_messaging_step(void);

/**************************************************************************//**
 * Bluetooth event handler of RAS Server messaging.
 * @param[in] evt Event coming from the Bluetooth stack.
 * @return true to send the event to the host in NCP case.
 *****************************************************************************/
bool cs_ras_server_messaging_on_bt_event(sl_bt_msg_t *evt);

/**************************************************************************//**
 * Transmit finished callback
 *
 * @param[in] tx     Pointer to the transmit structure
 * @param[in] status Transmission status
 * @param[in] sc     Status code of the transmission
 *****************************************************************************/
void cs_ras_server_messaging_on_transmit_finished(cs_ras_server_messaging_transmit_t *tx,
                                                  cs_ras_messaging_status_t          status,
                                                  sl_status_t                        sc);

#ifdef SL_CATALOG_POWER_MANAGER_PRESENT

/***************************************************************************//**
 * Checks if it is ok to sleep now / power manager
 * @return true - if ready to sleep, false otherwise
 ******************************************************************************/
bool cs_ras_server_messaging_is_ok_to_sleep(void);

#endif // SL_CATALOG_POWER_MANAGER_PRESENT

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_SERVER_MESSAGING_INTERNAL_H
