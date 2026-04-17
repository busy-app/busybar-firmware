/***************************************************************************//**
 * @file
 * @brief Host specific code related to the event table.
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
#include "load-control-event-table.h"
#include "app/framework/security/crypto-state.h"

void sli_zigbee_af_demand_response_load_control_cluster_dsa_sign_callback(sl_status_t status, uint8_t messageLength, uint8_t* message)
{
  // Message has been queued by the stack for sending.  Nothing more to do.
  sli_zigbee_af_crypto_operation_complete();

  if (status != SL_STATUS_OK) {
    sli_zigbee_af_note_signature_failure();
  }

  sl_zigbee_af_demand_response_load_control_cluster_println("sl_zigbee_ezsp_dsa_sign_handler() returned 0x%02X",
                                                            status);
}
