/***************************************************************************/ /**
 * @file sli_event_engine.h
 * @brief
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

#ifndef SLI_ASYNC_EVENT_HANDLER_H
#define SLI_ASYNC_EVENT_HANDLER_H

#include <stdint.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_slist.h"
#include "sli_queue_manager.h"

// Event flag for Event Engine Async Event
#define SLI_EVENT_ENGINE_ASYNC_EVENT (1 << 18)

/**
 * @typedef sli_event_engine_handler_t
 * @brief Function pointer type for event handler callbacks in the event engine.
 *
 * This type defines the signature for event handler functions that can be registered
 * with the event engine. The handler is called when an event occurs.
 *
 * @param event The event identifier (as a 32-bit unsigned integer).
 * @param data  Pointer to user-defined data associated with the event.
 */
typedef void (*sli_event_engine_handler_t)(uint32_t event, void *data);

/**
 * @brief Initializes the SLI Event Engine.
 *
 * This function sets up the event engine with the specified configuration and associates it
 * with the provided event flags identifier.
 *
 * @param[out] event_engine_eventId Pointer to the event flags identifier that will be initialized.
 * @param[in]  event_config         Pointer to the event engine configuration structure.
 *
 * @return sl_status_t
 *         - SL_STATUS_OK on success.
 *         - Appropriate error code otherwise.
 */
sl_status_t sli_event_engine_init(osEventFlagsId_t *event_engine_eventId);

/**
 * @brief Deinitializes the event engine.
 *
 * This function releases any resources allocated by the event engine and
 * prepares it for shutdown or reinitialization. After calling this function,
 * the event engine will no longer be operational until reinitialized.
 *
 * @return
 *   - SL_STATUS_OK if the deinitialization was successful.
 *   - Appropriate error code otherwise.
 */
sl_status_t sli_event_engine_deinit(void);

/**
 * @brief Registers an event handler for a specific event in the event engine.
 *
 * This function associates a handler function with a given event in the specified event queue.
 * When the SLI_EVENT_ENGINE_ASYNC_EVENT event occurs, the registered handler will be invoked 
 * with the event identifier and associated data.
 *
 * @param event_queue Pointer to the event queue where the event will be registered.
 * @param event Identifier of the event to register.
 * @param handler Function pointer to the event handler to be called when the event occurs.
 * @return sl_status_t Status code indicating success or failure of the registration.
 */

sl_status_t sli_event_engine_register_event(sli_queue_t *event_queue,
                                            uint32_t event,
                                            sli_event_engine_handler_t handler);

#endif // SLI_ASYNC_EVENT_HANDLER_H