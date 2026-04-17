/***************************************************************************//**
 * @file
 * @brief PSA Crypto DDP component initialization
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdbool.h>
#include "em_device.h"
#include "sl_ddp_cmd_psa_init.h"

#if !defined(SEMAILBOX_PRESENT) && defined(CRYPTOACC_PRESENT)
#include "fih.h"
#include "sl_se_manager.h"
#include "sl_se_manager_util.h"
#include "sli_se_manager_mailbox.h"
#include "psa/internal_trusted_storage.h"
#include "sli_psa_driver_common.h"

// -----------------------------------------------------------------------------
// Definitions

// Negative initialisation value.
#define A_NEGATIVE_VALUE                    (0xAAAA5555)
#define ROOT_MAILBOX_SIZE                   (512UL)
#define SLI_VERSION_REMOVE_DIE_ID(version)  ((version) & 0x00FFFFFFU)

// The oldest supported HSE/VSE firmware versions
  #if (_SILICON_LABS_32B_SERIES_2_CONFIG >= 3)
    #define OLDEST_SUPPORTED_SE_FW_VERSION (0x00020201)  // v2.2.1
  #else
    #define OLDEST_SUPPORTED_SE_FW_VERSION (0x0001020e)  // v1.2.14
  #endif
#endif // !SEMAILBOX_PRESENT && CRYPTOACC_PRESENT

// -----------------------------------------------------------------------------
// Forward declaration of private functions

#if !defined(SEMAILBOX_PRESENT) && defined(CRYPTOACC_PRESENT)
static fih_int __attribute__ ((noinline)) setup_storage_root_key(void);
static fih_int __attribute__ ((noinline)) vse_is_mailbox_valid(void);
static fih_int __attribute__ ((noinline)) vse_get_reply_ptr(uint32_t **vse_reply_ptr);
static fih_int __attribute__ ((noinline)) vse_get_se_version(const uint32_t *vse_reply_ptr, uint32_t *se_version);
static fih_int __attribute__ ((noinline)) vse_get_reply_status(const uint32_t *vse_reply_ptr, uint32_t *vse_reply_status);
static fih_int __attribute__ ((noinline)) vse_get_data_length(const uint32_t *vse_reply_ptr, uint32_t *vse_data_length);
static fih_int __attribute__ ((noinline)) setup_storage_root_key(void);
static fih_int __attribute__ ((noinline)) check_se_version(uint32_t se_version);
#endif // !SEMAILBOX_PRESENT && CRYPTOACC_PRESENT

static bool initialized = false;

// -----------------------------------------------------------------------------
// Public functions

// PSA Crypto DDP component initialization.
void sl_ddp_cmd_psa_init(void)
{
#if !defined(SEMAILBOX_PRESENT) && defined(CRYPTOACC_PRESENT)
  fih_int fih_rc = FIH_FAILURE;
  FIH_CALL(setup_storage_root_key, fih_rc);
  if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
    initialized = false;
  } else {
    initialized = true;
  }
#else // !SEMAILBOX_PRESENT && CRYPTOACC_PRESENT
  // Nothing to do.
  initialized = true;
#endif // !SEMAILBOX_PRESENT && CRYPTOACC_PRESENT
}

// Is the component initialized?
bool sl_ddp_cmd_psa_is_initialized(void)
{
  return initialized;
}

// -----------------------------------------------------------------------------
// Private functions

#if !defined(SEMAILBOX_PRESENT) && defined(CRYPTOACC_PRESENT)
static fih_int __attribute__ ((noinline)) setup_storage_root_key(void)
{
  fih_int fih_rc = FIH_FAILURE; // if fatal_error is glitched, return failure

  // Size of VSE Output Mailbox instance.
  #define VSE_REPLY_SIZE_BYTES                          20UL
  #define VSE_REPLY_SIZE_WORDS         (VSE_REPLY_SIZE_BYTES / sizeof(uint32_t))
  // The data field size is the full output mailbox minus
  // the header/reply structure which is 20 bytes.
  #define ROOT_MAILBOX_DATA_SIZE      (ROOT_MAILBOX_SIZE - VSE_REPLY_SIZE_BYTES)
  #define VSE_REPLY_STATUS_TRUSTZONE_ROOT_KEY      (1 << 22)
  #define TZ_SRK_SIZE_WORDS                               8
  typedef struct {
    uint32_t data[TZ_SRK_SIZE_WORDS];
    uint32_t checksum;
  } vse_srk_t;

  #define FIH_INIT_STEP_SETUP_STORAGE_ROOT_KEY  8

  uint32_t *vse_reply = NULL;
  uint32_t se_version = 0;
  uint32_t vse_reply_status = 0;
  uint32_t vse_data_length = ROOT_MAILBOX_SIZE; // initialise to invalid size
  vse_srk_t *srk;
  uint32_t checksum = 0;
  volatile psa_status_t status = PSA_ERROR_GENERIC_ERROR;

  FIH_CFI_STEP_INIT(FIH_INIT_STEP_SETUP_STORAGE_ROOT_KEY);

  // Check that the VSE mailbox is valid
  FIH_CALL(vse_is_mailbox_valid, fih_rc);
  if (fih_not_eq(fih_rc, fih_int_encode(SLI_SE_RESPONSE_OK))) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Get pointer to the VSE output mailbox reply structure
  FIH_CALL(vse_get_reply_ptr, fih_rc, &vse_reply);
  if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
    goto exit; // if fatal_error is glitched
  }
  if (fih_eq(fih_int_encode((int32_t)vse_reply),
             fih_int_encode((int32_t)NULL))) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Get VSE firmware version
  FIH_CALL(vse_get_se_version, fih_rc, vse_reply, &se_version);
  if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
    goto exit; // if fatal_error is glitched
  }

  // Verify that the SE firmware version is accepted.
  FIH_CALL(check_se_version, fih_rc, se_version);
  if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Verify that the TrustZone Root Key flag is present in the reply status
  FIH_CALL(vse_get_reply_status, fih_rc, vse_reply, &vse_reply_status);
  if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
    goto exit; // if fatal_error is glitched
  }
  if (fih_eq(fih_int_encode(vse_reply_status & VSE_REPLY_STATUS_TRUSTZONE_ROOT_KEY),
             fih_int_encode(0))) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Read the SRK next to the mailbox, i.e. after the VSE reply header, data length and
  // checksum (+1).
  FIH_CALL(vse_get_data_length, fih_rc, vse_reply, &vse_data_length);
  if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
    goto exit; // if fatal_error is glitched
  }
  if (vse_data_length > ROOT_MAILBOX_DATA_SIZE) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Set pointer to the SRK now as we determined the VSE data length
  srk = (vse_srk_t*)(vse_reply + VSE_REPLY_SIZE_WORDS + vse_data_length + 1);

  // Verify checksum
  for (uint32_t i = 0; i < sizeof(srk->data) / sizeof(uint32_t); ++i) {
    checksum ^= srk->data[i];
  }
  if (fih_not_eq(fih_int_encode(checksum), fih_int_encode(srk->checksum))) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Set the ITS root key. Note that this function is not dependent on PSA Crypto
  // being initialized.
  status = sli_psa_its_set_root_key((uint8_t*)srk->data, sizeof(srk->data));
  if (fih_not_eq(fih_int_encode(status), fih_int_encode(PSA_SUCCESS))) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Clear the SRK from the mailbox location
  status = PSA_ERROR_GENERIC_ERROR;
  status = sli_psa_zeroize(srk, sizeof(vse_srk_t));
  if (fih_not_eq(fih_int_encode(status), fih_int_encode(PSA_SUCCESS))) {
    goto exit; // if fatal_error is glitched
  } else {
    fih_rc = FIH_SUCCESS;
  }

  FIH_CFI_STEP_DECREMENT();

  FIH_RET(fih_rc);

  exit:
  FIH_CFI_STEP_ERR_RESET();
  FIH_RET(FIH_FAILURE);
}

static fih_int __attribute__ ((noinline)) vse_is_mailbox_valid(void)
{
  if (!sli_vse_mailbox_is_output_valid()) {
    FIH_RET(fih_int_encode(SLI_SE_RESPONSE_MAILBOX_INVALID));
  } else {
    FIH_RET(fih_int_encode(SLI_SE_RESPONSE_OK));
  }
}

static fih_int __attribute__ ((noinline)) vse_get_reply_ptr(uint32_t **vse_reply_ptr)
{
  #if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_AHBRADIO_S)) \
  || (defined(SL_TRUSTZONE_PERIPHERAL_AHBRADIO_S) && SL_TRUSTZONE_PERIPHERAL_AHBRADIO_S))

    #define RDMEM_FRCRAM_MEM_BASE RDMEM_FRCRAM_S_MEM_BASE
    #define ROOT_MAILBOX_OUTPUT_BASE SYSCFG->ROOTDATA1;
    #define ROOT_MAILBOX_OUTPUT_BASE_EXPECTED ROOT_MAILBOX_OUTPUT_S_BASE
  #else
    #define RDMEM_FRCRAM_MEM_BASE RDMEM_FRCRAM_NS_MEM_BASE
  // VSE will always output the secure address, if NS is desired, caculate the NS address.
    #define ROOT_MAILBOX_OUTPUT_BASE (SYSCFG->ROOTDATA1 - RDMEM_FRCRAM_S_MEM_BASE + RDMEM_FRCRAM_NS_MEM_BASE)
    #define ROOT_MAILBOX_OUTPUT_BASE_EXPECTED (RDMEM_FRCRAM_NS_MEM_END + 1 - ROOT_MAILBOX_SIZE)
  #endif

  CMU_TypeDef *cmu = CMU_NS;

  // Store current CLKEN0 state
  uint32_t clken0_state = cmu->CLKEN0;

  cmu->CLKEN0_SET = CMU_CLKEN0_SYSCFG;

  // Get pointer to VSE mailbox reply structure
  *vse_reply_ptr = (uint32_t*)(ROOT_MAILBOX_OUTPUT_BASE);

  // Restore prior CLKEN0 state
  cmu->CLKEN0 = clken0_state;

  if ((uint32_t) *vse_reply_ptr > ROOT_MAILBOX_OUTPUT_BASE_EXPECTED
      || (uint32_t)*vse_reply_ptr < RDMEM_FRCRAM_MEM_BASE) {
    FIH_RET(FIH_FAILURE);
  }
  FIH_RET(FIH_SUCCESS);
}

static fih_int __attribute__ ((noinline)) vse_get_se_version(const uint32_t *vse_reply_ptr, uint32_t *se_version)
{
  *se_version = vse_reply_ptr[1];
  FIH_RET(FIH_SUCCESS);
}

static fih_int __attribute__ ((noinline)) vse_get_reply_status(const uint32_t *vse_reply_ptr, uint32_t *vse_reply_status)
{
  *vse_reply_status = vse_reply_ptr[2];
  FIH_RET(FIH_SUCCESS);
}

static fih_int __attribute__ ((noinline)) vse_get_data_length(const uint32_t *vse_reply_ptr, uint32_t *vse_data_length)
{
  *vse_data_length = vse_reply_ptr[4];
  FIH_RET(FIH_SUCCESS);
}

static fih_int __attribute__ ((noinline)) check_se_version(uint32_t se_version)
{
  fih_int fih_rc = FIH_FAILURE; // if fatal_error is glitched, return failure
  volatile int32_t version_diff = A_NEGATIVE_VALUE;
  #define FIH_INIT_STEP_VERFIY_SE_VERSION 3
  FIH_CFI_STEP_INIT(FIH_INIT_STEP_VERFIY_SE_VERSION);

  // Verify that the SE version newer than the oldest supported version.
  if (se_version < OLDEST_SUPPORTED_SE_FW_VERSION) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Double check that that se_version is higher than the initial version with
  // SRK support in case the previous if statement was glitched.
  // The following subtraction is based on the assumption that both minuend and
  // subtrahend are unsigned 32-bit values masked with 0x00FFFFFFU, ref
  // definition of SLI_VERSION_REMOVE_DIE_ID.
  version_diff = SLI_VERSION_REMOVE_DIE_ID(se_version) - OLDEST_SUPPORTED_SE_FW_VERSION;
  if (version_diff < 0) {
    goto exit; // if fatal_error is glitched
  }

  FIH_CFI_STEP_DECREMENT();

  // Additional check that the version_diff is different than the initial value.
  if (fih_eq(fih_int_encode(version_diff),
             fih_int_encode(A_NEGATIVE_VALUE))) {
    goto exit; // if fatal_error is glitched
  } else {
    fih_rc = FIH_SUCCESS;
    FIH_CFI_STEP_DECREMENT();
  }

  FIH_RET(fih_rc);

  exit:
  FIH_CFI_STEP_ERR_RESET();
  FIH_RET(FIH_FAILURE);
}
#endif // !SEMAILBOX_PRESENT && CRYPTOACC_PRESENT