/***************************************************************************//**
 * @file
 * @brief LEDDRV Peripheral API
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

#include "sl_hal_leddrv.h"
#if defined(LEDDRV_COUNT) && (LEDDRV_COUNT > 0)

#include "sl_assert.h"
#include "sl_hal_gpio.h"

/***************************************************************************//**
 * @addtogroup leddrv LEDDRV
 * @brief LEDDRV Peripheral API
 * @details
 *   The LEDDRV module consists of these main parts:
 *   @li General LEDDRV configuration and enable control.
 *   @li Control functions that can be used when module is enabled.
 *   @li Interrupts handling.
 * @{
 ******************************************************************************/

/*******************************************************************************
 ***********************   UTILITIY FUNCTIONS PROTOTYPES   *********************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Get LEDDRV GPIO port.
 *
 * @param port Enum port (sl_hal_leddrv_port_select_t) value of LEDDRV.
 *
 * @return  GPIO port (sl_gpio_port_t).
 ******************************************************************************/
static uint32_t get_leddrv_gpio_port(sl_hal_leddrv_port_select_t port);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_leddrv_wait_ready(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_enable(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_disable(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_enable_dual_drive_mode(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_disable_dual_drive_mode(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_enable_peak_current_foldback(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_disable_peak_current_foldback(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_enable_direct_drive_mode(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_disable_direct_drive_mode(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_enable_ocflag_hardware_control(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_disable_ocflag_hardware_control(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_set_ton_forced_calibration(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_clear_ton_forced_calibration(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_clear_ocflag_channel0(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_clear_ocflag_channel1(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_reset(LEDDRV_TypeDef *leddrv);
extern __INLINE uint32_t sl_hal_leddrv_get_status(LEDDRV_TypeDef *leddrv);
extern __INLINE void sl_hal_leddrv_clear_interrupts(LEDDRV_TypeDef *leddrv,
                                                    uint32_t flags);
extern __INLINE void sl_hal_leddrv_set_interrupts(LEDDRV_TypeDef *leddrv,
                                                  uint32_t flags);
extern __INLINE void sl_hal_leddrv_disable_interrupts(LEDDRV_TypeDef *leddrv,
                                                      uint32_t flags);
extern __INLINE void sl_hal_leddrv_enable_interrupts(LEDDRV_TypeDef *leddrv,
                                                     uint32_t flags);
extern __INLINE uint32_t sl_hal_leddrv_get_pending_interrupts(LEDDRV_TypeDef *leddrv);
extern __INLINE uint32_t sl_hal_leddrv_get_enabled_interrupts(LEDDRV_TypeDef *leddrv);
extern __INLINE uint32_t sl_hal_leddrv_get_enabled_pending_interrupts(LEDDRV_TypeDef *leddrv);

/***************************************************************************//**
 * @brief
 *   Initialize LEDDRV.
 ******************************************************************************/
void sl_hal_leddrv_init(LEDDRV_TypeDef *leddrv,
                        const sl_hal_leddrv_init_t *init)
{
  sl_gpio_t temp;
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));
  // Init structure must be provided.
  EFM_ASSERT(init);

  sl_hal_leddrv_disable(leddrv);
  sl_hal_leddrv_wait_ready(leddrv);

  // Set config register.
  leddrv->CFG = ((uint32_t)init->time_on_max << _LEDDRV_CFG_TONMAXVAL_SHIFT)
                | ((uint32_t)init->time_off_max << _LEDDRV_CFG_TOFFMAXVAL_SHIFT)
                | ((uint32_t)init->ipeak_detect_ref_adjust << _LEDDRV_CFG_IPKDETREFADJ_SHIFT)
                | ((uint32_t)init->overcurrent_ref_adjust << _LEDDRV_CFG_IOCREFADJ_SHIFT)
                | ((uint32_t)init->fixed_ton_sel_lsb << _LEDDRV_CFG_FIXEDTONSEL_SHIFT);

  // Set the GPIO Port/Pin selection values
  leddrv->ABUSCTRL0 = ((uint32_t)init->vac_gpio_select.port << _LEDDRV_ABUSCTRL0_VACPORTSEL_SHIFT)
                      | ((uint32_t)init->vac_gpio_select.pin << _LEDDRV_ABUSCTRL0_VACPINSEL_SHIFT);
  leddrv->ABUSCTRL1 = ((uint32_t)init->current_sense0_gpio_select.port << _LEDDRV_ABUSCTRL1_ISNSPORTSEL0_SHIFT)
                      | ((uint32_t)init->current_sense0_gpio_select.pin << _LEDDRV_ABUSCTRL1_ISNSPINSEL0_SHIFT)
                      | ((uint32_t)init->drain_sense0_gpio_select.port << _LEDDRV_ABUSCTRL1_DSNSPORTSEL0_SHIFT)
                      | ((uint32_t)init->drain_sense0_gpio_select.pin << _LEDDRV_ABUSCTRL1_DSNSPINSEL0_SHIFT)
                      | ((uint32_t)init->current_sense1_gpio_select.port << _LEDDRV_ABUSCTRL1_ISNSPORTSEL1_SHIFT)
                      | ((uint32_t)init->current_sense1_gpio_select.pin << _LEDDRV_ABUSCTRL1_ISNSPINSEL1_SHIFT)
                      | ((uint32_t)init->drain_sense1_gpio_select.port << _LEDDRV_ABUSCTRL1_DSNSPORTSEL1_SHIFT)
                      | ((uint32_t)init->drain_sense1_gpio_select.pin << _LEDDRV_ABUSCTRL1_DSNSPINSEL1_SHIFT);

  // Set GPIO port/pin mode.
  if (init->vac_gpio_select.port != SL_HAL_LEDDRV_PORT_SELECT_NONE) {
    temp.port = (sl_gpio_port_t)get_leddrv_gpio_port(init->vac_gpio_select.port);
    temp.pin = init->vac_gpio_select.pin;
    sl_hal_gpio_set_pin_mode(&temp, SL_GPIO_MODE_DISABLED, false);
  }

  if (init->current_sense0_gpio_select.port != SL_HAL_LEDDRV_PORT_SELECT_NONE) {
    temp.port = (sl_gpio_port_t)get_leddrv_gpio_port(init->current_sense0_gpio_select.port);
    temp.pin = init->current_sense0_gpio_select.pin;
    sl_hal_gpio_set_pin_mode(&temp, SL_GPIO_MODE_DISABLED, false);
  }
  if (init->drain_sense0_gpio_select.port != SL_HAL_LEDDRV_PORT_SELECT_NONE) {
    temp.port = (sl_gpio_port_t)get_leddrv_gpio_port(init->drain_sense0_gpio_select.port);
    temp.pin = init->drain_sense0_gpio_select.pin;
    sl_hal_gpio_set_pin_mode(&temp, SL_GPIO_MODE_DISABLED, false);
  }
  if (init->current_sense1_gpio_select.port != SL_HAL_LEDDRV_PORT_SELECT_NONE) {
    temp.port = (sl_gpio_port_t)get_leddrv_gpio_port(init->current_sense1_gpio_select.port);
    temp.pin = init->current_sense1_gpio_select.pin;
    sl_hal_gpio_set_pin_mode(&temp, SL_GPIO_MODE_DISABLED, false);
  }
  if (init->drain_sense1_gpio_select.port != SL_HAL_LEDDRV_PORT_SELECT_NONE) {
    temp.port = (sl_gpio_port_t)get_leddrv_gpio_port(init->drain_sense1_gpio_select.port);
    temp.pin = init->drain_sense1_gpio_select.pin;
    sl_hal_gpio_set_pin_mode(&temp, SL_GPIO_MODE_DISABLED, false);
  }

  // Allocate GPIO port/pin for LEDDRV input.
  GPIO->CDBUSALLOC_SET = GPIO_CDBUSALLOC_CDEVEN0_LEDDRV;
  GPIO->CDBUSALLOC_SET = GPIO_CDBUSALLOC_CDEVEN1_LEDDRV;
  GPIO->CDBUSALLOC_SET = GPIO_CDBUSALLOC_CDODD0_LEDDRV;
  GPIO->CDBUSALLOC_SET = GPIO_CDBUSALLOC_CDODD1_LEDDRV;

  // Enable LEDDRV to set control register.
  sl_hal_leddrv_enable(leddrv);

  // Set control register.
  leddrv->CTRL = (init->dual_drive_mode_enable            ? LEDDRV_CTRL_ENADUALMODE : 0)
                 | (init->direct_drive_mode_enable        ? LEDDRV_CTRL_ENDIRDRV : 0)
                 | (init->peak_current_foldback_enable    ? LEDDRV_CTRL_IPKFOLDBACK : 0)
                 | (init->disable_ocflag_hardware_control ? LEDDRV_CTRL_HWOCFLAGDIS : 0)
                 | (init->enable_forced_calibration       ? LEDDRV_CTRL_ENFRC : 0);

  sl_hal_leddrv_disable(leddrv);
  sl_hal_leddrv_wait_ready(leddrv);

  // Calibration mode must be set after LEDDRV_CTRL_ENFRC
  leddrv->CFG_SET = ((uint32_t)init->ton_cal_mode << _LEDDRV_CFG_TONCALMODE_SHIFT);
}

static uint32_t get_leddrv_gpio_port(sl_hal_leddrv_port_select_t port)
{
  if (port == SL_HAL_LEDDRV_PORT_SELECT_PORT_C) {
    return (uint32_t)SL_GPIO_PORT_C;
  } else if (port == SL_HAL_LEDDRV_PORT_SELECT_PORT_D) {
    return (uint32_t)SL_GPIO_PORT_D;
  } else {
    // Invalid port
    EFM_ASSERT(false);
    return 0xFFFFFFFF;
  }
}

/** @} (end addtogroup leddrv) */
#endif /* defined(LEDDRV_COUNT) && (LEDDRV_COUNT > 0) */
