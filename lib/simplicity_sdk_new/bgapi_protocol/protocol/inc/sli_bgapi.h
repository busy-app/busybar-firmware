/***************************************************************************//**
 * @brief Silicon Labs internal API provided by the BGAPI protocol
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

#ifndef SLI_BGAPI_H
#define SLI_BGAPI_H

#include <stdbool.h>
#include "sl_status.h"
#include "sl_bgapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond DOXYGEN_INCLUDE_INTERNAL */

/***************************************************************************//**
 * @addtogroup sli_bgapi BGAPI Protocol internal API
 * @{
 *
 * @brief Silicon Labs internal API to the BGAPI Protocol
 *
 * The BGAPI Protocol provides some services that are needed internally by
 * Silicon Labs components. The APIs provided by this file must not be used by
 * application code.
 */

// -----------------------------------------------------------------------------
// Silicon Labs internal types for the BGAPI protocol

/**
 * @brief Function prototype for early init of a component of a BGAPI device.
 *
 * Functions of this type are used internally by BGAPI device implementations
 * for the early initialization of the components of the device. Applications
 * must not use the functions directly. Use the documented public APIs for
 * starting and stopping wireless stacks that are implemented as BGAPI devices.
 *
 * @param[in] config Pointer to the component-specific configuration structure
 *   or NULL if the component has no configuration
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
typedef sl_status_t sli_bgapi_component_init_func_t (const void *config);

/**
 * @brief Function prototype for starting a component of a BGAPI device.
 *
 * Functions of this type are used internally by BGAPI device implementations to
 * start the components of the device. Applications must not use the functions
 * directly. Use the documented public APIs for starting and stopping wireless
 * stacks that are implemented as BGAPI devices.
 *
 * @param[in] config Pointer to the component-specific configuration structure
 *   or NULL if the component has no configuration
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
typedef sl_status_t sli_bgapi_component_start_func_t (const void *config);

/**
 * @brief Function prototype for stopping a component of a BGAPI device.
 *
 * Functions of this type are used internally by BGAPI device implementations to
 * stop the components of the device. Applications must not use the functions
 * directly. Use the documented public APIs for starting and stopping wireless
 * stacks that are implemented as BGAPI devices.
 *
 * @param[in] mode The mode of stopping the BGAPI device
 */
typedef void sli_bgapi_component_stop_func_t (uint32_t mode);

/**
 * @brief Function prototype for de-initializing a component of a BGAPI device.
 *
 * Functions of this type are used internally by BGAPI device implementations to
 * de-initialize the components of the device. Applications must not use the
 * functions directly. Use the documented public APIs for starting and stopping
 * wireless stacks that are implemented as BGAPI devices.
 */
typedef void sli_bgapi_component_deinit_func_t (void);

/**
 * @brief Structure to define the information for component init function.
 *
 * Structures of this type are used internally to define the init function and
 * configuration for each component of a BGAPI device. The structures are
 * collected in an array based on the selection of SLC components included in
 * the application. Applications must not edit the structures or the array. Use
 * the documented configuration mechanisms to select which components to
 * include.
 */
typedef struct {
  /** Pointer to the init function */
  sli_bgapi_component_init_func_t *init_function;

  /** Pointer to the component-specific configuration structure */
  const void *config;
} sli_bgapi_component_init_info_t;

/**
 * @brief Structure to define the information for component start function.
 *
 * Structures of this type are used internally to define the start function and
 * configuration for each component of a BGAPI device. The structures are
 * collected in an array based on the selection of SLC components included in
 * the application. Applications must not edit the structures or the array. Use
 * the documented configuration mechanisms to select which components to
 * include.
 */
typedef struct {
  /** Pointer to the start function */
  sli_bgapi_component_start_func_t *start_function;

  /** Pointer to the component-specific configuration structure */
  const void *config;
} sli_bgapi_component_start_info_t;

/** @brief Forward declaration of the BGAPI class structure */
struct sli_bgapi_class;

/** @brief Forward declaration of the BGAPI device structure */
typedef struct bgapi_device sli_bgapi_device_t;

/**
 * @brief Structure to collect the read-only information of one BGAPI device.
 *
 * BGAPI Protocol implementation never writes into this structure. Instances of
 * this structure are meant to be stored in flash memory.
 */
typedef struct {
  /**
   * The array of structures that define the init function and configuration
   * for each BGAPI component that belongs to this device and is included in the
   * build. The array is terminated by NULL in @p init_function.
   */
  const sli_bgapi_component_init_info_t *component_init_info;

  /**
   * The array of structures that define the start function and configuration
   * for each BGAPI component that belongs to this device and is included in the
   * build. The array is terminated by NULL in @p start_function.
   */
  const sli_bgapi_component_start_info_t *component_start_info;

  /**
   * NULL-terminated array of pointers to the stop function for each BGAPI
   * component that belongs to this device and is included in the build.
   */
  sli_bgapi_component_stop_func_t * const *component_stop_functions;

  /**
   * NULL-terminated array of pointers to the deinit function for each BGAPI
   * component that belongs to this device and is included in the build.
   */
  sli_bgapi_component_deinit_func_t * const *component_deinit_functions;

  /**
   * NULL-terminated array of pointers to declarations of those classes that are
   * available when the BGAPI device has been started.
   */
  const struct sli_bgapi_class * const *bgapi_classes_when_started;

  /**
   * NULL-terminated array of pointers to declarations of those classes that are
   * available when the BGAPI device has been stopped.
   */
  const struct sli_bgapi_class * const *bgapi_classes_when_stopped;
} sli_bgapi_device_info_t;

/**
 * @brief Type definition for the command handler delegate function.
 *
 * The command handler delegate is used by the BGAPI protocol to handle a
 * command. The default delegate @ref sli_bgapi_cmd_handler_delegate executes
 * the BGAPI command handler function as a direct function call. The RTOS
 * adaptation uses @ref sli_bgapi_set_cmd_handler_delegate to set a special
 * delegate that handles the required inter-process communication (IPC) when an
 * RTOS is used.
 *
 * @param[in] handler The handler function to process the BGAPI command
 * @param[in] command_buf Pointer to the full BGAPI command message including
 *   the header and the command-specific payload
 * @param[out] response_buf Pointer to the response buffer to fill. It is
 *   acceptable to point to the same buffer as the command buffer in @p
 *   command_buf.
 * @param[in] response_buf_size The size of the response buffer
 *
 * @return SL_STATUS_OK if the command was executed, otherwise an error code.
 *   Note that SL_STATUS_OK does not mean that the command was successful, only
 *   that it was executed.
 */
typedef sl_status_t sli_bgapi_cmd_handler_delegate_t (sl_bgapi_handler handler,
                                                      const void* command_buf,
                                                      void *response_buf,
                                                      size_t response_buf_size);

#define SLI_BGAPI_BUFFER_SIZE (SL_BGAPI_MSG_HEADER_LEN + SL_BGAPI_MAX_PAYLOAD_SIZE)

/**
 * @brief The default command handler delegate function.
 */
sli_bgapi_cmd_handler_delegate_t sli_bgapi_cmd_handler_delegate;

// -----------------------------------------------------------------------------
// Silicon Labs internal macros provided by the BGAPI protocol

/**
 * @brief Macro to construct a BGAPI message header.
 *
 * It is the caller's responsibility to verify that the input values are within
 * valid range.
 *
 * @param[in] class_id The class ID
 * @param[in] command_id The command ID
 * @param[in] type Type field including message and device type
 * @param[in] payload_len Length of the full message payload
 *
 * @return The header as a `uint32_t` value
 */
#define SLI_BGAPI_MSG_HEADER(class_id, command_id, type, payload_len) \
  (uint32_t) (((uint32_t) (command_id) << 24)                         \
              | ((uint32_t) (class_id) << 16)                         \
              | (((uint32_t) (payload_len) & 0xFF) << 8)              \
              | ((uint32_t) (payload_len) >> 8) | ((uint32_t) (type)))

// -----------------------------------------------------------------------------
// Silicon Labs internal functions to control the BGAPI protocol

/**
 * @brief Set the BGAPI command handler delegate.
 *
 * @param[in] cmd_handler_delegate Pointer to the command handler delegate function
 */
void sli_bgapi_set_cmd_handler_delegate(sli_bgapi_cmd_handler_delegate_t *cmd_handler_delegate);

/**
 * @brief Get the maximum BGAPI event size of a specific device.
 *
 * When events are popped using @ref sli_bgapi_pop_event, the caller supplies
 * the event buffer to pop the event to. This function can be used to query the
 * maximum BGAPI event size (including the header and the payload) that the
 * specified device can produce. The caller can then allocate a suitable large
 * buffer to use with @ref sli_bgapi_pop_event.
 *
 * @param[in] dev_type The BGAPI device type
 * @param[out] max_event_message_size The maximum event size for this device
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
sl_status_t sli_bgapi_get_max_event_size(uint8_t dev_type,
                                         size_t *max_event_buffer_size);

/**
 * @brief Check if the specified device has BGAPI events in its queue.
 *
 * @return true if the device has events in its queue, false otherwise.
 */
bool sli_bgapi_device_has_event(const sli_bgapi_device_t *bgapi_device);

/**
 * @brief Get the total length of the first event in a device's event queue.
 *
 * The total length includes the BGAPI header size and the length of the
 * event-specific payload.
 *
 * @param[in] bgapi_device The BGAPI device to check for events
 *
 * @return The total length of the next event in the queue, or 0 if there are no
 *   events.
 */
size_t sli_bgapi_device_peek_event_len(const sli_bgapi_device_t *bgapi_device);

/**
 * @brief Pop an event out of the BGAPI event queue of a specific device.
 *
 * @param[in] dev_type The BGAPI device to pop events from
 * @param[in] event_buffer_size The size of the supplied event buffer
 * @param[out] event_buffer The event buffer to receive the popped event
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
sl_status_t sli_bgapi_pop_event(uint8_t dev_type,
                                size_t event_buffer_size,
                                void *event_buffer);

/**
 * @brief Pop an event out of the BGAPI event queue of a specific device.
 *
 * @param[in] bgapi_device The internal device structure of the BGAPI device
 * @param[in] event_buffer_size The size of the supplied event buffer
 * @param[out] event_buffer The event buffer to receive the popped event
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
sl_status_t sli_bgapi_device_pop_event(sli_bgapi_device_t *bgapi_device,
                                       size_t event_buffer_size,
                                       void *event_buffer);

/**
 * @brief Enable the specified BGAPI device to support task-internal calls.
 *
 * In the RTOS environment the BGAPI commands are executed by an RTOS task and
 * the message passing from calling tasks uses shared memory with mutex
 * protection. If BGAPI commands of a BGAPI device need to be used from within
 * the task that is executing the commands, an additional message buffer needs
 * to be allocated to service the task-internal calls.
 *
 * This function is called by components that need to enable task-internal calls
 * for a specific BGAPI device. When the enable call has succeeded and
 * task-internal calls are no longer needed, the caller must call @ref
 * sli_bgapi_disable_task_internal_calls to release the resources. The functions
 * operate as a pair and count the number of clients that need task-internal
 * calls. When the last client calls the disable, the extra resources are
 * released.
 *
 * @param[in] device_type The type of the BGAPI device, as enum @ref
 *   sl_bgapi_dev_types, that needs to support task-internal calls
 *
 * @return SL_STATUS_OK if successful, otherwise an error code
 */
sl_status_t sli_bgapi_enable_task_internal_calls(uint8_t device_type);

/**
 * @brief Disable the specified BGAPI device from supporting task-internal
 * calls.
 *
 * See @ref sli_bgapi_enable_task_internal_calls for information on the purpose
 * and use of this function.
 *
 * @param[in] device_type The type of the BGAPI device, as enum @ref
 *  sl_bgapi_dev_types, that no longer needs to support task-internal calls
 */
void sli_bgapi_disable_task_internal_calls(uint8_t device_type);

/** @} end sli_bgapi */

/** @endcond */ // DOXYGEN_INCLUDE_INTERNAL

#ifdef __cplusplus
}
#endif

#endif // SLI_BGAPI_H
