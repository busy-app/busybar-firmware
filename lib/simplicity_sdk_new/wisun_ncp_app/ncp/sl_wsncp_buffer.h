/***************************************************************************//**
 * @file sl_wsncp_buffer.h
 * @brief Wi-SUN NCP Fixed Buffer Management
 * 
 * BUFFER SIZING GUIDE:
 * ===================
 * 
 * The NCP buffer size (SL_WSNCP_BUFFER_SIZE) must be configured to accommodate:
 * 
 * 1. Wi-SUN Message Header (4 bytes)
 * 2. Message Body (varies by message type)
 * 3. Variable Data (if applicable)
 *    - Socket data (according to the MTU/MRU of the transport layer)
 *    - Certificates
 *    - Neighbor lists: depends on network size
 * 
 * CONFIGURATION:
 * Set SL_WSNCP_BUFFER_SIZE in your .slcp file based on your needs.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WSNCP_BUFFER_H
#define SL_WSNCP_BUFFER_H

#include "assert.h"
#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_wisun_types.h"
#include "sl_wisun_common.h"
#include "sl_wisun_msg_api.h"

// STRINGIFY macro for compile-time string conversion
#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

// Configuration-aware defines - use .slcp values if available, otherwise use defaults
#ifndef SL_WSNCP_BUFFER_SIZE
#define SL_WSNCP_BUFFER_SIZE           1500
#endif

// Minimum buffer size required for Wi-SUN message header
#define SL_WSNCP_MIN_BUFFER_SIZE sizeof(sl_wisun_msg_header_t)

// Static assertion to ensure buffer size is at least the header size
_Static_assert(SL_WSNCP_BUFFER_SIZE >= SL_WSNCP_MIN_BUFFER_SIZE, 
               "SL_WSNCP_BUFFER_SIZE must be at least " STRINGIFY(SL_WSNCP_MIN_BUFFER_SIZE) " bytes");

// Buffer state enumeration - simplified for NCP use case
// Note: No ERROR state - invalid messages are dropped immediately
typedef enum {
    SL_WSNCP_BUFFER_STATE_EMPTY,      // Buffer is empty and ready for new data
    SL_WSNCP_BUFFER_STATE_COMPLETE    // Buffer contains complete message
} sl_wsncp_buffer_state_t;

/**
 * @brief Wi-SUN NCP Fixed Buffer Structure
 * 
 * This structure provides a fixed-size buffer for NCP message handling.
 * The buffer size is configured at compile time via SL_WSNCP_BUFFER_SIZE.
 * 
 * Buffer Layout:
 * [Wi-SUN NCP Message Header][Wi-SUN NCP Message Body][Variable Data]
 * 
 * Total size = SL_WSNCP_BUFFER_SIZE bytes
 */
typedef struct {
    uint16_t length;                     // Current data length
    sl_wsncp_buffer_state_t state;       // Current buffer state
    uint8_t data[SL_WSNCP_BUFFER_SIZE];  // Simple fixed-size buffer
} sl_wsncp_buffer_t;


/**
 * @brief Initialize a message buffer
 *
 * @param[in,out] buffer Pointer to the buffer to initialize
 * @retval SL_STATUS_OK Buffer initialized successfully
 * @retval SL_STATUS_NULL_POINTER Buffer pointer is NULL
 */
sl_status_t sl_wsncp_buffer_init(sl_wsncp_buffer_t *buffer);

/**
 * @brief Add data to a message buffer
 *
 * @param[in,out] buffer Pointer to the buffer to add data to
 * @param[in] len Length of data to add
 * @param[in] data Pointer to data to add
 * @param[in] on_complete_callback Callback function to call when message is complete (can be NULL)
 * @retval SL_STATUS_OK Data added successfully
 * @retval SL_STATUS_NULL_POINTER Buffer or data pointer is NULL
 * @retval SL_STATUS_WOULD_OVERFLOW Message too large - dropped
 * @retval SL_STATUS_INVALID_PARAMETER Invalid message - dropped
 * @retval SL_STATUS_INVALID_STATE Buffer not in empty state
 * 
 * Note: Invalid messages are dropped immediately and buffer stays in EMPTY state
 */
sl_status_t sl_wsncp_buffer_add_data(sl_wsncp_buffer_t *buffer,
                                     uint16_t len,
                                     const uint8_t *data,
                                     void (*on_complete_callback)(void));

/**
 * @brief Clear a message buffer
 *
 * @param[in,out] buffer Pointer to the buffer to clear
 * @retval SL_STATUS_OK Buffer cleared successfully
 * @retval SL_STATUS_NULL_POINTER Buffer pointer is NULL
 */
sl_status_t sl_wsncp_buffer_clear(sl_wsncp_buffer_t *buffer);

/**
 * @brief Get buffer state
 *
 * @param[in] buffer Pointer to the buffer
 * @return Current buffer state (returns EMPTY if buffer is NULL)
 */
sl_wsncp_buffer_state_t sl_wsncp_buffer_get_state(const sl_wsncp_buffer_t *buffer);



#endif // SL_WSNCP_BUFFER_H