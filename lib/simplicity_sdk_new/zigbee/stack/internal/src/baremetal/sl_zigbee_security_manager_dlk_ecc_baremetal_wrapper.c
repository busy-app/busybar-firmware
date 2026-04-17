/***************************************************************************//**
 * @file sl_zigbee_security_manager_dlk_ecc_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_security_manager_dlk_ecc' as a thin-wrapper
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
// automatically generated from sl_zigbee_security_manager_dlk_ecc.h.  Do not manually edit
#include "stack/include/sl_zigbee_security_manager_dlk_ecc.h"
#include "stack/internal/inc/sl_zigbee_security_manager_dlk_ecc_internal_def.h"

sl_status_t sl_zigbee_sec_man_ecc_derive_link_key(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx)
{
  return sli_zigbee_stack_sec_man_ecc_derive_link_key(dlk_ecc_ctx);
}

sl_status_t sl_zigbee_sec_man_ecc_expand_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                       const uint8_t *our_eui,
                                                       const uint8_t *their_eui)
{
  return sli_zigbee_stack_sec_man_ecc_expand_shared_secret(dlk_ecc_ctx,
                                                           our_eui,
                                                           their_eui);
}

sl_status_t sl_zigbee_sec_man_ecc_extract_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                        const uint8_t *peer_public_key,
                                                        size_t peer_key_len)
{
  return sli_zigbee_stack_sec_man_ecc_extract_shared_secret(dlk_ecc_ctx,
                                                            peer_public_key,
                                                            peer_key_len);
}

void sl_zigbee_sec_man_ecc_free(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx)
{
  sli_zigbee_stack_sec_man_ecc_free(dlk_ecc_ctx);
}

sl_status_t sl_zigbee_sec_man_ecc_generate_keypair(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                   uint8_t *public_key_buff,
                                                   size_t *key_len_out)
{
  return sli_zigbee_stack_sec_man_ecc_generate_keypair(dlk_ecc_ctx,
                                                       public_key_buff,
                                                       key_len_out);
}

sl_status_t sl_zigbee_sec_man_ecc_init(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                       sl_zb_dlk_ecc_config_t *config,
                                       const uint8_t *psk)
{
  return sli_zigbee_stack_sec_man_ecc_init(dlk_ecc_ctx,
                                           config,
                                           psk);
}

sl_status_t sl_zigbee_sec_man_speke_expand_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                         const uint8_t *our_eui,
                                                         const uint8_t *their_eui)
{
  return sli_zigbee_stack_sec_man_speke_expand_shared_secret(dlk_ecc_ctx,
                                                             our_eui,
                                                             their_eui);
}
