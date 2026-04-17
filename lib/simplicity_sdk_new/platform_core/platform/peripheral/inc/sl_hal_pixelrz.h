/***************************************************************************//**
 * @file
 * @brief PIXELRZ peripheral API
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

#ifndef SL_HAL_PIXELRZ_H
#define SL_HAL_PIXELRZ_H

#include "em_device.h"
#if defined(PIXELRZ_COUNT) && (PIXELRZ_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_assert.h"
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup pixelrz PIXELRZ - Serial Pixel Interface
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   DEFINES   **********************************
 ******************************************************************************/

/// Check if PIXELRZ instance is valid.
#define SL_HAL_PIXELRZ_REF_VALID(pixelrz_ref)    (PIXELRZ_NUM(pixelrz_ref) != -1)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// PIXELRZ mode. Serial or parallel.
SL_ENUM(sl_hal_pixelrz_mode_t) {
  /// Sequential mode
  SL_HAL_PIXELRZ_MODE_SEQUENTIAL = _PIXELRZ_CFG_MODE_SEQUENTIAL,

  /// Parallel mode
  SL_HAL_PIXELRZ_MODE_PARALLEL = _PIXELRZ_CFG_MODE_PARALLEL
};

/// The frame trigger source.
SL_ENUM(sl_hal_pixelrz_trigger_t) {
  /// Software trigger mode.
  SL_HAL_PIXELRZ_TRIG_SW = _PIXELRZ_CFG_TRIGSEL_SW,

  /// Auto trigger mode.
  SL_HAL_PIXELRZ_TRIG_AUTO = _PIXELRZ_CFG_TRIGSEL_AUTO,

  /// Software trigger mode.
  SL_HAL_PIXELRZ_TRIG_PRS = _PIXELRZ_CFG_TRIGSEL_PRS
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// Low and high times of a symbol.
typedef struct {
  /// Low Time.
  uint16_t low_time;

  /// High Time.
  uint16_t high_time;
} sl_hal_pixelrz_symbol_t;

/// PIXELRZ initialization structure.
typedef struct {
  /// Set mode.
  sl_hal_pixelrz_mode_t mode;

  /// MSB is transmitted first when set.
  bool msb_first_enable;

  /// Memory alignment of data set to 0 when 8bit and 1 when 32bit boundary.
  bool memalign_32b_enable;

  /// Invert the output from the block.
  bool output_invert_enable;

  /// Jitter enable (used in parallel mode).
  bool jitter_enable;

  /// Enable sending stop symbol ( used in parallel mode).
  bool stop_symbol_enable;

  /// Enable sending reset symbol (used in serial mode).
  bool reset_symbol_enable;

  /// Trigger mode.
  sl_hal_pixelrz_trigger_t trigger_mode;

  /// Sets the watermark level for the TXDATA FIFO. If TXDATA FIFO has more free entries than this programmed level, DMA request will be asserted.
  uint8_t fifo_watermark_level;

  /// Sets the prescaler factor for the baud clock.  The baud clock will be divided by PRESCDIV+1. Must be set to 0 if not required.
  uint16_t prescaler;

  /// Indicates the number of pixels present in a frame that range from 1 to 512 pixel per frame.
  uint16_t pixel_number;

  /// The number of data bits in a single pixel. Range from 1 to 512 bits.
  uint16_t pixel_width;

  /// Indicates the number of trail data bits that will be present at the end of frame ends. Trail bits range from 0 to 256.
  uint16_t trail_data_width;

  /// Zero symbol high/low times.
  sl_hal_pixelrz_symbol_t zero_symbol;

  /// One symbol high/low times.
  sl_hal_pixelrz_symbol_t one_symbol;

  /// Stop symbol high/low times.
  sl_hal_pixelrz_symbol_t stop_symbol;

  /// Reset symbol high/low times.
  sl_hal_pixelrz_symbol_t reset_symbol;
} sl_hal_pixelrz_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_pixelrz_init_t sl_hal_pixelrz_config_t;
/** @endcond */

/// Default sequential configuration.
#define SL_HAL_PIXELRZ_SEQUENTIAL_INIT_DEFAULT                        \
  {                                                                   \
    SL_HAL_PIXELRZ_MODE_SEQUENTIAL, /* mode is sequential. */         \
    false,                          /* LSB first. */                  \
    false,                          /* Memory alignment of 8bits. */  \
    false,                          /* Do not invert output. */       \
    false,                          /* Jitter disabled. */            \
    false,                          /* Stop symbol disabled. */       \
    true,                           /* Reset symbol enabled. */       \
    SL_HAL_PIXELRZ_TRIG_SW,         /* Software trigger. */           \
    0,                              /* FIFO water mark is 0. */       \
    0,                              /* Prescaler is 0. */             \
    0,                              /* Pixel number is 0. */          \
    0,                              /* Pixel width is 0. */           \
    0,                              /* Trail data width is 0. */      \
    { 0, 0 },                       /* Zero symbol high/low times */  \
    { 0, 0 },                       /* One symbol high/low times */   \
    { 0, 0 },                       /* Stop symbol high/low times */  \
    { 0, 0 },                       /* Reset symbol high/low times */ \
  }

/// Default parallel configuration.
#define SL_HAL_PIXELRZ_PARALLEL_INIT_DEFAULT                        \
  {                                                                 \
    SL_HAL_PIXELRZ_MODE_PARALLEL, /* Mode is parallel. */           \
    false,                        /* LSB first. */                  \
    false,                        /* Memory alignment of 8bits. */  \
    false,                        /* Do not invert output. */       \
    false,                        /* Jitter disabled. */            \
    true,                         /* Stop symbol enabled. */        \
    false,                        /* Reset symbol disabled. */      \
    SL_HAL_PIXELRZ_TRIG_SW,       /* Software trigger. */           \
    0,                            /* FIFO water mark is 0. */       \
    0,                            /* Prescaler is 0. */             \
    0,                            /* Pixel number is 0. */          \
    0,                            /* Pixel width is 0. */           \
    0,                            /* Trail data width is 0. */      \
    { 0, 0 },                     /* Zero symbol high/low times */  \
    { 0, 0 },                     /* One symbol high/low times */   \
    { 0, 0 },                     /* Stop symbol high/low times */  \
    { 0, 0 },                     /* Reset symbol high/low times */ \
  }

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro name used for backward compatibility purposes.
#define SL_HAL_PIXELRZ_CONFIG_SEQUENTIAL_DEFAULT SL_HAL_PIXELRZ_SEQUENTIAL_INIT_DEFAULT
#define SL_HAL_PIXELRZ_CONFIG_PARALLEL_DEFAULT SL_HAL_PIXELRZ_PARALLEL_INIT_DEFAULT
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize PIXELRZ.
 *
 * @param[in] pixelrz
 *   A pointer to the PIXELRZ peripheral register block.
 *
 * @param[in] init
 *   A pointer to the PIXELRZ initialization structure.
 ******************************************************************************/
void sl_hal_pixelrz_init(PIXELRZ_TypeDef *pixelrz,
                         const sl_hal_pixelrz_init_t *init);

/***************************************************************************//**
 * @brief
 *   Get symbol from time settings. Check the datasheet for timing
 *   configuration.
 *
 * @param[in] time_high_us
 *   High time in microseconds.
 *
 * @param[in] time_low_us
 *   Low time in microseconds.
 *
 * @param[in] freq
 *   PIXELRZ source clock frequency.
 * @return
 *   Symbol high/low time values in clock cycles.
 ******************************************************************************/
sl_hal_pixelrz_symbol_t sl_hal_pixelrz_get_symbol_configuration(float time_high_us,
                                                                float time_low_us,
                                                                uint32_t freq);

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_wait_sync(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  while ((pixelrz->EN & _PIXELRZ_EN_EN_MASK) && (pixelrz->STATUS & _PIXELRZ_STATUS_SYNCBUSY_MASK)) {
    // Wait for any pending previous write operation to have been completed
    // in the low-frequency domain.
  }
}

/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_wait_ready(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  while (pixelrz->SWRST & _PIXELRZ_SWRST_RESETTING_MASK) {
    // Wait for reset to complete.
  }

  while (pixelrz->EN & _PIXELRZ_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Enable PIXELRZ.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_enable(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  // Enable peripheral.
  pixelrz->EN_SET = PIXELRZ_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable PIXELRZ.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_disable(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  // Disable PIXELRZ.
  pixelrz->EN_CLR = PIXELRZ_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Transmits one data word. Data will be written into FIFO.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @param[in] data
 *   Data to transmit.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_tx(PIXELRZ_TypeDef *pixelrz, uint32_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  while (pixelrz->STATUS & _PIXELRZ_STATUS_TXFULL_MASK) {
    // Check that transmit FIFO is not full.
  }

  pixelrz->TXDATA = data;
}

/***************************************************************************//**
 * @brief
 *   Enable PIXELRZ transmitter. The behaviour of this command will depend on the
 *   trigger configuration. If trigger is set to Software mode, this command will
 *   trigger transmission of one frame and stop, user must run this command for every
 *   frame. If trigger is set to Auto mode, the data will be transmitted whenever there's
 *   data in the FIFO and it will trigger transmission of all frames.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_enable_tx(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->CMD_SET = PIXELRZ_CMD_TXEN;
}

/***************************************************************************//**
 * @brief
 *   Disable PIXELRZ transmitter.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_disable_tx(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->CMD_SET = PIXELRZ_CMD_TXDIS;
}

/***************************************************************************//**
 * @brief
 *   Clear TX FIFO.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_clear_tx(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->CMD_SET = PIXELRZ_CMD_CLEARTX;
  sl_hal_pixelrz_wait_sync(pixelrz);

  while (((pixelrz->STATUS & _PIXELRZ_STATUS_CLEARTXBUSY_MASK) >> _PIXELRZ_STATUS_CLEARTXBUSY_SHIFT) != 0) {
    // Wait for clearing the TX FIFO to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Reset the PIXELRZ
 *
 * @param[in] pixelrz
 *   A pixelrz peripheral module.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_reset(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->SWRST_SET = PIXELRZ_SWRST_SWRST;
}

/***************************************************************************//**
 * @brief
 *   Gets PIXELRZ STATUS register value.
 *
 * @param[in] pixelrz
 *   A PIXELRZ peripheral module.
 *
 * @return  Current STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pixelrz_get_status(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  return pixelrz->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending PIXELRZ interrupts.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @param[in] flags
 *   Pending PIXELRZ interrupt source(s) to clear. Use one or more valid
 *   interrupt flags for the PIXELRZ module (PIXELRZ_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_clear_interrupts(PIXELRZ_TypeDef *pixelrz,
                                              uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending PIXELRZ interrupts from Software.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @param[in] flags
 *   PIXELRZ interrupt source(s) to set to pending. Use one or more valid
 *   interrupt flags for the PIXELRZ module (PIXELRZ_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_set_interrupts(PIXELRZ_TypeDef *pixelrz,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more PIXELRZ interrupts.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @param[in] flags
 *   PIXELRZ interrupt source(s) to disable. Use one or more valid
 *   interrupt flags for the PIXELRZ module (PIXELRZ_IEN_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_disable_interrupts(PIXELRZ_TypeDef *pixelrz,
                                                uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more PIXELRZ interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt.  To ignore a pending interrupt, consider using
 *   sl_hal_pixelrz_clear_interrupts() prior to enabling the interrupt.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @param[in] flags
 *   PIXELRZ interrupt source(s) to enable. Use one or more valid
 *   interrupt flags for the PIXELRZ module (PIXELRZ_IEN_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pixelrz_enable_interrupts(PIXELRZ_TypeDef *pixelrz,
                                               uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  pixelrz->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending PIXELRZ interrupt flags.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @return
 *   PIXELRZ interrupt source(s) pending. Returns one or more valid
 *   interrupt flags for the PIXELRZ module (PIXELRZ_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pixelrz_get_pending_interrupts(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  return pixelrz->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled PIXELRZ interrupt flags.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @return
 *   PIXELRZ interrupt source(s) enabled. Returns one or more valid
 *   enabled interrupt sources in PIXELRZx_IEN_nnn.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pixelrz_get_enabled_interrupts(PIXELRZ_TypeDef *pixelrz)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  return pixelrz->IEN;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending PIXELRZ interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] pixelrz
 *   Pointer to the PIXELRZ peripheral register block.
 *
 * @note
 *   Interrupt flags are not cleared by this function.
 *
 * @return
 *   Pending and enabled PIXELRZ interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in PIXELRZx_IEN_nnn
 *     register (PIXELRZx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the PIXELRZ module
 *     (PIXELRZx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_pixelrz_get_enabled_pending_interrupts(PIXELRZ_TypeDef *pixelrz)
{
  uint32_t ien;

  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));

  // Store PIXELRZ->IEN in temporary variable in order to define explicit order.
  // of volatile accesses.
  ien = pixelrz->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return pixelrz->IF & ien;
}

/** @} (end addtogroup pixelrz) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/****************************************************************************//**
 * @addtogroup pixelrz PIXELRZ - Serial Pixel Interface
 * @{
 *
 * @li @ref pixelrz_introduction
 * @li @ref pixelrz_example
 *
 *@n @section pixelrz_introduction Introduction
 *  PIXELRZ relays on external addressable LED to be configured correctly.
 *  In this example we will use WS2812B LEDS. Check datasheet of WS2812B for timing.
 *  To configure the timing, user must call @sl_hal_pixelrz_get_symbol_configuration().
 *
 *@n @section pixelrz_example Example
 *
 *  @code{.c}
 *  {
 *    #define PIXELRZ_GPIO         PB0
 *    #define PIXELRZ_PIXEL_COUNT  60  // 60 pixels
 *    #define PIXELRZ_PIXEL_WIDTH  24  // 24 bits per pixel
 *    #define PIXELRZ_PIXEL_TRAIL  0   // no trail data
 *
 *    // WS2812B timing values in microseconds. Check datasheet.
 *    #define WS2812B_T0_HIGH    0.4f
 *    #define WS2812B_T0_LOW     0.85f
 *    #define WS2812B_T1_HIGH    0.8f
 *    #define WS2812B_T1_LOW     0.45f
 *    #define WS2812B_TRST_HIGH  0.0f
 *    #define WS2812B_TRST_LOW   60.0f
 *
 *    sl_hal_pixelrz_init_t init_serial = SL_HAL_PIXELRZ_CONFIG_SEQUENTIAL_DEFAULT;
 *    uint32_t freq;
 *
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PIXELRZ0);
 *    sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_PIXELRZCLK, &freq);
 *
 *    init_serial.pixel_number = PIXELRZ_PIXEL_COUNT;
 *    init_serial.pixel_width = PIXELRZ_PIXEL_WIDTH;
 *    init_serial.trail_data_width = PIXELRZ_PIXEL_TRAIL;
 *
 *    init_serial.zero_symbol = sl_hal_pixelrz_get_symbol_configuration(WS2812B_T0_HIGH, WS2812B_T0_LOW, freq);
 *    init_serial.one_symbol = sl_hal_pixelrz_get_symbol_configuration(WS2812B_T1_HIGH, WS2812B_T1_LOW, freq);
 *    init_serial.reset_symbol = sl_hal_pixelrz_get_symbol_configuration(WS2812B_TRST_HIGH, WS2812B_TRST_LOW, freq);
 *
 *    // Configure the GPIO.
 *    sl_gpio_set_pin_mode(PIXELRZ_GPIO, SL_GPIO_MODE_PUSH_PULL, 0);
 *    GPIO->PIXELRZROUTE[PIXELRZ_NUM(PIXELRZ0)].RZTXOUTROUTE = (PIXELRZ_GPIO->port << _GPIO_PIXELRZ_RZTXOUTROUTE_PORT_SHIFT)
 *                                                           | (PIXELRZ_GPIO->pin << _GPIO_PIXELRZ_RZTXOUTROUTE_PIN_SHIFT);
 *    GPIO->PIXELRZROUTE[PIXELRZ_NUM(PIXELRZ0)].ROUTEEN = GPIO_PIXELRZ_ROUTEEN_RZTXOUTPEN;
 *
 *    sl_hal_pixelrz_init(PIXELRZ0, &init_serial);
 *    sl_hal_pixelrz_enable(PIXELRZ0);
 *    sl_hal_pixelrz_enable_tx(PIXELRZ0);
 *    sl_hal_pixelrz_wait_sync(PIXELRZ0);
 *
 *    // User can start transmitting data directly or using LDMA.
 *  }
 *
 *  @endcode
 *
 * @} (end addtogroup pixelrz)
 ******************************************************************************/
/* *INDENT-ON* */

#endif /* defined(PIXELRZ_COUNT) && (PIXELRZ_COUNT > 0) */
#endif /* SL_HAL_PIXELRZ_H */
