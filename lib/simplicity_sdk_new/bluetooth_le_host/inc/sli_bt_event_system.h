/***************************************************************************//**
 * @file
 * @brief Bluetooth Event System IPC
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SLI_BT_EVENT_SYSTEM_H
#define SLI_BT_EVENT_SYSTEM_H

#include "sl_status.h"
#include "sl_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup bluetooth_event_system Bluetooth Event System IPC
 * @{
 *
 * @brief Bluetooth Event System IPC
 *
 * The Bluetooth Event System IPC component (`bluetooth_event_system_ipc`) is an
 * optional component that can be included to use Event System IPC to deliver
 * Bluetooth events in an application that uses an RTOS. When the component is
 * included, the Bluetooth stack will not deliver events via callback functions
 * but uses the IPC mechanism provided by the `event_system` component to
 * publish events to the application.
 *
 * When `bluetooth_event_system_ipc` is included in the application, use the
 * APIs provided by `event_system` to create the subscriber event queue and to
 * subscribe to the Bluetooth events. To make sure the application does not miss
 * any event, perform the subscription already in the `app_init` function.
 */

/** @cond DOXYGEN_INCLUDE_INTERNAL */

/**
 * @brief Make permanent allocations for Bluetooth support for Event System IPC
 *
 * This function is called by Bluetooth RTOS adaptation at the right time in the
 * init sequence.
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
sl_status_t sli_bt_event_system_permanent_allocations(void);

/**
 * @brief Perform functional init for Bluetooth support for Event System IPC
 *
 * This function is called by Bluetooth RTOS adaptation at the right time in the
 * init sequence.
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
sl_status_t sli_bt_event_system_functional_init(void);

/**
 * @brief Publish a BGAPI event to Event System IPC
 *
 * This function is called by Bluetooth RTOS adaptation to publish BGAPI events
 * one at a time. Regardless of success or failure, the Bluetooth Event System
 * IPC component will call @ref sli_bt_rtos_set_event_handled to let the RTOS
 * adaptation know when it can free the event.
 *
 * @param[in] event The event to publish
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
sl_status_t sli_bt_publish_to_event_system(sl_bt_msg_t *event);

/** @endcond */ // DOXYGEN_INCLUDE_INTERNAL

/** @} end bluetooth_event_system */

#ifdef __cplusplus
}
#endif

#endif // SLI_BT_EVENT_SYSTEM_H
