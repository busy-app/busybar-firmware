/***************************************************************************//**
 * @file
 * @brief ECDSA signing functionality for Silicon Labs bootloader
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "btl_security_ecdsa.h"
#include "em_device.h"

#include <stddef.h>
#include <string.h> // For memset

#if defined(CRYPTO_PRESENT)

#include "ecc/ecc.h"
#include "em_cmu.h"

#if !defined(CRYPTO) && defined(CRYPTO0)
#define cmuClock_CRYPTO cmuClock_CRYPTO0
#define CRYPTO CRYPTO0
#endif

/** Verify the ECDSA signature of the SHA hash, using
 *  the public key in the relevant token, with the signature contained in
 *  the byte arrays pointed to.
 */
int32_t btl_verifyEcdsaP256r1(const uint8_t *sha256,
                              const uint8_t *signatureR,
                              const uint8_t *signatureS,
                              const uint8_t *keyX,
                              const uint8_t *keyY)
{
  if ((sha256 == NULL) || (signatureR == NULL) || (signatureS == NULL) || (keyX == NULL) || (keyY == NULL)) {
    return BOOTLOADER_ERROR_SECURITY_INVALID_PARAM;
  }

  // Re-enable the clock of the CRYPTO module, since mbedtls turned it off
  CMU_ClockEnable(cmuClock_CRYPTO, true);

  ECC_Point_t pubkey;
  memset(&pubkey, 0, sizeof(ECC_Point_t));
  ECC_ByteArrayToBigInt(pubkey.X, keyX);
  ECC_ByteArrayToBigInt(pubkey.Y, keyY);

  ECC_EcdsaSignature_t ecc_signature = { 0 };
  ECC_ByteArrayToBigInt(ecc_signature.r, signatureR);
  ECC_ByteArrayToBigInt(ecc_signature.s, signatureS);

  return ECC_ECDSA_VerifySignatureP256(CRYPTO,
                                       sha256,
                                       sizeof(ECC_BigInt_t),
                                       &pubkey,
                                       &ecc_signature);
}

#elif defined(SEMAILBOX_PRESENT)

#include "sli_se_manager_mailbox.h"

/** Verify the ECDSA signature of the SHA hash, using
 *  the public key in the relevant token, with the signature contained in
 *  the byte arrays pointed to.
 */
int32_t btl_verifyEcdsaP256r1(const uint8_t *sha256,
                              const uint8_t *signatureR,
                              const uint8_t *signatureS,
                              const uint8_t *keyX,
                              const uint8_t *keyY)
{
#if defined(_CMU_CLKEN1_SEMAILBOXHOST_MASK)
  CMU->CLKEN1_SET = CMU_CLKEN1_SEMAILBOXHOST;
#endif

  uint32_t keyspec;
  if ((sha256 == NULL) || (signatureR == NULL) || (signatureS == NULL)) {
    return BOOTLOADER_ERROR_SECURITY_INVALID_PARAM;
  }

  if ((keyX == NULL) || (keyY == NULL)) {
    // Keyspec for secp256r1, key by reference.
    keyspec = 0x80000000      // ECC
              | 31            // ECC-256
              | (1 << 10)     // SignKey
              | (1 << 13)     // PubKey
              | (0x1 << 26)   // Volatile key
              | (0xFC << 16); // KEY_INDEX_VK_MCU
  } else {
    // Keyspec for secp256r1, given public signing key
    keyspec = 0x80000000      // ECC
              | 31            // ECC-256
              | (1 << 10)     // SignKey
              | (1 << 13);    // PubKey
  }

  sli_se_mailbox_command_t command = SLI_SE_MAILBOX_COMMAND_DEFAULT(SLI_SE_COMMAND_SIGNATURE_VERIFY);
  sli_se_mailbox_command_add_parameter(&command, keyspec); // 0 = key in host memory
  sli_se_mailbox_command_add_parameter(&command, 32); // length of hash

  sli_se_datatransfer_t key_x = SLI_SE_DATATRANSFER_DEFAULT((uint8_t *)keyX, 32);
  sli_se_datatransfer_t key_y = SLI_SE_DATATRANSFER_DEFAULT((uint8_t *)keyY, 32);
  if ((keyX != NULL) && (keyY != NULL)) {
    sli_se_mailbox_command_add_input(&command, &key_x);
    sli_se_mailbox_command_add_input(&command, &key_y);
  }

  sli_se_datatransfer_t hash = SLI_SE_DATATRANSFER_DEFAULT((uint8_t *)sha256, 32);
  sli_se_mailbox_command_add_input(&command, &hash);

  sli_se_datatransfer_t signature_r = SLI_SE_DATATRANSFER_DEFAULT((uint8_t *)signatureR, 32);
  sli_se_datatransfer_t signature_s = SLI_SE_DATATRANSFER_DEFAULT((uint8_t *)signatureS, 32);
  sli_se_mailbox_command_add_input(&command, &signature_r);
  sli_se_mailbox_command_add_input(&command, &signature_s);

  sli_se_mailbox_execute_command(&command);

  volatile sli_se_mailbox_response_t response = 0x12345678U;
  response = sli_se_mailbox_read_response();

  if (response == SLI_SE_RESPONSE_OK) {
    return BOOTLOADER_OK;
  }

  return BOOTLOADER_ERROR_SECURITY_REJECTED;
}

#elif defined(CRYPTOACC_PRESENT)

#include "sx_errors.h"
#include "sx_ecdsa_alg.h"

#define ECC_COOR_SIZE_IN_BYTES       (32U)

/** Verify the ECDSA signature of the SHA hash, using
 *  the public key in the relevant token, with the signature contained in
 *  the byte arrays pointed to.
 */
int32_t btl_verifyEcdsaP256r1(const uint8_t *sha256,
                              const uint8_t *signatureR,
                              const uint8_t *signatureS,
                              const uint8_t *keyX,
                              const uint8_t *keyY)
{
  if ((sha256 == NULL) || (signatureR == NULL) || (signatureS == NULL) || (keyX == NULL) || (keyY == NULL)) {
    return BOOTLOADER_ERROR_SECURITY_INVALID_PARAM;
  }

  volatile uint32_t res = CRYPTOLIB_CRYPTO_ERR;
  uint32_t pub[ECC_COOR_SIZE_IN_BYTES * 2U / sizeof(uint32_t)] = { 0 };
  uint32_t signature[ECC_COOR_SIZE_IN_BYTES * 2U / sizeof(uint32_t)] = { 0 };

  CMU->CLKEN1_SET = CMU_CLKEN1_CRYPTOACC;
  CMU->CRYPTOACCCLKCTRL_SET = (CMU_CRYPTOACCCLKCTRL_PKEN | CMU_CRYPTOACCCLKCTRL_AESEN);

  uint32_t keyLen = 32;

  block_t  _pub = { 0 };
  block_t  _signature = { 0 };
  block_t  _digest = { 0 };

  memcpy(signature, signatureR, keyLen);
  memcpy((&signature[keyLen / sizeof(uint32_t)]), signatureS, keyLen);

  memcpy(pub, keyX, keyLen);
  memcpy((&pub[keyLen / sizeof(uint32_t)]), keyY, keyLen);

  _pub = block_t_convert(pub, keyLen * 2);
  _signature = block_t_convert(signature, keyLen * 2);
  _digest = block_t_convert(sha256, keyLen);

  res = ecdsa_verify_signature_digest(&sx_ecc_curve_p256, _digest, _pub, _signature);

  if (res == CRYPTOLIB_SUCCESS) {
    return BOOTLOADER_OK;
  }

  return BOOTLOADER_ERROR_SECURITY_REJECTED;
}

#endif
