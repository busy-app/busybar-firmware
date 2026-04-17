/******************************************************************************
* @file sl_mvp_hal_si91x.c
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
#include "sl_mvp_hal.h"
#include "sl_si91x_mvp_config.h"
#include "em_device.h"
#if SL_MVP_ENABLE_DMA
#include "sl_si91x_dma.h"
#include "sl_si91x_dma_inst_config.h"
#endif
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

//************************************
// Macro definitions

// This macro is used to configure if the MVP is enabled at all times between
// init and deinit by setting it to 1, or just enabled during command execution
// by setting it to 0.
#define SPEED_OVER_POWER 1

#define MVP_CLK_ENABLE_BIT 25

#define MVP_IRQHandler IRQ062_Handler

//************************************
// Static variables

static sli_mvp_enable_callback callback_enable   = NULL;
static sli_mvp_disable_callback callback_disable = NULL;
static sli_mvp_isr_callback callback_isr         = NULL;
static volatile bool mvp_is_initialized          = false;
static volatile bool mvp_is_busy                 = false;
static sli_mvp_hal_config_t hal_config           = { .use_dma = SL_MVP_ENABLE_DMA, .dma_ch = SL_MVP_DMA_CHANNEL };
#if SL_MVP_ENABLE_DMA
volatile uint8_t transfer_done = 0; //Transfer done flag
#endif

//************************************
// Static functions

/**
 * @brief
 *   Load a program using the CPU.
 */
static void cpu_load(uint32_t *src, uint32_t *dst, size_t length)
{
  length /= sizeof(uint32_t);
  for (size_t i = 0; i < length; i++) {
    dst[i] = src[i];
  }
}

#if SL_MVP_ENABLE_DMA
/**
 * @brief
 *   Transfer callback function.
 */
void transfer_complete_callback_dmadrv(uint32_t channel, void *data)
{
  transfer_done = 1;
}

/**
 * @brief
 *   Load a program using the LDMA.
 */
static sl_status_t dma_load(void *src, void *dst, size_t length)
{
  int status;
  length /= sizeof(uint32_t);
  transfer_done = 0;

  status = sl_si91x_dma_simple_transfer(SL_DMA_INSTANCE, hal_config.dma_ch, src, dst, length);
  if (status) {
    return status;
  }

  /*TODO: Need to check if DMA Auto starts MVP upon transfer, in that callback and transfer_done is not required*/
  while (!transfer_done)
    ;

  return SL_STATUS_OK;
}
#endif

/**
 * @brief
 *   Disable the MVP program execution.
 */
static void sli_mvp_hal_cmd_disable(void)
{
#if !SPEED_OVER_POWER
  if (callback_disable != NULL) {
    callback_disable();
  }
#endif
  mvp_is_busy = false;
}

//************************************
// Exported functions

sl_status_t sli_mvp_hal_config(sli_mvp_hal_config_t *config)
{
  sl_status_t status = SL_STATUS_OK;
  hal_config         = *config;
#if SL_MVP_ENABLE_DMA
  if (hal_config.use_dma) {
    sl_dma_init_t dma_init = { SL_DMA_INSTANCE };
    sl_dma_callback_t callbacks;
    status = sl_si91x_dma_init(&dma_init);
    if (status) {
      return status;
    }
    //Allocate channel for transfer
    status = sl_si91x_dma_allocate_channel(SL_DMA_INSTANCE, &hal_config.dma_ch, 0);
    if (status) {
      return status;
    }

    callbacks.transfer_complete_cb = transfer_complete_callback_dmadrv;
    status                         = sl_si91x_dma_register_callbacks(SL_DMA_INSTANCE, hal_config.dma_ch, &callbacks);
  }
#endif
  return status;
}

sl_status_t sli_mvp_hal_init(sli_mvp_enable_callback enable, sli_mvp_disable_callback disable, sli_mvp_isr_callback isr)
{
  if (mvp_is_initialized) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }
  callback_enable  = enable;
  callback_disable = disable;
  callback_isr     = isr;
  M4CLK->CLK_ENABLE_SET_REG1 |= BIT(MVP_CLK_ENABLE_BIT);
  M4CLK->DYN_CLK_GATE_DISABLE_REG |= BIT(MVP_CLK_ENABLE_BIT);
  sli_mvp_hal_config(&hal_config);
  NVIC_EnableIRQ(MVP_IRQn);
#if SPEED_OVER_POWER
  if (callback_enable != NULL) {
    callback_enable();
  }
#endif
  mvp_is_initialized = true;
  mvp_is_busy        = false;
  return SL_STATUS_OK;
}

sl_status_t sli_mvp_hal_deinit(void)
{
  if (!mvp_is_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (mvp_is_busy) {
    sli_mvp_hal_cmd_disable();
  }

#if SPEED_OVER_POWER
  if (callback_disable != NULL) {
    callback_disable();
  }
#endif
  mvp_is_initialized = false;
  NVIC_DisableIRQ(MVP_IRQn);
  M4CLK->CLK_ENABLE_CLEAR_REG1 = BIT(MVP_CLK_ENABLE_BIT);
  M4CLK->DYN_CLK_GATE_DISABLE_REG &= ~(BIT(MVP_CLK_ENABLE_BIT));
  return SL_STATUS_OK;
}

sl_status_t sli_mvp_hal_load_program(void *src, void *dst, size_t length)
{
  sl_status_t status = SL_STATUS_OK;
  if (!mvp_is_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (!mvp_is_busy) {
    return SL_STATUS_NOT_AVAILABLE;
  }
#if SL_MVP_ENABLE_DMA
  if (hal_config.use_dma) {
    status = dma_load(src, dst, length);
  } else {
    cpu_load(src, dst, length);
  }
#else
  cpu_load(src, dst, length);
#endif
  return status;
}

void sli_mvp_hal_cmd_enable(void)
{
#if !SPEED_OVER_POWER
  if (callback_enable != NULL) {
    callback_enable();
  }
#endif
  mvp_is_busy = true;
}

bool sli_mvp_hal_cmd_is_busy(void)
{
  if (!mvp_is_initialized) {
    return false;
  }
  return mvp_is_busy;
}

void sli_mvp_hal_cmd_wait_for_completion()
{
  if (!mvp_is_initialized) {
    return;
  }
  // Logically, just wait for the busy flag to go off. It's cleared in the ISR
  // when the operation is complete.
  // Because the busy flag is in memory, the MVP may add stalls if the flag is
  // accessed during program execution.
  // Mitigation: During most of the waiting, wait for the MVP to become idle.
  // This is the only improvement that requires knowledge of MVP registers in HAL.
  while (!(MVP->STATUS & MVP_STATUS_IDLE)) {
    // Wait until the MVP is idle.
  }
  // Finally, wait for the interrupt handler to clear the flag.
  // It may take a few more cycles after the MVP has become idle.
  while (mvp_is_busy) {
  }
}

/**
 * @brief
 *   MVP Interrupt handler
 *
 * @details
 *   This interrupt handler is triggered whenever a program is done or when one
 *   of the performance counters wraps.
 */
void MVP_IRQHandler(void)
{
  if (callback_isr != NULL) {
    if (callback_isr()) {
      sli_mvp_hal_cmd_disable();
    }
  }
}
