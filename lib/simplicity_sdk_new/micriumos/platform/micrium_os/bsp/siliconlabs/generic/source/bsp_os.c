/***************************************************************************//**
 * @file
 * @brief OS BSP
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/********************************************************************************************************
 ********************************************************************************************************
 *                                             INCLUDE FILES
 ********************************************************************************************************
 * Note(s) : (1) The kal_priv.h file is included only to be used by this file. Its content should remain
 *               private to rtos and not be used in an application's context.
 ********************************************************************************************************
 *******************************************************************************************************/

#include  "../include/bsp_os.h"

#include  <common/include/lib_def.h>
#include  <common/include/lib_utils.h>
#include  <common/include/rtos_utils.h>
#include  <common/include/rtos_path.h>
#include  <common/source/kal/kal_priv.h>                        // Private file, use should be limited. See Note #1.
#include  <rtos_description.h>

#ifdef  LEGACY_CONFIG_MODE                                      // Backwards compatibility with legacy configuration files.
#include  <bspconfig.h>
#endif

#ifdef  RTOS_MODULE_KERNEL_AVAIL
#include  <kernel/include/os.h>
#elif  defined(RTOS_MODULE_KERNEL_OS2_AVAIL)
#include  <os.h>
#endif

//                                                                 Third Party Library Includes
#include  <em_chip.h>
#include  <em_cmu.h>
#include  <em_emu.h>

#include  "../include/bsp_opt_def.h"
#include  <bsp_cfg.h>
#include  <sl_sleeptimer_config.h>
#include  "sl_device_init_dcdc.h"

/********************************************************************************************************
 ********************************************************************************************************
 *                                             LOCAL DEFINES
 ********************************************************************************************************
 *******************************************************************************************************/

#if defined(CMU_HFXOINIT_STK_DEFAULT)
#define BSP_HFXO_CONFIG  CMU_HFXOINIT_STK_DEFAULT               // Use STK default HFXO configuration
#elif defined(CMU_HFXOINIT_WSTK_DEFAULT)
#define BSP_HFXO_CONFIG  CMU_HFXOINIT_WSTK_DEFAULT              // Use Wireless STK default HFXO configuration
#else
#define BSP_HFXO_CONFIG  CMU_HFXOINIT_DEFAULT                   // Use standard emlib HFXO configuration
#endif

// Resolve Sleeptimer peripheral in auto mode
#if SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_DEFAULT
#if defined(RTCC_PRESENT) && RTCC_COUNT >= 1
#undef SL_SLEEPTIMER_PERIPHERAL
#define SL_SLEEPTIMER_PERIPHERAL SL_SLEEPTIMER_PERIPHERAL_RTCC
#elif defined(RTC_PRESENT) && RTC_COUNT >= 1
#undef SL_SLEEPTIMER_PERIPHERAL
#define SL_SLEEPTIMER_PERIPHERAL SL_SLEEPTIMER_PERIPHERAL_RTC
#elif defined(SYSRTC_PRESENT) && SYSRTC_COUNT >= 1
#undef SL_SLEEPTIMER_PERIPHERAL
#define SL_SLEEPTIMER_PERIPHERAL SL_SLEEPTIMER_PERIPHERAL_SYSRTC
#elif defined(BURTC_PRESENT) && BURTC_COUNT >= 1
#undef SL_SLEEPTIMER_PERIPHERAL
#define SL_SLEEPTIMER_PERIPHERAL SL_SLEEPTIMER_PERIPHERAL_BURTC
#endif
#endif

/********************************************************************************************************
 ********************************************************************************************************
 *                                       LOCAL FUNCTION PROTOTYPES
 ********************************************************************************************************
 *******************************************************************************************************/

/********************************************************************************************************
 ********************************************************************************************************
 *                                           GLOBAL FUNCTIONS
 ********************************************************************************************************
 *******************************************************************************************************/

/********************************************************************************************************
 *                                           BSP_SystemInit()
 *
 * Description : Initialize the Board Support Package (BSP).
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.
 *
 * Note(s)     : (1) This function MUST be called before any other BSP function is called.
 *******************************************************************************************************/
void BSP_SystemInit(void)
{
  CMU_Osc_TypeDef    sleeptimer_osc;
#if (BSP_HF_CLK_SEL == BSP_HF_CLK_HFXO) || (BSP_HF_CLK_SEL == BSP_HF_CLK_DPLL_48MHZ)
  CMU_HFXOInit_TypeDef hfxo_init = BSP_HFXO_CONFIG;
#endif
#if (BSP_LF_CLK_SEL == BSP_LF_CLK_LFXO)
  CMU_LFXOInit_TypeDef lfxo_init = CMU_LFXOINIT_DEFAULT;
#endif

  CHIP_Init();                                                  // Chip revision alignment and errata fixes
  sl_device_init_dcdc();

#if (BSP_HF_CLK_SEL == BSP_HF_CLK_DEFAULT)
#error "Missing definition of BSP_HF_CLK_SEL in bsp_cfg.h"

#elif (BSP_HF_CLK_SEL == BSP_HF_CLK_HFRCO)
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_CLOCK_SELECT_SET(HF, HFRCO);            // Select reference clock for High Freq. clock
#else
  CMU_CLOCK_SELECT_SET(SYSCLK, HFRCODPLL);        // Select reference clock for High Freq. clock
#endif

#elif (BSP_HF_CLK_SEL == BSP_HF_CLK_HFXO)
  CMU_HFXOInit(&hfxo_init);                                      // Initialize HFXO with board specific parameters
#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_CLOCK_SELECT_SET(HF, HFXO);             // Select reference clock for High Freq. clock
#else
  CMU_CLOCK_SELECT_SET(SYSCLK, HFXO);
#endif

#elif (BSP_HF_CLK_SEL == BSP_HF_CLK_DPLL_48MHZ)
  CMU_HFXOInit(&hfxo_init);                                      // Initialize HFXO with board specific parameters
  CMU_OscillatorEnable(cmuOsc_HFXO, true, true);

#if !defined(_SILICON_LABS_32B_SERIES_2)
#define DPLL_N 479
#define DPLL_M 499

  //                                                               Use HFRCO with DPLL to generate 48 MHz
  //                                                               The CMU_DPLLLock() could probably be used.
  CMU_HFRCOBandSet(cmuHFRCOFreq_48M0Hz);
  CMU->HFRCOCTRL |= CMU_HFRCOCTRL_FINETUNINGEN;
  //                                                               Setup DPLL frequency. The formula is Fdco = Fref*(N+1)/(M+1)
  CMU->DPLLCTRL1 = (DPLL_N << _CMU_DPLLCTRL1_N_SHIFT) | (DPLL_M << _CMU_DPLLCTRL1_M_SHIFT);
  //                                                               Enable DPLL
  CMU->OSCENCMD = CMU_OSCENCMD_DPLLEN;
  //                                                               Set HFClock from HFRCO
  CMU_CLOCK_SELECT_SET(HF, HFRCO);            // Select reference clock for High Freq. clock
#else
  //                                                               Use HFRCO with DPLL to generate 48 MHz
  CMU_HFRCODPLLBandSet(cmuHFRCODPLLFreq_48M0Hz);
  CMU_CLOCK_SELECT_SET(SYSCLK, HFRCODPLL);        // Select reference clock for High Freq. clock
#endif

#else
#error "The definition of BSP_HF_CLK_SEL in bsp_cfg.h is not handled."
#endif

#if !defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_HFLE, true);
#endif

#if ((SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RTCC)   \
  || (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RTC)    \
  || (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_SYSRTC) \
  || (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_BURTC))

#if (BSP_LF_CLK_SEL == BSP_LF_CLK_LFXO)
  CMU_LFXOInit(&lfxo_init);
  sleeptimer_osc = cmuOsc_LFXO;
#if (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RTCC)
  #if defined(_SILICON_LABS_32B_SERIES_2)
  CMU_CLOCK_SELECT_SET(RTCC, LFXO);
  #else
  CMU_CLOCK_SELECT_SET(LFE, LFXO);
  #endif
#elif (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RTC)
  CMU_CLOCK_SELECT_SET(LFA, LFXO);
#elif (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_SYSRTC)
  CMU_CLOCK_SELECT_SET(SYSRTC, LFXO);
#elif (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_BURTC)
  CMU_CLOCK_SELECT_SET(BURTC, LFXO);
#endif
#elif (BSP_LF_CLK_SEL == BSP_LF_CLK_LFRCO)
  sleeptimer_osc = cmuOsc_LFRCO;
#if (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RTCC)
  #if defined(_SILICON_LABS_32B_SERIES_2)
  CMU_CLOCK_SELECT_SET(RTCC, LFRCO);
  #else
  CMU_CLOCK_SELECT_SET(LFE, LFRCO);
  #endif
#elif (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RTC)
  CMU_CLOCK_SELECT_SET(LFA, LFRCO);
#elif (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_SYSRTC)
  CMU_CLOCK_SELECT_SET(SYSRTC, LFRCO);
#elif (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_BURTC)
  CMU_CLOCK_SELECT_SET(BURTC, LFRCO);
#endif
#else
  #error "The definition of BSP_LF_CLK_SEL in bsp_cfg.h is not handled."
#endif
#elif (SL_SLEEPTIMER_PERIPHERAL != SL_SLEEPTIMER_PERIPHERAL_PRORTC)
  #error "Invalid peripheral selected for SL_SLEEPTIMER in sl_sleeptimer_config.h"
#endif

  CMU_OscillatorEnable(sleeptimer_osc, true, false);
}

/********************************************************************************************************
 *                                              BSP_Init()
 *
 * Description : Initializes peripheral.
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.
 *
 * Note(s)     : (1) This function must be called after KAL has been initialized.
 *******************************************************************************************************/
void BSP_OS_Init(void)
{
}
