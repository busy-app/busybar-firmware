/***************************************************************************//**
 * @file
 * @brief LEDDRV peripheral API
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

#ifndef SL_HAL_LEDDRV_H
#define SL_HAL_LEDDRV_H

#include "em_device.h"
#if defined(LEDDRV_COUNT) && (LEDDRV_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_assert.h"
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup leddrv LEDDRV - LED Pre-Driver
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   DEFINES   **********************************
 ******************************************************************************/

/// Check if LEDDRV instance is valid.
#define SL_HAL_LEDDRV_REF_VALID(leddrv_ref)    (LEDDRV_NUM(leddrv_ref) != -1)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Used for Max On time and max Off time of LEDDRV.
SL_ENUM(sl_hal_leddrv_time_max_t) {
  SL_HAL_LEDDRV_TIME_MAX_DISABLE = 0,  /**< Max On/Off time is disabled. */
  SL_HAL_LEDDRV_TIME_MAX_6P4US,  /**< Max On/Off time is 6.4 us. */
  SL_HAL_LEDDRV_TIME_MAX_12P8US, /**< Max On/Off time is 12.8 us. */
  SL_HAL_LEDDRV_TIME_MAX_19P2US, /**< Max On/Off time is 19.2 us. */
  SL_HAL_LEDDRV_TIME_MAX_25P6US, /**< Max On/Off time is 25.6 us. */
  SL_HAL_LEDDRV_TIME_MAX_32P0US, /**< Max On/Off time is 32.0 us. */
  SL_HAL_LEDDRV_TIME_MAX_38P4US, /**< Max On/Off time is 38.4 us. */
  SL_HAL_LEDDRV_TIME_MAX_44P8US, /**< Max On/Off time is 44.8 us. */
};

/// Time On calibration channel mode.
SL_ENUM(sl_hal_leddrv_ton_cal_mode_t) {
  SL_HAL_LEDDRV_TON_CAL_MODE_CURRENT_CHANNEL_ONLY = 0, /**< Current channel only. */
  SL_HAL_LEDDRV_TON_CAL_MODE_BOTH_CHANNELS,        /**< Both channels. */
  SL_HAL_LEDDRV_TON_CAL_MODE_ON_FORCE,             /**< Calibration On force. */
  SL_HAL_LEDDRV_TON_CAL_MODE_FIXED,             /**< Calibration is Fixed. */
};

/// Peak current detect reference adjust value.
SL_ENUM(sl_hal_leddrv_ipeak_detect_ref_adjust_t) {
  SL_HAL_LEDDRV_IPEAK_DETECT_REF_ADJUST_200MV = 0, /**< ipeak detect reference adjust 200 mV. */
  SL_HAL_LEDDRV_IPEAK_DETECT_REF_ADJUST_300MV, /**< ipeak detect reference adjust 300 mV. */
  SL_HAL_LEDDRV_IPEAK_DETECT_REF_ADJUST_400MV, /**< ipeak detect reference adjust 400 mV. */
  SL_HAL_LEDDRV_IPEAK_DETECT_REF_ADJUST_500MV  /**< ipeak detect reference adjust 500 mV. */
};

/// Over current reference adjust.
SL_ENUM(sl_hal_leddrv_overcurrent_ref_adjust_t) {
  SL_HAL_LEDDRV_IPEAK_OVERCURRENT_REF_ADJUST_400MV = 0, /**< Overcurrent reference adjust 400 mV. */
  SL_HAL_LEDDRV_IPEAK_OVERCURRENT_REF_ADJUST_600MV, /**< Overcurrent reference adjust 600 mV. */
  SL_HAL_LEDDRV_IPEAK_OVERCURRENT_REF_ADJUST_800MV, /**< Overcurrent reference adjust 800 mV. */
  SL_HAL_LEDDRV_IPEAK_OVERCURRENT_REF_ADJUST_1000MV /**< Overcurrent reference adjust 1000 mV. */
};

/// Fixed Ton Select LSBs.
SL_ENUM(sl_hal_leddrv_fixed_ton_sel_lsb_t) {
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_0MV = 0, /**< LSB is 0 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_1MV, /**< LSB is 1 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_2MV, /**< LSB is 2 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_3MV, /**< LSB is 3 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_4MV, /**< LSB is 4 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_5MV, /**< LSB is 5 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_6MV, /**< LSB is 6 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_7MV, /**< LSB is 7 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_8MV, /**< LSB is 8 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_9MV, /**< LSB is 9 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_10MV, /**< LSB is 10 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_11MV, /**< LSB is 11 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_12MV, /**< LSB is 12 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_13MV, /**< LSB is 13 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_14MV, /**< LSB is 14 mV. */
  SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_15MV, /**< LSB is 15 mV. */
};

/// Port selection for LEDDRV gpio selection (only ports C/D allowed).
SL_ENUM(sl_hal_leddrv_port_select_t) {
  SL_HAL_LEDDRV_PORT_SELECT_NONE = 0,   /**< Select no Port. */
  SL_HAL_LEDDRV_PORT_SELECT_PORT_C, /**< Select Port C. */
  SL_HAL_LEDDRV_PORT_SELECT_PORT_D  /**< Select Port D. */
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// Port/Pin input of LEDDRV (Allowable ports are C and D only).
typedef struct {
  /// Port selection.
  sl_hal_leddrv_port_select_t port;

  /// Pin selection.
  uint8_t pin;
} sl_hal_leddrv_gpio_select_t;

/// LEDDRV initialization structure.
typedef struct {
  /// Enable dual drive mode
  bool dual_drive_mode_enable;

  /// Enable direct drive mode.
  bool direct_drive_mode_enable;

  /// Enable peak current foldback.
  bool peak_current_foldback_enable;

  /// Disable over current flag hardware control.
  bool disable_ocflag_hardware_control;

  /// Force enable; Override for internal enable that is gated on AC peak detect.
  bool enable_forced_calibration;

  /// Time On max.
  sl_hal_leddrv_time_max_t  time_on_max;

  /// Time Off max.
  sl_hal_leddrv_time_max_t time_off_max;

  /// Peak current detect reference adjust value.
  sl_hal_leddrv_ipeak_detect_ref_adjust_t ipeak_detect_ref_adjust;

  /// Over current reference adjust.
  sl_hal_leddrv_overcurrent_ref_adjust_t overcurrent_ref_adjust;

  /// Time On calibration channel mode.
  sl_hal_leddrv_ton_cal_mode_t ton_cal_mode;

  /// Fixed Ton Select LSBs.
  sl_hal_leddrv_fixed_ton_sel_lsb_t fixed_ton_sel_lsb;

  /// Vac gpio select.
  sl_hal_leddrv_gpio_select_t vac_gpio_select;

  /// Current sense 0 gpio select.
  sl_hal_leddrv_gpio_select_t current_sense0_gpio_select;

  /// Drain sense 0 gpio select.
  sl_hal_leddrv_gpio_select_t drain_sense0_gpio_select;

  /// Current sense 1 gpio select.
  sl_hal_leddrv_gpio_select_t current_sense1_gpio_select;

  /// Drain sense 1 gpio select.
  sl_hal_leddrv_gpio_select_t drain_sense1_gpio_select;
} sl_hal_leddrv_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_leddrv_init_t sl_hal_leddrv_config_t;
/** @endcond */

/// Default configuration for LEDDRV port/pin structure.
#define SL_HAL_LEDDRV_GPIO_SELECT_DEFAULT                     \
  {                                                           \
    _LEDDRV_ABUSCTRL0_VACPORTSEL_DEFAULT, /* Default port. */ \
    _LEDDRV_ABUSCTRL0_VACPINSEL_DEFAULT  /* Default pin. */   \
  }

/// Default configuration for LEDDRV initialization structure.
#define SL_HAL_LEDDRV_INIT_DEFAULT                                                                       \
  {                                                                                                      \
    _LEDDRV_CTRL_ENADUALMODE_DEFAULT, /* Disable dual drive mode. */                                     \
    _LEDDRV_CTRL_ENDIRDRV_DEFAULT,   /* Disable direct drive mode. */                                    \
    _LEDDRV_CTRL_IPKFOLDBACK_DEFAULT, /* Disable peak current foldback. */                               \
    _LEDDRV_CTRL_HWOCFLAGDIS_DEFAULT, /* Enable over current flag hardware control is set. */            \
    _LEDDRV_CTRL_ENFRC_DEFAULT, /* Force enable is not set; Override for internal enable. */             \
    _LEDDRV_CFG_TONMAXVAL_DEFAULT, /* Time On max default value is 44p8us. */                            \
    _LEDDRV_CFG_TOFFMAXVAL_DEFAULT, /* Time Off max default value. is 44p8us */                          \
    _LEDDRV_CFG_IPKDETREFADJ_DEFAULT, /* Default peak current detect reference adjust value is 300mV. */ \
    _LEDDRV_CFG_IOCREFADJ_DEFAULT, /* Default over current reference adjust value is 1000mV. */          \
    _LEDDRV_CFG_TONCALMODE_DEFAULT, /* Default time On calibration channel mode is both channels . */    \
    _LEDDRV_CFG_FIXEDTONSEL_DEFAULT, /* Default fixed Ton select LSB is 0mV. */                          \
    SL_HAL_LEDDRV_GPIO_SELECT_DEFAULT, /* Default Vac gpio select. */                                    \
    SL_HAL_LEDDRV_GPIO_SELECT_DEFAULT, /* Default Current sense 0 gpio select. */                        \
    SL_HAL_LEDDRV_GPIO_SELECT_DEFAULT, /* Default Drain sense 0 gpio select. */                          \
    SL_HAL_LEDDRV_GPIO_SELECT_DEFAULT, /* Default Current sense 1 gpio select. */                        \
    SL_HAL_LEDDRV_GPIO_SELECT_DEFAULT /* Default Drain sense 1 gpio select. */                           \
  }

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro name used for backward compatibility purposes.
#define SL_HAL_LEDDRV_CONFIG_DEFAULT SL_HAL_LEDDRV_INIT_DEFAULT
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize LEDDRV. This function will allocate GPIO CD bus to LEDDRV. Make
 *   sure to set port/pin to C/D ports and pins in the init structure.
 *
 * @param[in] leddrv
 *   A pointer to the LEDDRV peripheral register block.
 *
 * @param[in] init
 *   A pointer to the LEDDRV initialization structure.
 ******************************************************************************/
void sl_hal_leddrv_init(LEDDRV_TypeDef *leddrv,
                        const sl_hal_leddrv_init_t *init);

/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_wait_ready(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  while (leddrv->SWRST & _LEDDRV_SWRST_RESETTING_MASK) {
    // Wait for reset to complete.
  }

  while (leddrv->EN & _LEDDRV_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Enable LEDDRV.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_enable(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Enable peripheral.
  leddrv->EN_SET = LEDDRV_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable LEDDRV.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_disable(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  if ((leddrv->EN & LEDDRV_EN_EN) == 0) {
    return;
  }
  // Disable LEDDRV
  leddrv->EN_CLR = LEDDRV_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Enable LEDDRV dual drive mode.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_enable_dual_drive_mode(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Enable dual drive mode.
  leddrv->CTRL_SET = LEDDRV_CTRL_ENADUALMODE;
}

/***************************************************************************//**
 * @brief
 *   Disable LEDDRV dual drive mode.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_disable_dual_drive_mode(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Disable dual drive mode.
  leddrv->CTRL_CLR = LEDDRV_CTRL_ENADUALMODE;
}

/***************************************************************************//**
 * @brief
 *   Enable LEDDRV peak current foldback mode.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_enable_peak_current_foldback(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Enable peak current foldback.
  leddrv->CTRL_SET = LEDDRV_CTRL_IPKFOLDBACK;
}

/***************************************************************************//**
 * @brief
 *   Disable LEDDRV peak current foldback mode.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_disable_peak_current_foldback(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Disable peak current foldback.
  leddrv->CTRL_CLR = LEDDRV_CTRL_IPKFOLDBACK;
}

/***************************************************************************//**
 * @brief
 *   Enable LEDDRV direct drive mode.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_enable_direct_drive_mode(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Enable direct drive mode.
  leddrv->CTRL_SET = LEDDRV_CTRL_ENDIRDRV;
}

/***************************************************************************//**
 * @brief
 *   Disable LEDDRV direct drive mode.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_disable_direct_drive_mode(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Disable direct drive mode.
  leddrv->CTRL_CLR = LEDDRV_CTRL_ENDIRDRV;
}

/***************************************************************************//**
 * @brief
 *   Enable LEDDRV Hardware overcurrent control.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_enable_ocflag_hardware_control(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Hardware overcurrent control.
  leddrv->CTRL_CLR = LEDDRV_CTRL_HWOCFLAGDIS;
}

/***************************************************************************//**
 * @brief
 *   Disable LEDDRV Hardware overcurrent control.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_disable_ocflag_hardware_control(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Disable Hardware overcurrent control.
  leddrv->CTRL_SET = LEDDRV_CTRL_HWOCFLAGDIS;
}

/***************************************************************************//**
 * @brief
 *   Set LEDDRV Ton calibration trigger.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_set_ton_forced_calibration(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Set Ton calibration trigger.
  leddrv->CTRL_SET = LEDDRV_CTRL_TONFRCCAL;
}

/***************************************************************************//**
 * @brief
 *   Clear LEDDRV Ton calibration trigger.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_clear_ton_forced_calibration(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Clear Ton calibration trigger.
  leddrv->CTRL_CLR = LEDDRV_CTRL_TONFRCCAL;
}

/***************************************************************************//**
 * @brief
 *   Clear LEDDRV Overcurrent Flag on channel 0.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_clear_ocflag_channel0(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Clear Overcurrent Flag on channel 0.
  leddrv->CTRL_SET = LEDDRV_CTRL_SWCLROCFLAG0;
}

/***************************************************************************//**
 * @brief
 *   Clear LEDDRV Overcurrent Flag on channel 1.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_clear_ocflag_channel1(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Make sure module is enabled
  EFM_ASSERT(leddrv->EN & LEDDRV_EN_EN);

  // Clear Overcurrent Flag on channel 1.
  leddrv->CTRL_SET = LEDDRV_CTRL_SWCLROCFLAG1;
}

/***************************************************************************//**
 * @brief
 *   Reset the LEDDRV
 *
 * @param[in] leddrv
 *   A leddrv peripheral module.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_reset(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  leddrv->SWRST_SET = LEDDRV_SWRST_SWRST;
}

/***************************************************************************//**
 * @brief
 *   Gets LEDDRV STATUS register value.
 *
 * @param[in] leddrv
 *   A LEDDRV peripheral module.
 *
 * @return
 *   Current STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_leddrv_get_status(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  return leddrv->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending LEDDRV interrupts.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 *
 * @param[in] flags
 *   Pending LEDDRV interrupt source(s) to clear. Use one or more valid
 *   interrupt flags for the LEDDRV module (LEDDRV_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_clear_interrupts(LEDDRV_TypeDef *leddrv,
                                             uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  leddrv->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending LEDDRV interrupts from SW.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 *
 * @param[in] flags
 *   LEDDRV interrupt source(s) to set to pending. Use one or more valid
 *   interrupt flags for the LEDDRV module (LEDDRV_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_set_interrupts(LEDDRV_TypeDef *leddrv,
                                           uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  leddrv->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more LEDDRV interrupts.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 *
 * @param[in] flags
 *   LEDDRV interrupt source(s) to disable. Use one or more valid
 *   interrupt flags for the LEDDRV module (LEDDRV_IEN_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_disable_interrupts(LEDDRV_TypeDef *leddrv,
                                               uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  leddrv->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more LEDDRV interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt.  To ignore a pending interrupt, consider using
 *   sl_hal_leddrv_clear_interrupts() prior to enabling the interrupt.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 *
 * @param[in] flags
 *   LEDDRV interrupt source(s) to enable. Use one or more valid
 *   interrupt flags for the LEDDRV module (LEDDRV_IEN_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_leddrv_enable_interrupts(LEDDRV_TypeDef *leddrv,
                                              uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  leddrv->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending LEDDRV interrupt flags.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 *
 * @return
 *   LEDDRV interrupt source(s) pending. Returns one or more valid
 *   interrupt flags for the LEDDRV module (LEDDRV_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_leddrv_get_pending_interrupts(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  return leddrv->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled LEDDRV interrupt flags.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 *
 * @return
 *   LEDDRV interrupt source(s) enabled. Returns one or more valid
 *   enabled interrupt sources in LEDDRVx_IEN_nnn.
 ******************************************************************************/
__INLINE uint32_t sl_hal_leddrv_get_enabled_interrupts(LEDDRV_TypeDef *leddrv)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  return leddrv->IEN;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending LEDDRV interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] leddrv
 *   Pointer to the LEDDRV peripheral register block.
 *
 * @note
 *   Interrupt flags are not cleared by this function.
 *
 * @return
 *   Pending and enabled LEDDRV interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in LEDDRVx_IEN_nnn
 *     register (LEDDRVx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the LEDDRV module
 *     (LEDDRVx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_leddrv_get_enabled_pending_interrupts(LEDDRV_TypeDef *leddrv)
{
  uint32_t ien;

  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LEDDRV_REF_VALID(leddrv));

  // Store LEDDRV->IEN in temporary variable in order to define explicit order.
  // of volatile accesses.
  ien = leddrv->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return leddrv->IF & ien;
}

/** @} (end addtogroup leddrv) */
#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup leddrv LEDDRV - LED Pre-Driver
 * @{
 *
 * @n @section leddrv_example Example
 *  This example shows how to configure and use the LEDDRV peripheral for driving
 *  LED lights using both single and dual drive modes.
 *
 * @code
 * #include "sl_hal_leddrv.h"
 * #include "sl_clock_manager.h"
 *
 * // Example function showing how to initialize and use the LEDDRV for LED control
 * void leddrv_example(void)
 * {
 *   // Enable clock to the LEDDRV peripheral
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LEDDRV0);
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *
 *   // Configure LEDDRV peripheral
 *   sl_hal_leddrv_init_t init = {
 *     .dual_drive_mode_enable = true,             // Enable dual drive mode
 *     .direct_drive_mode_enable = false,          // Disable direct drive mode
 *     .peak_current_foldback_enable = true,       // Enable peak current foldback
 *     .disable_ocflag_hardware_control = false,   // Enable overcurrent flag hardware control
 *     .enable_forced_calibration = false,         // Don't force enable (use internal enable)
 *     .time_on_max = SL_HAL_LEDDRV_TIME_MAX_12P8US,  // Set max on time to 12.8us
 *     .time_off_max = SL_HAL_LEDDRV_TIME_MAX_38P4US, // Set max off time to 38.4us
 *     .ipeak_detect_ref_adjust = SL_HAL_LEDDRV_IPEAK_DETECT_REF_ADJUST_300MV,
 *     .overcurrent_ref_adjust = SL_HAL_LEDDRV_IPEAK_OVERCURRENT_REF_ADJUST_800MV,
 *     .ton_cal_mode = SL_HAL_LEDDRV_TON_CAL_MODE_BOTH_CHANNELS,
 *     .fixed_ton_sel_lsb = SL_HAL_LEDDRV_FIXED_TON_SEL_LSB_5MV,
 *     // Configure GPIO pins for LEDDRV - only Port C or D can be used
 *     .vac_gpio_select = { SL_HAL_LEDDRV_PORT_SELECT_PORT_C, 0 },
 *     .current_sense0_gpio_select = { SL_HAL_LEDDRV_PORT_SELECT_PORT_C, 1 },
 *     .drain_sense0_gpio_select = { SL_HAL_LEDDRV_PORT_SELECT_PORT_C, 2 },
 *     .current_sense1_gpio_select = { SL_HAL_LEDDRV_PORT_SELECT_PORT_C, 3 },
 *     .drain_sense1_gpio_select = { SL_HAL_LEDDRV_PORT_SELECT_PORT_C, 4 }
 *   };
 *
 *   // Initialize the LEDDRV with the configuration
 *   sl_hal_leddrv_init(LEDDRV0, &init);
 *
 *   // Enable the LEDDRV peripheral
 *   sl_hal_leddrv_enable(LEDDRV0);
 *
 *   // Configure interrupts if needed
 *   sl_hal_leddrv_clear_interrupts(LEDDRV0, _LEDDRV_IEN_MASK);
 *   sl_hal_leddrv_enable_interrupts(LEDDRV0, LEDDRV_IEN_OCDET0);  // Enable overcurrent detection interrupt
 *
 *   // Wait for a while to let the LEDDRV operate
 *   // In a real application, this would be replaced with application code
 *   // or you would respond to interrupts
 *   for (volatile int i = 0; i < 1000; i++) {}
 *
 *   // Check for any overcurrent conditions
 *   uint32_t status = sl_hal_leddrv_get_status(LEDDRV0);
 *   if (status & LEDDRV_STATUS_OCFLAG0) {
 *     // Handle overcurrent condition on channel 0
 *     sl_hal_leddrv_clear_ocflag_channel0(LEDDRV0);
 *   }
 *
 *   // Clean up and disable LEDDRV when done
 *   sl_hal_leddrv_disable_interrupts(LEDDRV0, _LEDDRV_IEN_MASK);
 *   sl_hal_leddrv_disable(LEDDRV0);
 *   sl_hal_leddrv_wait_ready(LEDDRV0);
 *
 *   // Disable clocks to save power
 *   sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_LEDDRV0);
 *   sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_GPIO);
 * }
 *
 * @endcode
 *
 * @} (end addtogroup leddrv)
 ******************************************************************************/

#endif /* defined(LEDDRV_COUNT) && (LEDDRV_COUNT > 0) */
#endif /* SL_HAL_LEDDRV_H */
