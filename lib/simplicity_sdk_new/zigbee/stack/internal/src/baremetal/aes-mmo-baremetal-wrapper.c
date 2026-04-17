/***************************************************************************//**
 * @file aes-mmo-baremetal-wrapper.c
 * @brief internal implementations for 'aes-mmo' as a thin-wrapper
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
// automatically generated from aes-mmo.h.  Do not manually edit
#include "stack/include/aes-mmo.h"
#include "stack/internal/inc/aes-mmo-internal-def.h"

sl_status_t sl_zigbee_aes_hash_simple(uint8_t totalLength,
                                      const uint8_t *data,
                                      uint8_t *result)
{
  return sli_zigbee_stack_aes_hash_simple(totalLength,
                                          data,
                                          result);
}

sl_status_t sl_zigbee_aes_mmo_hash_final(sl_zigbee_aes_mmo_hash_context_t *context,
                                         uint32_t length,
                                         const uint8_t *finalData)
{
  return sli_zigbee_stack_aes_mmo_hash_final(context,
                                             length,
                                             finalData);
}

void sl_zigbee_aes_mmo_hash_init(sl_zigbee_aes_mmo_hash_context_t *context)
{
  sli_zigbee_stack_aes_mmo_hash_init(context);
}

sl_status_t sl_zigbee_aes_mmo_hash_update(sl_zigbee_aes_mmo_hash_context_t *context,
                                          uint32_t length,
                                          const uint8_t *data)
{
  return sli_zigbee_stack_aes_mmo_hash_update(context,
                                              length,
                                              data);
}
