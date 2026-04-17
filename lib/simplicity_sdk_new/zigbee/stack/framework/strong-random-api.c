/***************************************************************************//**
 * @file
 * @brief EmberZNet APIs for pseudo and true random number generation.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef PLATFORM_HEADER
  #include PLATFORM_HEADER
#endif

#include "include/sl_zigbee.h"
#include "internal/inc/internal-defs-patch.h"
#include "hal/hal.h"
#include "internal/inc/internal-defs-patch.h"

#if defined(MBEDTLS_CONFIG_FILE)
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#if (defined SL_CATALOG_ZIGBEE_STRONG_RANDOM_API_RADIO_PRESENT) || defined (SL_ZIGBEE_SCRIPTED_TEST)
#define USE_RADIO_API_FOR_TRNG
#elif (defined SL_CATALOG_ZIGBEE_STRONG_RANDOM_API_PSA_PRESENT)
#define USE_PSA_API_FOR_TRNG
#elif !defined(USE_RADIO_API_FOR_TRNG) && !defined(USE_PSA_API_FOR_TRNG) // None of the components present, then error
#error Must select one of the strong random api with radio or PSA Crypto
#endif // SL_CATALOG_ZIGBEE_STRONG_RANDOM_API_

#if defined(USE_PSA_API_FOR_TRNG)
#include "psa/crypto.h"
#else // defined(USE_PSA_API_FOR_TRNG)
extern bool sli_802154phy_radio_get_random_numbers(uint16_t *randomNumber, uint8_t count);
#endif // defined(USE_PSA_API_FOR_TRNG)

uint16_t sli_zigbee_stack_get_pseudo_random_number(void)
{
  return halCommonGetRandom();
}

sl_status_t sli_zigbee_stack_get_strong_random_number_array(uint16_t *randomNumber, uint8_t count)
{
#if defined(USE_PSA_API_FOR_TRNG)
  // Get random numbers, lenght is in unsigned char blocks, making it uint16_t
  psa_status_t status;
  status = psa_generate_random((uint8_t*)randomNumber, 2 * count);

  if (status != PSA_SUCCESS) {
#else // defined(USE_MBEDTLS_API_FOR_TRNG)
  // Get random numbers from the radio API
  if (!sli_802154phy_radio_get_random_numbers(randomNumber, count)) {
#endif // defined(USE_MBEDTLS_API_FOR_TRNG)
    return SL_STATUS_ZIGBEE_INSUFFICIENT_RANDOM_DATA;
  }

  return SL_STATUS_OK;
}

sl_zigbee_entropy_source_t sli_zigbee_stack_get_strong_random_entropy_source()
{
#if defined(USE_PSA_API_FOR_TRNG)
#if defined(MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG) || defined(MBEDTLS_ENTROPY_HARDWARE_ALT)
  return SL_ZIGBEE_ENTROPY_SOURCE_MBEDTLS_TRNG;
#else
  return SL_ZIGBEE_ENTROPY_SOURCE_MBEDTLS;
#endif
#else // defined(USE_PSA_API_FOR_TRNG)
  return SL_ZIGBEE_ENTROPY_SOURCE_RADIO;
#endif // defined(USE_PSA_API_FOR_TRNG)
}
