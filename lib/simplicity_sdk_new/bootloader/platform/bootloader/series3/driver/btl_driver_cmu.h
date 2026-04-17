/***************************************************************************//**
 * @file
 * @brief Gecko bootloader driver cmu utility functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BTL_DRIVER_CMU_H
#define BTL_DRIVER_CMU_H

#include "em_device.h"
#if defined(CMU_PRESENT)

#include <stdbool.h>
#include "sl_enum.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Enable register bit positions, for internal use. */
#define CMU_EN_BIT_POS             0U
/* Enable register ID's for internal use. */
#define CMU_NO_EN_REG               0
#define CMU_CLKEN0_EN_REG           1
#define CMU_CLKEN1_EN_REG           2
#define CMU_EN_REG_POS              5U
/** Clock points in CMU clock-tree. */
SL_ENUM_GENERIC(CMU_Clock_TypeDef, uint32_t) {
#if defined(EUSART_PRESENT) && EUSART_COUNT > 0
  cmuClock_EUSART0 = (CMU_CLKEN1_EN_REG << CMU_EN_REG_POS)
                     | (_CMU_CLKEN1_EUSART0_SHIFT << CMU_EN_BIT_POS),           /**< EUSART0 clock. */
#endif
#if defined(EUSART_PRESENT) && EUSART_COUNT > 1
  cmuClock_EUSART1 = (CMU_CLKEN1_EN_REG << CMU_EN_REG_POS)
                     | (_CMU_CLKEN1_EUSART1_SHIFT << CMU_EN_BIT_POS),           /**< EUSART1 clock. */
#endif
#if defined(EUSART_PRESENT) && EUSART_COUNT > 2
  cmuClock_EUSART2 = (CMU_CLKEN1_EN_REG << CMU_EN_REG_POS)
                     | (_CMU_CLKEN1_EUSART2_SHIFT << CMU_EN_BIT_POS),           /**< EUSART2 clock. */
#endif
};
#endif /* defined(CMU_PRESENT) */
#endif
