/***************************************************************************//**
 * @file
 * @brief Part of Silicon Labs's platform abstraction layer for SxSymCrypt.
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

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_code_classification.h"

// -----------------------------------------------------------------------------
// Constant Definitions

#define SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO (0)
#define SLI_SXSYMCRYPT_CRYPTOMASTER_LPWAES        (1)
#define SLI_SXSYMCRYPT_CRYPTOMASTER_N_INSTANCES   (2)

// -----------------------------------------------------------------------------
// Type Definitions

// The SxSymCrypt library passes structs of this type around in order to
// identify the right CryptoMaster instance to use for an operation. We will
// only store the CryptoMaster index in this struct, and instead have consuming
// functions look up the data (constant or variable) based on the index.
struct sx_regs {
  unsigned int instance_index;
  uint8_t * volatile base_address;
  bool volatile yield;
};

typedef struct {
  uint32_t FETCHADDR;   ///< Fetcher Address
  uint32_t PUSHADDR;    ///< Pusher Address
} sli_cryptomaster_state_t;
// -----------------------------------------------------------------------------
// Global variable Declarations

extern unsigned int requested_cryptomaster_index;
extern volatile sl_status_t sli_crypto_preempted_status;

// -----------------------------------------------------------------------------
// Function Declarations

/*
 * \brief
 *   Initialize lock objects used by sxsymcrypt
 */
sl_status_t sli_sxsymcrypt_init_locks(void);

/*
 * \brief
 *   Select which CryptoMaster instance should service the upcoming HostCrypto
 *   operation.
 *
 * \details
 *   This function will aquire and keep a lock on a engine-selection variable
 *   until sli_sxsymcrypt_unlock_cryptomaster_selection() has been called.
 *
 * \attention
 *   This function should always be called before a call to any SxSymCrypt
 *   functions following the pattern: sx\_*category*\_create\_*mode*().
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_sxsymcrypt_lock_cryptomaster_selection(unsigned int instance,
                                                       bool yield);

/*
 * \brief
 *   Release the lock on the engine-selection variable aquired in a previous
 *   call to sli_sxsymcrypt_lock_cryptomaster_selection().
 *
 * \attention
 *   This function should always be called after a call to any SxSymCrypt
 *   functions following the pattern: sx\_*category*\_create\_*mode*().
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_sxsymcrypt_unlock_cryptomaster_selection(void);

/*
 * \brief
 *   Save cryptomaster register state to RAM
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sli_crypto_lpwaes_save_state(sli_cryptomaster_state_t *state);

/*
 * \brief
 *   Restore cryptomaster register state from RAM
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sli_crypto_lpwaes_restore_state(sli_cryptomaster_state_t *state);

/*
 * \brief
 *   Release the cryptomaster instance associated with regs
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sli_cmdma_release_hw(struct sx_regs *regs);
