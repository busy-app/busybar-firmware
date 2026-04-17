/***************************************************************************//**
 * @file
 * @brief CLI for the Key Establishment plugin.
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

#include "app/util/serial/sl_zigbee_command_interpreter.h"

void sli_zigbee_af_key_establishment_start_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  sl_802154_short_addr_t newPartnerId = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_af_core_println("Starting %sment w/ 0x%04X, EP: 0x%02X", \
                            "Key Establish",
                            newPartnerId,
                            endpoint);
  sl_zigbee_af_core_flush();

  status = sl_zigbee_af_initiate_key_establishment(newPartnerId, endpoint);
  sl_zigbee_af_core_println("%s", (status == SL_STATUS_OK ? "Success" : "Error"));
}

void sli_zigbee_af_key_establishment_interpan_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_802154_pan_id_t panId = (sl_802154_pan_id_t)sl_cli_get_argument_uint16(arguments, 0);
  sl_status_t status;
  sl_zigbee_copy_eui64_arg(arguments, 1, eui64, true);

  sl_zigbee_af_core_print("Starting %sment w/ ", "Key Establish");
  sl_zigbee_af_core_debug_exec(sl_zigbee_af_print_big_endian_eui64(eui64));
  sl_zigbee_af_core_println("");
  sl_zigbee_af_core_flush();

  status = sl_zigbee_af_initiate_inter_pan_key_establishment(panId, eui64);
  sl_zigbee_af_core_println("%s", (status == SL_STATUS_OK ? "Success" : "Error"));
}
