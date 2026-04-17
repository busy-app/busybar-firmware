/***************************************************************************//**
 * @file
 * @brief Declaration of types used in Bluetooth host HCI event lookup table
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_BT_HCI_EVENT_TABLE_H
#define SLI_BT_HCI_EVENT_TABLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_EXCLUDE_INTERNAL

/***************************************************************************//**
 * @addtogroup sli_bt_hci_event_table Bluetooth host HCI Event Table
 * @{
 *
 * @brief Declaration of types used in Bluetooth host HCI event lookup table
 *
 * The Bluetooth host stack uses a lookup table to map HCI events to the
 * corresponding handler functions. The types and variables related to the
 * lookup table are internal to the Bluetooth host stack and must not be
 * accessed by the application.
 */

/**
 * @brief Type of the HCI event lookup table keys.
 */
typedef uint16_t sli_bt_hci_event_key_t;

/**
 * @brief Opaque function type for the HCI event handler functions.
 */
typedef void (sli_bt_hci_event_handler_func_t)(void *, void **);

// Extern declaration of the HCI event lookup table variables
extern const uint8_t sli_bt_hci_event_lookup_num_entries;
extern const sli_bt_hci_event_key_t sli_bt_hci_event_lookup_keys[];
extern sli_bt_hci_event_handler_func_t * const sli_bt_hci_event_lookup_data[];

/** @} end sli_bt_hci_event_table */

#endif // DOXYGEN_EXCLUDE_INTERNAL

#ifdef __cplusplus
}
#endif

#endif // SLI_BT_HCI_EVENT_TABLE_H
