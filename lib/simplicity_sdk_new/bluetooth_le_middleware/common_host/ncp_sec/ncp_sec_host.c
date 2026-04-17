/*******************************************************************************
 * @file
 * @brief Secure NCP host program
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <openssl/conf.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include "app_log.h"

#include "sl_bt_api.h"
#include "sl_common.h"
#include "ncp_sec_host.h"

#define NCP_SEC_PAYLOAD_OVERHEAD  9
#define BGAPI_HDR_LEN_FIELD_LEN   2

typedef struct conn_nonce{
  uint32_t counter;
  uint8_t counter_hi;
  uint8_t host_iv[IV_SIZE];
  uint8_t target_iv[IV_SIZE];
}conn_nonce_t;

typedef struct ec_keypair{
  uint8_t pub[PUBLIC_KEYPAIR_SIZE];
  uint8_t priv[ECDH_PRIVATE_KEY_SIZE];
}ec_keypair_t;

typedef unsigned long ulong_t;

static uint8_t ccm_key[AES_CCM_KEY_SIZE];
static ec_keypair_t local_ec_key;
static conn_nonce_t sec_counter_in;
static conn_nonce_t sec_counter_out;

security_state_t security_state = SECURITY_STATE_UNDEFINED;

extern int sl_bgapi_user_cmd_increase_security(uint8_t *public_key,
                                               uint8_t *host_iv_to_target,
                                               uint8_t *host_iv_to_host);

SL_WEAK int sl_bgapi_user_cmd_increase_security(uint8_t *public_key,
                                                uint8_t *host_iv_to_target,
                                                uint8_t *host_iv_to_host)
{
  (void)(public_key);
  (void)(host_iv_to_target);
  (void)(host_iv_to_host);
  return 0;
}

#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/ec.h>

static sl_status_t ec_ephemeral_key(ec_keypair_t *key)
{
  sl_status_t e = SL_STATUS_ALLOCATION_FAILED;
  EVP_PKEY *pkey = NULL;
  EVP_PKEY_CTX *pctx = NULL;
  BIGNUM *bn_priv = NULL;
  int evp_error;

  do {
    uint8_t pub_buf[1 + PUBLIC_KEYPAIR_SIZE] = { 0 };
    size_t len = 0;
    OSSL_PARAM params[] = {
      OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, SN_X9_62_prime256v1, 0),
      OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT, "uncompressed", 0),
      OSSL_PARAM_construct_end()
    };

    pctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
    if (!pctx) {
      app_log_error("EVP_PKEY_CTX_new_id failed!" APP_LOG_NL);
      break;
    }

    if ((evp_error = EVP_PKEY_keygen_init(pctx)) <= 0) {
      e = SL_STATUS_INITIALIZATION;
      app_log_error("EVP_PKEY_keygen_init failed with error code %d." APP_LOG_NL, evp_error);
      break;
    }

    if ((evp_error = EVP_PKEY_CTX_set_params(pctx, params)) <= 0) {
      e = SL_STATUS_INVALID_PARAMETER;
      app_log_error("EVP_PKEY_CTX_set_params failed with error code %d." APP_LOG_NL, evp_error);
      break;
    }

    if ((evp_error = EVP_PKEY_generate(pctx, &pkey)) <= 0) {
      e = SL_STATUS_FAIL;
      app_log_error("EVP_PKEY_keygen failed with error code %d." APP_LOG_NL, evp_error);
      break;
    }

    if (!EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_PRIV_KEY, &bn_priv)) {
      app_log_error("EVP_PKEY_get_bn_param failed!" APP_LOG_NL);
      e = SL_STATUS_INVALID_SIGNATURE;
      break;
    }

    e = SL_STATUS_INVALID_COUNT;
    if (!bn_priv || ((len = BN_num_bytes(bn_priv)) != sizeof(key->priv))) {
      app_log_error("Invalid private key length of %lu." APP_LOG_NL, (ulong_t)len);
      break;
    }
    BN_bn2bin(bn_priv, key->priv);

    if (!EVP_PKEY_get_octet_string_param(pkey, OSSL_PKEY_PARAM_PUB_KEY, pub_buf, sizeof(pub_buf), &len)) {
      app_log_error("EVP_PKEY_get_octet_string_param failed!" APP_LOG_NL);
      e = SL_STATUS_INVALID_KEY;
      break;
    }

    if (len == sizeof(pub_buf)) {
      memcpy(key->pub, pub_buf + 1, sizeof(key->pub)); // Skip prefix byte - the UNCOMPRESSED flag
      e = SL_STATUS_OK;
    } else {
      app_log_error("Key size of %lu bytes to %lu buffer mismatch!" APP_LOG_NL, (ulong_t)len, (ulong_t)sizeof(pub_buf));
    }
  } while (0);

  EVP_PKEY_free(pkey);
  EVP_PKEY_CTX_free(pctx);
  BN_clear_free(bn_priv);

  if (e != SL_STATUS_OK) {
    // Get rid of the remains of a broken key
    memset(key->priv, 0, sizeof(key->priv));
    memset(key->pub, 0, sizeof(key->pub));
  }

  return e;
}

static EVP_PKEY *ec_key(const ec_keypair_t *key,
                        int both_parts)
{
  EVP_PKEY *result = NULL;
  EVP_PKEY_CTX *ctx = NULL;
  EVP_PKEY *pkey = NULL;
  BIGNUM *priv = NULL;
  OSSL_PARAM_BLD *param_bld;
  OSSL_PARAM *params = NULL;

  do {
    unsigned char pub_data[PUBLIC_KEYPAIR_SIZE + 1];
    const int selection = both_parts ? EVP_PKEY_KEYPAIR : EVP_PKEY_PUBLIC_KEY;

    pub_data[0] = POINT_CONVERSION_UNCOMPRESSED;
    memcpy(pub_data + 1, key->pub, sizeof(key->pub));
    param_bld = OSSL_PARAM_BLD_new();

    if (param_bld == NULL) {
      break;
    }

    priv = BN_bin2bn(key->priv, sizeof(key->priv), NULL);
    if (priv == NULL || !OSSL_PARAM_BLD_push_BN(param_bld, "priv", priv)) {
      break;
    }

    if (OSSL_PARAM_BLD_push_utf8_string(param_bld, "group",
                                        SN_X9_62_prime256v1, 0)
        && OSSL_PARAM_BLD_push_octet_string(param_bld, "pub",
                                            pub_data, sizeof(pub_data))) {
      params = OSSL_PARAM_BLD_to_param(param_bld);
    } else {
      break;
    }

    ctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);

    if (ctx == NULL
        || params == NULL
        || EVP_PKEY_fromdata_init(ctx) <= 0
        || EVP_PKEY_fromdata(ctx, &pkey, selection, params) <= 0) {
      break;
    } else {
      result = pkey;
      pkey = NULL; // Avoid freeing it as it will be in use from now on
    }
  } while (0);

  EVP_PKEY_free(pkey);
  EVP_PKEY_CTX_free(ctx);
  OSSL_PARAM_free(params);
  OSSL_PARAM_BLD_free(param_bld);
  BN_free(priv);

  return result;
}

static sl_status_t ecdh_secret(const ec_keypair_t *remote_ec_key)
{
  EVP_PKEY_CTX *ctxt = NULL;
  unsigned char *secret_ptr = NULL;
  EVP_PKEY *local_pkey = NULL;
  EVP_PKEY *remote_pkey = NULL;
  sl_status_t e = SL_STATUS_ALLOCATION_FAILED;

  do {
    size_t secret_len;
    uint8_t *hash = NULL;
    // Set up keys
    local_pkey = ec_key(&local_ec_key, 1);
    if (!local_pkey) {
      break;
    }

    remote_pkey = ec_key(remote_ec_key, 0);
    if (!remote_pkey) {
      break;
    }

    ctxt = EVP_PKEY_CTX_new(local_pkey, NULL);
    if (!ctxt) {
      break;
    }

    if (EVP_PKEY_derive_init(ctxt) < 1
        || EVP_PKEY_derive_set_peer(ctxt, remote_pkey) < 1
        || EVP_PKEY_derive(ctxt, NULL, &secret_len) < 1
        || secret_len != 32) {
      e = SL_STATUS_INITIALIZATION;
      break;
    }

    secret_ptr = OPENSSL_malloc(secret_len);
    if (!secret_ptr) {
      e = SL_STATUS_NO_MORE_RESOURCE;
      break;
    }

    if (EVP_PKEY_derive(ctxt, secret_ptr, &secret_len) < 1) {
      e = SL_STATUS_INVALID_KEY;
      break;
    }

    // Use sha256 to derive the AES CCM key.
    // NOTE: This is not thread safe
    hash = SHA256(secret_ptr, secret_len, NULL);
    if (!hash) {
      e = SL_STATUS_FAIL;
    } else {
      memcpy(ccm_key, hash, AES_CCM_KEY_SIZE);
      e = SL_STATUS_OK;
    }
  } while (0);

  (void)OPENSSL_free(secret_ptr);
  (void)EVP_PKEY_free(local_pkey);
  (void)EVP_PKEY_free(remote_pkey);
  (void)EVP_PKEY_CTX_free(ctxt);
  return e;
}

static sl_status_t aes_ccm_encrypt(const uint8_t *key, const uint8_t *nonce,
                                   const uint8_t *plain_text,
                                   const size_t text_len,
                                   const uint8_t *additional,
                                   const size_t additional_len,
                                   uint8_t *cipher_text, uint8_t *mac)
{
  EVP_CIPHER_CTX *ccm = NULL;
  sl_status_t r = SL_STATUS_ALLOCATION_FAILED;

  ccm = EVP_CIPHER_CTX_new();

  if (!ccm) {
    return r;
  }

  do {
    int len;

    if (EVP_EncryptInit_ex(ccm, EVP_aes_128_ccm(), NULL, NULL, NULL) != 1
        || EVP_CIPHER_CTX_ctrl(ccm, EVP_CTRL_CCM_SET_IVLEN,
                               NONCE_SIZE, NULL) != 1
        || EVP_CIPHER_CTX_ctrl(ccm, EVP_CTRL_CCM_SET_TAG,
                               MAC_LEN, NULL) != 1
        || EVP_EncryptInit_ex(ccm, NULL, NULL, key, nonce) != 1) {
      r = SL_STATUS_INITIALIZATION;
      break;
    }

    // Provide the total plain text length
    if (EVP_EncryptUpdate(ccm,
                          NULL, &len,
                          NULL, text_len) != 1) {
      r = SL_STATUS_BT_APPLICATION_ENCRYPTION_DECRYPTION_ERROR;
      break;
    }

    // Provide any AAD data. This can be called zero or one times as required
    if (additional) {
      if (EVP_EncryptUpdate(ccm,
                            NULL, &len,
                            additional, additional_len) != 1) {
        r = SL_STATUS_BT_APPLICATION_ENCRYPTION_DECRYPTION_ERROR;
        break;
      }
    }
    if ((size_t)len != additional_len) {
      r = SL_STATUS_FAIL;
      break;
    }

    // Provide the message to be encrypted, and obtain the encrypted output.
    // EVP_EncryptUpdate can only be called once for this
    if (EVP_EncryptUpdate(ccm,
                          cipher_text, &len,
                          plain_text, text_len) != 1) {
      r = SL_STATUS_BT_APPLICATION_ENCRYPTION_DECRYPTION_ERROR;
      break;
    }
    if ((size_t)len != text_len) {
      r = SL_STATUS_FAIL;
      break;
    }

    // Get the tag
    if (EVP_CIPHER_CTX_ctrl(ccm, EVP_CTRL_CCM_GET_TAG,
                            MAC_LEN, mac) != 1) {
      r = SL_STATUS_INVALID_SIGNATURE;
      break;
    }

    r = SL_STATUS_OK;
  } while (0);

  (void)EVP_CIPHER_CTX_free(ccm);
  return r;
}

static sl_status_t aes_ccm_decrypt(const uint8_t *key, const uint8_t *nonce,
                                   const uint8_t *cipher_text,
                                   const size_t text_len,
                                   const uint8_t *additional,
                                   const size_t additional_len,
                                   uint8_t *plain_text, const uint8_t *mac)
{
  EVP_CIPHER_CTX *ccm = NULL;
  sl_status_t r = SL_STATUS_ALLOCATION_FAILED;

  ccm = EVP_CIPHER_CTX_new();

  if (!ccm) {
    return r;
  }

  do {
    int len;
    if (EVP_DecryptInit_ex(ccm, EVP_aes_128_ccm(), NULL, NULL, NULL) != 1
        || EVP_CIPHER_CTX_ctrl(ccm, EVP_CTRL_CCM_SET_IVLEN,
                               NONCE_SIZE, NULL) != 1
        || EVP_CIPHER_CTX_ctrl(ccm, EVP_CTRL_CCM_SET_TAG,
                               MAC_LEN, (void *)mac) != 1
        || EVP_DecryptInit_ex(ccm, NULL, NULL, key, nonce) != 1) {
      r = SL_STATUS_INITIALIZATION;
      break;
    }

    // Provide the total plain text length
    if (EVP_DecryptUpdate(ccm,
                          NULL, &len,
                          NULL, text_len) != 1) {
      r = SL_STATUS_BT_APPLICATION_ENCRYPTION_DECRYPTION_ERROR;
      break;
    }

    // Provide any AAD data. This can be called zero or one times as required
    if (additional) {
      if (EVP_DecryptUpdate(ccm,
                            NULL, &len,
                            additional, additional_len) != 1) {
        r = SL_STATUS_BT_APPLICATION_ENCRYPTION_DECRYPTION_ERROR;
        break;
      }
    }
    if ((size_t)len != additional_len) {
      r = SL_STATUS_FAIL;
      break;
    }

    // Provide the message to be decrypted, and obtain the decrypted output.
    //   EVP_DecryptUpdate can be called multiple times if necessary
    if (EVP_DecryptUpdate(ccm,
                          plain_text, &len,
                          cipher_text, text_len) != 1) {
      r = SL_STATUS_SECURITY_DECRYPT_ERROR;
      break;
    }
    if ((size_t)len != text_len) {
      r = SL_STATUS_FAIL;
      break;
    }

    r = SL_STATUS_OK;
  } while (0);

  (void)EVP_CIPHER_CTX_free(ccm);
  return r;
}

static void increase_counter(conn_nonce_t *counter)
{
  if (counter->counter == UINT32_MAX) {
    counter->counter_hi++;
  }
  counter->counter++;
}

static void change_state(security_state_t ns)
{
  if (security_state == ns) {
    return;
  }
  security_state = ns;
  security_state_change_cb(security_state);
}

sl_status_t security_init()
{
  security_state = SECURITY_STATE_UNDEFINED;
  return SL_STATUS_OK;
}

void security_reset()
{
  change_state(SECURITY_STATE_UNENCRYPTED);
}

security_state_t  get_security_state()
{
  return security_state;
}

void security_start()
{
  switch (security_state) {
    case SECURITY_STATE_UNENCRYPTED: {
      app_log_info("Start encryption using OpenSSL 3.0" APP_LOG_NL);
      sec_counter_in.counter = 0;
      sec_counter_in.counter_hi = 0;
      sec_counter_out.counter = 0;
      sec_counter_out.counter_hi = 0;

      sl_status_t err = ec_ephemeral_key(&local_ec_key);
      if (err) {
        app_log_warning("EC keypair generation failed 0x%04x" APP_LOG_NL,
                        err);
        return;
      }
      int ret = RAND_bytes(sec_counter_out.host_iv,
                           sizeof(sec_counter_out.host_iv));
      if (ret != 1) {
        app_log_warning("Error generating random bytes 0x%lx" APP_LOG_NL,
                        ERR_get_error());
        return;
      }
      ret = RAND_bytes(sec_counter_in.host_iv, sizeof(sec_counter_in.host_iv));
      if (ret != 1) {
        app_log_warning("Error generating random bytes 0x%lx" APP_LOG_NL,
                        ERR_get_error());
        return;
      }
      security_state = SECURITY_STATE_INCREASE_SECURITY;
      sl_bgapi_user_cmd_increase_security(local_ec_key.pub,
                                          sec_counter_out.host_iv,
                                          sec_counter_in.host_iv);
      break;
    }
    default:
      break;
  }
}

/******************************************************************************
 * Callback which is called when security state changes.
 *
 * @note Weak implementation
 *****************************************************************************/
SL_WEAK void security_state_change_cb(security_state_t state)
{
  (void)state;
}

void security_increase_security_rsp(uint8_t *public_key,
                                    uint8_t *target_iv_to_target,
                                    uint8_t *target_iv_to_host)
{
  switch (security_state) {
    case SECURITY_STATE_INCREASE_SECURITY: {
      ec_keypair_t remote_ec_key = { 0 };
      memcpy(remote_ec_key.pub, public_key, PUBLIC_KEYPAIR_SIZE);
      memcpy(sec_counter_out.target_iv, target_iv_to_target,
             sizeof(sec_counter_out.target_iv));
      memcpy(sec_counter_in.target_iv, target_iv_to_host,
             sizeof(sec_counter_in.target_iv));
      sl_status_t err = ecdh_secret(&remote_ec_key);
      if (err) {
        app_log_warning("AES CCM key generation failed 0x%04x" APP_LOG_NL,
                        err);
        change_state(SECURITY_STATE_UNENCRYPTED);
        return;
      }
      change_state(SECURITY_STATE_ENCRYPTED);
      break;
    }
    default:
      break;
  }
}

void security_decrypt(char *src, char *dst, unsigned *len)
{
  if (security_state != SECURITY_STATE_ENCRYPTED) {
    memcpy(dst, src, *len);
  } else {
    security_decrypt_packet(src, dst, len);
  }
}

void security_decrypt_packet(char *src, char *dst, unsigned *len)
{
  if (*len < (NCP_SEC_PAYLOAD_OVERHEAD + BGAPI_HDR_LEN_FIELD_LEN) ) {
    app_log_warning("Invalid length parameter: %u, minimum length is %u" APP_LOG_NL, *len, NCP_SEC_PAYLOAD_OVERHEAD);
    *len = 0;
    return;
  }
  uint16_t new_length = (((uint16_t)(src[0] & 0x07) << 8) | (uint8_t)src[1]);
  if (new_length < NCP_SEC_PAYLOAD_OVERHEAD) {
    app_log_warning("Invalid packet length: %u" APP_LOG_NL, new_length);
    *len = 0;
    return;
  }
  new_length = new_length - NCP_SEC_PAYLOAD_OVERHEAD;

  //clr encrypted and length high bits
  *dst++ = ((src[0] & ~(1 << 6)) & 0xf8) | (uint8_t)(new_length >> 8);
  *dst++ = (uint8_t)(new_length & 0xff);

  // remove tag and counter value
  *len = *len - NCP_SEC_PAYLOAD_OVERHEAD;
  new_length += SL_BT_MSG_HEADER_LEN;
  if (new_length != (*len)) {
    app_log_warning("Packet length validation failed!" APP_LOG_NL);
    *len = 0;
    return;
  }

  //verify counter to prevent replay attacks
  conn_nonce_t nonce;
  memcpy(&nonce.counter, src + *len + 4, 4);
  memcpy(&nonce.counter_hi, src + *len + 8, 1);
  memcpy(nonce.host_iv, sec_counter_in.host_iv,
         sizeof(sec_counter_in.host_iv));
  memcpy(nonce.target_iv, sec_counter_in.target_iv,
         sizeof(sec_counter_in.target_iv));
  if ((nonce.counter_hi > sec_counter_in.counter_hi
       || (nonce.counter_hi == sec_counter_in.counter_hi
           && nonce.counter >= sec_counter_in.counter)) == 0) {
    app_log_warning("Error packet counter not valid" APP_LOG_NL);
    *len = 0;
    return;
  }

  uint8_t auth_data[7];
  memcpy(auth_data, src, BGAPI_HDR_LEN_FIELD_LEN);
  memcpy(auth_data + BGAPI_HDR_LEN_FIELD_LEN, &nonce.counter, 4);
  auth_data[6] = nonce.counter_hi;

  sl_status_t err = aes_ccm_decrypt(ccm_key, (uint8_t *)&nonce,
                                    (uint8_t *)src + BGAPI_HDR_LEN_FIELD_LEN, *len - BGAPI_HDR_LEN_FIELD_LEN,
                                    auth_data, sizeof(auth_data),
                                    (uint8_t *)dst, (uint8_t *)src + *len);
  if (err) {
    app_log_warning("Packet decryption failed 0x%04x, len: %u/%u" APP_LOG_NL, err, *len, new_length);
    app_log_hexdump_info(src, new_length);
    app_log_append(APP_LOG_NL);
    *len = 0;
    return;
  }

  // Update counter
  if (sec_counter_in.counter != nonce.counter
      || sec_counter_in.counter_hi != nonce.counter_hi) {
    sec_counter_in.counter = nonce.counter;
    sec_counter_in.counter_hi = nonce.counter_hi;
  }
  increase_counter(&sec_counter_in);
}

void security_encrypt(char *src, char *dst, unsigned *len)
{
  if (security_state != SECURITY_STATE_ENCRYPTED) {
    memcpy(dst, src, *len);
  } else {
    security_encrypt_packet(src, dst, len);
  }
}

void security_encrypt_packet(char *src, char *dst, unsigned *len)
{
  uint16_t new_length = (((uint16_t)(src[0] & 0x07) << 8) | (uint8_t)src[1]) + NCP_SEC_PAYLOAD_OVERHEAD;
  *dst++ = ((src[0] | (1 << 6)) & 0xf8) | (uint8_t)(new_length >> 8);//set encrypted and length high bits
  *dst++ = (uint8_t)(new_length & 0xff);

  uint8_t auth_data[7];
  memcpy(auth_data, dst - 2, 2);
  memcpy(auth_data + 2, &sec_counter_out.counter, 4);
  auth_data[6] = sec_counter_out.counter_hi;

  sl_status_t err = aes_ccm_encrypt(ccm_key, (uint8_t *)&sec_counter_out,
                                    (uint8_t *)src + 2, *len - 2,
                                    auth_data, 7,
                                    (uint8_t *)dst, (uint8_t *)dst + *len - 2);
  if (err) {
    app_log_warning("Packet encryption failed 0x%04x" APP_LOG_NL, err);
    *len = 0;
    return;
  }

  // message + tag
  dst += *len - 2 + MAC_LEN;

  // add counter to end
  memcpy(dst, auth_data + 2, 5);

  *len = *len + NCP_SEC_PAYLOAD_OVERHEAD;

  increase_counter(&sec_counter_out);
}
