/***************************************************************************//**
 * @file
 * @brief Analog Comparator ACMP peripheral API
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
#include "sl_hal_acmp.h"

#if defined(ACMP_PRESENT)
#include "stdint.h"
#include "sl_hal_system.h"
#include "sl_common.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************//**
 * @addtogroup acmp ACMP - Analog Comparator
 * @brief Analog comparator (ACMP) Peripheral API
 *
 * @details
 *  The Analog Comparator is used to compare voltage of two analog inputs
 *  with a digital output indicating which input voltage is higher. Inputs can
 *  either be one of the selectable internal references or from external pins.
 *  Response time and current consumption can be configured by
 *  altering the current supply to the comparator.
 *
 *  ACMP is available down to EM3 and is able to wake up the system when
 *  input signals pass a certain threshold. Use @ref sl_hal_acmp_enable_interrupts()
 *  to enable an edge interrupt to use this functionality.
 *
 * @note
 *  ACMP can also be used to compare two separate input pins.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_acmp_clear_interrupts(ACMP_TypeDef *acmp, uint32_t flags);
extern __INLINE void sl_hal_acmp_disable_interrupts(ACMP_TypeDef *acmp, uint32_t flags);
extern __INLINE void sl_hal_acmp_enable_interrupts(ACMP_TypeDef *acmp, uint32_t flags);
extern __INLINE uint32_t sl_hal_acmp_get_pending_interrupts(ACMP_TypeDef *acmp);
extern __INLINE uint32_t sl_hal_acmp_get_enabled_interrupts(ACMP_TypeDef *acmp);
extern __INLINE uint32_t sl_hal_acmp_get_enabled_pending_interrupts(ACMP_TypeDef *acmp);
extern __INLINE void sl_hal_acmp_set_interrupts(ACMP_TypeDef *acmp, uint32_t flags);
extern __INLINE sl_hal_acmp_input_t sl_hal_acmp_gpio_to_input(uint32_t port, uint32_t pin);
extern __INLINE void sl_hal_acmp_wait_sync(ACMP_TypeDef *acmp);
extern __INLINE void sl_hal_acmp_wait_ready(ACMP_TypeDef *acmp);
extern __INLINE uint32_t sl_hal_acmp_get_status(ACMP_TypeDef *acmp);

/***************************************************************************//**
 * @brief
 *   Initialize ACMP.
 ******************************************************************************/
void sl_hal_acmp_init(ACMP_TypeDef *acmp,
                      const sl_hal_acmp_init_t *init)
{
  /* Make sure the module exists on the selected chip. */
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  EFM_ASSERT(init->bias_prog
             <= (_ACMP_CFG_BIAS_MASK >> _ACMP_CFG_BIAS_SHIFT));

// PM-5507: enforce that biasProg is a functional value
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
  EFM_ASSERT(init->bias_prog >= PM5507_ACMP_CFG_BIAS_DEFAULT);
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
  // Allow customer to use BIASPROG in [2; 3]
  EFM_ASSERT(init->bias_prog >= 2);
#else
  // Allow customer to use BIASPROG in [0; 3]
  // but the implementation of the wait operation would be their responsibility
#endif

  // Make sure the ACMP is disabled since ACMP power source might be changed.
  sl_hal_acmp_disable(acmp);
  sl_hal_acmp_wait_ready(acmp);

  acmp->CFG = (acmp->CFG & ~(_ACMP_CFG_BIAS_MASK
#if defined(_ACMP_CFG_VCMRANGE_MASK)
                             | _ACMP_CFG_VCMRANGE_MASK
#else
                             | _ACMP_CFG_INPUTRANGE_MASK
#endif
#if defined(_ACMP_CFG_HYSTRISE_MASK)
                             | _ACMP_CFG_HYSTRISE_MASK
                             | _ACMP_CFG_HYSTFALL_MASK
#else
                             | _ACMP_CFG_HYST_MASK
#endif
                             | _ACMP_CFG_ACCURACY_MASK))
              | (init->bias_prog << _ACMP_CFG_BIAS_SHIFT)
#if defined(_ACMP_CFG_VCMRANGE_MASK)
              | (init->input_range << _ACMP_CFG_VCMRANGE_SHIFT)
#else
              | (init->input_range << _ACMP_CFG_INPUTRANGE_SHIFT)
#endif
#if defined(_ACMP_CFG_HYSTRISE_MASK)
              | (init->hysteresis_rise << _ACMP_CFG_HYSTRISE_SHIFT)
              | (init->hysteresis_fall << _ACMP_CFG_HYSTFALL_SHIFT)
#else
              | (init->hysteresis << _ACMP_CFG_HYST_SHIFT)
#endif
              | (init->accuracy << _ACMP_CFG_ACCURACY_SHIFT);

  acmp->CTRL = (acmp->CTRL & ~_ACMP_CTRL_NOTRDYVAL_SHIFT)
               | ((uint32_t)init->inactive_value << _ACMP_CTRL_NOTRDYVAL_SHIFT);
  sl_hal_acmp_enable(acmp);

  acmp->INPUTCTRL = (acmp->INPUTCTRL & ~_ACMP_INPUTCTRL_VREFDIV_MASK)
                    | (init->vref_div << _ACMP_INPUTCTRL_VREFDIV_SHIFT);

  sl_hal_acmp_disable(acmp);
  sl_hal_acmp_wait_ready(acmp);
}

/***************************************************************************//**
 * @brief
 *   Reset ACMP to the state it is after a hardware reset.
 ******************************************************************************/
void sl_hal_acmp_reset(ACMP_TypeDef *acmp)
{
  // Make sure the module exists on the selected chip
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));
#if defined(ACMP_SWRST_SWRST)
  acmp->SWRST_SET = ACMP_SWRST_SWRST;

#else
  acmp->IEN         = _ACMP_IEN_RESETVALUE;
  sl_hal_acmp_enable(acmp);
  acmp->INPUTCTRL   = _ACMP_INPUTCTRL_RESETVALUE;
  sl_hal_acmp_disable(acmp);
  sl_hal_acmp_wait_ready(acmp);
  acmp->CFG         = PM5507_ACMP_CFG_RESETVALUE;
  acmp->CTRL        = _ACMP_CTRL_RESETVALUE;
  acmp->IF_CLR      = _ACMP_IF_MASK;

#endif
}

/***************************************************************************//**
 * @brief
 *   Sets up GPIO output inversion from the ACMP.
 ******************************************************************************/
void sl_hal_acmp_setup_gpio_inversion(ACMP_TypeDef *acmp,
                                      bool invert)
{
  // Make sure the module exists on the selected chip
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  // Set GPIO inversion
  acmp->CTRL = (acmp->CTRL & ~_ACMP_CTRL_GPIOINV_MASK)
               | ((uint32_t)invert << _ACMP_CTRL_GPIOINV_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Set which channels are used in ACMP comparisons.
 ******************************************************************************/
void sl_hal_acmp_set_input(ACMP_TypeDef *acmp,
                           sl_hal_acmp_input_t negative_input,
                           sl_hal_acmp_input_t positive_input)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));
  // Make sure that positive_input and negative_input channel selectors are valid.
  EFM_ASSERT(negative_input <= _ACMP_INPUTCTRL_NEGSEL_PD15);
  EFM_ASSERT(positive_input <= _ACMP_INPUTCTRL_POSSEL_PD15);

  // Make sure that positive_input and negative_input channel selectors don't both use odd or even pins.
#if defined(_ACMP_INPUTCTRL_POSSEL_EXTPA)
  EFM_ASSERT(!((((positive_input >= _ACMP_INPUTCTRL_POSSEL_EXTPA)
                 && (positive_input <= _ACMP_INPUTCTRL_POSSEL_EXTPD))
                || (positive_input >= _ACMP_INPUTCTRL_POSSEL_PA0))
               && (negative_input >= _ACMP_INPUTCTRL_NEGSEL_PA0)
               && (positive_input % 2 == negative_input % 2)));

#else
  EFM_ASSERT(!((positive_input >= _ACMP_INPUTCTRL_POSSEL_PA0)
               && (negative_input >= _ACMP_INPUTCTRL_NEGSEL_PA0)
               && (positive_input % 2 == negative_input % 2)));

#endif

  // Make sure that the ACMP is enabled and not busy before changing INPUTCTRL.
  EFM_ASSERT(acmp->EN & ACMP_EN_EN);
  sl_hal_acmp_wait_sync(acmp);

  acmp->INPUTCTRL = (acmp->INPUTCTRL & ~(_ACMP_INPUTCTRL_POSSEL_MASK
                                         | _ACMP_INPUTCTRL_NEGSEL_MASK))
                    | (negative_input << _ACMP_INPUTCTRL_NEGSEL_SHIFT)
                    | (positive_input << _ACMP_INPUTCTRL_POSSEL_SHIFT);

  // Wait for sync of INPUTCTRL to complete.
  sl_hal_acmp_wait_sync(ACMP0);
}

/***************************************************************************//**
 * @brief
 *   Enable ACMP.
 ******************************************************************************/
void sl_hal_acmp_enable(ACMP_TypeDef *acmp)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

#if defined(_ACMP_EN_MASK)
  acmp->EN_SET = ACMP_EN_EN;
#else
  acmp->CTRL_SET = ACMP_CTRL_EN;
#endif
}

/***************************************************************************//**
 * @brief
 *   Disable ACMP.
 ******************************************************************************/
void sl_hal_acmp_disable(ACMP_TypeDef *acmp)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  sl_hal_acmp_wait_sync(acmp);
  acmp->EN_CLR = ACMP_EN_EN;
}

/** @} (end addtogroup ACMP) */
#endif /* defined( ACMP_PRESENT ) */
#ifdef __cplusplus
}
#endif
