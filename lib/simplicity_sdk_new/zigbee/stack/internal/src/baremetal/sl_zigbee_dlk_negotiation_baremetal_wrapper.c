/***************************************************************************//**
 * @file sl_zigbee_dlk_negotiation_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_dlk_negotiation' as a thin-wrapper
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
// automatically generated from sl_zigbee_dlk_negotiation.h.  Do not manually edit
#include "stack/include/sl_zigbee_dlk_negotiation.h"
#include "stack/internal/inc/sl_zigbee_dlk_negotiation_internal_def.h"

void sl_zigbee_dlk_close_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx)
{
  sli_zigbee_stack_dlk_close_key_exchange(dlk_negotiation_ctx);
}

sl_status_t sl_zigbee_dlk_finish_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx,
                                              const sl_zigbee_address_info *partner,
                                              const uint8_t *partner_key,
                                              size_t partner_key_length)
{
  return sli_zigbee_stack_dlk_finish_key_exchange(dlk_negotiation_ctx,
                                                  partner,
                                                  partner_key,
                                                  partner_key_length);
}

sl_status_t sl_zigbee_dlk_open_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx,
                                            const sl_zigbee_address_info *partner,
                                            sl_zigbee_dlk_negotiation_method keyProtocol,
                                            sl_zigbee_dlk_negotiation_shared_secret_source secret,
                                            const uint8_t *psk_bytes,
                                            sl_zigbee_dlk_start_complete_callback dlk_start_complete_callback,
                                            sl_zigbee_dlk_finish_complete_callback dlk_finish_complete_callback)
{
  return sli_zigbee_stack_dlk_open_key_exchange(dlk_negotiation_ctx,
                                                partner,
                                                keyProtocol,
                                                secret,
                                                psk_bytes,
                                                dlk_start_complete_callback,
                                                dlk_finish_complete_callback);
}

sl_status_t sl_zigbee_dlk_start_key_exchange(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx,
                                             const sl_zigbee_address_info *partner)
{
  return sli_zigbee_stack_dlk_start_key_exchange(dlk_negotiation_ctx,
                                                 partner);
}
