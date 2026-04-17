/***************************************************************************//**
 * @file
 * @brief sl_btmesh_hsl_client.h
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

#ifndef SL_BTMESH_HSL_CLIENT_H
#define SL_BTMESH_HSL_CLIENT_H

/***************************************************************************//**
 * @addtogroup btmesh_hsl_client
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * This function changes the hue and sends it to the server.
 *
 * @param[in] hue_percent Defines new hue value as percentage
 *                        Valid values 0-100 %
 ******************************************************************************/
void sl_btmesh_hsl_client_set_hue(uint8_t hue_percent);

/***************************************************************************//**
 * This function changes the saturation and sends it to the server.
 *
 * @param[in] saturation_percent Defines new color saturation value as percentage
 *                               Valid values 0-100 %
 ******************************************************************************/
void sl_btmesh_hsl_client_set_saturation(uint8_t saturation_percent);

/***************************************************************************//**
 * This function changes the lightness and sends it to the server.
 *
 * @param[in] lightness_percent Defines new lightness value as percentage
 *                              Valid values 0-100 %
 ******************************************************************************/
void sl_btmesh_hsl_client_set_lightness(uint8_t lightness_percent);

/** @} end btmesh_hsl_client */

#endif // SL_BTMESH_HSL_CLIENT_H
