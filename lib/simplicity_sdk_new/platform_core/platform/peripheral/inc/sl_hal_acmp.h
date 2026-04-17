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

#ifndef SL_HAL_ACMP_H
#define SL_HAL_ACMP_H

#include "em_device.h"

#if defined(ACMP_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_enum.h"
#include "sl_assert.h"

/***************************************************************************//**
 * @addtogroup acmp ACMP - Analog Comparator
 * @brief Analog Comparator peripheral API
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/// Validation of ACMP register block pointer reference for assert statements.
#define SL_HAL_ACMP_REF_VALID(ref)    (ACMP_NUM(ref) != -1)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Hysteresis level.
SL_ENUM(sl_hal_acmp_hysteresis_t) {
#if defined(_ACMP_CFG_HYSTRISE_MASK)
  SL_HAL_ACMP_HYSTERESIS_DISABLED = _ACMP_CFG_HYSTRISE_DISABLED,  ///< Hysteresis disabled for rising edge
  SL_HAL_ACMP_HYSTERESIS_10MV = _ACMP_CFG_HYSTRISE_HYST10POS,     ///< Hysteresis 10mV for rising edge
  SL_HAL_ACMP_HYSTERESIS_20MV = _ACMP_CFG_HYSTRISE_HYST20POS,     ///< Hysteresis 20mV for rising edge
  SL_HAL_ACMP_HYSTERESIS_30MV = _ACMP_CFG_HYSTRISE_HYST30POS,     ///< Hysteresis 30mV for rising edge
#else
  SL_HAL_ACMP_HYSTERESIS_DISABLED = _ACMP_CFG_HYST_DISABLED,      ///< Mode DISABLED for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_SYM_10MV = _ACMP_CFG_HYST_SYM10MV,       ///< Mode HYST10SYM for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_SYM_20MV = _ACMP_CFG_HYST_SYM20MV,       ///< Mode HYST20SYM for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_SYM_30MV = _ACMP_CFG_HYST_SYM30MV,       ///< Mode HYST30SYM for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_POS_10MV = _ACMP_CFG_HYST_POS10MV,       ///< Mode HYST10POS for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_POS_20MV = _ACMP_CFG_HYST_POS20MV,       ///< Mode HYST20POS for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_POS_30MV = _ACMP_CFG_HYST_POS30MV,       ///< Mode HYST30POS for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_NEG_10MV = _ACMP_CFG_HYST_NEG10MV,       ///< Mode HYST10NEG for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_NEG_20MV = _ACMP_CFG_HYST_NEG20MV,       ///< Mode HYST20NEG for ACMP_CFG
  SL_HAL_ACMP_HYSTERESIS_NEG_30MV = _ACMP_CFG_HYST_NEG30MV,       ///< Mode HYST30NEG for ACMP_CFG
#endif
};

/// vcm/input range.
SL_ENUM(sl_hal_acmp_input_range_t) {
#if defined(_ACMP_CFG_VCMRANGE_MASK)
  SL_HAL_ACMP_INPUT_RANGE_FULL = _ACMP_CFG_VCMRANGE_FULL,           ///< Input can be from 0 to VDD.
  SL_HAL_ACMP_INPUT_RANGE_HIGH = _ACMP_CFG_VCMRANGE_HIGH,           ///< Common mode voltage > 3V .
#else
  SL_HAL_ACMP_INPUT_RANGE_FULL = _ACMP_CFG_INPUTRANGE_FULL,         ///< Input can be from 0 to VDD.
  SL_HAL_ACMP_INPUT_RANGE_REDUCED = _ACMP_CFG_INPUTRANGE_REDUCED,   ///< Input can be from 0 to VDD-0.7 V.
#endif
};

/// Accuracy mode
SL_ENUM(sl_hal_acmp_accuracy_t) {
  SL_HAL_ACMP_ACCURACY_LOW = _ACMP_CFG_ACCURACY_LOW,   ///< Low-accuracy mode which consumes less current.
  SL_HAL_ACMP_ACCURACY_HIGH = _ACMP_CFG_ACCURACY_HIGH  ///< High-accuracy mode which consumes more current.
};

/// Input Selection.
SL_ENUM(sl_hal_acmp_input_t) {
  SL_HAL_ACMP_INPUT_VSS            = _ACMP_INPUTCTRL_POSSEL_VSS,            ///< Select VSS.
  SL_HAL_ACMP_INPUT_VREFDIVAVDD    = _ACMP_INPUTCTRL_POSSEL_VREFDIVAVDD,    ///< Select Divided AVDD.
  SL_HAL_ACMP_INPUT_VREFDIVAVDDLP  = _ACMP_INPUTCTRL_POSSEL_VREFDIVAVDDLP,  ///< Select Low-Power Divided AVDD.
  SL_HAL_ACMP_INPUT_VREFDIV1V25    = _ACMP_INPUTCTRL_POSSEL_VREFDIV1V25,    ///< Select Divided 1V25 reference.
  SL_HAL_ACMP_INPUT_VREFDIV1V25LP  = _ACMP_INPUTCTRL_POSSEL_VREFDIV1V25LP,  ///< Select Low-power Divided 1V25 reference.
  SL_HAL_ACMP_INPUT_VREFDIV2V5     = _ACMP_INPUTCTRL_POSSEL_VREFDIV2V5,     ///< Select Divided 2V5 reference.
  SL_HAL_ACMP_INPUT_VREFDIV2V5LP   = _ACMP_INPUTCTRL_POSSEL_VREFDIV2V5LP,   ///< Select Low-power Divided 2V5 reference.
  SL_HAL_ACMP_INPUT_VSENSE01DIV4   = _ACMP_INPUTCTRL_POSSEL_VSENSE01DIV4,   ///< Select VSENSE0 divided by 4.
  SL_HAL_ACMP_INPUT_VSENSE01DIV4LP = _ACMP_INPUTCTRL_POSSEL_VSENSE01DIV4LP, ///< Select Low-power VSENSE0 divided by 4.
  SL_HAL_ACMP_INPUT_VSENSE11DIV4   = _ACMP_INPUTCTRL_POSSEL_VSENSE11DIV4,   ///< Select VSENSE1 divided by 4.
  SL_HAL_ACMP_INPUT_VSENSE11DIV4LP = _ACMP_INPUTCTRL_POSSEL_VSENSE11DIV4LP, ///< Select Low-power VSENSE1 divided by 4.
#if defined(_ACMP_INPUTCTRL_POSSEL_VSENSE21DIV4)
  SL_HAL_ACMP_INPUT_VSENSE21DIV4   = _ACMP_INPUTCTRL_POSSEL_VSENSE21DIV4,   ///< Select VSENSE2 divided by 4.
  SL_HAL_ACMP_INPUT_VSENSE21DIV4LP = _ACMP_INPUTCTRL_POSSEL_VSENSE21DIV4LP, ///< Select Low-power VSENSE2 divided by 4.
  SL_HAL_ACMP_INPUT_VSENSE31DIV4   = _ACMP_INPUTCTRL_POSSEL_VSENSE31DIV4,   ///< Select VSENSE3 divided by 4.
  SL_HAL_ACMP_INPUT_VSENSE31DIV4LP = _ACMP_INPUTCTRL_POSSEL_VSENSE31DIV4LP, ///< Select Low-power VSENSE3 divided by 4.
#endif
#if defined(_ACMP_INPUTCTRL_POSSEL_VDACOUT0)
  SL_HAL_ACMP_INPUT_VDACOUT0       = _ACMP_INPUTCTRL_POSSEL_VDACOUT0, ///< Select VDAC0 channel 0 output.
  SL_HAL_ACMP_INPUT_VDACOUT1       = _ACMP_INPUTCTRL_POSSEL_VDACOUT1, ///< Select VDAC0 channel 1 output.
#endif
#if defined(_ACMP_INPUTCTRL_POSSEL_VDAC0OUT1)
  SL_HAL_ACMP_INPUT_VDAC0OUT1       = _ACMP_INPUTCTRL_POSSEL_VDAC0OUT1, ///< Select VDAC0 channel 1 output.
  SL_HAL_ACMP_INPUT_VDAC1OUT1       = _ACMP_INPUTCTRL_POSSEL_VDAC1OUT1, ///< Select VDAC1 channel 1 output.
#endif
#if defined(_ACMP_INPUTCTRL_POSSEL_EXTPA)
  SL_HAL_ACMP_INPUT_EXTPA          = _ACMP_INPUTCTRL_POSSEL_EXTPA,  ///< Select external interface, base is PA0.
  SL_HAL_ACMP_INPUT_EXTPB          = _ACMP_INPUTCTRL_POSSEL_EXTPB,  ///< Select external interface, base is PB0.
  SL_HAL_ACMP_INPUT_EXTPC          = _ACMP_INPUTCTRL_POSSEL_EXTPC,  ///< Select external interface, base is PC0.
  SL_HAL_ACMP_INPUT_EXTPD          = _ACMP_INPUTCTRL_POSSEL_EXTPD,  ///< Select external interface, base is PD0.
#endif
  SL_HAL_ACMP_INPUT_PA0            = _ACMP_INPUTCTRL_POSSEL_PA0,  ///< Select Port A Pin0.
  SL_HAL_ACMP_INPUT_PA1            = _ACMP_INPUTCTRL_POSSEL_PA1,  ///< Select Port A Pin1.
  SL_HAL_ACMP_INPUT_PA2            = _ACMP_INPUTCTRL_POSSEL_PA2,  ///< Select Port A Pin2.
  SL_HAL_ACMP_INPUT_PA3            = _ACMP_INPUTCTRL_POSSEL_PA3,  ///< Select Port A Pin3.
  SL_HAL_ACMP_INPUT_PA4            = _ACMP_INPUTCTRL_POSSEL_PA4,  ///< Select Port A Pin4.
  SL_HAL_ACMP_INPUT_PA5            = _ACMP_INPUTCTRL_POSSEL_PA5,  ///< Select Port A Pin5.
  SL_HAL_ACMP_INPUT_PA6            = _ACMP_INPUTCTRL_POSSEL_PA6,  ///< Select Port A Pin6.
  SL_HAL_ACMP_INPUT_PA7            = _ACMP_INPUTCTRL_POSSEL_PA7,  ///< Select Port A Pin7.
  SL_HAL_ACMP_INPUT_PA8            = _ACMP_INPUTCTRL_POSSEL_PA8,  ///< Select Port A Pin8.
  SL_HAL_ACMP_INPUT_PA9            = _ACMP_INPUTCTRL_POSSEL_PA9,  ///< Select Port A Pin9.
  SL_HAL_ACMP_INPUT_PA10           = _ACMP_INPUTCTRL_POSSEL_PA10, ///< Select Port A Pin10.
  SL_HAL_ACMP_INPUT_PA11           = _ACMP_INPUTCTRL_POSSEL_PA11, ///< Select Port A Pin11.
  SL_HAL_ACMP_INPUT_PA12           = _ACMP_INPUTCTRL_POSSEL_PA12, ///< Select Port A Pin12.
  SL_HAL_ACMP_INPUT_PA13           = _ACMP_INPUTCTRL_POSSEL_PA13, ///< Select Port A Pin13.
  SL_HAL_ACMP_INPUT_PA14           = _ACMP_INPUTCTRL_POSSEL_PA14, ///< Select Port A Pin14.
  SL_HAL_ACMP_INPUT_PA15           = _ACMP_INPUTCTRL_POSSEL_PA15, ///< Select Port A Pin15.
  SL_HAL_ACMP_INPUT_PB0            = _ACMP_INPUTCTRL_POSSEL_PB0,  ///< Select Port B Pin0.
  SL_HAL_ACMP_INPUT_PB1            = _ACMP_INPUTCTRL_POSSEL_PB1,  ///< Select Port B Pin1.
  SL_HAL_ACMP_INPUT_PB2            = _ACMP_INPUTCTRL_POSSEL_PB2,  ///< Select Port B Pin2.
  SL_HAL_ACMP_INPUT_PB3            = _ACMP_INPUTCTRL_POSSEL_PB3,  ///< Select Port B Pin3.
  SL_HAL_ACMP_INPUT_PB4            = _ACMP_INPUTCTRL_POSSEL_PB4,  ///< Select Port B Pin4.
  SL_HAL_ACMP_INPUT_PB5            = _ACMP_INPUTCTRL_POSSEL_PB5,  ///< Select Port B Pin5.
  SL_HAL_ACMP_INPUT_PB6            = _ACMP_INPUTCTRL_POSSEL_PB6,  ///< Select Port B Pin6.
  SL_HAL_ACMP_INPUT_PB7            = _ACMP_INPUTCTRL_POSSEL_PB7,  ///< Select Port B Pin7.
  SL_HAL_ACMP_INPUT_PB8            = _ACMP_INPUTCTRL_POSSEL_PB8,  ///< Select Port B Pin8.
  SL_HAL_ACMP_INPUT_PB9            = _ACMP_INPUTCTRL_POSSEL_PB9,  ///< Select Port B Pin9.
  SL_HAL_ACMP_INPUT_PB10           = _ACMP_INPUTCTRL_POSSEL_PB10, ///< Select Port B Pin10.
  SL_HAL_ACMP_INPUT_PB11           = _ACMP_INPUTCTRL_POSSEL_PB11, ///< Select Port B Pin11.
  SL_HAL_ACMP_INPUT_PB12           = _ACMP_INPUTCTRL_POSSEL_PB12, ///< Select Port B Pin12.
  SL_HAL_ACMP_INPUT_PB13           = _ACMP_INPUTCTRL_POSSEL_PB13, ///< Select Port B Pin13.
  SL_HAL_ACMP_INPUT_PB14           = _ACMP_INPUTCTRL_POSSEL_PB14, ///< Select Port B Pin14.
  SL_HAL_ACMP_INPUT_PB15           = _ACMP_INPUTCTRL_POSSEL_PB15, ///< Select Port B Pin15.
  SL_HAL_ACMP_INPUT_PC0            = _ACMP_INPUTCTRL_POSSEL_PC0,  ///< Select Port C Pin0.
  SL_HAL_ACMP_INPUT_PC1            = _ACMP_INPUTCTRL_POSSEL_PC1,  ///< Select Port C Pin1.
  SL_HAL_ACMP_INPUT_PC2            = _ACMP_INPUTCTRL_POSSEL_PC2,  ///< Select Port C Pin2.
  SL_HAL_ACMP_INPUT_PC3            = _ACMP_INPUTCTRL_POSSEL_PC3,  ///< Select Port C Pin3.
  SL_HAL_ACMP_INPUT_PC4            = _ACMP_INPUTCTRL_POSSEL_PC4,  ///< Select Port C Pin4.
  SL_HAL_ACMP_INPUT_PC5            = _ACMP_INPUTCTRL_POSSEL_PC5,  ///< Select Port C Pin5.
  SL_HAL_ACMP_INPUT_PC6            = _ACMP_INPUTCTRL_POSSEL_PC6,  ///< Select Port C Pin6.
  SL_HAL_ACMP_INPUT_PC7            = _ACMP_INPUTCTRL_POSSEL_PC7,  ///< Select Port C Pin7.
  SL_HAL_ACMP_INPUT_PC8            = _ACMP_INPUTCTRL_POSSEL_PC8,  ///< Select Port C Pin8.
  SL_HAL_ACMP_INPUT_PC9            = _ACMP_INPUTCTRL_POSSEL_PC9,  ///< Select Port C Pin9.
  SL_HAL_ACMP_INPUT_PC10           = _ACMP_INPUTCTRL_POSSEL_PC10, ///< Select Port C Pin10.
  SL_HAL_ACMP_INPUT_PC11           = _ACMP_INPUTCTRL_POSSEL_PC11, ///< Select Port C Pin11.
  SL_HAL_ACMP_INPUT_PC12           = _ACMP_INPUTCTRL_POSSEL_PC12, ///< Select Port C Pin12.
  SL_HAL_ACMP_INPUT_PC13           = _ACMP_INPUTCTRL_POSSEL_PC13, ///< Select Port C Pin13.
  SL_HAL_ACMP_INPUT_PC14           = _ACMP_INPUTCTRL_POSSEL_PC14, ///< Select Port C Pin14.
  SL_HAL_ACMP_INPUT_PC15           = _ACMP_INPUTCTRL_POSSEL_PC15, ///< Select Port C Pin15.
  SL_HAL_ACMP_INPUT_PD0            = _ACMP_INPUTCTRL_POSSEL_PD0,  ///< Select Port D Pin0.
  SL_HAL_ACMP_INPUT_PD1            = _ACMP_INPUTCTRL_POSSEL_PD1,  ///< Select Port D Pin1.
  SL_HAL_ACMP_INPUT_PD2            = _ACMP_INPUTCTRL_POSSEL_PD2,  ///< Select Port D Pin2.
  SL_HAL_ACMP_INPUT_PD3            = _ACMP_INPUTCTRL_POSSEL_PD3,  ///< Select Port D Pin3.
  SL_HAL_ACMP_INPUT_PD4            = _ACMP_INPUTCTRL_POSSEL_PD4,  ///< Select Port D Pin4.
  SL_HAL_ACMP_INPUT_PD5            = _ACMP_INPUTCTRL_POSSEL_PD5,  ///< Select Port D Pin5.
  SL_HAL_ACMP_INPUT_PD6            = _ACMP_INPUTCTRL_POSSEL_PD6,  ///< Select Port D Pin6.
  SL_HAL_ACMP_INPUT_PD7            = _ACMP_INPUTCTRL_POSSEL_PD7,  ///< Select Port D Pin7.
  SL_HAL_ACMP_INPUT_PD8            = _ACMP_INPUTCTRL_POSSEL_PD8,  ///< Select Port D Pin8.
  SL_HAL_ACMP_INPUT_PD9            = _ACMP_INPUTCTRL_POSSEL_PD9,  ///< Select Port D Pin9.
  SL_HAL_ACMP_INPUT_PD10           = _ACMP_INPUTCTRL_POSSEL_PD10, ///< Select Port D Pin10.
  SL_HAL_ACMP_INPUT_PD11           = _ACMP_INPUTCTRL_POSSEL_PD11, ///< Select Port D Pin11.
  SL_HAL_ACMP_INPUT_PD12           = _ACMP_INPUTCTRL_POSSEL_PD12, ///< Select Port D Pin12.
  SL_HAL_ACMP_INPUT_PD13           = _ACMP_INPUTCTRL_POSSEL_PD13, ///< Select Port D Pin13.
  SL_HAL_ACMP_INPUT_PD14           = _ACMP_INPUTCTRL_POSSEL_PD14, ///< Select Port D Pin14.
  SL_HAL_ACMP_INPUT_PD15           = _ACMP_INPUTCTRL_POSSEL_PD15, ///< Select Port D Pin15.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// ACMP initialization structure.
typedef struct {
  /// Controls the bias current.
  uint32_t                        bias_prog;

  /// Controls the VDD division factor. VREFOUT = VREFIN * (VREFDIV / 63).
  uint32_t                        vref_div;

  /// Selects the input range.
  sl_hal_acmp_input_range_t       input_range;

  /// Selects the ACMP accuracy mode.
  sl_hal_acmp_accuracy_t          accuracy;

#if defined(_ACMP_CFG_HYSTRISE_MASK)
  /// Selects the hysteresis level on rising edge.
  sl_hal_acmp_hysteresis_t        hysteresis_rise;

  /// Selects the hysteresis level on falling edge.
  sl_hal_acmp_hysteresis_t        hysteresis_fall;
#else
  /// Selects the hysteresis level.
  sl_hal_acmp_hysteresis_t        hysteresis;
#endif

  /// Inactive value emitted by ACMP during warmup.
  bool                            inactive_value;
} sl_hal_acmp_init_t;

/*******************************************************************************
 **************************   STRUCT INITIALIZERS   ****************************
 ******************************************************************************/

// PM5507: provide default configuration that is functional
/// Analog comparator CFG with initial bias value
#define PM5507_ACMP_CFG_BIAS_DEFAULT  0x00000004UL
/// Analog comparator reset value
#define PM5507_ACMP_CFG_RESETVALUE    0x00000004UL

/// Default configuration for ACMP regular initialization.
#if defined(_ACMP_CFG_HYSTRISE_MASK)
#define SL_HAL_ACMP_INIT_DEFAULT                        \
  {                                                     \
    .bias_prog = PM5507_ACMP_CFG_BIAS_DEFAULT,          \
    .input_range = SL_HAL_ACMP_INPUT_RANGE_FULL,        \
    .accuracy = SL_HAL_ACMP_ACCURACY_LOW,               \
    .hysteresis_rise = SL_HAL_ACMP_HYSTERESIS_DISABLED, \
    .hysteresis_fall = SL_HAL_ACMP_HYSTERESIS_DISABLED, \
    .inactive_value = false,                            \
    .vref_div = 0x3F,                                   \
  }
#else
#define SL_HAL_ACMP_INIT_DEFAULT                   \
  {                                                \
    .bias_prog = PM5507_ACMP_CFG_BIAS_DEFAULT,     \
    .input_range = SL_HAL_ACMP_INPUT_RANGE_FULL,   \
    .accuracy = SL_HAL_ACMP_ACCURACY_LOW,          \
    .hysteresis = SL_HAL_ACMP_HYSTERESIS_DISABLED, \
    .inactive_value = false,                       \
    .vref_div = 0x3F,                              \
  }
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize ACMP.
 *
 * @note
 *   A call to ACMP_Init can cause side effects since it can enable/disable
 *   the peripheral.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @param[in] init
 *   A pointer to the initialization structure used to configure ACMP.
 ******************************************************************************/
void sl_hal_acmp_init(ACMP_TypeDef *acmp,
                      const sl_hal_acmp_init_t *init);

/***************************************************************************//**
 * @brief
 *   Reset ACMP to the state it is after a hardware reset.
 *
 * @note
 *   The GPIO ACMP ROUTE register is NOT reset by this function to allow for
 *   centralized setup of this feature.
 *
 * @note
 *   The peripheral may be enabled and disabled during reset.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 ******************************************************************************/
void sl_hal_acmp_reset(ACMP_TypeDef *acmp);

/***************************************************************************//**
 * @brief
 *   Set up GPIO output inversion from the ACMP.
 *
 * @param[in] acmp
 *   Pointer to the ACMP peripheral register block.
 *
 * @param[in] invert
 *   true = GPIO output is inverted.
 *   false = GPIO output is not inverted.
 ******************************************************************************/
void sl_hal_acmp_setup_gpio_inversion(ACMP_TypeDef *acmp,
                                      bool invert);

/***************************************************************************//**
 * @brief
 *   Set which channels are used in ACMP comparisons.
 *
 * @note
 *   Can only be called when the peripheral is enabled.
 *
 * @note
 *   If GPIO is used for both posSel and negSel, they cannot both use even
 *   or odd pins.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @param[in] negative_input
 *   ACMP channel for the negative input.
 *
 * @param[in] positive_input
 *   ACMP channel for the positive input.
 ******************************************************************************/
void sl_hal_acmp_set_input(ACMP_TypeDef *acmp,
                           sl_hal_acmp_input_t negative_input,
                           sl_hal_acmp_input_t positive_input);

/***************************************************************************//**
 * @brief
 *   Enable ACMP.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 ******************************************************************************/
void sl_hal_acmp_enable(ACMP_TypeDef *acmp);

/***************************************************************************//**
 * @brief
 *   Disable ACMP.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 ******************************************************************************/
void sl_hal_acmp_disable(ACMP_TypeDef *acmp);

/***************************************************************************//**
 * @brief
 *   Clear one or more pending ACMP interrupts.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @param[in] flags
 *   Pending ACMP interrupt source to clear. Use one or more valid interrupt
 *   flags for the ACMP module (ACMP_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_acmp_clear_interrupts(ACMP_TypeDef *acmp,
                                           uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  acmp->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more ACMP interrupts.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @param[in] flags
 *   ACMP interrupt sources to disable. Use one or more valid interrupt
 *   flags for the ACMP module (ACMP_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_acmp_disable_interrupts(ACMP_TypeDef *acmp,
                                             uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  acmp->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more ACMP interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. Use ACMP_IntClear() prior to enabling if a pending
 *   interrupt should be ignored.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @param[in] flags
 *   ACMP interrupt sources to enable. Use one or more valid interrupt
 *   flags for the ACMP module (ACMP_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_acmp_enable_interrupts(ACMP_TypeDef *acmp,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  acmp->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending ACMP interrupt flags.
 *
 * @note
 *   This function does not clear event bits.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @return
 *   Pending ACMP interrupt sources. Use one or more valid interrupt
 *   flags for the ACMP module (ACMP_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_acmp_get_pending_interrupts(ACMP_TypeDef *acmp)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  return acmp->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending ACMP interrupt flags.
 *
 * @details
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note
 *   This function does not clear interrupt flags.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @return
 *   Pending and enabled ACMP interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in ACMPx_IEN_nnn
 *     register (ACMPx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the ACMP module
 *     (ACMPx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_acmp_get_enabled_pending_interrupts(ACMP_TypeDef *acmp)
{
  uint32_t tmp;
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  /* Store ACMPx->IEN in a temporary variable to define the explicit order
   * of volatile accesses. */
  tmp = acmp->IEN;

  /* Bitwise AND of pending and enabled interrupts. */
  return acmp->IF & tmp;
}

/***************************************************************************//**
 * @brief
 *   Get enabled ACMP interrupts.
 *
 * @details
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @return
 *   Enabled ACMP interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_acmp_get_enabled_interrupts(ACMP_TypeDef *acmp)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  return acmp->IEN;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending ACMP interrupts from software.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @param[in] flags
 *   ACMP interrupt sources to set as pending. Use a bitwise logic OR
 *   combination of valid interrupt flags for the ACMP module. The flags can be,
 *   for instance, ACMP_IFS_EDGE or ACMP_IFS_WARMUP.
 ******************************************************************************/
__INLINE void sl_hal_acmp_set_interrupts(ACMP_TypeDef *acmp,
                                         uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  acmp->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Convert GPIO port/pin to ACMP input selection.
 *
 * @param[in] port
 *   GPIO port to convert.
 *
 * @param[in] pin
 *   GPIO pin to convert.
 *
 * @return
 *   ACMP input selection of the selected GPIO port and pin.
 ******************************************************************************/
__INLINE sl_hal_acmp_input_t sl_hal_acmp_gpio_to_input(uint32_t port,
                                                       uint32_t pin)
{
  uint32_t input = ((port + (_ACMP_INPUTCTRL_POSSEL_PA0 >> 4)) << 4) | pin;

  return (sl_hal_acmp_input_t) input;
}

/***************************************************************************//**
 * Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_acmp_wait_sync(ACMP_TypeDef *acmp)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  while ((acmp->EN & _ACMP_EN_EN_MASK) && (acmp->SYNCBUSY != 0U)) {
    // Wait for synchronization to finish
  }
}

/***************************************************************************//**
 * Wait for disabling to finish.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_acmp_wait_ready(ACMP_TypeDef * acmp)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  while ( 0
#if defined(_ACMP_SWRST_RESETTING_MASK)
          || (acmp->SWRST & _ACMP_SWRST_RESETTING_MASK)
#endif
#if defined(_ACMP_EN_DISABLING_MASK)
          || (acmp->EN & _ACMP_EN_DISABLING_MASK)
#endif
          ) {
    // Wait for all synchronizations to finish
  }
}

/***************************************************************************//**
 * Return the content of the status register.
 *
 * @param[in] acmp
 *   A pointer to the ACMP peripheral register block.
 *
 * @return Status register.
 ******************************************************************************/
__INLINE uint32_t sl_hal_acmp_get_status(ACMP_TypeDef * acmp)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ACMP_REF_VALID(acmp));

  return acmp->STATUS;
}

/** @} (end addtogroup acmp) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/****************************************************************************//**
 * @addtogroup acmp ACMP - Analog Comparator
 * @{
 *
 *@n @section acmp_intro Introduction
 *  This module contains functions to control the Analog Comparator (ACMP)
 *  peripheral of Silicon Labs' 32-bit MCUs and SoCs.
 *
 *  The Analog Comparator compares the voltage of two analog inputs and outputs
 *  a digital signal indicating which input voltage is higher. Inputs can either
 *  be from internal references or from external pins. Response time, and thereby
 *  the current consumption, can be configured by altering the current supply to
 *  the comparator.
 *
 *@n @section acmp_example Example
 *
 *  This example shows how to use the em_acmp.h API for comparing an input
 *  pin to an internal 2.5 V reference voltage.
 *
 *  ACMP Base example:
 *  @code{.c}
 *  {
 *    #define GPIO_ACMPOUT    PD1  // Pin location for ACMP OUT varies depending on the device.
 *
 *    // Enable bus clocks.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ACMP0);
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *
 *    // Initialize with default settings.
 *    sl_hal_acmp_init_t init = SL_HAL_ACMP_INIT_DEFAULT;
 *
 *    sl_hal_acmp_init(ACMP0, &init);
 *    sl_hal_acmp_enable(ACMP0);
 *
 *    // Now we select the two inputs to compare. Here we compare the SL_HAL_ACMP_INPUT_PD2
 *    // input to the internal 2.5V reference. When SL_HAL_ACMP_INPUT_PD2 is lower than
 *    // 2.5 V then the ACMP output is 0 and when SL_HAL_ACMP_INPUT_PD2 is higher than
 *    // 2.5 V then the ACMP output is 1.
 *    sl_hal_acmp_set_input(ACMP0, SL_HAL_ACMP_INPUT_VREFDIV2V5, SL_HAL_ACMP_INPUT_PD2);
 *
 *    // Allocate CDEVEN0 to ACMP0 to be able to use the input.
 *    GPIO->CDBUSALLOC = GPIO_CDBUSALLOC_CDEVEN0_ACMP0;
 *
 *    // To be able to probe the output we can send the ACMP output to a pin.
 *    sl_hal_acmp_setup_gpio_inversion(ACMP0, false);
 *    GPIO->ACMPROUTE[ACMP_NUM(ACMP0)].ACMPOUTROUTE = (GPIO_ACMPOUT->port << _GPIO_ACMP_ACMPOUTROUTE_PORT_SHIFT)
 *                                                  | (GPIO_ACMPOUT->pin << _GPIO_ACMP_ACMPOUTROUTE_PIN_SHIFT);
 *    GPIO->ACMPROUTE[ACMP_NUM(ACMP0)].ROUTEEN = GPIO_ACMP_ROUTEEN_ACMPOUTPEN;
 *  }
 *
 *  @endcode
 *
 * @} (end addtogroup acmp)
 ******************************************************************************/
/* *INDENT-ON* */

#endif // ACMP_PRESENT
#endif // SL_HAL_ACMP_H
