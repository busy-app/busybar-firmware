/***************************************************************************/ /**
* @file sl_log.c
* @brief Implementation of the Silicon Labs Debug Logger
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

/**
 * @addtogroup sl_log
 * @{
 */

#include "sl_log.h"
#include "sl_log_platform_specific.h"
#include <stdbool.h>
#if defined (__GNUC__)
#include "cmsis_gcc.h"
#elif defined(__ICCARM__)
#include "cmsis_iccarm.h"
#endif
#include <stddef.h>
#include "sl_component_catalog.h"


/** 
* @brief Null pointer check
*/
#define sl_log_check_null(p) (!(p))


/** @} (end addtogroup sl_log_constants_local) */

/**
 * @defgroup sl_log_variables_global Global Variables
 * @brief Global variables used throughout the logging system
 * @{
 */
#ifdef SL_CATALOG_LOGGER_BACKEND_SYSTEMVIEW_PRESENT
/** @brief External global timestamp variable (temporary workaround) */
extern uint32_t timestamp_global;
#endif
/** @brief Static timestamp delta for multi-core synchronization */
static int sl_log_timestamp_delta = 0;

/** @} (end addtogroup sl_log_variables_global) */

/**
 * @defgroup sl_log_config_instances Configuration Instances
 * @brief Global configuration instances for different build modes
 * @{
 */

/**
 * @brief Default logging system configuration
 *
 * This configuration is used when Universal Configurator is disabled.
 * It uses compile-time constants to define the logging behavior, buffer
 * size, backend selection, and argument limits.
 */
sl_log_config_t sl_log_config = {
    .no_of_events = SL_LOG_NUMBER_OF_EVENTS,       ///< Buffer size from config
    .log_level = (sl_log_level_t)SL_LOG_CONFIG_LEVEL_COMPILE_TIME, ///< Compile-time log level
    .max_no_args = (sl_log_args_t)SL_LOG_CONFIG_ARG,         ///< Maximum arguments per log
};

/**
 * @brief Universal Configurator logging configuration
 *
 * This configuration is used when Universal Configurator is enabled.
 */
sl_log_config_t sl_log_uc_config = {
    .no_of_events = SL_LOG_NUMBER_OF_EVENTS,       ///< Configurable buffer size
    .log_level = (sl_log_level_t)SL_LOG_CONFIG_LEVEL_COMPILE_TIME, ///< Configurable log level
    .max_no_args = (sl_log_args_t)SL_LOG_CONFIG_ARG, ///< Configurable argument count
};

/** @} (end addtogroup sl_log_config_instances) */

/**
 * @defgroup sl_log_buffer_instances Buffer Management Instances
 * @brief Global instances for ring buffer and event management
 * @{
 */

/**
 * @brief Static storage array for log events
 *
 * Pre-allocated array that provides the actual storage space for log events
 * in the ring buffer. The size is determined at compile time to ensure
 * predictable memory usage in embedded systems.
 */
sl_log_event_t sl_log_buffer[SL_LOG_NUMBER_OF_EVENTS];

/**
 * @brief Ring buffer control structure instance
 *
 * Global instance of the ring buffer that manages the circular storage
 * of log events. Initialized with zero indices and points to the static
 * storage array for actual event data.
 */
sl_log_ring_buffer_t sl_log_ring_buffer = {
    .write_index = 0,               ///< Initialize write index to start
    .read_index = 0,                ///< Initialize read index to start
    .event_count = 0,               ///< Buffer starts empty
    .sl_log_buffer = sl_log_buffer, ///< Point to static storage array
    .available_event_slots=SL_LOG_NUMBER_OF_EVENTS, ///< number of avaialble slots
};

/**
 * @brief Temporary event structure for local operations
 *
 * Global temporary event structure used for building log events before
 * they are written to the ring buffer. This avoids stack allocation
 * in interrupt contexts and provides a consistent memory location.
 *
 * @note This variable is reused across multiple log operations and should
 *       not be accessed concurrently.
 */
sl_log_event_t sl_log_event;

sl_log_api_core_t *sli_log_api_core;

sl_log_backend_status_t sl_log_backend_status;

sl_log_event_t sl_log_overflow_event;

/**
 * @brief Update the overflow event
 * 
 * Updates the overflow event with the current timestamp, core ID, flags, argument count, event ID, and version.
 * 
 * @param[in] overflow_count The number of overflow events
 */
static inline void update_over_flow_event(uint32_t overflow_count); 

/**
 * @brief atomic check of the backend transfer status
 * 
 * Checks the backend transfer status atomically by disabling interrupts and checking the backend transfer status.
 * 
 * @return uint8_t True if the backend transfer is available, false otherwise
 */
static inline uint8_t sl_log_get_backend_status(void);

/** @} (end addtogroup sl_log_buffer_instances) */

/**
 * @defgroup sl_log_static_functions Static Helper Functions
 * @brief Internal helper functions for ring buffer management
 * @{
 */

/**
 * @brief Write a log event to the ring buffer
 *
 * Internal function that handles the actual insertion of log events into
 * the circular ring buffer. Manages buffer wraparound, automatic flushing
 * when threshold is reached, and ensures thread safety through interrupt
 * disable/enable around critical sections.
 *
 * This function implements the producer side of the ring buffer, updating
 * write indices and event counts atomically to prevent corruption in
 * interrupt-driven logging scenarios.
 *
 * @param[in] buffer Pointer to the log event to be written
 * @param[in] buffer_size Size of the log event (for consistency, typically
 * ignored)
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK: Event successfully written to ring buffer
 *
 * @note This function disables interrupts briefly to ensure atomic updates
 *       of ring buffer control variables.
 * @note Automatic flushing occurs when buffer reaches SL_LOG_THRESHOLD
 * capacity.
 */
static inline sl_status_t sl_log_write_to_ring_buffer(sl_log_event_t *buffer,
                                                      uint32_t buffer_size);

/** @} (end addtogroup sl_log_static_functions) */

/**
 * @defgroup sl_log_api_implementation API Implementation Functions
 * @brief Implementation of the public logging API functions
 * @{
 */
/**
 * @brief Initialize the Silicon Labs debug logger system
 *
 * Performs comprehensive initialization of the logging system including:
 * - Configuration validation and setup
 * - Ring buffer initialization
 * - Timestamp counter startup
 * - Backend interface initialization
 *
 * The function selects between default configuration (sl_log_config) and
 * Universal Configurator configuration (sl_log_uc_config) based on the
 * SL_LOG_ENABLE_UC_CONFIG compile-time setting.
 *
 * Validation includes checking:
 * - Log level validity (within defined enum range)
 * - Backend interface validity
 * - Event count limits
 * - Argument count limits
 *
 * @return sl_status_t Initialization result:
 *         - SL_STATUS_OK: Logger initialized successfully
 *         - SL_STATUS_NULL_POINTER: Configuration pointer is NULL (shouldn't
 * occur)
 *         - SL_STATUS_INVALID_PARAMETER: Invalid configuration parameter
 * detected
 *
 * @note This function must be called before any logging operations.
 * @note Ring buffer is reset to empty state during initialization.
 */
sl_status_t sl_log_init(void) {
#if (SL_LOG_ENABLE_UC_CONFIG == 1)
  sl_log_config_t *config = &sl_log_uc_config;
#else
  sl_log_config_t *config = &sl_log_config;
#endif

  if (config == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (config->log_level > SL_LOG_ENUM_CONFIG_INVALID) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (config->no_of_events > SL_LOG_MAX_NO_OF_EVENTS) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (config->max_no_args > SL_LOG_ENUM_CONFIG_ARG_INVALID) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_log_config = *config;
  sl_log_ring_buffer.write_index = 0;
  sl_log_ring_buffer.read_index = 0;
  sl_log_ring_buffer.event_count = 0;
  sl_log_ring_buffer.available_event_slots=SL_LOG_NUMBER_OF_EVENTS; 
  sl_log_backend_status.backend_transfer_done=1;
  sli_log_api_core = sl_log_get_api_core();
  sl_log_platform_core_init();
  sl_log_backend_init();
  return SL_STATUS_OK;
}

/**
 * @brief Send a log event with no arguments
 *
 * Creates and logs an event containing only an event ID and metadata.
 * This is the most efficient logging function as it minimizes memory
 * usage and processing overhead.
 *
 * The function:
 * - Captures current timestamp from host API
 * - Sets core ID to 0 (host core)
 * - Packages event data into log structure
 * - Writes to ring buffer for transmission
 *
 * @param[in] event_id Unique event identifier (format string pointer or numeric
 * ID)
 * @param[in] flags Combined log level and event type flags:
 *                  - Bits 1-7: Log level (DEBUG, INFO, WARN, ERROR, etc.)
 *                  - Bit 0: Event type (0=format string, 1=numeric event)
 *
 * @note This function is typically called by higher-level logging macros
 *       rather than directly by application code.
 * @note Function does not return status - logging is fire-and-forget for
 * performance.
 */
void sl_log_send_no_args(uint32_t event_id, uint8_t flags) {

  sl_log_event_t sl_log_event;
  sl_log_event.timestamp = sli_log_api_core ? sli_log_api_core->get_timestamp(SL_LOG_HOST_CORE_ID): 0;
  sl_log_event.core_id = 0;
  sl_log_event.flags = flags;
  sl_log_event.arg_count = 0;
  sl_log_event.event_id = event_id;
  sl_log_event.version = 1;

  sl_log_write_to_ring_buffer(&sl_log_event, sizeof(sl_log_event));
}

/**
 * @brief Send a log event with one argument
 *
 * Creates and logs an event with a single 32-bit argument. Suitable for
 * logging simple values like integers, pointers, status codes, or other
 * data that fits in a 32-bit value.
 *
 * The function follows the same pattern as sl_log_send_no_args() but
 * additionally stores one argument value in the event structure.
 *
 * @param[in] event_id Unique event identifier (format string pointer or numeric
 * ID)
 * @param[in] flags Combined log level and event type flags
 * @param[in] arg1 First argument value to be logged
 *
 * @note Arguments are stored as 32-bit values. Larger data types should
 *       be cast appropriately or split across multiple arguments.
 */
void sl_log_send_arg1(uint32_t event_id, uint8_t flags, uint32_t arg1) {
  sl_log_event_t sl_log_event;
  sl_log_event.timestamp = sli_log_api_core ? sli_log_api_core->get_timestamp(SL_LOG_HOST_CORE_ID): 0;
  sl_log_event.core_id = 0;
  sl_log_event.flags = flags;
  sl_log_event.arg_count = 1;
  sl_log_event.event_id = event_id;
  sl_log_event.args[0] = arg1;
  sl_log_event.version = 1;
  sl_log_write_to_ring_buffer(&sl_log_event, sizeof(sl_log_event));
}

/**
 * @brief Send a log event with two arguments
 *
 * Creates and logs an event with two 32-bit arguments. Useful for logging
 * pairs of related values, coordinates, before/after states, or more
 * complex data structures that require two parameters.
 *
 * @param[in] event_id Unique event identifier (format string pointer or numeric
 * ID)
 * @param[in] flags Combined log level and event type flags
 * @param[in] arg1 First argument value to be logged
 * @param[in] arg2 Second argument value to be logged
 *
 * @note Both arguments are stored as 32-bit values in the args[] array.
 */
void sl_log_send_arg2(uint32_t event_id, uint8_t flags, uint32_t arg1,
                      uint32_t arg2) {
  sl_log_event_t sl_log_event;
  sl_log_event.timestamp = sli_log_api_core ? sli_log_api_core->get_timestamp(SL_LOG_HOST_CORE_ID): 0;
  sl_log_event.core_id = 0;
  sl_log_event.flags = flags;
  sl_log_event.arg_count = 2;
  sl_log_event.event_id = event_id;
  sl_log_event.args[0] = arg1;
  sl_log_event.args[1] = arg2;
  sl_log_event.version = 1;

  sl_log_write_to_ring_buffer(&sl_log_event, sizeof(sl_log_event));
}

/**
 * @brief Send a log event with three arguments
 *
 * Creates and logs an event with three 32-bit arguments. This provides
 * the maximum argument capacity supported by the system for optimal
 * memory efficiency while still allowing complex data logging.
 *
 * Suitable for logging RGB values, 3D coordinates, complex state
 * information, or any data requiring three related parameters.
 *
 * @param[in] event_id Unique event identifier (format string pointer or numeric
 * ID)
 * @param[in] flags Combined log level and event type flags
 * @param[in] arg1 First argument value to be logged
 * @param[in] arg2 Second argument value to be logged
 * @param[in] arg3 Third argument value to be logged
 *
 * @note This function provides maximum argument capacity. For more than
 *       3 arguments, consider using multiple log events or structured logging.
 */
void sl_log_send_arg3(uint32_t event_id, uint8_t flags, uint32_t arg1,
                      uint32_t arg2, uint32_t arg3) {
  sl_log_event_t sl_log_event;
  sl_log_event.timestamp = sli_log_api_core ? sli_log_api_core->get_timestamp(SL_LOG_HOST_CORE_ID): 0;
  sl_log_event.core_id = 0;
  sl_log_event.flags = flags;
  sl_log_event.arg_count = 3;
  sl_log_event.event_id = event_id;
  sl_log_event.args[0] = arg1;
  sl_log_event.args[1] = arg2;
  sl_log_event.args[2] = arg3;
  sl_log_event.version = 1;

  sl_log_write_to_ring_buffer(&sl_log_event, sizeof(sl_log_event));
}
/**
 * @brief Flush all pending log events to the backend
 *
 * Forces immediate transmission of all events currently stored in the ring
 * buffer to the configured backend interface. This function handles:
 * - Empty buffer detection and early return
 * - Bulk transmission of all pending events
 * - Ring buffer index management and wraparound
 * - Consumer-side buffer state updates
 *
 * The function calculates the number of events to send and delegates
 * the actual transmission to sl_log_backend_write(), which handles
 * backend-specific formatting and transmission protocols.
 *
 * After successful transmission, the ring buffer read index is updated
 * to reflect that all events have been consumed, effectively emptying
 * the buffer for new events.
 *
 * @return sl_status_t Flush operation result:
 *         - SL_STATUS_OK: All events flushed successfully
 *         - SL_STATUS_EMPTY: No events to flush (buffer was empty)
 *
 * @note This function should be called from the lowest priority task to
 *       avoid blocking time-critical operations.
 * @note Automatic flushing occurs at SL_LOG_THRESHOLD, but this function
 *       can be called manually for immediate transmission.
 */
 sl_status_t sl_log_flush(void) {

  if(sl_log_get_backend_status()){
  uint32_t new_read_index = 0;
  __disable_irq();
  // Check if the ring buffer is empty
  if (sl_log_is_ring_buffer_empty(&sl_log_ring_buffer)) {
    sl_log_backend_status.backend_transfer_done=1;  
  __enable_irq();
    return SL_STATUS_EMPTY; // No data to send
  }
  uint32_t read_index = sl_log_ring_buffer.read_index;
  uint32_t event_count = sl_log_ring_buffer.event_count;

  __enable_irq();
  sl_log_backend_write(sl_log_ring_buffer.sl_log_buffer, read_index,
                       event_count);
  new_read_index = event_count + read_index;
 __disable_irq();
  if (new_read_index >= SL_LOG_NUMBER_OF_EVENTS) {
    sl_log_ring_buffer.read_index = new_read_index - SL_LOG_NUMBER_OF_EVENTS;
  } else {
    sl_log_ring_buffer.read_index = new_read_index;
  }
  sl_log_ring_buffer.event_count-=event_count;
  int available_event_slots=sl_log_ring_buffer.available_event_slots;

      if(sl_log_ring_buffer.available_event_slots<0){
      sl_log_ring_buffer.available_event_slots=0;
  }
  sl_log_ring_buffer.available_event_slots+=event_count;
  __enable_irq();
  if(available_event_slots<0){
      update_over_flow_event(0-available_event_slots);
      sl_log_backend_write(&sl_log_overflow_event,0,1);
  }
  __disable_irq();
    sl_log_backend_status.backend_transfer_done=1;
  __enable_irq();  
  }
  else{
    return SL_STATUS_BUSY;
  }

  return SL_STATUS_OK; // Data successfully sent
}
/**
 * @brief Set the runtime log level filter
 *
 * Updates the current log level filter that determines which log messages
 * are processed at runtime. Only messages at the specified level or higher
 * priority will be logged. This provides runtime control over logging
 * verbosity without requiring recompilation.
 *
 * The log level hierarchy (from lowest to highest priority):
 * - SL_LOG_ENUM_CONFIG_DEBUG: Most verbose, includes all messages
 * - SL_LOG_ENUM_CONFIG_INFO: Informational messages and above
 * - SL_LOG_ENUM_CONFIG_WARN: Warning messages and above
 * - SL_LOG_ENUM_CONFIG_ERROR: Error messages and above
 * - SL_LOG_ENUM_CONFIG_CRASH: Only crash-level messages
 * - SL_LOG_ENUM_CONFIG_NONE: No logging
 *
 * @param[in] level The new log level to set (must be valid enum value)
 * @return sl_status_t Operation result:
 *         - SL_STATUS_OK: Log level updated successfully
 *         - SL_STATUS_INVALID_PARAMETER: Invalid log level provided
 *
 * @note Changes take effect immediately for new log messages.
 * @note This setting works in conjunction with compile-time level filtering.
 */
sl_status_t sl_log_set_loglevel(sl_log_level_t level) {
  if (level >= SL_LOG_ENUM_CONFIG_INVALID) {
    return SL_STATUS_INVALID_PARAMETER; // Invalid log level parameter
  }
  sl_log_config.log_level = level;
  return SL_STATUS_OK;
}
/**
 * @brief Get the current runtime log level
 *
 * Retrieves the currently active log level filter that determines
 * which messages are being processed. This can be used by applications
 * to check logging configuration or implement conditional logging logic.
 *
 * @return sl_log_level_t Current active log level filter
 *
 * @note The returned value reflects the runtime setting, which may differ
 *       from compile-time settings if modified via sl_log_set_loglevel().
 */
sl_log_level_t sl_log_get_loglevel(void) { return sl_log_config.log_level; }

/**
 * @brief Get the current multi-core timestamp delta
 *
 * Returns the calculated timestamp offset used for synchronizing timestamps
 * between the host core and captive cores in multi-core systems. This delta
 * compensates for timing differences between different processor cores.
 *
 * @return int Current timestamp delta in system timer units
 *
 * @note This value is used internally for multi-core timestamp alignment.
 * @note A delta of 0 indicates no correction is being applied.
 */
int sl_log_get_timestamp_delta(void) { return sl_log_timestamp_delta; }

/**
 * @brief Write an event to the circular ring buffer (Internal Implementation)
 *
 * Core ring buffer insertion function that handles the low-level mechanics
 * of storing log events in the circular buffer. This function implements
 * the producer side of the ring buffer with the following key features:
 *
 * - Atomic index updates using interrupt disable/enable
 * - Automatic buffer wraparound when reaching buffer end
 * - Threshold-based automatic flushing for flow control
 * - Thread-safe operation in interrupt-driven environments
 *
 * The function uses a critical section (interrupt disable) around index
 * updates to ensure atomicity, but keeps the actual data copy outside
 * the critical section to minimize interrupt latency.
 *
 * Automatic flushing occurs when the buffer reaches SL_LOG_THRESHOLD
 * capacity (approximately 80% full), helping prevent buffer overruns
 * while maintaining good performance.
 *
 * @param[in] event_buffer Pointer to the log event to write to the buffer
 * @param[in] event_size Size of the event data (parameter for consistency,
 * typically unused)
 * @return sl_status_t Write operation result:
 *         - SL_STATUS_OK: Event successfully written to ring buffer
 *
 * @note This is an internal function called by the public logging APIs.
 * @note Critical sections are kept minimal to reduce interrupt latency.
 * @note Buffer overflow is handled by overwriting oldest events (circular
 * behavior).
 */
sl_status_t sl_log_write_to_ring_buffer(sl_log_event_t *event_buffer,
                                        uint32_t event_size) {
  (void)event_size;

  sl_log_ring_buffer_t *sl_log_ring_buffer_ptr = &sl_log_ring_buffer;
  __disable_irq();
  sl_log_ring_buffer_ptr->available_event_slots--;
  if(sl_log_ring_buffer_ptr->available_event_slots<0 && sl_log_backend_status.backend_transfer_done==0)
    {
      __enable_irq();
      return SL_STATUS_NOT_AVAILABLE;
    }
  uint32_t write_index = sl_log_ring_buffer_ptr->write_index;
  sl_log_ring_buffer_ptr->write_index =
      (write_index + 1u >= SL_LOG_NUMBER_OF_EVENTS) ? 0u : (write_index + 1u);
  __enable_irq();

  sl_log_ring_buffer_ptr->sl_log_buffer[write_index] = *event_buffer;

  __disable_irq();
  if (++sl_log_ring_buffer_ptr->event_count > SL_LOG_NUMBER_OF_EVENTS) {
    sl_log_ring_buffer_ptr->event_count = SL_LOG_NUMBER_OF_EVENTS;
    if (++sl_log_ring_buffer_ptr->read_index == SL_LOG_NUMBER_OF_EVENTS) {
      sl_log_ring_buffer_ptr->read_index = 0;
    }
  }
  __enable_irq();

  return SL_STATUS_OK;
}
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
sl_status_t sl_log_platform_core_init(void) {
  sli_log_api_core = sl_log_get_api_core();
 return sli_log_api_core ? sli_log_api_core->platform_core_init(): SL_STATUS_NULL_POINTER;
}

/**
 * @brief Get current timestamp counter value for specified core
 *
 * Retrieves the current timestamp value from either the host core or
 * a captive core, depending on the core_id parameter. This function
 * handles multi-core timestamp coordination and includes a temporary
 * workaround for global timestamp integration.
 *
 * The function implements the following logic:
 * 1. Check for temporary global timestamp override (SystemView integration)
 * 2. Route to host API for core_id 0 (host core)
 * 3. Route to captive core API for core_id > 0
 *
 * @param[in] core_id Core identifier:
 *                    - 0: Host core timestamp
 *                    - >0: Captive core timestamp
 * @return uint32_t Current timestamp value in system timer units
 *         - Returns 0 if core_id is invalid or API call fails
 *
 * @note The global timestamp_global variable is a temporary workaround
 *       and will be removed when SystemView API integration is complete.
 * @note Timestamp resolution and range depend on platform-specific
 * implementation.
 */
uint32_t sl_log_get_timestamp_count(uint8_t core_id) {
#ifdef SL_CATALOG_LOGGER_BACKEND_SYSTEMVIEW_PRESENT
  //@TODO remove global timestamp variable after getting api from systemview
  if (timestamp_global != 0) {
    volatile uint32_t timestamp_val = timestamp_global;
    timestamp_global = 0;
    return timestamp_val;
  }
#endif  
    return sli_log_api_core ? sli_log_api_core->get_timestamp(core_id): 0;
}
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
sl_status_t sl_log_platform_core_deinit(void) {
 return sli_log_api_core ? sli_log_api_core->platform_core_deinit(): SL_STATUS_NULL_POINTER;
}
/**
 * @brief Initialize the specified logging backend interface
 *
 * Performs backend-specific initialization based on the selected interface
 * type. Each backend may have different initialization requirements, hardware
 * setup needs, and configuration parameters.
 *
 * Supported backends:
 * - SL_LOG_BACKEND_PROPRIETARY: Platform-specific interface (typically
 * UART-based)
 * - SL_LOG_BACKEND_SYSTEMVIEW: SEGGER SystemView real-time analysis integration
 *
 * The function uses conditional compilation to include only the backends
 * that are configured at compile time, reducing code size and complexity.
 *
 * @param[in] interface The backend interface type to initialize
 * @return sl_status_t Initialization result:
 *         - SL_STATUS_OK: Backend initialized successfully
 *         - SL_STATUS_INVALID_PARAMETER: Unsupported interface type
 *         - Other codes: Backend-specific initialization errors
 *
 * @note This function is called automatically during sl_log_init().
 * @note Backend availability depends on compile-time configuration macros.
 * @note Some backends may require additional hardware or software setup.
 */
sl_status_t sl_log_backend_init() {
  sl_log_api_backend_t * sl_log_backend_api=sl_log_get_api_backend();
  return sl_log_backend_api ? sl_log_backend_api->backend_init(): SL_STATUS_NULL_POINTER;
}

/**
 * @brief Prepare logging system for sleep mode entry
 *
 * Performs necessary preparation steps before the system enters sleep mode
 * to ensure log data integrity and proper system behavior. This typically
 * includes flushing pending events, configuring wake-up sources, and
 * preparing hardware for low-power operation.
 *
 * The function delegates to the platform-specific host API implementation
 * which knows the appropriate steps for the target hardware and power
 * management requirements.
 *
 * Common preparation activities may include:
 * - Flushing all pending log events to prevent data loss
 * - Stopping or configuring timestamp counters for sleep mode
 * - Preparing backend interfaces for power-down
 * - Saving critical state information
 *
 * @param[in] args void pointer for any platform-specific arguments
 * @return sl_status_t Preparation result:
 *         - SL_STATUS_OK: Sleep preparation completed successfully
 *         - Other codes: Platform-specific preparation errors
 *
 * @note This function should be called before entering any sleep mode.
 * @note Must be paired with sl_log_post_sleep_process() after wake-up.
 */
sl_status_t sl_log_pre_sleep_process(void * args) {
  if(sli_log_api_core == NULL){
    return SL_STATUS_NOT_INITIALIZED;
  }
   return sli_log_api_core->pre_sleep_process(args);
}
/**
 * @brief Reinitialize logging system after sleep mode wake-up
 *
 * Restores the logging system to full operational state after waking from
 * sleep mode. This includes reinitializing hardware, restoring configuration,
 * and resuming normal logging operations.
 *
 * The function delegates to the platform-specific host API implementation
 * which handles the hardware-specific restoration procedures required
 * after power management events.
 *
 * Common restoration activities may include:
 * - Restarting timestamp counters with proper synchronization
 * - Reinitializing backend interface hardware
 * - Restoring saved configuration state
 * - Verifying system clock and timing references
 *
 * @param[in] args void pointer for any platform-specific arguments
 * @return sl_status_t Restoration result:
 *         - SL_STATUS_OK: Wake-up restoration completed successfully
 *         - Other codes: Platform-specific restoration errors
 *
 * @note This function should be called immediately after waking from sleep.
 * @note Must be paired with sl_log_pre_sleep_process() before sleep entry.
 * @note Logging functionality may be impaired until this function completes.
 */
sl_status_t sl_log_post_sleep_process(void * args) {
    if(sli_log_api_core == NULL){
    return SL_STATUS_NOT_INITIALIZED;
  }
 return sli_log_api_core->post_sleep_process(args);
}
/**
 * @brief Set the logger configurations for host or captive core.
 *
 * @param args Pointer to the platform specific configuration arguments.
 * @param core_id The core ID (0 for host, non-zero for captive core).
 * @return sl_status_t SL_STATUS_OK if successful, or an error code if
 * initialization fails.
 */
sl_status_t sl_log_set_configurations(void *args, uint8_t core_id) {
    if(sli_log_api_core == NULL){
    return SL_STATUS_NOT_INITIALIZED;
  }
return sli_log_api_core->set_configuration(args, core_id);
}

/**
 * @brief Get the logger configurations for host or captive core.
 *
 * @param args Pointer to the platform specific configuration arguments.
 * @param core_id The core ID (0 for host, non-zero for captive core).
 * @return sl_status_t SL_STATUS_OK if successful, or an error code if
 * initialization fails.
 */
sl_status_t sl_log_get_configurations(void *args, uint8_t core_id) {
    if(sli_log_api_core == NULL){
    return SL_STATUS_NOT_INITIALIZED;
  }
 return sli_log_api_core->get_configuration(args, core_id);
}

/**
 * @brief Write log events to the configured backend interface
 *
 * Transmits log events from the ring buffer to the currently selected
 * backend interface. This function handles different backend types and
 * manages ring buffer wraparound conditions during transmission.
 *
 * Backend-specific handling:
 * - SL_LOG_BACKEND_PROPRIETARY: Bulk transmission via proprietary protocol
 * - SL_LOG_BACKEND_SYSTEMVIEW: Individual event transmission to SystemView
 *
 * For ring buffer wraparound (when events span the buffer boundary),
 * the proprietary backend handles this by splitting the transmission
 * into two parts: from read_index to buffer end, then from buffer
 * start to the remaining events.
 *
 * The SystemView backend processes events individually in sequence,
 * automatically handling index wraparound during iteration.
 *
 * @param[in] buffer Pointer to the ring buffer array containing events
 * @param[in] read_index Starting index for reading events from buffer
 * @param[in] event_count Number of events to transmit
 * @return sl_status_t Transmission result:
 *         - SL_STATUS_OK: Events transmitted successfully
 *         - SL_STATUS_INVALID_PARAMETER: Invalid backend interface
 *         - Other codes: Backend-specific transmission errors
 *
 * @note This function is called by sl_log_flush() to transmit pending events.
 * @note Ring buffer event_count is reset to 0 for proprietary backend after
 * transmission.
 * @note SystemView backend processes events individually for real-time
 * analysis.
 */
sl_status_t sl_log_backend_write(sl_log_event_t *buffer, uint32_t read_index,
                                 uint32_t event_count) {
  sl_log_api_backend_t * sl_log_backend_api=sl_log_get_api_backend();
  if (buffer == NULL) {
    return SL_STATUS_NULL_POINTER; // Invalid parameters
  }
  if(read_index >= SL_LOG_NUMBER_OF_EVENTS || event_count > SL_LOG_NUMBER_OF_EVENTS){
    return SL_STATUS_INVALID_PARAMETER;
  }
  return sl_log_backend_api->backend_write(buffer,read_index,event_count);
}

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
 
uint32_t sl_log_get_timestamp_timer_frequency(uint8_t core_id){
    if(sli_log_api_core == NULL){
    return 0;
  }
  return sli_log_api_core->get_timestamp_timer_frequency(core_id);
}

/**
 * @brief Synchronize timestamps between host and captive cores
 *
 * Performs timestamp synchronization between the host core and specified
 * captive core to ensure coherent timing across multi-core log events.
 * This is essential for accurate event correlation and timing analysis
 * in multi-core systems.
 *
 * The function delegates to the platform-specific timer synchronization
 * implementation through the logging API, which handles the low-level
 * details of inter-core timing coordination.
 *
 * Synchronization may involve:
 * - Measuring and compensating for clock drift between cores
 * - Establishing common time reference points
 * - Calibrating timing offsets for different processor speeds
 * - Updating timestamp delta values for correction
 *
 * @param[in] core_id Target core identifier for synchronization
 * @param[in] args Platform-specific synchronization parameters
 * @return sl_status_t Synchronization result:
 *         - SL_STATUS_OK: Timestamp synchronization successful
 *         - SL_STATUS_FAIL: Synchronization operation failed
 *         - SL_STATUS_INVALID_PARAMETER: Invalid core ID or parameters
 *         - Other codes: Platform-specific synchronization errors
 *
 * @note This function is critical for accurate multi-core event correlation.
 * @note Should be called periodically to maintain synchronization accuracy.
 * @note Platform-specific implementation determines synchronization method.
 */
sl_status_t sl_log_sync_timestamp(uint8_t core_id, void *args) {
    if(sli_log_api_core == NULL){
    return SL_STATUS_NOT_INITIALIZED;
  }
  return sli_log_api_core->time_sync(args,core_id);
   
}

sl_log_ring_buffer_t *sl_log_get_ring_buffer_config(){
  return &sl_log_ring_buffer;
}

void update_over_flow_event(uint32_t overflow_count){
  sl_log_event_t sl_log_overflow_event_local;
  sl_log_overflow_event_local.timestamp = sli_log_api_core->get_timestamp(SL_LOG_HOST_CORE_ID);
  sl_log_overflow_event_local.core_id = 0;
  sl_log_overflow_event_local.flags = SL_LOG_CONFIG_LEVEL_WARN << 1 | 1;
  sl_log_overflow_event_local.arg_count = 1;
  sl_log_overflow_event_local.event_id = SL_LOG_OVERFLOW_EVENT_ID;
  sl_log_overflow_event_local.args[0] = overflow_count;
  sl_log_overflow_event_local.version = 1;
  sl_log_overflow_event=sl_log_overflow_event_local;
}

uint8_t sl_log_get_backend_status(void)
{
  uint8_t ok = false;
  __disable_irq();
  if (sl_log_backend_status.backend_transfer_done) {
    sl_log_backend_status.backend_transfer_done = 0;  // claim
    ok = true;
  }
  __enable_irq();
  return ok;
}

/** @} (end addtogroup sl_log_api_implementation) */

/** @} (end addtogroup sl_log) */
