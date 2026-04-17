/********************************************************************************
 * @file  app_xapi.h
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

#ifndef APP_XAPI_H
#define APP_XAPI_H

#include <stdint.h>
#include <stddef.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Macros to declare deprecated functions */
#if defined(__IAR_SYSTEMS_ICC__)
#define APP_XAPI_DEPRECATED _Pragma("deprecated")
#elif defined(__GNUC__)
#define APP_XAPI_DEPRECATED __attribute__((deprecated))
#else
#define APP_XAPI_DEPRECATED
#endif

/* Compatibility */
#ifndef PACKSTRUCT
/*Default packed configuration*/
#ifdef __GNUC__
#ifdef _WIN32
#define PACKSTRUCT(decl) decl __attribute__((__packed__, gcc_struct))
#else
#define PACKSTRUCT(decl) decl __attribute__((__packed__))
#endif
#define ALIGNED __attribute__((aligned(0x4)))
#elif __IAR_SYSTEMS_ICC__

#define PACKSTRUCT(decl) __packed decl

#define ALIGNED
#elif _MSC_VER /*msvc*/

#define PACKSTRUCT(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#define ALIGNED
#else
#define PACKSTRUCT(a) a PACKED
#endif
#endif

/**
 * The maximum XAPI command payload size.
 */
#ifndef APP_XAPI_MAX_PAYLOAD_SIZE
#define APP_XAPI_MAX_PAYLOAD_SIZE 256
#endif

/***************************************************************************/ /**
 * @addtogroup app_xapi_types XAPI Types
 * @brief Common types in XAPI protocol
 * @{
 */

#ifndef APP_WIFI_TYPE_UINT8ARRARY
#define APP_WIFI_TYPE_UINT8ARRARY
/** @brief Variable-length uint8_t array. Maximum length: 255 */
typedef struct {
  uint8_t len;    /**< Number of bytes stored in @p data */
  uint8_t data[]; /**< Data bytes*/
} uint8array;
#endif

#ifndef APP_WIFI_TYPE_BYTE_ARRARY
#define APP_WIFI_TYPE_BYTE_ARRARY
/** @brief Variable-length int8_t array. Maximum length: 65535 */
typedef struct {
  uint16_t len;  /**< Number of bytes stored in @p data */
  int8_t data[]; /**< Data bytes*/
} byte_array;
#endif

#ifndef APP_WIFI_TYPE_BDADDR
#define APP_WIFI_TYPE_BDADDR
/** @brief Bluetooth address */
typedef struct {
  uint8_t addr[6]; /**< @brief Bluetooth address in reverse byte order */
} bd_addr;
#endif

#ifndef APP_WIFI_TYPE_MACADDR
#define APP_WIFI_TYPE_MACADDR
/** @brief MAC address */
typedef struct {
  uint8_t addr[6]; /**< @brief MAC Address*/
} mac_addr;
#endif

#ifndef APP_WIFI_TYPE_IPV4ADDR
#define APP_WIFI_TYPE_IPV4ADDR
/** @brief IPv4 address */
typedef struct {
  uint8_t addr[4]; /**< @brief IPv4 address */
} ipv4_addr;
#endif

#ifndef APP_WIFI_TYPE_IPV6ADDR
#define APP_WIFI_TYPE_IPV6ADDR
/** @brief IPv6 address */
typedef struct {
  uint8_t addr[16]; /**< @brief IPv6 address */
} ipv6_addr;
#endif

#ifndef APP_WIFI_TYPE_UUID128
#define APP_WIFI_TYPE_UUID128
/** @brief 128-bit UUID */
typedef struct {
  uint8_t data[16]; /**< 128-bit UUID */
} uuid_128;
#endif

#ifndef APP_WIFI_TYPE_AES_KEY128
#define APP_WIFI_TYPE_AES_KEY128
/** @brief 128-bit AES key */
typedef struct {
  uint8_t data[16]; /**< 128-bit AES key */
} aes_key_128;
#endif

#ifndef APP_WIFI_TYPE_UUID16
#define APP_WIFI_TYPE_UUID16
/** @brief 16-bit UUID */
typedef struct {
  uint8_t data[2]; /**< 16-bit UUID */
} app_wifi_uuid_16_t;
#endif

#ifndef APP_WIFI_TYPE_UUID64
#define APP_WIFI_TYPE_UUID64
/** @brief 64-bit UUID */
typedef struct {
  uint8_t data[8]; /**< 64-bit UUID */
} app_wifi_uuid_64_t;
#endif

/** @} */ // end addtogroup app_xapi_types
/******************************************************************************/

/** @brief Internal function prototype for XAPI command handlers */
typedef void (*app_xapi_handler)(const void *);

typedef enum app_xapi_msg_types {
  app_xapi_msg_type_cmd = 0x00,
  app_xapi_msg_type_rsp = 0x00,
  app_xapi_msg_type_evt = 0x80
} app_xapi_msg_types_t;

enum app_xapi_dev_types {
  app_xapi_dev_type_app        = 0x00,
  app_xapi_dev_type_bt         = 0x20,
  app_xapi_dev_type_btmesh     = 0x28,
  app_xapi_dev_type_wifi       = 0x38,
  app_xapi_dev_type_xapi_debug = 0x30,
};

/***************************************************************************/ /**
 * @addtogroup app_xapi_types XAPI Types
 * @brief Common types in XAPI protocol
 * @{
 */

/**
 * @brief The length of a XAPI message header which is 4 bytes
 */
#define APP_XAPI_MSG_HEADER_LEN (4)

/**
 * @brief The length of the XAPI message payload of a generic error response
 *
 * See function @ref app_xapi_set_error_response for how to generate a generic
 * error response.
 */
#define APP_XAPI_MSG_ERROR_PAYLOAD_LEN (2)

/**
 * @brief Get the device type of a XAPI message
 *
 * @param HDR The header of the message as a uint32_t integer
 */
#define APP_XAPI_MSG_DEVICE_TYPE(HDR) ((HDR)&0x38)

/**
 * @brief Get the identifier of a XAPI message including device type, class ID,
 * message type and message ID.
 *
 * @param HDR The header of the message as a uint32_t integer
 */
#define APP_XAPI_MSG_ID(HDR) ((HDR)&0xffff00f8)

/**
 * @brief Get the data payload length in a XAPI message.
 *
 * @param HDR The header of the message as a uint32_t integer
 */
#define APP_XAPI_MSG_LEN(HDR) ((((HDR)&0x7) << 8) | (((HDR)&0xff00) >> 8))

/**
 * @brief The bit indicating whether data of a XAPI message is encrypted
 */
#define APP_XAPI_BIT_ENCRYPTED (1 << 6) // Bit

/**
 * @brief Check whether data of a XAPI message is encrypted.
 *
 * @param HDR The XAPI header of the message as a uint32_t integer
 */
#define APP_XAPI_MSG_ENCRYPTED(HDR) ((HDR)&APP_XAPI_BIT_ENCRYPTED)

/** @} */ // end addtogroup app_xapi_types
/******************************************************************************/

/**
 * Internal function for setting the command handler function. Used by API library.
 *
 * @param cmd_handler_delegate Pointer to command handler delegate function
 */
void app_internal_xapi_set_cmd_handler_delegate(void (*cmd_handler_delegate)(uint32_t, app_xapi_handler, const void *));

/**
 * @addtogroup app_xapi_functions XAPI Functions
 * @{
 *
 * @brief Functions provided by the XAPI protocol
 */

/**
 * @brief Lock the XAPI for exclusive access.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * XAPI commands and responses in their binary format in an application that
 * uses an RTOS. Normal application code that issues XAPI commands by calling
 * API functions defined by protocol stacks must never call this function
 * directly.
 *
 * See the documentation of @ref app_xapi_handle_command for the full sequence
 * that must be followed when processing commands in their binary format.
 *
 * @return SL_STATUS_OK if the lock has been obtained, otherwise an error code
 */
sl_status_t app_xapi_lock(void);

/**
 * @brief Release the lock obtained by @ref app_xapi_lock
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * XAPI commands and responses in their binary format in an application that
 * uses an RTOS. Normal application code that issues XAPI commands by calling
 * API functions defined by protocol stacks must never call this function
 * directly.
 *
 * See the documentation of @ref app_xapi_handle_command for the full sequence
 * that must be followed when processing commands in their binary format.
 */
void app_xapi_unlock(void);

/**
 * @brief Handle a XAPI command in binary format.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * XAPI commands and responses in their binary format. Normal application code
 * that issues XAPI commands by calling API functions defined by protocol
 * stacks must never call this function directly.
 *
 * If the application uses an RTOS, the caller must protect the XAPI handling
 * by obtaining the XAPI lock with @ref app_xapi_lock, handle the command with
 * @ref app_xapi_handle_command, read the response from the buffer returned by
 * @ref app_xapi_get_command_response, and then release the lock with @ref
 * app_xapi_unlock. Here's an example of the full sequence that's required:
 *
 * @code
 * // Lock XAPI for exclusive access
 * sl_status_t status = app_xapi_lock();
 * if (status != SL_STATUS_OK) {
 *   // Locking will only fail if there are fatal unrecoverable errors with the
 *   // RTOS primitives, so caller may choose to just assert in case of errors.
 * }
 *
 * // Process the command
 * app_xapi_handle_command(hdr, data);
 *
 * // Read the response
 * void *rsp = app_xapi_get_command_response();
 * uint32_t rsp_header = *((uint32_t *)rsp);
 * size_t rsp_len = APP_XAPI_MSG_LEN(rsp_header) + APP_XAPI_MSG_HEADER_LEN;
 * // Send the `rsp_len` bytes of response starting from `rsp`
 *
 * // Finally unlock the XAPI to allow other commands to proceed
 * app_xapi_unlock();
 * @endcode
 *
 * Empty stub implementations are provided for @ref app_xapi_lock and @ref
 * app_xapi_unlock, so the same sequence can be used for all NCP/CPC
 * implementations even if an RTOS is not present.
 *
 * @param[in] hdr The XAPI command header
 * @param[in] data The payload data associated with the command
 */
void app_xapi_handle_command(uint32_t hdr, const void *data);

/**
 * @brief Get the response of a handled XAPI command.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * XAPI commands and responses in their binary format. Normal application code
 * that issues XAPI commands by calling API functions defined by protocol
 * stacks must never call this function directly.
 *
 * See the documentation of @ref app_xapi_handle_command for the full sequence
 * that must be followed when processing commands in their binary format.
 *
 * @return Pointer to the XAPI response structure that was filled when the
 *   command was executed in @ref app_xapi_handle_command.
 */
void *app_xapi_get_command_response(void);

/**
 * @brief Set a generic error response to the specified buffer.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * XAPI commands and responses in their binary format. Normal application code
 * that issues XAPI commands by calling API functions defined by protocol
 * stacks must never call this function directly.
 *
 * This function is available for NCP components that have detected fatal errors
 * in command processing (for example have failed to receive a complete command
 * message from the NCP host) and need to generate an error response without
 * going through the normal XAPI command processing.
 *
 * @param[in] command_hdr The header of the command that we are responding to.
 *   It is possible in certain types of failures that the NCP implementation
 *   does not even have the full command header. In these cases it is
 *   recommended that the NCP implementation sets the unavailable bytes of the
 *   header to value zero to avoid transmitting uninitialized bytes. XAPI
 *   commands are processed one command at a time and the recipient will be able
 *   to handle the error response even if it's missing the device ID, the class
 *   ID, or the command ID.
 *
 * @param[in] result The value to set to the @p result field of the error
 *   response.
 *
 * @param[out] response The response buffer to fill
 *
 * @param[in] response_buf_size The size of the response buffer. The caller must
 *   provide a buffer that has at least @ref APP_XAPI_MSG_HEADER_LEN + @ref
 *   APP_XAPI_MSG_ERROR_PAYLOAD_LEN bytes available.
 */
void app_xapi_set_error_response(uint32_t command_hdr, uint16_t result, void *response, size_t response_buf_size);

/** @} */ // end addtogroup app_xapi_functions

#ifdef __cplusplus
}
#endif

#endif
