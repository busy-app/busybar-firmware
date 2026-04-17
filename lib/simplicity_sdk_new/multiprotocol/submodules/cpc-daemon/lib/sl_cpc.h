/***************************************************************************//**
 * @file
 * @brief Co-Processor Communication Protocol(CPC) - Library Header
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_CPC_H
#define SL_CPC_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#if !defined(__linux__)
#error Wrong platform - this header file is intended for Linux applications that use libcpc
#endif

#ifdef DOXYGEN
#define SL_ENUM(name) enum name
#define SL_ENUM_GENERIC(name, type) enum name
#else
#define SL_ENUM(name) typedef uint8_t name; enum name##_enum
#define SL_ENUM_GENERIC(name, type) typedef type name; enum name##_enum
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************//**
 * @addtogroup cpc libcpc
 * @brief libcpc API
 * @details
 * ## Overview
 *
 *  The CPC library provides an interface through which an application can
 *  connect to the CPC daemon (CPCd), and manage endpoints used to communicate
 *  with a CPC secondary.
 *
 * ## Initialization
 *
 *  The application must first initialize the CPC library by calling `cpc_init()`.
 *  On success, this function will return a handle which is used for subsequent
 *  API calls.
 *
 * ## CPC Endpoints
 *
 *  Each endpoint represents a channel of communication between the host application
 *  and the secondary. Some endpoints are reserved; available user endpoints are
 *  enumerated in #sl_cpc_user_endpoint_id_t. @n
 *  Before reading from or writing to an endpoint, it must first be successfully
 *  opened with a call to `cpc_open_endpoint()`. Note that the secondary must have
 *  opened its endpoint for the call to succeed. @n
 *  The state of an endpoint can be queried with the function `cpc_get_endpoint_state()`. @n
 *  Options can be set on an open endpoint through the function `cpc_set_endpoint_option()`,
 *  and queried with `cpc_get_endpoint_option()`.
 *
 * ## Connection Management
 *
 * The application can register a callback of type #cpc_reset_callback_t when it calls
 * `cpc_init()`. If the library is not connected to a daemon anymore, all connected clients
 * will have their callbacks called. The callback will execute in the context of a separate thread.
 * The application can call `cpc_restart()` to reconnect to the daemon. The endpoints
 * must be closed and re-configured as well. @n
 *
 *  * must be closed, re-opened and re-configured as well.
 *
 * `cpc_restart()` is not a re-entrant API and should not be called from within the reset callback.
 *
 * The recommended usage of the reset callback is to set a flag that will then notify
 * your application to call `cpc_restart`.
 *
 * ## Example
 *  @code{.c}
 *
 *  static cpc_handle_t cpc_handle;
 *  static cpc_endpoint_t endpoint_0;
 *  static bool secondary_reset = false;
 *
 *  // if the secondary resets unexpectedly, the callback will execute in the context of a separate thread.
 *  // CPC API calls are not allowed in this context since they are non-reentrant by design.
 *  void reset_callback(void)
 *  {
 *    // the application thread can monitor this flag for reset events
 *    secondary_reset = true;
 *  }
 *
 *  void open_endpoint(void)
 *  {
 *    // open user endpoint. The endpoint must have already been opened by
 *    // the secondary. The file descriptor that is returned can be passed
 *    // to select() or epoll(), for example.
 *    fd = cpc_open_endpoint(cpc_handle, &endpoint_0, SL_CPC_ENDPOINT_USER_ID_0, 1);
 *    assert(fd > 0);
 *
 *    // set the endpoint to blocking for read and write
 *    ret = cpc_set_endpoint_option(endpoint_0, CPC_OPTION_BLOCKING, true, sizeof(bool));
 *    assert(ret >= 0);
 *
 *    // optionally set a timeout for write operations
 *    ret = cpc_set_endpoint_option(endpoint_0, CPC_OPTION_TX_TIMEOUT, timeout, sizeof(cpc_timeval_t));
 *    assert(ret >= 0);
 *  }
 *
 *  void app(void)
 *  {
 *    int ret;
 *    int fd;
 *    ssize_t bytes;
 *    cpc_timeval_t timeout = {1, 0};
 *    const char *tx_buffer = "Hello World";
 *    char rx_buffer[SL_CPC_READ_MINIMUM_SIZE];
 *
 *    // connect to daemon
 *    do {
 *      ret = cpc_init(&cpc_handle, NULL, true, reset_callback);
 *      sleep(1);
 *    } while (ret != 0 );
 *
 *    while (1) {
 *
 *      // Detect if secondary disconnected
 *      if (secondary_reset) {
 *        secondary_reset = false;
 *        ret = cpc_close_endpoint(&endpoint_0);
 *        assert(ret == 0);
 *        do {
 *          // Attempt to reconnect to daemon
 *          ret = cpc_restart(&cpc_handle);
 *        } while (ret != 0 );
 *        open_endpoint();
 *      }
 *
 *      bytes = cpc_write_endpoint(endpoint_0, (const void *)tx_buffer, strlen(buffer), CPC_ENDPOINT_WRITE_FLAG_NONE);
 *      assert(bytes == strlen(buffer));
 *
 *      // As no timeout is set for reception, the read will block indefinitely.
 *      // If the secondary resets, the -ECONNRESET error will be returned.
 *      bytes = cpc_read_endpoint(endpoint_0, (void *)rx_buffer, SL_CPC_READ_MINIMUM_SIZE, CPC_ENDPOINT_READ_FLAG_NONE);
 *      assert(bytes >=  0 || (secondary_reset && bytes == -ECONNRESET));
 *    }
 *  }
 *
 * @endcode
 * @{
 ******************************************************************************/

#define SL_CPC_READ_MINIMUM_SIZE 4087

/// @brief Enumeration representing the possible endpoint state.
SL_ENUM(cpc_endpoint_state_t){
  SL_CPC_STATE_OPEN = 0,                      ///< State open
  SL_CPC_STATE_CLOSED,                        ///< State closed
  SL_CPC_STATE_CLOSING,                       ///< State closing
  SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE, ///< Error state, destination unreachable
  SL_CPC_STATE_ERROR_SECURITY_INCIDENT,       ///< Error state, security incident
  SL_CPC_STATE_ERROR_FAULT                    ///< Error state, fault
};

/// @brief Enumeration representing the possible write endpoint flags.
SL_ENUM(cpc_endpoint_write_flags_t){
  CPC_ENDPOINT_WRITE_FLAG_NONE = 0,                 ///< No flag
  CPC_ENDPOINT_WRITE_FLAG_NON_BLOCKING = (1 << 0)   ///< Set this transaction as non-blocking
};

/// @brief Enumeration representing the possible read endpoint flags.
SL_ENUM(cpc_endpoint_read_flags_t){
  CPC_ENDPOINT_READ_FLAG_NONE = 0,                  ///< No flag
  CPC_ENDPOINT_READ_FLAG_NON_BLOCKING = (1 << 0)    ///< Set this transaction as non-blocking
};

/// @brief Enumeration representing the possible event endpoint flags.
SL_ENUM(cpc_endpoint_event_flags_t){
  CPC_ENDPOINT_EVENT_FLAG_NONE = 0,                 ///< No flag
  CPC_ENDPOINT_EVENT_FLAG_NON_BLOCKING = (1 << 0)   ///< Set this transaction as non-blocking
};

/// @brief Enumeration representing the possible configurable options for an endpoint.
SL_ENUM(cpc_option_t){
  CPC_OPTION_NONE = 0,        ///< Option none
  CPC_OPTION_BLOCKING,        ///< Option blocking
  CPC_OPTION_RX_TIMEOUT,      ///< Option read timeout
  CPC_OPTION_TX_TIMEOUT,      ///< Option write timeout
  CPC_OPTION_SOCKET_SIZE,     ///< Option socket size
  CPC_OPTION_MAX_WRITE_SIZE,  ///< Option maximum socket write size
  CPC_OPTION_ENCRYPTED        ///< Option encryption state
};

/// @brief Enumeration representing the possible configurable options for an endpoint event handler.
SL_ENUM(cpc_endpoint_event_option_t){
  CPC_ENDPOINT_EVENT_OPTION_NONE = 0,          ///< Option none
  CPC_ENDPOINT_EVENT_OPTION_BLOCKING,          ///< Option blocking
  CPC_ENDPOINT_EVENT_OPTION_READ_TIMEOUT,      ///< Option read timeout
};

/// @brief Enumeration representing service endpoint.
SL_ENUM(sl_cpc_service_endpoint_id_t){
  SL_CPC_ENDPOINT_SYSTEM = 0,                  ///< System control
  SL_CPC_ENDPOINT_SECURITY = 1,                ///< Security - related functionality
  SL_CPC_ENDPOINT_BLUETOOTH = 2,               ///< Bluetooth (BGAPI) endpoint
  SL_CPC_SLI_CPC_ENDPOINT_RAIL_DOWNSTREAM = 3, ///< RAIL downstream endpoint
  SL_CPC_SLI_CPC_ENDPOINT_RAIL_UPSTREAM = 4,   ///< RAIL upstream endpoint
  SL_CPC_ENDPOINT_ZIGBEE = 5,                  ///< ZigBee EZSP endpoint
  SL_CPC_ENDPOINT_ZWAVE = 6,                   ///< Z-Wave endpoint
  SL_CPC_ENDPOINT_CONNECT = 7,                 ///< Connect endpoint
  SL_CPC_ENDPOINT_GPIO = 8,                    ///< GPIO endpoint for controlling GPIOs on SECONDARYs
  SL_CPC_ENDPOINT_OPENTHREAD = 9,              ///< Openthread Spinel endpoint
  SL_CPC_ENDPOINT_WISUN = 10,                  ///< WiSun endpoint
  SL_CPC_ENDPOINT_WIFI = 11,                   ///< WiFi endpoint(main control)
  SL_CPC_ENDPOINT_15_4 = 12,                   ///< 802.15.4 endpoint
  SL_CPC_ENDPOINT_CLI = 13,                    ///< Ascii based CLI for stacks / applications
  SL_CPC_ENDPOINT_BLUETOOTH_RCP = 14,          ///< Bluetooth RCP endpoint
  SL_CPC_ENDPOINT_ACP = 15,                    ///< ACP endpoint
  SL_CPC_ENDPOINT_SE = 16,                     ///< Secure Engine endpoint
  SL_CPC_ENDPOINT_NVM3 = 17,                   ///< NVM3 endpoint
};

/// @brief Enumeration representing user endpoint.
SL_ENUM(sl_cpc_user_endpoint_id_t){
  SL_CPC_ENDPOINT_USER_ID_0 = 90, ///< User endpoint ID 0
  SL_CPC_ENDPOINT_USER_ID_1 = 91, ///< User endpoint ID 1
  SL_CPC_ENDPOINT_USER_ID_2 = 92, ///< User endpoint ID 2
  SL_CPC_ENDPOINT_USER_ID_3 = 93, ///< User endpoint ID 3
  SL_CPC_ENDPOINT_USER_ID_4 = 94, ///< User endpoint ID 4
  SL_CPC_ENDPOINT_USER_ID_5 = 95, ///< User endpoint ID 5
  SL_CPC_ENDPOINT_USER_ID_6 = 96, ///< User endpoint ID 6
  SL_CPC_ENDPOINT_USER_ID_7 = 97, ///< User endpoint ID 7
  SL_CPC_ENDPOINT_USER_ID_8 = 98, ///< User endpoint ID 8
  SL_CPC_ENDPOINT_USER_ID_9 = 99, ///< User endpoint ID 9
};

/// @brief Enumeration representing possible event types.
SL_ENUM(cpc_event_type_t){
  SL_CPC_EVENT_ENDPOINT_UNKNOWN = 0,
  SL_CPC_EVENT_ENDPOINT_OPENED = 1,
  SL_CPC_EVENT_ENDPOINT_CLOSED = 2,
  SL_CPC_EVENT_ENDPOINT_CLOSING = 3,
  SL_CPC_EVENT_ENDPOINT_ERROR_DESTINATION_UNREACHABLE = 4,
  SL_CPC_EVENT_ENDPOINT_ERROR_SECURITY_INCIDENT = 5,
  SL_CPC_EVENT_ENDPOINT_ERROR_FAULT = 6,
};

/// @brief Struct representing a CPC library handle.
typedef struct {
  void *ptr; ///< void pointer.
} cpc_handle_t;

/// @brief Struct representing a CPC endpoint handle.
typedef struct {
  void *ptr; ///< void pointer.
} cpc_endpoint_t;

/// @brief Struct representing a CPC event handle.
typedef struct {
  void *ptr; ///< void pointer.
} cpc_endpoint_event_handle_t;

/// @brief Struct for configuring time options of endpoints
typedef struct {
  int seconds;      ///< Number of seconds
  int microseconds; ///< Number of microseconds
} cpc_timeval_t;

/***************************************************************************//**
 * @brief Callback to notify the application that the secondary has crashed/reset itself.
 *
 * @warning This callback is called in a separate thread. The user must not call
 *          any CPC API in this context.
 ******************************************************************************/
typedef void (*cpc_reset_callback_t) (void);

/***************************************************************************//**
 * @brief Callback to notify the application that an endpoint has changed state
 ******************************************************************************/
typedef void (*cpc_endpoint_state_callback_t) (uint8_t endpoint_id, cpc_endpoint_state_t endpoint_state);

/***************************************************************************//**
 * @brief Initialize the CPC library.
 *        Upon success the user will get a handle that must be passed to subsequent calls.
 *
 * @param[out] handle           CPC library handle
 * @param[in]  instance_name    The name of the daemon instance. It will be the value of the instance_name in the config file of the daemon.
 *                              This value can be NULL, and so the default "cpcd_0" value will be used. If running a single instance, this can
 *                              be left to NULL, but when running simultaneous instances, it will need to be supplied.
 * @param[in]  enable_tracing   Enable tracing over stderr
 * @param[in]  reset_callback   Optional callback for when the library is not connected to a daemon anymore.
 *                              The callback will execute in the context of a separate thread.
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 *
 * @note Only one libcpc instance per process is allowed
 *
 ******************************************************************************/
int cpc_init(cpc_handle_t *handle, const char *instance_name, bool enable_tracing, cpc_reset_callback_t reset_callback);

/***************************************************************************//**
 * @brief Restart the CPC library.
 *        The user is notified via the 'reset_callback' when the secondary has restarted.
 *        The user logic then has to call this function in order to [try] to re-connect
 *        the application to the daemon.
 *
 * @param[out] handle           CPC library handle
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 ******************************************************************************/
int cpc_restart(cpc_handle_t *handle);

/***************************************************************************//**
 * @brief Connect to the socket corresponding to the provided endpoint ID.
 *        The function will also allocate the memory for the endpoint structure and assign
 *        it to the provided pointer.
 *        This endpoint structure must then be used for further calls to the libcpc.
 *
 * @param[in]  handle           CPC library handle
 * @param[out] endpoint         CPC endpoint handle to open
 * @param[in]  id               CPC endpoint id to open
 * @param[in]  tx_window_size   CPC transmit window (only a window of 1 is supported at the moment)
 *
 * @return On error, a negative value of errno is returned.
 *         On success, the file descriptor of the socket is returned.
 ******************************************************************************/
int cpc_open_endpoint(cpc_handle_t handle, cpc_endpoint_t *endpoint, uint8_t id, uint8_t tx_window_size);

/***************************************************************************//**
 * @brief Close the socket connection to the endpoint.
 *        This function will also free the memory used to allocate the endpoint structure.
 *
 * @param[in] endpoint         CPC endpoint handle to close
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 ******************************************************************************/
int cpc_close_endpoint(cpc_endpoint_t *endpoint);

/***************************************************************************//**
 * @brief Attempt to read up to count bytes from a previously-opened endpoint socket.
 *        Once data is received, it will be copied to the user-provided buffer.
 *        The lifecycle of this buffer is handled by the user.
 *
 *        By default the cpc_read function will block indefinitely.
 *        A timeout can be configured with cpc_set_endpoint_option.
 *
 * @param[in] endpoint         CPC endpoint handle to read from
 * @param[out] buffer          The buffer to which the data will be copied to.
 *                             The buffer must be at least SL_CPC_READ_MINIMUM_SIZE bytes
 *                             long to ensure a complete packet reception.
 * @param[in] count            The number of bytes to copy to that buffer.
 *                             Count must be at least 4087.
 * @param[in] flags            Optional read flags
 *
 * @return On error, a negative value of errno is returned.
 *         On success, the function returns the amount of bytes that have been read.
 *
 * @note Flags are enumerated in #cpc_read_endpoint_flags_t
 *       - CPC_ENDPOINT_READ_FLAG_NONE
 *       - CPC_ENDPOINT_READ_FLAG_NON_BLOCKING
 ******************************************************************************/
ssize_t cpc_read_endpoint(cpc_endpoint_t endpoint, void *buffer, size_t count, cpc_endpoint_read_flags_t flags);

/***************************************************************************//**
 * @brief Write data to an open endpoint.
 *
 * @param[in] endpoint         CPC endpoint handle to write to
 * @param[in] data             The data to write on the CPC endpoint
 * @param[in] data_length      The length of the data to write on the CPC endpoint
 * @param[in] flags            Optional write flags
 *
 * @return On error, a negative value of errno is returned.
 *         On success, the function returns the amount of bytes that have been written.
 *
 * @note A successful write will always return the number of bytes that was requested. Partial writes
 *       are impossible.
 *       Flags are enumerated in #cpc_write_endpoint_flags_t
 *       - CPC_ENDPOINT_WRITE_FLAG_NONE
 *       - CPC_ENDPOINT_WRITE_FLAG_NON_BLOCKING
 ******************************************************************************/
ssize_t cpc_write_endpoint(cpc_endpoint_t endpoint, const void *data, size_t data_length, cpc_endpoint_write_flags_t flags);

/***************************************************************************//**
 * @brief Get the state of an endpoint by ID.
 *
 * @param[in]  handle          CPC library handle
 * @param[in]  id              The id from which to obtain the endpoint state
 * @param[out] state           The state of the provided CPC endpoint
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 *
 * @note States are enumerated in #cpc_endpoint_state_t
 *       - SL_CPC_STATE_OPEN
 *       - SL_CPC_STATE_CLOSED
 *       - SL_CPC_STATE_CLOSING
 *       - SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE
 *       - SL_CPC_STATE_ERROR_SECURITY_INCIDENT
 *       - SL_CPC_STATE_ERROR_FAULT
 ******************************************************************************/
int cpc_get_endpoint_state(cpc_handle_t handle, uint8_t id, cpc_endpoint_state_t *state);

/***************************************************************************//**
 * @brief Configure an endpoint with a specified option.
 *
 * @param[in] endpoint         CPC endpoint handle
 * @param[in] option           The option to configure
 * @param[in] optval           The value of the option
 * @param[in] optlen           The length of the value of the option
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 *
 * @note This API is not recommended, please use the safer set function calls available for each options
 *
 *      Options are as follows:
 *       - CPC_OPTION_RX_TIMEOUT:   Set a timeout for the read transaction. Optval
 *                                  must be a cpc_timeval_t.
 *       - CPC_OPTION_TX_TIMEOUT:   Set a timeout for the write transaction. Optval
 *                                  must be a cpc_timeval_t.
 *       - CPC_OPTION_BLOCKING:     Set every transactions (read or write) as blocking or not, optval
 *                                  is a boolean.
 *       - CPC_OPTION_SOCKET_SIZE:  Set the buffer size for the socket used to write on an endpoint.
 *                                  Optval is an integer. The kernel doubles this value (to allow space for
 *                                  bookkeeping overhead).
 ******************************************************************************/
int cpc_set_endpoint_option(cpc_endpoint_t endpoint, cpc_option_t option, const void *optval, size_t optlen);

/***************************************************************************//**
 * @brief Get the option configured for a specified endpoint.
 *
 * @param[in] endpoint         CPC endpoint handle
 * @param[in] option           The option to get
 * @param[out] optval          The value of the option
 * @param[in,out] optlen       The length of the value of the option optlen is a value-result argument,
 *                             initially containing the size of the buffer pointed to by optval,
 *                             and modified on return to indicate the actual size of the value returned.
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 *
 * @note This API is not recommended, please use the safer get function calls available for each options
 *
 *     Options are as follows:
 *       - CPC_OPTION_RX_TIMEOUT:     Get the timeout for the read transaction. Optval
 *                                    must be a cpc_timeval_t.
 *       - CPC_OPTION_TX_TIMEOUT:     Get the timeout for the write transaction. Optval
 *                                    must be a cpc_timeval_t.
 *       - CPC_OPTION_BLOCKING:       Get the socket access mode, optval is a boolean.
 *       - CPC_OPTION_SOCKET_SIZE:    Get the buffer size for the socket used to write on an endpoint.
 *                                    Optval is an integer.
 *       - CPC_OPTION_MAX_WRITE_SIZE: Get the maximum size of the payload that will can be written
 *                                    on an endpoint. Optval is an integer.
 *       - CPC_OPTION_ENCRYPTED:      True if the communication is encrypted. Optval is a boolean.
 ******************************************************************************/
int cpc_get_endpoint_option(cpc_endpoint_t endpoint, cpc_option_t option, void *optval, size_t *optlen);

/***************************************************************************//**
 * @brief Get the version of the library
 *
 * @return A character pointer pointing to the version string is returned
 ******************************************************************************/
const char* cpc_get_library_version(void);

/***************************************************************************//**
 * @brief Get the version of secondary application
 *
 * @param[in]  handle                         CPC library handle
 *
 * @return On error, NULL is returned.
 *         On success, an owned copy of the string is returned.
 *         This copy can be freed using cpc_free_secondary_app_version(copy).
 ******************************************************************************/
const char* cpc_get_secondary_app_version(cpc_handle_t handle);

/***************************************************************************//**
 * @brief Free the version of secondary application
 *
 * @param[in]  secondary_app_version          Secondary app version
 *
 * @return On error, -EINVAL is returned.
 *         On success, 0 is returned.
 ******************************************************************************/
int cpc_free_secondary_app_version(char *secondary_app_version);

/***************************************************************************//**
 * @brief Set the timeout for the endpoint read operations
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  timeval     Requested timeout
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_set_endpoint_read_timeout(cpc_endpoint_t endpoint, cpc_timeval_t timeval);

/***************************************************************************//**
 * @brief Get the timeout for the endpoint read operations
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  timeval     Pointer to the requested timeout
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_read_timeout(cpc_endpoint_t endpoint, cpc_timeval_t *timeval);

/***************************************************************************//**
 * @brief Set the timeout for the endpoint write operations
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  timeval     Requested timeout
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_set_endpoint_write_timeout(cpc_endpoint_t endpoint, cpc_timeval_t timeval);

/***************************************************************************//**
 * @brief Get the timeout for the endpoint read operations
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  timeval     Requested timeout
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_write_timeout(cpc_endpoint_t endpoint, cpc_timeval_t *timeval);

/***************************************************************************//**
 * @brief Select the blocking mode for the endpoint read and write operations
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  blocking    Boolean value whether to block or not
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_set_endpoint_blocking(cpc_endpoint_t endpoint, bool blocking);

/***************************************************************************//**
 * @brief Get the blocking mode for the endpoint read and write operations
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  is_blocking Boolean value whether to block or not
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_blocking_mode(cpc_endpoint_t endpoint, bool *is_blocking);

/***************************************************************************//**
 * @brief Set the buffer size for the socket used to write on an endpoint.
 *        The kernel doubles this value (to allow space for bookkeeping overhead).
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  socket_size The socket size represented as an integer
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_set_endpoint_socket_size(cpc_endpoint_t endpoint, uint32_t socket_size);

/***************************************************************************//**
 * @brief Get the buffer size for the socket used to write on an endpoint.
 *        The kernel doubles this value (to allow space for bookkeeping overhead).
 *
 * @param[in]  endpoint    CPC endpoint handle
 * @param[in]  socket_size The socket size represented as an integer
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_socket_size(cpc_endpoint_t endpoint, uint32_t *socket_size);

/***************************************************************************//**
 * @brief Get the maximum size of allowed for a single write operation.
 *
 * @param[in]  endpoint       CPC endpoint handle
 * @param[in]  max_write_size The socket size represented as an integer
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_max_write_size(cpc_endpoint_t endpoint, size_t *max_write_size);

/***************************************************************************//**
 * @brief Get the encryption state for the provided endpoint.
 *
 * @param[in]  endpoint       CPC endpoint handle
 * @param[in]  is_encrypted   A boolean representing the encryption state
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_encryption_state(cpc_endpoint_t endpoint, bool *is_encrypted);

/***************************************************************************//**
 * @brief Connect to the event socket corresponding to the provided endpoint ID.
 *
 * @param[in]  handle           CPC library handle
 * @param[out] event_handle     CPC endpoint event handle
 * @param[in]  id               CPC endpoint id to open
 *
 * @return On error, a negative value of errno is returned.
 *         On success, the file descriptor of the socket is returned.
 ******************************************************************************/
int cpc_init_endpoint_event(cpc_handle_t handle, cpc_endpoint_event_handle_t *event_handle, uint8_t endpoint_id);

/***************************************************************************//**
 * @brief Deinitialize the CPC endpoint event handle.
 *        Upon success the any allocated resources associated to the provided handle
 *        will be freed.
 *
 * @param[out] handle           CPC library handle
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 ******************************************************************************/
int cpc_deinit_endpoint_event(cpc_endpoint_event_handle_t *event_handle);

/***************************************************************************//**
 * @brief Read from the endpoint event socket.
 *        By default this function will block if there are no pending events
 *        unless CPC_ENDPOINT_EVENT_FLAG_NON_BLOCKING is provided as a flag.
 *        When called with the non-blocking flag, the errno EAGAIN will be set
 *        if no events are available.
 *
 * @param[in]  event_handle     CPC endpoint event handle
 * @param[out] event_type       CPC endpoint event type
 * @param[in]  flags            CPC endpoint event flag
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 *
 * @note Flags are enumerated in #cpc_endpoint_event_flags_t
 *       - CPC_ENDPOINT_EVENT_FLAG_NONE
 *       - CPC_ENDPOINT_EVENT_FLAG_NON_BLOCKING
 ******************************************************************************/
int cpc_read_endpoint_event(cpc_endpoint_event_handle_t event_handle, cpc_event_type_t *event_type, cpc_endpoint_event_flags_t flags);

/***************************************************************************//**
 * @brief Get the option configured for a specified endpoint event.
 *
 * @param[in] endpoint         CPC endpoint event handle
 * @param[in] option           The option to get
 * @param[out] optval          The value of the option
 * @param[in,out] optlen       The length of the value of the option optlen is a value-result argument,
 *                             initially containing the size of the buffer pointed to by optval,
 *                             and modified on return to indicate the actual size of the value returned.
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 *
 * @note This API is not recommended, please use the safer get function calls available for each options
 *
 *     Options are as follows:
 *       - CPC_ENDPOINT_EVENT_OPTION_READ_TIMEOUT:     Get the timeout for the read transaction. Optval
 *                                                     must be a cpc_timeval_t.
 *       - CPC_ENDPOINT_EVENT_OPTION_BLOCKING:         Get the socket access mode, optval is a boolean.
 ******************************************************************************/
int cpc_get_endpoint_event_option(cpc_endpoint_event_handle_t event_handle, cpc_endpoint_event_option_t option, void *optval, size_t *optlen);

/***************************************************************************//**
 * @brief Set the option configured for a specified endpoint event.
 *
 * @param[in] endpoint         CPC endpoint event handle
 * @param[in] option           The option to get
 * @param[in] optval           The value of the option
 * @param[in] optlen           The length of the value of the option optlen
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned.
 *
 * @note This API is not recommended, please use the safer get function calls available for each options
 *
 *     Options are as follows:
 *       - CPC_ENDPOINT_EVENT_OPTION_READ_TIMEOUT:     Get the timeout for the read transaction. Optval
 *                                                     must be a cpc_timeval_t.
 *       - CPC_ENDPOINT_EVENT_OPTION_BLOCKING:         Get the socket access mode, optval is a boolean.
 ******************************************************************************/
int cpc_set_endpoint_event_option(cpc_endpoint_event_handle_t event_handle, cpc_endpoint_event_option_t option, const void *optval, size_t optlen);

/***************************************************************************//**
 * @brief Set the timeout for the endpoint event read operation
 *
 * @param[in]  event_handle CPC endpoint event handle
 * @param[in]  timeval      Requested timeout
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_set_endpoint_event_read_timeout(cpc_endpoint_event_handle_t event_handle, cpc_timeval_t timeval);

/***************************************************************************//**
 * @brief Get the timeout for the endpoint event read operation
 *
 * @param[in]  event_handle CPC endpoint event handle
 * @param[in]  timeval      Pointer to the current timeout
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_event_read_timeout(cpc_endpoint_event_handle_t event_handle, cpc_timeval_t *timeval);

/***************************************************************************//**
 * @brief Set the blocking mode for the endpoint events read operation
 *
 * @param[in]  event_handle   CPC endpoint event handle
 * @param[in]  blocking       Boolean value whether to block or not
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_set_endpoint_event_blocking(cpc_endpoint_event_handle_t event_handle, bool blocking);

/***************************************************************************//**
 * @brief Get the blocking mode for the endpoint events read operation
 *
 * @param[in]  event_handle   CPC endpoint event handle
 * @param[in]  is_blocking    Boolean value whether to block or not
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 ******************************************************************************/
int cpc_get_endpoint_event_blocking_mode(cpc_endpoint_event_handle_t event_handle, bool *is_blocking);

/***************************************************************************//**
 * @brief Get a session id associated to this instance. This id is guaranteed to be
 * unique between all of the clients of a single endpoint on a single CPCd instance.
 *
 * @param[in]  endpoint CPC library handle
 * @param[out] session_id The unique id associated to this handle
 *
 * @return On error, a negative value of errno is returned.
 *         On success, 0 is returned
 *
 * @note This unique id may change if CPCd restarts
 ******************************************************************************/
int cpc_get_endpoint_session_id(cpc_endpoint_t endpoint, uint32_t *session_id);

/** @} (end addtogroup cpc) */

#ifdef __cplusplus
}
#endif

#endif // SL_CPC_H
