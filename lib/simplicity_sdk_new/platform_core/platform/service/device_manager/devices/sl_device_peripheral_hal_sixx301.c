/**************************************************************************//**
 * @file
 * @brief Device Manager Peripheral Definition
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

#include "em_device.h"
#include "sl_device_peripheral.h"
#include "sl_device_clock.h"

/***************************************************************************//**
 * @addtogroup device_peripheral Device Abstraction Peripheral
 * @{
 ******************************************************************************/

#if defined(ACMP0_BASE)
// Define peripheral ACMP0.
const sl_peripheral_val_t sl_peripheral_val_acmp0 = { .base = ACMP0_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                      .bus_clock = SL_BUS_CLOCK_ACMP0 };
#endif

#if defined(ACMP1_BASE)
// Define peripheral ACMP1.
const sl_peripheral_val_t sl_peripheral_val_acmp1 = { .base = ACMP1_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                      .bus_clock = SL_BUS_CLOCK_ACMP1 };
#endif

#if defined(ADC0_BASE)
// Define peripheral ADC0.
const sl_peripheral_adc_val_t sl_peripheral_val_adc0 = { .base = ADC0_BASE,
                                                     .clk_branch = SL_CLOCK_BRANCH_ADCCLK,
                                                     .bus_clock = SL_BUS_CLOCK_ADC0,
                                                     .dma_signal_scan = SL_DMA_SIGNAL_ADC0_SCAN };
#endif

#if defined(BURAM_BASE)
// Define peripheral BURAM.
const sl_peripheral_val_t sl_peripheral_val_buram = { .base = BURAM_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                      .bus_clock = SL_BUS_CLOCK_BURAM };
#endif

#if defined(BURTC_BASE)
// Define peripheral BURTC.
const sl_peripheral_val_t sl_peripheral_val_burtc = { .base = BURTC_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_EM4GRPACLK,
                                                      .bus_clock = SL_BUS_CLOCK_BURTC };
#endif

#if defined(CMU_BASE)
// Define peripheral CMU.
const sl_peripheral_val_t sl_peripheral_val_cmu = { .base = CMU_BASE,
                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                    .bus_clock = SL_BUS_CLOCK_INVALID };
#endif

#if defined(DEVINFO_BASE)
// Define peripheral DEVINFO.
const sl_peripheral_val_t sl_peripheral_val_devinfo = { .base = DEVINFO_BASE,
                                                        .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                        .bus_clock = SL_BUS_CLOCK_DEVINFO };
#endif

#if defined(DMEM_BASE)
// Define peripheral DMEM.
const sl_peripheral_val_t sl_peripheral_val_dmem = { .base = DMEM_BASE,
                                                     .clk_branch = SL_CLOCK_BRANCH_HCLK,
                                                     .bus_clock = SL_BUS_CLOCK_DMEM };
#endif

#if defined(DPLL0_BASE)
// Define peripheral DPLL0.
const sl_peripheral_val_t sl_peripheral_val_dpll0 = { .base = DPLL0_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                      .bus_clock = SL_BUS_CLOCK_DPLL0 };
#endif

#if defined(EMU_BASE)
// Define peripheral EMU.
const sl_peripheral_val_t sl_peripheral_val_emu = { .base = EMU_BASE,
                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                    .bus_clock = SL_BUS_CLOCK_INVALID };
#endif

#if defined(ETAMPDET_BASE)
// Define peripheral ETAMPDET.
const sl_peripheral_val_t sl_peripheral_val_etampdet = { .base = ETAMPDET_BASE,
                                                         .clk_branch = SL_CLOCK_BRANCH_EM4GRPACLK,
                                                         .bus_clock = SL_BUS_CLOCK_ETAMPDET };
#endif

#if defined(EUSART0_BASE)
// Define peripheral EUSART0.
const sl_peripheral_eusart_val_t sl_peripheral_val_eusart0 = { .base = EUSART0_BASE,
                                                        .clk_branch = SL_CLOCK_BRANCH_EUSART0CLK,
                                                        .bus_clock = SL_BUS_CLOCK_EUSART0,
                                                        .dma_signal_rxfl = SL_DMA_SIGNAL_EUSART0_RXFL,
                                                        .dma_signal_txfl = SL_DMA_SIGNAL_EUSART0_TXFL };
#endif

#if defined(EUSART1_BASE)
// Define peripheral EUSART1.
const sl_peripheral_eusart_val_t sl_peripheral_val_eusart1 = { .base = EUSART1_BASE,
                                                        .clk_branch = SL_CLOCK_BRANCH_EM01GRPCCLK,
                                                        .bus_clock = SL_BUS_CLOCK_EUSART1,
                                                        .dma_signal_rxfl = SL_DMA_SIGNAL_EUSART1_RXFL,
                                                        .dma_signal_txfl = SL_DMA_SIGNAL_EUSART1_TXFL };
#endif

#if defined(EUSART2_BASE)
// Define peripheral EUSART2.
const sl_peripheral_eusart_val_t sl_peripheral_val_eusart2 = { .base = EUSART2_BASE,
                                                        .clk_branch = SL_CLOCK_BRANCH_EM01GRPCCLK,
                                                        .bus_clock = SL_BUS_CLOCK_EUSART2,
                                                        .dma_signal_rxfl = SL_DMA_SIGNAL_EUSART2_RXFL,
                                                        .dma_signal_txfl = SL_DMA_SIGNAL_EUSART2_TXFL };
#endif

#if defined(FSRCO_BASE)
// Define peripheral FSRCO.
const sl_peripheral_val_t sl_peripheral_val_fsrco = { .base = FSRCO_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                      .bus_clock = SL_BUS_CLOCK_FSRCO };
#endif

#if defined(GPCRC0_BASE)
// Define peripheral GPCRC0.
const sl_peripheral_val_t sl_peripheral_val_gpcrc0 = { .base = GPCRC0_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_PCLK,
                                                       .bus_clock = SL_BUS_CLOCK_GPCRC0 };
#endif

#if defined(GPIO_BASE)
// Define peripheral GPIO.
const sl_peripheral_val_t sl_peripheral_val_gpio = { .base = GPIO_BASE,
                                                     .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                     .bus_clock = SL_BUS_CLOCK_GPIO };
#endif

#if defined(HFRCO0_BASE)
// Define peripheral HFRCO0.
const sl_peripheral_val_t sl_peripheral_val_hfrco0 = { .base = HFRCO0_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                       .bus_clock = SL_BUS_CLOCK_HFRCO0 };
#endif

#if defined(HFRCOEM23_BASE)
// Define peripheral HFRCOEM23.
const sl_peripheral_val_t sl_peripheral_val_hfrcoem23 = { .base = HFRCOEM23_BASE,
                                                          .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                          .bus_clock = SL_BUS_CLOCK_HFRCOEM23 };
#endif

#if defined(HFXO0_BASE)
// Define peripheral HFXO0.
const sl_peripheral_val_t sl_peripheral_val_hfxo0 = { .base = HFXO0_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                      .bus_clock = SL_BUS_CLOCK_HFXO0 };
#endif

#if defined(HOSTPORTAL_BASE)
// Define peripheral HOSTPORTAL.
const sl_peripheral_val_t sl_peripheral_val_hostportal = { .base = HOSTPORTAL_BASE,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_HOSTPORTAL };
#endif

#if defined(I2C0_BASE)
// Define peripheral I2C0.
const sl_peripheral_i2c_val_t sl_peripheral_val_i2c0 = { .base = I2C0_BASE,
                                                     .clk_branch = SL_CLOCK_BRANCH_I2C0CLK,
                                                     .bus_clock = SL_BUS_CLOCK_I2C0,
                                                     .dma_signal_rxdatav = SL_DMA_SIGNAL_I2C0_RXDATAV,
                                                     .dma_signal_txbl = SL_DMA_SIGNAL_I2C0_TXBL };
#endif

#if defined(I2C1_BASE)
// Define peripheral I2C1.
const sl_peripheral_i2c_val_t sl_peripheral_val_i2c1 = { .base = I2C1_BASE,
                                                     .clk_branch = SL_CLOCK_BRANCH_EM01GRPDCLK,
                                                     .bus_clock = SL_BUS_CLOCK_I2C1,
                                                     .dma_signal_rxdatav = SL_DMA_SIGNAL_I2C1_RXDATAV,
                                                     .dma_signal_txbl = SL_DMA_SIGNAL_I2C1_TXBL };
#endif

#if defined(I2C2_BASE)
// Define peripheral I2C2.
const sl_peripheral_i2c_val_t sl_peripheral_val_i2c2 = { .base = I2C2_BASE,
                                                     .clk_branch = SL_CLOCK_BRANCH_EM01GRPDCLK,
                                                     .bus_clock = SL_BUS_CLOCK_I2C2,
                                                     .dma_signal_rxdatav = SL_DMA_SIGNAL_I2C2_RXDATAV,
                                                     .dma_signal_txbl = SL_DMA_SIGNAL_I2C2_TXBL };
#endif

#if defined(L1ICACHE0_BASE)
// Define peripheral L1ICACHE0.
const sl_peripheral_val_t sl_peripheral_val_l1icache0 = { .base = L1ICACHE0_BASE,
                                                          .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                          .bus_clock = SL_BUS_CLOCK_ICACHE0 };
#endif

#if defined(L2ICACHE0_BASE)
// Define peripheral L2ICACHE0.
const sl_peripheral_val_t sl_peripheral_val_l2icache0 = { .base = L2ICACHE0_BASE,
                                                          .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                          .bus_clock = SL_BUS_CLOCK_L2ICACHE0 };
#endif

#if defined(LDMAXBAR0_BASE)
// Define peripheral LDMAXBAR0.
const sl_peripheral_ldmaxbar_val_t sl_peripheral_val_ldmaxbar0 = { .base = LDMAXBAR0_BASE,
                                                          .clk_branch = SL_CLOCK_BRANCH_HCLK,
                                                          .bus_clock = SL_BUS_CLOCK_LDMAXBAR0,
                                                          .dma_signal_prsreq0 = SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ0,
                                                          .dma_signal_prsreq1 = SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ1 };
#endif

#if defined(LEDDRV0_BASE)
// Define peripheral LEDDRV0.
const sl_peripheral_val_t sl_peripheral_val_leddrv0 = { .base = LEDDRV0_BASE,
                                                        .clk_branch = SL_CLOCK_BRANCH_PCLK,
                                                        .bus_clock = SL_BUS_CLOCK_LEDDRV0 };
#endif

#if defined(LETIMER0_BASE)
// Define peripheral LETIMER0.
const sl_peripheral_val_t sl_peripheral_val_letimer0 = { .base = LETIMER0_BASE,
                                                         .clk_branch = SL_CLOCK_BRANCH_EM23GRPACLK,
                                                         .bus_clock = SL_BUS_CLOCK_LETIMER0 };
#endif

#if defined(LFRCO_BASE)
// Define peripheral LFRCO.
const sl_peripheral_val_t sl_peripheral_val_lfrco = { .base = LFRCO_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                      .bus_clock = SL_BUS_CLOCK_LFRCO };
#endif

#if defined(LFXO_BASE)
// Define peripheral LFXO.
const sl_peripheral_val_t sl_peripheral_val_lfxo = { .base = LFXO_BASE,
                                                     .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                     .bus_clock = SL_BUS_CLOCK_LFXO };
#endif

#if defined(LPWAES_BASE)
// Define peripheral LPWAES.
const sl_peripheral_val_t sl_peripheral_val_lpwaes = { .base = LPWAES_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                       .bus_clock = SL_BUS_CLOCK_LPWAES };
#endif

#if defined(LPW0PORTAL_BASE)
// Define peripheral LPW0PORTAL.
const sl_peripheral_val_t sl_peripheral_val_lpw0portal = { .base = LPW0PORTAL_BASE,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_LPW0PORTAL };
#endif

#if defined(PCNT0_BASE)
// Define peripheral PCNT0.
const sl_peripheral_val_t sl_peripheral_val_pcnt0 = { .base = PCNT0_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_PCNT0CLK,
                                                      .bus_clock = SL_BUS_CLOCK_PCNT0 };
#endif

#if defined(PIXELRZ0_BASE)
// Define peripheral PIXELRZ0.
const sl_peripheral_pixelrz_val_t sl_peripheral_val_pixelrz0 = { .base = PIXELRZ0_BASE,
                                                         .clk_branch = SL_CLOCK_BRANCH_PIXELRZCLK,
                                                         .bus_clock = SL_BUS_CLOCK_PIXELRZ0,
                                                         .dma_signal_txf = SL_DMA_SIGNAL_PIXELRZ0REQ_TXF };
#endif

#if defined(PIXELRZ1_BASE)
// Define peripheral PIXELRZ1.
const sl_peripheral_pixelrz_val_t sl_peripheral_val_pixelrz1 = { .base = PIXELRZ1_BASE,
                                                         .clk_branch = SL_CLOCK_BRANCH_PIXELRZCLK,
                                                         .bus_clock = SL_BUS_CLOCK_PIXELRZ1,
                                                         .dma_signal_txf = SL_DMA_SIGNAL_PIXELRZ1REQ_TXF };
#endif

#if defined(PRS_BASE)
// Define peripheral PRS.
const sl_peripheral_val_t sl_peripheral_val_prs = { .base = PRS_BASE,
                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                    .bus_clock = SL_BUS_CLOCK_PRS };
#endif

#if defined(RPA_BASE)
// Define peripheral RPA.
const sl_peripheral_val_t sl_peripheral_val_rpa = { .base = RPA_BASE,
                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                    .bus_clock = SL_BUS_CLOCK_RPA };
#endif

#if defined(SCRATCHPAD_BASE)
// Define peripheral SCRATCHPAD.
const sl_peripheral_val_t sl_peripheral_val_scratchpad = { .base = SCRATCHPAD_BASE,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_SCRATCHPAD };
#endif

#if defined(SEMAILBOX_HOST_BASE)
// Define peripheral SEMAILBOX.
const sl_peripheral_val_t sl_peripheral_val_semailbox = { .base = SEMAILBOX_HOST_BASE,
                                                          .clk_branch = SL_CLOCK_BRANCH_HCLK,
                                                          .bus_clock = SL_BUS_CLOCK_SEMAILBOX };
#endif

#if defined(SEMAPHORE0_BASE)
// Define peripheral SEMAPHORE0.
const sl_peripheral_val_t sl_peripheral_val_semaphore0 = { .base = SEMAPHORE0_BASE,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_SEMAPHORE0 };
#endif

#if defined(SEMAPHORE1_BASE)
// Define peripheral SEMAPHORE1.
const sl_peripheral_val_t sl_peripheral_val_semaphore1 = { .base = SEMAPHORE1_BASE,
                                                           .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                           .bus_clock = SL_BUS_CLOCK_SEMAPHORE1 };
#endif

#if defined(SEPORTAL_BASE)
// Define peripheral SEPORTAL.
const sl_peripheral_val_t sl_peripheral_val_seportal = { .base = SEPORTAL_BASE,
                                                         .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                         .bus_clock = SL_BUS_CLOCK_SEPORTAL };
#endif

#if defined(SMU_BASE)
// Define peripheral SMU.
const sl_peripheral_val_t sl_peripheral_val_smu = { .base = SMU_BASE,
                                                    .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                    .bus_clock = SL_BUS_CLOCK_SMU };
#endif

#if defined(SOCPLL0_BASE)
// Define peripheral SOCPLL0.
const sl_peripheral_val_t sl_peripheral_val_socpll0 = { .base = SOCPLL0_BASE,
                                                        .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                        .bus_clock = SL_BUS_CLOCK_SOCPLL0 };
#endif

#if defined(SYMCRYPTO_BASE)
// Define peripheral SYMCRYPTO.
const sl_peripheral_val_t sl_peripheral_val_symcrypto = { .base = SYMCRYPTO_BASE,
                                                          .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                          .bus_clock = SL_BUS_CLOCK_SYMCRYPTO };
#endif

#if defined(SYSCFG_BASE)
// Define peripheral SYSCFG.
const sl_peripheral_val_t sl_peripheral_val_syscfg = { .base = SYSCFG_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                       .bus_clock = SL_BUS_CLOCK_SYSCFG };
#endif

#if defined(SYSRTC0_BASE)
// Define peripheral SYSRTC0.
const sl_peripheral_val_t sl_peripheral_val_sysrtc0 = { .base = SYSRTC0_BASE,
                                                        .clk_branch = SL_CLOCK_BRANCH_SYSRTCCLK,
                                                        .bus_clock = SL_BUS_CLOCK_SYSRTC0 };
#endif

#if defined(TIMER0_BASE)
// Define peripheral TIMER0.
const sl_peripheral_timer_val_t sl_peripheral_val_timer0 = { .base = TIMER0_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_EM01GRPACLK,
                                                       .bus_clock = SL_BUS_CLOCK_TIMER0,
                                                       .dma_signal_cc0 = SL_DMA_SIGNAL_TIMER0_CC0,
                                                       .dma_signal_cc1 = SL_DMA_SIGNAL_TIMER0_CC1,
                                                       .dma_signal_cc2 = SL_DMA_SIGNAL_TIMER0_CC2,
                                                       .dma_signal_ufof = SL_DMA_SIGNAL_TIMER0_UFOF,
                                                       .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                       .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                       .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                       .dma_signal_cc6 = SL_DMA_SIGNAL_NONE };
#endif

#if defined(TIMER1_BASE)
// Define peripheral TIMER1.
const sl_peripheral_timer_val_t sl_peripheral_val_timer1 = { .base = TIMER1_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_EM01GRPACLK,
                                                       .bus_clock = SL_BUS_CLOCK_TIMER1,
                                                       .dma_signal_cc0 = SL_DMA_SIGNAL_TIMER1_CC0,
                                                       .dma_signal_cc1 = SL_DMA_SIGNAL_TIMER1_CC1,
                                                       .dma_signal_cc2 = SL_DMA_SIGNAL_TIMER1_CC2,
                                                       .dma_signal_ufof = SL_DMA_SIGNAL_TIMER1_UFOF,
                                                       .dma_signal_cc3 = SL_DMA_SIGNAL_NONE,
                                                       .dma_signal_cc4 = SL_DMA_SIGNAL_NONE,
                                                       .dma_signal_cc5 = SL_DMA_SIGNAL_NONE,
                                                       .dma_signal_cc6 = SL_DMA_SIGNAL_NONE };
#endif

#if defined(TIMER2_BASE)
// Define peripheral TIMER2.
const sl_peripheral_timer_val_t sl_peripheral_val_timer2 = { .base = TIMER2_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_EM01GRPACLK,
                                                       .bus_clock = SL_BUS_CLOCK_TIMER2,
                                                       .dma_signal_cc0 = SL_DMA_SIGNAL_TIMER2_CC0,
                                                       .dma_signal_cc1 = SL_DMA_SIGNAL_TIMER2_CC1,
                                                       .dma_signal_cc2 = SL_DMA_SIGNAL_TIMER2_CC2,
                                                       .dma_signal_ufof = SL_DMA_SIGNAL_TIMER2_UFOF,
                                                       .dma_signal_cc3 = SL_DMA_SIGNAL_TIMER2_CC3,
                                                       .dma_signal_cc4 = SL_DMA_SIGNAL_TIMER2_CC4,
                                                       .dma_signal_cc5 = SL_DMA_SIGNAL_TIMER2_CC5,
                                                       .dma_signal_cc6 = SL_DMA_SIGNAL_TIMER2_CC6 };
#endif

#if defined(TIMER3_BASE)
// Define peripheral TIMER3.
const sl_peripheral_timer_val_t sl_peripheral_val_timer3 = { .base = TIMER3_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_EM01GRPACLK,
                                                       .bus_clock = SL_BUS_CLOCK_TIMER3,
                                                       .dma_signal_cc0 = SL_DMA_SIGNAL_TIMER3_CC0,
                                                       .dma_signal_cc1 = SL_DMA_SIGNAL_TIMER3_CC1,
                                                       .dma_signal_cc2 = SL_DMA_SIGNAL_TIMER3_CC2,
                                                       .dma_signal_ufof = SL_DMA_SIGNAL_TIMER3_UFOF,
                                                       .dma_signal_cc3 = SL_DMA_SIGNAL_TIMER3_CC3,
                                                       .dma_signal_cc4 = SL_DMA_SIGNAL_TIMER3_CC4,
                                                       .dma_signal_cc5 = SL_DMA_SIGNAL_TIMER3_CC5,
                                                       .dma_signal_cc6 = SL_DMA_SIGNAL_TIMER3_CC6 };
#endif

#if defined(ULFRCO_BASE)
// Define peripheral ULFRCO.
const sl_peripheral_val_t sl_peripheral_val_ulfrco = { .base = ULFRCO_BASE,
                                                       .clk_branch = SL_CLOCK_BRANCH_INVALID,
                                                       .bus_clock = SL_BUS_CLOCK_ULFRCO };
#endif

#if defined(WDOG0_BASE)
// Define peripheral WDOG0.
const sl_peripheral_val_t sl_peripheral_val_wdog0 = { .base = WDOG0_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_WDOG0CLK,
                                                      .bus_clock = SL_BUS_CLOCK_WDOG0 };
#endif

#if defined(WDOG1_BASE)
// Define peripheral WDOG1.
const sl_peripheral_val_t sl_peripheral_val_wdog1 = { .base = WDOG1_BASE,
                                                      .clk_branch = SL_CLOCK_BRANCH_WDOG1CLK,
                                                      .bus_clock = SL_BUS_CLOCK_WDOG1 };
#endif

/** @} (end addtogroup device_peripheral) */

/***************************************************************************//**
 * @addtogroup device_peripheral_dma Device Abstraction DMA Peripheral
 * @{
 ******************************************************************************/

#if defined(LDMA0_BASE)
/// Define DMA peripheral LDMA0.
const sl_peripheral_dma_val_t sl_peripheral_dma_val_ldma0 = { .base = LDMA0_BASE,
                                                              .clk_branch = SL_CLOCK_BRANCH_HCLK,
                                                              .bus_clock = SL_BUS_CLOCK_LDMA0,
                                                              .nbr_channel = 8,
                                                              .nbr_sync = 8,
                                                              .dual_destination_map = 0x0,
                                                              .rule_based_interleaving_map = 0x3 };
#endif

/** @} (end addtogroup device_peripheral_dma) */
