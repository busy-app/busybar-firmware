/***************************************************************************//**
 * @file sl_zigbee_security_manager_dlk_ecc_internal_def.h
 * @brief internal names for 'sl_zigbee_security_manager_dlk_ecc' declarations
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
#ifndef SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_INTERNAL_DEF_H
#define SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_security_manager_dlk_ecc.h"

// Command Indirection

sl_status_t sli_zigbee_stack_sec_man_ecc_derive_link_key(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx);

sl_status_t sli_zigbee_stack_sec_man_ecc_expand_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                              const uint8_t *our_eui,
                                                              const uint8_t *their_eui);

sl_status_t sli_zigbee_stack_sec_man_ecc_extract_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                               const uint8_t *peer_public_key,
                                                               size_t peer_key_len);

void sli_zigbee_stack_sec_man_ecc_free(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx);

sl_status_t sli_zigbee_stack_sec_man_ecc_generate_keypair(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                          uint8_t *public_key_buff,
                                                          size_t *key_len_out);

sl_status_t sli_zigbee_stack_sec_man_ecc_init(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                              sl_zb_dlk_ecc_config_t *config,
                                              const uint8_t *psk);

sl_status_t sli_zigbee_stack_sec_man_speke_expand_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                                const uint8_t *our_eui,
                                                                const uint8_t *their_eui);

#endif // SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_INTERNAL_DEF_H
