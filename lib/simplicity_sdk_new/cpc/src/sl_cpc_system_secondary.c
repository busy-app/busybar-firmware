/***************************************************************************/ /**
 * @file
 * @brief CPC System Endpoint RCP API implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_status.h"
#include "sli_cpc.h"
#include "sli_cpc_assert.h"
#include "sli_cpc_bootloader.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_hdlc.h"
#include "sli_cpc_instance.h"
#include "sli_cpc_memory.h"
#include "sli_cpc_system_secondary.h"
#include "sli_cpc_system_common.h"
#include "sl_atomic.h"
#include "sl_cpc.h"
#include "sl_cpc_config.h"
#include "sl_common.h"

#if (defined(SL_CATALOG_CPC_SECURITY_PRESENT))
#include "sli_cpc_security.h"
#endif

#define CPC_PACKED_ENDPOINT_PRESENT 0   // Not yet implemented

// On write complete argument magic words
#define ON_WRITE_COMPLETE_RESET       0xDEADBEEF
#define ON_WRITE_COMPLETE_CONNECT     0xCAFEBABE
#define ON_WRITE_COMPLETE_SHUTDOWN    0xBEEFBABE
#define ON_WRITE_COMPLETE_EP_ID_MASK  0xFF

/*******************************************************************************
 ***************************  GLOBAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * bootloader_reboot_mode.
 * 0 - At the next reboot application is executed.
 * 1 -  At the next reboot bootloader is executed. Depending on the
 *      configuration of the bootloader, it either updates the application or
 *      starts an NCP mode.
 ******************************************************************************/
static sli_cpc_system_reboot_mode_t prop_bootloader_reboot_mode = REBOOT_APPLICATION;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

static void on_write_completed(sl_cpc_user_endpoint_id_t endpoint_id,
                               void *buffer,
                               void *arg,
                               sl_status_t status);

static sl_status_t open_endpoint(sli_cpc_system_endpoint_t *system_ep);
static void on_system_ep_error(uint8_t endpoint_id, void *arg);

static sl_status_t send_endpoint_state(sli_cpc_system_endpoint_t *ep,
                                       uint8_t endpoint_id,
                                       sl_cpc_endpoint_state_t state);

#if (!defined(SLI_CPC_DEVICE_UNDER_TEST))
static void send_reset_reason(sli_cpc_system_endpoint_t *system_ep);
#endif

static void enter_irq_timer_callback(sli_cpc_timer_handle_t *handle,
                                     void *data);

#if defined(SLI_CPC_ENABLE_TEST_FEATURES)
SL_WEAK void system_on_information_received(uint8_t endpoint_id, void *arg);
#endif
SL_WEAK const char* sl_cpc_secondary_app_version(void);

/***************************************************************************//**
 * Called when secondary app version is requested.
 * The format is up to the user. The string should be null terminated.
 ******************************************************************************/
SL_WEAK const char* sl_cpc_secondary_app_version(void)
{
  return "UNDEFINED";
}

/***************************************************************************//**
 * Initialize CPC System
 ******************************************************************************/
static sl_status_t open_endpoint(sli_cpc_system_endpoint_t *system_ep)
{
  sl_status_t status;

  status = sli_cpc_open_service_endpoint_in_instance(&system_ep->handle,
                                                     sli_cpc_instance_from_system_ep(system_ep),
                                                     SL_CPC_ENDPOINT_SYSTEM,
                                                     SL_CPC_ENDPOINT_FLAG_UFRAME_ENABLE,
                                                     1);
  if (status == SL_STATUS_ALREADY_EXISTS) {
    return SL_STATUS_ALREADY_INITIALIZED;
  } else if (status != SL_STATUS_OK) {
    return status;
  }

#if defined(SLI_CPC_ENABLE_TEST_FEATURES)
  if (system_on_information_received) {
    status = sl_cpc_set_endpoint_option(&system_ep->handle,
                                        SL_CPC_ENDPOINT_ON_UFRAME_RECEIVE,
                                        (void *)system_on_information_received);
    if (status != SL_STATUS_OK) {
      sl_cpc_close_endpoint(&system_ep->handle);
      return status;
    }
  }
#endif

  status = sl_cpc_set_endpoint_option(&system_ep->handle,
                                      SL_CPC_ENDPOINT_ON_UFRAME_WRITE_COMPLETED,
                                      (void *)on_write_completed);
  if (status != SL_STATUS_OK) {
    sl_cpc_close_endpoint(&system_ep->handle);
    return status;
  }

  status = sl_cpc_set_endpoint_option(&system_ep->handle,
                                      SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED,
                                      (void *)on_write_completed);
  if (status != SL_STATUS_OK) {
    sl_cpc_close_endpoint(&system_ep->handle);
    return status;
  }

  status = sl_cpc_set_endpoint_option(&system_ep->handle,
                                      SL_CPC_ENDPOINT_ON_ERROR,
                                      (void *)on_system_ep_error);
  if (status != SL_STATUS_OK) {
    sl_cpc_close_endpoint(&system_ep->handle);
    return status;
  }

  status = sl_cpc_set_endpoint_option(&system_ep->handle,
                                      SL_CPC_ENDPOINT_ON_ERROR_ARG,
                                      (void *)system_ep);
  if (status != SL_STATUS_OK) {
    sl_cpc_close_endpoint(&system_ep->handle);
    return status;
  }

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Returns the state of the reset sequence
 ******************************************************************************/
bool sli_cpc_reboot_is_sequence_done(sli_cpc_system_endpoint_t *system_ep)
{
  return system_ep->reset_sequence_done;
}

/***************************************************************************//**
 * Process the system endpoint
 ******************************************************************************/
void sli_cpc_system_process(sli_cpc_system_endpoint_t *system_ep)
{
  if (system_ep->stop_flag) {
    sl_cpc_close_endpoint(&system_ep->handle);
    system_ep->stop_flag = false;
  }

  if (system_ep->process_uframes_flag) {
    uint16_t data_length;
    sl_status_t status;
    void *data;

    status = sl_cpc_read(&system_ep->handle,
                         &data,
                         &data_length,
                         0,
                         SL_CPC_FLAG_NO_BLOCK | SL_CPC_FLAG_UNNUMBERED_INFORMATION);
    if (status == SL_STATUS_OK) {
      if (data_length > 0) {
        system_ep->fc_validation_value += ((uint8_t *)data)[0];
      }
      sl_cpc_free_rx_buffer(data);
    }
  }
}

/***************************************************************************//**
 * Start CPC System
 ******************************************************************************/
sl_status_t sli_cpc_system_start(sli_cpc_system_endpoint_t *system_ep)
{
  sl_status_t status;
  status = open_endpoint(system_ep);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }
#if (!defined(SLI_CPC_DEVICE_UNDER_TEST))
  send_reset_reason(system_ep);
#endif
  return status;
}

/***************************************************************************//**
 * Initialize CPC System
 ******************************************************************************/
sl_status_t sli_cpc_system_init(sli_cpc_system_endpoint_t *system_ep)
{
  system_ep->enter_irq_end_ms = 0;
  system_ep->fc_validation_value = 0;
  system_ep->process_uframes_flag = false;
  system_ep->stop_flag = false;
  system_ep->reset_sequence_done = false;

  memset(&system_ep->enter_irq_timer, 0, sizeof(system_ep->enter_irq_timer));
  memset(&system_ep->handle, 0, sizeof(system_ep->handle));
  memset(&system_ep->primary_version, 0, sizeof(system_ep->primary_version));

#if defined(CPC_TEST_HEADER_SYNC)
  system_ep->process_uframes_flag = true;
#endif

#if defined(SLI_CPC_DEVICE_UNDER_TEST)
  system_ep->reset_sequence_done = true;
#else
  (void)system_ep;
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * System endpoint on error callback
 ******************************************************************************/
static void on_system_ep_error(uint8_t endpoint_id, void *arg)
{
  sli_cpc_system_endpoint_t *system_ep = (sli_cpc_system_endpoint_t*)arg;

  SLI_CPC_ASSERT(endpoint_id == SL_CPC_ENDPOINT_SYSTEM);
  SLI_CPC_ASSERT(system_ep != NULL);

  system_ep->stop_flag = true;
}

/***************************************************************************//**
 * System endpoint on write completed callback
 ******************************************************************************/
static void on_write_completed(sl_cpc_user_endpoint_id_t endpoint_id,
                               void *buffer,
                               void *arg,
                               sl_status_t status)
{
  sli_cpc_system_cmd_context_t *context = (sli_cpc_system_cmd_context_t*)arg;
  sli_cpc_instance_t *inst = sli_cpc_instance_from_system_ep(context->ep);
  uint8_t cmd_ep_id;
  uintptr_t magic;

  (void) endpoint_id;

  // make sure both are set or both are NULL
  SLI_CPC_ASSERT((buffer == NULL && arg == NULL)
                 || (buffer != NULL && arg != NULL));

  // Unnumbered ACK has no payload
  if (buffer == NULL) {
    return;
  }

  magic = (uintptr_t)context->on_complete_arg;

  if (magic == ON_WRITE_COMPLETE_RESET) {
    // accept the soft reset only if rebooting in application mode. If user
    // wants to enter bootloader mode, there is no alternatives but to do
    // a hard reset.
    if (inst->soft_reset && prop_bootloader_reboot_mode == REBOOT_APPLICATION) {
      sli_cpc_reset(inst);
    } else {
      cpc_system_reset(prop_bootloader_reboot_mode);
    }
  } else if ((magic & ~ON_WRITE_COMPLETE_EP_ID_MASK)
             == (ON_WRITE_COMPLETE_CONNECT & ~ON_WRITE_COMPLETE_EP_ID_MASK)) {
    cmd_ep_id = magic & ON_WRITE_COMPLETE_EP_ID_MASK;

    sli_cpc_instance_on_remote_ep_connected(sli_cpc_instance_from_system_ep(context->ep),
                                            cmd_ep_id,
                                            status);
  } else if ((magic & ~ON_WRITE_COMPLETE_EP_ID_MASK)
             == (ON_WRITE_COMPLETE_SHUTDOWN & ~ON_WRITE_COMPLETE_EP_ID_MASK)) {
    cmd_ep_id = magic & ON_WRITE_COMPLETE_EP_ID_MASK;

    // status is irrelevant. If remote failed to ACK the final disconnection message,
    // go to disconnected state regardless.
    sli_cpc_instance_on_remote_ep_disconnected(sli_cpc_instance_from_system_ep(context->ep),
                                               cmd_ep_id);
  }

  sli_cpc_free_system_command_context(sli_cpc_instance_from_system_ep(context->ep), context);
}

static sl_status_t send_endpoint_state(sli_cpc_system_endpoint_t *system_ep,
                                       uint8_t endpoint_id,
                                       sl_cpc_endpoint_state_t state)
{
  sli_cpc_system_property_cmd_t *tx_prop_command;
  sli_cpc_system_cmd_context_t *context;
  sl_cpc_endpoint_state_t *ep_state;
  sli_cpc_system_cmd_t *tx_command;
  sl_status_t status;

  status = sli_cpc_get_system_command_context(sli_cpc_instance_from_system_ep(system_ep), &context);
  if (status != SL_STATUS_OK) {
    return status;
  }

  context->ep = system_ep;
  context->ep_id = endpoint_id;

  tx_command = &context->command;
  tx_command->command_id = CMD_SYSTEM_PROP_VALUE_IS;
  tx_command->seq = 0;
  tx_command->length = sizeof(sli_cpc_system_property_cmd_t) + sizeof(sl_cpc_endpoint_state_t);

  tx_prop_command = (sli_cpc_system_property_cmd_t*)(tx_command->payload);
  tx_prop_command->property_id = EP_ID_TO_PROPERTY_STATE(endpoint_id);

  ep_state = (sl_cpc_endpoint_state_t *)tx_prop_command->payload;
  *ep_state = state;

  status = sl_cpc_write(&system_ep->handle,
                        (void*)tx_command,
                        SIZEOF_SYSTEM_COMMAND(tx_command),
                        SL_CPC_FLAG_NO_BLOCK,
                        context);
  return status;
}

/***************************************************************************//**
 * Endpoint was opened, notify the host/primary
 ******************************************************************************/
sl_status_t sli_cpc_system_send_opening_notification(sli_cpc_system_endpoint_t *system_ep,
                                                     uint8_t endpoint_id)
{
  return send_endpoint_state(system_ep, endpoint_id, SL_CPC_STATE_OPEN);
}

/***************************************************************************//**
 * Endpoint connection was terminated, notify the host
 ******************************************************************************/
sl_status_t sli_cpc_system_send_terminate_notification(sli_cpc_system_endpoint_t *system_ep,
                                                       uint8_t endpoint_id)
{
  return send_endpoint_state(system_ep, endpoint_id, SL_CPC_STATE_CLOSED);
}

/***************************************************************************//**
 * Endpoint is requesting a shutdown, notify the host
 ******************************************************************************/
sl_status_t sli_cpc_system_send_shutdown_request(sli_cpc_system_endpoint_t *system_ep,
                                                 uint8_t endpoint_id)
{
  return send_endpoint_state(system_ep, endpoint_id, SL_CPC_STATE_DISCONNECTED);
}

/***************************************************************************//**
 * Send reset reason
 ******************************************************************************/
#if (!defined(SLI_CPC_DEVICE_UNDER_TEST))
static void send_reset_reason(sli_cpc_system_endpoint_t *system_ep)
{
  sli_cpc_system_property_cmd_t *tx_prop_command;
  sl_cpc_system_status_t *tx_prop_last_status;
  sli_cpc_system_cmd_context_t *context;
  sli_cpc_system_cmd_t *tx_command;
  sl_status_t status;

  status = sli_cpc_get_system_command_context(sli_cpc_instance_from_system_ep(system_ep), &context);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);

  context->ep = system_ep;

  tx_command = &context->command;
  tx_command->command_id = CMD_SYSTEM_PROP_VALUE_IS;
  tx_command->seq = 0; /* Seq is irrelevant in u-frame information */

  tx_prop_command = (sli_cpc_system_property_cmd_t*)(tx_command->payload);

  tx_prop_command->property_id = PROP_LAST_STATUS;

  tx_prop_last_status = (sl_cpc_system_status_t*)tx_prop_command->payload;

  *tx_prop_last_status = cpc_get_reset_reason();

  tx_command->length = sizeof(sli_cpc_system_property_cmd_t) + sizeof(sl_cpc_system_status_t);

  status = sl_cpc_write(&system_ep->handle,
                        (void*)tx_command,
                        SIZEOF_SYSTEM_COMMAND(tx_command),
                        SL_CPC_FLAG_NO_BLOCK | SL_CPC_FLAG_UNNUMBERED_INFORMATION,
                        context);

  SLI_CPC_ASSERT(status == SL_STATUS_OK); //Ignore error
  SL_CPC_JOURNAL_RECORD_INFO("Sent reset reason", 0);
}
#endif

/*******************************************************************************
 ***************************  PROPERTY-GET HANDLERS  ***************************
 ******************************************************************************/

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_LAST_STATUS
 *   When replying to any CMD_PROPERTY_GET/SET which result in an unsuccessful
 *   operation, the RCP will reply with PROP_LAST_STATUS instead. Here, this
 *   property is specifically queried. There is nothing more meaningful than
 *   to reply with a STATUS_OK.
 ******************************************************************************/
static void on_property_get_last_status(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_LAST_STATUS;
  *((sl_cpc_system_status_t*)(prop_cmd_buff->payload)) = STATUS_OK;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_system_status_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_PROTOCOL_VERSION
 *   Ship the hardcoded protocol version number back to the primary.
 ******************************************************************************/
static void on_property_get_protocol_version(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_PROTOCOL_VERSION;
  prop_cmd_buff->payload[0] = SLI_CPC_PROTOCOL_VERSION;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint8_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_SECONDARY_CPC_VERSION
 *   Ship the hardcoded major and minor version number back to the primary.
 ******************************************************************************/
static void on_property_get_secondary_cpc_version(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;
  uint32_t* version;

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_SECONDARY_CPC_VERSION;
  version = (uint32_t*)(prop_cmd_buff->payload);

  version[0] = SL_CPC_MAJOR_VERSION;
  version[1] = SL_CPC_MINOR_VERSION;
  version[2] = SL_CPC_PATCH_VERSION;

  tx_command->length = sizeof(sli_cpc_property_id_t) + (3 * sizeof(uint32_t));
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_SECONDARY_APP_VERSION
 *   Send a string version of the secondary application to the primary
 ******************************************************************************/
static void on_property_get_secondary_app_version(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;
  const char* app_version;
  uint16_t app_version_len;

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_SECONDARY_APP_VERSION;
  app_version = sl_cpc_secondary_app_version();

  app_version_len = strlen(app_version) + 1;

  if (app_version_len > SLI_CPC_PROPERTY_PAYLOAD_LENGTH_MAX) {
    app_version_len = SLI_CPC_PROPERTY_PAYLOAD_LENGTH_MAX;
  }

  memcpy(prop_cmd_buff->payload, app_version, app_version_len);

  tx_command->length = sizeof(sli_cpc_property_id_t) + app_version_len;
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_CAPABILITIES
 *   Send the capabilities of the secondary to the primary
 ******************************************************************************/
static void on_property_get_capabilities(sli_cpc_system_endpoint_t *system_ep,
                                         sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;
  sli_cpc_instance_t *inst;
  uint32_t *capabilities;

  SLI_CPC_ASSERT(system_ep != NULL);
  inst = sli_cpc_instance_from_system_ep(system_ep);

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_CAPABILITIES;
  capabilities = (uint32_t*)(prop_cmd_buff->payload);

  *capabilities = SL_CPC_ENDPOINT_SECURITY_ENABLED * CPC_CAPABILITIES_SECURITY_ENDPOINT_MASK
                  + CPC_PACKED_ENDPOINT_PRESENT * CPC_CAPABILITIES_PACKED_ENDPOINT_MASK
                  + SLI_CPC_ENDPOINT_GPIO_ENABLED * CPC_CAPABILITIES_GPIO_ENDPOINT_MASK
                  + inst->sli_cpc_driver_capabilities.uart_flowcontrol * CPC_CAPABILITIES_UART_FLOW_CONTROL_MASK;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint32_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_RX_CAPABILITY
 *   Send the rx buffer capability of the secondary to the primary
 ******************************************************************************/
static void on_property_get_rx_capabilities(sli_cpc_system_endpoint_t *system_ep,
                                            sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;
  uint16_t *rx_capability;

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_RX_CAPABILITY;
  rx_capability = (uint16_t *)(prop_cmd_buff->payload);
  *rx_capability = sli_cpc_instance_from_system_ep(system_ep)->rx_user_payload_max_length;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint16_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_FC_VALIDATION_VALUE
 *   Send the flow control validation value of the secondary to the primary
 ******************************************************************************/
static void on_property_get_fc_validation_value(sli_cpc_system_endpoint_t *system_ep,
                                                sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;
  uint32_t *flow_control_validation_value;

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_FC_VALIDATION_VALUE;
  flow_control_validation_value = (uint32_t *)(prop_cmd_buff->payload);
  *flow_control_validation_value = system_ep->fc_validation_value;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint32_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_BUS_BITRATE_VALUE
 *   Send the bus bitrate value of the secondary to the primary.
 ******************************************************************************/
static void on_property_get_bus_bitrate_value(sli_cpc_system_endpoint_t *system_ep,
                                              sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;
  uint32_t *bus_bitrate_value;
  sli_cpc_instance_t *inst;

  SLI_CPC_ASSERT(system_ep != NULL);
  inst = sli_cpc_instance_from_system_ep(system_ep);

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_BUS_BITRATE_VALUE;
  bus_bitrate_value = (uint32_t *)(prop_cmd_buff->payload);
  *bus_bitrate_value = sli_cpc_instance_bus_get_bitrate(inst);

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint32_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_BUS_MAX_BITRATE_VALUE
 *   Send the bus max bitrate value of the secondary to the primary.
 ******************************************************************************/
static void on_property_get_bus_max_bitrate_value(sli_cpc_system_endpoint_t *system_ep,
                                                  sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *prop_cmd_buff;
  uint32_t *bus_max_bitrate_value;
  sli_cpc_instance_t *inst;

  SLI_CPC_ASSERT(system_ep != NULL);
  inst = sli_cpc_instance_from_system_ep(system_ep);

  prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  prop_cmd_buff->property_id = PROP_BUS_MAX_BITRATE_VALUE;
  bus_max_bitrate_value = (uint32_t *)(prop_cmd_buff->payload);
  *bus_max_bitrate_value = sli_cpc_instance_bus_get_max_bitrate(inst);

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint32_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_BOOTLOADER_INFO
 *   Reply to the PRIMARY the bootloader infos.
 ******************************************************************************/
static void on_property_get_bootloader_info(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *tx_property;
  sli_cpc_system_bootloader_info_t *infos;

  tx_property = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property->property_id = PROP_BOOTLOADER_INFO;
  infos = (sli_cpc_system_bootloader_info_t*)(tx_property->payload);

  cpc_get_bootloader_info(infos);

  tx_command->length = sizeof(sli_cpc_property_id_t) + 3 * sizeof(uint32_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_BOOTLOADER_REBOOT_MODE
 *   Reply to the PRIMARY the bootloader reboot mode.
 ******************************************************************************/
static void on_property_get_bootloader_reboot_mode(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *tx_property;
  sli_cpc_system_reboot_mode_t* mode;

  tx_property = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property->property_id = PROP_BOOTLOADER_REBOOT_MODE;
  mode = (sli_cpc_system_reboot_mode_t*)(tx_property->payload);
  *mode = prop_bootloader_reboot_mode;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sli_cpc_system_reboot_mode_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_SECURITY_STATE
 *   Reply to the PRIMARY the security state.
 ******************************************************************************/
static void on_property_get_security_state(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *tx_property;
  uint32_t* security_state;

  tx_property = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  security_state = (uint32_t*)(tx_property->payload);

#ifdef SL_CATALOG_CPC_SECURITY_PRESENT
  tx_property->property_id = PROP_SECURITY_STATE;
  *security_state = sl_cpc_security_get_state();
#else
  tx_property->property_id = PROP_LAST_STATUS;
  *security_state = STATUS_UNIMPLEMENTED;
#endif

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint32_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_ENDPOINT_STATE_x
 *   The primary queried the status of a specific endpoint number.
 ******************************************************************************/
static void on_property_get_endpoint_state(sli_cpc_system_cmd_t *tx_command,
                                           uint8_t ep_id)
{
  sli_cpc_system_property_cmd_t *reply_prop_cmd_buff;
  sl_cpc_endpoint_state_t *reply_ep_state;
  sl_cpc_endpoint_handle_t dummy_ep;

  reply_prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  reply_ep_state = (sl_cpc_endpoint_state_t*) reply_prop_cmd_buff->payload;
  dummy_ep.ep = NULL;
  dummy_ep.id = ep_id;

  reply_prop_cmd_buff->property_id = EP_ID_TO_PROPERTY_STATE(ep_id);

  *reply_ep_state = sl_cpc_get_endpoint_state(&dummy_ep);

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_endpoint_state_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_ENDPOINT_ENCRYPTION_x
 *   The primary queried the encryption state of a specific endpoint number.
 ******************************************************************************/
static void on_property_get_endpoint_encryption(sli_cpc_system_cmd_t *tx_command,
                                                uint8_t ep_id)
{
  sli_cpc_system_property_cmd_t *reply_prop_cmd_buff;
  sl_cpc_endpoint_handle_t dummy_ep;
  bool *reply_ep_encryption;

  dummy_ep.ep = NULL;
  dummy_ep.id = ep_id;

  reply_prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  reply_prop_cmd_buff->property_id = EP_ID_TO_PROPERTY_ENCRYPTION(ep_id);

  reply_ep_encryption = (bool*) reply_prop_cmd_buff->payload;
  *reply_ep_encryption = sl_cpc_get_endpoint_encryption(&dummy_ep);

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(bool);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_ENDPOINT_STATES
 *   The primary queried the status of all endpoints.
 ******************************************************************************/
static void on_property_get_endpoint_states(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *reply_prop_cmd_buff;
  uint8_t *reply_ep_states;

  reply_prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  reply_ep_states = (uint8_t*)(reply_prop_cmd_buff->payload);
  reply_prop_cmd_buff->property_id = PROP_ENDPOINT_STATES;

  // process 2 endpoints per iteration
  for (size_t i = 0; i != SL_CPC_ENDPOINT_MAX_COUNT / 2; i++) {
    sl_cpc_endpoint_handle_t dummy_ep1 = { .ep = NULL, .id = 2 * i };
    sl_cpc_endpoint_handle_t dummy_ep2 = { .ep = NULL, .id = 2 * i + 1 };

    sl_cpc_endpoint_state_t ep1_state = sl_cpc_get_endpoint_state(&dummy_ep1);
    sl_cpc_endpoint_state_t ep2_state = sl_cpc_get_endpoint_state(&dummy_ep2);

    // Although an 'sl_cpc_endpoint_state_t' is an 8 bit value, the number of
    // values in the enum makes it possible to encode it with a nibble (4 bits)
    // as only 3 bits are required to encode those 6 values. Put the first
    // endpoint in the low nibble and the second in the high nibble.
    // This aggregation will make it possible to send all 256 endpoint states
    // in one reply as it will fit within 255 bytes (limited by the length field
    // within a command frame)
    reply_ep_states[i] = (ep2_state << 4) | ep1_state;
  }

  tx_command->length = sizeof(sli_cpc_property_id_t) + (SL_CPC_ENDPOINT_MAX_COUNT * sizeof(uint8_t) / 2);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_GET
 * Property ID: PROP_DEBUG_COUNTERS
 *   The primary queried the debug counters.
 ******************************************************************************/
#if (SL_CPC_DEBUG_CORE_EVENT_COUNTERS == 1)
static void on_property_get_core_debug_counters(sli_cpc_system_endpoint_t *system_ep,
                                                sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *reply_prop_cmd_buff;
  sli_cpc_instance_t *inst;

  SLI_CPC_ASSERT(system_ep != NULL);
  inst = sli_cpc_instance_from_system_ep(system_ep);

  reply_prop_cmd_buff = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  memcpy(reply_prop_cmd_buff->payload, &inst->core_debug.core_counters, sizeof(sl_cpc_core_debug_counters_t));

  reply_prop_cmd_buff->property_id = PROP_CORE_DEBUG_COUNTERS;
  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_core_debug_counters_t);
}
#endif

/***************************************************************************//**
 * Handler for when the primary asks about a property not found:
 *   As with any property-get/set which is unsuccessful, the rcp replies with
 *   a property id of PROP_LAST_STATUS. Since the property the primary asked about
 *   can't be handled by the rcp, the status returned is STATUS_PROP_NOT_FOUND.
 ******************************************************************************/
static void on_property_get_set_not_found(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_cmd;

  tx_property_cmd = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property_cmd->property_id = PROP_LAST_STATUS;
  *((sl_cpc_system_status_t*)(tx_property_cmd->payload)) = STATUS_PROP_NOT_FOUND;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_system_status_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_SET:
 * Property ID: PROP_ENDPOINT_STATE_x
 *   The primary notifies the secondary of an endpoint state change
 ******************************************************************************/
static void on_property_set_endpoint_state(sli_cpc_system_endpoint_t *system_ep,
                                           uint8_t endpoint_id,
                                           sli_cpc_system_cmd_t *tx_command,
                                           sli_cpc_system_cmd_t *rx_command,
                                           void **on_write_complete_arg)
{
  sli_cpc_system_property_cmd_t *tx_prop_command = (sli_cpc_system_property_cmd_t*)(tx_command->payload);
  sli_cpc_system_property_cmd_t *rx_prop_command = (sli_cpc_system_property_cmd_t*)(rx_command->payload);
  sl_cpc_endpoint_state_t *ep_state = (sl_cpc_endpoint_state_t *)rx_prop_command->payload;
  uintptr_t magic;
  sl_status_t status;

  tx_command->length = sizeof(sli_cpc_system_property_cmd_t) + sizeof(sl_cpc_endpoint_state_t);
  tx_prop_command->property_id = EP_ID_TO_PROPERTY_STATE(endpoint_id);

  if (*ep_state == SL_CPC_STATE_CLOSING || *ep_state == SL_CPC_STATE_CLOSED) {
    // Terminate notification.
    sli_cpc_instance_on_remote_ep_terminated(sli_cpc_instance_from_system_ep(system_ep),
                                             endpoint_id,
                                             (sl_cpc_endpoint_state_t *)tx_prop_command->payload);
  } else if (*ep_state == SL_CPC_STATE_CONNECTED) {
    // Connection request.
    status = sli_cpc_instance_on_remote_ep_connecting(sli_cpc_instance_from_system_ep(system_ep),
                                                      endpoint_id,
                                                      (sl_cpc_endpoint_state_t *)tx_prop_command->payload);
    if (status == SL_STATUS_OK) {
      // Only notify the core that the connect command has finished when it
      // is accepted.
      magic = (ON_WRITE_COMPLETE_CONNECT & ~ON_WRITE_COMPLETE_EP_ID_MASK) | endpoint_id;
      *on_write_complete_arg = (void*)magic;
    }
  } else if (*ep_state == SL_CPC_STATE_DISCONNECTED) {
    // Shutdown request
    status = sli_cpc_instance_on_remote_ep_shutdown(sli_cpc_instance_from_system_ep(system_ep),
                                                    endpoint_id,
                                                    (sl_cpc_endpoint_state_t *)tx_prop_command->payload);
    if (status == SL_STATUS_OK) {
      // endpoint successfully shutdown, go to DISCONNECTED on write completed
      magic = (ON_WRITE_COMPLETE_SHUTDOWN & ~ON_WRITE_COMPLETE_EP_ID_MASK) | endpoint_id;
      *on_write_complete_arg = (void*)magic;
    }
  } else {
    // Unsupported endpoint state
    SLI_CPC_ASSERT(0);
    tx_command->length = sizeof(sli_cpc_property_id_t);
    tx_prop_command->property_id = PROP_LAST_STATUS;
    *((sl_cpc_system_status_t*)(tx_prop_command->payload)) = STATUS_INVALID_ARGUMENT;
  }
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_SET:
 * Property ID: PROP_UFRAME_PROCESSING
 *   The primary enables/disables u-frame processing on the secondary.
 ******************************************************************************/
static void on_property_set_uframe_processing(sli_cpc_system_endpoint_t *system_ep,
                                              sli_cpc_system_cmd_t *tx_command,
                                              sli_cpc_system_cmd_t *rx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_command;
  sli_cpc_system_property_cmd_t *rx_property_command;

  rx_property_command = (sli_cpc_system_property_cmd_t*) rx_command->payload;
  system_ep->process_uframes_flag = *(bool*)rx_property_command->payload;

  tx_property_command = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property_command->property_id = PROP_UFRAME_PROCESSING;
  tx_command->length = sizeof(sli_cpc_property_id_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_SET:
 * Property ID: PROP_ENTER_IRQ
 *   The primary requests the secondary to enter IRQ in start_ms for end_ms time.
 ******************************************************************************/
static void on_property_set_enter_irq(sli_cpc_system_endpoint_t *system_ep,
                                      sli_cpc_system_cmd_t *tx_command,
                                      sli_cpc_system_cmd_t *rx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_command;
  sli_cpc_system_property_cmd_t *rx_property_command;
  sli_cpc_system_enter_irq_cmd_t *enter_irq_command;

  rx_property_command = (sli_cpc_system_property_cmd_t*) rx_command->payload;
  enter_irq_command = (sli_cpc_system_enter_irq_cmd_t*) rx_property_command->payload;
  system_ep->enter_irq_end_ms = enter_irq_command->end_in_ms;
  sli_cpc_timer_start_timer_ms(&system_ep->enter_irq_timer,
                               enter_irq_command->start_in_ms,
                               enter_irq_timer_callback,
                               &system_ep->enter_irq_end_ms);

  tx_property_command = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property_command->property_id = PROP_ENTER_IRQ;
  tx_command->length = sizeof(sli_cpc_property_id_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_SET:
 * Property ID: PROP_PRIMARY_VERSION_VALUE
 *   The primary sets its version.
 ******************************************************************************/
static void on_property_set_primary_version_value(sli_cpc_system_endpoint_t *system_ep,
                                                  sli_cpc_system_cmd_t *tx_command,
                                                  sli_cpc_system_cmd_t *rx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_command;
  sli_cpc_system_property_cmd_t *rx_property_command;
  const sli_cpc_system_primary_version_t *primary_version_payload;

  rx_property_command = (sli_cpc_system_property_cmd_t*) rx_command->payload;
  primary_version_payload = (sli_cpc_system_primary_version_t*) rx_property_command->payload;
  memcpy(&system_ep->primary_version, primary_version_payload, sizeof(sli_cpc_system_primary_version_t));

  tx_property_command = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property_command->property_id = PROP_PRIMARY_VERSION_VALUE;
  tx_command->length = sizeof(sli_cpc_property_id_t);
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_SET:
 * Property ID: PROP_BOOTLOADER_REBOOT_MODE
 *   The primary sets the reboot mode.
 ******************************************************************************/
static void on_property_set_bootloader_reboot_mode(sli_cpc_system_cmd_t *tx_command,
                                                   sli_cpc_system_cmd_t *rx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_command;
  sli_cpc_system_property_cmd_t *rx_property_command;
  sli_cpc_system_reboot_mode_t *rx_mode;
  sli_cpc_system_reboot_mode_t *tx_mode;

  tx_property_command = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  rx_property_command = (sli_cpc_system_property_cmd_t*) rx_command->payload;
  rx_mode = (sli_cpc_system_reboot_mode_t*) rx_property_command->payload;
  tx_mode = (sli_cpc_system_reboot_mode_t*) tx_property_command->payload;

  tx_command->length = sizeof(sli_cpc_property_id_t);

  switch (*rx_mode) {
    case REBOOT_APPLICATION:
      /* We can always reset to the application */
      prop_bootloader_reboot_mode = REBOOT_APPLICATION;
      tx_property_command->property_id = PROP_BOOTLOADER_REBOOT_MODE;
      *tx_mode = REBOOT_APPLICATION;
      tx_command->length += sizeof(sli_cpc_system_reboot_mode_t);
      break;

    case REBOOT_BOOTLOADER:
      /* Check if the bootloader interface is present */

#if (defined(SL_CPC_HAS_BOOTLOADER_SUPPORT))
      prop_bootloader_reboot_mode = REBOOT_BOOTLOADER;
      *tx_mode = REBOOT_BOOTLOADER;
#else
      prop_bootloader_reboot_mode = REBOOT_APPLICATION; /* We don't support bootloader reboot, stay in application reboot */
      *tx_mode = REBOOT_APPLICATION; // Reply that we stay in reboot application mode
#endif
      tx_property_command->property_id = PROP_BOOTLOADER_REBOOT_MODE;
      tx_command->length += sizeof(sli_cpc_system_reboot_mode_t);
      break;

    default:
      // The requested bootloader mode doesn't exist. Reply with a
      // PROP_LAST_STATUS::STATUS_INVALID_ARGUMENT instead
      tx_property_command->property_id = PROP_LAST_STATUS;
      *((sl_cpc_system_status_t*)(tx_property_command->payload)) = STATUS_INVALID_ARGUMENT;
      tx_command->length += sizeof(sl_cpc_system_status_t);
      break;
  }
}

/***************************************************************************//**
 * Command ID:  CMD_PROPERTY_SET:
 * Property ID: PROP_SOFT_RESET
 *   The primary sets the soft reset property. When the host requests this device
 *   to reboot, it will try to do a soft reset instead of a NVIC reset. Some
 *   modes do no permit a soft reset, like when a reboot in bootloader mode is
 *   requested.
 ******************************************************************************/
static void on_property_set_soft_reset(sli_cpc_system_endpoint_t *system_ep,
                                       sli_cpc_system_cmd_t *tx_command,
                                       sli_cpc_system_cmd_t *rx_command)
{
  sli_cpc_instance_t *inst = sli_cpc_instance_from_system_ep(system_ep);
  sli_cpc_system_property_cmd_t *tx_property_command;
  sli_cpc_system_property_cmd_t *rx_property_command;
  bool soft_reset;

  rx_property_command = (sli_cpc_system_property_cmd_t*) rx_command->payload;
  soft_reset = *((uint8_t*) rx_property_command->payload);

  tx_property_command = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property_command->property_id = PROP_SOFT_RESET;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_system_status_t);
  *((sl_cpc_system_status_t*)(tx_property_command->payload)) = STATUS_OK;

  inst->soft_reset = soft_reset;
}

static void on_property_set_endpoint_encryption(uint8_t endpoint_id,
                                                sli_cpc_system_cmd_t *tx_command,
                                                sli_cpc_system_cmd_t *rx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_command;

  tx_property_command = (sli_cpc_system_property_cmd_t*)tx_command->payload;

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  sli_cpc_system_property_cmd_t *rx_property_command;
  sl_status_t status;
  uint32_t primary_rx_counter;
  uint32_t primary_tx_counter;

  rx_property_command = (sli_cpc_system_property_cmd_t*)rx_command->payload;

  // Check the length before processing RX payload
  if (rx_command->length != (sizeof(sli_cpc_property_id_t) + 2 * sizeof(uint32_t))) {
    tx_property_command->property_id = PROP_LAST_STATUS;
    *((sl_cpc_system_status_t*)(tx_property_command->payload)) = STATUS_INVALID_ARGUMENT;
    tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_system_status_t);

    return;
  }

  memcpy(&primary_rx_counter, rx_property_command->payload, sizeof(uint32_t));
  memcpy(&primary_tx_counter, rx_property_command->payload + sizeof(uint32_t), sizeof(uint32_t));

  // update in the core
  status = sli_cpc_set_security_counters(endpoint_id, primary_rx_counter, primary_tx_counter);
  if (status != SL_STATUS_OK) {
    sl_cpc_system_status_t system_status = STATUS_FAILURE;

    tx_property_command->property_id = PROP_LAST_STATUS;
    tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_system_status_t);

    if (status == SL_STATUS_NOT_READY || status == SL_STATUS_INVALID_STATE) {
      // endpoint not allocated or not in the open state
      system_status = STATUS_INVALID_STATE;
    }

    *((sl_cpc_system_status_t*)(tx_property_command->payload)) = system_status;
  } else {
    tx_property_command->property_id = EP_ID_TO_PROPERTY_ENCRYPTION(endpoint_id);
    memcpy(tx_property_command->payload, rx_property_command->payload, 2 * sizeof(uint32_t));

    tx_command->length = sizeof(sli_cpc_property_id_t) + 2 * sizeof(uint32_t);
  }
#else
  (void)endpoint_id;
  (void)rx_command;

  tx_property_command->property_id = PROP_LAST_STATUS;

  *((sl_cpc_system_status_t*)(tx_property_command->payload)) = STATUS_UNIMPLEMENTED;
  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_system_status_t);
#endif
}

static void on_property_set_tx_capability(sli_cpc_system_endpoint_t *system_ep,
                                          sli_cpc_system_cmd_t *tx_command,
                                          sli_cpc_system_cmd_t *rx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_command;
  sli_cpc_system_property_cmd_t *rx_property_command;
  uint16_t* rx_capability;
  uint16_t* tx_capability;

  tx_property_command = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  rx_property_command = (sli_cpc_system_property_cmd_t*) rx_command->payload;
  rx_capability = (uint16_t*) rx_property_command->payload;
  tx_capability = (uint16_t*) tx_property_command->payload;

  tx_property_command->property_id = PROP_TX_CAPABILITY;
  *tx_capability = *rx_capability;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(uint16_t);

  sli_cpc_set_remote_tx_max_payload_length(sli_cpc_instance_from_system_ep(system_ep),
                                           *rx_capability);
}

/***************************************************************************//**
 * Handler for when the primary sets a read-only property
 ******************************************************************************/
static void on_property_set_read_only(sli_cpc_system_cmd_t *tx_command)
{
  sli_cpc_system_property_cmd_t *tx_property_cmd;

  tx_property_cmd = (sli_cpc_system_property_cmd_t*) tx_command->payload;
  tx_property_cmd->property_id = PROP_LAST_STATUS;
  *((sl_cpc_system_status_t*)(tx_property_cmd->payload)) = STATUS_INVALID_COMMAND_FOR_PROP;

  tx_command->length = sizeof(sli_cpc_property_id_t) + sizeof(sl_cpc_system_status_t);
}

/*******************************************************************************
 ***************************  COMMAND HANDLERS  ********************************
 ******************************************************************************/

/***************************************************************************//**
 * Handle no-op from PRIMARY:
 *   This functions is called when a no-op command is received from the PRIMARY.
 *   The RCP simply sends a no-op back so that the primary can assert the success
 *   of the operation.
 ******************************************************************************/
static void on_noop(sli_cpc_system_cmd_t *noop,
                    uint32_t *reply_data_length)
{
  noop->command_id = CMD_SYSTEM_NOOP;
  noop->length = 0;
  *reply_data_length = SIZEOF_SYSTEM_COMMAND(noop);
}

/***************************************************************************//**
 * Handle reset request
 ******************************************************************************/
SL_WEAK sl_cpc_system_status_t cpc_secondary_on_reset_request(sli_cpc_system_reboot_mode_t reboot_mode)
{
  switch (reboot_mode) {
    case REBOOT_APPLICATION:
      // We can always reboot into application
      return STATUS_OK;
  #if (defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT))
    case REBOOT_BOOTLOADER:
    {
      BootloaderInformation_t btl_info;
      bootloader_getInfo(&btl_info);

      // In case of bootloader reboot, check that a bootloader is present
      if (btl_info.type == NO_BOOTLOADER ) {
        return STATUS_FAILURE;
      }
      return STATUS_OK;
    }
  #endif
    default:
      SLI_CPC_ASSERT(0);
      break;
  }

  return STATUS_FAILURE;
}

/***************************************************************************//**
 * Handle reset from PRIMARY:
 *   This functions is called when a reset command is received from the PRIMARY.
 *   The secondary will emit back a reset command with a status indicating
 *   wheter or not it can reset in the desired mode dictated by 'prop_bootloader
 *   _reboot_mode'
 ******************************************************************************/
static void on_reset(sli_cpc_system_cmd_t *reset,
                     uint32_t *reply_data_length,
                     void **on_write_complete_arg)
{
  sl_cpc_system_status_t *reset_status;

  reset_status = (sl_cpc_system_status_t*)(reset->payload);
  reset->command_id = CMD_SYSTEM_RESET;

  reset->length = sizeof(sl_cpc_system_status_t);
  *reply_data_length = SIZEOF_SYSTEM_COMMAND(reset);

  *reset_status = cpc_secondary_on_reset_request(prop_bootloader_reboot_mode);

  // Set on_write_complete argument to ON_WRITE_COMPLETE_RESET. This will
  // indicate to the on_write_complete callback to reset the device.
  // We need to postpone the reset to after the
  // write completion.
  if (*reset_status == STATUS_OK) {
    // The reset will be able to occurre in the mode requested, provide a
    // magic number to the callback so it knows it has to reset the device
    *on_write_complete_arg = (void*)ON_WRITE_COMPLETE_RESET;
  } else {
    // The reset won't occur, don't tell the callback to reset
    *on_write_complete_arg = 0;
  }
}

/***************************************************************************//**
 * Handle property-get from PRIMARY:
 *   This functions is called when a property-get command is received from the
 *   PRIMARY. Causes the SECONDARY to emit a "CMD_PROP_VALUE_IS " command for the
 *   given property identifier.
 ******************************************************************************/
static void on_property_get(sli_cpc_system_endpoint_t *system_ep,
                            sli_cpc_system_cmd_t *rx_command,
                            sli_cpc_system_cmd_t *reply,
                            uint32_t *reply_data_length)
{
  sli_cpc_system_property_cmd_t *rx_property_cmd = (sli_cpc_system_property_cmd_t *)rx_command->payload;

  // Reply to a PROPERTY-GET with a PROPERTY-IS
  reply->command_id = CMD_SYSTEM_PROP_VALUE_IS;

  // Populate the reply command buffer according to the property_id
  switch (rx_property_cmd->property_id) {
    case PROP_LAST_STATUS:
      on_property_get_last_status(reply);
      break;

    case PROP_PROTOCOL_VERSION:
      on_property_get_protocol_version(reply);
      break;

    case PROP_SECONDARY_CPC_VERSION:
      on_property_get_secondary_cpc_version(reply);
      break;

    case PROP_SECONDARY_APP_VERSION:
      on_property_get_secondary_app_version(reply);
      system_ep->reset_sequence_done = true;
      break;

    case PROP_CAPABILITIES:
      on_property_get_capabilities(system_ep, reply);
      break;

    case PROP_RX_CAPABILITY:
      on_property_get_rx_capabilities(system_ep, reply);
      break;

    case PROP_FC_VALIDATION_VALUE:
      on_property_get_fc_validation_value(system_ep, reply);
      break;

    case PROP_BUS_BITRATE_VALUE:
      on_property_get_bus_bitrate_value(system_ep, reply);
      break;

    case PROP_BUS_MAX_BITRATE_VALUE:
      on_property_get_bus_max_bitrate_value(system_ep, reply);
      break;

    case PROP_BOOTLOADER_INFO:
      on_property_get_bootloader_info(reply);
      break;

    case PROP_BOOTLOADER_REBOOT_MODE:
      on_property_get_bootloader_reboot_mode(reply);
      break;

    case PROP_SECURITY_STATE:
      on_property_get_security_state(reply);
      break;

    case PROP_ENDPOINT_STATES:
      on_property_get_endpoint_states(reply);
      break;

#if (SL_CPC_DEBUG_CORE_EVENT_COUNTERS == 1)
    case PROP_CORE_DEBUG_COUNTERS:
      on_property_get_core_debug_counters(system_ep, reply);
      break;
#endif

    default:
      // Deal with endpoint state range
      if (rx_property_cmd->property_id >= EP_ID_TO_PROPERTY_STATE(0)
          && rx_property_cmd->property_id <= EP_ID_TO_PROPERTY_STATE(255)) {
        uint8_t ep_id = PROPERTY_ID_TO_EP_ID(rx_property_cmd->property_id);
        on_property_get_endpoint_state(reply, ep_id);
        break;
      }

      if (rx_property_cmd->property_id >= EP_ID_TO_PROPERTY_ENCRYPTION(0)
          && rx_property_cmd->property_id <= EP_ID_TO_PROPERTY_ENCRYPTION(255)) {
        uint8_t ep_id = PROPERTY_ID_TO_EP_ID(rx_property_cmd->property_id);
        on_property_get_endpoint_encryption(reply, ep_id);
        break;
      }

      on_property_get_set_not_found(reply);
      break;
  }

  *reply_data_length = SIZEOF_SYSTEM_COMMAND(reply);
}

/***************************************************************************//**
 * Handle property-set from PRIMARY:
 *   This functions is called when a property-set command is received from the
 *   PRIMARY. Causes the RCP to emit a "CMD_PROP_VALUE_IS " command for the given
 *   property identifier.
 ******************************************************************************/
static void on_property_set(sli_cpc_system_endpoint_t *system_ep,
                            sli_cpc_system_cmd_t* rx_command,
                            sli_cpc_system_cmd_t *reply,
                            uint32_t * reply_data_length,
                            void **on_write_complete_arg)
{
  sli_cpc_system_property_cmd_t *rx_property_cmd;

  rx_property_cmd = (sli_cpc_system_property_cmd_t*)(rx_command->payload);

  // Reply to a PROPERTY-GET with a PROPERTY-IS
  reply->command_id = CMD_SYSTEM_PROP_VALUE_IS;

  // Populate the reply command buffer according to the property_id
  if (rx_property_cmd->property_id >= PROP_ENDPOINT_STATE_0
      && rx_property_cmd->property_id <= PROP_ENDPOINT_STATE_255) {
    on_property_set_endpoint_state(system_ep,
                                   PROPERTY_ID_TO_EP_ID(rx_property_cmd->property_id),
                                   reply,
                                   rx_command,
                                   on_write_complete_arg);
  } else if (rx_property_cmd->property_id >= PROP_ENDPOINT_ENCRYPTION
             && rx_property_cmd->property_id <= PROP_ENDPOINT_ENCRYPTION + 255) {
    on_property_set_endpoint_encryption(PROPERTY_ID_TO_EP_ID(rx_property_cmd->property_id),
                                        reply,
                                        rx_command);
  } else {
    switch (rx_property_cmd->property_id) {
      case PROP_LAST_STATUS:
      case PROP_PROTOCOL_VERSION:
      case PROP_SECONDARY_CPC_VERSION:
      case PROP_SECONDARY_APP_VERSION:
      case PROP_CAPABILITIES:
      case PROP_BOOTLOADER_INFO:
      case PROP_SECURITY_STATE:
      case PROP_ENDPOINT_STATES:
        // All those properties fall through read-only handling
        on_property_set_read_only(reply);
        break;

      case PROP_BOOTLOADER_REBOOT_MODE:
        on_property_set_bootloader_reboot_mode(reply, rx_command);
        break;

      case PROP_TX_CAPABILITY:
        on_property_set_tx_capability(system_ep, reply, rx_command);
        break;

      case PROP_UFRAME_PROCESSING:
        on_property_set_uframe_processing(system_ep, reply, rx_command);
        break;

      case PROP_ENTER_IRQ:
        on_property_set_enter_irq(system_ep, reply, rx_command);
        break;

      case PROP_PRIMARY_VERSION_VALUE:
        on_property_set_primary_version_value(system_ep, reply, rx_command);
        break;

      case PROP_SOFT_RESET:
        on_property_set_soft_reset(system_ep, reply, rx_command);
        break;

      default:
        on_property_get_set_not_found(reply);
        break;
    }
  }

  *reply_data_length = SIZEOF_SYSTEM_COMMAND(reply);
}

/***************************************************************************//**
 * Called by CPC core when uframe/poll is received
 ******************************************************************************/
sl_status_t sli_cpc_system_secondary_on_poll(sli_cpc_system_endpoint_t *system_ep,
                                             sli_cpc_system_ep_frame_type_t frame_type,
                                             void *poll_data,
                                             uint32_t poll_data_length,
                                             void **reply_data,
                                             uint32_t *reply_data_length,
                                             sli_cpc_system_cmd_context_t **context)
{
  sli_cpc_system_cmd_t *rx_command;
  sli_cpc_system_cmd_t *tx_command;
  sl_status_t status;

  // This function should only be called for U-Frames and I-Frames
  SLI_CPC_ASSERT(frame_type == SYSTEM_EP_UFRAME || frame_type == SYSTEM_EP_IFRAME);

  // check if received buffer is big enough to at least be a command header
  if (poll_data_length < SIZEOF_SYSTEM_COMMAND_HEADER) {
    // do no assert, we can get garbage from the bus.
    return SL_STATUS_INVALID_PARAMETER;
  }

  rx_command = (sli_cpc_system_cmd_t *)poll_data;

  // Make sure the length of the payload from the command matches the returned length.
  // If the packet is not crafted properly, drop it.
  if (SIZEOF_SYSTEM_COMMAND(rx_command) != poll_data_length) {
    return SL_STATUS_INVALID_PARAMETER; // Drop packet
  }

  *reply_data = NULL;
  *reply_data_length = 0;
  *context = NULL;

  // Allocate command buffer. Freed on acknowledgment. (On write completion callback)
  // If no more memory, leave the reply data to NULL and let the upper layer retry later
  status = sli_cpc_get_system_command_context(sli_cpc_instance_from_system_ep(system_ep),
                                              context);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // assign the system endpoint associated to this command to
  // be able to get it back in the on_write_completed callback
  (*context)->ep = system_ep;

  // get the command part of the context and assign it to reply_data,
  // that's what we want to send to the secondary
  tx_command = &((*context)->command);
  *reply_data = (void*)tx_command;

  // Assign the sequence number of the request to the reply so the primary can
  // route it back to the right request.
  tx_command->seq = rx_command->seq;

  // Only the reset can be a u-frame (non encrypted)
  if (frame_type == SYSTEM_EP_UFRAME) {
    sli_cpc_system_property_cmd_t *rx_property_cmd = (sli_cpc_system_property_cmd_t *)rx_command->payload;
    switch (rx_command->command_id) {
      case CMD_SYSTEM_RESET:
        on_reset(tx_command,
                 reply_data_length,
                 &((*context)->on_complete_arg));
        break;

      case CMD_SYSTEM_PROP_VALUE_GET:
        if (rx_property_cmd->property_id == PROP_RX_CAPABILITY
            || rx_property_cmd->property_id == PROP_CAPABILITIES
            || rx_property_cmd->property_id == PROP_BUS_BITRATE_VALUE
            || rx_property_cmd->property_id == PROP_BUS_MAX_BITRATE_VALUE
            || rx_property_cmd->property_id == PROP_PROTOCOL_VERSION
            || rx_property_cmd->property_id == PROP_BOOTLOADER_INFO
            || rx_property_cmd->property_id == PROP_SECONDARY_CPC_VERSION
            || rx_property_cmd->property_id == PROP_SECONDARY_APP_VERSION) {
          on_property_get(system_ep,
                          rx_command,
                          tx_command,
                          reply_data_length);
        } else {
          on_property_get_set_not_found(tx_command);
          *reply_data_length = SIZEOF_SYSTEM_COMMAND(tx_command);
        }
        break;

      case CMD_SYSTEM_PROP_VALUE_SET:
        if (rx_property_cmd->property_id == PROP_BOOTLOADER_REBOOT_MODE
            || rx_property_cmd->property_id == PROP_TX_CAPABILITY
            || rx_property_cmd->property_id == PROP_PRIMARY_VERSION_VALUE
            || rx_property_cmd->property_id == PROP_SOFT_RESET) {
          on_property_set(system_ep,
                          rx_command,
                          tx_command,
                          reply_data_length,
                          &((*context)->on_complete_arg));
        } else {
          on_property_get_set_not_found(tx_command);
          *reply_data_length = SIZEOF_SYSTEM_COMMAND(tx_command);
        }
        break;

      default:
        status = SL_STATUS_INVALID_PARAMETER;
        goto error;
    }
  } else if (frame_type == SYSTEM_EP_IFRAME) {
    switch (rx_command->command_id) {
      case CMD_SYSTEM_NOOP:
        on_noop((sli_cpc_system_cmd_t *)*reply_data, reply_data_length);
        break;

      case CMD_SYSTEM_PROP_VALUE_GET:
        on_property_get(system_ep, rx_command, (sli_cpc_system_cmd_t *)*reply_data, reply_data_length);
        break;

      case CMD_SYSTEM_PROP_VALUE_SET:
        on_property_set(system_ep,
                        rx_command,
                        (sli_cpc_system_cmd_t *)*reply_data,
                        reply_data_length,
                        &((*context)->on_complete_arg));
        break;

      default:
        status = SL_STATUS_INVALID_PARAMETER;
        goto error;
    }
  }

  error:
  if (status != SL_STATUS_OK) {
    sli_cpc_free_system_command_context(sli_cpc_instance_from_system_ep(system_ep), *context);
  }

  return status;
}

/***************************************************************************//**
 * Function called when enter_irq_timer expires.
 ******************************************************************************/
static void enter_irq_timer_callback(sli_cpc_timer_handle_t *handle,
                                     void *data)
{
  (void)handle;

  uint32_t now_ms = sli_cpc_timer_tick_to_ms(sli_cpc_timer_get_tick_count());
  uint32_t end_ms = *(uint32_t*) data;
  while ((sli_cpc_timer_tick_to_ms(sli_cpc_timer_get_tick_count()) - now_ms) <= end_ms) ;
}
