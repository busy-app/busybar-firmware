/******************************************************************************
* @file sl_si91x_clock_manager.c
* @brief Clock Manager Service API implementation
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

#include "sl_si91x_clock_manager.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "sl_component_catalog.h"
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
#include "sl_si91x_gpio.h"
#include "sl_gpio_board.h"
#include "sl_si91x_peripheral_gpio.h"
#include "sl_si91x_driver_gpio.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#endif
/************************************************************************************
 *************************  DEFINES / MACROS  ***************************************
 ************************************************************************************/
#define MANUAL_LOCK            1             // Manual lock enable
#define BYPASS_MANUAL_LOCK     1             // Bypass manual lock enable
#define SOC_PLL_MM_COUNT_LIMIT 0xA4          // Soc pll count limit
#define DIVISION_FACTOR        0             // Division factor
#define QSPI_ODD_DIV_EN        0             // Odd division enable for QSPI clock
#define QSPI_SWALLO_EN         0             // Swallo enable for QSPI clock
#define QSPI_DIV_FACTOR        1             // Division factor for QSPI clock
#define QSPI2_DIV_FACTOR       1             // Division factor for QSPI2 clock
#define PLL_PREFETCH_LIMIT     (120000000UL) // 120MHz Limit for pll clock
#define SOC_PLL_FREQ           (180000000UL) // 180MHz default SoC PLL Clock as source to Processor
#define INTF_PLL_FREQ          (160000000UL) // 160MHz default Interface PLL Clock as source to all peripherals
#define LOW_FREQ_CLK_DIV_FAC \
  6 // Division factor used for delay calibration when system clock is below the CLOCK_THRESHOLD macro
#define HIGH_FREQ_CLK_DIV_FAC \
  12 // Division factor used for delay calibration when system clock is above the CLOCK_THRESHOLD macro
#define MILLISECONDS_TO_CYCLES 1000      // Conversion factor from milliseconds to clock cycles.
#define CLOCK_THRESHOLD        120000000 //Threshold clock frequency in Hertz for delay calibration.
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
#define MCU_CLK_OUT_GPIO_OUTPUT_HIGH        1           // GPIO output high value
#define MCU_CLK_OUT_GPIO_OUTPUT_LOW         0           // GPIO output low value
#define MCU_CLK_OUT_DIV_FACTOR_MAX          0x3F        // Maximum division factor for MCU clock out
#define SL_SI91X_MCU_CLK_OUT_GPIO_DIRECTION GPIO_OUTPUT // Direction output
#endif
#define ULP_PROC_MAX_CLK_DIV_FAC 5 // Maximum division factor for ULP processor clock division
/************************************************************************************
 *************************  LOCAL VARIABLES  ****************************************
 ************************************************************************************/
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
sl_si91x_clock_manager_mcu_clk_out_config_t sl_mcu_clk_out_config = {
    .pin_config = {
        .port_pin = {
            .port = SL_SI91X_MCU_CLK_OUT_GPIO_PORT,
            .pin = SL_SI91X_MCU_CLK_OUT_GPIO_PIN
        },
        .direction = SL_SI91X_MCU_CLK_OUT_GPIO_DIRECTION,
    },
    .clk_source = SL_CLOCK_MANAGER_MCU_CLK_OUT_SOURCE,
    .div_factor = SL_CLOCK_MANAGER_MCU_CLK_OUT_DIV_FACTOR
};
#endif
/************************************************************************************
 *************************  LOCAL TYPE DEFINITIONS  *********************************
 ************************************************************************************/

/************************************************************************************
 *************************  LOCAL FUNCTION PROTOTYPES  ******************************
 ************************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error);
/************************************************************************************
 *************************  GLOBAL FUNCTION DEFINITIONS  ****************************
 ************************************************************************************/
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
sl_status_t sl_si91x_clock_manager_init(void)
{
#ifdef SL_SI91X_REQUIRES_INTF_PLL
  M4CLK_Type *pCLK = M4CLK;
#endif
  sl_status_t status = SL_STATUS_OK;

#ifdef SL_SI91X_ULP_STATE_ENABLE
  //Trimming the RC_32MHz clock down to 20MHz, which is utilized in the PS2 state
  RSI_IPMU_M20rcOsc_TrimEfuse();
  // Sets FSM HF frequency to 20MHz
  RSI_PS_FsmHfFreqConfig(20);
  // Updated the clock global variables
  RSI_PS_PS2UpdateClockVariable();
#endif
  // Configure FSM Low Frequency Clock
  // FSM based on XTAL or EXT_OSC has been done in SystemCoreClockUpdate

  /* Configure Ref clock to 40Mhz crystal */
  RSI_CLK_M4ssRefClkConfig(M4CLK, EXT_40MHZ_CLK);
  RSI_ULPSS_RefClkConfig(ULPSS_40MHZ_CLK);

  // Core Clock runs at 180MHz SOC PLL Clock
  status = sl_si91x_clock_manager_m4_set_core_clk(M4_SOCPLLCLK, SOC_PLL_FREQ);
  if (status != SL_STATUS_OK) {
    return status;
  }

#ifdef SL_SI91X_REQUIRES_INTF_PLL
  // Configuring the interface PLL clock to 160MHz used by the peripherals whose source clock is INTF_PLL
  status = sl_si91x_clock_manager_set_pll_freq(INTF_PLL, INTF_PLL_FREQ, PLL_REF_CLK_VAL_XTAL);
  if (status != SL_STATUS_OK) {
    return status;
  }
// Configure QSPI clock with INTF PLL as input source
#if defined(CLOCK_ROMDRIVER_PRESENT)
  ROMAPI_M4SS_CLK_API->clk_qspi_clk_config(pCLK, QSPI_INTFPLLCLK, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, QSPI_DIV_FACTOR);
#endif

#ifdef SLI_SI91X_MCU_PSRAM_PRESENT
  // Configure QSPI2 clock with INTF PLL as input source
#if defined(CLOCK_ROMDRIVER_PRESENT)
  ROMAPI_M4SS_CLK_API->clk_qspi_2_clk_config(pCLK, QSPI_INTFPLLCLK, QSPI_SWALLO_EN, QSPI_ODD_DIV_EN, QSPI2_DIV_FACTOR);
#endif
#endif
#endif /* SL_SI91X_REQUIRES_INTF_PLL */
#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
  sl_si91x_clock_manager_mcu_clk_out(sl_mcu_clk_out_config.pin_config,
                                     sl_mcu_clk_out_config.clk_source,
                                     sl_mcu_clk_out_config.div_factor);
#endif
  return status;
}

#if (defined(SL_SI91X_MCU_CLK_OUT_EN) && (SL_SI91X_MCU_CLK_OUT_EN == 1))
/***************************************************************************/
/**
 * @brief Configures the MCU Clock output on a specified GPIO pin.
 *
 * @details Sets up a GPIO pin to output a selected clock signal with optional frequency division.
 * If SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_GATED is selected, the clock output is disabled.
 *
 * @param[in] gpio_pin_config  GPIO pin configuration structure specifying the pin to use.
 *                             Supported only on GPIO_11, GPIO_12, or GPIO_15.
 * @param[in] mcu_clk_out_sel  Clock source selection for MCU clock generation.
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
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_clock_manager_mcu_clk_out(sl_si91x_gpio_pin_config_t gpio_pin_config,
                                               sl_clock_manager_mcu_clk_out_sel_t mcu_clk_out_sel,
                                               uint32_t div_factor)
{
  M4CLK_Type *pCLK         = M4CLK;
  sl_status_t status       = SL_STATUS_OK;
  rsi_error_t error_status = RSI_OK;
  sl_gpio_mode_t pin_mode  = SL_GPIO_MODE_0; // Default pin mode

  // Validate GPIO pin
  if ((gpio_pin_config.port_pin.pin != SL_SI91X_GPIO_11_PIN) && (gpio_pin_config.port_pin.pin != SL_SI91X_GPIO_12_PIN)
      && (gpio_pin_config.port_pin.pin != SL_SI91X_GPIO_15_PIN)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Validate Division factor
  if (div_factor > MCU_CLK_OUT_DIV_FACTOR_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable GPIO clock
  status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Configure GPIO pin for MCU_CLK_OUT (External Clock)
  status = sl_gpio_set_configuration(gpio_pin_config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Initially set pin mode to default
  status = sl_gpio_driver_set_pin_mode(&gpio_pin_config.port_pin, pin_mode, MCU_CLK_OUT_GPIO_OUTPUT_LOW);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Disable the External Clock on GPIO PAD before configuration
  error_status = RSI_CLK_PeripheralClkDisable(pCLK, MCUCLKOUT_CLK);
  status       = convert_rsi_to_sl_error_code(error_status);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Validate clock source
  switch (mcu_clk_out_sel) {
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_GATED:
      // For gated clock source, return success immediately as the MCU clock is already disabled
      return SL_STATUS_OK;
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32MHZ:
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL:
      // These clock sources are already available and don't need additional initialization / validation
      break;
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_RC_32KHZ:
      if (RSI_OK != RSI_ULPSS_EnableRefClks(MCU_ULP_32KHZ_RC_CLK_EN, ULP_PERIPHERAL_CLK, 0)) {
        return SL_STATUS_NOT_INITIALIZED;
      }
      break;
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_XTAL_32KHZ:
      if (RSI_OK != RSI_ULPSS_EnableRefClks(MCU_ULP_32KHZ_XTAL_CLK_EN, ULP_PERIPHERAL_CLK, 0)) {
        return SL_STATUS_NOT_INITIALIZED;
      }
      break;
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_INTF_PLL:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, INTF_PLL_CLK_PRESENT)) {
        return SL_STATUS_NOT_INITIALIZED;
      }
      break;
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_SOC_PLL:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, SOC_PLL_CLK_PRESENT)) {
        return SL_STATUS_NOT_INITIALIZED;
      }
      break;
    case SL_CLOCK_MANAGER_MCU_CLK_OUT_SEL_I2S_PLL:
      if (RSI_OK != RSI_CLK_CheckPresent(pCLK, I2S_PLL_CLK_PRESENT)) {
        return SL_STATUS_NOT_INITIALIZED;
      }
      break;
    default:
      return SL_STATUS_INVALID_PARAMETER;
  }

  // Set pin mode for MCU_CLK_OUT (External Clock)
  pin_mode = (gpio_pin_config.port_pin.pin == SL_SI91X_GPIO_11_PIN) ? SL_GPIO_MODE_12 : SL_GPIO_MODE_8;
  status   = sl_gpio_driver_set_pin_mode(&gpio_pin_config.port_pin, pin_mode, MCU_CLK_OUT_GPIO_OUTPUT_HIGH);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Set division factor for External Clock
  M4CLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_DIV_FAC = (unsigned int)(div_factor & 0x3F);

  // Set clock source to be used for External Clock
  M4CLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = (unsigned int)(mcu_clk_out_sel & 0x0F);

  // Enable the External Clock on GPIO PAD
  error_status = RSI_CLK_PeripheralClkEnable(pCLK, MCUCLKOUT_CLK, ENABLE_STATIC_CLK);
  status       = convert_rsi_to_sl_error_code(error_status);

  // Wait for the External Clock to be switched
  while ((pCLK->PLL_STAT_REG_b.MCU_CLKOUT_SWITCHED) != true)
    ;

  return status;
}
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
sl_status_t sl_si91x_clock_manager_m4_set_core_clk(M4_SOC_CLK_SRC_SEL_T clk_source, uint32_t pll_freq)
{
  M4CLK_Type *pCLK         = M4CLK;
  uint32_t div_factor      = DIVISION_FACTOR;
  rsi_error_t error_status = RSI_OK;
  sl_status_t status;

  // PLL reference clock set to XTAL_CLK for PLL configuration
  uint32_t pll_ref_clk = PLL_REF_CLK_VAL_XTAL;

  // Validating for correct Clock Source input
  if ((clk_source > M4_SLEEPCLK) || (pll_freq > MAX_PLL_FREQUENCY)) {
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }

  // Configure the registers for clock less than 120MHz
  if (pll_freq < PLL_PREFETCH_LIMIT) {
    RSI_PS_PS4ClearRegisters();
  }
  // Changing M4 SOC clock to M4_ULPREFCLK
  error_status = RSI_CLK_M4SocClkConfig(pCLK, M4_ULPREFCLK, 0);
  status       = convert_rsi_to_sl_error_code(error_status);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Configure the required PLL Clocks with desired frequency before configuring it to M4 Core
  if (clk_source == M4_ULPREFCLK) {
    // ULP REF clock has already been set as M4 SoC source by now
    UNUSED_PARAMETER(pll_freq);
    return status;
  } else if (clk_source == M4_INTFPLLCLK) {
    // RSI API to set INTF PLL clock frequency
    error_status = sl_si91x_clock_manager_set_pll_freq(INTF_PLL, pll_freq, pll_ref_clk);
  } else if (clk_source == M4_SOCPLLCLK) {
    // RSI API to set SOC PLL clock frequency
    error_status = sl_si91x_clock_manager_set_pll_freq(SOC_PLL, pll_freq, pll_ref_clk);
  }

  // The error status is converted to the SL error code
  status = convert_rsi_to_sl_error_code(error_status);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // RSI API to set M4 SOC clock is called and the status is converted to the SL error code.
  error_status = RSI_CLK_M4SocClkConfig(pCLK, clk_source, div_factor);
  status       = convert_rsi_to_sl_error_code(error_status);

#if defined(SL_CATALOG_KERNEL_PRESENT) && (SL_SI91X_TICKLESS_MODE == 0)
  // Reconfigure the system tick timer after changing the core clock
  SysTick_Config(SystemCoreClock / 1000);
#endif

  return status;
}

/***************************************************************************/
/**
 * @brief To set the selected PLL (Phase-Locked Loop) clock to the desired frequency.
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
sl_status_t sl_si91x_clock_manager_set_pll_freq(PLL_TYPE_T pll_type, uint32_t pll_freq, uint32_t pll_ref_clk)
{
  const M4CLK_Type *pCLK   = M4CLK;
  rsi_error_t error_status = RSI_OK;
  sl_status_t status;

  //Return the error code if frequency is more than 180MHz
  if (pll_freq > MAX_PLL_FREQUENCY) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Configure the registers for clock more than 120MHz in PS4
  if (pll_freq >= PLL_PREFETCH_LIMIT) {
    RSI_PS_PS4SetRegisters();
  }

  switch (pll_type) {
    case SOC_PLL:
      // Configure SOC-PLL lock settings before configuring SOC PLL clock
      RSI_CLK_SocPllLockConfig(MANUAL_LOCK, BYPASS_MANUAL_LOCK, SOC_PLL_MM_COUNT_LIMIT);

      // RSI API to set SOC PLL clock frequency
      error_status = RSI_CLK_SetSocPllFreq(pCLK, pll_freq, pll_ref_clk);
      break;

    case INTF_PLL:
      // RSI API to set INTF PLL clock frequency
      error_status = RSI_CLK_SetIntfPllFreq(pCLK, pll_freq, pll_ref_clk);
      break;

    case I2S_PLL:
      // RSI API to set I2S PLL clock frequency
      error_status = RSI_CLK_SetI2sPllFreq(pCLK, pll_freq, pll_ref_clk);
      break;

    default:
      break;
  }

  status = convert_rsi_to_sl_error_code(error_status);
  return status;
}

/***************************************************************************/
/**
 * @brief To read the currently active M4 core clock source and its frequency.
 * 
 * @param[out] core_clock Pointer to a variable where the current core clock frequency will be stored (in MHz).
 * 
 * @return sl_si91x_m4_soc_clk_src_sel_t The currently active core clock source:
 *         - 0: M4_ULPREFCLK
 *         - 2: M4_SOCPLLCLK
 *         - 3: M4_MODEMPLLCLK1
 *         - 4: M4_INTFPLLCLK
 *         - 5: M4_SLEEPCLK
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_si91x_m4_soc_clk_src_sel_t sl_si91x_clock_manager_m4_get_core_clk_src_freq(uint32_t *m4_core_clk_freq)
{
  const M4CLK_Type *pCLK = M4CLK;
  sl_si91x_m4_soc_clk_src_sel_t m4_core_clk_src;

  // return currently active core clock frequency via the pointer by reference
  *m4_core_clk_freq = system_clocks.soc_clock;

  // read currently active core clock source
  m4_core_clk_src = pCLK->CLK_CONFIG_REG5_b.M4_SOC_CLK_SEL;

  return m4_core_clk_src;
}

/***************************************************************************/
/**
 * @brief Gets the selected PLL (Phase-Locked Loop) clock to the desired frequency.
 * 
 * @param[in] pll_type Enum specifying the type of PLL to configure.
 * 
 * @return uint32_t PLL frequency value in MHz.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
uint32_t sl_si91x_clock_manager_get_pll_freq(PLL_TYPE_T pll_type)
{
  uint32_t pll_freq = 0;

  switch (pll_type) {
    case SOC_PLL:
      // return currently configured SOC PLL frequency
      pll_freq = system_clocks.soc_clock;
      break;

    case INTF_PLL:
      // return currently configured INTF PLL frequency
      pll_freq = system_clocks.intf_pll_clock;
      break;

    case I2S_PLL:
      // return currently configured I2S PLL frequency
      pll_freq = system_clocks.i2s_pll_clock;
      break;

    default:
      break;
  }

  return pll_freq;
}
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
sl_status_t sl_si91x_clock_manager_control_pll(PLL_TYPE_T pll_type, bool enable)
{
  sl_status_t status = SL_STATUS_OK;

  switch (pll_type) {
    case SOC_PLL:
      // Turn On/Off the SOC PLL
      enable ? RSI_CLK_SocPllTurnOn() : RSI_CLK_SocPllTurnOff();
      break;

    case INTF_PLL:
      // Turn On/Off the INTF PLL
      enable ? RSI_CLK_IntfPLLTurnOn() : RSI_CLK_IntfPLLTurnOff();
      break;

    case I2S_PLL:
      // Turn On/Off the I2S PLL
      enable ? RSI_CLK_I2sPllTurnOn() : RSI_CLK_I2sPllTurnOff();
      break;

    default:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  return status;
}
/*******************************************************************************
 * To validate the RSI error code
 * While calling the RSI APIs, it returns the RSI Error codes.
 * This function converts the RSI error codes into SL error codes.
 * It takes argument as RSI error type and returns the SL error type.
 * It has a single switch statement which is mapped with the SL error code and
 * after successful conversion it breaks the switch statement.
 * If the error code is not listed, by default is SL_STATUS_FAIL.
 ******************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error)
{
  sl_status_t status;
  switch (error) {
    case RSI_OK:
      status = SL_STATUS_OK;
      break;
    case INVALID_PARAMETERS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_INVALID_INPUT_FREQUENCY:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_CLOCK_NOT_ENABLED:
      status = SL_STATUS_NOT_INITIALIZED;
      break;
    case INVALID_SAMPLING_RATE:
      status = SL_STATUS_INVALID_RANGE;
      break;
    case INVALID_SAMPLE_LENGTH:
      status = SL_STATUS_INVALID_COUNT;
      break;
    case ERROR_ADC_INVALID_ARG:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_PS_INVALID_STATE:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }
  return status;
}

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
 ***************************************************************************/
void sl_si91x_delay_ms(uint32_t milli_seconds)
{
  extern uint32_t SystemCoreClock;                                   // Get the system clock frequency
  uint32_t cycles_per_ms = SystemCoreClock / MILLISECONDS_TO_CYCLES; // Calculate cycles for ms

  // Calibrate the delay based on the system clock frequency
  if (SystemCoreClock < CLOCK_THRESHOLD) {                                   // If SystemCoreClock < 120 MHz
    cycles_per_ms = (cycles_per_ms * milli_seconds) / LOW_FREQ_CLK_DIV_FAC;  // Use division factor 6
  } else {                                                                   // If SystemCoreClock >= 120 MHz
    cycles_per_ms = (cycles_per_ms * milli_seconds) / HIGH_FREQ_CLK_DIV_FAC; // Use division factor 12
  }

  // Delay loop using NOP instructions
  while (cycles_per_ms--) {
    __NOP(); // No operation instruction
  }
}
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
 * @note    This API should be invoked only when the system is in PS2 state, as the source clock in this mode is configured to use a 20 MHz RC
 *
 ******************************************************************************/
sl_status_t sl_si91x_clock_manager_ulp_processor_clk_division(uint8_t clk_div)
{
  sl_status_t status = SL_STATUS_OK;
  if (clk_div > ULP_PROC_MAX_CLK_DIV_FAC) {
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }
  ULPCLK->ULP_TA_CLK_GEN_REG_b.ULP_PROC_CLK_DIV_FACTOR = clk_div;
  return status;
}