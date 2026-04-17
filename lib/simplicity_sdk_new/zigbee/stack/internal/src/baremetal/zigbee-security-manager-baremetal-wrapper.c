/***************************************************************************//**
 * @file zigbee-security-manager-baremetal-wrapper.c
 * @brief internal implementations for 'zigbee-security-manager' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from zigbee-security-manager.h.  Do not manually edit
#include "stack/include/zigbee-security-manager.h"
#include "stack/internal/inc/zigbee-security-manager-internal-def.h"

sl_status_t sl_zigbee_get_key_from_install_code(sl_zigbee_key_data_t *key)
{
  return sli_zigbee_stack_get_key_from_install_code(key);
}

sl_status_t sl_zigbee_sec_man_aes_128_crypt_block(bool encrypt,
                                                  const uint8_t *input,
                                                  uint8_t *output)
{
  return sli_zigbee_stack_sec_man_aes_128_crypt_block(encrypt,
                                                      input,
                                                      output);
}

sl_status_t sl_zigbee_sec_man_aes_ccm_extended(uint8_t *nonce,
                                               bool encrypt,
                                               const uint8_t *input,
                                               uint8_t encryption_start_index,
                                               uint8_t length,
                                               uint8_t mic_length,
                                               uint8_t *output)
{
  return sli_zigbee_stack_sec_man_aes_ccm_extended(nonce,
                                                   encrypt,
                                                   input,
                                                   encryption_start_index,
                                                   length,
                                                   mic_length,
                                                   output);
}

sl_status_t sl_zigbee_sec_man_check_key_context(sl_zigbee_sec_man_context_t *context)
{
  return sli_zigbee_stack_sec_man_check_key_context(context);
}

bool sl_zigbee_sec_man_compare_key_to_value(sl_zigbee_sec_man_context_t *context,
                                            const sl_zigbee_sec_man_key_t *test_key)
{
  return sli_zigbee_stack_sec_man_compare_key_to_value(context,
                                                       test_key);
}

sl_status_t sl_zigbee_sec_man_delete_key(sl_zigbee_sec_man_context_t *context)
{
  return sli_zigbee_stack_sec_man_delete_key(context);
}

sl_status_t sl_zigbee_sec_man_delete_key_table_key(sl_zigbee_sec_man_context_t *context)
{
  return sli_zigbee_stack_sec_man_delete_key_table_key(context);
}

sl_status_t sl_zigbee_sec_man_delete_transient_key(sl_zigbee_sec_man_context_t *context)
{
  return sli_zigbee_stack_sec_man_delete_transient_key(context);
}

sl_status_t sl_zigbee_sec_man_export_key(sl_zigbee_sec_man_context_t *context,
                                         sl_zigbee_sec_man_key_t *plaintext_key)
{
  return sli_zigbee_stack_sec_man_export_key(context,
                                             plaintext_key);
}

sl_status_t sl_zigbee_sec_man_export_link_key_by_eui(sl_802154_long_addr_t eui,
                                                     sl_zigbee_sec_man_context_t *context,
                                                     sl_zigbee_sec_man_key_t *plaintext_key,
                                                     sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  return sli_zigbee_stack_sec_man_export_link_key_by_eui(eui,
                                                         context,
                                                         plaintext_key,
                                                         key_data);
}

sl_status_t sl_zigbee_sec_man_export_link_key_by_index(uint8_t index,
                                                       sl_zigbee_sec_man_context_t *context,
                                                       sl_zigbee_sec_man_key_t *plaintext_key,
                                                       sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  return sli_zigbee_stack_sec_man_export_link_key_by_index(index,
                                                           context,
                                                           plaintext_key,
                                                           key_data);
}

sl_status_t sl_zigbee_sec_man_export_symmetric_passphrase(sl_802154_long_addr_t address,
                                                          sl_zigbee_key_data_t *key_data)
{
  return sli_zigbee_stack_sec_man_export_symmetric_passphrase(address,
                                                              key_data);
}

sl_status_t sl_zigbee_sec_man_export_transient_key_by_eui(sl_802154_long_addr_t eui64,
                                                          sl_zigbee_sec_man_context_t *context,
                                                          sl_zigbee_sec_man_key_t *plaintext_key,
                                                          sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  return sli_zigbee_stack_sec_man_export_transient_key_by_eui(eui64,
                                                              context,
                                                              plaintext_key,
                                                              key_data);
}

sl_status_t sl_zigbee_sec_man_export_transient_key_by_index(uint8_t index,
                                                            sl_zigbee_sec_man_context_t *context,
                                                            sl_zigbee_sec_man_key_t *plaintext_key,
                                                            sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  return sli_zigbee_stack_sec_man_export_transient_key_by_index(index,
                                                                context,
                                                                plaintext_key,
                                                                key_data);
}

uint8_t sl_zigbee_sec_man_find_symmetric_passphrase_key_table_index(sl_802154_long_addr_t address)
{
  return sli_zigbee_stack_sec_man_find_symmetric_passphrase_key_table_index(address);
}

sl_status_t sl_zigbee_sec_man_get_aps_key_info(sl_zigbee_sec_man_context_t *context,
                                               sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  return sli_zigbee_stack_sec_man_get_aps_key_info(context,
                                                   key_data);
}

sl_status_t sl_zigbee_sec_man_get_network_key_info(sl_zigbee_sec_man_network_key_info_t *network_key_info)
{
  return sli_zigbee_stack_sec_man_get_network_key_info(network_key_info);
}

bool sl_zigbee_sec_man_have_link_key(sl_802154_long_addr_t eui)
{
  return sli_zigbee_stack_sec_man_have_link_key(eui);
}

void sl_zigbee_sec_man_hmac_aes_mmo(const uint8_t *input,
                                    const uint8_t data_length,
                                    uint8_t *output)
{
  sli_zigbee_stack_sec_man_hmac_aes_mmo(input,
                                        data_length,
                                        output);
}

sl_status_t sl_zigbee_sec_man_import_key(sl_zigbee_sec_man_context_t *context,
                                         const sl_zigbee_sec_man_key_t *plaintext_key)
{
  return sli_zigbee_stack_sec_man_import_key(context,
                                             plaintext_key);
}

sl_status_t sl_zigbee_sec_man_import_link_key(uint8_t index,
                                              sl_802154_long_addr_t address,
                                              sl_zigbee_sec_man_key_t *plaintext_key)
{
  return sli_zigbee_stack_sec_man_import_link_key(index,
                                                  address,
                                                  plaintext_key);
}

sl_status_t sl_zigbee_sec_man_import_symmetric_passphrase(uint8_t index,
                                                          sl_802154_long_addr_t address,
                                                          const sl_zigbee_key_data_t *key_data)
{
  return sli_zigbee_stack_sec_man_import_symmetric_passphrase(index,
                                                              address,
                                                              key_data);
}

sl_status_t sl_zigbee_sec_man_import_transient_key(sl_802154_long_addr_t eui64,
                                                   sl_zigbee_sec_man_key_t *plaintext_key)
{
  return sli_zigbee_stack_sec_man_import_transient_key(eui64,
                                                       plaintext_key);
}

void sl_zigbee_sec_man_init_context(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_stack_sec_man_init_context(context);
}

bool sl_zigbee_sec_man_link_key_slot_available(sl_802154_long_addr_t eui64)
{
  return sli_zigbee_stack_sec_man_link_key_slot_available(eui64);
}

sl_status_t sl_zigbee_sec_man_load_key_context(sl_zigbee_sec_man_context_t *context)
{
  return sli_zigbee_stack_sec_man_load_key_context(context);
}

void sl_zigbee_sec_man_set_context_aes_ecb_alg(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_stack_sec_man_set_context_aes_ecb_alg(context);
}

void sl_zigbee_sec_man_set_context_extended_ccm_alg(sl_zigbee_sec_man_context_t *context)
{
  sli_zigbee_stack_sec_man_set_context_extended_ccm_alg(context);
}

sl_status_t sl_zigbee_sec_man_update_symmetric_passphrase_eui(sl_802154_long_addr_t old_eui64,
                                                              sl_802154_long_addr_t new_eui64)
{
  return sli_zigbee_stack_sec_man_update_symmetric_passphrase_eui(old_eui64,
                                                                  new_eui64);
}

sl_status_t slx_zigbee_get_key_table_additional_info(uint8_t index,
                                                     uint32_t *tok)
{
  return slxi_zigbee_stack_get_key_table_additional_info(index,
                                                         tok);
}

sl_status_t slx_zigbee_get_trust_center_additional_info(uint32_t *tok)
{
  return slxi_zigbee_stack_get_trust_center_additional_info(tok);
}

sl_status_t slx_zigbee_set_key_table_additional_info(uint8_t index,
                                                     uint32_t tok)
{
  return slxi_zigbee_stack_set_key_table_additional_info(index,
                                                         tok);
}

sl_status_t slx_zigbee_set_trust_center_additional_info(uint32_t tok)
{
  return slxi_zigbee_stack_set_trust_center_additional_info(tok);
}
