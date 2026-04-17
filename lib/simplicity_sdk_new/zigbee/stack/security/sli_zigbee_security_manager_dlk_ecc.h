/***************************************************************************//**
 * @file sli_zigbee_security_manager_dlk_ecc.h
 * @brief stateless computation of dynamic link key elliptic curve cryptography
 * (INTERNAL)
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_ZIGBEE_SECURITY_MANAGER_DLK_ECC_H
#define SLI_ZIGBEE_SECURITY_MANAGER_DLK_ECC_H

#include "stack/include/sl_zigbee_security_manager_dlk_ecc.h"

/**
 * @brief checks if given ecc operation, curve, and hash are valid
 */
bool sli_zigbee_is_supported_ecc_operation(sl_zb_dlk_ecc_config_t *ecc_config);

/**
 * @brief serializes the generated public key into the given buffer
 * @param dlk_ecc_ctx a pointer to the context struct containing generated key
 * @param is_peer if true serialize the peer public key
 * @param public_key_buff the destination buffer to write the public key data to
 * @note must contain enough bytes to hold the key (64 bytes for DLK_ECC_CURVE_P256)
 * (32 bytes for DLK_ECC_CURVE_25519)
 * @param public_key_len a pointer arg that will contain the number of key bytes serialized
 * @return status indicating if anything went wrong
 */
sl_status_t sli_zb_sec_man_ecc_export_public_key(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                 bool is_peer,
                                                 uint8_t *public_key_buff,
                                                 size_t *public_key_len);

/**
 * @brief incorporates peer public key data into the ecc context
 * @param dlk_ecc_ctx a pointer to the context struct containing partial computation
 * @param public_key_buff the source buffer containing peer public key data
 * @param public_key_len the amount of key data contained in the input buffer
 * @return status indicating if something went wrong
 */
sl_status_t sli_zb_sec_man_ecc_import_peer_public_key(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                      const uint8_t *public_key_buff,
                                                      size_t public_key_len);

#endif // SL_ZIGBEE_SECURITY_MANAGER_DLK_ECC_H
