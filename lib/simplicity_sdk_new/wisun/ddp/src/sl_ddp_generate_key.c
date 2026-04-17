/***************************************************************************//**
 * @file sl_ddp_generate_key.c
 * @brief DDP command for generating a PSA Crypto key
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
#include "sl_ddp_generate_key.h"

/**************************************************************************//**
 * Generate a PSA Crypto key.
 *****************************************************************************/
sl_ddp_status_t sl_ddp_generate_key(const uint8_t *input,
                                    uint16_t input_len,
                                    uint8_t *output,
                                    uint16_t output_size,
                                    uint16_t *output_len)
{
  if (input == NULL || input_len < sizeof(sl_ddp_generate_key_req_t)) {
    return SL_DDP_STATUS_IN_ARGS_NOT_VALID_ERR;
  }

  psa_status_t ret;
  psa_key_id_t gen_key_id;
  psa_key_attributes_t attr;
  size_t key_len;
  const sl_ddp_generate_key_req_t *req = (const sl_ddp_generate_key_req_t *)input;
  sl_ddp_generate_key_rsp_t *rsp = (sl_ddp_generate_key_rsp_t *)output;

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
  ret = psa_generate_key(&attr, &gen_key_id);
  if (ret == PSA_ERROR_ALREADY_EXISTS) {
    return SL_DDP_STATUS_PSA_ALREADY_EXISTS_ERR;
  } else if (ret != PSA_SUCCESS) {
    return SL_DDP_STATUS_PSA_GENERATE_KEY_ERR;
  }
  if ((mbedtls_svc_key_id_t)req->key_id != (mbedtls_svc_key_id_t)gen_key_id) {
    return SL_DDP_STATUS_PSA_GENERATED_KEY_ID_ERR;
  }

  if (output && output_len) {
    ret = psa_export_public_key((mbedtls_svc_key_id_t)req->key_id, rsp->key, output_size, &key_len);
    if (ret != PSA_SUCCESS) {
      return SL_DDP_STATUS_PSA_EXPORT_KEY_ERR;
    }

    rsp->key_len = key_len;
    *output_len = sizeof(*rsp) + rsp->key_len;
  }

  return SL_DDP_STATUS_SUCCESS;
}
