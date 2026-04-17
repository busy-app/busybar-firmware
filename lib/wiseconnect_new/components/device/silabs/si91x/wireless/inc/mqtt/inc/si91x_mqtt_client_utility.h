/***************************************************************************/ /**
 * @file  si91x_mqtt_client_utility.h
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

#pragma once

#include "si91x_mqtt_client_callback_framework.h"

/**
 * An internal helper function to build sl_mqtt_client_event_t in case an API is called in asynchronous mode.
 * @param event 						Event that needs to be stored in context.
 * @param client 						Client address that needs to be stored in context.
 * @param user_context					user_context provided by user while calling API.
 * @param timeout						timeout which determines if API needs to be executed in sync or async mode.
 * @param si91x_mqtt_client_context_t** A double pointer to client context.This function allocates memory and initializes members if timeout is greater than 0 else pointer is initialized with null.
 * @return								return SL_STATUS_OK in case of success or appropriate status.
 */
sl_status_t sli_si91x_build_mqtt_sdk_context_if_async(sl_mqtt_client_event_t event,
                                                      sl_mqtt_client_t *client,
                                                      void *user_context,
                                                      void *sdk_data,
                                                      uint32_t timeout,
                                                      sl_si91x_mqtt_client_context_t **context);

/**
 * An internal helper function to get current mqtt client.
 * @param client current mqtt client
 *
 * Note: client would be NULL if this function is called before sl_mqtt_client_init or after sl_mqtt_client_deint
 */
void sli_si91x_get_mqtt_client(sl_mqtt_client_t **client);

/**
 * Cleans up resources used by the MQTT client.
 *
 * This function is responsible for releasing any resources allocated to the MQTT client,
 * ensuring a clean shutdown of the client instance. It should be called when the MQTT
 * client is no longer needed, typically at the end of the application or before a restart
 * of the MQTT client.
 *
 * Note: It is important to call this function to prevent resource leaks.
 */
void sli_mqtt_client_cleanup();
