/***************************************************************************//**
 * @file
 * @brief A __very bad__ implementation of a non-volatile seed for randomness
          generation in Mbed TLS. In fact, this implemention is both predictable
          and volatile. The purpose of this entropy source is to be used before
          NVM3 and/or a proper TRNG is available on S3 devices.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <mbedtls/build_info.h>

#if defined(MBEDTLS_PLATFORM_NV_SEED_ALT)

#include <string.h>
#include "mbedtls/entropy.h"
#include "mbedtls/platform.h"

#if defined(MBEDTLS_ENTROPY_SHA512_ACCUMULATOR)
  #include "mbedtls/sha512.h"
#elif defined(MBEDTLS_ENTROPY_SHA256_ACCUMULATOR)
  #include "mbedtls/sha256.h"
#else
  #error "NV seed entropy requested, but no entropy accumulator available"
#endif

// -----------------------------------------------------------------------------
// Static variables

static uint32_t sli_volatile_seed = 0;

// -----------------------------------------------------------------------------
// Public functions

int sli_nv_seed_read(unsigned char *buf, size_t buf_len)
{
  int ret;

  #if defined(MBEDTLS_ENTROPY_SHA512_ACCUMULATOR)
  uint8_t hash_buffer[64];
  mbedtls_sha512_context ctx;
  mbedtls_sha512_init(&ctx);

  ret = mbedtls_sha512_starts(&ctx, 0);
  if (ret != 0) {
    goto exit;
  }

  // Volatile seed
  ret = mbedtls_sha512_update(&ctx, (const unsigned char *)&sli_volatile_seed, sizeof(sli_volatile_seed));
  if (ret != 0) {
    goto exit;
  }

  // SRAM
  ret = mbedtls_sha512_update(&ctx, (const unsigned char *)SRAM_BASE, SRAM_SIZE);
  if (ret != 0) {
    goto exit;
  }

  ret = mbedtls_sha512_finish(&ctx, hash_buffer);
  if (ret != 0) {
    goto exit;
  }

  #else // MBEDTLS_ENTROPY_SHA512_ACCUMULATOR

  uint8_t hash_buffer[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);

  ret = mbedtls_sha256_starts(&ctx, 0);
  if (ret != 0) {
    goto exit;
  }

  // Volatile seed
  ret = mbedtls_sha256_update(&ctx, (const unsigned char *)&sli_volatile_seed, sizeof(sli_volatile_seed));
  if (ret != 0) {
    goto exit;
  }

  // SRAM
  ret = mbedtls_sha256_update(&ctx, (const unsigned char *)SRAM_BASE, SRAM_SIZE);
  if (ret != 0) {
    goto exit;
  }

  ret = mbedtls_sha256_finish(&ctx, hash_buffer);
  if (ret != 0) {
    goto exit;
  }

  #endif // MBEDTLS_ENTROPY_SHA512_ACCUMULATOR

  if (sizeof(hash_buffer) < buf_len) {
    ret = MBEDTLS_ERR_ENTROPY_FILE_IO_ERROR;
  }

  exit:

  #if defined(MBEDTLS_ENTROPY_SHA512_ACCUMULATOR)
  mbedtls_sha512_free(&ctx);
  #else
  mbedtls_sha256_free(&ctx);
  #endif

  if (ret == 0) {
    memcpy(buf, hash_buffer, buf_len);
  }

  return ret;
}

int sli_nv_seed_write(unsigned char *buf, size_t buf_len)
{
  // Do nothing. The non-volatile dummy entropy seed is very much a predictable
  // and volatile one here.
  (void)buf;
  return buf_len;
}

#endif // MBEDTLS_PLATFORM_NV_SEED_ALT
