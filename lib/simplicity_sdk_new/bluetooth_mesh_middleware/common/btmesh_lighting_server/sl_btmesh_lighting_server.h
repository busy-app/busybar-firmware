/***************************************************************************//**
 * @file
 * @brief btmesh_lighting_server.h
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

#ifndef SL_BTMESH_LIGHTING_SERVER_H
#define SL_BTMESH_LIGHTING_SERVER_H

/***************************************************************************//**
 * @addtogroup btmesh_lighting_server
 * @{
 ******************************************************************************/

#include "sl_btmesh_api.h"
#include "sl_btmesh_lighting_level_transition_handler.h"

/***************************************************************************//**
 * Lighting Server initialization.
 * This should be called at each boot if provisioning is already done.
 * Otherwise this function should be called after provisioning is completed.
 * This function is called automatically after enabling the component.
 ******************************************************************************/
void sl_btmesh_lighting_server_init(void);

/***************************************************************************//**
 * Handling of mesh lighting server events.
 *
 * This function is called automatically after enabling the component.
 *
 * @param[in] evt  Pointer to incoming event.
 ******************************************************************************/
void sl_btmesh_lighting_server_on_event(sl_btmesh_msg_t *evt);

/***************************************************************************//**
 * Handle node reset.
 *
 * Function for clearing component specific nvm content during node reset.
 * This function is called automatically by Universal Configurator after
 * enabling the component.
 *
 ******************************************************************************/
void sl_btmesh_lighting_server_on_node_reset(void);

/***************************************************************************//**
 * Get current lightness value
 *
 * @return Current lightness
 ******************************************************************************/
uint16_t sl_btmesh_get_lightness_current(void);

/***************************************************************************//**
 * Set current lightness value
 *
 * @param[in] lightness  Current lightness
 ******************************************************************************/
void sl_btmesh_set_lightness_current(uint16_t lightness);

/***************************************************************************//**
 * Get target lightness value
 *
 * @return Target lightness
 ******************************************************************************/
uint16_t sl_btmesh_get_lightness_target(void);

/***************************************************************************//**
 * Set target lightness value
 *
 * @param[in] lightness  Target lightness
 ******************************************************************************/
void sl_btmesh_set_lightness_target(uint16_t lightness);

/***************************************************************************//**
 * Get default lightness value
 *
 * @return Default lightness
 ******************************************************************************/
uint16_t sl_btmesh_get_lightness_default(void);

/***************************************************************************//**
 * Set default lightness value
 *
 * @param[in] lightness  Default lightness
 ******************************************************************************/
void sl_btmesh_set_lightness_default(uint16_t lightness);

/***************************************************************************//**
 * Set last lightness value
 *
 * @param[in] lightness  Last lightness
 ******************************************************************************/
void sl_btmesh_set_lightness_last(uint16_t lightness);

/***************************************************************************//**
 * This function converts mesh format of default transition time to milliseconds.
 *
 * @return Default transition time in milliseconds.
 ******************************************************************************/
uint32_t sl_btmesh_get_default_transition_time(void);

/***************************************************************************//**
 * Get default lightness value on power up
 *
 * @return Default lightness on power up
 ******************************************************************************/
uint8_t sl_btmesh_get_lightness_onpowerup(void);

/***************************************************************************//**
 * This function updates the lightness level in the mesh stack during a transition
 *
 * @param[in] lightness     Current lightness level within the transition
 * @param[in] remaining_ms  Remaining transition time in milliseconds
 ******************************************************************************/
void sl_btmesh_update_lightness(uint16_t lightness, uint32_t remaining_ms);

/***************************************************************************//**
 * This function stops generic level move on primary element.
 ******************************************************************************/
void pri_level_move_stop(void);

/***************************************************************************//**
 * Checks whether the provided lightness value is within the allowed range
 * for the lighting server. If the value is out of range, it will be corrected to
 * the nearest valid value
 *
 * @param lightness The input lightness value to validate and correct
 * @return uint16_t The validated and possibly corrected lightness value
 ******************************************************************************/
uint16_t lightness_validate_and_correct(uint16_t);

/** @} (end addtogroup btmesh_lighting_server) */

/***************************************************************************//**
 * Register a callback function to be called when the lightness state changes.
 * Example: when the lightness state is changed by a client request, either by
 * a Generic OnOff Set message or a Light Lightness Set message.
 *
 ******************************************************************************/
typedef void (*sl_btmesh_lighting_onoff_state_change_cb_t)(void);

sl_status_t sl_btmesh_register_lightness_onoff_state_change_cb(sl_btmesh_lighting_onoff_state_change_cb_t callback);

#endif // SL_BTMESH_LIGHTING_SERVER_H
