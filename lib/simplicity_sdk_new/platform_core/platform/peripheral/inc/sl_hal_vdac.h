/***************************************************************************//**
 * @file
 * @brief Digital to Analog Converter (VDAC) peripheral API
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

#ifndef SL_HAL_VDAC_H
#define SL_HAL_VDAC_H

#include "em_device.h"
#include "sl_enum.h"

#if defined(VDAC_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_assert.h"
#include <stdbool.h>

/***************************************************************************//**
 * @addtogroup vdac VDAC - Digital to Analog Voltage Converter.
 * @brief Digital to Analog Voltage Converter (VDAC) Peripheral API.
 * @details This module contains functions to control the VDAC peripheral of Silicon
 *          Labs' 32-bit MCUs and SoCs. VDAC converts digital values to analog
 *          signals at up to 500 ksps with 12-bit accuracy. VDAC is designed for
 *          low energy consumption, but can also provide very good performance.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/// VDAC instance validation.
#define SL_HAL_VDAC_REF_VALID(ref)    (VDAC_NUM(ref) != -1)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// VDAC channel.
SL_ENUM_GENERIC(sl_hal_vdac_channel_t, uint8_t) {
  SL_HAL_VDAC_CHANNEL_0 = 0,   ///< Channel 0 of VDAC.
  SL_HAL_VDAC_CHANNEL_1 = 1    ///< Channel 1 of VDAC.
};

/// Channel refresh period.
SL_ENUM(sl_hal_vdac_refresh_t) {
  SL_HAL_VDAC_REFRESH_CYCLES_2    = _VDAC_CFG_REFRESHPERIOD_CYCLES2,    ///< Refresh every 2 clock cycles.
  SL_HAL_VDAC_REFRESH_CYCLES_4    = _VDAC_CFG_REFRESHPERIOD_CYCLES4,    ///< Refresh every 4 clock cycles.
  SL_HAL_VDAC_REFRESH_CYCLES_8    = _VDAC_CFG_REFRESHPERIOD_CYCLES8,    ///< Refresh every 8 clock cycles.
  SL_HAL_VDAC_REFRESH_CYCLES_16   = _VDAC_CFG_REFRESHPERIOD_CYCLES16,   ///< Refresh every 16 clock cycles.
  SL_HAL_VDAC_REFRESH_CYCLES_32   = _VDAC_CFG_REFRESHPERIOD_CYCLES32,   ///< Refresh every 32 clock cycles.
  SL_HAL_VDAC_REFRESH_CYCLES_64   = _VDAC_CFG_REFRESHPERIOD_CYCLES64,   ///< Refresh every 64 clock cycles.
  SL_HAL_VDAC_REFRESH_CYCLES_128  = _VDAC_CFG_REFRESHPERIOD_CYCLES128,  ///< Refresh every 128 clock cycles.
  SL_HAL_VDAC_REFRESH_CYCLES_256  = _VDAC_CFG_REFRESHPERIOD_CYCLES256   ///< Refresh every 256 clock cycles.
};

/// Timer overflow period.
SL_ENUM(sl_hal_vdac_timer_overflow_period_t) {
  SL_HAL_VDAC_OVERFLOW_CYCLE_2  = _VDAC_CFG_TIMEROVRFLOWPERIOD_CYCLES2,    ///< Overflows every 2 clock cycles.
  SL_HAL_VDAC_OVERFLOW_CYCLE_4  = _VDAC_CFG_TIMEROVRFLOWPERIOD_CYCLES4,    ///< Overflows every 4 clock cycles.
  SL_HAL_VDAC_OVERFLOW_CYCLE_8  = _VDAC_CFG_TIMEROVRFLOWPERIOD_CYCLES8,    ///< Overflows every 8 clock cycles.
  SL_HAL_VDAC_OVERFLOW_CYCLE_16 = _VDAC_CFG_TIMEROVRFLOWPERIOD_CYCLES16,   ///< Overflows every 16 clock cycles.
  SL_HAL_VDAC_OVERFLOW_CYCLE_32 = _VDAC_CFG_TIMEROVRFLOWPERIOD_CYCLES32,   ///< Overflows every 32 clock cycles.
  SL_HAL_VDAC_OVERFLOW_CYCLE_64 = _VDAC_CFG_TIMEROVRFLOWPERIOD_CYCLES64    ///< Overflows every 64 clock cycles.
};

/// Reference voltage for VDAC.
SL_ENUM(sl_hal_vdac_vref_t) {
  SL_HAL_VDAC_VREF_1V25         = _VDAC_CFG_REFRSEL_V125,   ///< Internal 1.25 V band gap reference.
  SL_HAL_VDAC_VREF_2V5          = _VDAC_CFG_REFRSEL_V25,    ///< Internal 2.5 V band gap reference.
  SL_HAL_VDAC_VREF_AVDD         = _VDAC_CFG_REFRSEL_VDD,    ///< AVDD reference.
  SL_HAL_VDAC_VREF_EXTERNAL_PIN = _VDAC_CFG_REFRSEL_EXT     ///< External pin reference.
};

/// Refresh source for VDAC.
SL_ENUM(sl_hal_vdac_refresh_source_t) {
  SL_HAL_VDAC_REFRESH_NONE         = _VDAC_CH0CFG_REFRESHSOURCE_NONE,           ///< No refresh source.
  SL_HAL_VDAC_REFRESH_SOURCE_TIMER = _VDAC_CH0CFG_REFRESHSOURCE_REFRESHTIMER,   ///< Refresh triggered by refresh timer overflow.
  SL_HAL_VDAC_REFRESH_SYNC_PRS     = _VDAC_CH0CFG_REFRESHSOURCE_SYNCPRS,        ///< Refresh triggered by sync PRS.
  SL_HAL_VDAC_REFRESH_ASYNC_PRS    = _VDAC_CH0CFG_REFRESHSOURCE_ASYNCPRS        ///< Refresh triggered by async PRS.
};

/// Channel conversion trigger mode.
SL_ENUM(sl_hal_vdac_trigger_mode_t) {
  SL_HAL_VDAC_TRIGGER_MODE_NONE           = _VDAC_CH0CFG_TRIGMODE_NONE,           ///< No conversion trigger source selected.
  SL_HAL_VDAC_TRIGGER_MODE_SW             = _VDAC_CH0CFG_TRIGMODE_SW,             ///< Channel is triggered by CHnDATA or COMBDATA write.
  SL_HAL_VDAC_TRIGGER_MODE_SYNC_PRS       = _VDAC_CH0CFG_TRIGMODE_SYNCPRS,        ///< Channel is triggered by Sync PRS input.
#if defined(LESENSE_PRESENT)
  SL_HAL_VDAC_TRIGGER_MODE_LESENSE        = _VDAC_CH0CFG_TRIGMODE_LESENSE,        ///< Channel is triggered by LESENSE.
#endif
  SL_HAL_VDAC_TRIGGER_MODE_INTERNAL_TIMER = _VDAC_CH0CFG_TRIGMODE_INTERNALTIMER,  ///< Channel is triggered by Internal Timer.
  SL_HAL_VDAC_TRIGGER_MODE_ASYNC_PRS      = _VDAC_CH0CFG_TRIGMODE_ASYNCPRS        ///< Channel is triggered by Async PRS input.
};

/// Channel power mode.
SL_ENUM(sl_hal_vdac_power_mode_t) {
  SL_HAL_VDAC_POWER_MODE_HIGH_POWER    = _VDAC_CH0CFG_POWERMODE_HIGHPOWER,   ///< High power buffer mode.
  SL_HAL_VDAC_POWER_MODE_LOW_POWER     = _VDAC_CH0CFG_POWERMODE_LOWPOWER     ///< Low power buffer mode.
};

/// VDAC channel Abus port selection.
SL_ENUM(sl_hal_vdac_port_selection_t) {
  SL_HAL_VDAC_PORT_NONE    = _VDAC_OUTCTRL_ABUSPORTSELCH0_NONE,    ///< No GPIO selected.
  SL_HAL_VDAC_PORT_A       = _VDAC_OUTCTRL_ABUSPORTSELCH0_PORTA,   ///< Port A selected.
  SL_HAL_VDAC_PORT_B       = _VDAC_OUTCTRL_ABUSPORTSELCH0_PORTB,   ///< Port B selected.
  SL_HAL_VDAC_PORT_C       = _VDAC_OUTCTRL_ABUSPORTSELCH0_PORTC,   ///< Port C selected.
  SL_HAL_VDAC_PORT_D       = _VDAC_OUTCTRL_ABUSPORTSELCH0_PORTD    ///< Port D selected.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// VDAC initialization structure, common for both channels.
typedef struct {
  uint32_t                             warmup_time;            ///< Number of prescaled CLK_DAC + 1 for the VDAC to warmup.
  bool                                 debug_halt;             ///< Halt during debug.
  bool                                 on_demand_clk;          ///< Always allow clk_dac.
  bool                                 dma_wakeup;             ///< DMA Wakeup.
  bool                                 bias_keep_warm;         ///< Bias keep warm enable.
  sl_hal_vdac_refresh_t                refresh;                ///< Channel refresh period.
  sl_hal_vdac_timer_overflow_period_t  timer_overflow;         ///< Internal timer overflow period.
  uint32_t                             prescaler;              ///< Prescaler for VDAC clock. Clock is source clock divided by prescaler+1.
  sl_hal_vdac_vref_t                   reference;              ///< Reference voltage to use.
  bool                                 ch0_reset_prescaler;    ///< Enable/disable reset of prescaler on CH 0 start.
  bool                                 sine_reset;             ///< Sine reset mode.
  bool                                 sine_enable;            ///< Enable/disable sine mode.
  bool                                 diff;                   ///< Select if single ended or differential output mode.
#if defined(VDAC_CFG_SINEMODEPRS)
  bool                                 sine_mode_prs_enable;   ///< PRS controlled sinemode enable.
#endif
#if defined(VDAC_CFG_OUTENPRS)
  bool                                 prs_output_enable;      ///< PRS controlled channel output enable.
#endif
} sl_hal_vdac_init_t;

/// VDAC channel initialization structure.
typedef struct {
  bool                            warmup_keep_on;           ///< Warm-up mode, keep VDAC on (in idle) - or shutdown between conversions.
  bool                            high_cap_load_enable;     ///< Select high capacitance load mode in conjunction with high power.
  uint32_t                        fifo_low_data_threshold;  ///< Channel x FIFO Low threshold data valid level.
  sl_hal_vdac_refresh_source_t    ch_refresh_source;        ///< Channel refresh source.
  sl_hal_vdac_trigger_mode_t      trigger_mode;             ///< Channel conversion trigger mode.
  sl_hal_vdac_power_mode_t        power_mode;               ///< Channel power mode.
  bool                            sample_off_mode;          ///< Set channel conversion mode to sample/shut-off mode. Default is continuous.
  uint32_t                        pin;                      ///< Vdac channel output pin.
  sl_hal_vdac_port_selection_t    port;                     ///< Vdac channel output port.
  bool                            short_output;             ///< Short High power and low power output.
  bool                            aux_out_enable;           ///< Alternative output enable.
  bool                            main_out_enable;          ///< Main output enable.
  uint32_t                        hold_out_time;            ///< Channel output hold time.
} sl_hal_vdac_init_channel_t;

/// Default configuration for VDAC initialization structure.
#if defined(VDAC_CFG_SINEMODEPRS)
#define SL_HAL_VDAC_INIT_DEFAULT                                                                      \
  {                                                                                                   \
    _VDAC_CFG_WARMUPTIME_DEFAULT, /* Number of prescaled DAC_CLK for VDAC to warmup. */               \
    false,                        /* Continue while debugging. */                                     \
    false,                        /* On demand clock. */                                              \
    false,                        /* DMA wake up. */                                                  \
    false,                        /* Bias keep warm. */                                               \
    SL_HAL_VDAC_REFRESH_CYCLES_2, /* Refresh every 2nd cycle. */                                      \
    SL_HAL_VDAC_OVERFLOW_CYCLE_2, /* Internal overflow every 2nd cycle. */                            \
    0,                            /* No prescaling. */                                                \
    SL_HAL_VDAC_VREF_1V25,        /* 1.25 V internal low noise reference. */                          \
    false,                        /* Do not reset prescaler on CH 0 start. */                         \
    false,                        /* Sine wave is stopped at the sample it's currently outputting. */ \
    false,                        /* Disable sine mode. */                                            \
    false,                        /* Differential mode. */                                            \
    false,                        /* PRS controlled sinemode. */                                      \
    false,                        /* PRS controlled output enable. */                                 \
  }
#else
#define SL_HAL_VDAC_INIT_DEFAULT                                                                      \
  {                                                                                                   \
    _VDAC_CFG_WARMUPTIME_DEFAULT, /* Number of prescaled DAC_CLK for VDAC to warmup. */               \
    false,                        /* Continue while debugging. */                                     \
    false,                        /* On demand clock. */                                              \
    false,                        /* DMA wake up. */                                                  \
    false,                        /* Bias keep warm. */                                               \
    SL_HAL_VDAC_REFRESH_CYCLES_2, /* Refresh every 2nd cycle. */                                      \
    SL_HAL_VDAC_OVERFLOW_CYCLE_2, /* Internal overflow every 2nd cycle. */                            \
    0,                            /* No prescaling. */                                                \
    SL_HAL_VDAC_VREF_1V25,        /* 1.25 V internal low noise reference. */                          \
    false,                        /* Do not reset prescaler on CH 0 start. */                         \
    false,                        /* Sine wave is stopped at the sample it's currently outputting. */ \
    false,                        /* Disable sine mode. */                                            \
    false,                        /* Differential mode. */                                            \
  }
#endif

/// Sine mode configuration for VDAC initialization structure.
#if defined(VDAC_CFG_SINEMODEPRS)
#define SL_HAL_VDAC_INIT_SINE_GENERATION_MODE                                                         \
  {                                                                                                   \
    _VDAC_CFG_WARMUPTIME_DEFAULT, /* Number of prescaled DAC_CLK for VDAC to warmup.  */              \
    false,                        /* Continue while debugging. */                                     \
    true,                         /* On demand clock. */                                              \
    false,                        /* DMA wake up. */                                                  \
    false,                        /* Bias keep warm. */                                               \
    SL_HAL_VDAC_REFRESH_CYCLES_8, /* Refresh every 8th cycle. */                                      \
    SL_HAL_VDAC_OVERFLOW_CYCLE_2, /* Internal overflow every 2nd cycle. */                            \
    0,                            /* No prescaling. */                                                \
    SL_HAL_VDAC_VREF_1V25,        /* 1.25 V internal low noise reference. */                          \
    false,                        /* Do not reset prescaler on CH 0 start. */                         \
    false,                        /* Sine wave is stopped at the sample it's currently outputting. */ \
    true,                         /* Enable sine mode. */                                             \
    false,                        /* Differential mode. */                                            \
    false,                        /* PRS controlled sinemode. */                                      \
    false,                        /* PRS controlled output enable. */                                 \
  }
#else
#define SL_HAL_VDAC_INIT_SINE_GENERATION_MODE                                                         \
  {                                                                                                   \
    _VDAC_CFG_WARMUPTIME_DEFAULT, /* Number of prescaled DAC_CLK for VDAC to warmup. */               \
    false,                        /* Continue while debugging. */                                     \
    true,                         /* On demand clock. */                                              \
    false,                        /* DMA wake up. */                                                  \
    false,                        /* Bias keep warm. */                                               \
    SL_HAL_VDAC_REFRESH_CYCLES_8, /* Refresh every 8th cycle. */                                      \
    SL_HAL_VDAC_OVERFLOW_CYCLE_2, /* Internal overflow every 2nd cycle. */                            \
    0,                            /* No prescaling. */                                                \
    SL_HAL_VDAC_VREF_1V25,        /* 1.25 V internal low noise reference. */                          \
    false,                        /* Do not reset prescaler on CH 0 start. */                         \
    false,                        /* Sine wave is stopped at the sample it's currently outputting. */ \
    true,                         /* Enable sine mode. */                                             \
    false,                        /* Differential mode. */                                            \
  }
#endif

/// Default configuration for VDAC channel initialization structure.
#define SL_HAL_VDAC_CHANNEL_INIT_DEFAULT                                                        \
  {                                                                                             \
    false,                             /* Turn off between sample off conversions. */           \
    true,                              /* Enable High cap mode. */                              \
    0,                                 /* FIFO data low watermark at 0. */                      \
    SL_HAL_VDAC_REFRESH_NONE,          /* Channel refresh source. */                            \
    SL_HAL_VDAC_TRIGGER_MODE_SW,       /* Conversion triggered by CH0DATA or COMBDATA write. */ \
    SL_HAL_VDAC_POWER_MODE_HIGH_POWER, /* High power mode enabled. */                           \
    false,                             /* Continuous conversion mode. */                        \
    0,                                 /* ABUS pin selected. */                                 \
    SL_HAL_VDAC_PORT_NONE,             /* No Analog bus port selected. */                       \
    false,                             /* Output not shorted. */                                \
    false,                             /* Alternative output disabled. */                       \
    true,                              /* Main output enabled. */                               \
    0,                                 /* Hold out time. Previously called settle time. */      \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize VDAC.
 *
 * @details
 *   Initializes the common parts for both channels. This function will
 *   also load calibration values from the Device Information (DI) page
 *   into the VDAC calibration register.
 *   To complete a VDAC setup, channel control configuration must also
 *   be done. See @ref sl_hal_vdac_init_channel().
 *
 * @note
 *   This function will disable both channels prior to configuration.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] init
 *   A pointer to the VDAC initialization structure.
 ******************************************************************************/
void sl_hal_vdac_init(VDAC_TypeDef *vdac,
                      const sl_hal_vdac_init_t *init);

/***************************************************************************//**
 * @brief
 *   Initialize a VDAC channel.
 *
 * @details
 *   Configures a specific VDAC channel with the provided settings.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] init
 *   A pointer to the VDAC channel initialization structure.
 *
 * @param[in] channel
 *   A channel number to initialize.
 ******************************************************************************/
void sl_hal_vdac_init_channel(VDAC_TypeDef *vdac,
                              const sl_hal_vdac_init_channel_t *init,
                              uint32_t channel);

/***************************************************************************//**
 * @brief
 *   Enable the VDAC channel.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] channel
 *   A channel to enable.
 ******************************************************************************/
void sl_hal_vdac_enable_channel(VDAC_TypeDef *vdac,
                                uint32_t channel);

/***************************************************************************//**
 * @brief
 *   Disable the VDAC channel.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] channel
 *   A channel to disable.
 ******************************************************************************/
void sl_hal_vdac_disable_channel(VDAC_TypeDef *vdac,
                                 uint32_t channel);

/***************************************************************************//**
 * @brief
 *   Set the output signal of a VDAC channel to a given value.
 *
 * @details
 *   This function sets the output signal of a VDAC channel by writing
 *   @p value to the corresponding CHnDATA register.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] channel
 *   A channel number to set the output of.
 *
 * @param[in] value
 *   A value to write to the channel output register CHnDATA.
 ******************************************************************************/
void sl_hal_vdac_set_output_channel(VDAC_TypeDef *vdac,
                                    uint32_t channel,
                                    uint32_t value);

/***************************************************************************//**
 * @brief
 *   Calculate the prescaler value used to determine VDAC clock.
 *
 * @details
 *   The VDAC clock is given by the input clock divided by the prescaler+1.
 *   VDAC_CLK = IN_CLK / (prescale + 1)
 *   The maximum VDAC clock is 1 MHz.
 *
 * @note
 *   If the requested VDAC frequency is low and the maximum prescaler value
 *   can't adjust the actual VDAC frequency lower than requested, the maximum
 *   prescaler value is returned resulting in a higher VDAC frequency than
 *   requested.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] vdac_frequency
 *   VDAC frequency target. The frequency will automatically be adjusted to
 *   be below the maximum allowed VDAC clock.
 *
 * @param[in] reference_frequency
 *   Frequency in Hz of the reference VDAC.
 *
 * @return
 *   A prescaler value to use for VDAC to achieve a clock value less than
 *   or equal to @p vdac_frequency.
 ******************************************************************************/
uint32_t sl_hal_vdac_calculate_prescaler(VDAC_TypeDef *vdac,
                                         uint32_t vdac_frequency,
                                         uint32_t reference_frequency);

/***************************************************************************//**
 * @brief
 *   Reset VDAC to its default state.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 ******************************************************************************/
void sl_hal_vdac_reset(VDAC_TypeDef *vdac);

/***************************************************************************//**
 * @brief
 *   Wait for the vdac to complete all synchronization of register changes
 *   and commands.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_vdac_wait_sync(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  while (vdac->STATUS & VDAC_STATUS_SYNCBUSY) {
    // Wait for synchronization to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Wait for the vdac to be completely disabled.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_vdac_wait_ready(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  while (vdac->EN & _VDAC_EN_DISABLING_MASK) {
    // Wait for the VDAC to be completely disabled.
  }
}

/***************************************************************************//**
 * @brief
 *   Enables the VDAC module.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_vdac_enable(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  if (vdac->EN & VDAC_EN_EN) {
    // If the VDAC is already enabled, return.
    return;
  }

  vdac->EN_SET = VDAC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disables the VDAC module and ensures all operations are synchronized.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_vdac_disable(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  sl_hal_vdac_wait_sync(vdac);
  vdac->EN_CLR = VDAC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more VDAC interrupts.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] flags
 *   VDAC interrupt sources to enable. Use a bitwise logic OR
 *   combination of valid interrupt flags for the VDAC module
 *   (VDAC_IF_nnn).
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   @ref sl_hal_vdac_clear_interrupts() prior to enabling the interrupt.
 ******************************************************************************/
__INLINE void sl_hal_vdac_enable_interrupts(VDAC_TypeDef *vdac,
                                            uint32_t flags)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  vdac->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more VDAC interrupts.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] flags
 *   VDAC interrupt sources to disable. Use a bitwise logic OR
 *   combination of valid interrupt flags for the VDAC module
 *   (VDAC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_vdac_disable_interrupts(VDAC_TypeDef *vdac,
                                             uint32_t flags)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  vdac->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending VDAC interrupts from SW.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] flags
 *   VDAC interrupt sources to set to pending. Use a bitwise
 *   logic OR combination of valid interrupt flags for the VDAC
 *   module (VDAC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_vdac_set_interrupts(VDAC_TypeDef *vdac,
                                         uint32_t flags)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  vdac->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending VDAC interrupts.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @param[in] flags
 *   Pending VDAC interrupt source to clear. Use a bitwise logic
 *   OR combination of valid interrupt flags for the VDAC module
 *   (VDAC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_vdac_clear_interrupts(VDAC_TypeDef *vdac,
                                           uint32_t flags)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  vdac->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending VDAC interrupt flags.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @return
 *   VDAC interrupt sources pending. Use a bitwise logic OR combination
 *   of valid interrupt flags for the VDAC module (VDAC_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_vdac_get_pending_interrupts(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  return vdac->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending VDAC interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @return
 *   Pending and enabled VDAC interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in VDACx_IEN_nnn
 *   register (VDACx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the VDAC module
 *   (VDACx_IF_nnn).
 *
 * @note
 *   Interrupt flags are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_vdac_get_enabled_pending_interrupts(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  uint32_t ien = vdac->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return vdac->IF & ien;
}

/***************************************************************************//**
 * @brief
 *   Start Sine mode.
 *
 * @details
 *   This function sends the sine mode start signal to the DAC.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_vdac_sine_mode_start(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  sl_hal_vdac_wait_sync(vdac);
  // Start sine mode.
  vdac->CMD_SET = VDAC_CMD_SINEMODESTART;
  while (0UL == (vdac->STATUS & VDAC_STATUS_SINEACTIVE)) {
    // Wait for sine mode to be active.
  }
}

/***************************************************************************//**
 * @brief
 *   Stop Sine mode.
 *
 * @details
 *   This function sends the sine mode stop signal to the DAC.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_vdac_sine_mode_stop(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  sl_hal_vdac_wait_sync(vdac);
  // Stop sine mode.
  vdac->CMD_SET = VDAC_CMD_SINEMODESTOP;
  while (0UL != (vdac->STATUS & VDAC_STATUS_SINEACTIVE)) {
    // Wait for sine mode to be inactive.
  }
}

/***************************************************************************//**
 * @brief
 *   Get the VDAC Status register.
 *
 * @param[in] vdac
 *   A pointer to the VDAC peripheral register block.
 *
 * @return
 *   Current STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_vdac_get_status(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  return vdac->STATUS;
}

/** @} (end addtogroup vdac) */

#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup vdac VDAC - Voltage DAC
 * Digital to Analog Voltage Converter (VDAC) Peripheral API
 * @{
 *
 * The following steps are necessary for basic operation:
 *
 * Clock enable:
 * @code
 * sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_VDAC0);
 * @endcode
 *
 * Initialize the VDAC with default settings and modify selected fields:
 * @code
 * sl_hal_vdac_init_t vdac_init                 = SL_HAL_VDAC_INIT_DEFAULT;
 * sl_hal_vdac_init_channel_t vdac_channel_init = SL_HAL_VDAC_CHANNEL_INIT_DEFAULT;
 * @endcode
 *
 * Set prescaler to get 1 MHz VDAC clock frequency.
 * Calculate clock frequency used for VDAC.
 * @code
 * sl_clock_branch_t clock_branch;
 * uint32_t src_clk_freq;
 * clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_VDAC0);
 *
 * sl_clock_manager_get_clock_branch_frequency(clock_branch, &src_clk_freq);
 * vdac_init.prescaler = sl_hal_vdac_calculate_prescaler(VDAC0, 1000000, src_clk_freq);
 *
 * sl_hal_vdac_init(VDAC0, &vdac_init);
 * sl_hal_vdac_init_channel(VDAC0, &vdac_channel_init, 0);
 * sl_hal_vdac_enable_channel(VDAC0, 0);
 * @endcode
 *
 * Perform a conversion:
 * @code
 * sl_hal_vdac_set_output_channel(VDAC0, 0, 250);
 * @endcode
 *
 * @} (end addtogroup vdac)
 ******************************************************************************/

#endif /* defined(VDAC_PRESENT) */
#endif /* SL_HAL_VDAC_H */
