/***************************************************************************//**
 * @file cbke-crypto-engine-ipc-command-messages.c
 * @brief internal wrappers for 'cbke-crypto-engine' ipc commands
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
#include "stack/include/cbke-crypto-engine.h"
#include "stack/internal/inc/cbke-crypto-engine-internal-def.h"
#include "stack/internal/src/ipc/cbke-crypto-engine-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_calculate_smacs_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.calculate_smacs.response.result = sli_zigbee_stack_calculate_smacs(msg->data.calculate_smacs.request.amInitiator,
                                                                               &msg->data.calculate_smacs.request.partnerCert,
                                                                               &msg->data.calculate_smacs.request.partnerEphemeralPublicKey);
}

void sli_zigbee_stack_calculate_smacs_283k1_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.calculate_smacs_283k1.response.result = sli_zigbee_stack_calculate_smacs_283k1(msg->data.calculate_smacs_283k1.request.amInitiator,
                                                                                           &msg->data.calculate_smacs_283k1.request.partnerCert,
                                                                                           &msg->data.calculate_smacs_283k1.request.partnerEphemeralPublicKey);
}

void sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.clear_temporary_data_maybe_store_link_key.response.result = sli_zigbee_stack_clear_temporary_data_maybe_store_link_key(msg->data.clear_temporary_data_maybe_store_link_key.request.storeLinkKey);
}

void sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.clear_temporary_data_maybe_store_link_key_283k1.response.result = sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1(msg->data.clear_temporary_data_maybe_store_link_key_283k1.request.storeLinkKey);
}

void sli_zigbee_stack_dsa_sign_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dsa_sign.response.result = sli_zigbee_stack_dsa_sign(msg->data.dsa_sign.request.messageToSign);
}

void sli_zigbee_stack_dsa_verify_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dsa_verify.response.result = sli_zigbee_stack_dsa_verify(&msg->data.dsa_verify.request.digest,
                                                                     &msg->data.dsa_verify.request.signerCertificate,
                                                                     &msg->data.dsa_verify.request.receivedSig);
}

void sli_zigbee_stack_dsa_verify_283k1_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dsa_verify_283k1.response.result = sli_zigbee_stack_dsa_verify_283k1(&msg->data.dsa_verify_283k1.request.digest,
                                                                                 &msg->data.dsa_verify_283k1.request.signerCertificate,
                                                                                 &msg->data.dsa_verify_283k1.request.receivedSig);
}

void sli_zigbee_stack_generate_cbke_keys_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.generate_cbke_keys.response.result = sli_zigbee_stack_generate_cbke_keys();
}

void sli_zigbee_stack_generate_cbke_keys_283k1_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.generate_cbke_keys_283k1.response.result = sli_zigbee_stack_generate_cbke_keys_283k1();
}

void sli_zigbee_stack_get_certificate_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_certificate.response.result = sli_zigbee_stack_get_certificate(&msg->data.get_certificate.request.result);
}

void sli_zigbee_stack_get_certificate_283k1_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_certificate_283k1.response.result = sli_zigbee_stack_get_certificate_283k1(&msg->data.get_certificate_283k1.request.result);
}

void sli_zigbee_stack_get_stack_certificate_eui64_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_stack_certificate_eui64.response.result = sli_zigbee_stack_get_stack_certificate_eui64(msg->data.get_stack_certificate_eui64.request.certEui64);
}

void sli_zigbee_stack_set_preinstalled_cbke_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_preinstalled_cbke_data.response.result = sli_zigbee_stack_set_preinstalled_cbke_data(&msg->data.set_preinstalled_cbke_data.request.caPublic,
                                                                                                     &msg->data.set_preinstalled_cbke_data.request.myCert,
                                                                                                     &msg->data.set_preinstalled_cbke_data.request.myKey);
}

void sli_zigbee_stack_set_preinstalled_cbke_data_283k1_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_preinstalled_cbke_data_283k1.response.result = sli_zigbee_stack_set_preinstalled_cbke_data_283k1(&msg->data.set_preinstalled_cbke_data_283k1.request.caPublic,
                                                                                                                 &msg->data.set_preinstalled_cbke_data_283k1.request.myCert,
                                                                                                                 &msg->data.set_preinstalled_cbke_data_283k1.request.myKey);
}

// public entrypoints

sl_status_t sl_zigbee_calculate_smacs(bool amInitiator,
                                      sl_zigbee_certificate_data_t *partnerCert,
                                      sl_zigbee_public_key_data_t *partnerEphemeralPublicKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.calculate_smacs.request.amInitiator = amInitiator;

  if (partnerCert != NULL) {
    msg.data.calculate_smacs.request.partnerCert = *partnerCert;
  }

  if (partnerEphemeralPublicKey != NULL) {
    msg.data.calculate_smacs.request.partnerEphemeralPublicKey = *partnerEphemeralPublicKey;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_calculate_smacs_process_ipc_command, &msg);

  if (partnerCert != NULL) {
    *partnerCert = msg.data.calculate_smacs.request.partnerCert;
  }

  if (partnerEphemeralPublicKey != NULL) {
    *partnerEphemeralPublicKey = msg.data.calculate_smacs.request.partnerEphemeralPublicKey;
  }

  return msg.data.calculate_smacs.response.result;
}

sl_status_t sl_zigbee_calculate_smacs_283k1(bool amInitiator,
                                            sl_zigbee_certificate_283k1_data_t *partnerCert,
                                            sl_zigbee_public_key_283k1_data_t *partnerEphemeralPublicKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.calculate_smacs_283k1.request.amInitiator = amInitiator;

  if (partnerCert != NULL) {
    msg.data.calculate_smacs_283k1.request.partnerCert = *partnerCert;
  }

  if (partnerEphemeralPublicKey != NULL) {
    msg.data.calculate_smacs_283k1.request.partnerEphemeralPublicKey = *partnerEphemeralPublicKey;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_calculate_smacs_283k1_process_ipc_command, &msg);

  if (partnerCert != NULL) {
    *partnerCert = msg.data.calculate_smacs_283k1.request.partnerCert;
  }

  if (partnerEphemeralPublicKey != NULL) {
    *partnerEphemeralPublicKey = msg.data.calculate_smacs_283k1.request.partnerEphemeralPublicKey;
  }

  return msg.data.calculate_smacs_283k1.response.result;
}

sl_status_t sl_zigbee_clear_temporary_data_maybe_store_link_key(bool storeLinkKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.clear_temporary_data_maybe_store_link_key.request.storeLinkKey = storeLinkKey;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_process_ipc_command, &msg);

  return msg.data.clear_temporary_data_maybe_store_link_key.response.result;
}

sl_status_t sl_zigbee_clear_temporary_data_maybe_store_link_key_283k1(bool storeLinkKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.clear_temporary_data_maybe_store_link_key_283k1.request.storeLinkKey = storeLinkKey;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1_process_ipc_command, &msg);

  return msg.data.clear_temporary_data_maybe_store_link_key_283k1.response.result;
}

sl_status_t sl_zigbee_dsa_sign(sli_buffer_manager_buffer_t messageToSign)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.dsa_sign.request.messageToSign = messageToSign;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dsa_sign_process_ipc_command, &msg);

  return msg.data.dsa_sign.response.result;
}

sl_status_t sl_zigbee_dsa_verify(sl_zigbee_message_digest_t *digest,
                                 sl_zigbee_certificate_data_t *signerCertificate,
                                 sl_zigbee_signature_data_t *receivedSig)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (digest != NULL) {
    msg.data.dsa_verify.request.digest = *digest;
  }

  if (signerCertificate != NULL) {
    msg.data.dsa_verify.request.signerCertificate = *signerCertificate;
  }

  if (receivedSig != NULL) {
    msg.data.dsa_verify.request.receivedSig = *receivedSig;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dsa_verify_process_ipc_command, &msg);

  if (digest != NULL) {
    *digest = msg.data.dsa_verify.request.digest;
  }

  if (signerCertificate != NULL) {
    *signerCertificate = msg.data.dsa_verify.request.signerCertificate;
  }

  if (receivedSig != NULL) {
    *receivedSig = msg.data.dsa_verify.request.receivedSig;
  }

  return msg.data.dsa_verify.response.result;
}

sl_status_t sl_zigbee_dsa_verify_283k1(const sl_zigbee_message_digest_t *digest,
                                       const sl_zigbee_certificate_283k1_data_t *signerCertificate,
                                       const sl_zigbee_signature_283k1_data_t *receivedSig)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (digest != NULL) {
    msg.data.dsa_verify_283k1.request.digest = *digest;
  }

  if (signerCertificate != NULL) {
    msg.data.dsa_verify_283k1.request.signerCertificate = *signerCertificate;
  }

  if (receivedSig != NULL) {
    msg.data.dsa_verify_283k1.request.receivedSig = *receivedSig;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dsa_verify_283k1_process_ipc_command, &msg);

  return msg.data.dsa_verify_283k1.response.result;
}

sl_status_t sl_zigbee_generate_cbke_keys(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_generate_cbke_keys_process_ipc_command, &msg);

  return msg.data.generate_cbke_keys.response.result;
}

sl_status_t sl_zigbee_generate_cbke_keys_283k1(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_generate_cbke_keys_283k1_process_ipc_command, &msg);

  return msg.data.generate_cbke_keys_283k1.response.result;
}

sl_status_t sl_zigbee_get_certificate(sl_zigbee_certificate_data_t *result)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (result != NULL) {
    msg.data.get_certificate.request.result = *result;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_certificate_process_ipc_command, &msg);

  if (result != NULL) {
    *result = msg.data.get_certificate.request.result;
  }

  return msg.data.get_certificate.response.result;
}

sl_status_t sl_zigbee_get_certificate_283k1(sl_zigbee_certificate_283k1_data_t *result)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (result != NULL) {
    msg.data.get_certificate_283k1.request.result = *result;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_certificate_283k1_process_ipc_command, &msg);

  if (result != NULL) {
    *result = msg.data.get_certificate_283k1.request.result;
  }

  return msg.data.get_certificate_283k1.response.result;
}

bool sl_zigbee_get_stack_certificate_eui64(sl_802154_long_addr_t certEui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (certEui64 != NULL) {
    memmove(msg.data.get_stack_certificate_eui64.request.certEui64, certEui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_stack_certificate_eui64_process_ipc_command, &msg);

  if (certEui64 != NULL) {
    memmove(certEui64, msg.data.get_stack_certificate_eui64.request.certEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.get_stack_certificate_eui64.response.result;
}

sl_status_t sl_zigbee_set_preinstalled_cbke_data(sl_zigbee_public_key_data_t *caPublic,
                                                 sl_zigbee_certificate_data_t *myCert,
                                                 sl_zigbee_private_key_data_t *myKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (caPublic != NULL) {
    msg.data.set_preinstalled_cbke_data.request.caPublic = *caPublic;
  }

  if (myCert != NULL) {
    msg.data.set_preinstalled_cbke_data.request.myCert = *myCert;
  }

  if (myKey != NULL) {
    msg.data.set_preinstalled_cbke_data.request.myKey = *myKey;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_preinstalled_cbke_data_process_ipc_command, &msg);

  if (caPublic != NULL) {
    *caPublic = msg.data.set_preinstalled_cbke_data.request.caPublic;
  }

  if (myCert != NULL) {
    *myCert = msg.data.set_preinstalled_cbke_data.request.myCert;
  }

  if (myKey != NULL) {
    *myKey = msg.data.set_preinstalled_cbke_data.request.myKey;
  }

  return msg.data.set_preinstalled_cbke_data.response.result;
}

sl_status_t sl_zigbee_set_preinstalled_cbke_data_283k1(const sl_zigbee_public_key_283k1_data_t *caPublic,
                                                       const sl_zigbee_certificate_283k1_data_t *myCert,
                                                       const sl_zigbee_private_key_283k1_data_t *myKey)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (caPublic != NULL) {
    msg.data.set_preinstalled_cbke_data_283k1.request.caPublic = *caPublic;
  }

  if (myCert != NULL) {
    msg.data.set_preinstalled_cbke_data_283k1.request.myCert = *myCert;
  }

  if (myKey != NULL) {
    msg.data.set_preinstalled_cbke_data_283k1.request.myKey = *myKey;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_preinstalled_cbke_data_283k1_process_ipc_command, &msg);

  return msg.data.set_preinstalled_cbke_data_283k1.response.result;
}
