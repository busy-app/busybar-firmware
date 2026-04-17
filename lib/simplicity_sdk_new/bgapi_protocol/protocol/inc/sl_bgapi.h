/***************************************************************************//**
 * @brief Silicon Labs BGAPI types and macros
 *******************************************************************************
 * # License
 * <b>Copyright 2019-2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BGAPI_H
#define SL_BGAPI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_bgapi_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Macros to declare deprecated functions */
#if defined(__IAR_SYSTEMS_ICC__)
  #define SL_BGAPI_DEPRECATED _Pragma("deprecated")
#elif defined(__GNUC__)
  #define SL_BGAPI_DEPRECATED __attribute__((deprecated))
#else
  #define SL_BGAPI_DEPRECATED
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
#elif _MSC_VER  /*msvc*/

#define PACKSTRUCT(decl) __pragma(pack(push, 1) ) decl __pragma(pack(pop) )
#define ALIGNED
#else
#define PACKSTRUCT(a) a PACKED
#endif
#endif

/***************************************************************************//**
 * @addtogroup sl_bgapi_types BGAPI Types
 * @brief Common types in BGAPI protocol
 * @{
 */

#ifndef SL_BT_TYPE_UINT8ARRARY
#define SL_BT_TYPE_UINT8ARRARY
/** @brief Variable-length uint8_t array. Maximum length: 255 */
typedef struct {
  uint8_t len;    /**< Number of bytes stored in @p data */
  uint8_t data[]; /**< Data bytes*/
} uint8array;
#endif

#ifndef SL_BT_TYPE_BYTE_ARRARY
#define SL_BT_TYPE_BYTE_ARRARY
/** @brief Variable-length uint8_t array. Maximum length: 65535 */
PACKSTRUCT(struct byte_array_s {
  uint16_t len;    /**< Number of bytes stored in @p data */
  uint8_t  data[]; /**< Data bytes*/
});
typedef struct byte_array_s byte_array;
#endif

#ifndef SL_BT_TYPE_BDADDR
#define SL_BT_TYPE_BDADDR
/** @brief Bluetooth address */
typedef struct {
  uint8_t addr[6]; /**< @brief Bluetooth address in reverse byte order */
} bd_addr;
#endif

#ifndef SL_BT_TYPE_UUID128
#define SL_BT_TYPE_UUID128
/** @brief 128-bit UUID */
typedef struct {
  uint8_t data[16]; /**< 128-bit UUID */
} uuid_128;
#endif

#ifndef SL_BT_TYPE_AES_KEY128
#define SL_BT_TYPE_AES_KEY128
/** @brief 128-bit AES key */
typedef struct {
  uint8_t data[16]; /**< 128-bit AES key */
} aes_key_128;
#endif

#ifndef SL_BT_TYPE_UUID16
#define SL_BT_TYPE_UUID16
/** @brief 16-bit UUID */
typedef struct {
  uint8_t data[2]; /**< 16-bit UUID */
} sl_bt_uuid_16_t;
#endif

#ifndef SL_BT_TYPE_UUID64
#define SL_BT_TYPE_UUID64
/** @brief 64-bit UUID */
typedef struct {
  uint8_t data[8]; /**< 64-bit UUID */
} sl_bt_uuid_64_t;
#endif

/** @} */ // end addtogroup sl_bgapi_types
/******************************************************************************/

/** @brief Internal function prototype for BGAPI command handlers */
typedef void (*sl_bgapi_handler)(const void*);

typedef enum sl_bgapi_msg_types {
  sl_bgapi_msg_type_cmd = 0x00,
  sl_bgapi_msg_type_rsp = 0x00,
  sl_bgapi_msg_type_evt = 0x80
} sl_bgapi_msg_types_t;

enum sl_bgapi_dev_types {
  sl_bgapi_dev_type_app = 0x00,
  sl_bgapi_dev_type_bt = 0x20,
  sl_bgapi_dev_type_btmesh = 0x28,
  sl_bgapi_dev_type_bgapi_service = 0x30,
};

/**
 * @brief BGAPI response structure for generic error responses.
 *
 * This structure is used to communicate a generic error response for a BGAPI
 * command when a command-specific response cannot be used. For example, if an
 * unknown command is received, the response will use this generic error
 * response structure. Every BGAPI command that returns has the result code
 * as the first field of the response.
 */
PACKSTRUCT(struct sl_bgapi_rsp_error_s{
  /** @brief The result code of the command */
  uint16_t result;
});

typedef struct sl_bgapi_rsp_error_s sl_bgapi_rsp_error_t;

/***************************************************************************//**
 * @addtogroup sl_bgapi_types BGAPI Types
 * @brief Common types in BGAPI protocol
 * @{
 */

/**
 * @brief The length of a BGAPI message header which is 4 bytes
 */
#define SL_BGAPI_MSG_HEADER_LEN (4)

/**
 * @brief The length of the BGAPI message payload of a generic error response
 *
 * See function @ref sl_bgapi_set_error_response for how to generate a generic
 * error response.
 */
#define SL_BGAPI_MSG_ERROR_PAYLOAD_LEN (2)

/**
 * @brief Get the message type of a BGAPI message
 *
 * @param HDR The header of the message as a uint32_t integer
 */
#define SL_BGAPI_MSG_TYPE(HDR) ((HDR) & 0x80)

/**
 * @brief Get the device type of a BGAPI message
 *
 * @param HDR The header of the message as a uint32_t integer
 */
#define SL_BGAPI_MSG_DEVICE_TYPE(HDR) ((HDR) & 0x38)

/**
 * @brief Get the identifier of a BGAPI message including device type, class ID,
 * message type and message ID.
 *
 * @param HDR The header of the message as a uint32_t integer
 */
#define SL_BGAPI_MSG_ID(HDR) ((HDR) & 0xffff00f8)

/**
 * @brief The maximum value the BGAPI header payload length field can store
 */
#define SL_BGAPI_HEADER_MAX_PAYLOAD_LEN (0x7FF)

// Verify that the the payload size configuration we see is within valid range
#if SL_BGAPI_MAX_PAYLOAD_SIZE > SL_BGAPI_HEADER_MAX_PAYLOAD_LEN
#error Invalid BGAPI payload configuration. SL_BGAPI_MAX_PAYLOAD_SIZE must not exceed 2047.
#endif

/**
 * @brief Get the data payload length in a BGAPI message.
 *
 * @param HDR The header of the message as a uint32_t integer
 */
#define SL_BGAPI_MSG_LEN(HDR) ((((HDR) & 0x7) << 8) | (((HDR) & 0xff00) >> 8))

/**
 * @brief The bit indicating whether data of a BGAPI message is encrypted
 */
#define SL_BGAPI_BIT_ENCRYPTED (1 << 6) // Bit

/**
 * @brief Check whether data of a BGAPI message is encrypted.
 *
 * @param HDR The BGAPI header of the message as a uint32_t integer
 */
#define SL_BGAPI_MSG_ENCRYPTED(HDR) ((HDR)&SL_BGAPI_BIT_ENCRYPTED)

/**
 * @brief Construct a BGAPI message header from an event ID and payload length.
 *
 * It is the caller's responsibility to verify that the input values are within
 * valid range.
 *
 * @param[in] evt_id The full event ID constant, for example
 *   `sl_bt_evt_system_boot_id`
 * @param[in] payload_len Length of the full BGAPI message payload
 *
 * @return The header as a `uint32_t` value
 */
#define SL_BGAPI_MSG_HEADER_FROM_ID_AND_LEN(evt_id, payload_len) \
  ((uint32_t) (((uint32_t) (evt_id))                             \
               | (((uint32_t) (payload_len) & 0x00FF) << 8)      \
               | (((uint32_t) (payload_len) & 0x0700) >> 8)))    \

/** @} */ // end addtogroup sl_bgapi_types
/******************************************************************************/

/**
 * @addtogroup sl_bgapi_functions BGAPI Functions
 * @{
 *
 * @brief Functions provided by the BGAPI protocol
 */

/**
 * @brief Obtain a buffer that can be used to execute BGAPI or user commands.
 *
 * This function is provided as a convenience for NCP/CPC components that need
 * to handle BGAPI or user commands and responses in their binary format. If the
 * user of @ref sl_bgapi_execute_binary_command already has memory available for
 * the command and response buffer (such as the memory allocated for UART RX/TX
 * buffers), the user is encouraged to use the already existing memory when
 * executing @ref sl_bgapi_execute_binary_command.
 *
 * If the user needs to allocate dedicated memory for any reason, using this
 * function for the allocation may reduce the overall memory usage depending on
 * the application configuration. If the configuration allows safe sharing of a
 * message buffer (this is the case in a single-threaded baremetal application),
 * the function returns the shared message buffer instead of making a separate
 * allocation. When an RTOS is used, the function allocates a dedicated buffer
 * for the caller.
 *
 * Callers that use this function must call @ref sl_bgapi_release_message_buffer
 * to release the message buffer when it is no longer needed.
 *
 * @param[in] max_payload_size The maximum payload size of the messages that the
 *   buffer will be used for. The value may not exceed the BGAPI configuration
 *   value SL_BGAPI_MAX_PAYLOAD_SIZE.
 *
 * @param[out] buffer Set to point to the buffer that the caller can use to
 *   execute BGAPI or user commands
 *
 * @return SL_STATUS_OK if the buffer was obtained, otherwise an error code
 */
sl_status_t sl_bgapi_obtain_message_buffer(size_t max_payload_size,
                                           void **buffer);

/**
 * @brief Release a buffer that was obtained with @ref sl_bgapi_obtain_message_buffer.
 *
 * @param[in] buffer The buffer to release
 */
void sl_bgapi_release_message_buffer(void *buffer);

/**
 * @brief Check if the specified command is marked as sensitive.
 *
 * This function is provided for NCP/CPC components that need to handle BGAPI
 * commands and responses in their binary format. If the NCP/CPC transport
 * supports encryption, it can use this function to determine if the specified
 * command is considered as sensitive and should be encrypted. If the execution
 * of a sensitive command is attempted over an unencrypted transport, the
 * NCP/CPC component can choose to reject the command.
 *
 * @param[in] command_header The header of the command to check
 * @param[out] is_sensitive Set to true if the command is considered sensitive,
 *   otherwise set to false
 *
 * @return SL_STATUS_OK if the sensitivity of the command was successfully
 *   determined, otherwise an error code
 */
sl_status_t sl_bgapi_check_command_sensitivity(uint32_t command_header,
                                               bool *is_sensitive);

/**
 * @brief Execute a BGAPI command in binary format.
 *
 * This function is provided for NCP/CPC components that need to handle BGAPI
 * commands and responses in their binary format. This function automatically
 * performs any locking or inter-process communication required to execute the
 * command. The function returns when the command has been executed.
 *
 * If the caller does not need the command buffer after executing the command,
 * it is acceptable to point the response buffer @p response_buf to the same
 * memory location as the command buffer @p command_buf. In this case the
 * response overwrites the command buffer. Provided that the response buffer is
 * large enough, this function guarantees that the response buffer is always
 * filled with a valid response message. If command execution fails, the
 * response buffer is filled with a generic error response.
 *
 * @param[in] command_buf Pointer to the full BGAPI command message including
 *   the header and the command-specific payload
 * @param[in] command_buf_size The size of the command buffer. This is used to
 *   verify that the command header is valid and the full command message is
 *   available.
 * @param[out] response_buf Pointer to the response buffer to fill. It is
 *   acceptable to point to the same buffer as the command buffer in @p
 *   command_buf.
 * @param[in] response_buf_size The size of the response buffer
 *
 * @return SL_STATUS_OK if the command was executed, otherwise an error code.
 *   Note that SL_STATUS_OK does not mean that the command was successful, only
 *   that it was executed.
 */
sl_status_t sl_bgapi_execute_binary_command(const void *command_buf,
                                            size_t command_buf_size,
                                            void *response_buf,
                                            size_t response_buf_size);

/**
 * @brief Lock the BGAPI for exclusive access.
 *
 * <b>Deprecated</b> and replaced by @ref sl_bgapi_execute_binary_command. The
 * replacement provides a more flexible and efficient way to execute BGAPI
 * commands in their binary format. The new function automatically performs any
 * locking that's needed and enables re-using the command buffer memory for the
 * response.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * BGAPI commands and responses in their binary format in an application that
 * uses an RTOS. Normal application code that issues BGAPI commands by calling
 * API functions defined by protocol stacks must never call this function
 * directly.
 *
 * See the documentation of @ref sl_bgapi_handle_command for the full sequence
 * that must be followed when processing commands in their binary format.
 *
 * @return SL_STATUS_OK if the lock has been obtained, otherwise an error code
 */
SL_BGAPI_DEPRECATED sl_status_t sl_bgapi_lock(void);

/**
 * @brief Release the lock obtained by @ref sl_bgapi_lock
 *
 * <b>Deprecated</b> and replaced by @ref sl_bgapi_execute_binary_command. The
 * replacement provides a more flexible and efficient way to execute BGAPI
 * commands in their binary format. The new function automatically performs any
 * locking that's needed and enables re-using the command buffer memory for the
 * response.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * BGAPI commands and responses in their binary format in an application that
 * uses an RTOS. Normal application code that issues BGAPI commands by calling
 * API functions defined by protocol stacks must never call this function
 * directly.
 *
 * See the documentation of @ref sl_bgapi_handle_command for the full sequence
 * that must be followed when processing commands in their binary format.
 */
SL_BGAPI_DEPRECATED void sl_bgapi_unlock(void);

/**
 * @brief Handle a BGAPI command in binary format.
 *
 * <b>Deprecated</b> and replaced by @ref sl_bgapi_execute_binary_command. The
 * replacement provides a more flexible and efficient way to execute BGAPI
 * commands in their binary format. The new function automatically performs any
 * locking that's needed and enables re-using the command buffer memory for the
 * response.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * BGAPI commands and responses in their binary format. Normal application code
 * that issues BGAPI commands by calling API functions defined by protocol
 * stacks must never call this function directly.
 *
 * If the application uses an RTOS, the caller must protect the BGAPI handling
 * by obtaining the BGAPI lock with @ref sl_bgapi_lock, handle the command with
 * @ref sl_bgapi_handle_command, read the response from the buffer returned by
 * @ref sl_bgapi_get_command_response, and then release the lock with @ref
 * sl_bgapi_unlock. Here's an example of the full sequence that's required:
 *
 * @code
 * // Lock BGAPI for exclusive access
 * sl_status_t status = sl_bgapi_lock();
 * if (status != SL_STATUS_OK) {
 *   // Locking will only fail if there are fatal unrecoverable errors with the
 *   // RTOS primitives, so caller may choose to just assert in case of errors.
 * }
 *
 * // Process the command
 * sl_bgapi_handle_command(hdr, data);
 *
 * // Read the response
 * void *rsp = sl_bgapi_get_command_response();
 * uint32_t rsp_header = *((uint32_t *)rsp);
 * size_t rsp_len = SL_BGAPI_MSG_LEN(rsp_header) + SL_BGAPI_MSG_HEADER_LEN;
 * // Send the `rsp_len` bytes of response starting from `rsp`
 *
 * // Finally unlock the BGAPI to allow other commands to proceed
 * sl_bgapi_unlock();
 * @endcode
 *
 * Empty stub implementations are provided for @ref sl_bgapi_lock and @ref
 * sl_bgapi_unlock, so the same sequence can be used for all NCP/CPC
 * implementations even if an RTOS is not present.
 *
 * @param[in] hdr The BGAPI command header
 * @param[in] data The payload data associated with the command
 */
SL_BGAPI_DEPRECATED void sl_bgapi_handle_command(uint32_t hdr, const void* data);

/**
 * @brief Get the response of a handled BGAPI command.
 *
 * <b>Deprecated</b> and replaced by @ref sl_bgapi_execute_binary_command. The
 * replacement provides a more flexible and efficient way to execute BGAPI
 * commands in their binary format. The new function automatically performs any
 * locking that's needed and enables re-using the command buffer memory for the
 * response.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * BGAPI commands and responses in their binary format. Normal application code
 * that issues BGAPI commands by calling API functions defined by protocol
 * stacks must never call this function directly.
 *
 * See the documentation of @ref sl_bgapi_handle_command for the full sequence
 * that must be followed when processing commands in their binary format.
 *
 * @return Pointer to the BGAPI response structure that was filled when the
 *   command was executed in @ref sl_bgapi_handle_command.
 */
SL_BGAPI_DEPRECATED void* sl_bgapi_get_command_response(void);

/**
 * @brief Set a generic error response to the specified buffer.
 *
 * NOTE: This function is provided for NCP/CPC components that need to handle
 * BGAPI commands and responses in their binary format. Normal application code
 * that issues BGAPI commands by calling API functions defined by protocol
 * stacks must never call this function directly.
 *
 * This function is available for NCP components that have detected fatal errors
 * in command processing (for example have failed to receive a complete command
 * message from the NCP host) and need to generate an error response without
 * going through the normal BGAPI command processing.
 *
 * @param[in] command_hdr The header of the command that we are responding to.
 *   It is possible in certain types of failures that the NCP implementation
 *   does not even have the full command header. In these cases it is
 *   recommended that the NCP implementation sets the unavailable bytes of the
 *   header to value zero to avoid transmitting uninitialized bytes. BGAPI
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
 *   provide a buffer that has at least @ref SL_BGAPI_MSG_HEADER_LEN + @ref
 *   SL_BGAPI_MSG_ERROR_PAYLOAD_LEN bytes available.
 */
void sl_bgapi_set_error_response(uint32_t command_hdr,
                                 uint16_t result,
                                 void *response,
                                 size_t response_buf_size);

/** @} */ // end addtogroup sl_bgapi_functions

#ifdef __cplusplus
}
#endif

#endif
