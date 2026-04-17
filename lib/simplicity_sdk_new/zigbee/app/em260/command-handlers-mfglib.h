/***************************************************************************//**
 * @file
 * @brief Command handlers for Manufacturing Library commands.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

//------------------------------------------------------------------------------
// Ezsp Command Handlers

sl_status_t sl_zigbee_af_ezsp_mfglib_internal_start_command_cb(bool rxCallback);

sl_status_t sl_zigbee_af_ezsp_mfglib_internal_send_packet_command_cb(uint8_t packetLength,
                                                                     uint8_t packetContents[]);

void sl_zigbee_xncp_mfglib_get_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
void sl_zigbee_xncp_mfglib_set_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
