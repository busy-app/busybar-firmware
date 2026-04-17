/***************************************************************************//**
 * @file sl_wisun_ddp_generate_csr.c
 * @brief DDP command for generating a Wi-SUN CSR
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
#include <string.h>
#include "mbedtls/pk.h"
#include "mbedtls/psa_util.h"
#include "mbedtls/x509_csr.h"
#include "sl_ddp_common.h"
#include "sl_wisun_ddp_generate_csr.h"

/**************************************************************************//**
 * Generate a Wi-SUN CSR.
 *****************************************************************************/
sl_ddp_status_t sl_wisun_ddp_generate_csr(const uint8_t *input,
                                          uint16_t input_len,
                                          uint8_t *output,
                                          uint16_t output_size,
                                          uint16_t *output_len)
{
  mbedtls_pk_context pk;
  mbedtls_x509write_csr csr;
  sl_ddp_status_t status = SL_DDP_STATUS_SUCCESS;
  int ret;
  const sl_wisun_ddp_generate_csr_req_t *req;
  sl_wisun_ddp_generate_csr_rsp_t *rsp;

  if (input == NULL || input_len < sizeof(sl_wisun_ddp_generate_csr_req_t)) {
    return SL_DDP_STATUS_IN_ARGS_NOT_VALID_ERR;
  }

  if (output == NULL || output_len == NULL) {
    return SL_DDP_STATUS_OUT_ARGS_NOT_VALID_ERR;
  }

  req = (const sl_wisun_ddp_generate_csr_req_t *)input;
  rsp = (sl_wisun_ddp_generate_csr_rsp_t *)output;

  mbedtls_x509write_csr_init(&csr);
  mbedtls_pk_init(&pk);

  // EC key-pair is loaded from PSA since the private key is needed
  // for the signature
  ret = mbedtls_pk_setup_opaque(&pk, req->key_id);
  SL_DDP_ERROR_CHECK_SET_STATUS(ret >= 0, SL_DDP_STATUS_UNKNOWN);

  mbedtls_x509write_csr_set_key(&csr, &pk);

  // Signature algorithm is ecdsa-with-SHA256
  mbedtls_x509write_csr_set_md_alg(&csr, MBEDTLS_MD_SHA256);

  ret = mbedtls_x509write_csr_der(&csr, rsp->csr,
                                  output_size - sizeof(*rsp),
                                  mbedtls_psa_get_random, MBEDTLS_PSA_RANDOM_STATE);
  SL_DDP_ERROR_CHECK_SET_STATUS(ret >= 0, SL_DDP_STATUS_UNKNOWN);

  // mbedtls_x509write_csr_der writes to the END of the buffer,
  // move it to the beginning.
  memmove(rsp->csr, rsp->csr + output_size - sizeof(*rsp) - ret, ret);

  // Success
  rsp->csr_len = ret;
  *output_len = sizeof(*rsp) + rsp->csr_len;

error_handler:

  mbedtls_pk_free(&pk);
  mbedtls_x509write_csr_free(&csr);

  return status;
}
