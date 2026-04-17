/***************************************************************************//**
 * @file
 * @brief Analog Joystick Driver
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_joystick.h"
#include "em_device.h"
#if defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_hal_adc.h"
#include "sl_device_peripheral.h"
#include "sl_device_clock.h"
#else
#include "em_iadc.h"
#endif
#include "sl_clock_manager.h"
#include "sl_status.h"

/***************************************************************************//**
 * An internal local function used for IADC BUS Allocation. It parses through
 * all bus options and allocates a bus to ADC0 if available.
 *
 * @note If no bus is available to be allocated, the function returns
 * SL_STATUS_ALLOCATION_FAILED
 *
 * @param[in] port          Analog Joystick GPIO port
 *
 * @param[in] pin           Analog Joystick GPIO pin
 *
 ******************************************************************************/
static sl_status_t _joystick_IADC_busAllocation(sl_gpio_port_t port, uint8_t pin)
{
  sl_status_t status = SL_STATUS_OK;
  switch (port) {
#if (GPIO_PA_COUNT > 0)
    case SL_GPIO_PORT_A:

      if (0 == pin % 2) {
        if ((GPIO->ABUSALLOC & _GPIO_ABUSALLOC_AEVEN0_MASK) == GPIO_ABUSALLOC_AEVEN0_TRISTATE) {
          GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AEVEN0_ADC0;
        } else if ((GPIO->ABUSALLOC & _GPIO_ABUSALLOC_AEVEN1_MASK) == GPIO_ABUSALLOC_AEVEN1_TRISTATE) {
          GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AEVEN1_ADC0;
        } else {
          status = SL_STATUS_ALLOCATION_FAILED;
        }
      } else {
        if ((GPIO->ABUSALLOC & _GPIO_ABUSALLOC_AODD0_MASK) == GPIO_ABUSALLOC_AODD0_TRISTATE) {
          GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AODD0_ADC0;
        } else if ((GPIO->ABUSALLOC & _GPIO_ABUSALLOC_AODD1_MASK) == GPIO_ABUSALLOC_AODD1_TRISTATE) {
          GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AODD1_ADC0;
        } else {
          status = SL_STATUS_ALLOCATION_FAILED;
        }
      }
      break;
#endif

#if (GPIO_PB_COUNT > 0)
    case SL_GPIO_PORT_B:

      if (0 == pin % 2) {
        if ((GPIO->BBUSALLOC & _GPIO_BBUSALLOC_BEVEN0_MASK) == GPIO_BBUSALLOC_BEVEN0_TRISTATE) {
          GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BEVEN0_ADC0;
        } else if ((GPIO->BBUSALLOC & _GPIO_BBUSALLOC_BEVEN1_MASK) == GPIO_BBUSALLOC_BEVEN1_TRISTATE) {
          GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BEVEN1_ADC0;
        } else {
          status = SL_STATUS_ALLOCATION_FAILED;
        }
      } else {
        if ((GPIO->BBUSALLOC & _GPIO_BBUSALLOC_BODD0_MASK) == GPIO_BBUSALLOC_BODD0_TRISTATE) {
          GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BODD0_ADC0;
        } else if ((GPIO->BBUSALLOC & _GPIO_BBUSALLOC_BODD1_MASK) == GPIO_BBUSALLOC_BODD1_TRISTATE) {
          GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BODD1_ADC0;
        } else {
          status = SL_STATUS_ALLOCATION_FAILED;
        }
      }
      break;
#endif

#if (GPIO_PC_COUNT > 0 || GPIO_PD_COUNT > 0)
    case SL_GPIO_PORT_C:
    case SL_GPIO_PORT_D:

      if (0 == pin % 2) {
        if ((GPIO->CDBUSALLOC & _GPIO_CDBUSALLOC_CDEVEN0_MASK) == GPIO_CDBUSALLOC_CDEVEN0_TRISTATE) {
          GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDEVEN0_ADC0;
        } else if ((GPIO->CDBUSALLOC & _GPIO_CDBUSALLOC_CDEVEN1_MASK) == GPIO_CDBUSALLOC_CDEVEN1_TRISTATE) {
          GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDEVEN1_ADC0;
        } else {
          status = SL_STATUS_ALLOCATION_FAILED;
        }
      } else {
        if ((GPIO->CDBUSALLOC & _GPIO_CDBUSALLOC_CDODD0_MASK) == GPIO_CDBUSALLOC_CDODD0_TRISTATE) {
          GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDODD0_ADC0;
        } else if ((GPIO->CDBUSALLOC & _GPIO_CDBUSALLOC_CDODD1_MASK) == GPIO_CDBUSALLOC_CDODD1_TRISTATE) {
          GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDODD1_ADC0;
        } else {
          status = SL_STATUS_ALLOCATION_FAILED;
        }
      }
      break;
#endif
  }
  return status;
}

/***************************************************************************//**
 * Initializes the Analog Joystick driver.
 ******************************************************************************/
sl_status_t sl_joystick_init(sl_joystick_t *joystick_handle)
{
  sl_status_t status = SL_STATUS_OK;

#if defined(_SILICON_LABS_32B_SERIES_2)
  // Declare initialization structures
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;

  // Single input structure
  IADC_SingleInput_t singleInput = IADC_SINGLEINPUT_DEFAULT;

  // Enable IADC0 and GPIO register clock.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  // Set timer cycles to configure sampling rate
  init.timerCycles = TIMER_CYCLES;

  // Shutdown between conversions to reduce current
  init.warmup = iadcWarmupNormal;

  // Set the prescaler needed for the intended IADC clock frequency
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  /*
   * Configuration 0 is used by both scan and single conversions by
   * default.  Use unbuffered AVDD as reference and specify the
   * AVDD supply voltage in mV.
   *
   * Resolution is not configurable directly but is based on the
   * selected oversampling ratio (osrHighSpeed), which defaults to
   * 2x and generates 12-bit results.
   */
  initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;
  initAllConfigs.configs[0].vRef = REFERENCE_VOLTAGE;
  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed2x;

  /*
   * CLK_SRC_ADC must be prescaled by some value greater than 1 to
   * derive the intended CLK_ADC frequency.
   */
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                     CLK_ADC_FREQ,
                                                                     0,
                                                                     iadcCfgModeNormal,
                                                                     init.srcClkPrescale);

  // Selection of Timer for IADC trigger action
  initSingle.triggerSelect = iadcTriggerSelTimer;

  /*
   * Specify the input channel.  The negInput = iadcNegInputGnd by
   * default
   */
  singleInput.posInput   = IADC_portPinToPosInput(joystick_handle->port,
                                                  joystick_handle->pin);

  // Enable triggering of single conversion
  initSingle.start = true;

  // Allocate the analog bus for ADC0 inputs
  status = _joystick_IADC_busAllocation(joystick_handle->port,
                                        joystick_handle->pin);
  if (SL_STATUS_OK != status) {
    return status;
  }

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize a single-channel conversion
  IADC_initSingle(IADC0, &initSingle, &singleInput);

#else
  // Declare initialization structures
  sl_hal_adc_init_t init = SL_HAL_ADC_INIT_DEFAULT;

  // Enable ADC0 and GPIO register clock.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  // Shutdown between conversions to reduce current
  init.warmup_mode = SL_HAL_ADC_WARMUP_NORMAL;

  // Allocate the analog bus for ADC0 inputs
  status = _joystick_IADC_busAllocation(joystick_handle->port,
                                        joystick_handle->pin);
  if (SL_STATUS_OK != status) {
    return status;
  }

  sl_clock_branch_t clock_branch;
  clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_ADC0);

  uint32_t branch_clock_freq;
  sl_clock_manager_get_clock_branch_frequency(clock_branch, &branch_clock_freq);

  // Initialize IADC
  sl_hal_adc_init(ADC0, &init, branch_clock_freq);

#endif
  return status;
}

/***************************************************************************//**
 * Starts Analog Joystick data acquisition.
 ******************************************************************************/
void sl_joystick_start(sl_joystick_t *joystick_handle)
{
  /* Check if already enabled */
  if (joystick_handle->state != SL_JOYSTICK_DISABLED) {
    return;
  }

  joystick_handle->state = SL_JOYSTICK_ENABLED;

#if defined(_SILICON_LABS_32B_SERIES_2)
  IADC_command(IADC0, iadcCmdEnableTimer);

  /*
   * To check at least one result in the single FIFO is
   * ready to be read
   */
  while (0 == (IADC0->STATUS_CLR & IADC_STATUS_SINGLEFIFODV)) ;
#else
  sl_hal_adc_enable_timer(ADC0);
  /* check how to handle above while loop*/
#endif
}

/***************************************************************************//**
 * Stops Analog Joystick data acquisition.
 ******************************************************************************/
void sl_joystick_stop(sl_joystick_t *joystick_handle)
{
  /* Check if already disabled */
  if (joystick_handle->state != SL_JOYSTICK_ENABLED) {
    return;
  }

  joystick_handle->state = SL_JOYSTICK_DISABLED;

#if defined(_SILICON_LABS_32B_SERIES_2)
  IADC_command(IADC0, iadcCmdDisableTimer);
#else
  sl_hal_adc_disable_timer(ADC0);
#endif
}

/***************************************************************************//**
 * Get joystick position.
 ******************************************************************************/
sl_status_t sl_joystick_get_position(sl_joystick_t *joystick_handle, sl_joystick_position_t *pos)
{
  sl_joystick_position_t joystickDirection;

  uint32_t singleResult;

  // Return error if joystick data acquisistion not started
  if (joystick_handle->state != SL_JOYSTICK_ENABLED) {
    return SL_STATUS_NOT_READY;
  }

#if defined(_SILICON_LABS_32B_SERIES_2)
  // Read most recent single conversion result
  IADC_Result_t sample = IADC_readSingleResult(IADC0);
#else
  sl_hal_adc_result_t sample = sl_hal_adc_peek(ADC0);
#endif

  /*
   * Calculate the voltage converted as follows:
   *
   * For single-ended conversions, the result can range from 0 to
   * +Vref, i.e., for Vref = AVDD = 3300 mV, 0xFFF represents the
   * full scale value of 3300 mV.
   */
  singleResult = (sample.data * REFERENCE_VOLTAGE) / 0xFFF;

  uint32_t mV = singleResult;

  // determine which direction pad was pressed
  if ((mV <= JOYSTICK_MV_C + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_C;
  } else if ((mV >= JOYSTICK_MV_N - JOYSTICK_MV_ERROR) \
             && (mV <= JOYSTICK_MV_N + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_N;
  } else if ((mV >= JOYSTICK_MV_E - JOYSTICK_MV_ERROR) \
             && (mV <= JOYSTICK_MV_E + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_E;
  } else if ((mV >= JOYSTICK_MV_S - JOYSTICK_MV_ERROR) \
             && (mV <= JOYSTICK_MV_S + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_S;
  } else if ((mV >= JOYSTICK_MV_W - JOYSTICK_MV_ERROR) \
             && (mV <= JOYSTICK_MV_W + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_W;
  }
#if ENABLE_SECONDARY_DIRECTIONS == 1
  else if ((mV >= JOYSTICK_MV_NE - JOYSTICK_MV_ERROR) \
           && (mV <= JOYSTICK_MV_NE + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_NE;
  } else if ((mV >= JOYSTICK_MV_NW - JOYSTICK_MV_ERROR) \
             && (mV <= JOYSTICK_MV_NW + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_NW;
  } else if ((mV >= JOYSTICK_MV_SE - JOYSTICK_MV_ERROR) \
             && (mV <= JOYSTICK_MV_SE + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_SE;
  } else if ((mV >= JOYSTICK_MV_SW - JOYSTICK_MV_ERROR) \
             && (mV <= JOYSTICK_MV_SW + JOYSTICK_MV_ERROR)) {
    joystickDirection = JOYSTICK_SW;
  }
#endif
  else {
    joystickDirection = JOYSTICK_NONE;
  }

  *pos = joystickDirection;

  return SL_STATUS_OK;
}
