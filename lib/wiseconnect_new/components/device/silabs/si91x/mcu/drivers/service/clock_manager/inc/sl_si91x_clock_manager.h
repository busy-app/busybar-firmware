/******************************************************************************
* @file sl_si91x_clock_manager.h
* @brief Clock Manager Service API implementation
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

#ifndef SL_SI91X_CLOCK_MANAGER_H
#define SL_SI91X_CLOCK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "rsi_pll.h"

#ifdef SL_SI91X_MCU_CLK_OUT_CONFIG
#include "sl_si91x_clock_manager_mcu_clk_out_config.h"
#endif
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
#include "sl_driver_gpio.h"
#endif
/***************************************************************************/
/**
 * @addtogroup  CLOCK-MANAGER Clock Manager
 * @ingroup SI91X_SERVICE_APIS
 * @{ 
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------------
// GLOBAL DEFINES / MACROS
// Macros for defining supported PLL Ref Clock frequencies
#define PLL_REF_CLK_VAL_XTAL (40000000UL)  ///< PLL reference clock frequency value of XTAL CLK
#define MAX_PLL_FREQUENCY    (180000000UL) ///< Max PLL frequency is 180MHz
// -----------------------------------------------------------------------------------

#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
/***************************************************************************/
/**
 * @brief Enumeration for MCU Clock output source selection.
 *
 * @details This enumeration defines the various clock sources that can be selected for the MCU Clock output.
 */
typedef enum {
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_GATED      = 0,  ///< Output Clock is Gated.
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32MHZ   = 1,  ///< High Frequency RC Clock source.
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL       = 2,  ///< XTAL Clock source.
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32KHZ   = 7,  ///< Low Frequency RC Clock source.
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL_32KHZ = 8,  ///< Low Frequency XTAL Clock source.
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_INTF_PLL   = 10, ///< Interface PLL Clock.
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_SOC_PLL    = 13, ///< SoC PLL Clock.
  SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_I2S_PLL    = 14, ///< I2S PLL Clock.
} sl_clock_manager_mcu_clk_out_sel_t;

/**
 * @brief Structure to hold the parameters for configuring the external MCU clock output.
 */
typedef struct {
  sl_si91x_gpio_pin_config_t pin_config;         // Port of the GPIO pin
  sl_clock_manager_mcu_clk_out_sel_t clk_source; // Clock source selection
  uint8_t div_factor;                            // Clock division factor
} sl_si91x_clock_manager_mcu_clk_out_config_t;

#endif

/***************************************************************************/
/**
 * @typedef sl_si91x_m4_soc_clk_src_sel_t
 * @brief Typedef to select the clock source for the M4 core in the Si91x SOC.
 *
 * @details This typedef maps to the M4_SOC_CLK_SRC_SEL_T type, which defines the possible clock source selections for the M4 core. 
 *          The clock source selection impacts the operating frequency and performance characteristics of the M4 processor within the Si91x SOC.
 *
 *          Users can configure the clock source to optimize power consumption or performance based on application requirements.
 *
 * @note Ensure that the selected clock source is properly configured and stable before switching to avoid system instability.
 ******************************************************************************/
typedef M4_SOC_CLK_SRC_SEL_T sl_si91x_m4_soc_clk_src_sel_t;
// -----------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------
// GLOBAL FUNCTION PROTOTYPES

/***************************************************************************/
/**
 * @brief Initializes the M4_SOC and other required clocks.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - Corresponding error code on failure.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_clock_manager_init(void);
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
/***************************************************************************/
/**
 * @brief Configures the MCU Clock output on a specified GPIO pin.
 *
 * @details Sets up a GPIO pin to output a selected clock signal with optional frequency division.
 * If SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_GATED is selected, the clock output is disabled.
 *
 * @param[in] gpio_pin_config  GPIO pin configuration structure specifying the pin to use.
 *                             Only GPIO_11, GPIO_12, or GPIO_15 are supported.
 * @param[in] mcu_clk_out_sel  Clock source selection for MCU Clock output generation.
 * @param[in] div_factor       Division factor for the clock output.
 *                             - 0: Divider is bypassed.
 *                             - (> 0): clk_out = clk_in / (div_factor * 2).
 *
 * @return sl_status_t Status code indicating the result:
 *   - SL_STATUS_OK if the configuration was successful.
 *   - SL_STATUS_INVALID_PARAMETER if an invalid GPIO pin, clock source, or division factor is provided.
 *   - SL_STATUS_INITIALIZATION_FAILED if the selected clock is not present.
 *   - Corresponding error code on other failures.
 *
 * @note Ensure that the selected clock source is available and that div_factor is
 *       within the valid range of 0x00-0x3F (6 bits) to prevent operational issues.
 ***************************************************************************/
sl_status_t sl_si91x_clock_manager_mcu_clk_out(sl_si91x_gpio_pin_config_t gpio_pin_config,
                                               sl_clock_manager_mcu_clk_out_sel_t mcu_clk_out_sel,
                                               uint32_t div_factor);
#endif

/***************************************************************************/
/**
 * @brief To configure the M4 core clock source and configure the PLL frequency if selected as source.
 * 
 * @param[in] clk_source Enum value representing different core clock sources.
 * @param[in] pll_freq Desired M4 core frequency in MHz.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - Corresponding error code on failure.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_clock_manager_m4_set_core_clk(M4_SOC_CLK_SRC_SEL_T clk_source, uint32_t pll_freq);

/***************************************************************************/
/**
 * @brief To set the selected Phase-Locked Loop (PLL) clock to the desired frequency.
 * 
 * @param[in] pll_type Enum specifying the type of PLL to configure.
 * @param[in] pll_freq Desired frequency for the PLL clock (in MHz).
 * @param[in] pll_ref_clk Reference clock frequency for the PLL configuration.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - Corresponding error code on failure.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_clock_manager_set_pll_freq(PLL_TYPE_T pll_type, uint32_t pll_freq, uint32_t pll_ref_clk);

/***************************************************************************/
/**
 * @brief To read the currently active M4 core clock source and its frequency.
 * 
 * @param[out] m4_core_clk_freq Pointer to a variable where the current core clock frequency will be stored (in MHz).
 * 
 * @return sl_si91x_m4_soc_clk_src_sel_t The currently active core clock source:
 *         - 0: M4_ULPREFCLK
 *         - 2: M4_SOCPLLCLK
 *         - 3: M4_MODEMPLLCLK1
 *         - 4: M4_INTFPLLCLK
 *         - 5: M4_SLEEPCLK
 * 
 ******************************************************************************/
sl_si91x_m4_soc_clk_src_sel_t sl_si91x_clock_manager_m4_get_core_clk_src_freq(uint32_t *m4_core_clk_freq);

/***************************************************************************/
/**
 * @brief Gets the selected PLL (Phase-Locked Loop) clock to the desired frequency.
 * 
 * @param[in] pll_type Enum specifying the type of PLL to configure.
 * 
 * @return uint32_t PLL frequency value in MHz.
 ******************************************************************************/
uint32_t sl_si91x_clock_manager_get_pll_freq(PLL_TYPE_T pll_type);

/***************************************************************************/
/**
 * @brief Controls the selected PLL (Phase-Locked Loop) clock.
 * 
 * @param[in] pll_type Enum specifying the type of PLL to control.
 * @param[in] enable Boolean value to enable (true) or disable (false) the PLL.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - Corresponding error code on failure.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_clock_manager_control_pll(PLL_TYPE_T pll_type, bool enable);

/***************************************************************************/
/**
 * @brief Delays execution for the specified number of milliseconds.
 *
 * @param[in] milli_seconds Delay time in milliseconds.
 *
 * @note This function provides a blocking delay.The delay is calibrated based on the SystemCoreClock frequency.
 *  If SystemCoreClock < CLOCK_THRESHOLD, the delay is calibrated with a division factor of LOW_FREQ_CLK_DIV_FAC.
 *  If SystemCoreClock >= CLOCK_THRESHOLD, the delay is calibrated with a division factor of HIGH_FREQ_CLK_DIV_FAC.
 *  This function uses `__NOP()` instructions for the delay loop.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
void sl_si91x_delay_ms(uint32_t milli_seconds);

/***************************************************************************/
/** 
 * @fn     sl_si91x_clock_manager_ulp_processor_clk_division(uint8_t clk_div)
 * 
 * @brief   Set M4 ULP Processor clock division factor.
 *
 * @details This API sets the M4 ULP Processor clock division factor by configuring the clock control register.
 *          The division factor is calculated as 2*clk_div, where clk_div is the input parameter.
 *
 * @param clk_div Division factor (the clock will be divided by 2*clk_div)
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER - Invalid argument.
 *
 * @note  This API should be invoked only when the system is in PS2 state, as the source clock in this mode is configured to use a 20 MHz RC.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_clock_manager_ulp_processor_clk_division(uint8_t clk_div);

// -----------------------------------------------------------------------------------

/// @} end addtogroup CLOCK-MANAGER ******************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/***************************************************************************/
/***************************************************************************/
/**
 * @addtogroup CLOCK-MANAGER Clock Manager
 * @{
 *
 * @details
 *
 * @section Clock_Manager_Intro Introduction
 *
 * The Clock Manager module is responsible for configuring the clock tree in the Si91x system.
 * It enhances the clock management features provided by the GSDK's emlib and device_init services,
 * offering additional functionality for selecting clock sources and setting dividers.
 * This module ensures optimized performance and power efficiency across various system components
 * by managing clock configurations according to application requirements.
 *
 * @section Clock_Manager_Config Configuration
 *
 * The Clock Manager supports a variety of configurations:
 * - **Clock Source Selection**: Allows specific clock sources to be chosen for different branches.
 * - **Divider Configuration**: Modifies clock frequencies by setting divider values.
 *
 * Configuration is typically performed during device initialization using configuration files and source code.
 *
 * @section Clock_Manager_Usage Usage
 *
 * The following steps outline typical usage of the Clock Manager:
 * 1. Initialize the Clock Manager with desired configuration settings.
 * 2. Set up oscillator sources and configure clock dividers.
 * 3. Use runtime APIs to manage dynamic clock changes.
 * 4. Maintain stable clock configurations to ensure system reliability.
 *
 * @section Clock_Manager_Benefits Benefits
 *
 * - Manages complex clock trees efficiently.
 * - Enhances system stability and performance.
 * - Supports a wide range of Si91x devices.
 * - Optimizes power consumption through efficient clock management.
 *
 * @} (end addtogroup CLOCK-MANAGER)
 */
/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_CLOCK_MANAGER_H */