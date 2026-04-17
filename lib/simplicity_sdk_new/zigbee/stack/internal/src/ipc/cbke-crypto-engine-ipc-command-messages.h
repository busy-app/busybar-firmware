/***************************************************************************//**
 * @file cbke-crypto-engine-ipc-command-messages.h
 * @brief defines structured format for 'cbke-crypto-engine' ipc messages
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
// automatically generated from cbke-crypto-engine.h.  Do not manually edit
#ifndef CBKE_CRYPTO_ENGINE_IPC_COMMAND_MESSAGES_H
#define CBKE_CRYPTO_ENGINE_IPC_COMMAND_MESSAGES_H

#include "stack/include/cbke-crypto-engine.h"
#include "stack/internal/inc/cbke-crypto-engine-internal-def.h"

typedef struct {
  bool amInitiator;
  sl_zigbee_certificate_data_t partnerCert;
  sl_zigbee_public_key_data_t partnerEphemeralPublicKey;
} sli_zigbee_stack_calculate_smacs_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_calculate_smacs_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_calculate_smacs_ipc_req_t request;
  sli_zigbee_stack_calculate_smacs_ipc_rsp_t response;
} sli_zigbee_stack_calculate_smacs_ipc_msg_t;

typedef struct {
  bool amInitiator;
  sl_zigbee_certificate_283k1_data_t partnerCert;
  sl_zigbee_public_key_283k1_data_t partnerEphemeralPublicKey;
} sli_zigbee_stack_calculate_smacs_283k1_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_calculate_smacs_283k1_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_calculate_smacs_283k1_ipc_req_t request;
  sli_zigbee_stack_calculate_smacs_283k1_ipc_rsp_t response;
} sli_zigbee_stack_calculate_smacs_283k1_ipc_msg_t;

typedef struct {
  bool storeLinkKey;
} sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_ipc_req_t request;
  sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_ipc_rsp_t response;
} sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_ipc_msg_t;

typedef struct {
  bool storeLinkKey;
} sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1_ipc_req_t request;
  sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1_ipc_rsp_t response;
} sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1_ipc_msg_t;

typedef struct {
  sli_buffer_manager_buffer_t messageToSign;
} sli_zigbee_stack_dsa_sign_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_dsa_sign_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_dsa_sign_ipc_req_t request;
  sli_zigbee_stack_dsa_sign_ipc_rsp_t response;
} sli_zigbee_stack_dsa_sign_ipc_msg_t;

typedef struct {
  sl_zigbee_message_digest_t digest;
  sl_zigbee_certificate_data_t signerCertificate;
  sl_zigbee_signature_data_t receivedSig;
} sli_zigbee_stack_dsa_verify_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_dsa_verify_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_dsa_verify_ipc_req_t request;
  sli_zigbee_stack_dsa_verify_ipc_rsp_t response;
} sli_zigbee_stack_dsa_verify_ipc_msg_t;

typedef struct {
  sl_zigbee_message_digest_t digest;
  sl_zigbee_certificate_283k1_data_t signerCertificate;
  sl_zigbee_signature_283k1_data_t receivedSig;
} sli_zigbee_stack_dsa_verify_283k1_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_dsa_verify_283k1_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_dsa_verify_283k1_ipc_req_t request;
  sli_zigbee_stack_dsa_verify_283k1_ipc_rsp_t response;
} sli_zigbee_stack_dsa_verify_283k1_ipc_msg_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_generate_cbke_keys_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_generate_cbke_keys_ipc_rsp_t response;
} sli_zigbee_stack_generate_cbke_keys_ipc_msg_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_generate_cbke_keys_283k1_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_generate_cbke_keys_283k1_ipc_rsp_t response;
} sli_zigbee_stack_generate_cbke_keys_283k1_ipc_msg_t;

typedef struct {
  sl_zigbee_certificate_data_t result;
} sli_zigbee_stack_get_certificate_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_get_certificate_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_get_certificate_ipc_req_t request;
  sli_zigbee_stack_get_certificate_ipc_rsp_t response;
} sli_zigbee_stack_get_certificate_ipc_msg_t;

typedef struct {
  sl_zigbee_certificate_283k1_data_t result;
} sli_zigbee_stack_get_certificate_283k1_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_get_certificate_283k1_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_get_certificate_283k1_ipc_req_t request;
  sli_zigbee_stack_get_certificate_283k1_ipc_rsp_t response;
} sli_zigbee_stack_get_certificate_283k1_ipc_msg_t;

typedef struct {
  sl_802154_long_addr_t certEui64;
} sli_zigbee_stack_get_stack_certificate_eui64_ipc_req_t;

typedef struct {
  bool result;
} sli_zigbee_stack_get_stack_certificate_eui64_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_get_stack_certificate_eui64_ipc_req_t request;
  sli_zigbee_stack_get_stack_certificate_eui64_ipc_rsp_t response;
} sli_zigbee_stack_get_stack_certificate_eui64_ipc_msg_t;

typedef struct {
  sl_zigbee_public_key_data_t caPublic;
  sl_zigbee_certificate_data_t myCert;
  sl_zigbee_private_key_data_t myKey;
} sli_zigbee_stack_set_preinstalled_cbke_data_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_set_preinstalled_cbke_data_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_set_preinstalled_cbke_data_ipc_req_t request;
  sli_zigbee_stack_set_preinstalled_cbke_data_ipc_rsp_t response;
} sli_zigbee_stack_set_preinstalled_cbke_data_ipc_msg_t;

typedef struct {
  sl_zigbee_public_key_283k1_data_t caPublic;
  sl_zigbee_certificate_283k1_data_t myCert;
  sl_zigbee_private_key_283k1_data_t myKey;
} sli_zigbee_stack_set_preinstalled_cbke_data_283k1_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_set_preinstalled_cbke_data_283k1_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_set_preinstalled_cbke_data_283k1_ipc_req_t request;
  sli_zigbee_stack_set_preinstalled_cbke_data_283k1_ipc_rsp_t response;
} sli_zigbee_stack_set_preinstalled_cbke_data_283k1_ipc_msg_t;

#endif // CBKE_CRYPTO_ENGINE_IPC_COMMAND_MESSAGES_H
