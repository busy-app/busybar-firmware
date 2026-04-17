/********************************************************************************
 * @file  xapi.c
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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "x_bit.h"
#include "xapi.h"
#include "x_compat.h" // for WEAK
#include "x_config.h"
#include "app_xapi.h"

#define X_ASSERT(...)

struct app_xapi_packet *app_internal_xapi_cmd_msg_buf =
  NULL; // buffer for storing current command, used by host and native API.
struct app_xapi_packet *app_internal_xapi_rsp_msg_buf =
  NULL; // buffer for storing current response, used for host and native API.

static struct xapi_device *registered_xapi_devices = NULL;

static inline struct xapi_device *get_xapi_device(enum app_xapi_dev_types dev_type)
{
  struct xapi_device *d = registered_xapi_devices;

  while (d && d->dev_type != dev_type) {
    d = d->next;
  }

  return d;
}

static const struct xapi_lib *get_xapi_library(uint32_t cmdheader)
{
  // Get the device
  uint8_t dev_type      = APP_XAPI_MSG_DEVICE_TYPE(cmdheader);
  struct xapi_device *d = get_xapi_device((enum app_xapi_dev_types)dev_type);
  if (!d) {
    return NULL;
  }

  // Use the right library table depending on the state of the device
  const struct xapi_lib *const *ptr = (d->flags & app_internal_xapi_device_flag_started) ? d->lib_table
                                                                                         : d->lib_table_stopped;

  // Get the library
  uint8_t cls_name = APP_INTERNAL_XAPI_MSG_CLASS_ID(cmdheader);
  while (*ptr) {
    const struct xapi_lib *lib = *ptr;
    if (lib->cls_name == cls_name) {
      return lib;
    }
    ptr++;
  }

  return NULL;
}

/**
 * Finds a XAPI command message from given lookup tables.
 */
static app_xapi_handler find_command_impl(uint8_t command_id, const struct xapi_lib *lib)
{
  const struct xapi_cmd *msg = lib ? lib->cmd_table : 0;
  for (; msg && msg->handler; msg++) {
    if (msg->command_id == command_id) {
      return msg->handler;
    }
  }

  return NULL;
}

// Initialize command, response and event buffers for API handling
void xapi_buffer_init(void *(*alloc)(size_t size))
{
  // Allocate buffers to fit the header and the maximum possible payload
  size_t xapi_cmdbuf_len        = APP_XAPI_MSG_HEADER_LEN + APP_XAPI_MAX_PAYLOAD_SIZE;
  app_internal_xapi_cmd_msg_buf = (struct app_xapi_packet *)alloc(xapi_cmdbuf_len);
  app_internal_xapi_rsp_msg_buf = (struct app_xapi_packet *)alloc(xapi_cmdbuf_len);
}

// Initialize XAPI internal state
void xapi_init(void)
{
  registered_xapi_devices = NULL;
  xapi_buffer_init(malloc);
}

// Initialize command, response and event buffers for API handling

/**
 * @brief Count the number of libraries in the library table
 *
 * @param[in] lib_table Library table to count the libraries in
 *
 * @return The number of libraries in the table
 */
static uint32_t count_xapi_libraries(const struct xapi_lib *const *lib_table)
{
  uint32_t num_libraries = 0;
  if (lib_table) {
    while (lib_table[num_libraries] != NULL) {
      num_libraries++;
    }
  }

  return num_libraries;
}

/**
 * @brief De-initialize the specified number of XAPI libraries
 *
 * @param[in] lib_table Library table
 * @param[in] num_libraries Number of libraries to de-initialize
 */
static void deinit_xapi_libraries(const struct xapi_lib *const *lib_table, uint32_t num_libraries)
{
  // De-initialize in the opposite order of initialization, i.e. higher index
  // first
  while (num_libraries > 0) {
    num_libraries--;
    const struct xapi_lib *lib = lib_table[num_libraries];

    // Call the de-init function if the lib has it
    if (lib->deinit_func) {
      lib->deinit_func();
    }
  }
}

// Register a XAPI device
void xapi_register_device(struct xapi_device *xapi_device)
{
  // Find the end of the list and check for existing registration
  struct xapi_device **p_next = &registered_xapi_devices;
  while (*p_next != NULL) {
    struct xapi_device *device = *p_next;
    if (device->dev_type == xapi_device->dev_type) {
      // Already registered
      return;
    }

    p_next = &device->next;
  }

  // Link the new device to the end of the list
  xapi_device->flags |= (uint8_t)app_internal_xapi_device_flag_registered;
  xapi_device->next = NULL;
  *p_next           = xapi_device;
}

// Initialize a XAPI device
sl_status_t xapi_init_device(struct xapi_device *xapi_device)
{
  // If the device has not yet registered, register it first
  if ((xapi_device->flags & app_internal_xapi_device_flag_registered) == 0) {
    xapi_register_device(xapi_device);
  }

  // We're now committed to starting, so the device enters the "started" state
  // before the calls to the init functions.
  uint8_t stop_flags = (uint8_t)app_internal_xapi_device_flag_stopping_ongoing
                       | (uint8_t)app_internal_device_flag_stopping_ready;
  xapi_device->flags &= (uint8_t)~stop_flags;
  xapi_device->flags |= (uint8_t)app_internal_xapi_device_flag_started;

  // Initialize the libraries
  uint32_t i                              = 0;
  const struct xapi_lib *const *lib_table = xapi_device->lib_table;
  while (lib_table[i] != NULL) {
    const struct xapi_lib *lib = lib_table[i++];

    // Call the init function if the lib has it
    if (lib->init_func) {
      sl_status_t status = lib->init_func();
      if (status != SL_STATUS_OK) {
        // If failed, de-initialize this lib and any that we already
        // successfully initialized
        deinit_xapi_libraries(lib_table, i);
        xapi_device->flags &= (uint8_t)~app_internal_xapi_device_flag_started;
        return status;
      }
    }
  }

  return SL_STATUS_OK;
}

// Gracefully stop a XAPI device
void xapi_stop_device(struct xapi_device *xapi_device)
{
  // Set the flags to indicate that stopping is on-going
  uint8_t flags_to_clear = (uint8_t)app_internal_xapi_device_flag_started
                           | (uint8_t)app_internal_device_flag_stopping_ready;
  xapi_device->flags &= (uint8_t)~flags_to_clear;
  xapi_device->flags |= (uint8_t)app_internal_xapi_device_flag_stopping_ongoing;

  // The count of asynchronous stop operations may toggle between zero and
  // non-zero during the de-init if some asynchronous operations finish already
  // during the de-init sequence. To avoid prematurely setting the
  // "stopping_ready" flag, we count one asynchronous stop over the duration of
  // the library stop loop.
  X_ASSERT(xapi_device->num_async_lib_stops_ongoing == 0);
  xapi_device->num_async_lib_stops_ongoing = 1;

  // Stop in the opposite order of initialization, i.e. higher index first
  const struct xapi_lib *const *lib_table = xapi_device->lib_table;
  uint32_t num_libraries                  = count_xapi_libraries(lib_table);
  while (num_libraries > 0) {
    num_libraries--;
    const struct xapi_lib *lib = lib_table[num_libraries];
    if (lib->stop_func) {
      lib->stop_func();
    }
  }

  // The XAPI stop sequence we started above is now ready, so mark it finished.
  // Individual XAPI libraries may still have asynchronous stopping operations
  // on-going.
  app_internal_xapi_lib_stop_finished(xapi_device);
}

// Increase the count of asynchronous stopping operations
void app_internal_xapi_lib_stop_ongoing(struct xapi_device *xapi_device)
{
  X_ASSERT(xapi_device->num_async_lib_stops_ongoing < UINT16_MAX);
  xapi_device->num_async_lib_stops_ongoing++;
}

// Decrease the count of asynchronous stopping operations
void app_internal_xapi_lib_stop_finished(struct xapi_device *xapi_device)
{
  // Stopping is ready when the last asynchronous stop has finished
  X_ASSERT(xapi_device->num_async_lib_stops_ongoing > 0);
  if (xapi_device->num_async_lib_stops_ongoing > 0) {
    xapi_device->num_async_lib_stops_ongoing--;
  }
  if (xapi_device->num_async_lib_stops_ongoing == 0) {
    // Stopping is now finished
    xapi_device->flags &= (uint8_t)~app_internal_xapi_device_flag_stopping_ongoing;
    xapi_device->flags |= (uint8_t)app_internal_device_flag_stopping_ready;
  }
}

// De-initialize a XAPI device
void xapi_deinit_device(struct xapi_device *xapi_device)
{
  // De-init the libraries
  uint32_t num_libraries = count_xapi_libraries(xapi_device->lib_table);
  deinit_xapi_libraries(xapi_device->lib_table, num_libraries);

  // The device is now completely de-initialized. Clear all relevant state
  // flags.
  xapi_device->num_async_lib_stops_ongoing = 0;
  uint8_t flags_to_clear                   = (uint8_t)app_internal_xapi_device_flag_started
                           | (uint8_t)app_internal_xapi_device_flag_stopping_ongoing
                           | (uint8_t)app_internal_device_flag_stopping_ready;
  xapi_device->flags &= (uint8_t)~flags_to_clear;
}

// Return true if the specified XAPI device is started
bool app_internal_xapi_device_is_started(const struct xapi_device *xapi_device)
{
  return ((xapi_device->flags & app_internal_xapi_device_flag_started) != 0);
}

// Return true if the specified XAPI device has finished stopping
bool app_internal_xapi_device_has_finished_stopping(const struct xapi_device *xapi_device)
{
  return ((xapi_device->flags & app_internal_device_flag_stopping_ready) != 0);
}

// Return true if the specified XAPI device is stopped
bool app_internal_xapi_device_is_stopped(const struct xapi_device *xapi_device)
{
  uint8_t active_flags = (uint8_t)app_internal_xapi_device_flag_started
                         | (uint8_t)app_internal_xapi_device_flag_stopping_ongoing
                         | (uint8_t)app_internal_device_flag_stopping_ready;
  return ((xapi_device->flags & active_flags) == 0);
}

void api_internal_xapi_cmd_handler_delegate(uint32_t header, app_xapi_handler handler, const void *data)
{
  // Make sure the command buffer has the header
  app_internal_xapi_cmd_msg_buf->header = header;

  // Clear the response header to allow us to detect cases when the handler
  // returns without having set a response.
  app_internal_xapi_rsp_msg_buf->header = 0;

  sl_status_t status         = SL_STATUS_FAIL;
  const struct xapi_lib *lib = get_xapi_library(header);
  uint8_t command_id         = API_INTERNAL_XAPI_MSG_COMMAND_ID(header);
  if (lib && !handler) {
    handler = find_command_impl(command_id, lib);
  }
  if (!lib || !handler) {
    status = SL_STATUS_NOT_SUPPORTED;
  } else {
    handler(data);

    // If the handler failed to set a response, set a generic failure
    if (app_internal_xapi_rsp_msg_buf->header == 0) {
      status = SL_STATUS_FAIL;
    } else {
      status = SL_STATUS_OK;
    }
  }

  // If we failed to execute the handler, set a generic error response
  if (status != SL_STATUS_OK) {
    commander_rsp_error(APP_XAPI_MSG_DEVICE_TYPE(header),
                        APP_INTERNAL_XAPI_MSG_CLASS_ID(header),
                        API_INTERNAL_XAPI_MSG_COMMAND_ID(header),
                        status);
  }
}

void (*api_internal_xapi_cmd_handler_delegate_function)(uint32_t,
                                                        app_xapi_handler,
                                                        const void *) = api_internal_xapi_cmd_handler_delegate;

void app_internal_xapi_set_cmd_handler_delegate(void (*cmd_handler_delegate)(uint32_t, app_xapi_handler, const void *))
{
  api_internal_xapi_cmd_handler_delegate_function = cmd_handler_delegate;
}

const struct xapi_soc_cmd *current_cmd_decl = NULL;

/* Start the handling of XAPI command with no variable-length data */
sl_status_t app_internal_xapi_start_soc_command(const struct xapi_soc_cmd *cmd_decl)
{
  // Must have a declaration
  X_ASSERT(cmd_decl != NULL);

  // Lock
  sl_status_t status = app_xapi_lock();
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Commit to processing this command
  current_cmd_decl                      = cmd_decl;
  app_internal_xapi_cmd_msg_buf->header = cmd_decl->command_header;

  return SL_STATUS_OK;
}

/* Start the handling of XAPI command with variable-length data */
sl_status_t app_internal_xapi_start_soc_command_with_data(const struct xapi_soc_cmd *cmd_decl,
                                                          size_t length_field_size,
                                                          size_t data_len,
                                                          const void *data)
{
  // Must have a declaration
  X_ASSERT(cmd_decl != NULL);

  // First get the length of the fixed-size part of the command and verify the data fits
  uint32_t xapi_header    = cmd_decl->command_header;
  uint32_t cmd_header_len = APP_XAPI_MSG_LEN(xapi_header);
  uint32_t msg_len        = cmd_header_len + data_len;
  if (msg_len > APP_XAPI_MAX_PAYLOAD_SIZE) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // The data fits so we can start the handling. Lock and commit to processing this command.
  sl_status_t status = app_xapi_lock();
  if (status != SL_STATUS_OK) {
    return status;
  }
  current_cmd_decl = cmd_decl;

  // Set the final header with the full message length including the variable-length data
  uint8_t dev_type   = APP_XAPI_MSG_DEVICE_TYPE(xapi_header);
  uint8_t class_id   = APP_INTERNAL_XAPI_MSG_CLASS_ID(xapi_header);
  uint8_t command_id = API_INTERNAL_XAPI_MSG_COMMAND_ID(xapi_header);
  app_internal_xapi_cmd_msg_buf->header =
    API_INTERNAL_XAPI_MSG_HEADER(class_id, command_id, (uint8_t)app_xapi_msg_type_cmd | (uint8_t)dev_type, msg_len);

  // The data buffer for the variable-length data is always after the command header. Copy the data
  // first.
  uint8_t *data_buffer = ((uint8_t *)&app_internal_xapi_cmd_msg_buf->data) + cmd_header_len;
  memcpy(data_buffer, data, data_len);

  // The length field is always just before the data buffer but its type and length depends on the
  // type of the array parameter.
  if (length_field_size == sizeof(uint8array)) {
    // The array is a `uint8array` with an 8-bit length field
    uint8array *array_8bit = (uint8array *)(data_buffer - sizeof(uint8array));
    if (data_len > UINT8_MAX) {
      return SL_STATUS_WOULD_OVERFLOW;
    }
    array_8bit->len = (uint8_t)data_len;
  } else if (length_field_size == sizeof(byte_array)) {
    // The array is a `byte_array` with a 16-bit length field. The length can never overflow the
    // 16-bit value, as the XAPI message length limitation that we checked is always smaller.
    byte_array *array_16bit = (byte_array *)(data_buffer - sizeof(byte_array));
    X_ASSERT(data_len <= UINT16_MAX);
    array_16bit->len = (uint16_t)data_len;
  } else {
    // An unknown size can only occur if the generated code is somehow broken
    X_ASSERT(false);
  }

  return SL_STATUS_OK;
}

/* Call the command handler delegate for the current XAPI command */
sl_status_t app_internal_xapi_call_delegate_for_soc_command()
{
  // Processing must have been started already with app_internal_xapi_start_soc_command()
  X_ASSERT(current_cmd_decl != NULL);
  X_ASSERT(current_cmd_decl->command_handler != NULL);

  // Call the delegate
  api_internal_xapi_cmd_handler_delegate_function(app_internal_xapi_cmd_msg_buf->header,
                                                  current_cmd_decl->command_handler,
                                                  &app_internal_xapi_cmd_msg_buf->data);

  // Return the response from the response structure. The result code, if any, is always at a fixed
  // position and this is verified by the tools that generate the XAPI files.
  return app_internal_xapi_rsp_msg_buf->data.rsp_error.result;
}

/* End the handling of XAPI command */
void app_internal_xapi_end_soc_command()
{
  // If the implementation of a XAPI command handler calls another command, we will return here and
  // clear the command declaration even while the outer command is still being processed. That's OK,
  // though. The command declaration is only needed momentarily between
  // app_internal_xapi_start_soc_command() and app_internal_xapi_call_delegate_and_end_soc_command(), and no other
  // calls will happen between those. We can always clear this here to make sure we'll detect any
  // attempt to call the SoC delegate without having start the command handling first.
  current_cmd_decl = NULL;
  app_xapi_unlock();
}

/* Call the command handler delegate and end processing the current XAPI command */
sl_status_t app_internal_xapi_call_delegate_and_end_soc_command()
{
  sl_status_t status = app_internal_xapi_call_delegate_for_soc_command();
  app_internal_xapi_end_soc_command();
  return status;
}

/* Handle a XAPI command in binary format */
void app_xapi_handle_command(uint32_t header, const void *data)
{
  api_internal_xapi_cmd_handler_delegate_function(header, NULL, data);
}

uint8_t app_wifi_is_sensitive_message(uint32_t message_id)
{
  const struct xapi_lib *lib = get_xapi_library(message_id);

  if (!lib) {
    return 0;
  }

  uint8_t command_id = API_INTERNAL_XAPI_MSG_COMMAND_ID(message_id);
  for (const struct xapi_cmd *msg = lib->cmd_table; msg && msg->handler; msg++) {
    if (msg->command_id == command_id) {
      return msg->metadata.sensitive ? 1 : 0;
    }
  }

  return 0;
}

void *app_xapi_get_command_response(void)
{
  return app_internal_xapi_rsp_msg_buf;
}

// Set a generic error response to the specified buffer.
void app_xapi_set_error_response(uint32_t command_hdr, uint16_t result, void *response, size_t response_buf_size)
{
  // We can't trust the command header, so we'll rebuild a new header from its
  // parts. Some of the parts may be zeros if the NCP implementation does not
  // have a full command, but that's OK. XAPI commands are processed one
  // command at a time and the recipient will be able to handle the error
  // response even if some IDs are zeros.
  uint8_t device_type   = APP_XAPI_MSG_DEVICE_TYPE(command_hdr);
  uint8_t class_id      = APP_INTERNAL_XAPI_MSG_CLASS_ID(command_hdr);
  uint8_t command_id    = API_INTERNAL_XAPI_MSG_COMMAND_ID(command_hdr);
  uint32_t response_hdr = API_INTERNAL_XAPI_MSG_HEADER(class_id,
                                                       command_id,
                                                       (uint8_t)app_xapi_msg_type_rsp | device_type,
                                                       sizeof(struct app_xapi_rsp_error_s));

  // Fill the output if there's space
  if (response_buf_size >= sizeof(response_hdr) + sizeof(struct app_xapi_rsp_error_s)) {
    struct app_xapi_packet *xapi_msg = response;
    xapi_msg->header                 = response_hdr;
    xapi_msg->data.rsp_error.result  = result;
  }
}

uint32_t xapi_message_size()
{
  return sizeof(struct app_xapi_packet);
}

// Returns whether or not there are events in any of the queues for the user.
// uint8_t commander_check_evt()
// {
//   return get_first_device_with_events() ? 1 : 0;
// }

// Create a XAPI response with no variable-length data.
void *commander_create_xapi_response(uint32_t rsp_header)
{
  // Set the XAPI header
  app_internal_xapi_rsp_msg_buf->header = rsp_header;

  // Signal the need to update
  // commander_stack_update_callback();

  return (void *)app_internal_xapi_rsp_msg_buf;
}

// Create a XAPI response with variable-length data in contiguous memory.
struct app_xapi_packet *commander_create_xapi_response_data(uint32_t rsp_header, size_t data_len, const void *data)
{
  // Extract the types, IDs, and the length of the fixed-size part of the response
  uint8_t dev_type         = APP_XAPI_MSG_DEVICE_TYPE(rsp_header);
  uint8_t class_id         = APP_INTERNAL_XAPI_MSG_CLASS_ID(rsp_header);
  uint8_t command_id       = API_INTERNAL_XAPI_MSG_COMMAND_ID(rsp_header);
  uint32_t evt_header_size = APP_XAPI_MSG_LEN(rsp_header);

  // The combined size may not exceed the available XAPI payload size
  X_ASSERT(evt_header_size + data_len <= XAPI_PAYLOAD_SIZE);

  // Either the data length must be zero or we must have data
  X_ASSERT((data_len == 0) || (data != NULL));

  // Get the uint8array that starts at the last byte of the response-specific structure and set the
  // size
  uint8array *data_array = (uint8array *)(((uint8_t *)&app_internal_xapi_rsp_msg_buf->data) + evt_header_size - 1);
  data_array->len        = (uint8_t)data_len;

  // If the data is not already in the right place, copy it now. We allow callers to use the buffer
  // as temporary storage, so use "memmove" instead of "memcpy" just in case the data is already
  // somewhere in the middle of the buffer.
  if (data && (data != data_array->data)) {
    memmove(data_array->data, data, data_len);
  }

  // Construct the full header and let the no-data version fill the parts common to all responses
  uint32_t xapi_size = evt_header_size + data_len;
  uint32_t full_header =
    API_INTERNAL_XAPI_MSG_HEADER(class_id, command_id, (uint8_t)app_xapi_msg_type_rsp | (uint8_t)dev_type, xapi_size);
  return commander_create_xapi_response(full_header);
}
