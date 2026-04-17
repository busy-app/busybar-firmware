/***************************************************************************//**
 * @brief Provides API to create HCI event handler hooks to filter out events
 *  that are passed to host stack
 *
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

#ifndef SLI_BTCTRL_HCI_EVENT_H
#define SLI_BTCTRL_HCI_EVENT_H

#include <stddef.h>
#include <stdbool.h>
#include <sl_status.h>

enum sl_btctrl_hci_event_filter_status {
  SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_ACCEPT = 0x00,
  SL_BTCTRL_HCI_EVENT_FILTER_STATUS_EVENT_DISCARD = 0x01
};

/**
 * @brief Struct declaration for opaque pointer
 * It is intentionally undefined to prevent direct manipulation.
 */
struct sl_btctrl_hci_event;

/**
 * @brief HCI event filter callback function
 * @important Filtering function should only return false for advertising events or other discardable events
 * @important Host stack might end up in deadlock if command complete or other state affecting events are discarded
 * @param event Opaque handle to the HCI event
 * @return sl_btctrl_hci_event_filter_status SL_BTCTRL_HCI_FILTER_STATUS_EVENT_ACCEPT continue processing event,
 * SL_BTCTRL_HCI_FILTER_STATUS_EVENT_DISCARD discard event
 */
typedef enum sl_btctrl_hci_event_filter_status (*sl_btctrl_hci_event_callback)(struct sl_btctrl_hci_event *event);

/**
 * @brief Event handler structure
 * Event handler structure must be allocated from the heap
 * Controller will fill and use the structure members, do not modify directly
 */
typedef struct sl_btctrl_hci_event_handler {
  struct sl_btctrl_hci_event_handler *next;
  sl_btctrl_hci_event_callback handler;
} sl_btctrl_hci_event_handler_t;

/**
 * @brief Register HCI Event handler callback function
 *
 * @param handler_ptr Pointer to sl_btctrl_hci_event_handler_t structure, this is fully initialized by the function
 * @param callback function pointer to the filter callback function
 * @return sl_status_t SL_STATUS_OK if success
 */
sl_status_t sl_btctrl_hci_register_event_handler(sl_btctrl_hci_event_handler_t *handler_ptr, sl_btctrl_hci_event_callback callback);

/**
 * @brief Clear all registered event handler callbacks
 * @return sl_status_t SL_STATUS_OK if success
 */
sl_status_t sl_btctrl_hci_clear_event_handlers(void);

/**
 * @brief Get opcode of HCI event
 * @param event Event handler passed to the callback function
 * @param event_code pointer to where to place the opcode of event
 * @param subevent_code pointer to where to place the subevent of event, only used if event code is 0x3e (LE Meta event)
 * @return sl_status_t SL_STATUS_OK if success
 */
sl_status_t sl_btctrl_hci_event_get_opcode(struct sl_btctrl_hci_event *event, uint8_t *event_code, uint8_t *subevent_code);

/**
 * @brief Get length of event payload
 * @param event Event handler passed to the callback function
 * @param length pointer to size_t where to store the length
 * @return sl_status_t SL_STATUS_OK if success
 */
sl_status_t sl_btctrl_hci_event_get_length(struct sl_btctrl_hci_event *event, size_t *length);

/**
 * @brief Get event parameters
 * @param event Event handler passed to the callback function
 * @param destination pointer to destination buffer
 * @param destination_length length of the destination buffer
 * @param offset offset of where to read the event payload
 * @param length pointer to size_t where to store how many bytes have been copied to the buffer
 * @return sl_status_t SL_STATUS_OK if success
 */
sl_status_t sl_btctrl_hci_event_get_parameters(struct sl_btctrl_hci_event *event, uint8_t *destination, size_t destination_length, size_t offset, size_t *length);

#endif // SLI_BTCTRL_HCI_EVENT_H
