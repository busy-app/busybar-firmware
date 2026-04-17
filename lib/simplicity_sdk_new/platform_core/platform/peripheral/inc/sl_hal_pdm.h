/***************************************************************************//**
 * @file
 * @brief Pulse Density Modulation (PDM) peripheral API
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

#ifndef SL_HAL_PDM_H
#define SL_HAL_PDM_H

#include "em_device.h"

#include <stdint.h>
#include <stdbool.h>

#if defined(PDM_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_assert.h"
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup pdm PDM - Pulse Density Modulation
 * @brief Pulse Density Modulation API
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   **********************************
 ******************************************************************************/

/// Default PDM clock prescaler value, no prescaling.
#define SL_HAL_PDM_CLOCK_PRESCALER 0U

/// Default PDM down sampling rate, 32 times oversampling.
#define SL_HAL_PDM_DOWN_SAMPLING_RATE 32U

/// Default PDM gain, 5 dB.
#define SL_HAL_PDM_GAIN 5U

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Configure channel clock polarity.
SL_ENUM(sl_hal_pdm_ch_clock_polarity_t) {
  SL_HAL_PDM_CH_CLOCK_POLARITY_RISING_EDGE = _PDM_CFG0_CH0CLKPOL_NORMAL, ///< Input data clocked on rising clock edge.
  SL_HAL_PDM_CH_CLOCK_POLARITY_FALLING_EDGE = _PDM_CFG0_CH0CLKPOL_INVERT ///< Input data clocked on falling clock edge.
};

/// Configure FIFO Data valid level water-mark.
SL_ENUM(sl_hal_pdm_fifo_valid_watermark_t) {
  SL_HAL_PDM_FIFO_VALID_WATERMARK_ONE   = _PDM_CFG0_FIFODVL_ONE,   ///< At least one word.
  SL_HAL_PDM_FIFO_VALID_WATERMARK_TWO   = _PDM_CFG0_FIFODVL_TWO,   ///< Two words.
  SL_HAL_PDM_FIFO_VALID_WATERMARK_THREE = _PDM_CFG0_FIFODVL_THREE, ///< Three words.
  SL_HAL_PDM_FIFO_VALID_WATERMARK_FOUR  = _PDM_CFG0_FIFODVL_FOUR   ///< Four words.
};

/// Configure PDM filter data output format.
SL_ENUM(sl_hal_pdm_data_format_t) {
  SL_HAL_PDM_DATA_FORMAT_RIGHT_16   = _PDM_CFG0_DATAFORMAT_RIGHT16,   ///< Right aligned 16-bit, left bits are sign extended.
  SL_HAL_PDM_DATA_FORMAT_DOUBLE_16  = _PDM_CFG0_DATAFORMAT_DOUBLE16,  ///< Pack two 16-bit samples into one 32-bit word.
  SL_HAL_PDM_DATA_FORMAT_RIGHT_24   = _PDM_CFG0_DATAFORMAT_RIGHT24,   ///< Right aligned 24bit, left bits are sign extended.
  SL_HAL_PDM_DATA_FORMAT_FULL_32BIT = _PDM_CFG0_DATAFORMAT_FULL32BIT, ///< 32 bit data.
  SL_HAL_PDM_DATA_FORMAT_LEFT_16    = _PDM_CFG0_DATAFORMAT_LEFT16,    ///< Left aligned 16-bit, right bits are zeros.
  SL_HAL_PDM_DATA_FORMAT_LEFT_24    = _PDM_CFG0_DATAFORMAT_LEFT24,    ///< Left aligned 24-bit, right bits are zeros.
  SL_HAL_PDM_DATA_FORMAT_RAW_32BIT  = _PDM_CFG0_DATAFORMAT_RAW32BIT   ///< RAW 32 bit data from integrator.
};

/// Configure number of PDM channels.
SL_ENUM(sl_hal_pdm_number_of_channels_t) {
  SL_HAL_PDM_NUMBER_OF_CHANNELS_ONE   = _PDM_CFG0_NUMCH_ONE,   ///< Only one Channel.
  SL_HAL_PDM_NUMBER_OF_CHANNELS_TWO   = _PDM_CFG0_NUMCH_TWO,   ///< Two Channels.
#if defined(PDM_CFG0_NUMCH_THREE)
  SL_HAL_PDM_NUMBER_OF_CHANNELS_THREE = _PDM_CFG0_NUMCH_THREE, ///< Three Channels.
  SL_HAL_PDM_NUMBER_OF_CHANNELS_FOUR  = _PDM_CFG0_NUMCH_FOUR   ///< Four Channels.
#endif
};

/// Configure order of the PDM filter.
SL_ENUM(sl_hal_pdm_filter_order_t) {
  SL_HAL_PDM_FILTER_ORDER_SECOND = _PDM_CFG0_FORDER_SECOND, ///< Second order filter.
  SL_HAL_PDM_FILTER_ORDER_THIRD  = _PDM_CFG0_FORDER_THIRD,  ///< Third order filter.
  SL_HAL_PDM_FILTER_ORDER_FOURTH = _PDM_CFG0_FORDER_FOURTH, ///< Fourth order filter.
  SL_HAL_PDM_FILTER_ORDER_FIFTH  = _PDM_CFG0_FORDER_FIFTH   ///< Fifth order filter.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// PDM initialization structure.
typedef struct {
#if defined(PDM_CFG0_NUMCH_THREE)
  bool                               ch2ch3_stereo_enable; ///< Enable stereo mode for channel pair CH2 and CH3.
#endif
  bool                               ch0ch1_stereo_enable; ///< Enable stereo mode for channel pair CH0 and CH1.
  sl_hal_pdm_filter_order_t          filter_order;         ///< PDM filter order.
  sl_hal_pdm_number_of_channels_t    number_channels;      ///< Number of PDM channels.
  sl_hal_pdm_data_format_t           data_format;          ///< PDM filter data output format.
  sl_hal_pdm_fifo_valid_watermark_t  fifo_valid_watermark; ///< FIFO Data valid level water-mark.
  sl_hal_pdm_ch_clock_polarity_t     ch0_clock_polarity;   ///< Ch 0 clock polarity.
  sl_hal_pdm_ch_clock_polarity_t     ch1_clock_polarity;   ///< Ch 1 clock polarity.
#if defined(PDM_CFG0_NUMCH_THREE)
  sl_hal_pdm_ch_clock_polarity_t     ch2_clock_polarity;   ///< Ch 2 clock polarity.
  sl_hal_pdm_ch_clock_polarity_t     ch3_clock_polarity;   ///< Ch 3 clock polarity.
#endif
  uint32_t                           gain;                 ///< PDM gain.
  uint32_t                           down_sampling_rate;   ///< PDM down sampling rate.
  uint32_t                           clk_prescaler;        ///< PDM clock prescaler, resulting PDM clock is input clock / (prescaler + 1).
} sl_hal_pdm_init_t;

/*******************************************************************************
 **************************   STRUCT INITIALIZERS   ****************************
 ******************************************************************************/

/// Default configuration for PDM.
/// Stereo Ch0/1, 16bit samples, 44,100 Hz sampling rate,
/// 32 times oversampling (requires 1,411,209 Hz PDM clock).
#if defined(PDM_CFG0_NUMCH_THREE)
#define SL_HAL_PDM_INIT_DEFAULT                                                                         \
  {                                                                                                     \
    false,                                      /* Disable stereo mode for channel pair CH2 and CH3. */ \
    true,                                       /* Enable stereo mode for channel pair CH0 and CH1. */  \
    SL_HAL_PDM_FILTER_ORDER_FIFTH,              /* Fifth order filter. */                               \
    SL_HAL_PDM_NUMBER_OF_CHANNELS_TWO,          /* Two Channels. */                                     \
    SL_HAL_PDM_DATA_FORMAT_DOUBLE_16,           /* Two 16-bit samples per FIFO entry. */                \
    SL_HAL_PDM_FIFO_VALID_WATERMARK_ONE,        /* At least one word water-mark level. */               \
    SL_HAL_PDM_CH_CLOCK_POLARITY_RISING_EDGE,   /* Input data clocked on rising clock edge. */          \
    SL_HAL_PDM_CH_CLOCK_POLARITY_FALLING_EDGE,  /* Input data clocked on falling clock edge. */         \
    SL_HAL_PDM_CH_CLOCK_POLARITY_RISING_EDGE,   /* Input data clocked on rising clock edge. */          \
    SL_HAL_PDM_CH_CLOCK_POLARITY_RISING_EDGE,   /* Input data clocked on rising clock edge. */          \
    SL_HAL_PDM_GAIN,                            /* PDM gain. */                                         \
    SL_HAL_PDM_DOWN_SAMPLING_RATE,              /* PDM down sampling rate. */                           \
    SL_HAL_PDM_CLOCK_PRESCALER                  /* No clock prescaling. */                              \
  }
#else
#define SL_HAL_PDM_INIT_DEFAULT                                                                         \
  {                                                                                                     \
    true,                                        /* Enable stereo mode for channel pair CH0 and CH1. */ \
    SL_HAL_PDM_FILTER_ORDER_FIFTH,               /* Fifth order filter. */                              \
    SL_HAL_PDM_NUMBER_OF_CHANNELS_TWO,           /* Two Channels. */                                    \
    SL_HAL_PDM_DATA_FORMAT_DOUBLE_16,            /* Two 16-bit samples per FIFO entry. */               \
    SL_HAL_PDM_FIFO_VALID_WATERMARK_ONE,         /* At least one word water-mark level. */              \
    SL_HAL_PDM_CH_CLOCK_POLARITY_RISING_EDGE,    /* Input data clocked on rising clock edge. */         \
    SL_HAL_PDM_CH_CLOCK_POLARITY_FALLING_EDGE,   /* Input data clocked on falling clock edge. */        \
    SL_HAL_PDM_GAIN,                             /* PDM gain. */                                        \
    SL_HAL_PDM_DOWN_SAMPLING_RATE,               /* PDM down sampling rate. */                          \
    SL_HAL_PDM_CLOCK_PRESCALER                   /* No clock prescaling. */                             \
  }
#endif /* defined(PDM_CFG0_NUMCH_THREE) */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/**************************************************************************//**
 * @brief Initialize the PDM peripheral.
 *
 * @details This function will configure basic settings in PDM according to values
 *          in the initialization data structure.
 *          Notice that enabling of PDM clock, setup of PDM pins and setup of PRS
 *          is not covered by this function.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @param[in] init A pointer to the initialization structure used to configure the PDM.
 *****************************************************************************/
void sl_hal_pdm_init(PDM_TypeDef *pdm,
                     const sl_hal_pdm_init_t *init);

/**************************************************************************//**
 * @brief Initialize PDM registers with reset values.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *****************************************************************************/
void sl_hal_pdm_reset(PDM_TypeDef *pdm);

/***************************************************************************//**
 * @brief Waits for the PDM to complete all synchronization of register changes
 *        and commands.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pdm_wait_sync(PDM_TypeDef *pdm)
{
  while (pdm->SYNCBUSY != 0U) {
    // Wait for any pending CMD synchronization.
  }
}

/***************************************************************************//**
 * @brief Waits for the PDM to complete disabling procedure.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @note This function is only needed if the PDM peripheral supports disabling
 *       bit in the EN register. If the PDM peripheral doesn't support, this
 *       function will do nothing.
 ******************************************************************************/
__INLINE void sl_hal_pdm_wait_ready(PDM_TypeDef *pdm)
{
#if defined(PDM_EN_DISABLING)
  while (pdm->EN & _PDM_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
}
#else
  (void)pdm;
#endif
}

/**************************************************************************//**
 * @brief Enable the PDM peripheral.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *****************************************************************************/
__INLINE void sl_hal_pdm_enable(PDM_TypeDef *pdm)
{
  // Enable PDM.
  pdm->EN_SET = PDM_EN_EN;
}

/**************************************************************************//**
 * @brief Disable the PDM peripheral.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *****************************************************************************/
__INLINE void sl_hal_pdm_disable(PDM_TypeDef *pdm)
{
  // Disable PDM.
  pdm->EN_CLR = PDM_EN_EN;
}

/***************************************************************************//**
 * @brief Clear the PDM filter.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pdm_clear(PDM_TypeDef *pdm)
{
  sl_hal_pdm_wait_sync(pdm);
  pdm->CMD_SET = PDM_CMD_CLEAR;
}

/***************************************************************************//**
 * @brief Flush the PDM sample FIFO.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pdm_fifo_flush(PDM_TypeDef *pdm)
{
  sl_hal_pdm_wait_sync(pdm);
  pdm->CMD_SET = PDM_CMD_FIFOFL;
}

/***************************************************************************//**
 * @brief Start the PDM operation (start the PDM filter).
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pdm_start(PDM_TypeDef *pdm)
{
  sl_hal_pdm_wait_sync(pdm);
  pdm->CMD_SET = PDM_CMD_START;
}

/***************************************************************************//**
 * @brief Stop the PDM operation (stop the PDM filter).
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pdm_stop(PDM_TypeDef *pdm)
{
  sl_hal_pdm_wait_sync(pdm);
  pdm->CMD_SET = PDM_CMD_STOP;
}

/***************************************************************************//**
 * @brief Read one entry from the PDM FIFO.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @return The entry read from the FIFO.
 *
 * @note This function will wait until a sample is available in the FIFO.
 *       Depending on PDM configuration, a FIFO entry can consist of one or two
 *       samples.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pdm_rx(PDM_TypeDef *pdm)
{
  while ((pdm->STATUS & PDM_STATUS_EMPTY) == PDM_STATUS_EMPTY) {
    // Wait for data in FIFO.
  }
  return pdm->RXDATA;
}

/***************************************************************************//**
 * @brief Get the PDM STATUS register.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @return STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pdm_get_status(PDM_TypeDef *pdm)
{
  return pdm->STATUS;
}

/***************************************************************************//**
 * @brief Clear one or more pending PDM interrupts.
 *
 * @param[in] pdm   A pointer to the PDM peripheral register block.
 *
 * @param[in] flags Pending PDM interrupt sources to clear. Use one or more valid
 *                  interrupt flags for the PDM module. The flags are PDM_IFC_DV,
 *                  PDM_IFC_DVL, PDM_IFC_OF and PDM_IFC_UF.
 ******************************************************************************/
__INLINE void sl_hal_pdm_clear_interrupts(PDM_TypeDef *pdm,
                                          uint32_t flags)
{
  pdm->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief Set one or more pending PDM interrupts.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @param[in] flags PDM interrupt sources to set to pending. Use one or more valid
 *                  interrupt flags for the PDM module. The flags are PDM_IFS_DV,
 *                  PDM_IFS_DVL, PDM_IFS_OF and PDM_IFS_UF.
 ******************************************************************************/
__INLINE void sl_hal_pdm_set_interrupts(PDM_TypeDef *pdm,
                                        uint32_t flags)
{
  pdm->IF_SET = flags;
}

/***************************************************************************//**
 * @brief Enable one or more PDM interrupts.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @param[in] flags PDM interrupt sources to enable. Use one or more valid
 *                  interrupt flags for the PDM module. The flags are PDM_IEN_DV,
 *                  PDM_IEN_DVL, PDM_IEN_OF and PDM_IEN_UF.
 *
 * @note Depending on the use, a pending interrupt may already be set prior to
 *       enabling the interrupt. To ignore a pending interrupt, consider using
 *       sl_hal_pdm_clear_interrupts() prior to enabling the interrupt.
 ******************************************************************************/
__INLINE void sl_hal_pdm_enable_interrupts(PDM_TypeDef *pdm,
                                           uint32_t flags)
{
  pdm->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief Disable one or more PDM interrupts.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @param[in] flags PDM interrupt sources to disable. Use one or more valid
 *                  interrupt flags for the PDM module. The flags are PDM_IEN_DV,
 *                  PDM_IEN_DVL, PDM_IEN_OF and PDM_IEN_UF.
 ******************************************************************************/
__INLINE void sl_hal_pdm_disable_interrupts(PDM_TypeDef *pdm,
                                            uint32_t flags)
{
  pdm->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief Get pending PDM interrupt flags.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @return PDM interrupt sources pending. Returns one or more valid
 *         interrupt flags for PDM module. The flags are PDM_IF_DV,
 *         PDM_IF_DVL, PDM_IF_OF and PDM_IF_UF.
 *
 * @note Event bits are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pdm_get_pending_interrupts(PDM_TypeDef *pdm)
{
  return pdm->IF;
}

/***************************************************************************//**
 * @brief Get enabled and pending PDM interrupt flags.
 *        Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] pdm A pointer to the PDM peripheral register block.
 *
 * @return Pending and enabled PDM interrupt sources
 *         Return value is the bitwise AND of
 *         - the enabled interrupt sources in PDM_IEN and
 *         - the pending interrupt flags PDM_IF
 *
 * @note Interrupt flags are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pdm_get_enabled_pending_interrupts(PDM_TypeDef *pdm)
{
  uint32_t ien;

  ien = pdm->IEN;

  return pdm->IF & ien;
}

/* *INDENT-OFF* */
/*******************************************************************************
 *  @addtogroup pdm PDM - Pulse Density Modulation
 *  Pulse Density Modulation (PDM) peripheral API
 *
 *  @details
 *  PDM API functions provide full support for the PDM peripheral.
 *  The PDM peripheral accepts PDM bitstreams and produces PCM encoded output.
 *
 *  <b> The following is an example PDM usage when interfacing to PDM microphones: </b>
 *
 *  Configure clocks and GPIO pins:
 *
 *  @code
 *  uint32_t pBuffer[1000];
 *  sl_hal_pdm_init_t pdm_init = SL_HAL_PDM_INIT_DEFAULT;
 *  sl_gpio_t mic_clk_gpio;
 *  sl_gpio_t mic_data_gpio;
 *
 *  mic_clk_gpio.port = SL_MIC_PDM_CLK_PORT;
 *  mic_clk_gpio.pin = SL_MIC_PDM_CLK_PIN;
 *  mic_data_gpio.port = SL_MIC_PDM_DAT0_PORT;
 *  mic_data_gpio.pin = SL_MIC_PDM_DAT0_PIN;
 *
 *  sl_hal_gpio_set_pin_mode(&mic_clk_gpio, SL_GPIO_MODE_PUSH_PULL, 0);
 *  sl_hal_gpio_set_pin_mode(&mic_data_gpio, SL_GPIO_MODE_INPUT, 0);
 *
 *  Set fast slew rate on PDM mic CLK and DATA pins.
 *  sl_hal_gpio_set_slew_rate(&mic_clk_gpio, 7U);
 *
 *  // Enable PDM peripheral clock.
 *  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PDM);
 *  // Note: In clock manager configuration UI, select clock source for EM01GRPBCLK branch (PDM reference clock source) as HFRCODPLL.
 *  // Note: Select pll frequency in clock manager UI to be 1,411,209 Hz
 *  // to achieve 44,100 kHz PCM sampling rate when using 32x PDM oversampling.
 *
 *  Route PDM signals to correct GPIO's.
 *  GPIO->PDMROUTE.ROUTEEN = GPIO_PDM_ROUTEEN_CLKPEN;
 *  GPIO->PDMROUTE.CLKROUTE = (mic_clk_gpio.port  << _GPIO_PDM_CLKROUTE_PORT_SHIFT)
 *                            | (mic_clk_gpio.pin << _GPIO_PDM_CLKROUTE_PIN_SHIFT);
 *  GPIO->PDMROUTE.DAT0ROUTE = (mic_data_gpio.port  << _GPIO_PDM_DAT0ROUTE_PORT_SHIFT)
 *                              | (mic_data_gpio.pin << _GPIO_PDM_DAT0ROUTE_PIN_SHIFT);
 *  @endcode
 *
 *  Initialize and start PDM, then read PDM samples from FIFO:
 *
 *  @code
 *  sl_hal_pdm_init_t init = SL_HAL_PDM_INIT_DEFAULT;
 *  sl_hal_pdm_init(PDM, &init);
 *  sl_hal_pdm_enable(PDM);
 *  sl_hal_pdm_clear(PDM);
 *  sl_hal_pdm_fifo_flush(PDM);
 *  sl_hal_pdm_start(PDM);
 *  
 *  for(uint32_t i = 0; i < 1000; i++) {
 *    pBuffer[i] = sl_hal_pdm_rx(PDM);
 *  }
 *  sl_hal_pdm_stop(PDM);
 *  @endcode
 *
 * @{
 * @} (end addtogroup pdm)
 ******************************************************************************/
/* *INDENT-ON* */

#ifdef __cplusplus
}
#endif

#endif /* defined(PDM_PRESENT) */
#endif /* SL_HAL_PDM_H */
