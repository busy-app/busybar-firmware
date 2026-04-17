/**************************************************************************//**
 * @file
 * @brief Device Manager API Definition
 ******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/

#include <stddef.h>
#include "sl_device_peripheral.h"
#include "sl_device_clock.h"
#include "cmsis_compiler.h"

/***************************************************************************//**
 * Device Abstraction Peripheral default values.
 ******************************************************************************/

// Weak definition of peripheral ACMP0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_acmp0 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral ACMP1.
__WEAK const sl_peripheral_val_t sl_peripheral_val_acmp1 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral ADC0.
__WEAK const sl_peripheral_adc_val_t sl_peripheral_val_adc0 = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                .dma_signal_scan = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral ADC1.
__WEAK const sl_peripheral_adc_val_t sl_peripheral_val_adc1 = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID,
                                                            .dma_signal_scan = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral AES.
__WEAK const sl_peripheral_val_t sl_peripheral_val_aes = { .base = 0xFFFFFFFF,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral AMUXCP0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_amuxcp0 = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral BUFC.
__WEAK const sl_peripheral_val_t sl_peripheral_val_bufc = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral BURAM.
__WEAK const sl_peripheral_val_t sl_peripheral_val_buram = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral BURTC.
__WEAK const sl_peripheral_val_t sl_peripheral_val_burtc = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral CMU.
__WEAK const sl_peripheral_val_t sl_peripheral_val_cmu = { .base = 0xFFFFFFFF,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral CRYPTOACC.
__WEAK const sl_peripheral_val_t sl_peripheral_val_cryptoacc = { .base = 0xFFFFFFFF,
                                                                 .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                 .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral DCDC.
__WEAK const sl_peripheral_val_t sl_peripheral_val_dcdc = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral DEVINFO.
__WEAK const sl_peripheral_val_t sl_peripheral_val_devinfo = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral DMEM.
__WEAK const sl_peripheral_val_t sl_peripheral_val_dmem = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral DMEM0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_dmem0 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral DMEM1.
__WEAK const sl_peripheral_val_t sl_peripheral_val_dmem1 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral DPLL0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_dpll0 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral EMU.
__WEAK const sl_peripheral_val_t sl_peripheral_val_emu = { .base = 0xFFFFFFFF,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral ETAMPDET.
__WEAK const sl_peripheral_val_t sl_peripheral_val_etampdet = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral EUART0.
__WEAK const sl_peripheral_euart_val_t sl_peripheral_val_euart0 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_rxfl = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txfl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral EUSART0.
__WEAK const sl_peripheral_eusart_val_t sl_peripheral_val_eusart0 = { .base = 0xFFFFFFFF,
                                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                      .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                      .dma_signal_rxfl = SL_DMA_SIGNAL_NONE,
                                                                      .dma_signal_txfl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral EUSART1.
__WEAK const sl_peripheral_eusart_val_t sl_peripheral_val_eusart1 = { .base = 0xFFFFFFFF,
                                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                      .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                      .dma_signal_rxfl = SL_DMA_SIGNAL_NONE,
                                                                      .dma_signal_txfl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral EUSART2.
__WEAK const sl_peripheral_eusart_val_t sl_peripheral_val_eusart2 = { .base = 0xFFFFFFFF,
                                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                      .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                      .dma_signal_rxfl = SL_DMA_SIGNAL_NONE,
                                                                      .dma_signal_txfl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral EUSART3.
__WEAK const sl_peripheral_eusart_val_t sl_peripheral_val_eusart3 = { .base = 0xFFFFFFFF,
                                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                      .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                      .dma_signal_rxfl = SL_DMA_SIGNAL_NONE,
                                                                      .dma_signal_txfl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral EUSART4.
__WEAK const sl_peripheral_eusart_val_t sl_peripheral_val_eusart4 = { .base = 0xFFFFFFFF,
                                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                      .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                      .dma_signal_rxfl = SL_DMA_SIGNAL_NONE,
                                                                      .dma_signal_txfl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral FSRCO.
__WEAK const sl_peripheral_val_t sl_peripheral_val_fsrco = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral GPCRC0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_gpcrc0 = { .base = 0xFFFFFFFF,
                                                              .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                              .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral GPIO.
__WEAK const sl_peripheral_val_t sl_peripheral_val_gpio = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral HFRCO0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_hfrco0 = { .base = 0xFFFFFFFF,
                                                              .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                              .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral HFRCOEM23.
__WEAK const sl_peripheral_val_t sl_peripheral_val_hfrcoem23 = { .base = 0xFFFFFFFF,
                                                                 .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                 .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral HFXO0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_hfxo0 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral HOSTMAILBOX.
__WEAK const sl_peripheral_val_t sl_peripheral_val_hostmailbox = { .base = 0xFFFFFFFF,
                                                                   .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                   .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral HOSTPORTAL.
__WEAK const sl_peripheral_val_t sl_peripheral_val_hostportal = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID };

__WEAK const sl_peripheral_i2c_val_t sl_peripheral_val_i2c0 = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                .dma_signal_rxdatav = SL_DMA_SIGNAL_NONE,
                                                                .dma_signal_txbl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral I2C1.
__WEAK const sl_peripheral_i2c_val_t sl_peripheral_val_i2c1 = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                .dma_signal_rxdatav = SL_DMA_SIGNAL_NONE,
                                                                .dma_signal_txbl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral I2C2.
__WEAK const sl_peripheral_i2c_val_t sl_peripheral_val_i2c2 = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                .dma_signal_rxdatav = SL_DMA_SIGNAL_NONE,
                                                                .dma_signal_txbl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral I2C3.
__WEAK const sl_peripheral_i2c_val_t sl_peripheral_val_i2c3 = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                .dma_signal_rxdatav = SL_DMA_SIGNAL_NONE,
                                                                .dma_signal_txbl = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral IADC0.
__WEAK const sl_peripheral_iadc_val_t sl_peripheral_val_iadc0 = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                  .dma_signal_scan = SL_DMA_SIGNAL_NONE,
                                                                  .dma_signal_single = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral ICACHE0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_icache0 = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral KEYSCAN.
__WEAK const sl_peripheral_val_t sl_peripheral_val_keyscan = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral L1ICACHE0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_l1icache0 = { .base = 0xFFFFFFFF,
                                                                 .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                 .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral L2ICACHE0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_l2icache0 = { .base = 0xFFFFFFFF,
                                                                 .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                 .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LCD.
__WEAK const sl_peripheral_lcd_val_t sl_peripheral_val_lcd = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID,
                                                               .dma_signal_lcd = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral LCDRF.
__WEAK const sl_peripheral_val_t sl_peripheral_val_lcdrf = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LDMAXBAR0.
__WEAK const sl_peripheral_ldmaxbar_val_t sl_peripheral_val_ldmaxbar0 = { .base = 0xFFFFFFFF,
                                                                          .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                          .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                          .dma_signal_prsreq0 = SL_DMA_SIGNAL_NONE,
                                                                          .dma_signal_prsreq1 = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral LEDSINK.
__WEAK const sl_peripheral_val_t sl_peripheral_val_ledsink0 = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LEDDRV0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_leddrv0 = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LESENSE.
__WEAK const sl_peripheral_lesense_val_t sl_peripheral_val_lesense = { .base = 0xFFFFFFFF,
                                                                       .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                       .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                       .dma_signal_fifo = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral LETIMER0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_letimer0 = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LFRCO.
__WEAK const sl_peripheral_val_t sl_peripheral_val_lfrco = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LFXO.
__WEAK const sl_peripheral_val_t sl_peripheral_val_lfxo = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LPWAES.
__WEAK const sl_peripheral_val_t sl_peripheral_val_lpwaes = { .base = 0xFFFFFFFF,
                                                              .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                              .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LPW0PORTAL.
__WEAK const sl_peripheral_val_t sl_peripheral_val_lpw0portal = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LVGD.
__WEAK const sl_peripheral_val_t sl_peripheral_val_lvgd = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral MSC.
__WEAK const sl_peripheral_msc_val_t sl_peripheral_val_msc = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID,
                                                               .dma_signal_wdata = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral MVP.
__WEAK const sl_peripheral_mvp_val_t sl_peripheral_val_mvp = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID,
                                                               .dma_signal_req = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral PCNT0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_pcnt0 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral PDM.
__WEAK const sl_peripheral_pdm_val_t sl_peripheral_val_pdm = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID,
                                                               .dma_signal_rxdata = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral PFMXPPRF.
__WEAK const sl_peripheral_val_t sl_peripheral_val_pfmxpprf = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral PIXELRZ0.
__WEAK const sl_peripheral_pixelrz_val_t sl_peripheral_val_pixelrz0 = { .base = 0xFFFFFFFF,
                                                                        .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                        .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                        .dma_signal_txf = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral PIXELRZ1.
__WEAK const sl_peripheral_pixelrz_val_t sl_peripheral_val_pixelrz1 = { .base = 0xFFFFFFFF,
                                                                        .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                        .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                        .dma_signal_txf = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral PRORTC.
__WEAK const sl_peripheral_val_t sl_peripheral_val_prortc = { .base = 0xFFFFFFFF,
                                                              .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                              .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral PRS.
__WEAK const sl_peripheral_val_t sl_peripheral_val_prs = { .base = 0xFFFFFFFF,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral RADIOAES.
__WEAK const sl_peripheral_val_t sl_peripheral_val_radioaes = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral RFFPLL0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_rffpll0 = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral RPA.
__WEAK const sl_peripheral_val_t sl_peripheral_val_rpa = { .base = 0xFFFFFFFF,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral RTCC.
__WEAK const sl_peripheral_val_t sl_peripheral_val_rtcc = { .base = 0xFFFFFFFF,
                                                            .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                            .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SCRATCHPAD.
__WEAK const sl_peripheral_val_t sl_peripheral_val_scratchpad = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SEMAILBOX.
__WEAK const sl_peripheral_val_t sl_peripheral_val_semailbox = { .base = 0xFFFFFFFF,
                                                                 .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                 .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SEMAPHORE0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_semaphore0 = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SEMAPHORE1.
__WEAK const sl_peripheral_val_t sl_peripheral_val_semaphore1 = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SEPORTAL.
__WEAK const sl_peripheral_val_t sl_peripheral_val_seportal = { .base = 0xFFFFFFFF,
                                                                .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SEPUF.
__WEAK const sl_peripheral_val_t sl_peripheral_val_sepuf = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SMU.
__WEAK const sl_peripheral_val_t sl_peripheral_val_smu = { .base = 0xFFFFFFFF,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SOCPLL0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_socpll0 = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SYMCRYPTO.
__WEAK const sl_peripheral_val_t sl_peripheral_val_symcrypto = { .base = 0xFFFFFFFF,
                                                                 .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                 .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SYSCFG.
__WEAK const sl_peripheral_val_t sl_peripheral_val_syscfg = { .base = 0xFFFFFFFF,
                                                              .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                              .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral SYSRTC0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_sysrtc0 = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral TIMER0.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer0 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER1.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer1 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER2.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer2 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER3.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer3 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER4.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer4 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER5.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer5 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER6.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer6 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER7.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer7 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER8.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer8 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral TIMER9.
__WEAK const sl_peripheral_timer_val_t sl_peripheral_val_timer9 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_cc0 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc1 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc2 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_cc6 = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_ufof = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral ULFRCO.
__WEAK const sl_peripheral_val_t sl_peripheral_val_ulfrco = { .base = 0xFFFFFFFF,
                                                              .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                              .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral USART0.
__WEAK const sl_peripheral_usart_val_t sl_peripheral_val_usart0 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_rxdatav = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_rxdatavright = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txbl = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txblright = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txempty = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral USART1.
__WEAK const sl_peripheral_usart_val_t sl_peripheral_val_usart1 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_rxdatav = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_rxdatavright = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txbl = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txblright = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txempty = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral USART2.
__WEAK const sl_peripheral_usart_val_t sl_peripheral_val_usart2 = { .base = 0xFFFFFFFF,
                                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                    .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                    .dma_signal_rxdatav = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_rxdatavright = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txbl = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txblright = SL_DMA_SIGNAL_NONE,
                                                                    .dma_signal_txempty = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral USB.
__WEAK const sl_peripheral_val_t sl_peripheral_val_usb = { .base = 0xFFFFFFFF,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral USBAHB.
__WEAK const sl_peripheral_val_t sl_peripheral_val_usbahb = { .base = 0xFFFFFFFF,
                                                              .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                              .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral USBPLL0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_usbpll0 = { .base = 0xFFFFFFFF,
                                                               .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                               .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral VDAC0.
__WEAK const sl_peripheral_vdac_val_t sl_peripheral_val_vdac0 = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                  .dma_signal_ch0req = SL_DMA_SIGNAL_NONE,
                                                                  .dma_signal_ch1req = SL_DMA_SIGNAL_NONE };

// Weak definition of peripheral VDAC1.
__WEAK const sl_peripheral_vdac_val_t sl_peripheral_val_vdac1 = { .base = 0xFFFFFFFF,
                                                                  .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                  .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                  .dma_signal_ch0req = SL_DMA_SIGNAL_NONE,
                                                                  .dma_signal_ch1req = SL_DMA_SIGNAL_NONE };// Weak definition of peripheral WDOG0.
__WEAK const sl_peripheral_val_t sl_peripheral_val_wdog0 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral WDOG1.
__WEAK const sl_peripheral_val_t sl_peripheral_val_wdog1 = { .base = 0xFFFFFFFF,
                                                             .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                             .bus_clock = SL_BUS_CLOCK_INVALID };

// Weak definition of peripheral LDMA0.
__WEAK const sl_peripheral_dma_val_t sl_peripheral_dma_val_ldma0 = {  .base = 0xFFFFFFFF,
                                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                                      .bus_clock = SL_BUS_CLOCK_INVALID,
                                                                      .nbr_channel = 0,
                                                                      .nbr_sync = 0,
                                                                      .dual_destination_map = 0x0,
                                                                      .rule_based_interleaving_map = 0x0 };

#if defined(__ICCARM__)
// Disable IAR multiple typedefs declaration warning.
#pragma diag_suppress=Pe301
#endif

// External base address getter declaration for ACMP.
extern ACMP_TypeDef *sl_device_peripheral_acmp_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for ADC.
extern ADC_TypeDef *sl_device_peripheral_adc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for AES.
extern AES_TypeDef *sl_device_peripheral_aes_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for AMUXCP.
extern AMUXCP_TypeDef *sl_device_peripheral_amuxcp_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for BUFC.
extern BUFC_TypeDef *sl_device_peripheral_bufc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for BURAM.
extern BURAM_TypeDef *sl_device_peripheral_buram_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for BURTC.
extern BURTC_TypeDef *sl_device_peripheral_burtc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for CMU.
extern CMU_TypeDef *sl_device_peripheral_cmu_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for CRYPTOACC.
extern CRYPTOACC_TypeDef *sl_device_peripheral_cryptoacc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for CRYPTOACC_PKCTRL.
extern CRYPTOACC_PKCTRL_TypeDef *sl_device_peripheral_cryptoacc_pkctrl_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for CRYPTOACC_RNGCTRL.
extern CRYPTOACC_RNGCTRL_TypeDef *sl_device_peripheral_cryptoacc_rngctrl_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for DCDC.
extern DCDC_TypeDef *sl_device_peripheral_dcdc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for DEVINFO.
extern DEVINFO_TypeDef *sl_device_peripheral_devinfo_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for DPLL.
extern DPLL_TypeDef *sl_device_peripheral_dpll_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for EMU.
extern EMU_TypeDef *sl_device_peripheral_emu_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for EMU_CFGNS.
extern EMU_CFGNS_TypeDef *sl_device_peripheral_emu_cfgns_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for ETAMPDET.
extern ETAMPDET_TypeDef *sl_device_peripheral_etampdet_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for EUSART.
extern EUSART_TypeDef *sl_device_peripheral_eusart_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for FSRCO.
extern FSRCO_TypeDef *sl_device_peripheral_fsrco_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for GPCRC.
extern GPCRC_TypeDef *sl_device_peripheral_gpcrc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for GPIO.
extern GPIO_TypeDef *sl_device_peripheral_gpio_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for HFRCO.
extern HFRCO_TypeDef *sl_device_peripheral_hfrco_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for HFXO.
extern HFXO_TypeDef *sl_device_peripheral_hfxo_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for HOSTPORTAL.
extern HOSTPORTAL_TypeDef *sl_device_peripheral_hostportal_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for HYDRARAM.
extern HYDRARAM_TypeDef *sl_device_peripheral_hydraram_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for I2C.
extern I2C_TypeDef *sl_device_peripheral_i2c_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for IADC.
extern IADC_TypeDef *sl_device_peripheral_iadc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for ICACHE.
extern ICACHE_TypeDef *sl_device_peripheral_icache_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for KEYSCAN.
extern KEYSCAN_TypeDef *sl_device_peripheral_keyscan_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for L2CACHE.
extern L2CACHE_TypeDef *sl_device_peripheral_l2cache_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LCD.
extern LCD_TypeDef *sl_device_peripheral_lcd_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LCDRF.
extern LCDRF_TypeDef *sl_device_peripheral_lcdrf_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LDMA.
extern LDMA_TypeDef *sl_device_peripheral_ldma_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LDMAXBAR.
extern LDMAXBAR_TypeDef *sl_device_peripheral_ldmaxbar_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LEDDRV.
extern LEDDRV_TypeDef *sl_device_peripheral_leddrv_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LESENSE.
extern LESENSE_TypeDef *sl_device_peripheral_lesense_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LETIMER.
extern LETIMER_TypeDef *sl_device_peripheral_letimer_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LFRCO.
extern LFRCO_TypeDef *sl_device_peripheral_lfrco_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LFXO.
extern LFXO_TypeDef *sl_device_peripheral_lfxo_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LPWAES.
extern LPWAES_TypeDef *sl_device_peripheral_lpwaes_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LPW0PORTAL.
extern LPW0PORTAL_TypeDef *sl_device_peripheral_lpw0portal_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for LVGD.
extern LVGD_TypeDef *sl_device_peripheral_lvgd_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for MAILBOX.
extern MAILBOX_TypeDef *sl_device_peripheral_mailbox_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for MPAHBRAM.
extern MPAHBRAM_TypeDef *sl_device_peripheral_mpahbram_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for MSC.
extern MSC_TypeDef *sl_device_peripheral_msc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for MVP.
extern MVP_TypeDef *sl_device_peripheral_mvp_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for PCNT.
extern PCNT_TypeDef *sl_device_peripheral_pcnt_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for PDM.
extern PDM_TypeDef *sl_device_peripheral_pdm_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for PFMXPPRF.
extern PFMXPPRF_TypeDef *sl_device_peripheral_pfmxpprf_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for PIXELRZ.
extern PIXELRZ_TypeDef *sl_device_peripheral_pixelrz_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for PRS.
extern PRS_TypeDef *sl_device_peripheral_prs_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for RFFPLL.
extern RFFPLL_TypeDef *sl_device_peripheral_rffpll_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for RPA.
extern RPA_TypeDef *sl_device_peripheral_rpa_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for RTCC.
extern RTCC_TypeDef *sl_device_peripheral_rtcc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SCRATCHPAD.
extern SCRATCHPAD_TypeDef *sl_device_peripheral_scratchpad_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SEMAILBOX_AHBHOST.
extern SEMAILBOX_AHBHOST_TypeDef *sl_device_peripheral_semailbox_ahbhost_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SEMAILBOX_HOST.
extern SEMAILBOX_HOST_TypeDef *sl_device_peripheral_semailbox_host_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SEMAPHORE.
extern SEMAPHORE_TypeDef *sl_device_peripheral_semaphore_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SEPORTAL.
extern SEPORTAL_TypeDef *sl_device_peripheral_seportal_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SEPUF_APBCFG.
extern SEPUF_APBCFG_TypeDef *sl_device_peripheral_sepuf_apbcfg_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SMU.
extern SMU_TypeDef *sl_device_peripheral_smu_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SOCPLL.
extern SOCPLL_TypeDef *sl_device_peripheral_socpll_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SYMCRYPTO.
extern SYMCRYPTO_TypeDef *sl_device_peripheral_symcrypto_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SYSCFG.
extern SYSCFG_TypeDef *sl_device_peripheral_syscfg_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SYSCFG_CFGNS.
extern SYSCFG_CFGNS_TypeDef *sl_device_peripheral_syscfg_cfgns_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for SYSRTC.
extern SYSRTC_TypeDef *sl_device_peripheral_sysrtc_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for TIMER.
extern TIMER_TypeDef *sl_device_peripheral_timer_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for ULFRCO.
extern ULFRCO_TypeDef *sl_device_peripheral_ulfrco_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for USART.
extern USART_TypeDef *sl_device_peripheral_usart_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for USBAHB_AHBS.
extern USBAHB_AHBS_TypeDef *sl_device_peripheral_usbahb_ahbs_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for USBPLL.
extern USBPLL_TypeDef *sl_device_peripheral_usbpll_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for USB_APBS.
extern USB_APBS_TypeDef *sl_device_peripheral_usb_apbs_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for VDAC.
extern VDAC_TypeDef *sl_device_peripheral_vdac_get_base_addr(const sl_peripheral_t peripheral);

// External base address getter declaration for WDOG.
extern WDOG_TypeDef *sl_device_peripheral_wdog_get_base_addr(const sl_peripheral_t peripheral);

// External clock branch getter declaration.
extern sl_clock_branch_t sl_device_peripheral_get_clock_branch(const sl_peripheral_t peripheral);

// External bus clock getter declaration.
extern sl_bus_clock_t sl_device_peripheral_get_bus_clock(const sl_peripheral_t peripheral);

// External DMA channel count getter declaration.
extern uint8_t sl_device_peripheral_get_dma_channel_count(const sl_peripheral_t peripheral);

// External DMA SYNC count getter declaration.
extern uint8_t sl_device_peripheral_get_dma_sync_count(const sl_peripheral_t peripheral);

// External DMA dual destination bitmap getter declaration.
extern uint32_t sl_device_peripheral_get_dma_dual_destination_bitmap(const sl_peripheral_t peripheral);

// External DMA interleaving bitmap getter declaration.
extern uint32_t sl_device_peripheral_get_dma_interleaving_bitmap(const sl_peripheral_t peripheral);

// External TXDAV signal getter declaration.
extern sl_dma_signal_t sl_device_peripheral_get_i2c_txdav_dma_signal(const sl_peripheral_t peripheral);

// External RXDAV signal getter declaration.
extern sl_dma_signal_t sl_device_peripheral_get_i2c_rxdav_dma_signal(const sl_peripheral_t peripheral);

// External TIMER signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_timer_cc0_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_timer_cc1_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_timer_cc2_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_timer_ufof_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_timer_cc3_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_timer_cc4_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_timer_cc5_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_timer_cc6_dma_signal(const sl_peripheral_t peripheral);

// External PROTIMER signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_protimer_cc0_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_protimer_cc1_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_protimer_cc2_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_protimer_cc3_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_protimer_bof_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_protimer_pof_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_protimer_wof_dma_signal(const sl_peripheral_t peripheral);

// External IADC signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_iadc_scan_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_iadc_single_dma_signal(const sl_peripheral_t peripheral);

// External ADC signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_adc_scan_dma_signal(const sl_peripheral_t peripheral);

// External EUSART signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_eusart_rxfl_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_eusart_txfl_dma_signal(const sl_peripheral_t peripheral);

// External EUART signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_euart_rxfl_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_euart_txfl_dma_signal(const sl_peripheral_t peripheral);

// External USART signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_usart_rxdatav_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_usart_rxdatavright_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_usart_txbl_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_usart_txblright_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_usart_txempty_dma_signal(const sl_peripheral_t peripheral);

// External MSC signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_msc_wdata_dma_signal(const sl_peripheral_t peripheral);

// External MVP signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_mvp_req_dma_signal(const sl_peripheral_t peripheral);

// External PIXELRZ signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_pixelrz_txf_dma_signal(const sl_peripheral_t peripheral);

// External PDM signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_pdm_rxdata_dma_signal(const sl_peripheral_t peripheral);

// External LCD signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_lcd_lcd_dma_signal(const sl_peripheral_t peripheral);

// External LESENSE signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_lesense_fifo_dma_signal(const sl_peripheral_t peripheral);

// External VDAC signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_vdac_ch0req_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_vdac_ch1req_dma_signal(const sl_peripheral_t peripheral);

// External LDMAXBAR signal getter declarations.
extern sl_dma_signal_t sl_device_peripheral_get_ldmaxbar_prsreq0_dma_signal(const sl_peripheral_t peripheral);
extern sl_dma_signal_t sl_device_peripheral_get_ldmaxbar_prsreq1_dma_signal(const sl_peripheral_t peripheral);

#if defined(__ICCARM__)
// Disable IAR multiple typedefs declaration warning.
#pragma diag_default=Pe301
#endif
