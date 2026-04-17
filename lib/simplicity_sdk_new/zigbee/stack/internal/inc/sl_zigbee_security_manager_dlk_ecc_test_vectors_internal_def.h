/***************************************************************************//**
 * @file sl_zigbee_security_manager_dlk_ecc_test_vectors_internal_def.h
 * @brief internal names for 'sl_zigbee_security_manager_dlk_ecc_test_vectors' declarations
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
// automatically generated from sl_zigbee_security_manager_dlk_ecc_test_vectors.h.  Do not manually edit
#ifndef SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_TEST_VECTORS_INTERNAL_DEF_H
#define SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_TEST_VECTORS_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_security_manager_dlk_ecc_test_vectors.h"

// Command Indirection

sl_zigbee_dlk_ecc_test_vector_profile_data_t * sli_zigbee_stack_dlk_ecc_test_vector_get_profile(void);

void sli_zigbee_stack_dlk_ecc_test_vector_set_profile(const sl_zigbee_dlk_ecc_test_vector_profile_data_t *profile);

sl_status_t sli_zigbee_stack_sec_man_ecc_init_with_test_vector(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                               sl_zb_dlk_ecc_config_t *config,
                                                               uint8_t *psk,
                                                               sl_zigbee_dlk_ecc_test_vector_bundle_t *vector_data);

#endif // SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_TEST_VECTORS_INTERNAL_DEF_H
