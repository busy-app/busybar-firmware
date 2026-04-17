/***************************************************************************//**
 * @file
 * @brief Power Manager EM4 API implementation.
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

#include "sl_power_manager.h"
#include "sl_power_manager_config.h"
#include "sli_power_manager.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "em_device.h"
#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_emu.h"
#include "em_cmu.h"
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
#include "em_iadc.h"
#endif
#endif

#if defined(SL_CATALOG_METRIC_EM4_WAKE_PRESENT)
#include "sli_metric_em4_wake.h"
#endif

#include <stdlib.h>
#include <stdint.h>

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/

#if defined(WDOG_PRESENT)
// Macros to determine if WDOG instances are clocked or not

#if defined(CMU_CLKEN0_WDOG0)
#define WDOG0_CLOCK_ENABLED_BIT (CMU->CLKEN0 & CMU_CLKEN0_WDOG0)
#else
// There's no CMU->CLKEN1 so assume the WDOG0 is clocked
#define WDOG0_CLOCK_ENABLED_BIT 1
#endif

#if defined(CMU_CLKEN1_WDOG1)
#define WDOG1_CLOCK_ENABLED_BIT (CMU->CLKEN1 & CMU_CLKEN1_WDOG1)
#else
// There's no CMU->CLKEN1 so assume the WDOG1 is clocked
#define WDOG1_CLOCK_ENABLED_BIT 1
#endif

#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static bool is_em4_blocked(void);

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) && (SL_POWER_MANAGER_RAMP_DVDD_EN == 1)
static void ramp_dvdd_and_switch_to_dcdc_bypass_mode(void);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * update energy mode 4 configurations.
 ******************************************************************************/
void sli_power_manager_init_em4(void)
{
#if !defined(_SILICON_LABS_32B_SERIES_2)
  EMU->EM4CTRL = (EMU->EM4CTRL & ~_EMU_EM4CTRL_EM4IORETMODE_MASK)
                 | SL_POWER_MANAGER_INIT_EMU_EM4_PIN_RETENTION_MODE;
#else
  EMU_EM4Init_TypeDef em4_init = EMU_EM4INIT_DEFAULT;
  em4_init.pinRetentionMode = (EMU_EM4PinRetention_TypeDef)SL_POWER_MANAGER_INIT_EMU_EM4_PIN_RETENTION_MODE;
  EMU_EM4Init(&em4_init);
#endif
}

/******************************************************************************
 * Event called before entering EM4 sleep.
 *****************************************************************************/
SL_WEAK void sl_power_manager_em4_presleep_hook(void)
{
  // This implementation is empty, but this function can be redefined as it's a weak implementation.
}

/***************************************************************************//**
 * Enter energy mode 4 (EM4).
 *
 * @note  You should not expect to return from this function. Once the device
 *        enters EM4, only a power on reset or external reset pin can wake the
 *        device.
 *
 * @note  On xG22 devices, this function re-configures the IADC if EM4 entry
 *        is possible.
 ******************************************************************************/
__NO_RETURN void sl_power_manager_enter_em4(void)
{
  /* Device with Boost DC-DC cannot enter EM4 because Boost DC-DC module does not
   * have BYPASS switch so DC-DC converter can not be set to bypass mode. */
#if (defined(_SILICON_LABS_DCDC_FEATURE) \
  && (_SILICON_LABS_DCDC_FEATURE == _SILICON_LABS_DCDC_FEATURE_DCDC_BOOST))
  EFM_ASSERT(false);
#endif

  // Make sure that we are not interrupted while we are entering em4
  CORE_CRITICAL_IRQ_DISABLE();

  EFM_ASSERT(is_em4_blocked() == false);

#if defined(SL_CATALOG_METRIC_EM4_WAKE_PRESENT)
  sli_metric_em4_wake_init();
#endif

  uint32_t em4seq2 = (EMU->EM4CTRL & ~_EMU_EM4CTRL_EM4ENTRY_MASK)
                     | (2U << _EMU_EM4CTRL_EM4ENTRY_SHIFT);
  uint32_t em4seq3 = (EMU->EM4CTRL & ~_EMU_EM4CTRL_EM4ENTRY_MASK)
                     | (3U << _EMU_EM4CTRL_EM4ENTRY_SHIFT);

  // Make sure that the register write lock is disabled.
  EMU->LOCK = EMU_LOCK_LOCKKEY_UNLOCK;

#if defined(_DCDC_IF_EM4ERR_MASK)
  // Workaround for bug that may cause a Hard Fault on EM4 entry
  CMU_CLOCK_SELECT_SET(SYSCLK, FSRCO);
  // The buck DC-DC is available in all energy modes except for EM4.
  // The DC-DC converter must first be turned off and switched over to bypass mode.
#if (defined(EMU_SERIES2_DCDC_BUCK_PRESENT) \
  || defined(EMU_SERIES2_DCDC_BOOST_PRESENT))
  #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) && (SL_POWER_MANAGER_RAMP_DVDD_EN == 1)
  ramp_dvdd_and_switch_to_dcdc_bypass_mode();
  #else
  EMU_DCDCModeSet(emuDcdcMode_Bypass);
  #endif
#endif
#endif

  sl_power_manager_em4_presleep_hook();

  for (uint8_t i = 0; i < 4; i++) {
    EMU->EM4CTRL = em4seq2;
    EMU->EM4CTRL = em4seq3;
  }
  EMU->EM4CTRL = em4seq2;
  __DSB();
  __ISB();
  __WFI();

  for (;; ) {
    // __NO_RETURN
  }
}

/***************************************************************************//**
 *   When EM4 pin retention is set to power_manager_pin_retention_latch,
 *   then pins are retained through EM4 entry and wakeup. The pin state is
 *   released by calling this function. The feature allows peripherals or
 *   GPIO to be re-initialized after EM4 exit (reset), and when
 *   initialization is done, this function can release pins and return
 *   control to the peripherals or GPIO.
 ******************************************************************************/
void sl_power_manager_em4_unlatch_pin_retention(void)
{
#if defined(_EMU_EM4CTRL_EM4IORETMODE_MASK)
  EMU->CMD = EMU_CMD_EM4UNLATCH;
#endif
}

/***************************************************************************//**
 * Returns true if em4 entry is blocked by a watchdog peripheral.
 ******************************************************************************/
static bool is_em4_blocked(void)
{
#if defined(WDOG_PRESENT)
#if WDOG_COUNT > 0
  if ( WDOG0_CLOCK_ENABLED_BIT && (WDOG0->CFG & WDOG_CFG_EM4BLOCK) && (WDOG0->EN & WDOG_EN_EN) ) {
    return true;
  }
#endif
#if WDOG_COUNT > 1
  if ( WDOG1_CLOCK_ENABLED_BIT && (WDOG1->CFG & WDOG_CFG_EM4BLOCK) && (WDOG1->EN & WDOG_EN_EN) ) {
    return true;
  }
#endif
#endif
  return false;
}

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) && (SL_POWER_MANAGER_RAMP_DVDD_EN == 1)

/* The following section provides an optimization to improve peak current
 * consumption on xG22 devices.
 */

// ADC clock frequency (source and after prescale)
#define CLK_SRC_ADC_FREQ     9600000   // CLK_SRC_ADC
#define CLK_ADC_FREQ         1000000   // CLK_ADC

extern void sli_delay_loop(uint32_t cycles);

uint32_t * dcdc_test_addr = (uint32_t *)(DCDC_NS_BASE + 0x80);
uint32_t ipkval = 7;
uint32_t ipktimeout = 1;

/* Pulse generation sequence TOCTRIG (bit 3) TOCMODE (bit 2)*/
uint32_t cmd[2] = { (1 << 2) | (1 << 3), (1 << 2) };

/***************************************************************************//**
 * The voltage of Dvdd is ramped up to VMCU by sending pulses to a DCDC test
 * register. These pulses are delayed, and ipkval and ipktimeout are tuned
 * such that the voltage at Dvdd increases gradually to the voltage level of
 * VMCU. Using the IADC, once Dvdd has gotten sufficiently close to VMCU,
 * the DCDC peripheral is then switched into bypass mode. The IADC is used to
 * detect this by sampling the voltage of Dvdd periodically, and calculating the
 * difference between samples, when the difference is within some margin of zero
 * then we know that the ramp sequence has reached a plateau.
 ******************************************************************************/
static void ramp_dvdd_and_switch_to_dcdc_bypass_mode(void)
{
  // Initialize the IADC for the purposes of detecting when the Dvdd ramp
  // reaches a plateau.
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;
  CMU_ClockEnable(cmuClock_IADC0, true);

  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = 1210;
  initAllConfigs.configs[0].analogGain = iadcCfgAnalogGain1x;
  initAllConfigs.configs[0].digAvg = iadcDigitalAverage1;
  initAllConfigs.configs[0].adcClkPrescale =
    IADC_calcAdcClkPrescale(IADC0,
                            CLK_ADC_FREQ,
                            0,
                            iadcCfgModeNormal,
                            init.srcClkPrescale);
  init.warmup = iadcWarmupKeepWarm;

  IADC_reset(IADC0);
  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_EM01GRPACLK);
  initSingle.triggerAction = iadcTriggerActionContinuous;
  initSingle.alignment = iadcAlignRight12;
  initSingleInput.compare = false;                   // Disable Window CMP
  initSingleInput.posInput = iadcPosInputDvdd;
  IADC_init(IADC0, &init, &initAllConfigs);
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  // Start capturing
  IADC_command(IADC0, iadcCmdStartSingle);

  // Initialize DCDC peak current value and timeout to reach peak current value
  DCDC->EM01CTRL0 = (DCDC->EM01CTRL0 & ~_DCDC_EM01CTRL0_IPKVAL_MASK) | (ipkval << 0);
  DCDC->CTRL = (DCDC->CTRL & ~_DCDC_CTRL_IPKTMAXCTRL_MASK) | (ipktimeout << 4);

  /* Generate pulses */
  uint32_t iter = 1U;
  IADC_Result_t prev_result;
  volatile IADC_Result_t current_result = IADC_readSingleResult(IADC0);
  while (true) {
    // If the algorithm doesn't converge after 500 pulses, switch to dcdc
    // bypass anyways.
    if (iter >= 500) {
      DCDC->CTRL_CLR = DCDC_CTRL_MODE;
      EFM_ASSERT(false);
      return;
    }

    /* Pulse generation sequence TOCTRIG (bit 3) TOCMODE (bit 2)*/
    *dcdc_test_addr = cmd[0];
    *dcdc_test_addr = cmd[1];

    // In DCDC mode, MCU input voltage VREGVDD cannot be directly measured, so
    // we can't know what the target DVDD voltage is. Instead, since DVDD
    // ramp-up should follow a RC charge curve, measure DVDD and keep charging
    // until the delta between measures is smaller than the set tolerance.
    if (iter % 20U == 0U) {
      prev_result = current_result;
      current_result = IADC_readSingleResult(IADC0);
      if ( abs((int32_t)(current_result.data - prev_result.data)) < SL_POWER_MANAGER_RAMP_DVDD_TOLERANCE ) {
        DCDC->CTRL_CLR = DCDC_CTRL_MODE;
        return;
      }
    }

    if (DCDC->IF & DCDC_IF_TMAX) {
      if (ipkval) {
        ipkval--; // DCDC peak current value
      }

      if (ipktimeout < 7) {
        ipktimeout++; // Timeout to reach peak current value
      }

      DCDC->EM01CTRL0 = (DCDC->EM01CTRL0 & ~_DCDC_EM01CTRL0_IPKVAL_MASK) | (ipkval << 0);
      DCDC->CTRL = (DCDC->CTRL & ~_DCDC_CTRL_IPKTMAXCTRL_MASK) | (ipktimeout << 4);

      DCDC->IF_CLR = DCDC_IF_TMAX;
    }

    /* delay for 8 clock cycles */
    sli_delay_loop(8);
    iter++;
  }
}

#endif // defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) && (SL_POWER_MANAGER_RAMP_DVDD_EN == 1)
