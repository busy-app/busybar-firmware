/*******************************************************************************
 * @file  sl_si91x_crypto_thread.c
 * @brief SLI SI91X Crypto thread safety file
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <string.h>
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_status.h"
#include "sl_si91x_crypto_thread.h"

osSemaphoreId_t crypto_aes_mutex         = 0;
osSemaphoreId_t crypto_ccm_mutex         = 0;
osSemaphoreId_t crypto_sha_mutex         = 0;
osSemaphoreId_t crypto_gcm_mutex         = 0;
osSemaphoreId_t crypto_chachapoly_mutex  = 0;
osSemaphoreId_t crypto_attestation_mutex = 0;
osSemaphoreId_t crypto_ecdh_mutex        = 0;
osSemaphoreId_t crypto_ecdsa_mutex       = 0;
osSemaphoreId_t crypto_hmac_mutex        = 0;
osSemaphoreId_t crypto_trng_mutex        = 0;
osStatus_t mutex_result                  = osOK;

osSemaphoreId_t sl_si91x_crypto_threadsafety_init(osSemaphoreId_t *mutex)
{
  if (mutex == NULL) {

    osSemaphoreAttr_t sl_osSemaphoreAttr_t;

    //Set Semaphore flags attributes
    sl_osSemaphoreAttr_t.name      = "crypto_mutex";
    sl_osSemaphoreAttr_t.attr_bits = 0U;
    sl_osSemaphoreAttr_t.cb_mem    = NULL;
    sl_osSemaphoreAttr_t.cb_size   = 0U;

    mutex = osSemaphoreNew(1U, 1U, &sl_osSemaphoreAttr_t);

    osSemaphoreRelease(mutex);
  }
  return mutex;
}

osStatus_t sl_si91x_crypto_mutex_acquire(osSemaphoreId_t *mutex)
{
  mutex_result = osSemaphoreAcquire(mutex, osWaitForever);
  return mutex_result;
}

osStatus_t sl_si91x_crypto_mutex_release(osSemaphoreId_t *mutex)
{
  mutex_result = osSemaphoreRelease(mutex);
  return mutex_result;
}
