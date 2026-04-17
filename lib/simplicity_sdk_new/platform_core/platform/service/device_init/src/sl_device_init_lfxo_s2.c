/***************************************************************************//**
 * @file
 * @brief Device initialization for LFXO.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_device_init_lfxo.h"
#include "sl_device_init_lfxo_config.h"

#include "em_cmu.h"

// Fetch CTUNE value from USERDATA page as a manufacturing token
#define MFG_CTUNE_ADDR 0x0FE0009CUL
#define MFG_CTUNE_VAL  (*((uint8_t *) (MFG_CTUNE_ADDR)))

sl_status_t sl_device_init_lfxo(void)
{
  CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;

  lfxoInit.mode = SL_DEVICE_INIT_LFXO_MODE;
  lfxoInit.timeout = SL_DEVICE_INIT_LFXO_TIMEOUT;

  int ctune = -1;

#if defined(_DEVINFO_MODXOCAL_LFXOCAPTUNE_MASK)
  // Use LFXO tuning value from DEVINFO if available (PCB modules)
  if ((DEVINFO->MODULEINFO & _DEVINFO_MODULEINFO_LFXOCALVAL_MASK) == _DEVINFO_MODULEINFO_LFXOCALVAL_VALID) {
    ctune = DEVINFO->MODXOCAL & _DEVINFO_MODXOCAL_LFXOCAPTUNE_MASK;
  }
#endif

  // Use LFXO tuning value from MFG token in UD page if not already set
  if ((ctune == -1) && (MFG_CTUNE_VAL != 0xFF)) {
    ctune = MFG_CTUNE_VAL;
  }

  // Use LFXO tuning value from configuration header as fallback
  if (ctune == -1) {
    ctune = SL_DEVICE_INIT_LFXO_CTUNE;
  }

  lfxoInit.capTune = ctune;
  CMU_LFXOInit(&lfxoInit);
  CMU_LFXOPrecisionSet(SL_DEVICE_INIT_LFXO_PRECISION);

  return SL_STATUS_OK;
}
