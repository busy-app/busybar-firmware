/***************************************************************************//**
 * @file sl_ddp_inject_key.c
 * @brief DDP command for injecting a PSA Crypto key
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdlib.h>
#include "psa/crypto.h"
#include "sl_ddp_inject_key.h"

/**************************************************************************//**
 * Inject a PSA Crypto key.
 *****************************************************************************/
sl_ddp_status_t sl_ddp_inject_key(const uint8_t *input,
                                  uint16_t input_len,
                                  uint8_t *output,
                                  uint16_t output_size,
                                  uint16_t *output_len)
{
  mbedtls_svc_key_id_t key_id;
  (void)output;
  (void)output_len;
  (void)output_size;

  if (input == NULL || input_len < sizeof(sl_ddp_inject_key_req_t)) {
    return SL_DDP_STATUS_IN_ARGS_NOT_VALID_ERR;
  }

  psa_key_attributes_t attr;
  psa_status_t ret;
  const sl_ddp_inject_key_req_t *req = (const sl_ddp_inject_key_req_t *)input;

  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    return SL_DDP_STATUS_PSA_CRYPTO_INIT_ERR;
  }

  attr = psa_key_attributes_init();
  psa_set_key_lifetime(&attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION((psa_key_lifetime_t)req->lifetime, (psa_key_location_t)req->location));
  psa_set_key_id(&attr, (mbedtls_svc_key_id_t)req->key_id);
  psa_set_key_usage_flags(&attr, (psa_key_usage_t)req->usage_flags);
  psa_set_key_bits(&attr, (size_t)req->bits);
  psa_set_key_type(&attr, (psa_key_type_t)req->type);
  psa_set_key_algorithm(&attr, (psa_algorithm_t)req->algo);
  ret = psa_import_key(&attr, req->key, req->key_len, &key_id);
  if (ret != PSA_SUCCESS) {
    return SL_DDP_STATUS_PSA_IMPORT_KEY_ERR;
  }

  return SL_DDP_STATUS_SUCCESS;
}
