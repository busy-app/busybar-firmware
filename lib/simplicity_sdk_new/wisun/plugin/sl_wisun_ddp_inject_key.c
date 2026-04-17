/***************************************************************************//**
 * @file sl_wisun_ddp_inject_key.c
 * @brief DDP command for injecting a Wi-SUN device key pair
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
#include "em_system.h"
#include "psa/crypto.h"
#include "sl_ddp_common.h"
#include "sl_ddp_inject_key.h"
#include "sl_wisun_ddp_inject_key.h"

// DDP command request
static uint32_t key_input[32];

/**************************************************************************//**
 * Inject a Wi-SUN device key pair.
 *****************************************************************************/
sl_ddp_status_t sl_wisun_ddp_inject_key(const uint8_t *input,
                                        uint16_t input_len,
                                        uint8_t *output,
                                        uint16_t output_size,
                                        uint16_t *output_len)
{
  const sl_wisun_ddp_inject_key_req_t *req;
  sl_ddp_inject_key_req_t *key_req;
  sl_ddp_status_t status = SL_DDP_STATUS_SUCCESS;
  uint32_t location = PSA_KEY_LOCATION_LOCAL_STORAGE;
  (void)output;
  (void)output_size;
  (void)output_len;

  if (input == NULL || input_len < sizeof(sl_wisun_ddp_inject_key_req_t)) {
    return SL_DDP_STATUS_IN_ARGS_NOT_VALID_ERR;
  }

#if defined(SEMAILBOX_PRESENT)
  if (SYSTEM_GetSecurityCapability() == securityCapabilityVault)
  {
    // If the device has Secure Vault, always use wrapped keys
    location = PSA_KEY_LOCATION_SLI_SE_OPAQUE;
  }
#endif

  req = (const sl_wisun_ddp_inject_key_req_t *)input;
  key_req = (sl_ddp_inject_key_req_t *)key_input;
  key_req->lifetime = PSA_KEY_LIFETIME_PERSISTENT;
  key_req->location = location;
  key_req->usage_flags = PSA_KEY_USAGE_SIGN_HASH;
  key_req->bits = 256;
  key_req->algo = PSA_ALG_ECDSA(PSA_ALG_SHA_256);
  key_req->type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
  key_req->key_id = req->key_id;
  key_req->key_len = req->key_len;
  memcpy(key_req->key, req->key, req->key_len);

  status = sl_ddp_inject_key((const uint8_t *)key_req,
                             sizeof(*key_req),
                             NULL,
                             0,
                             NULL);
  SL_DDP_ERROR_CHECK(status == SL_DDP_STATUS_SUCCESS);

error_handler:

  return status;
}
