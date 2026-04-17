/***************************************************************************//**
 * @file
 * @brief CLI for the Partner Link Key Exchange plugin.
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
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/partner-link-key-exchange/partner-link-key-exchange.h"

#if defined(EZSP_HOST)
extern sl_status_t sl_zigbee_af_set_ezsp_policy(sl_zigbee_ezsp_policy_id_t policyId,
                                                sl_zigbee_ezsp_decision_id_t decisionId,
                                                const char * policyName,
                                                const char * decisionName);
#endif // EZSP_HOST

#include "sl_cli.h"

void cbkeAllowPartner(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_allow_partner_link_key = sl_cli_get_argument_uint8(arguments, 0);

#if defined(EZSP_HOST)
  sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_APP_KEY_REQUEST_POLICY,
                               (sli_zigbee_af_allow_partner_link_key
                                ? SL_ZIGBEE_EZSP_ALLOW_APP_KEY_REQUESTS
                                : SL_ZIGBEE_EZSP_DENY_APP_KEY_REQUESTS),
                               "App Link Key Request Policy",
                               (sli_zigbee_af_allow_partner_link_key
                                ? "Allow"
                                : "Deny"));

#endif
}

void cbkePartnerCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t target = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t endpoint     = sl_cli_get_argument_uint8(arguments, 1);
  sl_status_t status = sl_zigbee_af_initiate_partner_link_key_exchange(target,
                                                                       endpoint,
                                                                       NULL); // callback
  sl_zigbee_af_core_debug_exec(sli_zigbee_af_print_status("partner link key request", status));
}
