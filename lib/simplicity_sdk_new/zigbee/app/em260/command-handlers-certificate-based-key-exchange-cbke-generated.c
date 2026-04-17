/*****************************************************************************/
/**
 * Copyright 2021 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// *** Generated file. Do not edit! ***
//
// Description: Handlers for the EZSP frames that directly correspond to Ember
// API calls.

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/internal/inc/internal-defs-patch.h"
#include "ezsp-enum.h"
#include "app/em260/command-context.h"
#include "stack/include/cbke-crypto-engine.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/internal/inc/mfglib_internal_def.h"
#include "stack/include/binding-table.h"
#include "stack/include/message.h"
#include "stack/include/mac-layer.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"
#include "app/em260/command-handlers-cbke.h"
#include "app/em260/command-handlers-binding.h"
#include "app/em260/command-handlers-mfglib.h"
#include "app/em260/command-handlers-security.h"
#include "app/em260/command-handlers-zll.h"
#include "app/em260/command-handlers-zigbee-pro.h"
#include "child.h"
#include "message.h"
#include "zll-api.h"
#include "security.h"
#include "stack-info.h"
#include "network-formation.h"
#include "zigbee-device-stack.h"
#include "sl_zigbee_duty_cycle.h"
#include "multi-phy.h"
#include "stack/include/gp-sink-table.h"
#include "stack/include/gp-proxy-table.h"
#include "stack/include/source-route.h"
#include "stack/include/multi-network.h"
#include "stack/include/sl_zigbee_dhc.h"
#include "app/framework/plugin/dhc/dhc-ncp.h"

bool sli_zigbee_af_process_ezsp_command_certificate_based_key_exchange_cbke(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS: {
      sl_status_t status;
      status = sli_zigbee_stack_generate_cbke_keys();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CALCULATE_SMACS: {
      sl_status_t status;
      bool amInitiator;
      sl_zigbee_certificate_data_t partnerCertificate;
      sl_zigbee_public_key_data_t partnerEphemeralPublicKey;
      amInitiator = fetchInt8u();
      fetch_sl_zigbee_certificate_data_t(&partnerCertificate);
      fetch_sl_zigbee_public_key_data_t(&partnerEphemeralPublicKey);
      status = sli_zigbee_stack_calculate_smacs(amInitiator, &partnerCertificate, &partnerEphemeralPublicKey);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS_283K1: {
      sl_status_t status;
      status = sli_zigbee_stack_generate_cbke_keys_283k1();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CALCULATE_SMACS_283K1: {
      sl_status_t status;
      bool amInitiator;
      sl_zigbee_certificate_283k1_data_t partnerCertificate;
      sl_zigbee_public_key_283k1_data_t partnerEphemeralPublicKey;
      amInitiator = fetchInt8u();
      fetch_sl_zigbee_certificate_283k1_data_t(&partnerCertificate);
      fetch_sl_zigbee_public_key_283k1_data_t(&partnerEphemeralPublicKey);
      status = sli_zigbee_stack_calculate_smacs_283k1(amInitiator, &partnerCertificate, &partnerEphemeralPublicKey);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CLEAR_TEMPORARY_DATA_MAYBE_STORE_LINK_KEY: {
      sl_status_t status;
      bool storeLinkKey;
      storeLinkKey = fetchInt8u();
      status = sli_zigbee_stack_clear_temporary_data_maybe_store_link_key(storeLinkKey);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_CLEAR_TEMPORARY_DATA_MAYBE_STORE_LINK_KEY_283K1: {
      sl_status_t status;
      bool storeLinkKey;
      storeLinkKey = fetchInt8u();
      status = sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1(storeLinkKey);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_CERTIFICATE: {
      sl_status_t status;
      sl_zigbee_certificate_data_t localCert;
      status = sli_zigbee_stack_get_certificate(&localCert);
      appendInt32u(status);
      append_sl_zigbee_certificate_data_t(&localCert);
      break;
    }

    case SL_ZIGBEE_EZSP_GET_CERTIFICATE_283K1: {
      sl_status_t status;
      sl_zigbee_certificate_283k1_data_t localCert;
      status = sli_zigbee_stack_get_certificate_283k1(&localCert);
      appendInt32u(status);
      append_sl_zigbee_certificate_283k1_data_t(&localCert);
      break;
    }

    case SL_ZIGBEE_EZSP_DSA_SIGN: {
      sl_status_t status;
      uint8_t messageLength;
      uint8_t *messageContents;
      messageLength = fetchInt8u();
      messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
      status = sl_zigbee_af_ezsp_dsa_sign_command_cb(messageLength, messageContents);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_DSA_VERIFY: {
      sl_status_t status;
      sl_zigbee_message_digest_t digest;
      sl_zigbee_certificate_data_t signerCertificate;
      sl_zigbee_signature_data_t receivedSig;
      fetch_sl_zigbee_message_digest_t(&digest);
      fetch_sl_zigbee_certificate_data_t(&signerCertificate);
      fetch_sl_zigbee_signature_data_t(&receivedSig);
      status = sli_zigbee_stack_dsa_verify(&digest, &signerCertificate, &receivedSig);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_DSA_VERIFY_283K1: {
      sl_status_t status;
      sl_zigbee_message_digest_t digest;
      sl_zigbee_certificate_283k1_data_t signerCertificate;
      sl_zigbee_signature_283k1_data_t receivedSig;
      fetch_sl_zigbee_message_digest_t(&digest);
      fetch_sl_zigbee_certificate_283k1_data_t(&signerCertificate);
      fetch_sl_zigbee_signature_283k1_data_t(&receivedSig);
      status = sli_zigbee_stack_dsa_verify_283k1(&digest, &signerCertificate, &receivedSig);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SET_PREINSTALLED_CBKE_DATA: {
      sl_status_t status;
      sl_zigbee_public_key_data_t caPublic;
      sl_zigbee_certificate_data_t myCert;
      sl_zigbee_private_key_data_t myKey;
      fetch_sl_zigbee_public_key_data_t(&caPublic);
      fetch_sl_zigbee_certificate_data_t(&myCert);
      fetch_sl_zigbee_private_key_data_t(&myKey);
      status = sli_zigbee_stack_set_preinstalled_cbke_data(&caPublic, &myCert, &myKey);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_SAVE_PREINSTALLED_CBKE_DATA_283K1: {
      sl_status_t status;
      status = sl_zigbee_af_ezsp_save_preinstalled_cbke_data_283k1_command_cb();
      appendInt32u(status);
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
