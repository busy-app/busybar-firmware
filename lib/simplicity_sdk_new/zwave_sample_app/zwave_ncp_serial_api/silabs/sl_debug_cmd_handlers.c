/**
 * @file sl_debug_cmd_handlers.c
 * @brief Silicon Labs specific Radio Debug command handlers for Z-Wave Serial API
 * @copyright 2025 Silicon Laboratories Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cmd_handlers.h"
#include "SerialAPI.h"
#include "app.h"
#include "serialapi_file.h"
// #include "utils.h"

#include "sl_component_catalog.h"

#ifdef SL_CATALOG_RAIL_UTIL_PTI_PRESENT
#include "em_device.h"
#include "sl_gpio.h"
#include "sl_rail.h"
#include "sl_rail_types.h"
#include "sl_rail_util_pti_config.h"

/**
 * @brief SAPI PTI configuration - baud rate only (4 bytes)
 */
typedef struct {
  uint8_t baud_msb;    /**< Baud rate MSB */
  uint8_t baud_2;      /**< Baud rate byte 2 */
  uint8_t baud_1;      /**< Baud rate byte 1 */
  uint8_t baud_lsb;    /**< Baud rate LSB */
} __attribute__((packed)) sapi_pti_config_frame_t;
#endif

#define DEBUG_INTERFACE_PROTOCOL_SILABS_PTI     0
#define RADIO_DEBUG_COMMANDS_SUPPORTED_VERSION  2

ZW_ADD_CMD(FUNC_ID_RADIO_DEBUG_GET_PROTOCOL_LIST)
{
  // Response frame:
  // ZW->HOST: GET_PROTOCOL_LIST | Radio Debug Commands Version | Count of supported protocols
  //           [| Supported Radio Debug Protocol 1..N (MSB) | Supported Radio Debug Protocol 1..N (LSB) (N bytes)]
  compl_workbuf[0] = RADIO_DEBUG_COMMANDS_SUPPORTED_VERSION; // Radio Debug Commands Version (for 0xE6, 0xE7, 0xE8)
  compl_workbuf[1] = 1; // For now, we declare support for only PTI protocol
  compl_workbuf[2] = 0; // PTI(0) MSB
  compl_workbuf[3] = 0; // PTI(0) LSB

  DoRespond_workbuf(4);
}

ZW_ADD_CMD(FUNC_ID_RADIO_DEBUG_ENABLE)
{
  /* HOST->ZW: Enable [| Debug Interface Protocol (MSB) | Debug Interface Protocol (LSB) [| Configuration Size | Configuration (N bytes)]]
   * ZW->HOST: Command Status
   *
   * PTI configuration is built from sl_rail_util_pti_config.h constants.
   * Configuration can contain up to 4 bytes of baud rate to override default.
   */
  uint8_t command_status = 0;

  if (frame_payload_len(frame) < 1) {
    DoRespond(command_status);
    return;
  }
  command_status = SaveApplicationEnablePTI(frame->payload[0]);
#ifdef SL_CATALOG_RAIL_UTIL_PTI_PRESENT
  bool enable = 0 != frame->payload[0];

  // Build PTI config from sl_rail_util_pti_config.h constants
  sl_rail_pti_config_t rail_pti_config = {
    .mode = enable ? SL_RAIL_UTIL_PTI_MODE : SL_RAIL_PTI_MODE_DISABLED,
    .baud = SL_RAIL_UTIL_PTI_BAUD_RATE_HZ,
#if defined(SL_RAIL_UTIL_PTI_DOUT_PORT) && defined(SL_RAIL_UTIL_PTI_DOUT_PIN)
    .dout_port = (uint8_t)SL_RAIL_UTIL_PTI_DOUT_PORT,
    .dout_pin = SL_RAIL_UTIL_PTI_DOUT_PIN,
#endif
#if defined(SL_RAIL_UTIL_PTI_DCLK_PORT) && defined(SL_RAIL_UTIL_PTI_DCLK_PIN)
    .dclk_port = (uint8_t)SL_RAIL_UTIL_PTI_DCLK_PORT,
    .dclk_pin = SL_RAIL_UTIL_PTI_DCLK_PIN,
#endif
#if defined(SL_RAIL_UTIL_PTI_DFRAME_PORT) && defined(SL_RAIL_UTIL_PTI_DFRAME_PIN)
    .dframe_port = (uint8_t)SL_RAIL_UTIL_PTI_DFRAME_PORT,
    .dframe_pin = SL_RAIL_UTIL_PTI_DFRAME_PIN,
#endif
  };

  if (enable) {
    // Override baud rate if provided
    if (frame_payload_len(frame) >= 3) {
      uint16_t debug_interface_protocol = (uint16_t)(((uint16_t)frame->payload[1] << 8) | frame->payload[2]);

      // Only PTI (protocol value 0) is supported
      if (debug_interface_protocol != DEBUG_INTERFACE_PROTOCOL_SILABS_PTI) {
        DoRespond(0);
        return;
      }

      // Check for Configuration Size and Configuration
      if (frame_payload_len(frame) >= 4) {
        uint8_t config_size = frame->payload[3];

        // Validate configuration size and extract baud rate if provided
        if (config_size >= sizeof(sapi_pti_config_frame_t) && frame_payload_len(frame) >= (4 + config_size)) {
          const sapi_pti_config_frame_t *pti_frame = (const sapi_pti_config_frame_t *)&frame->payload[4];

          uint32_t baud_rate = ((uint32_t)pti_frame->baud_msb << 24)
                               | ((uint32_t)pti_frame->baud_2 << 16)
                               | ((uint32_t)pti_frame->baud_1 << 8)
                               | (uint32_t)pti_frame->baud_lsb;

          // Validate baud rate range [147800-20000000] - ignore if invalid
          if (baud_rate >= 147800 && baud_rate <= 20000000) {
            rail_pti_config.baud = baud_rate;
          }
        }
      }
    }
  }

  // Configure and enable PTI
  sl_rail_status_t rail_status = sl_rail_config_pti(SL_RAIL_EFR32_HANDLE, &rail_pti_config);
  if (rail_status == SL_RAIL_STATUS_NO_ERROR) {
    rail_status = sl_rail_enable_pti(SL_RAIL_EFR32_HANDLE, enable);
  }
  command_status = (rail_status == SL_RAIL_STATUS_NO_ERROR) ? 1 : 0;
#endif
  DoRespond(command_status);
}

ZW_ADD_CMD(FUNC_ID_RADIO_DEBUG_STATUS)
{
#ifdef SL_CATALOG_RAIL_UTIL_PTI_PRESENT
  /* HOST->ZW: No parameters
   * ZW->HOST: Debug Interface Status [| Debug Interface Protocol (MSB) | Debug Interface Protocol (LSB)]
   *
   * Returns V2 format (3 bytes): status (1 byte) + protocol (2 bytes)
   */
  sl_rail_pti_config_t current_pti_config;
  sl_rail_status_t rail_status = sl_rail_get_pti_config(SL_RAIL_EFR32_HANDLE, &current_pti_config);

  uint8_t debug_interface_status = 0;
  if (rail_status == SL_RAIL_STATUS_NO_ERROR && current_pti_config.mode != SL_RAIL_PTI_MODE_DISABLED) {
    debug_interface_status = 1;
  }

  uint16_t debug_interface_protocol = DEBUG_INTERFACE_PROTOCOL_SILABS_PTI; // PTI protocol

  compl_workbuf[0] = debug_interface_status;
  compl_workbuf[1] = (debug_interface_protocol >> 8) & 0xFF;
  compl_workbuf[2] = debug_interface_protocol & 0xFF;

  DoRespond_workbuf(3);
#else
  DoRespond(0);
#endif
}
