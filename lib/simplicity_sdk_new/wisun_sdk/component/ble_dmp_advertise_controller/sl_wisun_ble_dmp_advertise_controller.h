/***************************************************************************//**
 * @file sl_wisun_ble_dmp_advertise_controller.h
 * @brief Wi-SUN BLE DMP Advertise Controller
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

#ifndef SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_H
#define SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "ble_peer_manager_common.h"
#include "ble_peer_manager_peripheral.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// Thread loop definition
#ifndef SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_THREAD_LOOP
  #define SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_THREAD_LOOP while (1)
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Initialize the Wi-SUN BLE DMP Advertise Controller module.
 * @details This function initializes the Wi-SUN BLE DMP Advertise Controller.
 *****************************************************************************/
void sl_wisun_ble_dmp_advertise_controller_init(void);

/**************************************************************************//**
 * @brief Enable BLE advertisement.
 * @details This function enabled BLE advertisement.
 *****************************************************************************/
void sl_wisun_ble_dmp_advertise_controller_adv_enable(void);

/**************************************************************************//**
 * @brief Disable BLE advertisement.
 * @details This function disables BLE advertisement.
 *****************************************************************************/
void sl_wisun_ble_dmp_advertise_controller_adv_disable(void);

/***************************************************************************//**
 * BLE Peer manager event handler.
 *
 * @param[in] event Event coming from the peer manager
 ******************************************************************************/
void ble_peer_manager_on_event_wisun_advertise_controller(ble_peer_manager_evt_type_t *event);

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_BLE_DMP_ADVERTISE_CONTROLLER_H
