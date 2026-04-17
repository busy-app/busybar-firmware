/***************************************************************************//**
 * @file
 * @brief Host routines for the Diagnostic Server plugin.
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

#include "app/framework/include/af.h"
#include "diagnostic-server.h"

// This is just a stub for the host processor
bool sl_zigbee_af_read_diagnostic_attribute(
  sl_zigbee_af_attribute_metadata_t *attributeMetadata,
  uint8_t *buffer)
{
  (void)attributeMetadata;
  (void)buffer;
  return false;
}

bool sl_zigbee_af_pre_message_received_cb(sl_zigbee_af_incoming_message_t* incomingMessage)
{
  (void)incomingMessage;
  return false;
}
