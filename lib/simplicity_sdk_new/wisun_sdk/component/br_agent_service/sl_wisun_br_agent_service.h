/***************************************************************************//**
 * @file sl_wisun_br_agent_service.h
 * @brief Wi-SUN Border Router Agent Service
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

#ifndef SL_WISUN_BR_AGENT_SERVICE_H
#define SL_WISUN_BR_AGENT_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sl_wisun_events.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Thread loop definition
#ifndef SL_WISUN_BR_AGENT_SERVICE_LOOP
  #define SL_WISUN_BR_AGENT_SERVICE_LOOP while (1)
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Initialize the SoC Agent Service module.
 * @details This function initializes the SoC Agent Service module.
 *****************************************************************************/
void sl_wisun_br_agent_service_init(void);

/**************************************************************************//**
 * @brief Send graph topology information to the host Agent Service.
 * @details This function sends the graph topology information to the host Agent Service.
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_agent_service_send_graph_info(void);

/**************************************************************************//**
 * @brief Set the remote address of the host Border Router Bridge Agent Service.
 * @details This function sets the remote address of the
 *          host Border Router Bridge Agent Service.
 *          The port is set to the default value.
 *
 * @param[in] remote_address Pointer to the remote address string
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_agent_service_set_bridge_agent_addr(const char *remote_address);

/**************************************************************************//**
 * @brief Get the remote address of the host Border Router Bridge Agent Service.
 * @details This function gets the remote address of the host Border Router Bridge Agent Service.
 *          The returned pointer must be freed by the caller using sl_free().
 *
 * @return Pointer to the remote address string
 *****************************************************************************/
const char *sl_wisun_br_agent_service_get_bridge_agent_addr(void);

/**************************************************************************//**
 * @brief Start the Agent Service.
 * @details This function starts the Agent Service.
 *          Should be called after Wi-Fi connection is established.
 *****************************************************************************/
void sl_wisun_agent_start_service(void);

/**************************************************************************//**
 * @brief Send registration (config) information to the host Agent Service.
 * @details This function sends the current BR config information to the host Agent Service.
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
sl_status_t sl_wisun_br_agent_service_send_reg(void);

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_BR_AGENT_SERVICE_H
