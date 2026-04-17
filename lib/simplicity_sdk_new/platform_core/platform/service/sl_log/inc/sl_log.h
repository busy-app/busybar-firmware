/***************************************************************************/ /**
 @file sl_log.h
* @brief Silicon Labs Debug Logger API
* @version 1.0.0
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

#ifndef SL_LOG_H
#define SL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_log_common_config.h"
#include "sl_status.h"
#include "sl_compiler.h"
/** @addtogroup sl_log Silicon Labs Debug Logger
 * @brief Comprehensive logging system with multiple backends and efficient
 * event handling
 *
 * The Silicon Labs Debug Logger provides a flexible, efficient logging system
 * that supports multiple backends including proprietary interfaces and SEGGER
 * SystemView. It offers both printf-style and event-based logging with
 * compile-time and runtime filtering capabilities.
 *
 * Key features:
 * - Multiple log levels (INFO, DEBUG, WARN, ERROR, CRASH)
 * - Multiple backend support (Proprietary, SystemView)
 * - Efficient ring buffer implementation
 * - Multi-core timestamp synchronization
 * - Compile-time optimization for reduced overhead
 * - Power management integration
 *
 * @{
 */

/**
 * @defgroup sl_log_constants Constants and Limits
 * @brief Constants defining system limits and capabilities
 * @{
 */

/** @brief Maximum number of events that can be configured in the system */
#define SL_LOG_MAX_NO_OF_EVENTS 255

/** @brief Threshold for automatic buffer flushing (80% of buffer capacity) */
#define SL_LOG_THRESHOLD_PERCENTAGE 80

/** @brief Threshold event count */
#define SL_LOG_THRESHOLD (SL_LOG_NUMBER_OF_EVENTS*SL_LOG_THRESHOLD_PERCENTAGE)/100

/** @brief Core ID of Host*/
#define SL_LOG_HOST_CORE_ID 0

#define SL_LOG_OVERFLOW_EVENT_ID 0xFFFFFFFF

/** @} (end addtogroup sl_log_constants) */

/**
 * @defgroup sl_log_types Type Definitions
 * @brief Core type definitions for the logging system
 * @{
 */

/**
 * @brief Log level enumeration
 *
 * Defines the various log levels available in the system, from no logging
 * to crash-level events. Higher numeric values indicate more critical events.
 *
 * The log levels are hierarchical - enabling a higher level automatically
 * includes all lower-numbered (more critical) levels.
 */
typedef enum {
  /** @brief Debug level - detailed debugging information */
  SL_LOG_ENUM_CONFIG_DEBUG = SL_LOG_CONFIG_LEVEL_DEBUG,
  /** @brief Information level - general informational messages */
  SL_LOG_ENUM_CONFIG_INFO = SL_LOG_CONFIG_LEVEL_INFO,
  /** @brief Warning level - warning conditions that should be noted */
  SL_LOG_ENUM_CONFIG_WARN = SL_LOG_CONFIG_LEVEL_WARN,
  /** @brief Error level - error conditions that affect functionality */
  SL_LOG_ENUM_CONFIG_ERROR = SL_LOG_CONFIG_LEVEL_ERROR,
  /** @brief Crash level - critical system failures */
  SL_LOG_ENUM_CONFIG_CRASH = SL_LOG_CONFIG_LEVEL_CRASH,
  /** @brief No logging - all log messages are disabled */
  SL_LOG_ENUM_CONFIG_NONE = SL_LOG_CONFIG_LEVEL_NONE,  
  /** @brief Invalid level - used for validation purposes */
  SL_LOG_ENUM_CONFIG_INVALID,
} sl_log_level_t;

/**
 * @brief Maximum log arguments enumeration
 *
 * Defines the maximum number of arguments that can be passed to log functions.
 * This setting affects memory usage and performance - higher values provide
 * more flexibility but consume more resources.
 */
typedef enum {
  /** @brief No arguments supported - string-only logging */
  SL_LOG_ENUM_CONFIG_ARG0 = SL_LOG_CONFIG_ARG0,
  /** @brief Up to 1 argument supported */
  SL_LOG_ENUM_CONFIG_ARG1,
  /** @brief Up to 2 arguments supported */
  SL_LOG_ENUM_CONFIG_ARG2,
  /** @brief Up to 3 arguments supported */
  SL_LOG_ENUM_CONFIG_ARG3,
  /** @brief Invalid argument count - used for validation */
  SL_LOG_ENUM_CONFIG_ARG_INVALID,
} sl_log_args_t;

/**
 * @brief Log system configuration structure
 *
 * Contains all configuration parameters for the logging system. This structure
 * is used to configure the logger's behavior at runtime and defines the
 * system's capabilities and limits.
 */
typedef struct {
  /** @brief Maximum number of events that can be stored in the ring buffer */
  uint32_t no_of_events;
  /** @brief Current log level filter - only messages at this level or higher
   * are processed */
  sl_log_level_t log_level;
  /** @brief Maximum number of arguments supported in log messages */
  sl_log_args_t max_no_args;
} sl_log_config_t;

/**
 * @brief Structure representing a single log event
 *
 * This structure contains all information for a single log event, including
 * timing information, event identification, arguments, and metadata. Events
 * are stored in a ring buffer and transmitted to the selected backend.
 *
 * @note The total size of this structure affects memory usage and should be
 *       kept as compact as possible for embedded systems.
 */
typedef __PACKED_STRUCT {
  /** @brief Timestamp when the event was logged (in system timer units) */
  uint32_t timestamp;
  /** @brief Unique event identifier (pointer to format string or numeric ID) */
  uint32_t event_id;
  /** @brief Array of arguments associated with the event (up to
   * SL_LOG_CONFIG_ARG items) */
  uint32_t args[SL_LOG_CONFIG_ARG];
  /** @brief Number of valid arguments in the args array (0 to
   * SL_LOG_CONFIG_ARG) */
  uint16_t arg_count;
  /** @brief Core identifier that generated the event (0 = host core) */
  uint8_t core_id;
  /** @brief Event flags - bits 1-7: log level, bit 0: event type (0=format
   * string, 1=numeric) */
  uint8_t flags;
  /** @brief Version of the logging component that generated this event */
  uint8_t version;
} sl_log_event_t;

/**
 * @brief Ring buffer control structure
 *
 * Manages a circular buffer of log events. The ring buffer provides efficient
 * storage for log events with automatic wraparound when full. Newer events
 * overwrite older ones when the buffer capacity is exceeded.
 *
 * @note This structure is designed for single-producer, single-consumer access
 *       patterns typical in logging systems.
 */
typedef struct {
  /** @brief Index where the next event will be written (0 to no_of_events-1) */
  uint32_t write_index;
  /** @brief Index of the oldest unread event (0 to no_of_events-1) */
  uint32_t read_index;
  /** @brief Current number of events stored in buffer (0 to no_of_events) */
  uint32_t event_count;
  /** @brief Number of available event slots left in the buffer */
  int32_t available_event_slots;
  /** @brief Pointer to the actual ring buffer storage array */
  sl_log_event_t *sl_log_buffer;
} sl_log_ring_buffer_t;

/** @} (end addtogroup sl_log_types) */

/**
 * @defgroup sl_log_globals Global Variables
 * @brief Global variables used by the logging system
 * @{
 */

/** @brief Global logging system configuration */
extern sl_log_config_t sl_log_config;

/** @} (end addtogroup sl_log_globals) */

/**
 * @defgroup sl_log_api_common Common API Functions
 * @brief Common logging API functions available to all users
 *
 * These functions provide the main interface for initializing, configuring,
 * and using the logging system. They are designed to work across all
 * supported platforms and backends.
 *
 * @{
 */

/**
 * @brief Initialize the debug logger.
 *
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Initialization successful.
 *         - SL_STATUS_NULL_POINTER: Null pointer passed as parameter.
 *         - SL_STATUS_FAIL: Initialization failed.
 */
sl_status_t sl_log_init(void);

/**
 * @brief Set the log level for the logger.
 *
 * @param[in] level Log level to be set.
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Log level set successfully.
 *         - SL_STATUS_INVALID_PARAMETER: Invalid log level parameter.
 */
sl_status_t sl_log_set_loglevel(sl_log_level_t level);

/**
 * @brief Get the current log level of the logger.
 *
 * @return sl_log_level_t Current log level.
 */
sl_log_level_t sl_log_get_loglevel(void);

/**
 * @brief Synchronize the timestamp between the host and captive core.
 *
 * @param[in] core_id Core identifier.
 * @param[in] args Pointer to additional arguments if needed.
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Synchronization successful.
 *         - SL_STATUS_FAIL: Synchronization failed.
 *         - SL_STATUS_INVALID_PARAMETER: Invalid parameters provided.
 *         - any other error codes as defined by the underlying implementation.
 */
sl_status_t sl_log_sync_timestamp(uint8_t core_id, void *args);

/**
 * @brief Send a log event with no arguments
 *
 * Logs an event containing only an event ID and level information.
 * This is the most efficient logging function as it requires minimal
 * memory and processing overhead.
 *
 * @param[in] event_id Event identifier (format string pointer or numeric ID)
 * @param[in] log_level Log level combined with event type flags
 *
 * @note This function is typically called by higher-level logging macros
 *       rather than directly by application code.
 */
void sl_log_send_no_args(uint32_t event_id, uint8_t log_level);

/**
 * @brief Send a log event with one argument
 *
 * Logs an event with a single 32-bit argument. Suitable for logging
 * simple values like integers, pointers, or status codes.
 *
 * @param[in] event_id Event identifier (format string pointer or numeric ID)
 * @param[in] log_level Log level combined with event type flags
 * @param[in] arg1 First argument for the log event
 *
 * @note Arguments are stored as 32-bit values. Larger data types should
 *       be cast or split across multiple arguments.
 */
void sl_log_send_arg1(uint32_t event_id, uint8_t log_level, uint32_t arg1);

/**
 * @brief Send a log event with two arguments
 *
 * Logs an event with two 32-bit arguments. Useful for logging pairs
 * of related values or more complex data structures.
 *
 * @param[in] event_id Event identifier (format string pointer or numeric ID)
 * @param[in] log_level Log level combined with event type flags
 * @param[in] arg1 First argument for the log event
 * @param[in] arg2 Second argument for the log event
 */
void sl_log_send_arg2(uint32_t event_id, uint8_t log_level, uint32_t arg1,
                      uint32_t arg2);

/**
 * @brief Send a log event with three arguments
 *
 * Logs an event with three 32-bit arguments. This is the maximum number
 * of arguments supported by the system for optimal memory efficiency.
 *
 * @param[in] event_id Event identifier (format string pointer or numeric ID)
 * @param[in] log_level Log level combined with event type flags
 * @param[in] arg1 First argument for the log event
 * @param[in] arg2 Second argument for the log event
 * @param[in] arg3 Third argument for the log event
 *
 * @note This function provides the maximum argument capacity. For more
 *       than 3 arguments, consider using multiple log events or structured
 * logging.
 */
void sl_log_send_arg3(uint32_t event_id, uint8_t log_level, uint32_t arg1,
                      uint32_t arg2, uint32_t arg3);

/**
 * @brief Flush the logger buffer
 *
 * Forces immediate transmission of all pending log events in the ring buffer
 * to the selected backend. This function blocks until all events are sent
 * or an error occurs.
 *
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: All events flushed successfully
 *         - SL_STATUS_FAIL: Flush operation failed
 *         - SL_STATUS_INVALID_STATE: Logger not properly initialized
 *
 * @note This function is useful before entering sleep mode or at critical
 *       points where log data must be preserved.
 */
sl_status_t sl_log_flush(void);

/** @} (end addtogroup sl_log_api_common) */

/**
 * @defgroup sl_log_api_internal Internal API Functions
 * @brief Internal functions used by the logging system implementation
 *
 * These functions are primarily for internal use by the logging system
 * and platform-specific implementations. They provide access to low-level
 * functionality and system state information.
 *
 * @{
 */

/**
 * @brief Calculate timestamp delta between cores
 *
 * Computes the timestamp difference between captive cores and the host
 * timestamp reference. This is used for timestamp synchronization in
 * multi-core logging scenarios.
 *
 * @return int Timestamp delta value in system timer units
 *
 * @note This function is primarily used internally for multi-core
 *       timestamp alignment and synchronization.
 */
int sl_log_get_timestamp_delta(void);

/**
 * @brief Retrieve current logging configuration
 *
 * Returns a snapshot of the current logging system configuration,
 * including log level, backend interface, and buffer settings.
 *
 * @return sl_log_config_t Copy of current logging configuration
 *
 * @note This function returns a copy of the configuration structure,
 *       so modifications to the returned value do not affect the
 *       actual system configuration.
 */
sl_log_config_t sl_log_get_config(void);

/**
 * @brief Check if ring buffer is empty
 *
 * Efficiently determines whether the ring buffer contains any log events.
 * This inline function provides optimal performance for frequent buffer
 * status checks.
 *
 * @param[in] ring_buffer Pointer to the ring buffer control structure
 * @return uint8_t Non-zero if buffer is empty, zero if it contains events
 *
 * @note This function should only be called with a valid ring_buffer pointer.
 */
static inline uint8_t
sl_log_is_ring_buffer_empty(sl_log_ring_buffer_t *ring_buffer) {
  return (ring_buffer->event_count == 0);
}

/**
 * @brief Check if ring buffer is full
 *
 * Efficiently determines whether the ring buffer has reached its maximum
 * capacity. When full, new events will overwrite the oldest events.
 *
 * @param[in] ring_buffer Pointer to the ring buffer control structure
 * @return uint8_t Non-zero if buffer is full, zero if space is available
 *
 * @note This function should only be called with a valid ring_buffer pointer.
 */
static inline uint8_t
sl_log_is_ring_buffer_full(sl_log_ring_buffer_t *ring_buffer) {
  return ((ring_buffer->event_count == SL_LOG_NUMBER_OF_EVENTS));
}

/** @} (end addtogroup sl_log_api_internal) */

/**
 * @defgroup sl_log_api_platform Platform-Specific API Functions
 * @brief Platform-specific functions for logging system integration
 *
 * These functions provide platform-specific functionality for power management,
 * backend initialization, and hardware-specific operations. Implementations
 * are provided by the platform-specific code.
 *
 * @{
 */

/**
 * @brief Prepare logger for sleep mode
 *
 * Configures the logging system before entering sleep mode. This typically
 * involves flushing pending events, configuring wake-up sources, and
 * preparing hardware for low-power operation.
 *
 * @param[in] args void pointer for any platform-specific arguments
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Sleep preparation successful
 *         - SL_STATUS_FAIL: Sleep preparation failed
 *         - SL_STATUS_NULL_POINTER: Invalid config pointer
 *
 * @note This function should be called before entering any sleep mode
 *       to ensure log data integrity and proper system behavior.
 */
sl_status_t sl_log_pre_sleep_process(void * args);

/**
 * @brief Initialize logger after wake-up
 *
 * Reinitializes the logging system after waking from sleep mode. This
 * includes restoring hardware state, reinitializing timers, and resuming
 * normal logging operations.
 *
 * @param[in] args void pointer for any platform-specific arguments
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Wake-up initialization successful
 *         - SL_STATUS_FAIL: Wake-up initialization failed
 *         - SL_STATUS_NULL_POINTER: Invalid config pointer
 *
 * @note This function should be called immediately after waking from
 *       sleep mode to restore full logging functionality.
 */
sl_status_t sl_log_post_sleep_process(void * args);

/**
 * @brief Initialize the selected logging backend
 *
 * Initializes the specified backend interface for log output. This function
 * sets up the necessary hardware, communication channels, and data structures
 * required by the selected backend.
 *
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Backend initialized successfully
 *         - SL_STATUS_INVALID_PARAMETER: Invalid interface parameter
 *         - SL_STATUS_FAIL: Backend initialization failed
 *
 * @note This function must be called before using any backend-specific
 *       logging functionality. Different backends may have different
 *       initialization requirements and capabilities.
 */
sl_status_t sl_log_backend_init(void);

/**
 * @brief Write log events to backend interface
 *
 * Transmits log events from the ring buffer to the selected backend interface.
 * This function handles ring buffer wraparound and ensures reliable
 * transmission of all requested events to the backend.
 *
 * @param[in] buffer Pointer to the ring buffer containing log events
 * @param[in] read_index Starting index in the buffer for reading events
 * @param[in] event_count Number of events to transmit from the buffer
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: All events written successfully
 *         - SL_STATUS_INVALID_PARAMETER: Invalid buffer pointer or indices
 *         - SL_STATUS_FAIL: Backend write operation failed
 *
 * @note This function may be called multiple times for large event counts
 *       to handle ring buffer wraparound conditions efficiently.
 */
sl_status_t sl_log_backend_write(sl_log_event_t *buffer, uint32_t read_index,
                                 uint32_t event_count);


/**
 * @brief Initializes the core platform logging infrastructure.
 *
 * This function sets up any platform-specific resources required by
 * the logging system (e.g., timers). It is intended to be called once during system
 * startup before any other logging APIs are used.
 *
 *
 * @return SL_STATUS_OK on successful initialization.
 *       any other platform specific error codes on failure.
 *
 */
sl_status_t sl_log_platform_core_init(void);
/**
 * @brief De-initializes the core platform logging infrastructure.
 *
 * This function deinitializes any platform-specific resources allocated by
 * sl_log_platform_core_init().
 *
 * @return SL_STATUS_OK on successful de-initialization.
 *       any other platform specific error codes on failure.
 *
 */
sl_status_t sl_log_platform_core_deinit(void);

/**
 * @brief Get current timestamp counter value
 *
 * Retrieves the current value from the timestamp counter for the specified
 * core. This function provides access to the raw timestamp value used for
 * logging operations.
 *
 * @param[in] core_id Core identifier (0 = host core, >0 = captive cores)
 * @return uint32_t Current timestamp counter value in system timer units
 *
 * @note The timestamp resolution and range depend on the underlying
 *       hardware timer configuration. Use
 * sl_log_get_timestamp_timer_frequency() to convert to actual time units.
 */
uint32_t sl_log_get_timestamp_count(uint8_t core_id);

/**
 * @brief Get timestamp timer frequency
 *
 * Returns the frequency in Hz of the timestamp timer used for the specified
 * core. This frequency value can be used to convert raw timestamp values
 * to actual time units (seconds, milliseconds, etc.).
 *
 * @param[in] core_id Core identifier (0 = host core, >0 = captive cores)
 * @return uint32_t Frequency of the timestamp timer in Hz
 *
 * @note The returned frequency depends on the underlying timer/counter
 *       hardware configuration and may vary between different cores
 *       in a multi-core system.
 */
uint32_t sl_log_get_timestamp_timer_frequency(uint8_t core_id);

/**
 * @brief Set core-specific logger configurations
 *
 * Configures logging parameters for either the host core or a specific
 * captive core. The function automatically delegates to the appropriate
 * API (host or captive core) based on the core ID.
 *
 * @param[in] args Pointer to core-specific configuration arguments
 * @param[in] core_id Core identifier (0 = host core, >0 = captive core)
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Configuration applied successfully
 *         - SL_STATUS_INVALID_PARAMETER: Invalid arguments or core ID
 *         - SL_STATUS_NULL_POINTER: NULL args pointer provided
 *
 * @note The format and content of the args parameter depends on the
 *       specific core type and its configuration requirements.
 */
sl_status_t sl_log_set_configurations(void *args, uint8_t core_id);

/**
 * @brief Get core-specific logger configurations
 *
 * Retrieves current logging configuration parameters for either the host
 * core or a specific captive core. The function automatically delegates
 * to the appropriate API based on the core ID.
 *
 * @param[out] args Pointer to buffer for storing configuration arguments
 * @param[in] core_id Core identifier (0 = host core, >0 = captive core)
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Configuration retrieved successfully
 *         - SL_STATUS_INVALID_PARAMETER: Invalid core ID provided
 *         - SL_STATUS_NULL_POINTER: NULL args pointer provided
 *
 * @note The caller must provide a sufficiently large buffer in args to
 *       hold the core-specific configuration data.
 */
sl_status_t sl_log_get_configurations(void *args, uint8_t core_id);

/**
 * @brief Retrieve the logging ring buffer configuration instance.
 *
 * Provides access to the global (singleton) ring buffer configuration used by
 * the logging subsystem. This structure typically contains buffer size,
 * write/read indices, pointer to ring buffer and any state needed to manage in-memory log storage.
 *
 * The returned pointer refers to an internally managed object; callers MUST NOT
 * free or modify ownership-related aspects of the structure. If mutation of
 * fields is allowed by design, ensure proper synchronization (see Thread Safety).
 *
 * @return Pointer to the logging ring buffer configuration. Returns nullptr if
 *         the configuration has not been initialized or an internal error occurred.
 *
 * @thread_safety
 * - If the logging system initializes the ring buffer during startup and only
 *   mutates it through its own synchronized APIs, read-only access through this
 *   pointer is typically safe.
 * - If callers intend to modify the structure directly, they must ensure
 *   external synchronization to avoid data races.
 *
 */
sl_log_ring_buffer_t *sl_log_get_ring_buffer_config();

/** @} (end addtogroup sl_log_api_platform) */

/** @} (end addtogroup sl_log) */

#ifdef __cplusplus
}
#endif
#endif // SL_DEBUG_LOGGER_H
