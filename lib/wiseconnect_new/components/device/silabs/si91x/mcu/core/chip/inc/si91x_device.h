/******************************************************************************
* @file  si91x_device.h
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

/** @addtogroup Silicon Lab Inc.
 * @{
 */

/** @addtogroup RS1xxxx
 * @{
 */

#ifndef __RS1XXXX_H__
#define __RS1XXXX_H__

#include "base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Configuration_of_CMSIS
  * @{
  */

/* ===========================================================================================================================
  */
/* ================                                Interrupt Number Definition
  * ================ */
/* ===========================================================================================================================
  */

typedef enum {
  /* =======================================  ARM Cortex-M4 Specific Interrupt
      Numbers  ======================================== */
  Reset_IRQn            = -15, /*!< -15  Reset Vector, invoked on Power up and warm reset */
  NonMaskableInt_IRQn   = -14, /*!< -14  Non maskable Interrupt, cannot be stopped or preempted */
  HardFault_IRQn        = -13, /*!< -13  Hard Fault, all classes of Fault        */
  MemoryManagement_IRQn = -12, /*!< -12  Memory Management, MPU mismatch,
                                   including Access Violation and No Match */
  BusFault_IRQn         = -11, /*!< -11  Bus Fault, Pre-Fetch-, Memory Access Fault,
                              other address/memory    related Fault    */
  UsageFault_IRQn       = -10, /*!< -10  Usage Fault, i.e. Undef Instruction, Illegal
                              State Transition        */
  SVCall_IRQn           = -5,  /*!< -5 System Service Call via SVC instruction       */
  DebugMonitor_IRQn     = -4,  /*!< -4 Debug Monitor */
  PendSV_IRQn           = -2,  /*!< -2 Pendable request for system service       */
  SysTick_IRQn          = -1,  /*!< -1 System Tick Timer      */
  /* ==========================================  RS1xxxx Specific Interrupt
      Numbers  =========================================== */
  TIMER0_IRQn                = 2,  /*!< 2  TIMER0                 */
  TIMER1_IRQn                = 3,  /*!< 3  TIMER1                 */
  TIMER2_IRQn                = 4,  /*!< 4  TIMER2                 */
  TIMER3_IRQn                = 5,  /*!< 5  TIMER3                 */
  CAP_SENSOR_IRQn            = 6,  /*!< 6  CAP_SENSOR             */
  COMP2_IRQn                 = 7,  /*!< 7  COMP2                  */
  COMP1_IRQn                 = 8,  /*!< 8  COMP1                  */
  UDMA1_IRQn                 = 10, /*!< 10 UDMA1                 */
  ADC_IRQn                   = 11, /*!< 11 ADC                   */
  ULPSS_UART_IRQn            = 12, /*!< 12 ULPSS_UART            */
  I2C2_IRQn                  = 13, /*!< 13 I2C2                  */
  I2S1_IRQn                  = 14, /*!< 14 I2S1                  */
  IR_DECODER_IRQn            = 15, /*!< 15 IR_DECODER            */
  SSI2_IRQn                  = 16, /*!< 16 SSI2                  */
  FIM_IRQn                   = 17, /*!< 17 FIM                   */
  ULP_EGPIO_PIN_IRQn         = 18, /*!< 18 ULP_EGPIO_PIN         */
  ULP_EGPIO_GROUP_IRQn       = 19, /*!< 19 ULP_EGPIO_GROUP       */
  NPSS_TO_MCU_WDT_INTR_IRQn  = 20, /*!< 20 NPSS_TO_MCU_WDT_INTR  */
  NPSS_TO_MCU_GPIO_INTR_IRQn = 21, /*!< 21 NPSS_TO_MCU_GPIO_INTR */
#if defined(SLI_SI917B0)
  NPSS_TO_MCU_SYRTC_INTR_IRQn = 22, /*!< 22 NPSS_TO_MCU_SYSRTC_INTR */
#else
  NPSS_TO_MCU_CMP_RF_WKP_INTR_IRQn = 22,               /*!< 22 NPSS_TO_MCU_CMP_RF_WKP_INTR                           */
#endif
  NPSS_TO_MCU_BOD_INTR_IRQn      = 23, /*!< 23 NPSS_TO_MCU_BOD_INTR      */
  NPSS_TO_MCU_BUTTON_INTR_IRQn   = 24, /*!< 24 NPSS_TO_MCU_BUTTON_INTR   */
  NPSS_TO_MCU_SDC_INTR_IRQn      = 25, /*!< 25 NPSS_TO_MCU_SDC_INTR      */
  NPSS_TO_MCU_WIRELESS_INTR_IRQn = 26, /*!< 26 NPSS_TO_MCU_WIRELESS_INTR */
  NPSS_MCU_INTR_IRQn             = 27, /*!< 27 NPSS_MCU_INTR             */
  MCU_CAL_ALARM_IRQn             = 28, /*!< 28 MCU_CAL_ALARM             */
  MCU_CAL_RTC_IRQn               = 29, /*!< 29 MCU_CAL_RTC               */
  GPDMA_IRQn                     = 31, /*!< 31 GPDMA                     */
  UDMA0_IRQn                     = 33, /*!< 33 UDMA0                     */
  CT_IRQn                        = 34, /*!< 34 CT                        */
  HIF0_IRQn                      = 35, /*!< 35 HIF0                      */
  HIF1_IRQn                      = 36, /*!< 36 HIF1                      */
  SIO_IRQn                       = 37, /*!< 37 SIO                       */
  USART0_IRQn                    = 38, /*!< 38 USART0                    */
  UART1_IRQn                     = 39, /*!< 39 UART1                     */
  EGPIO_WAKEUP_IRQn              = 41, /*!<  41  EGPIO_WAKEUP                  */
  I2C0_IRQn                      = 42, /*!<  42  I2C0          */
  SSISlave_IRQn                  = 44, /*!< 44 SSISlave      */
  GSPI0_IRQn                     = 46, /*!< 46 GSPI0         */
  SSI0_IRQn                      = 47, /*!< 47 SSI0          */
  MCPWM_IRQn                     = 48, /*!< 48 MCPWM         */
  QEI_IRQn                       = 49, /*!< 49 QEI           */
  EGPIO_GROUP_0_IRQn             = 50, /*!< 50 EGPIO_GROUP_0 */
  EGPIO_GROUP_1_IRQn             = 51, /*!< 51 EGPIO_GROUP_1 */
  EGPIO_PIN_0_IRQn               = 52, /*!< 52 EGPIO_PIN_0   */
  EGPIO_PIN_1_IRQn               = 53, /*!< 53 EGPIO_PIN_1   */
  EGPIO_PIN_2_IRQn               = 54, /*!< 54 EGPIO_PIN_2   */
  EGPIO_PIN_3_IRQn               = 55, /*!< 55 EGPIO_PIN_3   */
  EGPIO_PIN_4_IRQn               = 56, /*!< 56 EGPIO_PIN_4   */
  EGPIO_PIN_5_IRQn               = 57, /*!< 57 EGPIO_PIN_5   */
  EGPIO_PIN_6_IRQn               = 58, /*!< 58 EGPIO_PIN_6   */
  EGPIO_PIN_7_IRQn               = 59, /*!< 59 EGPIO_PIN_7   */
  QSPI_IRQn                      = 60, /*!< 60 QSPI          */
  I2C1_IRQn                      = 61, /*!< 61 I2C1          */
#if defined(SLI_SI917B0)
  MVP_IRQn        = 62, /*!< 62  MVP        */
  MVP_WAKEUP_IRQn = 63, /*!< 63  MVP_WAKEUP */
#endif
  I2S0_IRQn      = 64, /*!< 64 I2S0      */
  PLL_CLOCK_IRQn = 69, /*!< 69 PLL_CLOCK */
  TASS_P2P_IRQn  = 74  /*!< 74 TASS_P2P   */
} IRQn_Type;

/* ===========================================================================================================================
  */
/* ================                           Processor and Core Peripheral
  * Section                           ================ */
/* ===========================================================================================================================
  */

/* ===========================  Configuration of the ARM Cortex-M4 Processor and
  * Core Peripherals  =========================== */
#define __CM4_REV              0x0100U /*!< CM4 Core Revision */
#define __NVIC_PRIO_BITS       6       /*!< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig 0       /*!< Set to 1 if different SysTick Config is used */
#define __MPU_PRESENT          1       /*!< MPU present */
#define __FPU_PRESENT          1       /*!< FPU present */

/** @} */ /* End of group Configuration_of_CMSIS */

#include "core_cm4.h"     /*!< ARM Cortex-M4 processor and core peripherals                              */
#include "system_si91x.h" /*!< RS1xxxx System                                                            */

#ifndef __IM /*!< Fallback for older CMSIS versions */
#define __IM __I
#endif
#ifndef __OM /*!< Fallback for older CMSIS versions */
#define __OM __O
#endif
#ifndef __IOM /*!< Fallback for older CMSIS versions */
#define __IOM __IO
#endif

/* ========================================  Start of section using anonymous
  * unions  ======================================== */
#if defined(__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc11-extensions"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
#pragma warning 586
#elif defined(__CSMC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif

/* ===========================================================================================================================
  */
/* ================                              Device Specific Cluster Section
  * ================ */
/* ===========================================================================================================================
  */

/** @addtogroup Device_Peripheral_clusters
  * @{
  */

/**
  * @brief MCPWM_PWM_DEADTIME [PWM_DEADTIME] ([0..3])
  */
typedef struct {
  union {
    __IOM unsigned int PWM_DEADTIME_A; /*!< (@ 0x00000000) PWM deadtime for A and
                                       channel varies from 0 to 3 */

    struct {
      __IOM unsigned int DEADTIME_A_CH : 6; /*!< [5..0] Dead time A value to load into dead
                                 time counter A of channel0 to channel3 */
      __IOM unsigned int RESERVED1 : 26;    /*!< [31..6] reserved1 */
    } PWM_DEADTIME_A_b;
  };

  union {
    __IOM unsigned int PWM_DEADTIME_B; /*!< (@ 0x00000004) PWM deadtime for B and
                                       channel varies from 0 to 3 */

    struct {
      __IOM unsigned int DEADTIME_B_CH : 6; /*!< [5..0] Dead time B value to load into deadtime
                                 counter B of channel0 to channel3 */
      __IOM unsigned int RESERVED1 : 26;    /*!< [31..6] reserved1 */
    } PWM_DEADTIME_B_b;
  };
} MCPWM_PWM_DEADTIME_Type; /*!< Size = 8 (0x8) */

/**
  * @brief GPDMA_G_GLOBAL [GLOBAL] (GLOBAL)
  */
typedef struct {
  union {
    __IOM unsigned int INTERRUPT_REG; /*!< (@ 0x00000000) Interrupt Register */

    struct {
      __IOM unsigned int GPDMAC_INT_STAT : 8; /*!< [7..0] Interrupt Status */
      __IM unsigned int RESERVED1 : 24;       /*!< [31..8] reserved1       */
    } INTERRUPT_REG_b;
  };

  union {
    __IOM unsigned int INTERRUPT_MASK_REG; /*!< (@ 0x00000004) Interrupt Mask Register */

    struct {
      __IOM unsigned int RESERVED1 : 8;            /*!< [7..0] reserved1 */
      __IOM unsigned int LINK_LIST_FETCH_MASK : 8; /*!< [15..8] Linked list fetch done
                                            interrupt bit mask control. By     default,
                                            descriptor fetch done interrupt is
                                            masked.                       */
      __IOM unsigned int TFR_DONE_MASK : 8;        /*!< [23..16] Transfer done interrupt
                                            bit mask control. */
      __IOM unsigned int RESERVED2 : 8;            /*!< [31..24] reserved2     */
    } INTERRUPT_MASK_REG_b;
  };

  union {
    __IOM unsigned int INTERRUPT_STAT_REG; /*!< (@ 0x00000008) Interrupt status register */

    struct {
      __IOM unsigned int HRESP_ERR0 : 1;            /*!< [0..0] DMA error bit */
      __IOM unsigned int LINK_LIST_FETCH_DONE0 : 1; /*!< [1..1] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 0 */
      __IOM unsigned int TFR_DONE0 : 1;             /*!< [2..2] This bit indicates the status of DMA
                             transfer done interrupt for channel 0 */
      __IOM unsigned int GPDMAC_ERR0 : 1;           /*!< [3..3] transfer size or burst size or
                                          h size mismatch error               */
      __IOM unsigned int HRESP_ERR1 : 1;            /*!< [4..4] HRESP error bit  */
      __IOM unsigned int LINK_LIST_FETCH_DONE1 : 1; /*!< [5..5] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 1 */
      __IOM unsigned int TFR_DONE1 : 1;             /*!< [6..6] This bit indicates the status of DMA
                             transfer done interrupt for channel 1. */
      __IOM unsigned int GPDMAC_ERR1 : 1;           /*!< [7..7] transfer size or burst size or
                                          h size mismatch error               */
      __IOM unsigned int HRESP_ERR2 : 1;            /*!< [8..8] HRESP error bit  */
      __IOM unsigned int LINK_LIST_FETCH_DONE2 : 1; /*!< [9..9] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 2. */
      __IOM unsigned int TFR_DONE2 : 1;             /*!< [10..10] This bit indicates the status of DMA
                             transfer done interrupt for channel 2. */
      __IOM unsigned int GPDMAC_ERR2 : 1;           /*!< [11..11] transfer size or burst size
                                          or h size mismatch error             */
      __IOM unsigned int HRESP_ERR3 : 1;            /*!< [12..12] HRESP error bit  */
      __IOM unsigned int LINK_LIST_FETCH_DONE3 : 1; /*!< [13..13] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 3. */
      __IOM unsigned int TFR_DONE3 : 1;             /*!< [14..14] This bit indicates the status of DMA
                             transfer done interrupt for channel 3. */
      __IOM unsigned int GPDMAC_ERR3 : 1;           /*!< [15..15] transfer size or burst size
                                          or h size mismatch error             */
      __IOM unsigned int HRESP_ERR4 : 1;            /*!< [16..16] HRESP error bit  */
      __IOM unsigned int LINK_LIST_FETCH_DONE4 : 1; /*!< [17..17] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 4. */
      __IOM unsigned int TFR_DONE4 : 1;             /*!< [18..18] This bit indicates the status of DMA
                             transfer done interrupt for channel 4. */
      __IOM unsigned int GPDMAC_ERR4 : 1;           /*!< [19..19] transfer size or burst size
                                          or h size mismatch error             */
      __IOM unsigned int HRESP_ERR5 : 1;            /*!< [20..20] HRESP error bit  */
      __IOM unsigned int LINK_LIST_FETCH_DONE5 : 1; /*!< [21..21] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 5. */
      __IOM unsigned int TFR_DONE5 : 1;             /*!< [22..22] This bit indicates the status of DMA
                             transfer done interrupt for channel 5. */
      __IOM unsigned int GPDMAC_ERR5 : 1;           /*!< [23..23] transfer size or burst size
                                          or h size mismatch error             */
      __IM unsigned int HRESP_ERR6 : 1;             /*!< [24..24] HRESP error bit   */
      __IOM unsigned int LINK_LIST_FETCH_DONE6 : 1; /*!< [25..25] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 6. */
      __IOM unsigned int TFR_DONE6 : 1;             /*!< [26..26] This bit indicates the status of DMA
                             transfer done interrupt for channel 6. */
      __IOM unsigned int GPDMAC_ERR6 : 1;           /*!< [27..27] transfer size or burst size
                                          or h size mismatch error             */
      __IOM unsigned int HRESP_ERR7 : 1;            /*!< [28..28] HRESP error bit  */
      __IOM unsigned int LINK_LIST_FETCH_DONE7 : 1; /*!< [29..29] This bit indicates the status
                                         of linked list descriptor
                                           fetch done for channel 7. */
      __IOM unsigned int TFR_DONE7 : 1;             /*!< [30..30] This bit indicates the status of DMA
                             transfer done interrupt for channel 7. */
      __IOM unsigned int GPDMAC_ERR7 : 1;           /*!< [31..31] transfer size or burst size
                                          or h size mismatch error             */
    } INTERRUPT_STAT_REG_b;
  };

  union {
    __IOM unsigned int DMA_CHNL_ENABLE_REG; /*!< (@ 0x0000000C) This register used
                                            for enable DMA channel */

    struct {
      __IOM unsigned int CH_ENB : 8;    /*!< [7..0] CWhen a bit is set to one, it indicates,
                          corresponding channel is enabled for dma operation */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] Reserved1 */
    } DMA_CHNL_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int DMA_CHNL_SQUASH_REG; /*!< (@ 0x00000010) This register used
                                            for enable DMA channel squash */

    struct {
      __IOM unsigned int CH_DIS : 8;    /*!< [7..0] CPU Will be masked to write zeros,
                                     CPU is allowed write 1 only */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] Reserved1 */
    } DMA_CHNL_SQUASH_REG_b;
  };

  union {
    __IOM unsigned int DMA_CHNL_LOCK_REG; /*!< (@ 0x00000014) This register used for
                                          enable DMA channel squash           */

    struct {
      __IOM unsigned int CHNL_LOCK : 8; /*!< [7..0] When set entire DMA block transfer is done,
                             before other DMA request is serviced */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] Reserved1 */
    } DMA_CHNL_LOCK_REG_b;
  };
} GPDMA_G_GLOBAL_Type; /*!< Size = 24 (0x18) */

/**
  * @brief GPDMA_C_CHANNEL_CONFIG [CHANNEL_CONFIG] ([0..7])
  */
typedef struct {
  union {
    __IOM unsigned int LINK_LIST_PTR_REGS; /*!< (@ 0x00000000) Link List Register
                                           for channel 0 to 7 */

    struct {
      __IOM unsigned int LINK_LIST_PTR_REG_CHNL : 32; /*!< [31..0] This is the address of the
                                           memory location from which
                                           we get our next descriptor */
    } LINK_LIST_PTR_REGS_b;
  };

  union {
    __IOM unsigned int SRC_ADDR_REG_CHNL; /*!< (@ 0x00000004) Source Address
                                          Register for channel 0 to 7 */

    struct {
      __IOM unsigned int SRC_ADDR : 32; /*!< [31..0] This is the address of the memory location
                             from which we get our next descriptor */
    } SRC_ADDR_REG_CHNL_b;
  };

  union {
    __IOM unsigned int DEST_ADDR_REG_CHNL; /*!< (@ 0x00000008) Source Address
                                           Register for channel 0 to 7 */

    struct {
      __IOM unsigned int DEST_ADDR : 32; /*!< [31..0] This is the destination
                                         address to whih the data is sent */
    } DEST_ADDR_REG_CHNL_b;
  };

  union {
    __IOM unsigned int CHANNEL_CTRL_REG_CHNL; /*!< (@ 0x0000000C) Channel Control
                                              Register for channel 0 to 7 */

    struct {
      __IOM unsigned int DMA_BLK_SIZE : 12;        /*!< [11..0] This is data to be transmitted. Loaded
                                 at the beginning of the DMA transfer and
                                 decremented at every dma transaction. */
      __IOM unsigned int TRNS_TYPE : 2;            /*!< [13..12] DMA transfer type     */
      __IOM unsigned int DMA_FLOW_CTRL : 2;        /*!< [15..14] DMA flow control */
      __IOM unsigned int MSTR_IF_FETCH_SEL : 1;    /*!< [16..16] This selects the MASTER IF from
                                     which data to be fetched         */
      __IOM unsigned int MSTR_IF_SEND_SEL : 1;     /*!< [17..17] This selects the MASTER
                                               IF from which data to be sent */
      __IOM unsigned int DEST_DATA_WIDTH : 2;      /*!< [19..18] Data transfer to destination. */
      __IOM unsigned int SRC_DATA_WIDTH : 2;       /*!< [21..20] Data transfer from source. */
      __IOM unsigned int SRC_ALIGN : 1;            /*!< [22..22] Reserved.Value set to 0 We do not do any
                             singles. We just do burst, save first 3 bytes in to
                             residue buffer in one cycle,  In the next cycle send
                             4 bytes to fifo, save 3 bytes in to residue. This
                             continues on.                            */
      __IOM unsigned int LINK_LIST_ON : 1;         /*!< [23..23] This mode is set, when we
                                           do link listed operation */
      __IOM unsigned int LINK_LIST_MSTR_SEL : 1;   /*!< [24..24] This mode is set, when we do
                                      link listed operation               */
      __IOM unsigned int SRC_ADDR_CONTIGUOUS : 1;  /*!< [25..25] Indicates Address is
                                                  contiguous from previous */
      __IOM unsigned int DEST_ADDR_CONTIGUOUS : 1; /*!< [26..26] Indicates Address is
                                             contiguous from previous      */
      __IOM unsigned int RETRY_ON_ERROR : 1;       /*!< [27..27] When this bit is set, if
                                             we recieve HRESPERR, We will retry
                                             the DMA for that channel. */
      __IOM unsigned int LINK_INTERRUPT : 1;       /*!< [28..28] This bit is set in link list
                                  descriptor.Hard ware will send an interrupt
                                  when the DMA transfer is done for the
                                           corresponding link list address */
      __IOM unsigned int SRC_FIFO_MODE : 1;        /*!< [29..29] If set to 1; source address will not
                                 be incremented(means fifo mode for source) */
      __IOM unsigned int DEST_FIFO_MODE : 1;       /*!< [30..30] If set to 1; destination address
                                  will not be incremented(means fifo mode for
                                  destination) */
      __IM unsigned int RESERVED1 : 1;             /*!< [31..31] Reserved1 */
    } CHANNEL_CTRL_REG_CHNL_b;
  };

  union {
    __IOM unsigned int MISC_CHANNEL_CTRL_REG_CHNL; /*!< (@ 0x00000010) Misc Channel Control
                                        Register for channel 0                */

    struct {
      __IOM unsigned int AHB_BURST_SIZE : 3;  /*!< [2..0] Burst size  */
      __IOM unsigned int DEST_DATA_BURST : 6; /*!< [8..3] Burst writes in beats to
                                              destination.(000000-64 beats
                                                      .....111111-63 beats) */
      __IOM unsigned int SRC_DATA_BURST : 6;  /*!< [14..9] Burst writes in beats from
                                             source(000000-64 beats
                                             .....111111-63 beats) */
      __IOM unsigned int DEST_CHNL_ID : 6;    /*!< [20..15] This is the destination channel Id to
                                which the data is sent. Must be set up prior to
                                DMA_CHANNEL_ENABLE                       */
      __IOM
      unsigned int SRC_CHNL_ID : 6;           /*!< [26..21] This is the source channel Id,
                                        from which the data is fetched. must be
                                        set up prior to DMA_CHANNEL_ENABLE */
      __IOM unsigned int DMA_PROT : 3;        /*!< [29..27] Protection level to go with the data. It
                            will be concatenated with 1 b1 as there will be no
                            opcode fetching and directly assign to hprot in AHB
                            interface                                          */
      __IOM unsigned int MEM_FILL_ENABLE : 1; /*!< [30..30] Enable for memory
                                              filling with either 1s or 0s. */
      __IOM unsigned int MEM_ONE_FILL : 1;    /*!< [31..31] Select for memory filling
                                           with either 1s or 0s. */
    } MISC_CHANNEL_CTRL_REG_CHNL_b;
  };

  union {
    __IOM unsigned int FIFO_CONFIG_REGS; /*!< (@ 0x00000014) FIFO Configuration
                                         Register for channel 1 */

    struct {
      __IOM unsigned int FIFO_STRT_ADDR : 6; /*!< [5..0] Starting row address of channel */
      __IOM unsigned int FIFO_SIZE : 6;      /*!< [11..6] Channel size */
      __IM unsigned int RESERVED1 : 20;      /*!< [31..12] Reserved1 */
    } FIFO_CONFIG_REGS_b;
  };

  union {
    __IOM unsigned int PRIORITY_CHNL_REGS; /*!< (@ 0x00000018) Priority Register for
                                           channel 0 to 7 */

    struct {
      __IOM unsigned int PRIORITY_CH : 2; /*!< [1..0] Set a value between 2 b00 to 2 b11. The
                               channel having highest number is the highest
                               priority channel.                           */
      __IM unsigned int RESERVED1 : 30;   /*!< [31..2] Reserved1 */
    } PRIORITY_CHNL_REGS_b;
  };
  __IM unsigned int RESERVED[57];
} GPDMA_C_CHANNEL_CONFIG_Type; /*!< Size = 256 (0x100) */

/**
  * @brief TIMERS_MATCH_CTRL [MATCH_CTRL] ([0..3])
  */
typedef struct {
  union {
    __IOM unsigned int MCUULP_TMR_MATCH; /*!< (@ 0x00000000) Timer Match Register */

    struct {
      __IOM unsigned int TMR_MATCH : 32; /*!< [31..0] This bits are used to program the lower
                              significant 16-bits of  timer time out value in
                              millisecond or number of system clocks */
    } MCUULP_TMR_MATCH_b;
  };

  union {
    __IOM unsigned int MCUULP_TMR_CNTRL; /*!< (@ 0x00000004) Timer Control Register */

    struct {
      __OM unsigned int TMR_START : 1;        /*!< [0..0] This Bit are Used to start the timer timer
                             gets reset upon setting this bit */
      __OM unsigned int TMR_INTR_CLR : 1;     /*!< [1..1] This Bit are Used to clear the
                                          timer                               */
      __IOM unsigned int TMR_INTR_ENABLE : 1; /*!< [2..2] This Bit are Used to
                                              enable the time out interrupt */
      __IOM unsigned int TMR_TYPE : 2;        /*!< [4..3] This Bit are Used to select the
                                       type of timer                      */
      __IOM unsigned int TMR_MODE : 1;        /*!< [5..5] This Bit are Used to select the
                                       mode working of timer              */
      __OM unsigned int TMR_STOP : 1;         /*!< [6..6] This Bit are Used to stop the timer */
      __IOM unsigned int COUNTER_UP : 1;      /*!< [7..7] For reading/tracking counter in
                                         up counting this bit has to be set */
      __IOM unsigned int RESERVED1 : 24;      /*!< [31..8] reserved1 */
    } MCUULP_TMR_CNTRL_b;
  };
} TIMERS_MATCH_CTRL_Type; /*!< Size = 8 (0x8) */

/**
  * @brief I2S0_CHANNEL_CONFIG [CHANNEL_CONFIG] ([0..3])
  */
typedef struct {
  union {
    union {
      __IM unsigned int I2S_LRBR; /*!< (@ 0x00000000) Left Receive Buffer Register */

      struct {
        __IM unsigned int LRBR : 24;     /*!< [23..0] Data received serially from the
                                     received channel input            */
        __IM unsigned int RESERVED1 : 8; /*!< [31..24] Reserved for future use */
      } I2S_LRBR_b;
    };

    union {
      __OM unsigned int I2S_LTHR; /*!< (@ 0x00000000) Left Receive Buffer Register */

      struct {
        __OM unsigned int LTHR : 24;     /*!< [23..0] The Left Stereo Data to be transmitted
                           serially from the Transmitted channel output */
        __OM unsigned int RESERVED1 : 8; /*!< [31..24] Reserved for future use */
      } I2S_LTHR_b;
    };
  };

  union {
    union {
      __IM unsigned int I2S_RRBR; /*!< (@ 0x00000004) Right Receive Buffer Register */

      struct {
        __IM unsigned int RRBR : 24;     /*!< [23..0] The Right Stereo Data received serially from
                           the received channel input through this register */
        __IM unsigned int RESERVED1 : 8; /*!< [31..24] Reserved for future use */
      } I2S_RRBR_b;
    };

    union {
      __OM unsigned int I2S_RTHR; /*!< (@ 0x00000004) Right Transmit Holding Register */

      struct {
        __OM unsigned int RTHR : 24;     /*!< [23..0] The Right Stereo Data to be transmitted
                           serially from the Transmit channel output written
                           through this register                 */
        __OM unsigned int RESERVED1 : 8; /*!< [31..24] Reserved for future use */
      } I2S_RTHR_b;
    };
  };

  union {
    __IOM unsigned int I2S_RER; /*!< (@ 0x00000008) Receive Enable Register */

    struct {
      __IOM unsigned int RXCHEN : 1;     /*!< [0..0] This Bit enables/disables a receive channel
                          independently of all other channels */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_RER_b;
  };

  union {
    __IOM unsigned int I2S_TER; /*!< (@ 0x0000000C) Transmit Enable Register */

    struct {
      __IOM unsigned int TXCHEN : 1;     /*!< [0..0] This Bit enables/disables a transmit channel
                          independently of all other channels */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_TER_b;
  };

  union {
    __IOM unsigned int I2S_RCR; /*!< (@ 0x00000010) Receive Configuration Register */

    struct {
      __IOM unsigned int WLEN : 3;       /*!< [2..0] This Bits are used to program the desired data
                        resolution of the receiver and enables LSB of the
                        incoming left or right word */
      __IOM unsigned int RESERVED1 : 29; /*!< [31..3] Reserved for future use */
    } I2S_RCR_b;
  };

  union {
    __IOM unsigned int I2S_TCR; /*!< (@ 0x00000014) Transmit Configuration Register */

    struct {
      __IOM unsigned int WLEN : 3;       /*!< [2..0] This Bits are used to program the desired data
                        resolution of the transmitter and ensure that MSB of the
                        data is transmitted first. */
      __IOM unsigned int RESERVED1 : 29; /*!< [31..3] Reserved for future use */
    } I2S_TCR_b;
  };

  union {
    __IM unsigned int I2S_ISR; /*!< (@ 0x00000018) Interrupt Status Register */

    struct {
      __IM unsigned int RXDA : 1;       /*!< [0..0] Receive Data Available       */
      __IM unsigned int RXFO : 1;       /*!< [1..1] Receive Data FIFO       */
      __IM unsigned int RESERVED1 : 2;  /*!< [3..2] Reserved for future use  */
      __IM unsigned int TXFE : 1;       /*!< [4..4] Transmit FIFO Empty       */
      __IM unsigned int TXFO : 1;       /*!< [5..5] Transmit FIFO       */
      __IM unsigned int RESERVED2 : 26; /*!< [31..6] Reserved for future use */
    } I2S_ISR_b;
  };

  union {
    __IOM unsigned int I2S_IMR; /*!< (@ 0x0000001C) Interrupt Mask Register */

    struct {
      __IOM unsigned int RXDAM : 1;      /*!< [0..0] RX Data Available Mask Interrupt            */
      __IOM unsigned int RXFOM : 1;      /*!< [1..1] RX FIFO Overrun Mask Interrupt */
      __IOM unsigned int RESERVED1 : 2;  /*!< [3..2] Reserved for future use  */
      __IOM unsigned int TXFEM : 1;      /*!< [4..4] TX FIFO Empty Interrupt      */
      __IOM unsigned int TXFOM : 1;      /*!< [5..5] TX FIFO Overrun Interrupt      */
      __IOM unsigned int RESERVED2 : 26; /*!< [31..6] Reserved for future use */
    } I2S_IMR_b;
  };

  union {
    __IM unsigned int I2S_ROR; /*!< (@ 0x00000020) Receive Overrun Register */

    struct {
      __IM unsigned int RXCHO : 1;      /*!< [0..0] Read this bit to clear the RX FIFO
                                   data overrun interrupt          */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_ROR_b;
  };

  union {
    __IM unsigned int I2S_TOR; /*!< (@ 0x00000024) Transmit Overrun Register */

    struct {
      __IM unsigned int TXCHO : 1;      /*!< [0..0] Read this bit to clear the TX FIFO
                                   data overrun interrupt          */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_TOR_b;
  };

  union {
    __IOM unsigned int I2S_RFCR; /*!< (@ 0x00000028) Receive FIFO Configuration Register0 */

    struct {
      __IOM unsigned int RXCHDT : 4;     /*!< [3..0] This bits program the trigger level in the RX
                          FIFO  at which the data available interrupt is
                          generated                           */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved for future use */
    } I2S_RFCR_b;
  };

  union {
    __IOM unsigned int I2S_TXFCR; /*!< (@ 0x0000002C) Transmit FIFO Configuration Register */

    struct {
      __IOM
      unsigned int TXCHET : 4;          /*!< [3..0] This bits program the trigger level
                                   in the TX FIFO  at which the Empty Threshold
                                   Reached interrupt is generated */
      __IM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved for future use */
    } I2S_TXFCR_b;
  };

  union {
    __OM unsigned int I2S_RFF; /*!< (@ 0x00000030) Receive FIFO Flush */

    struct {
      __OM unsigned int RXCHFR : 1;     /*!< [0..0] Writing a 1 to this register flushes an
                          individual RX FIFO RX channel or block must be disable
                          prior to writing to this bit */
      __OM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_RFF_b;
  };

  union {
    __OM unsigned int I2S_TFF; /*!< (@ 0x00000034) Transmit FIFO Flush */

    struct {
      __OM unsigned int TXCHFR : 1;     /*!< [0..0] Writing a 1 to this register flushes an
                          individual TX FIFO TX channel or block must be disable
                          prior to writing to this bit */
      __OM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_TFF_b;
  };
  __IM unsigned int RSVD0;  /*!< (@ 0x00000038) none      */
  __IM unsigned int RSVD1;  /*!< (@ 0x0000003C) none      */
} I2S0_CHANNEL_CONFIG_Type; /*!< Size = 64 (0x40) */

/**
  * @brief EGPIO_PIN_CONFIG [PIN_CONFIG] ([0..79])
  */
typedef struct {
  union {
    __IOM unsigned int GPIO_CONFIG_REG; /*!< (@ 0x00000000) GPIO Configuration Register */

    struct {
      __IOM unsigned int DIRECTION : 1;                 /*!< [0..0] Direction of the GPIO pin */
      __IOM unsigned int PORTMASK : 1;                  /*!< [1..1] Port mask value  */
      __IOM unsigned int MODE : 4;                      /*!< [5..2] GPIO Pin Mode Used for GPIO Pin Muxing */
      __IOM unsigned int RESERVED1 : 2;                 /*!< [7..6] Reserved1 */
      __IOM unsigned int GROUP_INTERRUPT1_ENABLE : 1;   /*!< [8..8] When set, the corresponding
                                         GPIO is pin is selected for
                                         group intr 1 generation */
      __IOM unsigned int GROUP_INTERRUPT1_POLARITY : 1; /*!< [9..9] Decides the active value of
                                           the pin to be considered for group
                                           interrupt 1 generation */
      __IOM unsigned int GROUP_INTERRUPT2_ENABLE : 1;   /*!< [10..10] When set, the corresponding
                                         GPIO is pin is selected
                                         for group intr 2 generation */
      __IOM unsigned int GROUP_INTERRUPT2_POLARITY : 1; /*!< [11..11] Decides the active value
                                           of the pin to be considered
                                           for group interrupt 2 generation */
      __IOM unsigned int RESERVED2 : 4;                 /*!< [15..12] Reserved2      */
      __IOM unsigned int RESERVED3 : 16;                /*!< [31..16] Reserved3     */
    } GPIO_CONFIG_REG_b;
  };

  union {
    __IOM unsigned int BIT_LOAD_REG; /*!< (@ 0x00000004) Bit Load */

    struct {
      __IOM unsigned int BIT_LOAD : 1;   /*!< [0..0] Loads 0th bit on to the pin on write. And
                            reads the value on pin on read into 0th bit */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved1 */
    } BIT_LOAD_REG_b;
  };

  union {
    __IOM unsigned int WORD_LOAD_REG; /*!< (@ 0x00000008) Word Load */

    struct {
      __IOM unsigned int WORD_LOAD : 16; /*!< [15..0] Loads 1 on the pin when any of the bit in
                              load value is 1. On read pass the bit status into
                              all bits.                          */
      __IOM unsigned int RESERVED1 : 16; /*!< [31..16] Reserved1 */
    } WORD_LOAD_REG_b;
  };
  __IM unsigned int RESERVED;
} EGPIO_PIN_CONFIG_Type; /*!< Size = 16 (0x10) */

/**
  * @brief EGPIO_PORT_CONFIG [PORT_CONFIG] ([0..5])
  */
typedef struct {
  union {
    __IOM unsigned int PORT_LOAD_REG; /*!< (@ 0x00000000) Port Load */

    struct {
      __IOM unsigned int PORT_LOAD : 16; /*!< [15..0] Loads the value on to pin on write. And
                              reads the value of load register on read */
      __IM unsigned int RES : 16;        /*!< [31..16] RES */
    } PORT_LOAD_REG_b;
  };

  union {
    __OM unsigned int PORT_SET_REG; /*!< (@ 0x00000004) Port Set Register */

    struct {
      __OM unsigned int PORT_SET : 16;  /*!< [15..0] Sets the pin when corresponding bit is
                             high. Writing zero has no effect. */
      __OM unsigned int RESERVED1 : 16; /*!< [31..16] Reserved1 */
    } PORT_SET_REG_b;
  };

  union {
    __OM unsigned int PORT_CLEAR_REG; /*!< (@ 0x00000008) Port Clear Register */

    struct {
      __OM unsigned int PORT_CLEAR : 16; /*!< [15..0] Clears the pin when corresponding bit is
                               high. Writing zero has no effect. */
      __OM unsigned int RESERVED1 : 16;  /*!< [31..16] Reserved1 */
    } PORT_CLEAR_REG_b;
  };

  union {
    __OM unsigned int PORT_MASKED_LOAD_REG; /*!< (@ 0x0000000C) Port Masked Load Register */

    struct {
      __OM unsigned int PORT_MASKED_LOAD : 16; /*!< [15..0] Only loads into pins which are not
                                     masked. On read, pass only status unmasked
                                     pins */
      __OM unsigned int RESERVED1 : 16;        /*!< [31..16] Reserved1 */
    } PORT_MASKED_LOAD_REG_b;
  };

  union {
    __OM unsigned int PORT_TOGGLE_REG; /*!< (@ 0x00000010) Port Toggle Register */

    struct {
      __OM unsigned int PORT_TOGGLE : 16; /*!< [15..0] Toggles the pin when corresponding bit
                                is high. Writing zero has not effect. */
      __OM unsigned int RESERVED1 : 16;   /*!< [31..16] Reserved1 */
    } PORT_TOGGLE_REG_b;
  };

  union {
    __IM unsigned int PORT_READ_REG; /*!< (@ 0x00000014) Port Read Register */

    struct {
      __IM unsigned int PORT_READ : 16; /*!< [15..0] Reads the value on GPIO pins
                                        irrespective of the pin mode. */
      __IM unsigned int RESERVED1 : 16; /*!< [31..16] Reserved1 */
    } PORT_READ_REG_b;
  };
  __IM unsigned int RESERVED[2];
} EGPIO_PORT_CONFIG_Type; /*!< Size = 32 (0x20) */

/**
  * @brief EGPIO_INTR [INTR] ([0..5])
  */
typedef struct {
  union {
    __IOM unsigned int GPIO_INTR_CTRL; /*!< (@ 0x00000000) GPIO Interrupt Control Register */

    struct {
      __IOM unsigned int LEVEL_HIGH_ENABLE : 1; /*!< [0..0] enables interrupt generation when
                                     pin level is 1                   */
      __IOM unsigned int LEVEL_LOW_ENABLE : 1;  /*!< [1..1] enables interrupt generation when
                                    pin level is 0                   */
      __IOM unsigned int RISE_EDGE_ENABLE : 1;  /*!< [2..2] enables interrupt generation when
                                    rising edge is detected on pin */
      __IOM unsigned int FALL_EDGE_ENABLE : 1;  /*!< [3..3] enables interrupt generation when
                                    Falling edge is detected on pin */
      __IOM unsigned int MASK : 1;              /*!< [4..4] Masks the interrupt. Interrupt will still be
                        seen in status register when enabled */
      __IOM unsigned int RESERVED1 : 3;         /*!< [7..5] Reserved1   */
      __IOM unsigned int PIN_NUMBER : 4;        /*!< [11..8] GPIO Pin to be chosen for
                                          interrupt generation  */
      __IOM unsigned int PORT_NUMBER : 2;       /*!< [13..12] GPIO Port to be chosen for
                                          interrupt generation */
      __IOM unsigned int RESERVED2 : 18;        /*!< [31..14] Reserved2  */
    } GPIO_INTR_CTRL_b;
  };

  union {
    __IOM unsigned int GPIO_INTR_STATUS; /*!< (@ 0x00000004) GPIO Interrupt Status
                                         Register                             */

    struct {
      __IOM unsigned int INTERRUPT_STATUS : 1; /*!< [0..0] Gets set when interrupt
                                               is enabled and occurs. */
      __IOM unsigned int RISE_EDGE_STATUS : 1; /*!< [1..1] Gets set when rise edge
                                               is enabled and occurs. */
      __IOM unsigned int FALL_EDGE_STATUS : 1; /*!< [2..2] Gets set when Fall edge
                                               is enabled and occurs. */
      __OM unsigned int MASK_SET : 1;          /*!< [3..3] Mask set          */
      __OM unsigned int MASK_CLEAR : 1;        /*!< [4..4] Mask Clear        */
      __IOM unsigned int RESERVED1 : 27;       /*!< [31..5] Reserved1       */
    } GPIO_INTR_STATUS_b;
  };
} EGPIO_INTR_Type; /*!< Size = 8 (0x8) */

/**
  * @brief EGPIO_GPIO_GRP_INTR [GPIO_GRP_INTR] ([0..3])
  */
typedef struct {
  union {
    __IOM unsigned int GPIO_GRP_INTR_CTRL_REG; /*!< (@ 0x00000000) GPIO Interrupt 0
                                               Control Register */

    struct {
      __IOM unsigned int AND_OR : 1;           /*!< [0..0] AND/OR     */
      __IOM unsigned int LEVEL_EDGE : 1;       /*!< [1..1] Level/Edge */
      __IOM unsigned int ENABLE_WAKEUP : 1;    /*!< [2..2] For wakeup generation, actual pin
                                 status has to be seen(before double ranking
                                 point) */
      __IOM unsigned int ENABLE_INTERRUPT : 1; /*!< [3..3] Enable Interrupt */
      __IOM unsigned int MASK : 1;             /*!< [4..4] Mask             */
      __IOM unsigned int RESERVED1 : 27;       /*!< [31..5] Reserved1       */
    } GPIO_GRP_INTR_CTRL_REG_b;
  };

  union {
    __IOM unsigned int GPIO_GRP_INTR_STS; /*!< (@ 0x00000004) GPIO Interrupt 0
                                          Status Register */

    struct {
      __IOM unsigned int INTERRUPT_STATUS : 1; /*!< [0..0] Interrupt status is available in
                                        this bit when interrupt     is enabled and
                                        generated.  When 1 is written, interrupt
                                               gets cleared.     */
      __IM unsigned int WAKEUP : 1;            /*!< [1..1] Double ranked version of wakeup.
                                        Gets set when wakeup     is enabled and occurs.
                                        When 1 is written it gets cleared     */
      __IOM unsigned int RESERVED1 : 1;        /*!< [2..2] Reserved1 */
      __IOM unsigned int MASK_SET : 1;         /*!< [3..3] Gives zero on read  */
      __IOM unsigned int MASK_CLEAR : 1;       /*!< [4..4] Gives zero on read */
      __IOM unsigned int RESERVED2 : 27;       /*!< [31..5] Reserved2 */
    } GPIO_GRP_INTR_STS_b;
  };
} EGPIO_GPIO_GRP_INTR_Type; /*!< Size = 8 (0x8) */

/**
  * @brief MCU_RET_NPSS_GPIO_CNTRL [NPSS_GPIO_CNTRL] ([0..4])
  */
typedef struct {
  union {
    __IOM unsigned int NPSS_GPIO_CTRLS; /*!< (@ 0x00000000) NPSS GPIO Control register */

    struct {
      __IOM unsigned int NPSS_GPIO_MODE : 3;     /*!< [2..0] NPSS GPIO 0 mode select.           */
      __IOM unsigned int NPSS_GPIO_REN : 1;      /*!< [3..3] NPSS GPIO 0 Input Buffer
                                            Enable.  1- Enable 0- Disable. */
      __IOM unsigned int NPSS_GPIO_OEN : 1;      /*!< [4..4] NPSS GPIO 0 Output Buffer Enable.  1-
                                 Input Direction 0- Output Direction. */
      __IOM unsigned int NPSS_GPIO_OUT : 1;      /*!< [5..5] NPSS GPIO 0 Output value.        */
      __IOM unsigned int RESERVED1 : 2;          /*!< [7..6] Reserved1 */
      __IOM unsigned int NPSS_GPIO_POLARITY : 1; /*!< [8..8] NPSS GPIO 0 Polarity 1
                                                 - When signal is High 0 - When
                                                      signal is Ligh. */
      __IOM unsigned int RESERVED2 : 7;          /*!< [15..9] Reserved2          */
      __IOM unsigned int USE_ULPSS_PAD : 1;      /*!< [16..16] Input from ULPSS GPIOs.         */
      __IOM unsigned int RESERVED3 : 15;         /*!< [31..17] Reserved3 */
    } NPSS_GPIO_CTRLS_b;
  };
} MCU_RET_NPSS_GPIO_CNTRL_Type; /*!< Size = 4 (0x4) */

/**
  * @brief ULPCLK_ULP_SOC_GPIO_MODE_REG [ULP_SOC_GPIO_MODE_REG] ([0..15])
  */
typedef struct {
  union {
    __IOM unsigned int ULP_SOC_GPIO_MODE_REG; /*!< (@ 0x00000000) ulp soc gpio mode
                                              register */

    struct {
      __IOM unsigned int ULP_SOC_GPIO_MODE_REG : 3; /*!< [2..0] mode bits for soc gpio. */
      __IOM unsigned int RESERVED1 : 29;            /*!< [31..3] reserved1 */
    } ULP_SOC_GPIO_MODE_REG_b;
  };
} ULPCLK_ULP_SOC_GPIO_MODE_REG_Type; /*!< Size = 4 (0x4) */

/**
  * @brief AUX_ADC_DAC_COMP_ADC_CH_BIT_MAP_CONFIG [ADC_CH_BIT_MAP_CONFIG]
  * ([0..15])
  */
typedef struct {
  union {
    __IOM unsigned int ADC_CH_BIT_MAP_CONFIG_0; /*!< (@ 0x00000000) This is configuration
                                     register0 to explain the bit map for ADC
                                     channels */

    struct {
      __IOM unsigned int CHANNEL_BITMAP : 32; /*!< [31..0] ADC Channels bit map */
    } ADC_CH_BIT_MAP_CONFIG_0_b;
  };

  union {
    __IOM unsigned int ADC_CH_BIT_MAP_CONFIG_1; /*!< (@ 0x00000004) This is configuration
                                     register1 to explain the bit map for ADC
                                     channels */

    struct {
      __IOM unsigned int CHANNEL_BITMAP : 32; /*!< [31..0] ADC Channels bit map */
    } ADC_CH_BIT_MAP_CONFIG_1_b;
  };

  union {
    __IOM unsigned int ADC_CH_BIT_MAP_CONFIG_2; /*!< (@ 0x00000008) This is configuration
                                     register2 to explain the bit map for ADC
                                     channels */

    struct {
      __IOM unsigned int CHANNEL_BITMAP : 32; /*!< [31..0] ADC Channels bit map */
    } ADC_CH_BIT_MAP_CONFIG_2_b;
  };

  union {
    __IOM unsigned int ADC_CH_BIT_MAP_CONFIG_3; /*!< (@ 0x0000000C) This is configuration
                                     register3 to explain the bit map for ADC
                                     channels */

    struct {
      __IOM unsigned int CHANNEL_BITMAP : 5; /*!< [4..0] ADC Channels bit map */
      __IOM unsigned int RESERVED1 : 27;     /*!< [31..5] Reserved1     */
    } ADC_CH_BIT_MAP_CONFIG_3_b;
  };
} AUX_ADC_DAC_COMP_ADC_CH_BIT_MAP_CONFIG_Type; /*!< Size = 16 (0x10) */

/** @} */ /* End of group Device_Peripheral_clusters */

/* ===========================================================================================================================
  */
/* ================                            Device Specific Peripheral
  * Section                             ================ */
/* ===========================================================================================================================
  */

/** @addtogroup Device_Peripheral_peripherals
  * @{
  */

/* ===========================================================================================================================
  */
/* ================                                           I2C0
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief Inter Integrated Circuit(I2C) is programmable control bus that
   provides support for the communications link between integrated circuits in a
   system (I2C0)
   */

typedef struct { /*!< (@ 0x44010000) I2C0 Structure */

  union {
    __IOM unsigned int IC_CON; /*!< (@ 0x00000000) This register can be written only
                               when the i2c is disabled, which corresponds to
                               IC_ENABLE[0] being set to 0. Writes at other times
                               have no effect. */

    struct {
      __IOM unsigned int MASTER_MODE : 1;                /*!< [0..0] This bit controls whether the
                                          I2C master is enabled.               */
      __IOM unsigned int SPEED : 2;                      /*!< [2..1] These bits control at which speed
                                    the I2C operates. Hardware protects against
                                    illegal values being programmed by software.
                                  */
      __IOM unsigned int IC_10BITADDR_SLAVE : 1;         /*!< [3..3] When acting as a slave,
                                                 this bit controls whether the
                                                      I2C responds  to 7- or
                                                 10-bit addresses. */
      __IM unsigned int IC_10BITADDR_MASTER_RD_ONLY : 1; /*!< [4..4] the function of this bit
                                               is handled by bit 12 of IC_TAR
                                                        register, and becomes  a
                                               read-only copy called
                                               IC_10BITADDR_MASTER_rd_onl
                                                                                                                                  */
      __IOM unsigned int IC_RESTART_EN : 1;              /*!< [5..5] Determines whether RESTART conditions
                                 may be sent when acting as a master */
      __IOM unsigned int IC_SLAVE_DISABLE : 1;           /*!< [6..6] This bit controls whether
                                               I2C has its slave disabled */
      __IOM unsigned int STOP_DET_IFADDRESSED : 1;       /*!< [7..7] The STOP DETECTION interrupt is
                                        generated only when the transmitted
                                        address matches the slave address of SAR
                                      */
      __IOM unsigned int TX_EMPTY_CTRL : 1;              /*!< [8..8] This bit controls the
                                            generation of the TX EMPTY interrupt,
                                               as described in the IC RAW INTR
                                            STAT register. */
      __IM unsigned int RESERVED1 : 1;                   /*!< [9..9] reserved1      */
      __IOM unsigned int STOP_DET_IF_MASTER_ACTIVE : 1;  /*!< [10..10] In Master mode. */
      __IOM unsigned int BUS_CLEAR_FEATURE_CTRL : 1;     /*!< [11..11] In Master mode. */
      __IOM unsigned int RESERVED2 : 20;                 /*!< [31..12] reserved2  */
    } IC_CON_b;
  };

  union {
    __IOM unsigned int IC_TAR; /*!< (@ 0x00000004) I2C Target Address Register */

    struct {
      __IOM unsigned int IC_TAR : 10;             /*!< [9..0] This is the target address for any
                                      master transaction              */
      __IOM unsigned int GC_OR_START : 1;         /*!< [10..10] If bit 11 (SPECIAL) is set
                                          to 1, then this bit indicates whether a
                                          General Call or START byte command  is
                                          to be performed by the DW_apb_i2c */
      __IOM unsigned int SPECIAL : 1;             /*!< [11..11] This bit indicates whether software
                           performs a General Call or START BYTE command */
      __IOM unsigned int IC_10BITADDR_MASTER : 1; /*!< [12..12] This bit controls
                                                  whether the i2c starts its
                                                  transfers in 7-or 10-bit
                                                  addressing mode when acting as
                                                  a master                    */
      __IOM unsigned int DEVICE_ID : 1;           /*!< [13..13] If bit 11 (SPECIAL) is set to 1, then
                             this bit indicates whether a Device-ID of a
                             particular slave mentioned in IC_TAR[6:0] is to be
                             performed by the I2C Master */
      __IM unsigned int RESERVED1 : 18;           /*!< [31..14] reserved1 */
    } IC_TAR_b;
  };

  union {
    __IOM unsigned int IC_SAR; /*!< (@ 0x00000008) I2C Slave Address Register */

    struct {
      __IOM unsigned int IC_SAR : 10;   /*!< [9..0] The IC_SAR holds the slave address when the
                           I2C is operating as a slave. For 7-bit addressing,
                           only IC_SAR[6:0] is used.               */
      __IM unsigned int RESERVED1 : 22; /*!< [31..10] reserved1 */
    } IC_SAR_b;
  };

  union {
    __IOM unsigned int IC_HS_MADDR; /*!< (@ 0x0000000C) I2C High Speed Master Mode
                                    Code Address Register           */

    struct {
      __IOM unsigned int IC_HS_MAR : 3; /*!< [2..0] This bit field holds the value
                                        of the I2C HS mode master code */
      __IM unsigned int RESERVED1 : 29; /*!< [31..3] reserved1 */
    } IC_HS_MADDR_b;
  };

  union {
    __IOM unsigned int IC_DATA_CMD; /*!< (@ 0x00000010) I2C Rx/Tx Data Buffer and
                                    Command Register                 */

    struct {
      __IOM unsigned int DAT : 8;            /*!< [7..0] This register contains the data to be
                                  transmitted or received on the I2C bus */
      __OM unsigned int CMD : 1;             /*!< [8..8] This bit controls whether a read or a
                                  write is performed           */
      __OM unsigned int STOP : 1;            /*!< [9..9] This bit controls whether a STOP is
                                  issued after the byte is sent or received */
      __OM unsigned int RESTART : 1;         /*!< [10..10] This bit controls whether a RESTART is
                           issued before the byte is sent or received */
      __IM unsigned int FIRST_DATA_BYTE : 1; /*!< [11..11] Indicates the first data byte
                                        received after the address      phase for receive
                                        transfer in Master receiver or Slave
                                                 receiver mode      */
      __IM unsigned int RESERVED1 : 20;      /*!< [31..12] reserved1 */
    } IC_DATA_CMD_b;
  };

  union {
    __IOM unsigned int IC_SS_SCL_HCNT; /*!< (@ 0x00000014) Standard Speed I2C Clock
                                       SCL High Count Register           */

    struct {
      __IOM unsigned int IC_SS_SCL_HCNT : 16; /*!< [15..0] This register must be set before any
                                   I2C bus transaction can take place to ensure
                                   proper I/O timing */
      __IM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1 */
    } IC_SS_SCL_HCNT_b;
  };

  union {
    __IOM unsigned int IC_SS_SCL_LCNT; /*!< (@ 0x00000018) Standard Speed I2C Clock
                                       SCL Low Count Register            */

    struct {
      __IOM unsigned int IC_SS_SCL_LCNT : 16; /*!< [15..0] This register must be set before any
                                   I2C bus transaction can take place to ensure
                                   proper I/O timing */
      __IM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1 */
    } IC_SS_SCL_LCNT_b;
  };

  union {
    __IOM unsigned int IC_FS_SCL_HCNT; /*!< (@ 0x0000001C) Fast Speed I2C Clock SCL
                                       High Count Register               */

    struct {
      __IOM unsigned int IC_FS_SCL_HCNT : 16; /*!< [15..0] This register must be set before any
                                   I2C bus transaction can take place to ensure
                                   proper I/O timing */
      __IM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1 */
    } IC_FS_SCL_HCNT_b;
  };

  union {
    __IOM unsigned int IC_FS_SCL_LCNT; /*!< (@ 0x00000020) Fast Speed I2C Clock SCL
                                       Low Count Register                */

    struct {
      __IOM unsigned int IC_FS_SCL_LCNT : 16; /*!< [15..0] This register must be set before any
                                   I2C bus transaction can take place to ensure
                                   proper I/O timing */
      __IM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1 */
    } IC_FS_SCL_LCNT_b;
  };

  union {
    __IOM unsigned int IC_HS_SCL_HCNT; /*!< (@ 0x00000024) High Speed I2C Clock SCL
                                       High Count Register               */

    struct {
      __IOM unsigned int IC_HS_SCL_HCNT : 16; /*!< [15..0] This register must be set before any
                                   I2C bus transaction can take place to ensure
                                   proper I/O timing */
      __IM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1 */
    } IC_HS_SCL_HCNT_b;
  };

  union {
    __IOM unsigned int IC_HS_SCL_LCNT; /*!< (@ 0x00000028) High Speed I2C Clock SCL
                                       Low Count Register                */

    struct {
      __IOM unsigned int IC_HS_SCL_LCNT : 16; /*!< [15..0] This register must be set before any
                                   I2C bus transaction can take place to ensure
                                   proper I/O timing */
      __IM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1 */
    } IC_HS_SCL_LCNT_b;
  };

  union {
    __IM unsigned int IC_INTR_STAT; /*!< (@ 0x0000002C) I2C Interrupt Status Register */

    struct {
      __IM unsigned int R_RX_UNDER : 1;         /*!< [0..0] Set if the processor attempts to
                                        read the receive buffer
                                                   when it is empty by reading
                                        from the IC_DATA_CMD register */
      __IM unsigned int R_RX_OVER : 1;          /*!< [1..1] Set if the receive buffer is completely
                             filled to IC_RX_BUFFER_DEPTH and an additional byte
                             is received from an external I2C device */
      __IM unsigned int R_RX_FULL : 1;          /*!< [2..2] Set when the receive buffer
                                       reaches or goes above the  RX_TL threshold
                                       in the IC_RX_TL register.  */
      __IM unsigned int R_TX_OVER : 1;          /*!< [3..3] Set during transmit if the
                                       transmit buffer is filled to
                                       IC_TX_BUFFER_DEPTH and the processor
                                       attempts to issue another I2C command by
                                       writing to the IC_DATA_CMD register. */
      __IM unsigned int R_TX_EMPTY : 1;         /*!< [4..4] This bit is set to 1 when the transmit
                              buffer is at or below the threshold value set in
                              the IC_TX_TL register.                   */
      __IM unsigned int R_RD_REQ : 1;           /*!< [5..5] This bit is set to 1 when DW_apb_i2c is
                            acting as a slave and another I2C master is
                            attempting to read data from DW_apb_i2c. */
      __IM unsigned int R_TX_ABRT : 1;          /*!< [6..6] This bit indicates if DW_apb_i2c, as an I2C
                             transmitter, is unable to complete the intended
                             actions on the contents of the transmit FIFO */
      __IM unsigned int R_RX_DONE : 1;          /*!< [7..7] When the DW_apb_i2c is acting as a
                             slave-transmitter, this bit is set to 1 if the
                             master does not acknowledge a transmitted byte */
      __IM unsigned int R_ACTIVITY : 1;         /*!< [8..8] This bit captures DW_apb_i2c activity and
                              stays set until it is cleared */
      __IM unsigned int R_STOP_DET : 1;         /*!< [9..9] Indicates whether a STOP
                                         condition has occurred on the   I2C
                                         interface regardless of whether DW_apb_i2c
                                         is operating   in slave or master mode.   */
      __IM unsigned int R_START_DET : 1;        /*!< [10..10] Indicates whether a START or
                                         RESTART condition has occurred on the
                                         I2C interface regardless of  whether
                                         DW_apb_i2c is operating in slave or
                                         master mode. */
      __IM unsigned int R_GEN_CALL : 1;         /*!< [11..11] Set only when a General Call address is
                              received and it is acknowledged */
      __IM unsigned int R_RESTART_DET : 1;      /*!< [12..12] Indicates whether a RESTART condition
                                 has occurred on the I2C interface when
                                 DW_apb_i2c is operating in slave mode and the
                                 slave is the addressed slave */
      __IM unsigned int R_MST_ON_HOLD : 1;      /*!< [13..13] Indicates whether a master is holding
                                 the bus and the Tx FIFO is empty. */
      __IM unsigned int M_SCL_STUCK_AT_LOW : 1; /*!< [14..14] Indicates whether the
                                                SCL Line is stuck at low for
                                                      the IC_SCL_STUCK_LOW_TIMOUT
                                                number of ic_clk periods */
      __IM unsigned int RESERVED1 : 17;         /*!< [31..15] reserved1         */
    } IC_INTR_STAT_b;
  };

  union {
    __IOM unsigned int IC_INTR_MASK; /*!< (@ 0x00000030) I2C Interrupt Mask Register */

    struct {
      __IOM unsigned int M_RX_UNDER : 1;         /*!< [0..0] This bit mask their
                                         corresponding interrupt status bits in
                                         the IC_INTR_STAT register. */
      __IOM unsigned int M_RX_OVER : 1;          /*!< [1..1] This bit mask their corresponding interrupt
                             status bits in the IC_INTR_STAT register. */
      __IOM unsigned int M_RX_FULL : 1;          /*!< [2..2] This bit mask their corresponding interrupt
                             status bits in the IC_INTR_STAT register. */
      __IOM unsigned int M_TX_OVER : 1;          /*!< [3..3] This bit mask their corresponding interrupt
                             status bits in the IC_INTR_STAT register */
      __IOM unsigned int M_TX_EMPTY : 1;         /*!< [4..4] This bit mask their
                                         corresponding interrupt status bits in
                                         the IC_INTR_STAT register. */
      __IOM unsigned int M_RD_REQ : 1;           /*!< [5..5] This bit mask their corresponding interrupt
                            status bits in the IC_INTR_STAT register. */
      __IOM unsigned int M_TX_ABRT : 1;          /*!< [6..6] This bit mask their corresponding interrupt
                             status bits in the IC_INTR_STAT register. */
      __IOM unsigned int M_RX_DONE : 1;          /*!< [7..7] This bit mask their corresponding interrupt
                             status bits in the IC_INTR_STAT register. */
      __IOM unsigned int M_ACTIVITY : 1;         /*!< [8..8] This bit mask their
                                         corresponding interrupt status bits in
                                         the IC_INTR_STAT register. */
      __IOM unsigned int M_STOP_DET : 1;         /*!< [9..9] This bit mask their
                                         corresponding interrupt status bits in
                                         the IC_INTR_STAT register. */
      __IOM unsigned int M_START_DET : 1;        /*!< [10..10] This bit mask their corresponding
                               interrupt status bits in the IC_INTR_STAT
                               register. */
      __IOM unsigned int M_GEN_CALL : 1;         /*!< [11..11] This bit mask their
                                         corresponding interrupt status bits in
                                         the IC_INTR_STAT register. */
      __IOM unsigned int M_RESTART_DET : 1;      /*!< [12..12] Indicates whether a RESTART condition
                                 has occurred on the I2C interface when
                                 DW_apb_i2c is operating in slave mode and the
                                 slave is the addressed slave */
      __IOM unsigned int M_MST_ON_HOLD : 1;      /*!< [13..13] Indicates whether a master is holding
                                 the bus and the Tx FIFO is empty. */
      __IOM unsigned int M_SCL_STUCK_AT_LOW : 1; /*!< [14..14] Indicates whether the
                                                 SCL Line is stuck at low for
                                                      the IC_SCL_STUCK_LOW_TIMOUT
                                                 number of ic_clk periods */
      __IM unsigned int RESERVED1 : 17;          /*!< [31..15] reserved1          */
    } IC_INTR_MASK_b;
  };

  union {
    __IM unsigned int IC_RAW_INTR_STAT; /*!< (@ 0x00000034) I2C Raw Interrupt Status
                                        Register                          */

    struct {
      __IM unsigned int RX_UNDER : 1;         /*!< [0..0] Set if the processor attempts to read the
                            receive buffer when it is empty by reading from the
                            IC_DATA_CMD register                 */
      __IM unsigned int RX_OVER : 1;          /*!< [1..1] Set if the receive buffer is completely
                           filled to IC_RX_BUFFER_DEPTH and an additional byte is
                           received from an external I2C device */
      __IM unsigned int RX_FULL : 1;          /*!< [2..2] Set when the receive buffer reaches
                                     or goes above the RX_TL threshold in the
                                     IC_RX_TL register. */
      __IM unsigned int TX_OVER : 1;          /*!< [3..3] Set during transmit if the transmit buffer is
                           filled to IC_TX_BUFFER_DEPTH and the processor
                           attempts to issue another I2C command by writing to
                           the IC_DATA_CMD register.               */
      __IM unsigned int TX_EMPTY : 1;         /*!< [4..4] This bit is set to 1 when the
                                      transmit buffer is at or below the
                                      threshold value set in the IC_TX_TL
                                      register.                   */
      __IM unsigned int RD_REQ : 1;           /*!< [5..5] This bit is set to 1 when DW_apb_i2c is acting
                          as a slave and another I2C master is attempting to read
                          data from DW_apb_i2c. */
      __IM unsigned int TX_ABRT : 1;          /*!< [6..6] This bit indicates if DW_apb_i2c, as an I2C
                           transmitter, is unable to complete the intended
                           actions on the contents of the transmit FIFO */
      __IM unsigned int RX_DONE : 1;          /*!< [7..7] When the DW_apb_i2c is acting as a
                           slave-transmitter, this bit is set to 1 if the master
                           does not acknowledge a transmitted byte */
      __IM unsigned int ACTIVITY : 1;         /*!< [8..8] This bit captures DW_apb_i2c activity and
                            stays set until it is cleared */
      __IM unsigned int STOP_DET : 1;         /*!< [9..9] Indicates whether a STOP condition has
                            occurred on the I2C interface regardless of whether
                            DW_apb_i2c is operating in slave or master mode. */
      __IM unsigned int START_DET : 1;        /*!< [10..10] Indicates whether a START or RESTART
                             condition has occurred on the I2C interface
                             regardless of  whether DW_apb_i2c is operating in
                             slave or master mode. */
      __IM unsigned int GEN_CALL : 1;         /*!< [11..11] Set only when a General Call address is
                            received and it is acknowledged */
      __IM unsigned int RESTART_DET : 1;      /*!< [12..12] Indicates whether a RESTART condition
                               has occurred on the I2C interface when DW_apb_i2c
                               is operating in slave mode and the slave is the
                               addressed slave                                 */
      __IM unsigned int MST_ON_HOLD : 1;      /*!< [13..13] Indicates whether a master is holding
                               the bus and the Tx FIFO is empty. */
      __IM unsigned int SCL_STUCK_AT_LOW : 1; /*!< [14..14] Indicates whether the
                                              SCL Line is stuck at low for the
                                              IC_SCL_STUCK_LOW_TIMOUT number of
                                              ic_clk periods */
      __IM unsigned int RESERVED1 : 17;       /*!< [31..15] reserved1       */
    } IC_RAW_INTR_STAT_b;
  };

  union {
    __IOM unsigned int IC_RX_TL; /*!< (@ 0x00000038) I2C Receive FIFO Threshold Register */

    struct {
      __IOM unsigned int RX_TL : 8;     /*!< [7..0] Receive FIFO Threshold Level     */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
    } IC_RX_TL_b;
  };

  union {
    __IOM unsigned int IC_TX_TL; /*!< (@ 0x0000003C) I2C Transmit FIFO Threshold Register */

    struct {
      __IOM unsigned int TX_TL : 8;     /*!< [7..0] Transmit FIFO Threshold Level     */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
    } IC_TX_TL_b;
  };

  union {
    __IM unsigned int IC_CLR_INTR; /*!< (@ 0x00000040) Clear Combined and Individual
                                   Interrupt Register           */

    struct {
      __IM unsigned int CLR_INTR : 1;   /*!< [0..0] Read this register to clear the combined
                            interrupt, all individual interrupts, and the
                            IC_TXABRT_SOURCE register                  */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } IC_CLR_INTR_b;
  };

  union {
    __IM unsigned int IC_CLR_RX_UNDER; /*!< (@ 0x00000044) Clear RX_UNDER Interrupt
                                       Register                          */

    struct {
      __IM unsigned int CLR_RX_UNDER : 1; /*!< [0..0] Read this register to clear
                                          the RX_UNDER interrupt (bit 0) of the
                                          IC_RAW_INTR_STAT register. */
      __IM unsigned int RESERVED1 : 31;   /*!< [31..1] reserved1   */
    } IC_CLR_RX_UNDER_b;
  };

  union {
    __IM unsigned int IC_CLR_RX_OVER; /*!< (@ 0x00000048) Clear RX_OVER Interrupt
                                      Register                           */

    struct {
      __IM unsigned int CLR_RX_OVER : 1; /*!< [0..0] Read this register to clear the
                                         RX_OVER interrupt (bit 1) of the
                                         IC_RAW_INTR_STAT register */
      __IM unsigned int RESERVED1 : 31;  /*!< [31..1] reserved1  */
    } IC_CLR_RX_OVER_b;
  };

  union {
    __IM unsigned int IC_CLR_TX_OVER; /*!< (@ 0x0000004C) Clear TX_OVER Interrupt
                                      Register                           */

    struct {
      __IM unsigned int CLR_TX_OVER : 1; /*!< [0..0] Read this register to clear the
                                         TX_OVER interrupt (bit 3) of the
                                         IC_RAW_INTR_STAT register. */
      __IM unsigned int RESERVED1 : 31;  /*!< [31..1] reserved1  */
    } IC_CLR_TX_OVER_b;
  };

  union {
    __IM unsigned int IC_CLR_RD_REQ; /*!< (@ 0x00000050) Clear RD_REQ Interrupt Register */

    struct {
      __IM unsigned int CLR_RD_REQ : 1; /*!< [0..0] Read this register to clear the
                                        RD_REQ interrupt (bit 5) of the
                                        IC_RAW_INTR_STAT register. */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } IC_CLR_RD_REQ_b;
  };

  union {
    __IM unsigned int IC_CLR_TX_ABRT; /*!< (@ 0x00000054) Clear TX_ABRT Interrupt
                                      Register                           */

    struct {
      __IM unsigned int CLR_TX_ABRT : 1; /*!< [0..0] Read this register to clear the TX_ABRT
                               interrupt (bit 6) of the C_RAW_INTR_STAT register,
                               and the IC_TX_ABRT_SOURCE register */
      __IM unsigned int RESERVED1 : 31;  /*!< [31..1] reserved1 */
    } IC_CLR_TX_ABRT_b;
  };

  union {
    __IM unsigned int IC_CLR_RX_DONE; /*!< (@ 0x00000058) Clear RX_DONE Interrupt
                                      Register                           */

    struct {
      __IM unsigned int CLR_RX_DONE : 1; /*!< [0..0] Read this register to clear the
                                         RX_DONE interrupt (bit 7) of the
                                         IC_RAW_INTR_STAT register */
      __IM unsigned int RESERVED1 : 31;  /*!< [31..1] reserved1  */
    } IC_CLR_RX_DONE_b;
  };

  union {
    __IM unsigned int IC_CLR_ACTIVITY; /*!< (@ 0x0000005C) Clear ACTIVITY Interrupt
                                       Register                          */

    struct {
      __IM unsigned int CLR_ACTIVITY : 1; /*!< [0..0] Reading this register clears
                                          the ACTIVITY interrupt if the I2C is
                                          not active any more */
      __IM unsigned int RESERVED1 : 31;   /*!< [31..1] reserved1   */
    } IC_CLR_ACTIVITY_b;
  };

  union {
    __IM unsigned int IC_CLR_STOP_DET; /*!< (@ 0x00000060) Clear STOP_DET Interrupt
                                       Register                          */

    struct {
      __IM unsigned int CLR_STOP_DET : 1; /*!< [0..0] Read this register to clear
                                          the STOP_DET interrupt (bit 9) of the
                                          IC_RAW_INTR_STAT register. */
      __IM unsigned int RESERVED1 : 31;   /*!< [31..1] reserved1   */
    } IC_CLR_STOP_DET_b;
  };

  union {
    __IM unsigned int IC_CLR_START_DET; /*!< (@ 0x00000064) Clear START_DET
                                        Interrupt Register */

    struct {
      __IM unsigned int CLR_START_DET : 1; /*!< [0..0] Read this register to clear
                                           the START_DET interrupt (bit 10) of
                                           the IC_RAW_INTR_STAT register */
      __IM unsigned int RESERVED1 : 31;    /*!< [31..1] reserved1    */
    } IC_CLR_START_DET_b;
  };

  union {
    __IM unsigned int IC_CLR_GEN_CALL; /*!< (@ 0x00000068) Clear GEN_CALL Interrupt
                                       Register                          */

    struct {
      __IM unsigned int CLR_GEN_CALL : 1; /*!< [0..0] Read this register to clear
                                          the GEN_CALL interrupt (bit 11) of
                                          IC_RAW_INTR_STAT register */
      __IM unsigned int RESERVED1 : 31;   /*!< [31..1] reserved1   */
    } IC_CLR_GEN_CALL_b;
  };

  union {
    __IOM unsigned int IC_ENABLE; /*!< (@ 0x0000006C) Clear GEN_CALL Interrupt Register */

    struct {
      __IOM unsigned int EN : 1;                        /*!< [0..0] Controls whether the DW_apb_i2c is enabled */
      __IOM unsigned int ABORT : 1;                     /*!< [1..1] When set, the controller initiates
                                    the transfer abort              */
      __IOM unsigned int TX_CMD_BLOCK : 1;              /*!< [2..2] none              */
      __IOM unsigned int SDA_STUCK_RECOVERY_ENABLE : 1; /*!< [3..3] SDA STUCK
                                                        RECOVERY ENABLE */
      __IM unsigned int RESERVED1 : 28;                 /*!< [31..4] reserved1                 */
    } IC_ENABLE_b;
  };

  union {
    __IM unsigned int IC_STATUS; /*!< (@ 0x00000070) I2C Status Register */

    struct {
      __IM unsigned int ACTIVITY : 1;                /*!< [0..0] I2C Activity Status */
      __IM unsigned int TFNF : 1;                    /*!< [1..1] Transmit FIFO Not Full     */
      __IM unsigned int TFE : 1;                     /*!< [2..2] Transmit FIFO Completely Empty      */
      __IM unsigned int RFNE : 1;                    /*!< [3..3] Receive FIFO Not Empty     */
      __IM unsigned int RFF : 1;                     /*!< [4..4] Receive FIFO Completely Full      */
      __IM unsigned int MST_ACTIVITY : 1;            /*!< [5..5] Master FSM Activity Status           */
      __IM unsigned int SLV_ACTIVITY : 1;            /*!< [6..6] Slave FSM Activity Status */
      __IM unsigned int MST_HOLD_TX_FIFO_EMPTY : 1;  /*!< [7..7] The I2C master stalls the
                                          write transfer when Tx FIFO is empty,
                                          and  the the last byte does not have
                                          the Stop bit set. */
      __IM unsigned int MST_HOLD_RX_FIFO_FULL : 1;   /*!< [8..8] This bit indicates the BUS Hold
                                         in Master mode due to Rx FIFO is Full
                                         and additional byte has been received.
                                       */
      __IM unsigned int SLV_HOLD_TX_FIFO_EMPTY : 1;  /*!< [9..9] This bit indicates the BUS
                                          Hold in Slave mode for the
                                           Read request when the  Tx FIFO is
                                          empty. */
      __IM unsigned int SLV_HOLD_RX_FIFO_FULL : 1;   /*!< [10..10] This bit indicates the BUS
                                         Hold in Slave mode due to the Rx FIFO
                                         being Full and  an additional byte being
                                         received.            */
      __IM unsigned int SDA_STUCK_NOT_RECOVERED : 1; /*!< [11..11] This bit indicates that an
                                           SDA stuck at low is not recovered
                                           after the  recovery mechanism. */
      __IM unsigned int RESERVED1 : 20;              /*!< [31..12] reserved1    */
    } IC_STATUS_b;
  };

  union {
    __IM unsigned int IC_TXFLR; /*!< (@ 0x00000074) I2C Transmit FIFO Level Register */

    struct {
      __IM unsigned int TXFLR : 4;      /*!< [3..0] Contains the number of valid data
                                   entries in the transmit FIFO. */
      __IM unsigned int RESERVED1 : 28; /*!< [31..4] reserved1 */
    } IC_TXFLR_b;
  };

  union {
    __IM unsigned int IC_RXFLR; /*!< (@ 0x00000078) I2C Receive FIFO Level Register */

    struct {
      __IM unsigned int RXFLR : 4;      /*!< [3..0] Receive FIFO Level. Contains the number of
                         valid data entries in the receive FIFO */
      __IM unsigned int RESERVED1 : 28; /*!< [31..4] reserved1 */
    } IC_RXFLR_b;
  };

  union {
    __IOM unsigned int IC_SDA_HOLD; /*!< (@ 0x0000007C) I2C SDA Hold Time Length Register */

    struct {
      __IOM unsigned int IC_SDA_TX_HOLD : 16; /*!< [15..0] Sets the required SDA hold time in
                                   units of ic_clk period,when I2C  acts as a
                                   transmitter. */
      __IOM unsigned int IC_SDA_RX_HOLD : 8;  /*!< [23..16] Sets the required SDA hold time in
                                  units of ic_clk period,when  I2C acts as a
                                  receiver. */
      __IM unsigned int RESERVED1 : 8;        /*!< [31..24] reserved1 */
    } IC_SDA_HOLD_b;
  };

  union {
    __IM unsigned int IC_TX_ABRT_SOURCE; /*!< (@ 0x00000080) I2C Transmit Abort
                                         Source Register */

    struct {
      __IM unsigned int ABRT_7B_ADDR_NOACK : 1;        /*!< [0..0] 1: Master is in 7-bit addressing
                                      mode and the address sent was not
                                      acknowledged by any slave */
      __IM unsigned int ABRT_10ADDR1_NOACK : 1;        /*!< [1..1] 1: Master is in 10-bit
                                                address mode and the first 10-bit
                                                    address byte was not
                                                acknowledged by any slave */
      __IM unsigned int ABRT_10ADDR2_NOACK : 1;        /*!< [2..2] 1: Master is in 10-bit address
                                      mode and the second address byte of the
                                      10-bit address was not acknowledged by any
                                          slave */
      __IM unsigned int ABRT_TXDATA_NOACK : 1;         /*!< [3..3] 1: This is a master-mode only bit.
                                     Master has received an acknowledgement for
                                     the address, but when it sent data byte(s)
                                     following the address, it did not receive an
                                     acknowledge from the remote slave(s) */
      __IM unsigned int ABRT_GCALL_NOACK : 1;          /*!< [4..4] 1: DW_apb_i2c in master mode sent a
                                    General Call and no slave on the bus
                                    acknowledged the General Call */
      __IM unsigned int ABRT_GCALL_READ : 1;           /*!< [5..5] 1: DW_apb_i2c in master mode sent a
                                   General Call but the user programmed the byte
                                   following the  General Call
                                       to be a read from the bus (IC_DATA_CMD[9]
                                   is set to 1)                    */
      __IM unsigned int ABRT_HS_ACKDET : 1;            /*!< [6..6] 1: Master is in High Speed
                                            mode and the High Speed Master code
                                            was acknowledged */
      __IM unsigned int ABRT_SBYTE_ACKDET : 1;         /*!< [7..7] 1: Master has sent a START Byte and
                                     the START Byte was acknowledged (wrong
                                     behavior) */
      __IM unsigned int ABRT_HS_NORSTRT : 1;           /*!< [8..8] 1: The restart is disabled
                                   (IC_RESTART_EN bit (IC_CON[5]) = 0) and the
                                   user is trying to use the master to transfer
                                       data in High Speed mode */
      __IM unsigned int ABRT_SBYTE_NORSTRT : 1;        /*!< [9..9] 1: The restart is disabled
                                      (IC_RESTART_EN bit (IC_CON[5])
                                          = 0) and the user is trying to send a
                                      START Byte                          */
      __IM unsigned int ABRT_10B_RD_NORSTRT : 1;       /*!< [10..10] 1: The restart is disabled
                                       (IC_RESTART_EN bit (IC_CON[5]) = 0) and
                                       the master sends a read command in 10-bit
                                       addressing mode */
      __IM unsigned int ABRT_MASTER_DIS : 1;           /*!< [11..11] 1: User tries to initiate a Master
                                   operation with the Master mode disabled */
      __IM unsigned int ARB_LOST : 1;                  /*!< [12..12] 1: Master has lost arbitration, or if
                            IC_TX_ABRT_SOURCE[14] is also set, then the slave
                            transmitter has lost arbitration              */
      __IM unsigned int ABRT_SLVFLUSH_TXFIFO : 1;      /*!< [13..13] 1: Slave has received a
                                              read command and some data   exists in
                                              the TX FIFO so the slave issues a
                                              TX_ABRT interrupt   to flush old data
                                              in TX FIFO   */
      __IM unsigned int ABRT_SLV_ARBLOST : 1;          /*!< [14..14] 1: Slave lost the bus
                                              while transmitting data to a remote
                                              master. IC_TX_ABRT_SOURCE[12] is
                                              set at the same time */
      __IM unsigned int ABRT_SLVRD_INTX : 1;           /*!< [15..15] 1: When the processor side responds
                                   to a slave mode request for data to be
                                   transmitted to a remote master and
                                       user writes a 1 in CMD (bit 8) of
                                   IC_DATA_CMD register                    */
      __IM unsigned int ABRT_USER_ABRT : 1;            /*!< [16..16] This is a master-mode-only bit.
                                  Master has detected the transfer abort
                                  (IC_ENABLE[1]). */
      __IM unsigned int ABRT_SDA_STUCK_AT_LOW : 1;     /*!< [17..17] Master detects the
                                                   SDA is Stuck at low for the
                                                   IC_SDA_STUCK_AT_LOW_TI EOUT
                                                   value of ic_clks */
      __IM unsigned int ABRT_DEVICE_NOACK : 1;         /*!< [18..18] Master initiates the DEVICE_ID
                                    transfer and the device ID sent is not
                                    acknowledged by any slave */
      __IM unsigned int ABRT_DEVICE_SLVADDR_NOACK : 1; /*!< [19..19] Master is initiating the
                                            DEVICE_ID transfer and the slave
                                            address  sent was not acknowledged by
                                            any slave                     */
      __IM unsigned int ABRT_DEVICE_WRITE : 1;         /*!< [20..20] Master is initiating the
                                      DEVICE_ID transfer and the
                                      Tx- FIFO consists of write commands.   */
      __IM unsigned int RESERVED1 : 2;                 /*!< [22..21] reserved1 */
      __IM unsigned int TX_FLUSH_CNT : 9;              /*!< [31..23] This field indicates the number of Tx
                                FIFO data commands that are flushed due to
                                TX_ABRT interrupt */
    } IC_TX_ABRT_SOURCE_b;
  };

  union {
    __IOM unsigned int IC_SLV_DATA_NACK_ONLY; /*!< (@ 0x00000084) Generate Slave
                                              Data NACK Register */

    struct {
      __IOM unsigned int NACK : 1;      /*!< [0..0] Generate NACK. This NACK generation only occurs
                        when DW_apb_i2c is a slave receiver. */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } IC_SLV_DATA_NACK_ONLY_b;
  };

  union {
    __IOM unsigned int IC_DMA_CR; /*!< (@ 0x00000088) DMA Control Register */

    struct {
      __IOM unsigned int RDMAE : 1;     /*!< [0..0] Receive DMA Enable */
      __IOM unsigned int TDMAE : 1;     /*!< [1..1] Transmit DMA Enable.This bit enables/disables
                         the transmit FIFO DMA channel */
      __IM unsigned int RESERVED1 : 30; /*!< [31..2] reserved1 */
    } IC_DMA_CR_b;
  };

  union {
    __IOM unsigned int IC_DMA_TDLR; /*!< (@ 0x0000008C) DMA Transmit Data Level Register */

    struct {
      __IOM unsigned int DMATDL : 4;    /*!< [3..0] This bit field controls the level at which a
                          DMA request is made by the transmit logic */
      __IM unsigned int RESERVED1 : 28; /*!< [31..4] reserved1 */
    } IC_DMA_TDLR_b;
  };

  union {
    __IOM unsigned int IC_DMA_RDLR; /*!< (@ 0x00000090) I2C Receive Data Level Register */

    struct {
      __IOM unsigned int DMARDL : 4;    /*!< [3..0] This bit field controls the level at which a
                          DMA request is made by the receive logic */
      __IM unsigned int RESERVED1 : 28; /*!< [31..4] reserved1 */
    } IC_DMA_RDLR_b;
  };

  union {
    __IOM unsigned int IC_SDA_SETUP; /*!< (@ 0x00000094) I2C SDA Setup Register */

    struct {
      __IOM unsigned int SDA_SETUP : 8; /*!< [7..0] This register controls the amount of time
                             delay (in terms of number of ic_clk clock periods)
                           */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
    } IC_SDA_SETUP_b;
  };

  union {
    __IOM unsigned int IC_ACK_GENERAL_CALL; /*!< (@ 0x00000098) I2C ACK General Call
                                            Register */

    struct {
      __IOM unsigned int ACK_GEN_CALL : 1; /*!< [0..0] ACK General Call */
      __IM unsigned int RESERVED1 : 31;    /*!< [31..1] reserved1    */
    } IC_ACK_GENERAL_CALL_b;
  };

  union {
    __IM unsigned int IC_ENABLE_STATUS; /*!< (@ 0x0000009C) I2C Enable Status Register */

    struct {
      __IM unsigned int IC_EN : 1;                   /*!< [0..0] This bit always reflects the value
                                   driven on the output port ic_en. */
      __IM unsigned int SLV_DISABLED_WHILE_BUSY : 1; /*!< [1..1] This bit indicates if a
                                           potential or active Slave operation
                                                        has been aborted due to
                                           the setting of the IC_ENABLE register
                                                        from 1 to 0 */
      __IM unsigned int SLV_RX_DATA_LOST : 1;        /*!< [2..2] Slave Received Data Lost     */
      __IM unsigned int RESERVED1 : 29;              /*!< [31..3] reserved1 */
    } IC_ENABLE_STATUS_b;
  };

  union {
    __IOM unsigned int IC_FS_SPKLEN; /*!< (@ 0x000000A0) I2C SS and FS Spike
                                     Suppression Limit Register             */

    struct {
      __IOM unsigned int IC_FS_SPKLEN : 8; /*!< [7..0] This register sets the
                                           duration, measured in ic_clk cycles,
                                                      of the longest spike in the
                                           SCL or SDA lines that are filtered
                                                      out by the spike
                                           suppression logic */
      __IM unsigned int RESERVED1 : 24;    /*!< [31..8] reserved1    */
    } IC_FS_SPKLEN_b;
  };

  union {
    __IOM unsigned int IC_HS_SPKLEN; /*!< (@ 0x000000A4) I2C HS Spike Suppression
                                     Limit Register                    */

    struct {
      __IOM unsigned int IC_HS_SPKLEN : 8; /*!< [7..0] This register sets the
                                           duration, measured in ic_clk cycles,
                                                      of the longest spike in the
                                           SCL or SDA lines that are filtered
                                                      out by the spike
                                           suppression logic */
      __IM unsigned int RESERVED1 : 24;    /*!< [31..8] reserved1    */
    } IC_HS_SPKLEN_b;
  };

  union {
    __IM unsigned int IC_CLR_RESTART_DET; /*!< (@ 0x000000A8) Clear RESTART_DET
                                          Interrupt Register */

    struct {
      __IM unsigned int CLR_RESTART_DET : 1; /*!< [0..0] Read this register to clear
                                             the RESTART_DET interrupt (bit 12)
                                             of the IC_RAW_INTR_STAT registe */
      __IM unsigned int RESERVED1 : 31;      /*!< [31..1] reserved1      */
    } IC_CLR_RESTART_DET_b;
  };

  union {
    __IOM unsigned int IC_SCL_STUCK_AT_LOW_TIMEOUT; /*!< (@ 0x000000AC) I2C SCL
                                                    Stuck at Low Timeout */

    struct {
      __IOM unsigned int IC_SCL_STUCK_LOW_TIMEOUT : 32; /*!< [31..0] Generates the interrupt to
                                           indicate SCL stuck at low if it
                                           detects the SCL stuck at low for the
                                           IC_SCL_STUCK_LOW_TIMEOUT in units of
                                           ic_clk period */
    } IC_SCL_STUCK_AT_LOW_TIMEOUT_b;
  };

  union {
    __IOM unsigned int IC_SDA_STUCK_AT_LOW_TIMEOUT; /*!< (@ 0x000000B0) I2C SDA
                                                    Stuck at Low Timeout */

    struct {
      __IOM unsigned int IC_SDA_STUCK_LOW_TIMEOUT : 32; /*!< [31..0] Initiates the recovery of
                                           SDA line , if it detects the SDA stuck
                                           at low for the
                                           IC_SDA_STUCK_LOW_TIMEOUT in units of
                                           ic_clk period. */
    } IC_SDA_STUCK_AT_LOW_TIMEOUT_b;
  };

  union {
    __IM unsigned int IC_CLR_SCL_STUCK_DET; /*!< (@ 0x000000B4) Clear SCL Stuck at
                                            Low Detect Interrupt Register */

    struct {
      __IM unsigned int CLR_SCL_STUCK : 1; /*!< [0..0] Read this register to clear
                                           the SCL_STUCK_DET interrupt */
      __IM unsigned int RESERVED1 : 31;    /*!< [31..1] reserved1    */
    } IC_CLR_SCL_STUCK_DET_b;
  };

  union {
    __IM unsigned int IC_DEVICE_ID; /*!< (@ 0x000000B8) I2C Device ID */

    struct {
      __IM unsigned int DEVICE_ID : 24; /*!< [23..0] Contains the Device-ID of the component
                              assigned through the configuration parameter */
      __IM unsigned int RESERVED1 : 8;  /*!< [31..24] reserved1 */
    } IC_DEVICE_ID_b;
  };

  union {
    __IOM unsigned int IC_SMBUS_CLOCK_LOW_SEXT; /*!< (@ 0x000000BC) SMBUS Slave Clock Extend
                                     Timeout Register                  */

    struct {
      __IOM unsigned int SMBUS_CLK_LOW_SEXT_TIMEOUT : 32; /*!< [31..0] The values in this
                                               register are in units of ic_clk
                                               period.   */
    } IC_SMBUS_CLOCK_LOW_SEXT_b;
  };

  union {
    __IOM unsigned int IC_SMBUS_CLOCK_LOW_MEXT; /*!< (@ 0x000000C0) SMBUS Master extend clock
                                     Timeout Register                 */

    struct {
      __IOM unsigned int SMBUS_CLK_LOW_MEXT_TIMEOUT : 32; /*!< [31..0] The values in this
                                               register are in units of ic_clk
                                               period..  */
    } IC_SMBUS_CLOCK_LOW_MEXT_b;
  };

  union {
    __IOM unsigned int IC_SMBUS_THIGH_MAX_IDLE_COUNT; /*!< (@ 0x000000C4) SMBus Thigh MAX
                                           Bus-Idle count Register */

    struct {
      __IOM unsigned int SMBUS_THIGH_MAX_BUS_IDLE_CNT : 16; /*!< [15..0] The values in this
                                                 register are in units of ic_clk
                                                 period. */
      __IOM unsigned int RESERVED1 : 16;                    /*!< [31..16] Reserved1         */
    } IC_SMBUS_THIGH_MAX_IDLE_COUNT_b;
  };

  union {
    __IOM unsigned int IC_SMBUS_INTR_STAT; /*!< (@ 0x000000C8) SMBUS Interrupt
                                           Status Register */

    struct {
      __IOM unsigned int RESERVED1 : 32; /*!< [31..0] Reserved1 */
    } IC_SMBUS_INTR_STAT_b;
  };

  union {
    __IOM unsigned int IC_SMBUS_INTR_MASK; /*!< (@ 0x000000CC) Interrupt Mask Register */

    struct {
      __IOM unsigned int RESERVED1 : 32; /*!< [31..0] Reserved1 */
    } IC_SMBUS_INTR_MASK_b;
  };

  union {
    __IOM unsigned int IC_SMBUS_INTR_RAW_STATUS; /*!< (@ 0x000000D0) SMBUS Raw
                                                 Interrupt Status Register */

    struct {
      __IOM unsigned int RESERVED1 : 32; /*!< [31..0] Reserved1. */
    } IC_SMBUS_INTR_RAW_STATUS_b;
  };

  union {
    __IOM unsigned int IC_CLR_SMBUS_INTR; /*!< (@ 0x000000D4) Clear SMBUS Interrupt
                                          Register                             */

    struct {
      __IOM unsigned int RESERVED1 : 32; /*!< [31..0] RESERVED1 */
    } IC_CLR_SMBUS_INTR_b;
  };

  union {
    __IOM unsigned int IC_OPTIONAL_SAR; /*!< (@ 0x000000D8) Optional Slave Address
                                        Register                            */

    struct {
      __IOM unsigned int RESERVED1 : 32; /*!< [31..0] Reserved1. */
    } IC_OPTIONAL_SAR_b;
  };

  union {
    __IOM unsigned int IC_SMBUS_UDID_LSB; /*!< (@ 0x000000DC) SMBUS ARP UDID LSB Register */

    struct {
      __IOM unsigned int IC_SMBUS_ARP_UDID_LSB : 32; /*!< [31..0] This field is used to store
                                          the LSB 32 bit value of slave unique
                                          device identifier used in Address
                                          Resolution Protocol. */
    } IC_SMBUS_UDID_LSB_b;
  };
  __IM unsigned int RESERVED[5];

  union {
    __IM unsigned int IC_COMP_PARAM_1; /*!< (@ 0x000000F4) I2C HS Spike Suppression
                                       Limit Register                    */

    struct {
      __IM unsigned int CLR_RESTART_DET : 2;    /*!< [1..0] Read this register to clear the
                                   RESTART_DET interrupt (bit 12) of the
                                   IC_RAW_INTR_STAT register */
      __IM unsigned int MAX_SPEED_MODE : 2;     /*!< [3..2] Maximum Speed Mode */
      __IM unsigned int HC_COUNT_VALUES : 1;    /*!< [4..4] Hard Code the count values   */
      __IM unsigned int INTR_IO : 1;            /*!< [5..5] Single Interrupt Output port */
      __IM unsigned int HAS_DMA : 1;            /*!< [6..6] DMA Handshake Interface signal */
      __IM unsigned int ADD_ENCODED_PARAMS : 1; /*!< [7..7] Add Encoded Parameters */
      __IM unsigned int RX_BUFFER_DEPTH : 8;    /*!< [15..8] Depth of receive buffer;the buffer
                                   is 8 bits wide;2 to 256 */
      __IM unsigned int TX_BUFFER_DEPTH : 8;    /*!< [23..16] Depth of Transmit buffer;the buffer
                                   is 8 bits wide;2 to 256 */
      __IM unsigned int RESERVED1 : 8;          /*!< [31..24] reserved1 */
    } IC_COMP_PARAM_1_b;
  };

  union {
    __IM unsigned int IC_COMP_VERSION; /*!< (@ 0x000000F8) I2C Component Version Register */

    struct {
      __IM unsigned int IC_COMP_VERSION : 32; /*!< [31..0] Signifies the component
                                              version */
    } IC_COMP_VERSION_b;
  };

  union {
    __IM unsigned int IC_COMP_TYPE; /*!< (@ 0x000000FC) I2C Component Type Register */

    struct {
      __IM unsigned int IC_COMP_TYPE : 32; /*!< [31..0] Design ware Component Type
                                           number = 0x44_57_01_40 */
    } IC_COMP_TYPE_b;
  };
} I2C0_Type; /*!< Size = 256 (0x100) */

/* ===========================================================================================================================
  */
/* ================                                           MCPWM
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief The Motor Control PWM (MCPWM) controller is used to generate a
   periodic pulse waveform, which is useful in motor control and power control
   applications (MCPWM)
   */

typedef struct { /*!< (@ 0x47070000) MCPWM Structure */

  union {
    __IM unsigned int PWM_INTR_STS; /*!< (@ 0x00000000) PWM Interrupt Status Register */

    struct {
      __IM unsigned int RISE_PWM_TIME_PERIOD_MATCH_INTR_CH0 : 1; /*!< [0..0] This time base
                                                       interrupt for 0th channel
                                                       without considering
                                                        postscaler */
      __IM unsigned int PWM_TIME_PRD_MATCH_INTR_CH0 : 1;         /*!< [1..1] This time base interrupt
                                               for 0th channel, which considers
                                                        postscaler value */
      __IM unsigned int FLT_A_INTR : 1;                          /*!< [2..2] When the fault A pin is driven
                                        low, this interrupt is raised. */
      __IM unsigned int FLT_B_INTR : 1;                          /*!< [3..3] When the fault B pin is driven
                                        low, this interrupt is raised. */
      __IM unsigned int RISE_PWM_TIME_PERIOD_MATCH_INTR_CH1 : 1; /*!< [4..4] This time base
                                                       interrupt for 1st channel
                                                       without considering
                                                        postscaler value */
      __IM unsigned int PWM_TIME_PRD_MATCH_INTR_CH1 : 1;         /*!< [5..5] This time base interrupt
                                               for 1st channel, which considers
                                                        postscaler value. */
      __IM unsigned int RISE_PWM_TIME_PERIOD_MATCH_INTR_CH2 : 1; /*!< [6..6] This time base
                                                       interrupt for 2nd channel
                                                       without considering
                                                        postscaler value. */
      __IM unsigned int PWM_TIME_PRD_MATCH_INTR_CH2 : 1;         /*!< [7..7] This time base interrupt
                                               for 2nd channel, which considers
                                                        postscaler value */
      __IM unsigned int RISE_PWM_TIME_PERIOD_MATCH_INTR_CH3 : 1; /*!< [8..8] This time base
                                                       interrupt for 3rd channel
                                                       without considering
                                                        postscaler value. */
      __IM unsigned int PWM_TIME_PRD_MATCH_INTR_CH3 : 1;         /*!< [9..9] This time base interrupt
                                               for 3rd channel, which considers
                                                        postscaler value. */
      __IM unsigned int RESERVED1 : 22;                          /*!< [31..10] reserved1        */
    } PWM_INTR_STS_b;
  };

  union {
    __IOM unsigned int PWM_INTR_UNMASK; /*!< (@ 0x00000004) PWM Interrupt Unmask Register */

    struct {
      __IOM unsigned int PWM_INTR_UNMASK : 16; /*!< [15..0] Interrupt Unmask */
      __IOM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1       */
    } PWM_INTR_UNMASK_b;
  };

  union {
    __IOM unsigned int PWM_INTR_MASK; /*!< (@ 0x00000008) PWM Interrupt mask Register */

    struct {
      __IOM unsigned int PWM_INTR_UNMASK : 16; /*!< [15..0] Interrupt Mask */
      __IOM unsigned int RESERVED1 : 16;       /*!< [31..16] reserved1       */
    } PWM_INTR_MASK_b;
  };

  union {
    __IOM unsigned int PWM_INTR_ACK; /*!< (@ 0x0000000C) PWM Interrupt
                                     Acknowledgement Register */

    struct {
      __OM unsigned int RISE_PWM_TIME_PERIOD_MATCH_CH0_ACK : 1; /*!< [0..0] pwm time
                                                                period match
                                                                interrupt for 0th
                                                                channel will be
                                                                cleared. */
      __OM unsigned int PWM_TIME_PRD_MATCH_INTR_CH0_ACK : 1;    /*!< [1..1] pwm time period match
                                                   interrupt for 0th channel will
                                                        be cleared */
      __OM unsigned int FLT_A_INTR_ACK : 1;                     /*!< [2..2] pwm fault A interrupt will
                                            be cleared. */
      __OM unsigned int FLT_B_INTR_ACK : 1;                     /*!< [3..3] pwm fault B interrupt will
                                            be cleared. */
      __OM unsigned int RISE_PWM_TIME_PERIOD_MATCH_CH1_ACK : 1; /*!< [4..4] pwm time
                                                                period match
                                                                interrupt for 1st
                                                                channel will be
                                                                cleared */
      __OM unsigned int PWM_TIME_PRD_MATCH_INTR_CH1_ACK : 1;    /*!< [5..5] pwm time period match
                                                   interrupt for 1st channel will
                                                        be cleared. */
      __OM unsigned int RISE_PWM_TIME_PERIOD_MATCH_CH2_ACK : 1; /*!< [6..6] pwm time
                                                                period match
                                                                interrupt for 2nd
                                                                channel will be
                                                                cleared. */
      __OM unsigned int PWM_TIME_PRD_MATCH_INTR_CH2_ACK : 1;    /*!< [7..7] pwm time period match
                                                   interrupt for 2nd channel will
                                                        be cleared. */
      __OM unsigned int RISE_PWM_TIME_PERIOD_MATCH_CH3_ACK : 1; /*!< [8..8] pwm time
                                                                period match
                                                                interrupt for 3rd
                                                                channel will be
                                                                cleared. */
      __OM unsigned int PWM_TIME_PRD_MATCH_INTR_CH3_ACK : 1;    /*!< [9..9] pwm time period match
                                                   interrupt for 3rd channel will
                                                        be cleared. */
      __IOM unsigned int RESERVED1 : 22;                        /*!< [31..10] reserved1           */
    } PWM_INTR_ACK_b;
  };
  __IM unsigned int RESERVED[6];

  union {
    __IOM unsigned int PWM_TIME_PRD_WR_REG_CH0; /*!< (@ 0x00000028) Base timer
                                                period register of channel 0 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_REG_WR_VALUE_CH0 : 16; /*!< [15..0] Value to update the
                                                  base timer period register of
                                                  channel
                                                        0 */
      __IOM unsigned int RESERVED1 : 16;                     /*!< [31..16] reserved1          */
    } PWM_TIME_PRD_WR_REG_CH0_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH0; /*!< (@ 0x0000002C) Base time
                                                     counter initial value
                                                     register for channel 0 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH0 : 16; /*!< [15..0] To update the base
                                                 time counter initial value for
                                                 channel
                                                        0 */
      __IOM unsigned int RESERVED1 : 16;                    /*!< [31..16] reserved1         */
    } PWM_TIME_PRD_CNTR_WR_REG_CH0_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_PARAM_REG_CH0; /*!< (@ 0x00000030) Base time period config
                                        parameter's register for channel0 */

    struct {
      __IOM unsigned int TMR_OPEARATING_MODE_CH0 : 3;            /*!< [2..0] Base timer operating mode for
                                           channel0                            */
      __IOM unsigned int RESERVED1 : 1;                          /*!< [3..3] reserved1    */
      __IOM unsigned int PWM_TIME_PRD_PRE_SCALAR_VALUE_CH0 : 3;  /*!< [6..4] Base timer input
                                                     clock pre scale select value
                                                     for channel0. */
      __IOM unsigned int RESERVED2 : 1;                          /*!< [7..7] reserved2              */
      __IOM unsigned int PWM_TIME_PRD_POST_SCALAR_VALUE_CH0 : 4; /*!< [11..8] Time base output
                                                      post scale bits for
                                                      channel0         */
      __IOM unsigned int RESERVED3 : 20;                         /*!< [31..12] reserved3              */
    } PWM_TIME_PRD_PARAM_REG_CH0_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CTRL_REG_CH0; /*!< (@ 0x00000034) Base time counter initial
                                       value register for channel 0 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< [0..0] Time period counter
                                                 soft reset */
      __IOM unsigned int PWM_TIME_BASE_EN_FRM_REG_CH0 : 1;  /*!< [1..1] Base timer enable for
                                                channnel0 */
      __IOM unsigned int PWM_SFT_RST : 1;                   /*!< [2..2] MC PWM soft reset       */
      __IM unsigned int RESERVED1 : 29;                     /*!< [31..3] reserved1         */
    } PWM_TIME_PRD_CTRL_REG_CH0_b;
  };

  union {
    __IM unsigned int PWM_TIME_PRD_STS_REG_CH0; /*!< (@ 0x00000038) Base time period
                                                status register for channel0 */

    struct {
      __IM unsigned int PWM_TIME_PRD_DIR_STS_CH0 : 1; /*!< [0..0] Time period counter
                                            direction status for channel0 */
      __IM unsigned int RESERVED1 : 31;               /*!< [31..1] reserved1     */
    } PWM_TIME_PRD_STS_REG_CH0_b;
  };

  union {
    __IM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH0; /*!< (@ 0x0000003C) Base Time
                                                   period counter current value
                                                   register for channel0 */

    struct {
      __IM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH0 : 16; /*!< [15..0] Time period counter
                                                current value for channel0 */
      __IM unsigned int RESERVED1 : 16;                   /*!< [31..16] reserved1         */
    } PWM_TIME_PRD_CNTR_VALUE_CH0_b;
  };
  __IM unsigned int RESERVED1[4];

  union {
    __IOM unsigned int PWM_DUTYCYCLE_CTRL_SET_REG; /*!< (@ 0x00000050) Duty cycle
                                                   Control Set Register */

    struct {
      __IOM unsigned int IMDT_DUTYCYCLE_UPDATE_EN : 4; /*!< [3..0] Enable to update the duty
                                            cycle immediately */
      __IOM unsigned int DUTYCYCLE_UPDATE_DISABLE : 4; /*!< [7..4] Duty cycle register updation
                                            disable. There is a separate
                                                        bit for each channel */
      __IM unsigned int RESERVED1 : 24;                /*!< [31..8] reserved1     */
    } PWM_DUTYCYCLE_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int PWM_DUTYCYCLE_CTRL_RESET_REG; /*!< (@ 0x00000054) Duty cycle
                                                     Control Reset Register */

    struct {
      __IOM unsigned int IMDT_DUTYCYCLE_UPDATE_EN : 4; /*!< [3..0] Enable to update the duty
                                            cycle immediately */
      __IOM unsigned int DUTYCYCLE_UPDATE_DISABLE : 4; /*!< [7..4] Duty cycle
                                                       register updation disable.
                                                       There is a separate bit
                                                       for each channel. */
      __IM unsigned int RESERVED1 : 24;                /*!< [31..8] reserved1                */
    } PWM_DUTYCYCLE_CTRL_RESET_REG_b;
  };

  union {
    __IOM unsigned int PWM_DUTYCYCLE_REG_WR_VALUE[4]; /*!< (@ 0x00000058) Duty cycle Value
                                           Register for Channel0 to channel3 */

    struct {
      __IOM unsigned int PWM_DUTYCYCLE_REG_WR_VALUE_CH : 16; /*!< [15..0] Duty cycle value for
                                                  channel0 to channel3 */
      __IM unsigned int RESERVED1 : 16;                      /*!< [31..16] reserved1           */
    } PWM_DUTYCYCLE_REG_WR_VALUE_b[4];
  };
  __IM unsigned int RESERVED2[4];

  union {
    __IOM unsigned int PWM_DEADTIME_CTRL_SET_REG; /*!< (@ 0x00000078) Dead time
                                                  Control Set Register */

    struct {
      __IOM unsigned int DEADTIME_SELECT_ACTIVE : 4;   /*!< [3..0] Dead time select bits for PWM
                                          going active                         */
      __IOM unsigned int DEADTIME_SELECT_INACTIVE : 4; /*!< [7..4] Dead time select bits for
                                            PWM going inactive */
      __IOM unsigned int DEADTIME_DISABLE_FRM_REG : 4; /*!< [11..8] Dead time counter soft
                                            reset for each channel. */
      __IM unsigned int RESERVED1 : 20;                /*!< [31..12] reserved1     */
    } PWM_DEADTIME_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int PWM_DEADTIME_CTRL_RESET_REG; /*!< (@ 0x0000007C) Dead time
                                                    Control Reset Register */

    struct {
      __IOM unsigned int DEADTIME_SELECT_ACTIVE : 4;   /*!< [3..0] Dead time select bits for PWM
                                          going active                         */
      __IOM unsigned int DEADTIME_SELECT_INACTIVE : 4; /*!< [7..4] Dead time select bits for
                                            PWM going inactive */
      __IOM unsigned int DEADTIME_DISABLE_FRM_REG : 4; /*!< [11..8] Dead time counter soft
                                            reset for each channel. */
      __IM unsigned int RESERVED1 : 20;                /*!< [31..12] reserved1     */
    } PWM_DEADTIME_CTRL_RESET_REG_b;
  };

  union {
    __IOM unsigned int PWM_DEADTIME_PRESCALE_SELECT_A; /*!< (@ 0x00000080) Dead time Prescale
                                            Select Register for A */

    struct {
      __IOM unsigned int DEADTIME_PRESCALE_SELECT_A : 8; /*!< [7..0] Dead time prescale
                                              selection bits for unit A. */
      __IM unsigned int RESERVED1 : 24;                  /*!< [31..8] reserved1       */
    } PWM_DEADTIME_PRESCALE_SELECT_A_b;
  };

  union {
    __IOM unsigned int PWM_DEADTIME_PRESCALE_SELECT_B; /*!< (@ 0x00000084) Dead time Prescale
                                            Select Register for B */

    struct {
      __IOM unsigned int DEADTIME_PRESCALE_SELECT_B : 8; /*!< [7..0] Dead time prescale
                                              selection bits for unit B */
      __IM unsigned int RESERVED1 : 24;                  /*!< [31..8] reserved1       */
    } PWM_DEADTIME_PRESCALE_SELECT_B_b;
  };
  __IOM MCPWM_PWM_DEADTIME_Type PWM_DEADTIME[4]; /*!< (@ 0x00000088) [0..3] */
  __IM unsigned int RESERVED3[8];

  union {
    __IOM unsigned int PWM_OP_OVERRIDE_CTRL_SET_REG; /*!< (@ 0x000000C8) output override
                                          control set register */

    struct {
      __IOM unsigned int OP_OVERRIDE_SYNC : 1; /*!< [0..0] Output override is synced with pwm
                                    time period depending on operating mode */
      __IM unsigned int RESERVED1 : 31;        /*!< [31..1] reserved1 */
    } PWM_OP_OVERRIDE_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int PWM_OP_OVERRIDE_CTRL_RESET_REG; /*!< (@ 0x000000CC) output override
                                            control reset register */

    struct {
      __IOM unsigned int OP_OVERRIDE_SYNC : 1; /*!< [0..0] Output override is synced with pwm
                                    time period depending on operating mode */
      __IOM unsigned int RESERVED1 : 31;       /*!< [31..1] reserved1 */
    } PWM_OP_OVERRIDE_CTRL_RESET_REG_b;
  };

  union {
    __IOM unsigned int PWM_OP_OVERRIDE_ENABLE_SET_REG; /*!< (@ 0x000000D0) output override
                                            enable set register */

    struct {
      __IOM unsigned int PWM_OP_OVERRIDE_ENABLE_REG : 8; /*!< [7..0] Pwm output over
                                                         ride enable */
      __IOM unsigned int RESERVED1 : 24;                 /*!< [31..8] reserved1                 */
    } PWM_OP_OVERRIDE_ENABLE_SET_REG_b;
  };

  union {
    __IOM unsigned int PWM_OP_OVERRIDE_ENABLE_RESET_REG; /*!< (@ 0x000000D4) output override
                                              enable reset register */

    struct {
      __IOM unsigned int PWM_OP_OVERRIDE_ENABLE_REG : 8; /*!< [7..0] Pwm output over
                                                         ride enable */
      __IOM unsigned int RESERVED1 : 24;                 /*!< [31..8] reserved1                 */
    } PWM_OP_OVERRIDE_ENABLE_RESET_REG_b;
  };

  union {
    __IOM unsigned int PWM_OP_OVERRIDE_VALUE_SET_REG; /*!< (@ 0x000000D8) output override value
                                           set register                        */

    struct {
      __IOM unsigned int OP_OVERRIDE_VALUE : 8; /*!< [7..0] Pwm output over ride value.     */
      __IOM unsigned int RESERVED1 : 24;        /*!< [31..8] reserved1 */
    } PWM_OP_OVERRIDE_VALUE_SET_REG_b;
  };

  union {
    __IOM unsigned int PWM_OP_OVERRIDE_VALUE_RESET_REG; /*!< (@ 0x000000DC) output override
                                             enable reset register */

    struct {
      __IOM unsigned int OP_OVERRIDE_VALUE : 8; /*!< [7..0] Pwm output over ride value.     */
      __IOM unsigned int RESERVED1 : 24;        /*!< [31..8] reserved1 */
    } PWM_OP_OVERRIDE_VALUE_RESET_REG_b;
  };

  union {
    __IOM unsigned int PWM_FLT_OVERRIDE_CTRL_SET_REG; /*!< (@ 0x000000E0) fault override
                                           control set register */

    struct {
      __IOM unsigned int FLT_A_MODE : 1;         /*!< [0..0] Fault A mode    */
      __IOM unsigned int FLT_B_MODE : 1;         /*!< [1..1] Fault B mode    */
      __IOM unsigned int OP_POLARITY_H : 1;      /*!< [2..2] Ouput polarity for high (H3,
                                            H2, H1, H0) side signals */
      __IOM unsigned int OP_POLARITY_L : 1;      /*!< [3..3] Ouput polarity for low (L3,
                                            L2, L1, L0) side signals. */
      __IOM unsigned int FLT_A_ENABLE : 4;       /*!< [7..4] Fault A enable. Separate
                                            enable bit is present for channel  */
      __IOM unsigned int FLT_B_ENABLE : 4;       /*!< [11..8] Fault B enable. Separate
                                            enable bit is present for channel  */
      __IOM unsigned int COMPLEMENTARY_MODE : 4; /*!< [15..12] PWM I/O pair mode    */
      __IOM unsigned int RESERVED1 : 16;         /*!< [31..16] reserved1 */
    } PWM_FLT_OVERRIDE_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int PWM_FLT_OVERRIDE_CTRL_RESET_REG; /*!< (@ 0x000000E4) fault override
                                             control reset register */

    struct {
      __IOM unsigned int FLT_A_MODE : 1;         /*!< [0..0] Fault B mode    */
      __IOM unsigned int FLT_B_MODE : 1;         /*!< [1..1] Fault B mode    */
      __IOM unsigned int OP_POLARITY_H : 1;      /*!< [2..2] Ouput polarity for high (H3,
                                            H2, H1, H0) side signals */
      __IOM unsigned int OP_POLARITY_L : 1;      /*!< [3..3] Ouput polarity for low (L3,
                                            L2, L1, L0) side signals. */
      __IOM unsigned int FLT_A_ENABLE : 4;       /*!< [7..4] Fault A enable. Separate
                                            enable bit is present for channel  */
      __IOM unsigned int FLT_B_ENABLE : 4;       /*!< [11..8] Fault B enable. Separate
                                            enable bit is present for channel  */
      __IOM unsigned int COMPLEMENTARY_MODE : 4; /*!< [15..12] PWM I/O pair mode    */
      __IOM unsigned int RESERVED1 : 16;         /*!< [31..16] reserved1 */
    } PWM_FLT_OVERRIDE_CTRL_RESET_REG_b;
  };

  union {
    __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_REG; /*!< (@ 0x000000E8) Fault input
                                                     A PWM override value */

    struct {
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_L0 : 1; /*!< [0..0] 0 bit for L0 */
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_L1 : 1; /*!< [1..1] 1 bit for L1 */
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_L2 : 1; /*!< [2..2] 2 bit for L2 */
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_L3 : 1; /*!< [3..3] 3 bit for L3 */
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_H0 : 1; /*!< [4..4] 4 bit for H0 */
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_H1 : 1; /*!< [5..5] 5 bit for H1 */
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_H2 : 1; /*!< [6..6] 6 bit for H2 */
      __IOM unsigned int PWM_FLT_A_OVERRIDE_VALUE_H3 : 1; /*!< [7..7] 7 bit for H3 */
      __IOM unsigned int RESERVED1 : 24;                  /*!< [31..8] reserved1       */
    } PWM_FLT_A_OVERRIDE_VALUE_REG_b;
  };

  union {
    __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_REG; /*!< (@ 0x000000EC) Fault input
                                                     B PWM override value */

    struct {
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_L0 : 1; /*!< [0..0] 0 bit for L0 */
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_L1 : 1; /*!< [1..1] 1 bit for L1 */
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_L2 : 1; /*!< [2..2] 2 bit for L2 */
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_L3 : 1; /*!< [3..3] 3 bit for L3 */
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_H0 : 1; /*!< [4..4] 4 bit for H0 */
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_H1 : 1; /*!< [5..5] 5 bit for H1 */
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_H2 : 1; /*!< [6..6] 6 bit for H2 */
      __IOM unsigned int PWM_FLT_B_OVERRIDE_VALUE_H3 : 1; /*!< [7..7] 7 bit for H3 */
      __IOM unsigned int RESERVED1 : 24;                  /*!< [31..8] reserved1       */
    } PWM_FLT_B_OVERRIDE_VALUE_REG_b;
  };

  union {
    __IOM unsigned int PWM_SVT_CTRL_SET_REG; /*!< (@ 0x000000F0) NONE */

    struct {
      __IOM unsigned int SVT_ENABLE_FRM : 1;    /*!< [0..0] Special event trigger enable. This is
                                  used to enable
                                           generation special event trigger */
      __IOM unsigned int SVT_DIRECTION_FRM : 1; /*!< [1..1] Special event trigger
                                                for time base direction */
      __IOM unsigned int RESERVED1 : 30;        /*!< [31..2] reserved1        */
    } PWM_SVT_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int PWM_SVT_CTRL_RESET_REG; /*!< (@ 0x000000F4) Special event
                                               control reset register */

    struct {
      __IOM unsigned int SVT_ENABLE_FRM : 1;    /*!< [0..0] Special event trigger enable. This is
                                  used to enable
                                           generation special event trigger */
      __IOM unsigned int SVT_DIRECTION_FRM : 1; /*!< [1..1] Special event trigger
                                                for time base direction */
      __IOM unsigned int RESERVED1 : 30;        /*!< [31..2] reserved1        */
    } PWM_SVT_CTRL_RESET_REG_b;
  };

  union {
    __IOM unsigned int PWM_SVT_PARAM_REG; /*!< (@ 0x000000F8) Special event
                                          parameter register */

    struct {
      __IOM unsigned int SVT_POSTSCALER_SELECT : 4; /*!< [3..0] PWM special event trigger
                                         output postscale select bits */
      __IOM unsigned int RESERVED1 : 28;            /*!< [31..4] reserved1 */
    } PWM_SVT_PARAM_REG_b;
  };

  union {
    __IOM unsigned int PWM_SVT_COMPARE_VALUE_REG; /*!< (@ 0x000000FC) Special event
                                                  compare value register */

    struct {
      __IOM unsigned int PWM_SVT_COMPARE_VALUE : 16; /*!< [15..0] Special event compare value.
                                          This is used to compare with pwm time
                                          period counter to generate special
                                          event trigger */
      __IOM unsigned int RESERVED1 : 16;             /*!< [31..16] reserved1  */
    } PWM_SVT_COMPARE_VALUE_REG_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_WR_REG_CH1; /*!< (@ 0x00000100) Base timer
                                                period register of channel1 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_REG_WR_VALUE_CH1 : 16; /*!< [15..0] Value to update the
                                                  base timer period register of
                                                  channel
                                                        1 */
      __IOM unsigned int RESERVED1 : 16;                     /*!< [31..16] reserved1          */
    } PWM_TIME_PRD_WR_REG_CH1_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH1; /*!< (@ 0x00000104) Base time
                                                     counter initial value
                                                     register for channel1 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH1 : 16; /*!< [15..0] To update the base
                                                 time counter initial value for
                                                 channel
                                                        1 */
      __IOM unsigned int RESERVED1 : 16;                    /*!< [31..16] reserved1         */
    } PWM_TIME_PRD_CNTR_WR_REG_CH1_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_PARAM_REG_CH1; /*!< (@ 0x00000108) NONE */

    struct {
      __IOM unsigned int TMR_OPEARATING_MODE_CH1 : 3;            /*!< [2..0] Base timer operating mode for
                                           channel1                            */
      __IOM unsigned int RESERVED1 : 1;                          /*!< [3..3] reserved1    */
      __IOM unsigned int PWM_TIME_PRD_PRE_SCALAR_VALUE_CH1 : 3;  /*!< [6..4] Base timer input
                                                     clock prescale select value
                                                     for channel1. */
      __IOM unsigned int RESERVED2 : 1;                          /*!< [7..7] reserved2              */
      __IOM unsigned int PWM_TIME_PRD_POST_SCALAR_VALUE_CH1 : 4; /*!< [11..8] Time base output
                                                      post scale bits for
                                                      channel1         */
      __IOM unsigned int RESERVED3 : 20;                         /*!< [31..12] reserved3              */
    } PWM_TIME_PRD_PARAM_REG_CH1_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CTRL_REG_CH1; /*!< (@ 0x0000010C) Base time period control
                                       register for channel1             */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< [0..0] Time period counter
                                                 soft reset */
      __IOM unsigned int PWM_TIME_BASE_EN_FRM_REG_CH1 : 1;  /*!< [1..1] Base timer enable for
                                                channnel1 */
      __IOM unsigned int PWM_SFT_RST : 1;                   /*!< [2..2] MC PWM soft reset       */
      __IOM unsigned int RESERVED1 : 29;                    /*!< [31..3] reserved1        */
    } PWM_TIME_PRD_CTRL_REG_CH1_b;
  };

  union {
    __IM unsigned int PWM_TIME_PRD_STS_REG_CH1; /*!< (@ 0x00000110) Base time period
                                                status register for channel1 */

    struct {
      __IM unsigned int PWM_TIME_PRD_DIR_STS_CH1 : 1; /*!< [0..0] Time period counter
                                            direction status for channel1. */
      __IM unsigned int RESERVED1 : 31;               /*!< [31..1] reserved1     */
    } PWM_TIME_PRD_STS_REG_CH1_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH1; /*!< (@ 0x00000114) Time period counter
                                         current value for channel1 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH1 : 1; /*!< [0..0] Time period counter
                                               current value for channel1 */
      __IM unsigned int RESERVED1 : 31;                   /*!< [31..1] reserved1        */
    } PWM_TIME_PRD_CNTR_VALUE_CH1_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_WR_REG_CH2; /*!< (@ 0x00000118) Base timer
                                                period register of channel2 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_REG_WR_VALUE_CH2 : 16; /*!< [15..0] Value to update the
                                                  base timer period register of
                                                  channel
                                                        2 */
      __IOM unsigned int RESERVED1 : 16;                     /*!< [31..16] reserved1          */
    } PWM_TIME_PRD_WR_REG_CH2_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH2; /*!< (@ 0x0000011C) Base time
                                                     counter initial value
                                                     register for channel2 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH2 : 16; /*!< [15..0] To update the base
                                                 time counter initial value for
                                                 channel
                                                        2 */
      __IOM unsigned int RESERVED1 : 16;                    /*!< [31..16] reserved1         */
    } PWM_TIME_PRD_CNTR_WR_REG_CH2_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_PARAM_REG_CH2; /*!< (@ 0x00000120) Base time period config
                                        parameter's register for channel2 */

    struct {
      __IOM unsigned int TMR_OPEARATING_MODE_CH2 : 3;            /*!< [2..0] Base timer operating mode for
                                           channel2                            */
      __IOM unsigned int RESERVED1 : 1;                          /*!< [3..3] reserved1    */
      __IOM unsigned int PWM_TIME_PRD_PRE_SCALAR_VALUE_CH2 : 3;  /*!< [6..4] Base timer input
                                                     clock pre scale select value
                                                     for channel2. */
      __IOM unsigned int RESERVED2 : 1;                          /*!< [7..7] reserved2              */
      __IOM unsigned int PWM_TIME_PRD_POST_SCALAR_VALUE_CH2 : 4; /*!< [11..8] Time base output
                                                      post scale bits for
                                                      channel2         */
      __IOM unsigned int RESERVED3 : 20;                         /*!< [31..12] reserved3              */
    } PWM_TIME_PRD_PARAM_REG_CH2_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CTRL_REG_CH2; /*!< (@ 0x00000124) Base time period control
                                       register for channel2             */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< [0..0] Time period counter
                                                 soft reset */
      __IOM unsigned int PWM_TIME_BASE_EN_FRM_REG_CH2 : 1;  /*!< [1..1] Base timer enable for
                                                channnel2 */
      __IOM unsigned int PWM_SFT_RST : 1;                   /*!< [2..2] MC PWM soft reset       */
      __IOM unsigned int RESERVED1 : 29;                    /*!< [31..3] reserved1        */
    } PWM_TIME_PRD_CTRL_REG_CH2_b;
  };

  union {
    __IM unsigned int PWM_TIME_PRD_STS_REG_CH2; /*!< (@ 0x00000128) Base time period
                                                status register for channel2 */

    struct {
      __IM unsigned int PWM_TIME_PRD_DIR_STS_CH2 : 1; /*!< [0..0] Time period counter
                                            direction status for channel2. */
      __IM unsigned int RESERVED1 : 31;               /*!< [31..1] reserved1     */
    } PWM_TIME_PRD_STS_REG_CH2_b;
  };

  union {
    __IM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH2; /*!< (@ 0x0000012C) Time period counter
                                         current value register for channel2 */

    struct {
      __IM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH2 : 1; /*!< [0..0] Time period counter
                                               current value for channel2 */
      __IM unsigned int RESERVED1 : 11;                  /*!< [11..1] reserved1        */
      __IM unsigned int RESERVED2 : 20;                  /*!< [31..12] reserved2        */
    } PWM_TIME_PRD_CNTR_VALUE_CH2_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_WR_REG_CH3; /*!< (@ 0x00000130) Base timer
                                                period register of channel3 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_REG_WR_VALUE_CH3 : 16; /*!< [15..0] To update the base
                                                  time counter initial value for
                                                  channel
                                                        3 */
      __IOM unsigned int RESERVED1 : 16;                     /*!< [31..16] reserved1          */
    } PWM_TIME_PRD_WR_REG_CH3_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH3; /*!< (@ 0x00000134) Base time
                                                     counter initial value
                                                     register for channel3 */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_WR_REG_CH3 : 16; /*!< [15..0] Value to update the
                                                 base timer period register of
                                                 channel
                                                        3 */
      __IOM unsigned int RESERVED1 : 16;                    /*!< [31..16] reserved1         */
    } PWM_TIME_PRD_CNTR_WR_REG_CH3_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_PARAM_REG_CH3; /*!< (@ 0x00000138) Base time period config
                                        parameter's register for channel3 */

    struct {
      __IOM unsigned int TMR_OPEARATING_MODE_CH3 : 3;            /*!< [2..0] Base timer operating mode for
                                           channel3                            */
      __IOM unsigned int RESERVED1 : 1;                          /*!< [3..3] reserved1    */
      __IOM unsigned int PWM_TIME_PRD_PRE_SCALAR_VALUE_CH3 : 3;  /*!< [6..4] Base timer input
                                                     clock pre scale select value
                                                     for channel2. */
      __IOM unsigned int RESERVED2 : 1;                          /*!< [7..7] reserved2              */
      __IOM unsigned int PWM_TIME_PRD_POST_SCALAR_VALUE_CH3 : 4; /*!< [11..8] Time base output
                                                      post scale bits for
                                                      channel3         */
      __IOM unsigned int RESERVED3 : 20;                         /*!< [31..12] reserved3              */
    } PWM_TIME_PRD_PARAM_REG_CH3_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_CTRL_REG_CH3; /*!< (@ 0x0000013C) Base time period control
                                       register for channel3             */

    struct {
      __IOM unsigned int PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< [0..0] Time period counter
                                                 soft reset */
      __IOM unsigned int PWM_TIME_BASE_EN_FRM_REG_CH3 : 1;  /*!< [1..1] Base timer enable for
                                                channnel3 */
      __IOM unsigned int PWM_SFT_RST : 1;                   /*!< [2..2] MC PWM soft reset       */
      __IOM unsigned int RESERVED1 : 29;                    /*!< [31..3] reserved1        */
    } PWM_TIME_PRD_CTRL_REG_CH3_b;
  };

  union {
    __IM unsigned int PWM_TIME_PRD_STS_REG_CH3; /*!< (@ 0x00000140) Base time period
                                                status register for channel3 */

    struct {
      __IM unsigned int PWM_TIME_PRD_DIR_STS_CH3 : 1; /*!< [0..0] Time period counter
                                            direction status for channel3. */
      __IM unsigned int RESERVED1 : 15;               /*!< [15..1] reserved1     */
      __IM unsigned int RESERVED2 : 16;               /*!< [31..16] reserved2     */
    } PWM_TIME_PRD_STS_REG_CH3_b;
  };

  union {
    __IM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH3; /*!< (@ 0x00000144) Time period counter
                                         current value register for channel3 */

    struct {
      __IM unsigned int PWM_TIME_PRD_CNTR_VALUE_CH3 : 16; /*!< [15..0] Time period counter
                                                current value for channe3 */
      __IM unsigned int RESERVED1 : 16;                   /*!< [31..16] reserved1         */
    } PWM_TIME_PRD_CNTR_VALUE_CH3_b;
  };

  union {
    __IOM unsigned int PWM_TIME_PRD_COMMON_REG; /*!< (@ 0x00000148) Time period
                                                common register */

    struct {
      __IOM unsigned int PWM_TIME_PRD_USE_0TH_TIMER_ONLY : 1; /*!< [0..0] Instead of use four
                                                   base timers for four channels,
                                                   use only one base timer for
                                                   all channels. */
      __IOM unsigned int PWM_TIME_PRD_COMMON_TIMER_VALUE : 2; /*!< [2..1] Base timers select to
                                                   generate special event trigger
                                                 */
      __IOM unsigned int USE_EXT_TIMER_TRIG_FRM_REG : 1;      /*!< [3..3] Enable to use external
                                           trigger for base time counter
                                           increment or decrement. */
      __IOM unsigned int RESERVED1 : 28;                      /*!< [31..4] reserved1      */
    } PWM_TIME_PRD_COMMON_REG_b;
  };
} MCPWM_Type; /*!< Size = 332 (0x14c) */

/* ===========================================================================================================================
  */
/* ================                                           UDMA0
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief DMA Performs data transfers along with Addresses and control
  * information (UDMA0)
  */

typedef struct { /*!< (@ 0x44030000) UDMA0 Structure */

  union {
    __IM unsigned int DMA_STATUS; /*!< (@ 0x00000000) UDMA Status Register */

    struct {
      __IM unsigned int MASTER_ENABLE : 1; /*!< [0..0] Enable status of controller       */
      __IM unsigned int RESERVED1 : 3;     /*!< [3..1] Reserved1 */
      __IM unsigned int STATE : 4;         /*!< [7..4] Current state of the control state machine */
      __IM unsigned int RESERVED2 : 8;     /*!< [15..8] Reserved2    */
      __IM unsigned int CHNLS_MINUS1 : 5;  /*!< [20..16] Number of available DMA
                                          channels minus one */
      __IM unsigned int RESERVED3 : 7;     /*!< [27..21] Reserved3    */
      __IM unsigned int TEST_STATUS : 4;   /*!< [31..28] To reduce the gate count you
                                          can configure the controller        */
    } DMA_STATUS_b;
  };

  union {
    __OM unsigned int DMA_CFG; /*!< (@ 0x00000004) DMA Configuration */

    struct {
      __OM unsigned int MASTER_ENABLE : 1;  /*!< [0..0] Enable for the controller       */
      __OM unsigned int RESERVED1 : 4;      /*!< [4..1] Reserved1 */
      __OM unsigned int CHNL_PROT_CTRL : 3; /*!< [7..5] Sets the AHB-Lite protection by
                                        controlling the HPROT[3:1]]       signal levels as
                                        follows Bit[7]-Controls HPROT[3] to indicate       if
                                        cacheable access is occurring Bit[6]-Controls
                                        HPROT[2]       to indicate if cacheable access is
                                        occurring Bit[5]-Controls
                                                  HPROT[1] to indicate if cacheable
                                        access is occurring                     */
      __OM unsigned int RESERVED2 : 24;     /*!< [31..8] Reserved2 */
    } DMA_CFG_b;
  };

  union {
    __IOM unsigned int CTRL_BASE_PTR; /*!< (@ 0x00000008) Channel Control Data Base
                                      Pointer                          */

    struct {
      __OM unsigned int RESERVED1 : 10;      /*!< [9..0] Reserved1 */
      __IOM unsigned int CTRL_BASE_PTR : 22; /*!< [31..10] Pointer to the base address of the
                                  primary data structure        */
    } CTRL_BASE_PTR_b;
  };

  union {
    __IM unsigned int ALT_CTRL_BASE_PTR; /*!< (@ 0x0000000C) Channel Alternate
                                         Control Data Base Pointer */

    struct {
      __IM unsigned int ALT_CTRL_BASE_PTR : 32; /*!< [31..0] Base address of the
                                                alternative data structure */
    } ALT_CTRL_BASE_PTR_b;
  };

  union {
    __IM unsigned int DMA_WAITONREQUEST_STATUS; /*!< (@ 0x00000010) Channel Wait on
                                                request status register */

    struct {
      __IM unsigned int DMA_WAITONREQ_STATUS : 32; /*!< [31..0] Per Channel wait on
                                                   request status */
    } DMA_WAITONREQUEST_STATUS_b;
  };

  union {
    __OM unsigned int CHNL_SW_REQUEST; /*!< (@ 0x00000014) Channel Software Request */

    struct {
      __OM unsigned int CHNL_SW_REQUEST : 32; /*!< [31..0] Set the appropriate bit to generate
                                    a software DMA request on the corresponding
                                    DMA channel */
    } CHNL_SW_REQUEST_b;
  };

  union {
    __IOM unsigned int CHNL_USEBURST_SET; /*!< (@ 0x00000018) UDMA Channel use burst set */

    struct {
      __IOM unsigned int CHNL_USEBURST_SET : 32; /*!< [31..0] The use burst status,
                                                 or disables dma_sreq[C] from
                                                 generating DMA requests. */
    } CHNL_USEBURST_SET_b;
  };

  union {
    __OM unsigned int CHNL_USEBURST_CLR; /*!< (@ 0x0000001C) UDMA Channel use burst clear */

    struct {
      __OM unsigned int CHNL_USEBURST_CLR : 32; /*!< [31..0] Set the appropriate bit to enable
                                      dma_sreq[] to generate requests */
    } CHNL_USEBURST_CLR_b;
  };

  union {
    __IOM unsigned int CHNL_REQ_MASK_SET; /*!< (@ 0x00000020) UDMA Channel request
                                          mask set Register */

    struct {
      __IOM unsigned int CHNL_REQ_MASK_SET : 32; /*!< [31..0] Returns the request mask status
                                      of dma_req[] and dma_sreq[], or disables
                                      the corresponding channel from generating
                                      DMA requests */
    } CHNL_REQ_MASK_SET_b;
  };

  union {
    __OM unsigned int CHNL_REQ_MASK_CLR; /*!< (@ 0x00000024) UDMA Channel request
                                         mask clear                            */

    struct {
      __OM unsigned int CHNL_REQ_MASK_CLR : 32; /*!< [31..0] Set the appropriate bit
                                                to enable DMA requests for the
                                                      channel corresponding to
                                                dma_req[] and dma_sreq[] */
    } CHNL_REQ_MASK_CLR_b;
  };

  union {
    __IOM unsigned int CHNL_ENABLE_SET; /*!< (@ 0x00000028) UDMA Channel enable register */

    struct {
      __IOM unsigned int CHNL_ENABLE_SET : 32; /*!< [31..0] This Bits are Used to Load the
                                    16bits of Source address           */
    } CHNL_ENABLE_SET_b;
  };

  union {
    __OM unsigned int CHNL_ENABLE_CLR; /*!< (@ 0x0000002C) UDMA Channel enable clear
                                       register                         */

    struct {
      __OM unsigned int CHNL_ENABLE_CLR : 32; /*!< [31..0] Set the appropriate bit to disable
                                    the corresponding DMA channel */
    } CHNL_ENABLE_CLR_b;
  };

  union {
    __IOM unsigned int CHNL_PRI_ALT_SET; /*!< (@ 0x00000030) UDMA Channel primary or
                                         alternate set                      */

    struct {
      __IOM unsigned int CHNL_PRI_ALT_SET : 32; /*!< [31..0] Returns the channel control data
                                     structure status or selects the  alternate
                                     data structure for the corresponding DMA
                                     channel */
    } CHNL_PRI_ALT_SET_b;
  };

  union {
    __OM unsigned int CHNL_PRI_ALT_CLR; /*!< (@ 0x00000034) UDMA Channel primary
                                        alternate clear                       */

    struct {
      __OM unsigned int CHNL_PRI_ALT_CLR : 32; /*!< [31..0] Set the appropriate bit to select
                                     the primary data structure for the
                                     corresponding DMA channel */
    } CHNL_PRI_ALT_CLR_b;
  };

  union {
    __IOM unsigned int CHNL_PRIORITY_SET; /*!< (@ 0x00000038) UDMA Channel Priority Set */

    struct {
      __IOM unsigned int CHNL_PRIORITY_SET : 32; /*!< [31..0] Set the appropriate bit to select
                                      the primary data structure for  the
                                      corresponding DMA channel */
    } CHNL_PRIORITY_SET_b;
  };

  union {
    __OM unsigned int CHNL_PRIORITY_CLR; /*!< (@ 0x0000003C) UDMA Channel Priority Clear */

    struct {
      __OM unsigned int CHNL_PRIORITY_CLR : 32; /*!< [31..0] Set the appropriate bit to select
                                      the default priority level for the
                                      specified DMA channel */
    } CHNL_PRIORITY_CLR_b;
  };
  __IM unsigned int RESERVED[3];

  union {
    __IOM unsigned int ERR_CLR; /*!< (@ 0x0000004C) UDMA Bus Error Clear Register */

    struct {
      __IOM unsigned int ERR_CLR : 1;    /*!< [0..0] Returns the status of dma_err */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved1 */
    } ERR_CLR_b;
  };

  union {
    __IOM unsigned int UDMA_SKIP_DESC_FETCH_REG; /*!< (@ 0x00000050) UDMA skip
                                                 descriptor fetch Register */

    struct {
      __IOM unsigned int SKIP_DESC_FETCH : 32; /*!< [31..0] improving the
                                               performance of transfer and saves
                                               bus cycles. This features has to
                                               be enabled always. */
    } UDMA_SKIP_DESC_FETCH_REG_b;
  };
  __IM unsigned int RESERVED1[491];

  union {
    __IOM unsigned int UDMA_DONE_STATUS_REG; /*!< (@ 0x00000800) UDMA Done status Register */

    struct {
      __IOM unsigned int DONE_STATUS_CHANNEL_0 : 1;  /*!< [0..0] UDMA done Status of
                                                     the channel 0  */
      __IOM unsigned int DONE_STATUS_CHANNEL_1 : 1;  /*!< [1..1] UDMA done Status of
                                                     the channel 1  */
      __IOM unsigned int DONE_STATUS_CHANNEL_2 : 1;  /*!< [2..2] UDMA done Status of
                                                     the channel 2  */
      __IOM unsigned int DONE_STATUS_CHANNEL_3 : 1;  /*!< [3..3] UDMA done Status of
                                                     the channel 3  */
      __IOM unsigned int DONE_STATUS_CHANNEL_4 : 1;  /*!< [4..4] UDMA done Status of
                                                     the channel 4  */
      __IOM unsigned int DONE_STATUS_CHANNEL_5 : 1;  /*!< [5..5] UDMA done Status of
                                                     the channel 5  */
      __IOM unsigned int DONE_STATUS_CHANNEL_6 : 1;  /*!< [6..6] UDMA done Status of
                                                     the channel 6  */
      __IOM unsigned int DONE_STATUS_CHANNEL_7 : 1;  /*!< [7..7] UDMA done Status of
                                                     the channel 7  */
      __IOM unsigned int DONE_STATUS_CHANNEL_8 : 1;  /*!< [8..8] UDMA done Status of
                                                     the channel 8  */
      __IOM unsigned int DONE_STATUS_CHANNEL_9 : 1;  /*!< [9..9] UDMA done Status of
                                                     the channel 9  */
      __IOM unsigned int DONE_STATUS_CHANNEL_10 : 1; /*!< [10..10] UDMA done Status
                                                     of the channel 10 */
      __IOM unsigned int DONE_STATUS_CHANNEL_11 : 1; /*!< [11..11] UDMA done Status
                                                     of the channel 3 */
      __IOM unsigned int DONE_STATUS_CHANNEL_12 : 1; /*!< [12..12] UDMA done Status
                                                     of the channel 12 */
      __IOM unsigned int DONE_STATUS_CHANNEL_13 : 1; /*!< [13..13] UDMA done Status
                                                     of the channel 13 */
      __IOM unsigned int DONE_STATUS_CHANNEL_14 : 1; /*!< [14..14] UDMA done Status
                                                     of the channel 14 */
      __IOM unsigned int DONE_STATUS_CHANNEL_15 : 1; /*!< [15..15] UDMA done Status
                                                     of the channel 15 */
      __IOM unsigned int DONE_STATUS_CHANNEL_16 : 1; /*!< [16..16] UDMA done Status
                                                     of the channel 16 */
      __IOM unsigned int DONE_STATUS_CHANNEL_17 : 1; /*!< [17..17] UDMA done Status
                                                     of the channel 17 */
      __IOM unsigned int DONE_STATUS_CHANNEL_18 : 1; /*!< [18..18] UDMA done Status
                                                     of the channel 18 */
      __IOM unsigned int DONE_STATUS_CHANNEL_19 : 1; /*!< [19..19] UDMA done Status
                                                     of the channel 19 */
      __IOM unsigned int DONE_STATUS_CHANNEL_20 : 1; /*!< [20..20] UDMA done Status
                                                     of the channel 3 */
      __IOM unsigned int DONE_STATUS_CHANNEL_21 : 1; /*!< [21..21] UDMA done Status
                                                     of the channel 21 */
      __IOM unsigned int DONE_STATUS_CHANNEL_22 : 1; /*!< [22..22] UDMA done Status
                                                     of the channel 22 */
      __IOM unsigned int DONE_STATUS_CHANNEL_23 : 1; /*!< [23..23] UDMA done Status
                                                     of the channel 23 */
      __IOM unsigned int DONE_STATUS_CHANNEL_24 : 1; /*!< [24..24] UDMA done Status
                                                     of the channel 24 */
      __IOM unsigned int DONE_STATUS_CHANNEL_25 : 1; /*!< [25..25] UDMA done Status
                                                     of the channel 25 */
      __IOM unsigned int DONE_STATUS_CHANNEL_26 : 1; /*!< [26..26] UDMA done Status
                                                     of the channel 26 */
      __IOM unsigned int DONE_STATUS_CHANNEL_27 : 1; /*!< [27..27] UDMA done Status
                                                     of the channel 27 */
      __IOM unsigned int DONE_STATUS_CHANNEL_28 : 1; /*!< [28..28] UDMA done Status
                                                     of the channel 28 */
      __IOM unsigned int DONE_STATUS_CHANNEL_29 : 1; /*!< [29..29] UDMA done Status
                                                     of the channel 29 */
      __IOM unsigned int DONE_STATUS_CHANNEL_30 : 1; /*!< [30..30] UDMA done Status
                                                     of the channel 30 */
      __IOM unsigned int DONE_STATUS_CHANNEL_31 : 1; /*!< [31..31] UDMA done Status
                                                     of the channel 31 */
    } UDMA_DONE_STATUS_REG_b;
  };

  union {
    __IM unsigned int CHANNEL_STATUS_REG; /*!< (@ 0x00000804) Channel status Register */

    struct {
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_0 : 1;  /*!< [0..0] Reading 1 indicates
                                                  that the channel 0 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_1 : 1;  /*!< [1..1] Reading 1 indicates
                                                  that the channel 1 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_2 : 1;  /*!< [2..2] Reading 1 indicates
                                                  that the channel 2 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_3 : 1;  /*!< [3..3] Reading 1 indicates
                                                  that the channel 3 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_4 : 1;  /*!< [4..4] Reading 1 indicates
                                                  that the channel 4 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_5 : 1;  /*!< [5..5] Reading 1 indicates
                                                  that the channel 5 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_6 : 1;  /*!< [6..6] Reading 1 indicates
                                                  that the channel 6 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_7 : 1;  /*!< [7..7] Reading 1 indicates
                                                  that the channel 7 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_8 : 1;  /*!< [8..8] Reading 1 indicates
                                                  that the channel 8 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_9 : 1;  /*!< [9..9] Reading 1 indicates
                                                  that the channel 9 is busy */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_10 : 1; /*!< [10..10] Reading 1
                                                             indicates that the
                                                             channel 10 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_11 : 1; /*!< [11..11] Reading 1
                                                             indicates that the
                                                             channel 11 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_12 : 1; /*!< [12..12] Reading 1
                                                             indicates that the
                                                             channel 12 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_13 : 1; /*!< [13..13] Reading 1
                                                             indicates that the
                                                             channel 13 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_14 : 1; /*!< [14..14] Reading 1
                                                             indicates that the
                                                             channel 14 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_15 : 1; /*!< [15..15] Reading 1
                                                             indicates that the
                                                             channel 15 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_16 : 1; /*!< [16..16] Reading 1
                                                             indicates that the
                                                             channel 16 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_17 : 1; /*!< [17..17] Reading 1
                                                             indicates that the
                                                             channel 17 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_18 : 1; /*!< [18..18] Reading 1
                                                             indicates that the
                                                             channel 18 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_19 : 1; /*!< [19..19] Reading 1
                                                             indicates that the
                                                             channel 19 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_20 : 1; /*!< [20..20] Reading 1
                                                             indicates that the
                                                             channel 20 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_21 : 1; /*!< [21..21] Reading 1
                                                             indicates that the
                                                             channel 21 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_22 : 1; /*!< [22..22] Reading 1
                                                             indicates that the
                                                             channel 22 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_23 : 1; /*!< [23..23] Reading 1
                                                             indicates that the
                                                             channel 23 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_24 : 1; /*!< [24..24] Reading 1
                                                             indicates that the
                                                             channel 24 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_25 : 1; /*!< [25..25] Reading 1
                                                             indicates that the
                                                             channel 25 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_26 : 1; /*!< [26..26] Reading 1
                                                             indicates that the
                                                             channel 26 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_27 : 1; /*!< [27..27] Reading 1
                                                             indicates that the
                                                             channel 27 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_28 : 1; /*!< [28..28] Reading 1
                                                             indicates that the
                                                             channel 28 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_29 : 1; /*!< [29..29] Reading 1
                                                             indicates that the
                                                             channel 29 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_30 : 1; /*!< [30..30] Reading 1
                                                             indicates that the
                                                             channel 30 is busy
                                                           */
      __IM unsigned int BUSY_OR_IDEAL_STATUS_CHANNEL_31 : 1; /*!< [31..31] Reading 1
                                                             indicates that the
                                                             channel 31 is busy
                                                           */
    } CHANNEL_STATUS_REG_b;
  };
  __IM unsigned int RESERVED2[8];

  union {
    __IOM unsigned int UDMA_CONFIG_CTRL_REG; /*!< (@ 0x00000828) DMA Controller
                                             Transfer Length Register */

    struct {
      __IOM unsigned int SINGLE_REQUEST_ENABLE : 1; /*!< [0..0] Enabled signal for
                                                    single request */
      __IOM unsigned int RESERVED1 : 31;            /*!< [31..1] Reserved for future use. */
    } UDMA_CONFIG_CTRL_REG_b;
  };

  union {
    __IOM unsigned int UDMA_INTR_MASK_REG; /*!< (@ 0x0000082C) Mask the uDMA
                                           interrupt register */

    struct {
      __IOM unsigned int UDMA_INTR_MASK : 12; /*!< [11..0] Mask the uDMA interrupt
                                              register */
      __IM unsigned int RESERVED1 : 20;       /*!< [31..12] RESERVED1       */
    } UDMA_INTR_MASK_REG_b;
  };
} UDMA0_Type; /*!< Size = 2096 (0x830) */

/* ===========================================================================================================================
  */
/* ================                                          GPDMA_G
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief GPDMA is an AMBA complaint peripheral unit supports 8-channels
  * (GPDMA_G)
  */

typedef struct { /*!< (@ 0x21080000) GPDMA_G Structure */
  __IM unsigned int RESERVED[1057];
  __IOM GPDMA_G_GLOBAL_Type GLOBAL; /*!< (@ 0x00001084) GLOBAL */
} GPDMA_G_Type;                     /*!< Size = 4252 (0x109c)                     */

/* ===========================================================================================================================
  */
/* ================                                          GPDMA_C
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief GPDMAC (dma controller) is an AMBA complaint peripheral unit supports
  * 8-channels (GPDMA_C)
  */

typedef struct {                                       /*!< (@ 0x21081004) GPDMA_C Structure */
  __IOM GPDMA_C_CHANNEL_CONFIG_Type CHANNEL_CONFIG[8]; /*!< (@ 0x00000000) [0..7] */
} GPDMA_C_Type;                                        /*!< Size = 2048 (0x800)          */

/* ===========================================================================================================================
  */
/* ================                                           HWRNG
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief Random numbers generated are 16-bit random numbers and are generated
   using either the True random number generator or the Pseudo random number
   generator. (HWRNG)
   */

typedef struct { /*!< (@ 0x45090000) HWRNG Structure */

  union {
    __IOM unsigned int HWRNG_CTRL_REG; /*!< (@ 0x00000000) Random Number Generator
                                       Control Register                   */

    struct {
      __IOM unsigned int HWRNG_RNG_ST : 1;           /*!< [0..0] This bit is used to start the
                                           true number generation. */
      __IOM unsigned int HWRNG_PRBS_ST : 1;          /*!< [1..1] This bit is used to start the pseudo
                                 random number generation      */
      __IOM unsigned int SOFT_RESET : 1;             /*!< [2..2] This bit is used to start the
                                         pseudo random number generation      */
      __IOM unsigned int TAP_LFSR_INPUT : 1;         /*!< [3..3] This bit is used to Enable bit for 
                                         Tapping LFSR input data which is coming from RING Oscillator      */
      __IM unsigned int LFSR_32_BIT_INPUT_VALID : 1; /*!< [4..4] This bit is used to
                                   Indicates when a valid 32 bit LFSR input data is latched 
                                   After setting this bit, Firmware has to read LFSR_INPUT_LATCH_REG      */
      __IM unsigned int RESERVED1 : 27;              /*!< [31..5] RESERVED1  */

    } HWRNG_CTRL_REG_b;
  };

  union {
    __IM unsigned int HWRNG_RAND_NUM_REG; /*!< (@ 0x00000004) Hardware Random Number
                                          Register                            */

    struct {
      __IM unsigned int HWRNG_RAND_NUM : 32; /*!< [31..0] Generated random number
                                             can be read from this register. */
    } HWRNG_RAND_NUM_REG_b;
    struct {
      __IM unsigned int HWRNG_LFSR_INPUT_LATCH_REG : 32; /*!< [31..0] LFSR Input Latch Register. */
    } HWRNG_LFSR_INPUT_LATCH_REG_REG_b;
  };
} HWRNG_Type; /*!< Size = 8 (0x8) */

/* ===========================================================================================================================
  */
/* ================                                          TIMERS
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief TIMER can be used to generate various timing events for the software
  * (TIMERS)
  */

typedef struct {                              /*!< (@ 0x24042000) TIMERS Structure */
  __IOM TIMERS_MATCH_CTRL_Type MATCH_CTRL[4]; /*!< (@ 0x00000000) [0..3] */
  __IM unsigned int RESERVED[24];

  union {
    __IOM unsigned int MCUULP_TMR_INTR_STAT; /*!< (@ 0x00000080) Timer Status Register */

    struct {
      __IOM unsigned int TMR0_INTR_STATUS : 1; /*!< [0..0] This bit indicates status of the
                                    interrupt generated by timer 0 */
      __IOM unsigned int TMR1_INTR_STATUS : 1; /*!< [1..1] This bit indicates status of the
                                    interrupt generated by timer 1 */
      __IOM unsigned int TMR2_INTR_STATUS : 1; /*!< [2..2] This bit indicates status of the
                                    interrupt generated by timer 2 */
      __IOM unsigned int TMR3_INTR_STATUS : 1; /*!< [3..3] This bit indicates status of the
                                        interrupt generated     by timer 3     */
      __IM unsigned int RESERVED1 : 28;        /*!< [31..4] reserved1 */
    } MCUULP_TMR_INTR_STAT_b;
  };

  union {
    __IOM unsigned int MCUULP_TMR_US_PERIOD_INT; /*!< (@ 0x00000084) Timer micro second period
                                      Integral Part Register           */

    struct {
      __IOM unsigned int TMR_US_PERIOD_INT : 16; /*!< [15..0] This bits are used to program the
                                      integer part of number
                                           of clock cycles per microseconds of
                                      the system clock used                 */
      __IM unsigned int RESERVED1 : 16;          /*!< [31..16] reserved1 */
    } MCUULP_TMR_US_PERIOD_INT_b;
  };

  union {
    __IOM unsigned int MCUULP_TMR_US_PERIOD_FRAC; /*!< (@ 0x00000088) Timer microsecond period
                                       Fractional Part Register          */

    struct {
      __IOM unsigned int TMR_US_PERIOD_FRAC : 8; /*!< [7..0] This bits are used to program the
                                      fractional part of number of clock cycles
                                      per microseconds of the system clock used
                                    */
      __IM unsigned int RESERVED1 : 24;          /*!< [31..8] reserved1 */
    } MCUULP_TMR_US_PERIOD_FRAC_b;
  };

  union {
    __IOM unsigned int MCUULP_TMR_MS_PERIOD_INT; /*!< (@ 0x0000008C) Timer 256 microsecond
                                      period Integral Part Register        */

    struct {
      __IOM unsigned int TMR_MS_PERIOD_INT : 16; /*!< [15..0] This bits are used to program the
                                      integer part of number of clock cycles per
                                      256 microseconds of the system clock used
                                    */
      __IM unsigned int RESERVED1 : 16;          /*!< [31..16] reserved1 */
    } MCUULP_TMR_MS_PERIOD_INT_b;
  };

  union {
    __IOM unsigned int MCUULP_TMR_MS_PERIOD_FRAC; /*!< (@ 0x00000090) Timer 256 microsecond
                                       period Fractional Part Register */

    struct {
      __IOM unsigned int TMR_MS_PERIOD_FRAC : 8; /*!< [7..0] This bits are used to program the
                                      fractional part of number of clock cycles
                                      per 256 microseconds of the system clock
                                      used */
      __IM unsigned int RESERVED1 : 24;          /*!< [31..8] reserved1 */
    } MCUULP_TMR_MS_PERIOD_FRAC_b;
  };
  __IM unsigned int RESERVED1[2];

  union {
    __IM unsigned int MCUULP_TMR_ACTIVE_STATUS; /*!< (@ 0x0000009C) Timer Active
                                                Status Register */

    struct {
      __IM unsigned int TIMER_ACTIVE : 4; /*!< [3..0] Timer active status for each
                                          timer. LSB bit specifies
                                                      the status for 0th timer
                                          and so on. */
      __IM unsigned int RESERVED1 : 28;   /*!< [31..4] reserved1   */
    } MCUULP_TMR_ACTIVE_STATUS_b;
  };
} TIMERS_Type; /*!< Size = 160 (0xa0) */

/* ===========================================================================================================================
  */
/* ================                                            QEI
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief The Quadrature Encoder Interface (QEI) module provides the interface
   to incremental encoders for obtaining mechanical position data (QEI)
   */

typedef struct { /*!< (@ 0x47060000) QEI Structure */

  union {
    __IM unsigned int QEI_STATUS_REG; /*!< (@ 0x00000000) Quadrature Encoder status
                                      register                         */

    struct {
      __IM unsigned int QEI_INDEX : 1;               /*!< [0..0] This is a direct value from the
                                       position signal generator          */
      __IM unsigned int QEI_POSITION_B : 1;          /*!< [1..1] This is a direct value from the
                                  position signal generator.Value refers to the
                                  signal Position_B from the generator. */
      __IM unsigned int QEI_POSITION_A : 1;          /*!< [2..2] This is a direct value from the
                                  position signal generator.Value refers to the
                                  signal Position_A from the generator. */
      __IM unsigned int POSITION_CNTR_ERR : 1;       /*!< [3..3] Count Error Status Flag bit */
      __IM unsigned int POSITION_CNTR_DIRECTION : 1; /*!< [4..4] Position Counter
                                                     Direction Status bit */
      __IM unsigned int RESERVED1 : 27;              /*!< [31..5] Reserved1              */
    } QEI_STATUS_REG_b;
  };

  union {
    __IOM unsigned int QEI_CTRL_REG_SET; /*!< (@ 0x00000004) Quadrature Encoder
                                         control set register */

    struct {
      __IM unsigned int QEI_SFT_RST : 1;                /*!< [0..0] Quadrature encoder soft reset.
                                         It is self reset signal.            */
      __IOM unsigned int QEI_SWAP_PHASE_AB : 1;         /*!< [1..1] Phase A and Phase B
                                                Input Swap Select bit */
      __IOM unsigned int POS_CNT_RST_WITH_INDEX_EN : 1; /*!< [2..2] Phase A and Phase B Input
                                             Swap Select bit */
      __IOM unsigned int RESERVED1 : 1;                 /*!< [3..3] Reserved1      */
      __IOM unsigned int POS_CNT_DIRECTION_CTRL : 1;    /*!< [4..4] NONE */
      __IOM unsigned int POS_CNT_DIR_FRM_REG : 1;       /*!< [5..5] Position Counter Direction
                                        indication from user                    */
      __IOM unsigned int RESERVED2 : 1;                 /*!< [6..6] Reserved2 */
      __IOM unsigned int RESERVED3 : 1;                 /*!< [7..7] Reserved3 */
      __IOM unsigned int INDEX_CNT_RST_EN : 1;          /*!< [8..8] Index count reset enable */
      __IOM unsigned int DIGITAL_FILTER_BYPASS : 1;     /*!< [9..9] NONE */
      __IOM unsigned int TIMER_MODE : 1;                /*!< [10..10] NONE            */
      __IOM unsigned int START_VELOCITY_CNTR : 1;       /*!< [11..11] Starting the velocity counter.
                                       It is self reset bit.             */
      __IOM unsigned int QEI_STOP_IN_IDLE : 1;          /*!< [12..12] NONE */
      __IOM unsigned int QEI_POS_CNT_16_BIT_MODE : 1;   /*!< [13..13] Qei position counter 16 bit
                                           mode enable                         */
      __IOM unsigned int POS_CNT_RST : 1;               /*!< [14..14] 1=position counter is going
                                           to reset                             */
      __IOM unsigned int INDEX_CNT_RST : 1;             /*!< [15..15] 1= index counter is going
                                            to reset. */
      __IOM unsigned int RESERVED4 : 16;                /*!< [31..16] Reserved4    */
    } QEI_CTRL_REG_SET_b;
  };

  union {
    __IOM unsigned int QEI_CTRL_REG_RESET; /*!< (@ 0x00000008) Quadrature Encoder
                                           control reset register */

    struct {
      __IM unsigned int QEI_SFT_RST : 1;                /*!< [0..0] Quadrature encoder soft reset.
                                         It is self reset signal             */
      __IOM unsigned int QEI_SWAP_PHASE_AB : 1;         /*!< [1..1] Phase A and Phase B
                                                Input Swap Select bit */
      __IOM unsigned int POS_CNT_RST_WITH_INDEX_EN : 1; /*!< [2..2] Phase A and Phase B Input
                                             Swap Select bit */
      __IOM unsigned int RESERVED1 : 1;                 /*!< [3..3] Reserved1      */
      __IOM unsigned int POS_CNT_DIRECTION_CTRL : 1;    /*!< [4..4] NONE */
      __IOM unsigned int POS_CNT_DIR_FRM_REG : 1;       /*!< [5..5] Position Counter Direction
                                        indication from user                    */
      __IOM unsigned int RESERVED2 : 1;                 /*!< [6..6] Reserved2 */
      __IOM unsigned int RESERVED3 : 1;                 /*!< [7..7] Reserved3 */
      __IOM unsigned int INDEX_CNT_RST_EN : 1;          /*!< [8..8] NONE      */
      __IOM unsigned int DIGITAL_FILTER_BYPASS : 1;     /*!< [9..9] NONE */
      __IOM unsigned int TIMER_MODE : 1;                /*!< [10..10] NONE            */
      __IOM unsigned int START_VELOCITY_CNTR : 1;       /*!< [11..11] Starting the velocity counter.
                                       It is self reset bit.             */
      __IOM unsigned int QEI_STOP_IN_IDLE : 1;          /*!< [12..12] NONE */
      __IOM unsigned int QEI_POS_CNT_16_BIT_MODE : 1;   /*!< [13..13] Qei position counter 16 bit
                                           mode enable                         */
      __IOM unsigned int POS_CNT_RST : 1;               /*!< [14..14] 1=position counter is going
                                           to reset                             */
      __IOM unsigned int INDEX_CNT_RST : 1;             /*!< [15..15] 1= index counter is going
                                            to reset. */
      __IOM unsigned int RESERVED4 : 16;                /*!< [31..16] Reserved4    */
    } QEI_CTRL_REG_RESET_b;
  };

  union {
    __IOM unsigned int QEI_CNTLR_INIT_REG; /*!< (@ 0x0000000C) Quadrature Encoder
                                           initialization register */

    struct {
      __IOM unsigned int QEI_ENCODING_MODE : 2;       /*!< [1..0] NONE */
      __IOM unsigned int RESERVED1 : 2;               /*!< [3..2] Reserved1         */
      __IOM unsigned int INDEX_MATCH_VALUE : 2;       /*!< [5..4] These bits allow user to specify
                                     the state of position A and B during index
                                     pulse generation. */
      __IOM unsigned int DF_CLK_DIVIDE_SLT : 4;       /*!< [9..6] Digital Filter Clock
                                                Divide Select bits */
      __IOM unsigned int UNIDIRECTIONAL_VELOCITY : 1; /*!< [10..10] Uni directional
                                                      velocity enable. */
      __IOM unsigned int UNIDIRECTIONAL_INDEX : 1;    /*!< [11..11] Uni directional
                                                      index enable.    */
      __IOM unsigned int INDEX_CNT_INIT : 1;          /*!< [12..12] Index counter initial value in
                                  unidirectional index enable mode. */
      __IOM unsigned int RESERVED2 : 19;              /*!< [31..13] Reserved2 */
    } QEI_CNTLR_INIT_REG_b;
  };

  union {
    __IOM unsigned int QEI_INDEX_CNT_REG; /*!< (@ 0x00000010) Quadrature Encoder
                                          index counter register */

    struct {
      __IOM unsigned int QEI_INDEX_CNT : 16;          /*!< [15..0] Index counter value.User
                                             can initialize/change the index
                                             counter using this register */
      __IOM unsigned int QEI_INDEX_CNT_WR_VALUE : 16; /*!< [31..16] User can initialize/change
                                           the index counter using
                                           this register. */
    } QEI_INDEX_CNT_REG_b;
  };

  union {
    __IOM unsigned int QEI_INDEX_MAX_CNT_REG; /*!< (@ 0x00000014) Quadrature Encoder maximum
                                   index counter value register */

    struct {
      __IOM unsigned int QEI_INDEX_MAX_CNT : 16; /*!< [15..0] This is a maximum count value
                                         that is allowed to increment    in the index
                                         counter. If index counter reaches this
                                         value,    will get reset to zero    */
      __IOM unsigned int RESERVED1 : 16;         /*!< [31..16] Reserved1 */
    } QEI_INDEX_MAX_CNT_REG_b;
  };

  union {
    __IOM unsigned int QEI_POSITION_CNT_REG; /*!< (@ 0x00000018) Quadrature Encoder maximum
                                  position counter value register */

    struct {
      __IOM unsigned int QEI_POSITION_CNT_WR_VALUE_L : 16; /*!< [15..0] This is a maximum count
                                                value that is allowed to
                                                increment in the position
                                                counter. */
      __IOM unsigned int QEI_POSITION_CNT_WR_VALUE_H : 16; /*!< [31..16] This is a maximum
                                                count value that is allowed to
                                                increment in the position
                                                counter. */
    } QEI_POSITION_CNT_REG_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int QEI_POSITION_MAX_CNT_LSW_REG; /*!< (@ 0x00000020) Quadrature
                                                     Encoder maximum position
                                                     counter value register */

    struct {
      __IOM unsigned int QEI_POSITION_MAX_CNT_L : 16; /*!< [15..0] This is a maximum
                                                      count value that is allowed
                                                      to increment in the
                                                      position counter. */
      __IOM unsigned int QEI_POSITION_MAX_CNT_H : 16; /*!< [31..16] This is a maximum count
                                           value that is allowed to increment
                                                        in the position counter.
                                         */
    } QEI_POSITION_MAX_CNT_LSW_REG_b;
  };
  __IM unsigned int RESERVED1;

  union {
    __IM unsigned int QEI_INTR_STS_REG; /*!< (@ 0x00000028) Quadrature Encoder
                                        interrupt status register */

    struct {
      __IM unsigned int QEI_POSITION_CNT_RESET_INTR_LEV : 1;        /*!< [0..0] This is raised when
                                                   the position counter reaches
                                                   it's extremes */
      __IM unsigned int QEI_INDEX_CNT_MATCH_INTR_LEV : 1;           /*!< [1..1] This is raised when
                                                index counter reaches max value
                                                loaded in to index_max_cnt
                                                register. */
      __IM unsigned int POSITION_CNTR_ERR_INTR_LEV : 1;             /*!< [2..2] Whenever number of
                                            possible positions are mismatched
                                            with actual positions are received
                                            between two index pulses this will
                                            raised */
      __IM unsigned int VELOCITY_LESS_THAN_INTR_LEV : 1;            /*!< [3..3] When velocity count is
                                               less than the value given in
                                               velocity_value_to_c mpare
                                               register, interrupt is raised */
      __IM unsigned int QEI_POSITION_CNT_MATCH_INTR_LEV : 1;        /*!< [4..4] This is raised when
                                                   the position counter reaches
                                                   position
                                                        match value, which is
                                                   programmable. */
      __IM unsigned int QEI_VELOCITY_COMPUTATION_OVER_INTR_LEV : 1; /*!< [5..5] When velocity
                                                        count is computed for
                                                        given delta time, than
                                                        interrupt is raised. */
      __IM unsigned int RESERVED1 : 26;                             /*!< [31..6] Reserved1                   */
    } QEI_INTR_STS_REG_b;
  };

  union {
    __IOM unsigned int QEI_INTR_ACK_REG; /*!< (@ 0x0000002C) Quadrature Encoder
                                         interrupt acknowledge register */

    struct {
      __IOM unsigned int QEI_POSITION_CNT_RESET_INTR_LEV : 1;    /*!< [0..0]
                                                              Qei_position_cnt_reset_intr_ack
                                                            */
      __IOM unsigned int QEI_INDEX_CNT_MATCH_INTR_LEV : 1;       /*!< [1..1] NONE    */
      __IOM unsigned int POSITION_CNTR_ERR_INTR_LEV : 1;         /*!< [2..2] Position_cntr_err_intr_ack
                                            */
      __IOM unsigned int VELOCITY_LESS_THAN_INTR_LEV : 1;        /*!< [3..3]
                                                                 Velocity_less_than_intr_ack                   */
      __IOM unsigned int QEI_POSITION_CNT_MATCH_INTR_LEV : 1;    /*!< [4..4]
                                                                 Qei_position_cnt_match_intr_ack
                                                               */
      __IOM unsigned int VELOCITY_COMPUTATION_OVER_INTR_LEV : 1; /*!< [5..5]
                                                                 Velocity_computation_over_intr_ack
                                                               */
      __IOM unsigned int RESERVED1 : 26;                         /*!< [31..6] Reserved1 */
    } QEI_INTR_ACK_REG_b;
  };

  union {
    __IOM unsigned int QEI_INTR_MASK_REG; /*!< (@ 0x00000030) Quadrature Encoder
                                          interrupt mask register */

    struct {
      __IOM unsigned int QEI_POSITION_CNT_RESET_INTR_MASK : 1;    /*!< [0..0]
                                                               Qei_position_cnt_reset_intr_mask
                                                             */
      __IOM unsigned int QEI_INDEX_CNT_MATCH_INTR_MASK : 1;       /*!< [1..1]
                                                               Qei_index_cnt_match_intr_mask
                                                             */
      __IOM unsigned int POSITION_CNTR_ERR_INTR_MASK : 1;         /*!< [2..2]
                                               Position_cntr_err_intr_mask */
      __IOM unsigned int VELOCITY_LESS_THAN_INTR_MASK : 1;        /*!< [3..3]
                                                                  Velocity_less_than_intr_mask                   */
      __IOM unsigned int QEI_POSITION_CNT_MATCH_INTR_MASK : 1;    /*!< [4..4]
                                                                  Qei_position_cnt_match_intr_mask
                                                                */
      __IOM unsigned int VELOCITY_COMPUTATION_OVER_INTR_MASK : 1; /*!< [5..5]
                                                                  Velocity_computation_over_intr_mask
                                                                */
      __IOM unsigned int RESERVED1 : 26;                          /*!< [31..6] Reserved1 */
    } QEI_INTR_MASK_REG_b;
  };

  union {
    __IOM unsigned int QEI_INTR_UNMASK_REg; /*!< (@ 0x00000034) Quadrature Encoder
                                            interrupt unmask register */

    struct {
      __IOM unsigned int QEI_POSITION_CNT_RESET_INTR_UNMASK : 1; /*!< [0..0]
                                                                 Qei_position_cnt_reset_intr_unmask
                                                               */
      __IOM unsigned int QEI_INDEX_CNT_MATCH_INTR_UNMASK : 1;    /*!< [1..1]
                                                                 Qei_index_cnt_match_intr_unmask
                                                               */
      __IOM unsigned int POSITION_CNTR_ERR_INTR_UNMASK : 1;      /*!< [2..2]
                                                                 Position_cntr_err_intr_unmask
                                                               */
      __IOM unsigned int VELOCITY_LESS_THAN_INTR_UNMASK : 1;     /*!< [3..3]
                                                                 Velocity_less_than_intr_unmask
                                                               */
      __IOM unsigned int QEI_POSITION_CNT_MATCH_INTR_UNMASK : 1; /*!< [4..4]
                                                                 Qei_position_cnt_match_intr_unmask
                                                               */
      __IOM unsigned int RESERVED1 : 27;                         /*!< [31..5] Reserved1 */
    } QEI_INTR_UNMASK_REg_b;
  };

  union {
    __IOM unsigned int QEI_CLK_FREQ_REG; /*!< (@ 0x00000038) Quadrature Encoder
                                         clock frequency register */

    struct {
      __IOM unsigned int QEI_CLK_FREQ : 9; /*!< [8..0] Indication of clock frequency on which
                                QEI controller is running. */
      __IOM unsigned int RESERVED1 : 23;   /*!< [31..9] Reserved1 */
    } QEI_CLK_FREQ_REG_b;
  };

  union {
    __IOM unsigned int QEI_DELTA_TIME_REG; /*!< (@ 0x0000003C) Quadrature Delta time
                                           register */

    struct {
      __IOM unsigned int DELTA_TIME_FOR_VELOCITY : 20; /*!< [19..0] Delta time LSW
                                                       to compute velocity */
      __IOM unsigned int RESERVED1 : 12;               /*!< [31..20] Reserved1               */
    } QEI_DELTA_TIME_REG_b;
  };
  __IM unsigned int RESERVED2;

  union {
    __IOM unsigned int QEI_VELOCITY_REG; /*!< (@ 0x00000044) Quadrature velocity register */

    struct {
      __IOM unsigned int VELOCITY_VALUE_TO_COMPARE_L : 16; /*!< [15..0] For read operation :It
                                                is the velocity count to compare
                                                        using NWP firmware For
                                                write operation :It is the
                                                velocity value to compare with
                                                velocity count */
      __IOM unsigned int VELOCITY_VALUE_TO_COMPARE_H : 16; /*!< [31..16] For read operation :It
                                                is the velocity count to compare
                                                        using NWP firmware For
                                                write operation :It is the
                                                velocity value to compare with
                                                velocity count */
    } QEI_VELOCITY_REG_b;
  };
  __IM unsigned int RESERVED3;

  union {
    __IOM unsigned int QEI_POSITION_MATCH_REG; /*!< (@ 0x0000004C) Quadrature
                                               position match register */

    struct {
      __IOM unsigned int POSTION_MATCH_VALUE_L : 16; /*!< [15..0] Position match value to
                                          compare the position counter. */
      __IOM unsigned int POSTION_MATCH_VALUE_H : 16; /*!< [31..16] Position match value to
                                          compare the position counter. */
    } QEI_POSITION_MATCH_REG_b;
  };
} QEI_Type; /*!< Size = 80 (0x50) */

/* ===========================================================================================================================
  */
/* ================                                          USART0
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief Universal Asynchronous Receiver/Transmitter is for serial
   communication with peripherals, modems and datasets (USART0)
   */

typedef struct { /*!< (@ 0x44000100) USART0 Structure */

  union {
    union {
      __IOM unsigned int DLL; /*!< (@ 0x00000000) Divisor Latch Low */

      struct {
        __IOM unsigned int DLL : 8;       /*!< [7..0] Lower 8-bits of a 16-bit, read/write, Divisor
                         Latch register that contains the baud rate divisor for
                         the UART.                         */
        __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
      } DLL_b;
    };

    union {
      __OM unsigned int THR; /*!< (@ 0x00000000) Transmit Holding Register */

      struct {
        __OM unsigned int THR_LSB_8_BITS : 8; /*!< [7..0] Data to be transmitted on serial
                                   output port                       */
        __OM unsigned int
          THR_MSB_9TH_BIT : 1; /*!< [8..8] Data to be transmitted on the serial output port (sout) in UART mode for the MSB
                                    9th bit */
        __OM unsigned int RESERVED1 : 23; /*!< [31..9] reserved1 */
      } THR_b;
    };

    union {
      __IM unsigned int RBR; /*!< (@ 0x00000000) Receive Buffer Register */

      struct {
        __IM unsigned int RBR_LSB_8_BITS : 8; /*!< [7..0] Receive Buffer Field        */
        __IM unsigned int
          RBR_MSB_9TH_BIT : 1; /*!< [8..8] Data byte received on the serial input port (sin) in UART mode for the MSB
                                    9th bit */
        __IM unsigned int RESERVED1 : 23; /*!< [31..9] reserved1 */
      } RBR_b;
    };
  };

  union {
    union {
      __IOM unsigned int IER; /*!< (@ 0x00000004) Interrupt Enable Register */

      struct {
        __IOM unsigned int ERBFI : 1;     /*!< [0..0] Enable Received Data Available Interrupt */
        __IOM unsigned int ETBEI : 1;     /*!< [1..1] Enable Transmit Holding Register
                                      Empty Interrupt                   */
        __IOM unsigned int ELSI : 1;      /*!< [2..2] Enable Receiver Line Status Interrupt */
        __IOM unsigned int EDSSI : 1;     /*!< [3..3] Enable Modem Status Interrupt */
        __IM unsigned int RESERVED1 : 3;  /*!< [6..4] reserved1 */
        __IOM unsigned int PTIME : 1;     /*!< [7..7] Programmable THRE Interrupt Mode Enable */
        __IM unsigned int RESERVED2 : 24; /*!< [31..8] reserved2 */
      } IER_b;
    };

    union {
      __IOM unsigned int DLH; /*!< (@ 0x00000004) Divisor Latch High */

      struct {
        __IOM unsigned int DLH : 8;       /*!< [7..0] Upper 8-bits of a 16-bit, read/write, Divisor
                         Latch register that contains the baud rate divisor for
                         the UART                          */
        __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
      } DLH_b;
    };
  };

  union {
    union {
      __OM unsigned int FCR; /*!< (@ 0x00000008) FIFO Control Register */

      struct {
        __OM unsigned int FIFOE : 1;      /*!< [0..0] This enables/disables the transmit
                                      (XMIT) and receive  (RCVR) FIFOs  */
        __OM unsigned int RFIFOR : 1;     /*!< [1..1] RCVR FIFO Reset */
        __OM unsigned int XFIFOR : 1;     /*!< [2..2] XMIT FIFO Reset */
        __OM unsigned int DMAM : 1;       /*!< [3..3] DMA signalling mode   */
        __OM unsigned int TET : 2;        /*!< [5..4] TX Empty Trigger    */
        __OM unsigned int RT : 2;         /*!< [7..6] This is used to select the trigger level in the
                        receiver FIFO at which the Received Data Available
                        Interrupt is generated */
        __OM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
      } FCR_b;
    };

    union {
      __IM unsigned int IIR; /*!< (@ 0x00000008) Interrupt Identity Register */

      struct {
        __IM unsigned int IID : 4;        /*!< [3..0] Interrupt ID       */
        __IM unsigned int RESERVED1 : 2;  /*!< [5..4] reserved1 */
        __IM unsigned int FIFOSE : 2;     /*!< [7..6] This is used to indicate whether
                                      the FIFOs are enabled or disabled. */
        __IM unsigned int RESERVED2 : 24; /*!< [31..8] reserved2 */
      } IIR_b;
    };
  };

  union {
    __IOM unsigned int LCR; /*!< (@ 0x0000000C) Line Control Register */

    struct {
      __IOM unsigned int DLS : 2;          /*!< [1..0] Data Length Select,This is used to
                                   select the number  of data bits per character
                                   that the peripheral transmits  and receives  */
      __IOM unsigned int STOP : 1;         /*!< [2..2] This is used to select the number of
                                   stop bits per character that the peripheral
                                   transmits and receives */
      __IOM unsigned int PEN : 1;          /*!< [3..3] This bit is used to enable and disable parity
                       generation and detection in transmitted and received
                       serial character                */
      __IOM unsigned int EPS : 1;          /*!< [4..4] This is used to select between even
                                  and odd parity                 */
      __IOM unsigned int STICK_PARITY : 1; /*!< [5..5] This bit is used to force
                                           parity value */
      __IOM unsigned int BC : 1;           /*!< [6..6] This is used to cause a break condition
                                 to be transmitted to the receiving device */
      __IOM unsigned int DLAB : 1;         /*!< [7..7] This bit is used to enable reading
                                   and writing of the Divisor Latch register to
                                   set the baud rate of the UART */
      __IOM unsigned int RESERVED1 : 24;   /*!< [31..8] reserved1 */
    } LCR_b;
  };

  union {
    __IOM unsigned int MCR; /*!< (@ 0x00000010) Modem Control Register */

    struct {
      __IOM unsigned int DTR : 1;       /*!< [0..0] This is used to directly control the
                                   Data Terminal Ready  (dtr_n) output  */
      __IOM unsigned int RTS : 1;       /*!< [1..1] This is used to directly control the
                                   Request to Send  (rts_n) output  */
      __IOM unsigned int OUT1 : 1;      /*!< [2..2] This is used to directly control the
                                   user-designated Output1 (out1_n) output */
      __IOM unsigned int OUT2 : 1;      /*!< [3..3] This is used to directly control the
                                   user-designated Output2 (out2_n) output */
      __IOM unsigned int LB : 1;        /*!< [4..4] This is used to put the UART into a
                                   diagnostic mode for   test purposes   */
      __IOM unsigned int AFCE : 1;      /*!< [5..5] This is used to directly control the
                                   user-designated Output2 (out2_n) output */
      __IOM unsigned int SIRE : 1;      /*!< [6..6] This is used to enable/disable the
                                   IrDA SIR Mode features          */
      __IM unsigned int RESERVED1 : 25; /*!< [31..7] reserved1 */
    } MCR_b;
  };

  union {
    __IM unsigned int LSR; /*!< (@ 0x00000014) Line Status Register */

    struct {
      __IM unsigned int DR : 1;         /*!< [0..0] This is used to indicate that the
                                receiver contains at least one character in the
                                RBR or the receiver FIFO                       */
      __IM unsigned int OE : 1;         /*!< [1..1] This is used to indicate the occurrence
                                of an overrun error */
      __IM unsigned int PE : 1;         /*!< [2..2] This is used to indicate the occurrence
                                of a parity error in the receiver  if the Parity
                                Enable (PEN) bit (LCR[3]) is set */
      __IM unsigned int FE : 1;         /*!< [3..3] This is used to indicate the occurrence
                                of a framing error in the receiver */
      __IM unsigned int BI : 1;         /*!< [4..4] his is used to indicate the detection of
                                a break sequence on the serial input data */
      __IM unsigned int THRE : 1;       /*!< [5..5] Transmit Holding Register Empty bit           */
      __IM unsigned int TEMT : 1;       /*!< [6..6] Transmitter Empty bit */
      __IM unsigned int RFE : 1;        /*!< [7..7] This is used to indicate if there is at
                                 least one parity error,framing error, or break
                                 indication in the FIFO                      */
      __IM unsigned int ADDR_RCVD : 1;  /*!< [8..8] This is used to indicate the ADDR Received  */
      __IM unsigned int RESERVED1 : 23; /*!< [31..9] reserved1 */
    } LSR_b;
  };

  union {
    __IM unsigned int MSR; /*!< (@ 0x00000018) Modem Status Register */

    struct {
      __IM unsigned int DCTS : 1;       /*!< [0..0] This is used to indicate that the modem control
                        line cts_n has changed since the last time the MSR was
                        read                    */
      __IM unsigned int DDSR : 1;       /*!< [1..1] This is used to indicate that the modem control
                        line dsr_n has changed since the last time the MSR was
                        read                    */
      __IM unsigned int TERI : 1;       /*!< [2..2] This is used to indicate that a change on the
                        input ri_n(from an active-low to an inactive-high state)
                        has occurred since the last time the MSR was read */
      __IM unsigned int DDCD : 1;       /*!< [3..3] This is used to indicate that the modem control
                        line dcd_n has  changed since the last time the MSR was
                        read                   */
      __IM unsigned int CTS : 1;        /*!< [4..4] This is used to indicate the current
                                 state of the modem control line cts_n */
      __IM unsigned int DSR : 1;        /*!< [5..5] This is used to indicate the current
                                 state of the modem control line dsr_n */
      __IM unsigned int RI : 1;         /*!< [6..6] This is used to indicate the current
                                 state of the modem  control line ri_n  */
      __IM unsigned int DCD : 1;        /*!< [7..7] This is used to indicate the current
                                 state of the modem control line dcd_n */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
    } MSR_b;
  };

  union {
    __IOM unsigned int SCR; /*!< (@ 0x0000001C) Scratch pad Register */

    struct {
      __IOM unsigned int SCRATCH_PAD : 8; /*!< [7..0] This register is for programmers to use
                               as a temporary storage space. It has no defined
                               purpose                                  */
      __IM unsigned int RESERVED1 : 24;   /*!< [31..8] reserved1 */
    } SCR_b;
  };

  union {
    __IOM unsigned int LPDLL; /*!< (@ 0x00000020) Low Power Divisor Latch Low Register */

    struct {
      __IOM unsigned int LOW_POWER_DLL : 8; /*!< [7..0] This register makes up the lower 8-bits
                                 of a 16-bit, read/write, Low Power Divisor Latch
                                 register that contains the baud rate divisor for
                                 the UART, which must give a baud
                                           rate of 115.2K */
      __IOM unsigned int RESERVED1 : 24;    /*!< [31..8] reserved1 */
    } LPDLL_b;
  };

  union {
    __IOM unsigned int LPDLH; /*!< (@ 0x00000024) Low Power Divisor Latch High Register */

    struct {
      __IOM unsigned int LOW_POWER_DLH : 8; /*!< [7..0] This register makes up the upper 8-bits
                                 of a 16-bit, read/write, Low Power Divisor Latch
                                 register that contains the baud rate divisor for
                                 the UART, which must give a baud
                                           rate of 115200 */
      __IOM unsigned int RESERVED1 : 24;    /*!< [31..8] reserved1 */
    } LPDLH_b;
  };
  __IM unsigned int RESERVED[6];

  union {
    __IOM unsigned int HDEN; /*!< (@ 0x00000040) none */

    struct {
      __IOM unsigned int FULL_DUPLEX_MODE : 1; /*!< [0..0] none */
      __IOM unsigned int TX_MODE_RX_MODE : 1;  /*!< [1..1] This signal is valid when
                                               full_duplex_mode is disabled  */
      __IM unsigned int RESERVED1 : 30;        /*!< [31..2] reserved1        */
    } HDEN_b;
  };
  __IM unsigned int RESERVED1[5];

  union {
    __IOM unsigned int SMCR; /*!< (@ 0x00000058) none */

    struct {
      __IOM unsigned int SYNC_MODE : 1;      /*!< [0..0] none      */
      __IOM unsigned int MST_MODE : 1;       /*!< [1..1] none       */
      __IOM unsigned int RESERVED1 : 2;      /*!< [3..2] reserved1      */
      __IOM unsigned int CONTI_CLK_MODE : 1; /*!< [4..4] none */
      __IOM unsigned int START_STOP_EN : 1;  /*!< [5..5] none  */
      __IOM unsigned int RESERVED2 : 26;     /*!< [31..6] reserved2     */
    } SMCR_b;
  };
  __IM unsigned int RESERVED2[5];

  union {
    __IOM unsigned int FAR; /*!< (@ 0x00000070) none */

    struct {
      __IOM unsigned int SYNC_MODE : 1; /*!< [0..0] none */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } FAR_b;
  };

  union {
    __IM unsigned int TFR; /*!< (@ 0x00000074) none */

    struct {
      __IM unsigned int TX_FIFO_RD : 8; /*!< [7..0] Transmit FIFO Read */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
    } TFR_b;
  };

  union {
    __IOM unsigned int RFW; /*!< (@ 0x00000078) none */

    struct {
      __IOM unsigned int RFWD : 8;      /*!< [7..0] Receive FIFO Write Data      */
      __IOM unsigned int RFPE : 1;      /*!< [8..8] Receive FIFO Parity Error      */
      __IOM unsigned int RFFE : 1;      /*!< [9..9] Receive FIFO Framing Error      */
      __IM unsigned int RESERVED1 : 22; /*!< [31..10] reserved1 */
    } RFW_b;
  };

  union {
    __IM unsigned int USR; /*!< (@ 0x0000007C) UART Status Register */

    struct {
      __IM unsigned int BUSY : 1;       /*!< [0..0] Indicates that a serial transfer is in
                                  progress                    */
      __IM unsigned int TFNF : 1;       /*!< [1..1] To Indicate that the transmit FIFO is
                                  not full                     */
      __IM unsigned int TFE : 1;        /*!< [2..2] To Indicate that the transmit FIFO is
                                  completely empty             */
      __IM unsigned int RFNE : 1;       /*!< [3..3] To Indicate that the receive FIFO
                                  contains one or more entries */
      __IM unsigned int RFE : 1;        /*!< [4..4] To Indicate that the receive FIFO is
                                  completely full               */
      __IM unsigned int RESERVED1 : 27; /*!< [31..5] reserved1 */
    } USR_b;
  };

  union {
    __IM unsigned int TFL; /*!< (@ 0x00000080) Transmit FIFO Level */

    struct {
      __IM unsigned int FIFO_ADDR_WIDTH : 30; /*!< [29..0] Transmit FIFO Level. This
                                              is indicates the number of data
                                              entries in the transmit FIFO. */
      __IM unsigned int RESERVED1 : 2;        /*!< [31..30] reserved1        */
    } TFL_b;
  };

  union {
    __IM unsigned int RFL; /*!< (@ 0x00000084) Receive FIFO Level */

    struct {
      __IM unsigned int FIFO_ADDR_WIDTH : 30; /*!< [29..0] Receive FIFO Level. This
                                              is indicates the number of data
                                              entries in the receive FIFO. */
      __IM unsigned int RESERVED1 : 2;        /*!< [31..30] reserved1        */
    } RFL_b;
  };

  union {
    __OM unsigned int SRR; /*!< (@ 0x00000088) Software Reset Register */

    struct {
      __OM unsigned int UR : 1;         /*!< [0..0] UART Reset         */
      __OM unsigned int RFR : 1;        /*!< [1..1] RCVR FIFO Reset        */
      __OM unsigned int XFR : 1;        /*!< [2..2] XMIT FIFO Reset        */
      __OM unsigned int RESERVED1 : 29; /*!< [31..3] reserved1 */
    } SRR_b;
  };

  union {
    __IOM unsigned int SRTS; /*!< (@ 0x0000008C) Shadow Request to Send */

    struct {
      __IOM unsigned int SRTS : 1;      /*!< [0..0] Shadow Request to Send.      */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } SRTS_b;
  };

  union {
    __IOM unsigned int SBCR; /*!< (@ 0x00000090) Shadow Break Control Register */

    struct {
      __IOM unsigned int SBCR : 1;      /*!< [0..0] Shadow Break Control Bit      */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } SBCR_b;
  };

  union {
    __IOM unsigned int SDMAM; /*!< (@ 0x00000094) Shadow DMA Mode */

    struct {
      __IOM unsigned int SDMAM : 1;     /*!< [0..0] Shadow DMA Mode     */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } SDMAM_b;
  };

  union {
    __IOM unsigned int SFE; /*!< (@ 0x00000098) Shadow FIFO Enable */

    struct {
      __IOM unsigned int SFE : 1;       /*!< [0..0] Shadow FIFO Enable       */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } SFE_b;
  };

  union {
    __IOM unsigned int SRT; /*!< (@ 0x0000009C) Shadow RCVR Trigger */

    struct {
      __IOM unsigned int SRT : 2;       /*!< [1..0] Shadow RCVR Trigger       */
      __IM unsigned int RESERVED1 : 30; /*!< [31..2] reserved1 */
    } SRT_b;
  };

  union {
    __IOM unsigned int STET; /*!< (@ 0x000000A0) Shadow TX Empty Trigger */

    struct {
      __IOM unsigned int STET : 2;      /*!< [1..0] Shadow TX Empty Trigger      */
      __IM unsigned int RESERVED1 : 30; /*!< [31..2] reserved1 */
    } STET_b;
  };

  union {
    __IOM unsigned int HTX; /*!< (@ 0x000000A4) Halt Transmit */

    struct {
      __IOM unsigned int HALT_TX : 1;   /*!< [0..0] This register is use to halt
                                        transmissions for testing             */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } HTX_b;
  };

  union {
    __IOM unsigned int DMASA; /*!< (@ 0x000000A8) DMA Software Acknowledge */

    struct {
      __OM unsigned int DMA_SOFTWARE_ACK : 1; /*!< [0..0] This register is use to perform a
                                    DMA software acknowledge
                                            if a transfer needs to be terminated
                                    due to an error condition            */
      __IM unsigned int RESERVED1 : 31;       /*!< [31..1] reserved1 */
    } DMASA_b;
  };

  union {
    __IOM unsigned int TCR; /*!< (@ 0x000000AC) Transceiver Control Register. */

    struct {
      __IOM unsigned int RS485_EN : 1;  /*!< [0..0] RS485 Transfer Enable.  */
      __IOM unsigned int RE_POL : 1;    /*!< [1..1] Receiver Enable Polarity.    */
      __IOM unsigned int DE_POL : 1;    /*!< [2..2] Driver Enable Polarity.    */
      __IOM unsigned int XFER_MODE : 2; /*!< [4..3] Transfer Mode. */
      __IM unsigned int RESERVED1 : 27; /*!< [31..5] reserved1 */
    } TCR_b;
  };

  union {
    __IOM unsigned int DE_EN; /*!< (@ 0x000000B0) Driver Output Enable Register. */

    struct {
      __IOM unsigned int DE_EN : 1;     /*!< [0..0] DE Enable control.     */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } DE_EN_b;
  };

  union {
    __IOM unsigned int RE_EN; /*!< (@ 0x000000B4) Receiver Output Enable Register. */

    struct {
      __IOM unsigned int RE_EN : 1;     /*!< [0..0] RE Enable control.     */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] reserved1 */
    } RE_EN_b;
  };

  union {
    __IOM unsigned int DET; /*!< (@ 0x000000B8) Driver Output Enable Timing Register. */

    struct {
      __IOM unsigned int DE_ASSERT_TIME : 8;    /*!< [7..0] Driver enable assertion time. */
      __IOM unsigned int RES : 8;               /*!< [15..8] reserved. */
      __IOM unsigned int DE_DE_ASSERT_TIME : 8; /*!< [23..16] Driver enable
                                                de-assertion time. */
      __IM unsigned int RESERVED1 : 8;          /*!< [31..24] reserved1          */
    } DET_b;
  };

  union {
    __IOM unsigned int TAT; /*!< (@ 0x000000BC) TurnAround Timing Register */

    struct {
      __IOM unsigned int DE_RE : 16; /*!< [15..0] Driver Enable to Receiver Enable
                                     TurnAround time.                 */
      __IOM unsigned int RE_DE : 16; /*!< [31..16] Receiver Enable to Driver Enable
                                     TurnAround time.                */
    } TAT_b;
  };

  union {
    __IOM unsigned int DLF; /*!< (@ 0x000000C0) Divisor Latch Fraction Register. */

    struct {
      __IOM unsigned int DLF : 6; /*!< [5..0] Fractional part of divisor. */
      __IM unsigned int : 1;
      __IM unsigned int RESERVED1 : 25; /*!< [31..7] reserved1 */
    } DLF_b;
  };

  union {
    __IOM unsigned int RAR; /*!< (@ 0x000000C4) Receive Address Register. */

    struct {
      __IOM unsigned int RAR : 8;       /*!< [7..0] This is an address matching register
                                  during receive mode.          */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
    } RAR_b;
  };

  union {
    __IOM unsigned int TAR; /*!< (@ 0x000000C8) Transmit Address Register. */

    struct {
      __IOM unsigned int TAR : 8;       /*!< [7..0] This is an address matching register
                                  during transmit mode. */
      __IM unsigned int RESERVED1 : 24; /*!< [31..8] reserved1 */
    } TAR_b;
  };

  union {
    __IOM unsigned int LCR_EXT; /*!< (@ 0x000000CC) Line Extended Control Register */

    struct {
      __IOM unsigned int DLS_E : 1;         /*!< [0..0] Extension for DLS.      */
      __IOM unsigned int ADDR_MATCH : 1;    /*!< [1..1] Address Match Mode. */
      __IOM unsigned int SEND_ADDR : 1;     /*!< [2..2] Send address control bit.  */
      __IOM unsigned int TRANSMIT_MODE : 1; /*!< [3..3] Transmit mode control bit.        */
      __IM unsigned int RESERVED1 : 28;     /*!< [31..4] reserved1 */
    } LCR_EXT_b;
  };
  __IM unsigned int RESERVED3[9];

  union {
    __IM unsigned int CPR; /*!< (@ 0x000000F4) Component Parameter Register */

    struct {
      __IM unsigned int APB_DATA_WIDTH : 2;          /*!< [1..0] APB data width register.            */
      __IM unsigned int RESERVED1 : 2;               /*!< [3..2] reserved1       */
      __IM unsigned int AFCE_MODE : 1;               /*!< [4..4] none       */
      __IM unsigned int THRE_MODE : 1;               /*!< [5..5] none       */
      __IM unsigned int SIR_MODE : 1;                /*!< [6..6] none        */
      __IM unsigned int SIR_LP_MODE : 1;             /*!< [7..7] none     */
      __IM unsigned int ADDITIONAL_FEAT : 1;         /*!< [8..8] none */
      __IM unsigned int FIFO_ACCESS : 1;             /*!< [9..9] none     */
      __IM unsigned int FIFO_STAT : 1;               /*!< [10..10] none       */
      __IM unsigned int SHADOW : 1;                  /*!< [11..11] none          */
      __IM unsigned int UART_ADD_ENCODED_PARAMS : 1; /*!< [12..12] none */
      __IM unsigned int DMA_EXTRA : 1;               /*!< [13..13] none               */
      __IM unsigned int RESERVED2 : 2;               /*!< [15..14] reserved2               */
      __IM unsigned int FIFO_MODE : 8;               /*!< [23..16] none               */
      __IM unsigned int RESERVED3 : 8;               /*!< [31..24] reserved3               */
    } CPR_b;
  };

  union {
    __IM unsigned int UCV; /*!< (@ 0x000000F8) UART Component Version */

    struct {
      __IM unsigned int UART_COMP_VER : 32; /*!< [31..0] ASCII value for each number
                                            in the version, followed by * */
    } UCV_b;
  };

  union {
    __IM unsigned int CTR; /*!< (@ 0x000000FC) Component Type Register */

    struct {
      __IM unsigned int UART_COMP_VER : 32; /*!< [31..0] This register contains the
                                            peripherals identification code. */
    } CTR_b;
  };
} USART0_Type; /*!< Size = 256 (0x100) */

/* ===========================================================================================================================
  */
/* ================                                           GSPI0
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief GSPI, or Generic SPI, is a module which has been derived from QSPI.
  * GSPI can act only as a master (GSPI0)
  */

typedef struct { /*!< (@ 0x45030000) GSPI0 Structure */

  union {
    __IOM unsigned int GSPI_CLK_CONFIG; /*!< (@ 0x00000000) GSPI Clock Configuration
                                        Register                          */

    struct {
      __IOM unsigned int GSPI_CLK_SYNC : 1; /*!< [0..0] If the clock frequency to FLASH
                                 (spi_clk) and SOC clk is same. */
      __IOM unsigned int GSPI_CLK_EN : 1;   /*!< [1..1] GSPI clock enable */
      __IOM unsigned int RESERVED1 : 30;    /*!< [31..2] reserved for future use  */
    } GSPI_CLK_CONFIG_b;
  };

  union {
    __IOM unsigned int GSPI_BUS_MODE; /*!< (@ 0x00000004) GSPI Bus Mode Register */

    struct {
      __IOM unsigned int GSPI_DATA_SAMPLE_EDGE : 1;   /*!< [0..0] Samples MISO data on
                                                    clock edges. This should be
                                                    ZERO for mode3 clock */
      __IOM unsigned int GSPI_CLK_MODE_CSN0 : 1;      /*!< [1..1] NONE    */
      __IOM unsigned int GSPI_CLK_MODE_CSN1 : 1;      /*!< [2..2] NONE    */
      __IOM unsigned int GSPI_CLK_MODE_CSN2 : 1;      /*!< [3..3] NONE    */
      __IOM unsigned int GSPI_CLK_MODE_CSN3 : 1;      /*!< [4..4] NONE    */
      __IOM unsigned int GSPI_GPIO_MODE_ENABLES : 6;  /*!< [10..5] These bits are used to map
                                          GSPI on GPIO pins */
      __IOM unsigned int SPI_HIGH_PERFORMANCE_EN : 1; /*!< [11..11] High performance
                                                      features are enabled when
                                                      this bit is set to one */
      __IOM unsigned int RESERVED1 : 20;              /*!< [31..12] reserved for future use */
    } GSPI_BUS_MODE_b;
  };
  __IM unsigned int RESERVED[2];

  union {
    __IOM unsigned int GSPI_CONFIG1; /*!< (@ 0x00000010) GSPI Configuration 1 Register */

    struct {
      __IOM unsigned int GSPI_MANUAL_CSN : 1;        /*!< [0..0] SPI CS in manual mode */
      __IOM unsigned int GSPI_MANUAL_WR : 1;         /*!< [1..1] Write enable for manual
                                              mode when CS is low.  */
      __IOM unsigned int GSPI_MANUAL_RD : 1;         /*!< [2..2] Read enable for manual mode
                                             when CS is low */
      __IOM unsigned int GSPI_MANUAL_RD_CNT : 10;    /*!< [12..3] Indicates total
                                                  number of bytes to be read */
      __IOM unsigned int GSPI_MANUAL_CSN_SELECT : 2; /*!< [14..13] Indicates which CSn is
                                          valid. Can be programmable in manual
                                          mode */
      __IOM unsigned int SPI_FULL_DUPLEX_EN : 1;     /*!< [15..15] Full duplex mode enable    */
      __IOM unsigned int RESERVED1 : 16;             /*!< [31..16] reserved for future use */
    } GSPI_CONFIG1_b;
  };

  union {
    __IOM unsigned int GSPI_CONFIG2; /*!< (@ 0x00000014) GSPI Manual Configuration 2
                                     Register                       */

    struct {
      __IOM unsigned int GSPI_WR_DATA_SWAP_MNL_CSN0 : 1;       /*!< [0..0] Swap the write data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_WR_DATA_SWAP_MNL_CSN1 : 1;       /*!< [1..1] Swap the write data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_WR_DATA_SWAP_MNL_CSN2 : 1;       /*!< [2..2] Swap the write data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_WR_DATA_SWAP_MNL_CSN3 : 1;       /*!< [3..3] Swap the write data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_RD_DATA_SWAP_MNL_CSN0 : 1;       /*!< [4..4] Swap the read data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_RD_DATA_SWAP_MNL_CSN1 : 1;       /*!< [5..5] Swap the read data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_RD_DATA_SWAP_MNL_CSN2 : 1;       /*!< [6..6] Swap the read data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_RD_DATA_SWAP_MNL_CSN3 : 1;       /*!< [7..7] Swap the read data inside
                                              the GSPI controller it-self. */
      __IOM unsigned int GSPI_MANUAL_SIZE_FRM_REG : 1;         /*!< [8..8] Manual reads and
                                                       manual writes */
      __IOM unsigned int RESERVED1 : 1;                        /*!< [9..9] reserved for future use */
      __IOM unsigned int TAKE_GSPI_MANUAL_WR_SIZE_FRM_REG : 1; /*!< [10..10] NONE */
      __IOM unsigned int MANUAL_GSPI_MODE : 1;                 /*!< [11..11] Internally the priority
                                               is given to manual mode */
      __IOM unsigned int RESERVED2 : 20;                       /*!< [31..12] reserved for future use */
    } GSPI_CONFIG2_b;
  };

  union {
    __IOM unsigned int GSPI_WRITE_DATA2; /*!< (@ 0x00000018) GSPI Write Data 2 Register */

    struct {
      __IOM unsigned int GSPI_MANUAL_WRITE_DATA2 : 4; /*!< [3..0] Number of bits to be written
                                           in write mode */
      __IOM unsigned int RESERVED1 : 3;               /*!< [6..4] reserved for future use    */
      __IOM unsigned int USE_PREV_LENGTH : 1;         /*!< [7..7] Use previous length */
      __IOM unsigned int RESERVED2 : 24;              /*!< [31..8] reserved for future use */
    } GSPI_WRITE_DATA2_b;
  };

  union {
    __IOM unsigned int GSPI_FIFO_THRLD; /*!< (@ 0x0000001C) GSPI FIFO Threshold Register */

    struct {
      __IOM unsigned int FIFO_AEMPTY_THRLD : 4; /*!< [3..0] FIFO almost empty threshold */
      __IOM unsigned int FIFO_AFULL_THRLD : 4;  /*!< [7..4] FIFO almost full threshold       */
      __IOM unsigned int WFIFO_RESET : 1;       /*!< [8..8] Write FIFO reset */
      __IOM unsigned int RFIFO_RESET : 1;       /*!< [9..9] read FIFO reset */
      __IOM unsigned int RESERVED1 : 22;        /*!< [31..10] reserved for future use  */
    } GSPI_FIFO_THRLD_b;
  };

  union {
    __IM unsigned int GSPI_STATUS; /*!< (@ 0x00000020) GSPI Status Register */

    struct {
      __IM unsigned int GSPI_BUSY : 1;           /*!< [0..0] State of Manual mode         */
      __IM unsigned int FIFO_FULL_WFIFO_S : 1;   /*!< [1..1] Full status indication
                                               for Wfifo in manual mode */
      __IM unsigned int FIFO_AFULL_WFIFO_S : 1;  /*!< [2..2] Almost full status indication for
                                      Wfifo in manual mode             */
      __IM unsigned int FIFO_EMPTY_WFIFO : 1;    /*!< [3..3] Empty status indication
                                              for Wfifo in manual mode */
      __IM unsigned int RESERVED1 : 1;           /*!< [4..4] reserved for future use        */
      __IM unsigned int FIFO_FULL_RFIFO : 1;     /*!< [5..5] Full status indication for
                                              Rfifo in manual mode  */
      __IM unsigned int RESERVED2 : 1;           /*!< [6..6] reserved for future use        */
      __IM unsigned int FIFO_EMPTY_RFIFO_S : 1;  /*!< [7..7] Empty status indication
                                                for Rfifo in manual mode */
      __IM unsigned int FIFO_AEMPTY_RFIFO_S : 1; /*!< [8..8] Aempty status indication for
                                       Rfifo in manual mode                  */
      __IM unsigned int GSPI_MANUAL_RD_CNT : 1;  /*!< [9..9] This is a result of 10
                                                bits ORing counter */
      __IM unsigned int GSPI_MANUAL_CSN : 1;     /*!< [10..10] Provide the status of
                                                chip select signal    */
      __IM unsigned int RESERVED3 : 21;          /*!< [31..11] reserved for future use */
    } GSPI_STATUS_b;
  };

  union {
    __IOM unsigned int GSPI_INTR_MASK; /*!< (@ 0x00000024) GSPI Interrupt Mask Register */

    struct {
      __IOM unsigned int GSPI_INTR_MASK : 1;         /*!< [0..0] GSPI Interrupt mask bit */
      __IOM unsigned int FIFO_AEMPTY_RFIFO_MASK : 1; /*!< [1..1] NONE */
      __IOM unsigned int FIFO_AFULL_RFIFO_MASK : 1;  /*!< [2..2] NONE  */
      __IOM unsigned int FIFO_AEMPTY_WFIFO_MASK : 1; /*!< [3..3] NONE */
      __IOM unsigned int FIFO_AFULL_WFIFO_MASK : 1;  /*!< [4..4] NONE  */
      __IOM unsigned int FIFO_FULL_WFIFO_MASK : 1;   /*!< [5..5] NONE   */
      __IOM unsigned int FIFO_EMPTY_RFIFO_MASK : 1;  /*!< [6..6] NONE  */
      __IOM unsigned int RESERVED1 : 25;             /*!< [31..7] reserved for future use */
    } GSPI_INTR_MASK_b;
  };

  union {
    __IOM unsigned int GSPI_INTR_UNMASK; /*!< (@ 0x00000028) GSPI Interrupt Unmask
                                         Register                             */

    struct {
      __IOM unsigned int GSPI_INTR_UNMASK : 1;         /*!< [0..0] GSPI Interrupt unmask bit */
      __IOM unsigned int FIFO_AEMPTY_RFIFO_UNMASK : 1; /*!< [1..1] NONE */
      __IOM unsigned int FIFO_AFULL_RFIFO_UNMASK : 1;  /*!< [2..2] NONE  */
      __IOM unsigned int FIFO_AEMPTY_WFIFO_UNMASK : 1; /*!< [3..3] NONE */
      __IOM unsigned int FIFO_AFULL_WFIFO_UNMASK : 1;  /*!< [4..4] NONE  */
      __IOM unsigned int FIFO_FULL_WFIFO_UNMASK : 1;   /*!< [5..5] NONE   */
      __IOM unsigned int FIFO_EMPTY_RFIFO_UNMASK : 1;  /*!< [6..6] NONE  */
      __IOM unsigned int RESERVED1 : 25;               /*!< [31..7] reserved for future use */
    } GSPI_INTR_UNMASK_b;
  };

  union {
    __IM unsigned int GSPI_INTR_STS; /*!< (@ 0x0000002C) GSPI Interrupt Status Register */

    struct {
      __IM unsigned int GSPI_INTR_LVL : 1;         /*!< [0..0] GSPI Interrupt status bit */
      __IM unsigned int FIFO_AEMPTY_RFIFO_LVL : 1; /*!< [1..1] NONE */
      __IM unsigned int FIFO_AFULL_RFIFO_LVL : 1;  /*!< [2..2] NONE  */
      __IM unsigned int FIFO_AEMPTY_WFIFO_LVL : 1; /*!< [3..3] NONE */
      __IM unsigned int FIFO_AFULL_WFIFO_LVL : 1;  /*!< [4..4] NONE  */
      __IM unsigned int FIFO_FULL_WFIFO_LVL : 1;   /*!< [5..5] NONE   */
      __IM unsigned int FIFO_EMPTY_RFIFO_LVL : 1;  /*!< [6..6] NONE  */
      __IM unsigned int RESERVED2 : 25;            /*!< [31..7] reserved for future use */
    } GSPI_INTR_STS_b;
  };

  union {
    __OM unsigned int GSPI_INTR_ACK; /*!< (@ 0x00000030) GSPI Interrupt Acknowledge
                                     Register                        */

    struct {
      __OM unsigned int GSPI_INTR_ACK : 1;         /*!< [0..0] GSPI Interrupt status bit */
      __OM unsigned int FIFO_AEMPTY_RFIFO_ACK : 1; /*!< [1..1] NONE */
      __OM unsigned int FIFO_AFULL_RFIFO_ACK : 1;  /*!< [2..2] NONE  */
      __OM unsigned int FIFO_AEMPTY_WFIFO_ACK : 1; /*!< [3..3] NONE */
      __OM unsigned int FIFO_AFULL_WFIFO_ACK : 1;  /*!< [4..4] NONE  */
      __OM unsigned int FIFO_FULL_WFIFO_ACK : 1;   /*!< [5..5] NONE   */
      __OM unsigned int FIFO_EMPTY_RFIFO_ACK : 1;  /*!< [6..6] NONE  */
      __OM unsigned int RESERVED2 : 25;            /*!< [31..7] reserved1            */
    } GSPI_INTR_ACK_b;
  };

  union {
    __IM unsigned int GSPI_STS_MC; /*!< (@ 0x00000034) GSPI State Machine Monitor
                                   Register                        */

    struct {
      __IM unsigned int BUS_CTRL_PSTATE : 3; /*!< [2..0] Provides SPI bus controller
                                             present state */
      __IM unsigned int SPI_RD_CNT : 13;     /*!< [15..3] number of pending bytes to be
                                         read by device                      */
      __IM unsigned int RESERVED1 : 16;      /*!< [31..16] reserved1  */
    } GSPI_STS_MC_b;
  };

  union {
    __IOM unsigned int GSPI_CLK_DIV; /*!< (@ 0x00000038) GSPI Clock Division Factor
                                     Register                        */

    struct {
      __IOM unsigned int GSPI_CLK_DIV_FACTOR : 8; /*!< [7..0] Provides GSPI clock division
                                        factor to the clock divider,  which takes
                                        SOC clock as input clock and generates
                                        required  clock according to division
                                        factor  */
      __IM unsigned int RESERVED1 : 24;           /*!< [31..8] reserved1 */
    } GSPI_CLK_DIV_b;
  };

  union {
    __IOM unsigned int GSPI_CONFIG3; /*!< (@ 0x0000003C) GSPI Configuration 3 Register */

    struct {
      __IOM unsigned int SPI_MANUAL_RD_LNTH_TO_BC : 15; /*!< [14..0] Bits are used to indicate
                                           the total number of bytes
                                           to read from flash during read
                                           operation */
      __IOM unsigned int RESERVED1 : 17;                /*!< [31..15] reserved1     */
    } GSPI_CONFIG3_b;
  };
  __IM unsigned int RESERVED1[16];

  union {
    union {
      __OM unsigned int GSPI_WRITE_FIFO[16]; /*!< (@ 0x00000080) GSPI fifo */

      struct {
        __OM unsigned int WRITE_FIFO : 32; /*!< [31..0] FIFO data is write to this
                                           address space */
      } GSPI_WRITE_FIFO_b[16];
    };

    union {
      __IM unsigned int GSPI_READ_FIFO[16]; /*!< (@ 0x00000080) GSPI READ FIFO */

      struct {
        __IM unsigned int READ_FIFO : 32; /*!< [31..0] FIFO data is read from this
                                          address space */
      } GSPI_READ_FIFO_b[16];
    };
  };
} GSPI0_Type; /*!< Size = 192 (0xc0) */

/* ===========================================================================================================================
  */
/* ================                                           SSI0
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief Synchronous Serial Interface(SSI) (SSI0)
  */

typedef struct { /*!< (@ 0x44020000) SSI0 Structure */

  union {
    __IOM unsigned int CTRLR0; /*!< (@ 0x00000000) Control Register 0 */

    struct {
      __IOM unsigned int DFS : 4;       /*!< [3..0] Select the data frame length (4-bit to
                                  16-bit serial data transfers) */
      __IOM unsigned int FRF : 2;       /*!< [5..4] Frame Format, Selects which serial
                                  protocol transfers the data */
      __IOM unsigned int SCPH : 1;      /*!< [6..6] Serial Clock Phase. Valid when the
                                   frame format (FRF) is set to Motorola SPI */
      __IOM unsigned int SCPOL : 1;     /*!< [7..7] Serial Clock Polarity. Valid when the frame
                         format (FRF) is set to Motorola SPI */
      __IOM unsigned int TMOD : 2;      /*!< [9..8] Selects the mode of transfer for
                                     serial communication              */
      __IOM unsigned int SLV_OE : 1;    /*!< [10..10] DW_apb_ssi is configured as a
                                     serial-slave device                */
      __IOM unsigned int SRL : 1;       /*!< [11..11] Shift Register Loop Used for testing
                                  purposes only               */
      __IOM unsigned int CFS : 4;       /*!< [15..12] Control Frame Size Selects the length of the
                       control word for the Micro wire frame format */
      __IOM unsigned int DFS_32 : 5;    /*!< [20..16] Selects the data frame length             */
      __IOM unsigned int SPI_FRF : 2;   /*!< [22..21] Selects data frame format for
                                      transmitting or receiving data */
      __IOM unsigned int RESERVED1 : 9; /*!< [31..23] Reserved for future use */
    } CTRLR0_b;
  };

  union {
    __IOM unsigned int CTRLR1; /*!< (@ 0x00000004) Control Register 1 */

    struct {
      __IOM unsigned int NDF : 16;       /*!< [15..0] Number of Data Frames.When TMOD = 10 or TMOD =
                        11, this register field sets the number of data frames to
                        be continuously received by the ssi_master */
      __IOM unsigned int RESERVED1 : 16; /*!< [31..16] Reserved for future use. */
    } CTRLR1_b;
  };

  union {
    __IOM unsigned int SSIENR; /*!< (@ 0x00000008) SSI Enable Register */

    struct {
      __IOM unsigned int SSI_EN : 1;     /*!< [0..0] Enables and disables all ssi operations */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } SSIENR_b;
  };

  union {
    __IOM unsigned int MWCR; /*!< (@ 0x0000000C) Micro wire Control Register */

    struct {
      __IOM unsigned int MWMOD : 1;      /*!< [0..0] The Micro wire transfer is
                                    sequential or non-sequential            */
      __IOM unsigned int MDD : 1;        /*!< [1..1] The direction of the data word when
                                    the Micro wire serial   protocol is used   */
      __IOM unsigned int MHS : 1;        /*!< [2..2] Microwire Handshaking. Used to enable
                                  and disable the busy/ready handshaking
                                  interface for the Microwire protocol */
      __IOM unsigned int RESERVED1 : 29; /*!< [31..3] Reserved for future use */
    } MWCR_b;
  };

  union {
    __IOM unsigned int SER; /*!< (@ 0x00000010) SLAVE ENABLE REGISTER */

    struct {
      __IOM unsigned int SER : 4;        /*!< [3..0] Each bit in this register corresponds to a slave
                       select line (ss_x_n) from the SSI master. */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved for future use */
    } SER_b;
  };

  union {
    __IOM unsigned int BAUDR; /*!< (@ 0x00000014) Baud Rate Select Register */

    struct {
      __IOM unsigned int SCKDV : 16;     /*!< [15..0] SSI Clock Divider.The LSB for this
                                     field is always set to 0 and is unaffected
                                     by a write operation, which ensures
                                                  an even value is held in this
                                     register */
      __IOM unsigned int RESERVED1 : 16; /*!< [31..16] Reserved for future use */
    } BAUDR_b;
  };

  union {
    __IOM unsigned int TXFTLR; /*!< (@ 0x00000018) Transmit FIFO Threshold Level Register */

    struct {
      __IOM unsigned int TFT : 4;        /*!< [3..0] Controls the level of entries (or below) at which
                       the transmit  FIFO controller triggers an interrupt */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved for future use */
    } TXFTLR_b;
  };

  union {
    __IOM unsigned int RXFTLR; /*!< (@ 0x0000001C) Receive FIFO Threshold Level */

    struct {
      __IOM unsigned int RFT : 4;        /*!< [3..0] Controls the level of entries (or above) at which
                       the receive FIFO controller triggers an interrupt */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved for future use */
    } RXFTLR_b;
  };

  union {
    __IM unsigned int TXFLR; /*!< (@ 0x00000020) Transmit FIFO Level Register */

    struct {
      __IM unsigned int TXTFL : 5;      /*!< [4..0] Contains the number of valid data
                                   entries in the transmit FIFO */
      __IM unsigned int RESERVED1 : 27; /*!< [31..5] Reserved for future use */
    } TXFLR_b;
  };

  union {
    __IM unsigned int RXFLR; /*!< (@ 0x00000024) Receive FIFO Level Register */

    struct {
      __IM unsigned int RXTFL : 5;      /*!< [4..0] Contains the number of valid data
                                   entries in the receive FIFO */
      __IM unsigned int RESERVED1 : 27; /*!< [31..5] Reserved for future use */
    } RXFLR_b;
  };

  union {
    __IM unsigned int SR; /*!< (@ 0x00000028) Status Register */

    struct {
      __IM unsigned int BUSY : 1;       /*!< [0..0] indicates that a serial transfer is in
                                  progress                    */
      __IM unsigned int TFNF : 1;       /*!< [1..1] Set when the transmit FIFO contains one or more
                        empty locations and is cleared when the FIFO is full */
      __IM unsigned int TFE : 1;        /*!< [2..2] When the transmit FIFO is completely
                                  empty this bit is  set  */
      __IM unsigned int RFNE : 1;       /*!< [3..3] Set when the receive FIFO contains one
                                  or more entries and is  cleared when the
                                  receive FIFO is empty */
      __IM unsigned int RFF : 1;        /*!< [4..4] When the receive FIFO is completely
                                  full this bit is  set  */
      __IM unsigned int TXE : 1;        /*!< [5..5] This bit is cleared when read  */
      __IM unsigned int DCOL : 1;       /*!< [6..6] This bit is set if the ss_in_n input
                                  is asserted by another master, while the ssi
                                  master is in the middle of the transfer */
      __IM unsigned int RESERVED1 : 25; /*!< [31..7] Reserved for future use */
    } SR_b;
  };

  union {
    __IOM unsigned int IMR; /*!< (@ 0x0000002C) Interrupt Mask Register */

    struct {
      __IOM unsigned int TXEIM : 1;     /*!< [0..0] Transmit FIFO Empty Interrupt Mask */
      __IOM unsigned int TXOIM : 1;     /*!< [1..1] Transmit FIFO Overflow Interrupt Mask */
      __IOM unsigned int RXUIM : 1;     /*!< [2..2] Receive FIFO Underflow Interrupt Mask */
      __IOM unsigned int RXOIM : 1;     /*!< [3..3] Receive FIFO Overflow Interrupt Mask */
      __IOM unsigned int RXFIM : 1;     /*!< [4..4] Receive FIFO Full Interrupt Mask */
      __IOM unsigned int MSTIM : 1;     /*!< [5..5] Multi-Master Contention Interrupt Mask */
      __IM unsigned int RESERVED1 : 26; /*!< [31..6] Reserved for future use */
    } IMR_b;
  };

  union {
    __IM unsigned int ISR; /*!< (@ 0x00000030) Interrupt Status Register */

    struct {
      __IM unsigned int TXEIS : 1;      /*!< [0..0] Transmit FIFO Empty Interrupt Status */
      __IM unsigned int TXOIS : 1;      /*!< [1..1] Transmit FIFO Overflow Interrupt Status */
      __IM unsigned int RXUIS : 1;      /*!< [2..2] Receive FIFO Underflow Interrupt Status */
      __IM unsigned int RXOIS : 1;      /*!< [3..3] Receive FIFO Overflow Interrupt Status */
      __IM unsigned int RXFIS : 1;      /*!< [4..4] Receive FIFO Full Interrupt Status */
      __IM unsigned int MSTIS : 1;      /*!< [5..5] Multi-Master Contention Interrupt Status */
      __IM unsigned int RESERVED1 : 26; /*!< [31..6] Reserved for future use */
    } ISR_b;
  };

  union {
    __IM unsigned int RISR; /*!< (@ 0x00000034) Raw Interrupt Status Register */

    struct {
      __IM unsigned int TXEIR : 1;      /*!< [0..0] Transmit FIFO Empty Raw Interrupt Status */
      __IM unsigned int TXOIR : 1;      /*!< [1..1] Transmit FIFO Overflow Raw Interrupt
                                   Status                        */
      __IM unsigned int RXUIR : 1;      /*!< [2..2] Receive FIFO Underflow Raw Interrupt
                                   Status                        */
      __IM unsigned int RXOIR : 1;      /*!< [3..3] Receive FIFO Overflow Raw Interrupt Status */
      __IM unsigned int RXFIR : 1;      /*!< [4..4] Receive FIFO Full Raw Interrupt Status */
      __IM unsigned int MSTIR : 1;      /*!< [5..5] Multi-Master Contention Raw Interrupt
                                   Status                       */
      __IM unsigned int RESERVED1 : 26; /*!< [31..6] Reserved for future use */
    } RISR_b;
  };

  union {
    __IM unsigned int TXOICR; /*!< (@ 0x00000038) Transmit FIFO Overflow Interrupt
                              Clear Register            */

    struct {
      __IM unsigned int TXOICR : 1;     /*!< [0..0] Clear Transmit FIFO Overflow Interrupt This
                          register reflects the status of the interrupt */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } TXOICR_b;
  };

  union {
    __IM unsigned int RXOICR; /*!< (@ 0x0000003C) Receive FIFO Overflow Interrupt
                              Clear Register             */

    struct {
      __IM unsigned int RXOICR : 1;     /*!< [0..0] This register reflects the status of
                                    the interrupt A read from this  register
                                    clears the ssi_rxo_intr interrupt */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } RXOICR_b;
  };

  union {
    __IM unsigned int RXUICR; /*!< (@ 0x00000040) Receive FIFO Underflow Interrupt
                              Clear Register            */

    struct {
      __IM unsigned int RXUICR : 1;     /*!< [0..0] This register reflects the status of
                                    the interrupt A read from this  register
                                    clears the ssi_rxu_intr interrupt */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } RXUICR_b;
  };

  union {
    __IM unsigned int MSTICR; /*!< (@ 0x00000044) Multi-Master Interrupt Clear Register */

    struct {
      __IM unsigned int MSTICR : 1;     /*!< [0..0] This register reflects the status of
                                    the interrupt A read from this  register
                                    clears the ssi_mst_intr interrupt */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } MSTICR_b;
  };

  union {
    __IM unsigned int ICR; /*!< (@ 0x00000048) Interrupt Clear Register */

    struct {
      __IM unsigned int ICR : 1;        /*!< [0..0] This register is set if any of the
                                 interrupts below are active A read clears the
                                 ssi_txo_intr, ssi_rxu_intr, ssi_rxo_intr, and
                                 the ssi_mst_intr interrupts */
      __IM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } ICR_b;
  };

  union {
    __IOM unsigned int DMACR; /*!< (@ 0x0000004C) DMA Control Register */

    struct {
      __IOM unsigned int RDMAE : 1;     /*!< [0..0] This bit enables/disables the
                                        receive FIFO DMA channel             */
      __IOM unsigned int TDMAE : 1;     /*!< [1..1] This bit enables/disables the
                                        transmit FIFO DMA channel            */
      __IM unsigned int RESERVED1 : 30; /*!< [31..2] Reserved for future use */
    } DMACR_b;
  };

  union {
    __IOM unsigned int DMATDLR; /*!< (@ 0x00000050) DMA Transmit Data Level */

    struct {
      __IOM unsigned int DMATDL : 4;    /*!< [3..0] This bit field controls the level at which a
                          DMA request is made by the transmit logic */
      __IM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved for future use */
    } DMATDLR_b;
  };

  union {
    __IOM unsigned int DMARDLR; /*!< (@ 0x00000054) DMA Receive Data Level Register */

    struct {
      __IOM unsigned int DMARDL : 4;     /*!< [3..0] This bit field controls the level at which a
                          DMA request is made by the receive logic */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved for future use */
    } DMARDLR_b;
  };

  union {
    __IM unsigned int IDR; /*!< (@ 0x00000058) Identification Register */

    struct {
      __IM unsigned int IDCODE : 32; /*!< [31..0] This register contains the
                                     peripherals identification code */
    } IDR_b;
  };

  union {
    __IM unsigned int SSI_COMP_VERSION; /*!< (@ 0x0000005C) coreKit version ID register */

    struct {
      __IM unsigned int SSI_COMP_VERSION : 32; /*!< [31..0] Contains the hex representation of
                                     the Synopsys component version */
    } SSI_COMP_VERSION_b;
  };

  union {
    __IOM unsigned int DR; /*!< (@ 0x00000060) Data Register */

    struct {
      __IOM unsigned int DR : 32; /*!< [31..0] When writing to this register must
                                  right-justify the data */
    } DR_b;
  };
  __IM unsigned int RESERVED[35];

  union {
    __IOM unsigned int RX_SAMPLE_DLY; /*!< (@ 0x000000F0) Rx Sample Delay Register */

    struct {
      __IOM unsigned int RSD : 8;        /*!< [7..0] Receive Data (rxd) Sample Delay. This register is
                       used to delay the sample of the rxd input signal. */
      __IOM unsigned int RESERVED1 : 24; /*!< [31..8] Reserved for future use */
    } RX_SAMPLE_DLY_b;
  };

  union {
    __IOM unsigned int SPI_CTRLR0; /*!< (@ 0x000000F4) SPI control Register */

    struct {
      __IOM unsigned int TRANS_TYPE : 2;  /*!< [1..0] Address and instruction
                                         transfer format */
      __IOM unsigned int ADDR_L : 4;      /*!< [5..2] This bit defines length of address to be
                          transmitted, The transfer begins only after these many
                          bits are programmed into the FIFO */
      __IM unsigned int RESERVED1 : 2;    /*!< [7..6] Reserved for future use */
      __IOM unsigned int INST_L : 2;      /*!< [9..8] DUAL/QUAD length in bits   */
      __IM unsigned int RESERVED2 : 1;    /*!< [10..10] Reserved for future use */
      __IOM unsigned int WAIT_CYCLES : 4; /*!< [14..11] This bit defines the wait cycles in
                               dual/quad mode between control frames transmit and
                               data reception, Specified as number of SPI clock
                               cycles */
      __IM unsigned int RESERVED3 : 17;   /*!< [31..15] Reserved for future use */
    } SPI_CTRLR0_b;
  };
} SSI0_Type; /*!< Size = 248 (0xf8) */

/* ===========================================================================================================================
  */
/* ================                                            SIO
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief SERIAL GENERAL PERPOSE INPUT/OUTPUT (SIO)
  */

typedef struct { /*!< (@ 0x47000000) SIO Structure */

  union {
    __IOM unsigned int SIO_ENABLE_REG; /*!< (@ 0x00000000) ENABLE REGISTER */

    struct {
      __IOM unsigned int SIO_OPERATION_ENABLE : 16; /*!< [15..0] Contains the
                                                    Enables for all SIO */
      __IM unsigned int RESERVED3 : 16;             /*!< [31..16] Reserved for future use */
    } SIO_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int SIO_PAUSE_REG; /*!< (@ 0x00000004) PAUSE REGISTER */

    struct {
      __IOM unsigned int SIO_POSITION_COUNTER_DISABLE : 16; /*!< [15..0] Contains
                                                            sio position counter
                                                            disable for all SIOs
                                                          */
      __IM unsigned int RESERVED3 : 16;                     /*!< [31..16] Reserved for future use */
    } SIO_PAUSE_REG_b;
  };

  union {
    __IM unsigned int SIO_GPIO_IN_REG; /*!< (@ 0x00000008) GPIO Input Register */

    struct {
      __IM unsigned int IN_VALUE : 32; /*!< [31..0] GPIO input pin status */
    } SIO_GPIO_IN_REG_b;
  };

  union {
    __IOM unsigned int SIO_GPIO_OUT_REG; /*!< (@ 0x0000000C) GPIO Output Register */

    struct {
      __IOM unsigned int OUT_VALUE : 32; /*!< [31..0] Value to be loaded on GPIO out pins */
    } SIO_GPIO_OUT_REG_b;
  };

  union {
    __IOM unsigned int SIO_GPIO_OEN_REG; /*!< (@ 0x00000010) GPIO Output enable Register */

    struct {
      __IOM unsigned int OEN_VALUE : 32; /*!< [31..0] OEN for the GPIO pins */
    } SIO_GPIO_OEN_REG_b;
  };

  union {
    __IOM unsigned int SIO_GPIO_INTR_EN_SET_REG; /*!< (@ 0x00000014) GPIO Interrupt
                                                 Enable Set Register */

    struct {
      __OM unsigned int INTR_ENABLE_SET : 16; /*!< [15..0] gpio interrupt enable set
                                              register for all SIOs */
      __IM unsigned int RESERVED1 : 16;       /*!< [31..16] Reserved for future use */
    } SIO_GPIO_INTR_EN_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_GPIO_INTR_EN_CLEAR_REG; /*!< (@ 0x00000018) GPIO Interrupt
                                                  Enable Clear Register */

    struct {
      __OM unsigned int INTR_ENABLE_CLEAR : 16; /*!< [15..0] gpio interrupt enable
                                                Clear register for all SIOs */
      __OM unsigned int RESERVED1 : 16;         /*!< [31..16] Reserved for future use */
    } SIO_GPIO_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_GPIO_INTR_MASK_SET_REG; /*!< (@ 0x0000001C) GPIO Interrupt Enable
                                        Clear Register                       */

    struct {
      __IOM unsigned int INTR_MASK_SET : 16; /*!< [15..0] Common gpio interrupt mask
                                             set register for all SIOs */
      __IM unsigned int RESERVED1 : 16;      /*!< [31..16] Reserved for future use */
    } SIO_GPIO_INTR_MASK_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_GPIO_INTR_MASK_CLEAR_REG; /*!< (@ 0x00000020) GPIO Interrupt Enable
                                          Clear Register                       */

    struct {
      __OM unsigned int INTR_MASK_CLEAR : 16; /*!< [15..0] gpio interrupt mask clear
                                              register for all SIOs */
      __OM unsigned int RESERVED1 : 16;       /*!< [31..16] Reserved for future use */
    } SIO_GPIO_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_GPIO_INTR_STATUS_REG; /*!< (@ 0x00000024) GPIO Interrupt
                                                 Status Register */

    struct {
      __OM unsigned int INTR_MASK_SET : 16; /*!< [15..0] Common gpio interrupt
                                            status register for all SIOs */
      __IM unsigned int RESERVED1 : 16;     /*!< [31..16] Reserved for future use     */
    } SIO_GPIO_INTR_STATUS_REG_b;
  };

  union {
    __IM unsigned int SIO_SHIFT_COUNTER[16]; /*!< (@ 0x00000028) Shift counter register */

    struct {
      __IM unsigned int SHIFT_COUNTER : 14; /*!< [13..0] shift counter current value       */
      __IM unsigned int RESERVED1 : 18;     /*!< [31..14] Reserved for future use */
    } SIO_SHIFT_COUNTER_b[16];
  };

  union {
    __IOM unsigned int SIO_BUFFER_REG[16]; /*!< (@ 0x00000068) Buffer Register */

    struct {
      __IOM unsigned int DATA : 32; /*!< [31..0] Data to load into the shift register */
    } SIO_BUFFER_REG_b[16];
  };

  union {
    __IOM unsigned int SIO_SHIFT_COUNT_PRELOAD_REG[16]; /*!< (@ 0x000000A8) Shift counter
                                             Reload Register */

    struct {
      __IOM unsigned int RELOAD_VALUE : 14; /*!< [13..0] division factor required to
                                            generate shift clock */
      __IM unsigned int RESERVED1 : 1;      /*!< [14..14] Reserved for future use      */
      __IOM unsigned int REVERSE_LOAD : 1;  /*!< [15..15] When set, the data on APB is loaded to
                                buffer is reverse order */
      __IM unsigned int RESERVED2 : 16;     /*!< [31..16] Reserved for future use */
    } SIO_SHIFT_COUNT_PRELOAD_REG_b[16];
  };

  union {
    __IOM unsigned int SIO_DATA_POS_COUNT_REG[16]; /*!< (@ 0x000000E8) Data Position
                                                   Counter Register */

    struct {
      __IOM unsigned int RELOAD_VALUE : 8;     /*!< [7..0] No. of shifts to happen before reloading
                                the shift register with data/ pausing the
                                operation */
      __IOM unsigned int POSITION_COUNTER : 8; /*!< [15..8] The position counter can
                                               be loaded via AHB */
      __IM unsigned int RESERVED3 : 16;        /*!< [31..16] Reserved for future use */
    } SIO_DATA_POS_COUNT_REG_b[16];
  };

  union {
    __IOM unsigned int SIO_CONFIG_REG[16]; /*!< (@ 0x00000128) Configuration Register */

    struct {
      __IOM unsigned int FULL_ENABLE : 1;                  /*!< [0..0] When set, fifo full indication would be
                               asserted when internal buffer is full */
      __IOM unsigned int EMPTY_ENABLE : 1;                 /*!< [1..1] When set, fifo full indication would be
                                asserted when internal buffer is empty */
      __IOM unsigned int EDGE_SEL : 1;                     /*!< [2..2] edge selection */
      __IOM unsigned int CLK_SEL : 1;                      /*!< [3..3] clock selection  */
      __IOM unsigned int IGNORE_FIRST_SHIFT_CONDITION : 1; /*!< [4..4] data shift
                                                           condition */
      __IOM unsigned int FLOW_CONTROL_ENABLED : 1;         /*!< [5..5] flow control */
      __IOM unsigned int PATTERN_MATCH_ENABLE : 1;         /*!< [6..6] pattern match */
      __IOM unsigned int QUALIFIER_MODE : 1;               /*!< [7..7] qualifier mode       */
      __IOM unsigned int QUALIFY_CLOCK : 1;                /*!< [8..8] qualify clock        */
      __IOM unsigned int INVERT_CLOCK : 1;                 /*!< [9..9] invert clock         */
      __IOM unsigned int PARALLEL_MODE : 2;                /*!< [11..10] No. of bits to
                                                   shift/capture at valid clk edge        */
      __IOM unsigned int PIN_DETECTION_MODE : 2;           /*!< [13..12] Pin mode to be considered for
                                      gpio interrupt                     */
      __IOM unsigned int SET_CLK_OUT : 1;                  /*!< [14..14] When high sets the sio clock_out port.
                               This is used only when sio is not enabled */
      __IOM unsigned int RESET_CLK_OUT : 1;                /*!< [15..15] When high resets the sio
                                            clock_out port. This is used only
                                            when sio is not enabled */
      __IOM unsigned int LOAD_DATA_POS_CNTR_VIA_APB : 1;   /*!< [16..16] When set, data position
                                              counter can be loaded via APB */
      __IM unsigned int RESERVED1 : 15;                    /*!< [31..17] Reserved for future use */
    } SIO_CONFIG_REG_b[16];
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_MASK_REG_SLICE_0; /*!< (@ 0x00000168) Pattern Match
                                                Mask Register 0 */

    struct {
      __IOM unsigned int MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower
                                                       16 bits */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_0_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_MASK_REG_SLICE_1; /*!< (@ 0x0000016C) Pattern Match
                                                Mask Register Slice 1 */

    struct {
      __IOM unsigned int MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower
                                                       16 bits */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_1_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_MASK_REG_SLICE_2; /*!< (@ 0x00000170) Pattern Match
                                                Mask Register Slice 2 */

    struct {
      __IOM unsigned int MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower
                                                       16 bits */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_2_b;
  };
  __IM unsigned int RESERVED[5];

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_MASK_REG_SLICE_8; /*!< (@ 0x00000188) Pattern Match
                                                Mask Register Slice 8 */

    struct {
      __IOM unsigned int MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower
                                                       16 bits */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_8_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_MASK_REG_SLICE_9; /*!< (@ 0x0000018C) Pattern Match
                                                Mask Register Slice 9 */

    struct {
      __IOM unsigned int MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower
                                                       16 bits */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_9_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_MASK_REG_SLICE_10; /*!< (@ 0x00000190) Pattern Match
                                                 Mask Register Slice 10 */

    struct {
      __IOM unsigned int MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower
                                                       16 bits */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_10_b;
  };
  __IM unsigned int RESERVED1[5];

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_REG_SLICE_0; /*!< (@ 0x000001A8) Pattern Match Mask
                                           Register Slice 0 */

    struct {
      __IOM unsigned int PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16-bits of pattern
                                               to be detected */
    } SIO_PATTERN_MATCH_REG_SLICE_0_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_REG_SLICE_1; /*!< (@ 0x000001AC) Pattern Match Mask
                                           Register Slice 1 */

    struct {
      __IOM unsigned int PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16-bits of pattern
                                               to be detected */
    } SIO_PATTERN_MATCH_REG_SLICE_1_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_REG_SLICE_2; /*!< (@ 0x000001B0) Pattern Match Mask
                                           Register Slice 2 */

    struct {
      __IOM unsigned int PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16-bits of pattern
                                               to be detected */
    } SIO_PATTERN_MATCH_REG_SLICE_2_b;
  };
  __IM unsigned int RESERVED2[5];

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_REG_SLICE_8; /*!< (@ 0x000001C8) Pattern Match Mask
                                           Register Slice 8 */

    struct {
      __IOM unsigned int PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16 bits of pattern
                                               to be detected */
    } SIO_PATTERN_MATCH_REG_SLICE_8_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_REG_SLICE_9; /*!< (@ 0x000001CC) Pattern Match Mask
                                           Register Slice 9 */

    struct {
      __IOM unsigned int PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16 bits of pattern
                                               to be detected */
    } SIO_PATTERN_MATCH_REG_SLICE_9_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_REG_SLICE_10; /*!< (@ 0x000001D0) Pattern Match Mask
                                            Register Slice 10 */

    struct {
      __IOM unsigned int PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16 bits of pattern
                                               to be detected */
    } SIO_PATTERN_MATCH_REG_SLICE_10_b;
  };
  __IM unsigned int RESERVED3[7];

  union {
    __IOM unsigned int SIO_SHIFT_INTR_EN_SET_REG; /*!< (@ 0x000001F0) Shift Interrupt Enable
                                       Set Register                        */

    struct {
      __IOM unsigned int INTR_ENABLE_SET : 16; /*!< [15..0] Common shift interrupt enable set
                                    register for all SIOs           */
      __IM unsigned int RESERVED3 : 16;        /*!< [31..16] Reserved for future use */
    } SIO_SHIFT_INTR_EN_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_SHIFT_INTR_EN_CLEAR_REG; /*!< (@ 0x000001F4) Shift Interrupt Enable
                                         Clear Register                      */

    struct {
      __OM unsigned int INRT_ENABLE_CLEAR : 16; /*!< [15..0] Common shift interrupt enable
                                        Clear register for all   SIOs   */
      __OM unsigned int RESERVED3 : 16;         /*!< [31..16] Reserved for future use */
    } SIO_SHIFT_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_SHIFT_INTR_MASK_SET_REG; /*!< (@ 0x000001F8) Shift Interrupt Mask
                                         Set Register                          */

    struct {
      __IOM unsigned int INTR_MASK_SET : 16; /*!< [15..0] Common shift interrupt enable Set
                                  register for all SIOs           */
      __IM unsigned int RESERVED1 : 16;      /*!< [31..16] Reserved for future use */
    } SIO_SHIFT_INTR_MASK_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_SHIFT_INTR_MASK_CLEAR_REG; /*!< (@ 0x000001FC) Shift Interrupt Mask
                                           Clear Register */

    struct {
      __OM unsigned int INTR_MASK_CLEAR : 16; /*!< [15..0] Common shift interrupt mask clear
                                    register for all SIOs           */
      __OM unsigned int RESERVED1 : 16;       /*!< [31..16] Reserved for future use */
    } SIO_SHIFT_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_SHIFT_INTR_STATUS_REG; /*!< (@ 0x00000200) Shift
                                                  Interrupt Status Register */

    struct {
      __IOM unsigned int INTR_ENABLE_SET : 16; /*!< [15..0] Common shift interrupt mask clear
                                    register for all SIOs           */
      __IM unsigned int RESERVED1 : 16;        /*!< [31..16] Reserved for future use */
    } SIO_SHIFT_INTR_STATUS_REG_b;
  };

  union {
    __IOM unsigned int SIO_SWAP_INTR_EN_SET_REG; /*!< (@ 0x00000204) Swap Interrupt
                                                 Enable Set Register */

    struct {
      __IOM unsigned int INTR_ENABLE_SET : 16; /*!< [15..0] Swap interrupt enable
                                               set register for all SIOs */
      __IM unsigned int RESERVED1 : 16;        /*!< [31..16] Reserved for future use */
    } SIO_SWAP_INTR_EN_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_SWAP_INTR_EN_CLEAR_REG; /*!< (@ 0x00000208) Swap Interrupt
                                                  Enable Clear Register */

    struct {
      __OM unsigned int INTR_ENABLE_CLEAR : 16; /*!< [15..0] Swap interrupt enable
                                                Clear register for all SIOs */
      __OM unsigned int RESERVED1 : 16;         /*!< [31..16] Reserved for future use */
    } SIO_SWAP_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_SWAP_INTR_MASK_SET_REG; /*!< (@ 0x0000020C) Swap Interrupt Mask Set
                                        Register                           */

    struct {
      __IOM unsigned int INTR_MASK_SET : 16; /*!< [15..0] Common swap interrupt mask
                                             set register for all SIOs */
      __IM unsigned int RESERVED1 : 16;      /*!< [31..16] Reserved for future use */
    } SIO_SWAP_INTR_MASK_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_SWAP_INTR_MASK_CLEAR_REG; /*!< (@ 0x00000210) Swap Interrupt Mask
                                          Clear Register */

    struct {
      __OM unsigned int INTR_MASK_CLEAR : 16; /*!< [15..0] Common swap interrupt mask Clear
                                    register for all SIOs            */
      __OM unsigned int RESERVED1 : 16;       /*!< [31..16] Reserved for future use */
    } SIO_SWAP_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_SWAP_INTR_STATUS_REG; /*!< (@ 0x00000214) Swap Interrupt
                                                 Statusr Register */

    struct {
      __IOM unsigned int INTR_ENABLE_SET : 16; /*!< [15..0] Common swap interrupt
                                               status register for all SIOs */
      __IM unsigned int RESERVED1 : 16;        /*!< [31..16] Reserved for future use */
    } SIO_SWAP_INTR_STATUS_REG_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_INTR_EN_SET_REG; /*!< (@ 0x00000218) Pattern Match
                                               Interrupt Enable Set Register */

    struct {
      __IOM unsigned int INTR_ENABLE_SET : 16; /*!< [15..0] Common pattern or buffer under run
                                    interrupt enable set register for all SIOs.
                                    Each bit corresponds to one SIO */
      __IM unsigned int RESERVED1 : 16;        /*!< [31..16] Reserved for future use */
    } SIO_PATTERN_MATCH_INTR_EN_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_PATTERN_MATCH_INTR_EN_CLEAR_REG; /*!< (@ 0x0000021C) Pattern Match
                                                 Interrupt Enable Clear Register
                                               */

    struct {
      __OM unsigned int INRT_ENABLE_CLEAR : 16; /*!< [15..0] Common pattern or buffer under
                                        run interrupt enable
                                              clear register for all SIOs   */
      __OM unsigned int RESERVED1 : 16;         /*!< [31..16] Reserved for future use */
    } SIO_PATTERN_MATCH_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_INTR_MASK_SET_REG; /*!< (@ 0x00000220) Pattern Match
                                                 Interrupt Mask Set Register */

    struct {
      __IOM unsigned int INTR_MASK_SET : 16; /*!< [15..0] Common pattern or buffer under run
                                  interrupt mask set register for all SIOs */
      __IM unsigned int RESERVED1 : 16;      /*!< [31..16] Reserved for future use */
    } SIO_PATTERN_MATCH_INTR_MASK_SET_REG_b;
  };

  union {
    __OM unsigned int SIO_PATTERN_MATCH_INTR_MASK_CLEAR_REG; /*!< (@ 0x00000224) Pattern Match
                                                   Interrupt Mask Clear Register
                                                 */

    struct {
      __OM unsigned int INTR_MASK_CLEAR : 16; /*!< [15..0] Common pattern or buffer
                                              under run interrupt mask clear
                                                      register for all SIOs */
      __OM unsigned int RESERVED1 : 16;       /*!< [31..16] Reserved for future use */
    } SIO_PATTERN_MATCH_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SIO_PATTERN_MATCH_INTR_STATUS_REG; /*!< (@ 0x00000228) Pattern Match
                                               Interrupt Status Register */

    struct {
      __IOM unsigned int INTR_STATUS : 16; /*!< [15..0] Common pattern interrupt
                                           status register for all SIOs */
      __IM unsigned int RESERVED3 : 16;    /*!< [31..16] Reserved for future use    */
    } SIO_PATTERN_MATCH_INTR_STATUS_REG_b;
  };

  union {
    __IOM unsigned int SIO_BUFFER_INTR_STATUS_REG; /*!< (@ 0x0000022C) Buffer
                                                   Interrupt Status Register */

    struct {
      __IOM unsigned int INTR_STATUS : 16; /*!< [15..0] Common pattern interrupt
                                           status register for all SIOs */
      __IM unsigned int RESERVED1 : 16;    /*!< [31..16] Reserved for future use    */
    } SIO_BUFFER_INTR_STATUS_REG_b;
  };

  union {
    __IOM unsigned int SIO_OUT_MUX_REG[16]; /*!< (@ 0x00000230) Output muxing Register */

    struct {
      __IOM unsigned int DOUT_OEN_SEL : 3; /*!< [2..0] OEN select for GPIO pin 0 */
      __IOM unsigned int DOUT_SEL : 3;     /*!< [5..3] Output mux select for GPIO pin 0 */
      __IM unsigned int RESERVED1 : 26;    /*!< [31..6] Reserved for future use */
    } SIO_OUT_MUX_REG_b[16];
  };

  union {
    __IOM unsigned int SIO_INPUT_MUX_REG[16]; /*!< (@ 0x00000270) Input muxing Register */

    struct {
      __IOM unsigned int CLK_SEL : 3;          /*!< [2..0] Input clock select for SIO 0 */
      __IOM unsigned int QUALIFIER_SELECT : 2; /*!< [4..3] qualifier select */
      __IOM unsigned int QUALIFIER_MODE : 2;   /*!< [6..5] qualifier mode   */
      __IOM unsigned int DIN_SEL : 3;          /*!< [9..7] Data in mux select          */
      __IM unsigned int RESERVED1 : 22;        /*!< [31..10] Reserved for future use */
    } SIO_INPUT_MUX_REG_b[16];
  };

  union {
    __IOM unsigned int SIO_FIFO_WR_RD_REG; /*!< (@ 0x000002B0) FIFO READ/WRITE Register */

    struct {
      __IOM unsigned int FIFO_DATA_REGISTER : 32; /*!< [31..0] Writes and read into
                                                  this register will be written
                                                  into SIO buffer register */
    } SIO_FIFO_WR_RD_REG_b;
  };

  union {
    __IOM unsigned int SIO_FIFO_WR_OFFSET_START_REG; /*!< (@ 0x000002B4) Points to start slice
                                          number forming the FIFO              */

    struct {
      __IOM unsigned int SIO_START_SLICE_NUMBER : 32; /*!< [31..0] Points to start slice number
                                           forming the FIFO,On write,
                                           FIFO_WR_OFFSET_CNT_REG  will also be
                                           reset to the value pointed written
                                           into this register */
    } SIO_FIFO_WR_OFFSET_START_REG_b;
  };

  union {
    __IOM unsigned int SIO_FIFO_WR_OFFSET_END_REG; /*!< (@ 0x000002B8) SIO last slice no
                                        indication Register */

    struct {
      __IOM unsigned int SIO_END_SLICE_NUMBER : 32; /*!< [31..0] points to last
                                                    slice no forming fifo */
    } SIO_FIFO_WR_OFFSET_END_REG_b;
  };

  union {
    __IOM unsigned int SIO_FIFO_WR_OFFSET_CNT_REG; /*!< (@ 0x000002BC) Points to current slice
                                        number forming the FIFO            */

    struct {
      __IOM unsigned int SIO_CURRENT_SLICE_NUMBER : 32; /*!< [31..0] Next FIFO operation will
                                           happen to buffer in the slice
                                           pointed by this register */
    } SIO_FIFO_WR_OFFSET_CNT_REG_b;
  };

  union {
    __IOM unsigned int SIO_FIFO_RD_OFFSET_START_REG; /*!< (@ 0x000002C0) Points to start slice
                                          number forming the FIFO              */

    struct {
      __IOM unsigned int SIO_START_SLICE_NUMBER : 32; /*!< [31..0] Points to start slice number
                                           forming the FIFO                    */
    } SIO_FIFO_RD_OFFSET_START_REG_b;
  };

  union {
    __IOM unsigned int SIO_FIFO_RD_OFFSET_END_REG; /*!< (@ 0x000002C4) Points to last slice
                                        number forming the FIFO               */

    struct {
      __IOM unsigned int SIO_END_SLICE_NUMBER : 32; /*!< [31..0] Points to last slice number
                                         forming the FIFO                      */
    } SIO_FIFO_RD_OFFSET_END_REG_b;
  };

  union {
    __IOM unsigned int SIO_FIFO_RD_OFFSET_CNT_REG; /*!< (@ 0x000002C8) Points to start current
                                        number forming the FIFO            */

    struct {
      __IOM unsigned int SIO_CURRENT_SLICE_NUMBER : 32; /*!< [31..0] Next FIFO operation will
                                           happen to buffer in the slice pointed
                                           by this register This register has to
                                           be set to zero before starting fresh
                                           DMA operation */
    } SIO_FIFO_RD_OFFSET_CNT_REG_b;
  };
} SIO_Type; /*!< Size = 716 (0x2cc) */

/* ===========================================================================================================================
  */
/* ================                                           QSPI
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The queued serial peripheral interface module provides a serial
  * peripheral interface with queued transfer capability (QSPI)
  */

typedef struct { /*!< (@ 0x12000000) QSPI Structure */

  union {
    __IOM unsigned int QSPI_CLK_CONFIG; /*!< (@ 0x00000000) QSPI Clock Configuration
                                        Register                          */

    struct {
      __IOM unsigned int QSPI_AUTO_CSN_HIGH_CNT : 5;    /*!< [4..0] Minimum SOC clock cycles,
                                          during which QSPI auto csn should be
                                          high between consecutive CSN assertions
                                        */
      __IOM unsigned int QSPI_CLK_SYNC : 1;             /*!< [5..5] If the clock frequency to
                                          FLASH(spi_clk) and QSPI(hclk)          controller is
                                          same,  this bit can be set to one to by-pass
                                           the syncros results in time consumption          */
      __IOM unsigned int RESERVED1 : 2;                 /*!< [7..6] reserved1   */
      __IOM unsigned int QSPI_CLK_EN : 1;               /*!< [8..8] QSPI clock enable */
      __IOM unsigned int RESERVED2 : 3;                 /*!< [11..9] reserved2   */
      __IOM unsigned int SPI_CLK_DELAY_VAL : 6;         /*!< [17..12] Delay value programmed to RX QSPI
                                     DLL on read side. This delay is used to
                                     delay the pad clock/DQS according to the
                                     requirement */
      __IOM unsigned int OCTA_MODE_ENABLE_WITH_DQS : 1; /*!< [18..18] Enables SPI octa mode
                                             along with DQS in DDR mode */
      __IOM unsigned int QSPI_DLL_ENABLE : 1;           /*!< [19..19] Enable for RX QSPI DLL in read
                                   mode.This is used in M4SS QSPI DDR pads to
                                   delay the pad clock DQS input */
      __IOM unsigned int DDR_CLK_POLARITY_FROM_REG : 1; /*!< [20..20] Used this bit to sample
                                           the data at posedge negedge after
                                           interface FFs with internal qspi clock
                                           0-Sample at negedge 1-Sample at
                                           posedge */
      __IOM unsigned int QSPI_DLL_ENABLE_TX : 1;        /*!< [21..21] Enable for TX QSPI DLL in write
                                    path. This is used in M4SS QSPI DDR pads to
                                    delay the qspi clock output. 0–DLL is
                                    disabled bypassed 1–DLL is enabled */
      __IOM unsigned int SPI_CLK_DELAY_VAL_TX : 6;      /*!< [27..22] Delay value programmed to TX
                                      QSPI DLL in write path. This delay is used
                                      to delay the qspi clock output according
                                      to the requirement */
      __IOM unsigned int QSPI_RX_DQS_DLL_CALIB : 1;     /*!< [28..28] Delay value programmed to TX
                                       QSPI DLL in write path. This delay is used
                                       to delay the qspi clock output according
                                       to the requirement */
      __IOM unsigned int RESERVED3 : 3;                 /*!< [31..29] reserved3  */
    } QSPI_CLK_CONFIG_b;
  };

  union {
    __IOM unsigned int QSPI_BUS_MODE; /*!< (@ 0x00000004) QSPI Bus Mode Register */

    struct {
      __IOM unsigned int QSPI_9116_FEATURE_EN : 1;               /*!< [0..0] 9115 specific features are
                                        enabled with this enable */
      __IOM unsigned int QSPI_MAN_MODE_CONF_CSN0 : 2;            /*!< [2..1] Configures the QSPI flash for
                                           Single/Dual/Quad mode operation in
                                           manual mode */
      __IOM unsigned int AUTO_MODE_RESET : 1;                    /*!< [3..3] QSPI Auto controller reset. This is
                                   not a Self clearing bit */
      __IOM unsigned int QSPI_PREFETCH_EN : 1;                   /*!< [4..4] Pre-fetch of data from the model
                                    which is connected to QSPI, automatically
                                    with out reading on AHB and is supplied to
                                    AHB,  when address is matched with AHB read
                                    transaction address */
      __IOM unsigned int QSPI_WRAP_EN : 1;                       /*!< [5..5] Model wrap is considered with this bit
                                and uses wrap instruction to read from FLASH */
      __IOM unsigned int QSPI_AUTO_MODE_FRM_REG : 1;             /*!< [6..6] QSPI Mode of Operation */
      __IOM unsigned int PROGRAMMABLE_AUTO_CSN_BASE_ADDR_EN : 1; /*!< [7..7] Programmable auto
                                                      csn mode enable */
      __IOM unsigned int QSPI_D2_OEN_CSN0 : 1;                   /*!< [8..8] Direction Control for SPI_IO2 in
                                    case of dual/single mode for chip select0
                                    csn0. It is used both in Auto and
                                      Manual Mode */
      __IOM unsigned int QSPI_D3_OEN_CSN0 : 1;                   /*!< [9..9] Direction Control for SPI_IO3 in
                                    case of dual/single mode for chip select0
                                    csn0. It is used both in Auto and Manual
                                    Mode. */
      __IOM unsigned int QSPI_D2_DATA_CSN0 : 1;                  /*!< [10..10] Value of SPI_IO2 in case of
                                     dual/single mode for chip select0 csn0. It
                                     is used both in Auto and Manual Mode. */
      __IOM unsigned int QSPI_D3_DATA_CSN0 : 1;                  /*!< [11..11] Value of SPI_IO3 in case of
                                     dual/single mode for chip select0 csn0. It
                                     is used both in Auto and Manual Mode */
      __IOM unsigned int QSPI_D2_OEN_CSN1 : 1;                   /*!< [12..12] Direction Control for
                                                SPI_IO2 in case of dual/single
                                                  mode for chip select1 csn1  */
      __IOM unsigned int QSPI_D3_OEN_CSN1 : 1;                   /*!< [13..13] Direction Control for
                                                SPI_IO3 in case of dual/single
                                                  mode for chip select1 csn1  */
      __IOM unsigned int QSPI_D2_DATA_CSN1 : 1;                  /*!< [14..14] Direction Control for
                                                SPI_IO3 in case of dual/single
                                                  mode for chip select1 csn1 */
      __IOM unsigned int QSPI_D3_DATA_CSN1 : 1;                  /*!< [15..15] Value of SPI_IO3 in case of
                                     dual/single mode for chip select1 csn1 */
      __IOM unsigned int QSPI_DATA_SAMPLE_EDGE : 1;              /*!< [16..16] Samples MISO data
                                                    on clock edges */
      __IOM unsigned int QSPI_CLK_MODE_CSN0 : 1;                 /*!< [17..17] QSPI Clock Mode    */
      __IOM unsigned int QSPI_CLK_MODE_CSN1 : 1;                 /*!< [18..18] QSPI Clock Mode    */
      __IOM unsigned int QSPI_CLK_MODE_CSN2 : 1;                 /*!< [19..19] QSPI Clock Mode    */
      __IOM unsigned int QSPI_CLK_MODE_CSN3 : 1;                 /*!< [20..20] QSPI Clock Mode    */
      __IOM unsigned int FLASH_AW_FIFO_LS_EN : 1;                /*!< [21..21] Qspi flash auto write fifo
                                       light sleep enable                    */
      __IOM unsigned int FLASH_SEC_AES_LS_EN : 1;                /*!< [22..22] Qspi flash auto write fifo
                                        light sleep enable                    */
      __IOM unsigned int RESERVED1 : 1;                          /*!< [23..23] reserved1 */
      __IOM unsigned int QSPI_D2_OEN_CSN2 : 1;                   /*!< [24..24] Direction Control for SPI_IO2 in
                                    case of dual/single mode for chip select2
                                    csn2 */
      __IOM unsigned int QSPI_D3_OEN_CSN2 : 1;                   /*!< [25..25] Direction Control for SPI_IO3 in
                                    case of dual/single mode for chip select2
                                    csn2 */
      __IOM unsigned int QSPI_D2_DATA_CSN2 : 1;                  /*!< [26..26] Value of SPI_IO2 in case of
                                     dual/single mode for chip select2 csn2 */
      __IOM unsigned int QSPI_D3_DATA_CSN2 : 1;                  /*!< [27..27] Value of SPI_IO3 in case of
                                     dual/single mode for chip select2 csn2 */
      __IOM unsigned int QSPI_D2_OEN_CSN3 : 1;                   /*!< [28..28] Direction Control for SPI_IO2 in
                                    case of dual/single mode for chip select3
                                    csn3 */
      __IOM unsigned int QSPI_D3_OEN_CSN3 : 1;                   /*!< [29..29] Direction Control for SPI_IO3 in
                                    case of dual/single mode for chip select3
                                    csn3 */
      __IOM unsigned int QSPI_D2_DATA_CSN3 : 1;                  /*!< [30..30] Value of SPI_IO2 in case of
                                     dual/single mode for chip select3 csn3 */
      __IOM unsigned int QSPI_D3_DATA_CSN3 : 1;                  /*!< [31..31] Value of SPI_IO3 in case of
                                     dual/single mode for chip select3 csn3 */
    } QSPI_BUS_MODE_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_CONFIG_1; /*!< (@ 0x00000008) QSPI Auto Controller
                                           Configuration 1 Register */

    struct {
      __IOM unsigned int QSPI_EXT_BYTE_MODE_CSN0 : 2;    /*!< [1..0] Mode of operation of QSPI in
                                           the extra byte phase */
      __IOM unsigned int QSPI_DUMMY_MODE_CSN0 : 2;       /*!< [3..2] Mode of operation of
                                                   QSPI in instruction phase */
      __IOM unsigned int QSPI_ADDR_MODE_CSN0 : 2;        /*!< [5..4] Mode of operation of
                                                   QSPI in instruction phase  */
      __IOM unsigned int QSPI_CMD_MODE_CSN0 : 2;         /*!< [7..6] Mode of operation of
                                                   QSPI in instruction phase   */
      __IOM unsigned int QSPI_DATA_MODE_CSN0 : 2;        /*!< [9..8] Mode of operation of
                                                   QSPI in DATA phase  */
      __IOM unsigned int QSPI_EXTRA_BYTE_CSN0 : 8;       /*!< [17..10] Value of the extra byte to be
                                        transmitted, if the extra byte mode is
                                        enabled */
      __IOM unsigned int QSPI_EXTRA_BYTE_EN_CSN0 : 2;    /*!< [19..18] Value of the extra byte to
                                           be transmitted, if the extra
                                           byte mode is enabled */
      __IOM unsigned int QSPI_WRAP_SIZE : 2;             /*!< [21..20] Qspi auto wrap size    */
      __IOM unsigned int RESERVED1 : 1;                  /*!< [22..22] reserved1         */
      __IOM unsigned int QSPI_PG_JUMP_CSN0 : 1;          /*!< [23..23] NONE */
      __IOM unsigned int QSPI_DUMMY_BYTES_INCR_CSN0 : 4; /*!< [27..24] Specifies the number of
                                           dummy bytes 0 to 7 for the selected
                                           SPI mode */
      __IOM unsigned int QSPI_DUMMY_BYTES_WRAP_CSN0 : 4; /*!< [31..28] Specifies the number of
                                           dummy bytes 0 to 7 for the selected
                                           SPI mode in case of wrap instruction
                                         */
    } QSPI_AUTO_CONFIG_1_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_CONFIG_2; /*!< (@ 0x0000000C) QSPI Auto Controller
                                           Configuration 2 Register */

    struct {
      __IOM unsigned int QSPI_RD_DATA_SWAP_AUTO_CSN0 : 1;         /*!< [0..0] NONE */
      __IOM unsigned int QSPI_ADR_SIZE_16_BIT_AUTO_MODE_CSN0 : 1; /*!< [1..1] NONE */
      __IOM unsigned int QSPI_CONTI_RD_EN_CSN0 : 1;               /*!< [2..2] NONE    */
      __IOM unsigned int DUMMY_BYTES_WR_RD_CSN0 : 1;              /*!< [3..3] Dummy bytes to the model to be
                                          read or to be write                 */
      __IOM unsigned int QSPI_DUMMY_BYTES_JMP_CSN : 4;            /*!< [7..4] Dummy cycles to be selected
                                            in case of JUMP */
      __IOM unsigned int QSPI_RD_INST_CSN0 : 8;                   /*!< [15..8] Read instruction to be used for
                                     the selected SPI modes and when wrap */
      __IOM unsigned int QSPI_RD_WRAP_INT_CSN0 : 8;               /*!< [23..16] Read instruction
                                                    to be used, when wrap mode is
                                                    supported by QSPI flash */
      __IOM unsigned int QSPI_PG_JUMP_INST_CSN0 : 8;              /*!< [31..24] Read instruction to be used,
                                          when Page jump is to be used */
    } QSPI_AUTO_CONFIG_2_b;
  };

  union {
    __IOM unsigned int QSPI_MANUAL_CONFIG1; /*!< (@ 0x00000010) QSPI Manual
                                            Configuration 1 Register */

    struct {
      __IOM unsigned int QSPI_MANUAL_CSN : 1;                  /*!< [0..0] SPI CS in manual mode */
      __IOM unsigned int QSPI_MANUAL_WR : 1;                   /*!< [1..1] Write enable for manual
                                              mode when CS is low  */
      __IOM unsigned int QSPI_MANUAL_RD : 1;                   /*!< [2..2] Read enable for manual mode
                                             when CS is low */
      __IOM unsigned int QSPI_MANUAL_RD_CNT : 10;              /*!< [12..3] Indicates total number of bytes
                                       to be read along with 31:27 bits of this
                                       register. Maximum length supported is
                                           32k bytes */
      __IOM unsigned int QSPI_MANUAL_CSN_SELECT : 2;           /*!< [14..13] Indicates which
                                                     CSn is valid */
      __IOM unsigned int RESERVED1 : 4;                        /*!< [18..15] reserved1              */
      __IOM unsigned int QSPI_MANUAL_SIZE_FRM_REG : 2;         /*!< [20..19] Manual reads and manual
                                            writes follow this size */
      __IOM unsigned int TAKE_QSPI_MANUAL_WR_SIZE_FRM_REG : 1; /*!< [21..21] NONE */
      __IOM unsigned int QSPI_FULL_DUPLEX_EN : 1;              /*!< [22..22] Full duplex mode enable.  */
      __IOM unsigned int RESERVED2 : 2;                        /*!< [24..23] reserved2 */
      __IOM unsigned int HW_CTRLD_QSPI_MODE_CTRL : 1;          /*!< [25..25] Hardware controlled qspi
                                           mode in between AUTO and manual */
      __IOM unsigned int QSPI_MANUAL_QSPI_MODE : 1;            /*!< [26..26] Internally the priority is
                                         given to manual mode                  */
      __IOM unsigned int QSPI_MANUAL_RD_CNT1 : 5;              /*!< [31..27] Indicates total
                                                  number of bytes or bits */
    } QSPI_MANUAL_CONFIG1_b;
  };

  union {
    __IOM unsigned int QSPI_MANUAL_CONFIG2; /*!< (@ 0x00000014) QSPI Manual
                                            Configuration 2 Register */

    struct {
      __IOM unsigned int QSPI_WR_DATA_SWAP_MNL_CSN0 : 1;          /*!< [0..0] Swap the write data inside
                                              the QSPI controller it-self */
      __IOM unsigned int QSPI_WR_DATA_SWAP_MNL_CSN1 : 1;          /*!< [1..1] Swap the write data inside
                                              the QSPI controller it-self. */
      __IOM unsigned int QSPI_WR_DATA_SWAP_MNL_CSN2 : 1;          /*!< [2..2] Swap the write data inside
                                              the QSPI controller itself. */
      __IOM unsigned int QSPI_WR_DATA_SWAP_MNL_CSN3 : 1;          /*!< [3..3] Swap the write data inside
                                              the QSPI controller itself. */
      __IOM unsigned int QSPI_RD_DATA_SWAP_MNL_CSN0 : 1;          /*!< [4..4] Swap the read data inside
                                              the QSPIcontroller it self. */
      __IOM unsigned int QSPI_RD_DATA_SWAP_MNL_CSN1 : 1;          /*!< [5..5] Swap the read data inside
                                              the QSPIcontroller itself. */
      __IOM unsigned int QSPI_RD_DATA_SWAP_MNL_CSN2 : 1;          /*!< [6..6] Swap the read data inside
                                              the QSPIcontroller it-self */
      __IOM unsigned int QSPI_RD_DATA_SWAP_MNL_CSN3 : 1;          /*!< [7..7] Swap the read data inside
                                              the QSPIcontroller itself */
      __IOM unsigned int QSPI_MAN_MODE_CONF_CSN1 : 2;             /*!< [9..8] Configures the QSPI flash for
                                           Single/Dual/Quad mode operation in
                                           manual mode for chip select1 csn1 */
      __IOM unsigned int QSPI_MAN_MODE_CONF_CSN2 : 2;             /*!< [11..10] Configures the QSPI flash
                                           for Single or Dual or Quad mode
                                           operation in manual mode for chip
                                           select2 csn2                       */
      __IOM unsigned int QSPI_MAN_MODE_CONF_CSN3 : 2;             /*!< [13..12] Configures the QSPI flash
                                           for Single or Dual or Quad mode
                                           operation in manual mode for chip
                                           select3 csn3                       */
      __IOM unsigned int LOOP_BACK_EN : 1;                        /*!< [14..14] Internal loop back test mode. */
      __IOM unsigned int QSPI_MANUAL_DDR_PHASE : 1;               /*!< [15..15] DDR operations can be
                                              performed even in manual mode      */
      __IOM unsigned int QSPI_DDR_CLK_EN : 1;                     /*!< [16..16] DDR operations can be
                                              performed even in manual mode */
      __IOM unsigned int RESERVED1 : 1;                           /*!< [17..17] reserved1       */
      __IOM unsigned int QSPI_RD_DATA_SWAP_WORD_LVL_MNL_CSN0 : 1; /*!< [18..18] Set this bit
                                                       for read data byte
                                                       swapping within the word.
                                                       It is valid only for octa
                                                       ddr mode. It is valid
                                                        for csn0. */
      __IOM unsigned int QSPI_RD_DATA_SWAP_WORD_LVL_MNL_CSN1 : 1; /*!< [19..19] Set this bit
                                                       for read data byte
                                                       swapping within the word.
                                                       It is valid only for octa
                                                       ddr mode. It is valid
                                                        for csn1. */
      __IOM unsigned int QSPI_RD_DATA_SWAP_WORD_LVL_MNL_CSN2 : 1; /*!< [20..20] Set this bit
                                                       for read data byte
                                                       swapping within the word.
                                                       It is valid only for octa
                                                       ddr mode. It is valid
                                                        for csn2. */
      __IOM unsigned int QSPI_WR_DATA_SWAP_WORD_LVL_MNL_CSN0 : 1; /*!< [21..21] Set this bit
                                                       for write data byte
                                                       swapping within the word.
                                                       It is valid only for octa
                                                       ddr mode. It is valid
                                                        for csn0. */
      __IOM unsigned int QSPI_WR_DATA_SWAP_WORD_LVL_MNL_CSN1 : 1; /*!< [22..22] Set this bit
                                                       for write data byte
                                                       swapping within the word.
                                                       It is valid only for octa
                                                       ddr mode. It is valid
                                                        for csn1. */
      __IOM unsigned int QSPI_WR_DATA_SWAP_WORD_LVL_MNL_CSN2 : 1; /*!< [23..23] Set this bit
                                                       for write data byte
                                                       swapping within the word.
                                                       It is valid only for octa
                                                       ddr mode. It is valid
                                                        for csn2. */
      __IOM unsigned int QSPI_WR_DATA_SWAP_WORD_LVL_MNL_CSN3 : 1; /*!< [24..24] Set this bit
                                                       for write data byte
                                                       swapping within the word.
                                                       It is valid only for octa
                                                       ddr mode. It is valid
                                                        for csn3. */
      __IOM unsigned int QSPI_MANUAL_DUMMY_BYTE_OR_BIT_MODE : 1;  /*!< [25..25] Indicates
                                                      qspi_manual_rd_cnt values
                                                      are dummy bytes
                                                        or bits in manual mode.
                                                    */
      __IOM unsigned int RESERVED2 : 6;                           /*!< [31..26] reserved2               */
    } QSPI_MANUAL_CONFIG2_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int QSPI_FIFO_THRLD; /*!< (@ 0x0000001C) QSPI FIFO Threshold Register */

    struct {
      __IOM unsigned int FIFO_AEMPTY_THRLD : 4; /*!< [3..0] FIFO almost empty threshold */
      __IOM unsigned int FIFO_AFULL_THRLD : 4;  /*!< [7..4] FIFO almost full threshold       */
      __IOM unsigned int WFIFO_RESET : 1;       /*!< [8..8] Write fifo reset */
      __IOM unsigned int RFIFO_RESET : 1;       /*!< [9..9] Read fifo reset */
      __IOM unsigned int RESERVED1 : 22;        /*!< [31..10] reserved1  */
    } QSPI_FIFO_THRLD_b;
  };

  union {
    __IM unsigned int QSPI_MANUAL_STATUS; /*!< (@ 0x00000020) QSPI Manual Status Register */

    struct {
      __IM unsigned int QSPI_BUSY : 1;                   /*!< [0..0] State of Manual mode.          */
      __IM unsigned int FIFO_FULL_WFIFO_S : 1;           /*!< [1..1] Status indication for
                                                Wfifo in manual mode  */
      __IM unsigned int FIFO_AFULL_WFIFO_S : 1;          /*!< [2..2] Status indication for
                                                Wfifo in manual mode */
      __IM unsigned int FIFO_EMPTY_WFIFO : 1;            /*!< [3..3] Status indication for
                                                Wfifo in manual mode   */
      __IM unsigned int FIFO_AEMPTY_WFIFO : 1;           /*!< [4..4] Status indication for
                                                Wfifo in manual mode  */
      __IM unsigned int FIFO_FULL_RFIFO : 1;             /*!< [5..5] Status indication for Rfifo
                                             in manual mode */
      __IM unsigned int FIFO_AFULL_RFIFO : 1;            /*!< [6..6] Status indication for
                                                 Rfifo in manual mode    */
      __IM unsigned int FIFO_EMPTY_RFIFO_S : 1;          /*!< [7..7] Status indication for
                                                 Rfifo in manual mode  */
      __IM unsigned int FIFO_AEMPTY_RFIFO_S : 1;         /*!< [8..8] Status indication for
                                                 Rfifo in manual mode */
      __IM unsigned int GSPI_MANUAL_RD_CNT : 1;          /*!< [9..9] This is a result of 10
                                                 bits ORing counter  */
      __IM unsigned int AUTO_MODE_FSM_IDLE_SCLK : 1;     /*!< [10..10] Auto mode idle signal to
                                           track auto controller is busy
                                            or idle. */
      __IM unsigned int QSPI_AUTO_MODE : 1;              /*!< [11..11] QSPI controller status. */
      __IM unsigned int QSPI_AUTO_MODE_FRM_REG_SCLK : 1; /*!< [12..12] QSPI auto mode status.
                                                Valid only when
                                                HW_CTRLD_QSPI_MODE_CTRL  is zero.
                                              */
      __IM unsigned int HW_CTRLD_MODE_SCLK : 1;          /*!< [13..13] QSPI mode status in
                                                HW_CTRLD_MODE */
      __IM unsigned int HW_CTRLD_MODE_CTRL_SCLK : 1;     /*!< [14..14] HW_CTRLD_MODE status */
      __IM unsigned int AW_CTRL_BUSY : 1;                /*!< [15..15] Auto write busy indication.         */
      __IM unsigned int RESERVED1 : 16;                  /*!< [31..16] reserved1 */
    } QSPI_MANUAL_STATUS_b;
  };

  union {
    __IOM unsigned int QSPI_INTR_MASK; /*!< (@ 0x00000024) QSPI Interrupt Mask Register */

    struct {
      __IOM unsigned int QSPI_INTR_MASK : 1;                    /*!< [0..0] Interrupt Status bit */
      __IOM unsigned int FIFO_AEMPTY_RFIFO_MASK : 1;            /*!< [1..1] NONE */
      __IOM unsigned int FIFO_AFULL_RFIFO_MASK : 1;             /*!< [2..2] NONE  */
      __IOM unsigned int FIFO_AEMPTY_WFIFO_MASK : 1;            /*!< [3..3] NONE */
      __IOM unsigned int FIFO_AFULL_WFIFO_MASK : 1;             /*!< [4..4] NONE  */
      __IOM unsigned int FIFO_FULL_WFIFO_MASK : 1;              /*!< [5..5] NONE   */
      __IOM unsigned int FIFO_EMPTY_RFIFO_MASK : 1;             /*!< [6..6] NONE  */
      __IOM unsigned int AHB_AUTO_WRITE_INTR_MASK : 1;          /*!< [7..7] Rising interrupt for any
                                           auto write operation on AHB
                                           bus. This bit is a mask for this
                                           interrupt */
      __IOM unsigned int QSPI_AUTO_BASE_ADDR_ERR_INTR_MASK : 1; /*!< [8..8] Rising interrupt
                                                     when no csn is selected
                                                     using programmable
                                                        auto base address. This
                                                     bit is a mask for this
                                                     interrupt. */
      __IOM unsigned int M4QSPI_MANUAL_BLOCKED_INTR_MASK : 1;   /*!< [9..9] Rising interrupt when
                                                   M4 QSPI tries to do manual
                                                   mode transactions in Common
                                                   flash mode (3).  This bit is a
                                                   mask for this interrupt. */
      __IOM unsigned int M4_AUTO_READ_OUT_range_intr_mask : 1;  /*!< [10..10] Rising interrupt
                                                    when M4 QSPI tries to read NWP
                                                    locations in Common flash
                                                    mode (3).  This bit is a mask
                                                    for this interrupt. */
      __IOM unsigned int RESERVED1 : 21;                        /*!< [31..11] reserved1            */
    } QSPI_INTR_MASK_b;
  };

  union {
    __IOM unsigned int QSPI_INTR_UNMASK; /*!< (@ 0x00000028) QSPI Interrupt Unmask
                                         Register                             */

    struct {
      __IOM unsigned int QSPI_INTR_UNMASK : 1;                    /*!< [0..0] Interrupt Status bit */
      __IOM unsigned int FIFO_AEMPTY_RFIFO_UN : 1;                /*!< [1..1] NONE     */
      __IOM unsigned int FIFO_AFULL_RFIFO_UNMASK : 1;             /*!< [2..2] NONE  */
      __IOM unsigned int FIFO_AEMPTY_WFIFO_UNMASK : 1;            /*!< [3..3] NONE */
      __IOM unsigned int FIFO_AFULL_WFIFO_UNMASK : 1;             /*!< [4..4] NONE  */
      __IOM unsigned int FIFO_FULL_WFIFO_UNMASK : 1;              /*!< [5..5] NONE   */
      __IOM unsigned int FIFO_EMPTY_RFIFO_UNMASK : 1;             /*!< [6..6] NONE  */
      __IOM unsigned int AHB_AUTO_WRITE_INTR_UNMASK : 1;          /*!< [7..7] Rising interrupt for any
                                           auto write operation on AHB
                                           bus. This bit is a unmask for this
                                           interrupt. */
      __IOM unsigned int QSPI_AUTO_BASE_ADDR_ERR_INTR_UNMASK : 1; /*!< [8..8] Rising interrupt
                                                       when M4 QSPI tries to do
                                                       manual mode transactions
                                                       in Common flash mode (3).
                                                       This bit is a unmask for
                                                       this interrupt. */
      __IOM unsigned int M4QSPI_MANUAL_BLOCKED_INTR_UNMASK : 1;   /*!< [9..9] Rising interrupt
                                                     when M4 QSPI tries to do
                                                     manual mode transactions in
                                                     Common flash mode (3).  This
                                                     bit is a unmask for this
                                                     interrupt. */
      __IOM unsigned int M4_AUTO_READ_OUT_RANGE_INTR_UNMASK : 1;  /*!< [10..10] Rising interrupt
                                                      when M4 QSPI tries to read
                                                      NWP locations in Common
                                                      flash mode (3).  This bit
                                                      is a unmask for this
                                                        interrupt. */
      __IOM unsigned int RESERVED1 : 21;                          /*!< [31..11] reserved1              */
    } QSPI_INTR_UNMASK_b;
  };

  union {
    __IM unsigned int QSPI_INTR_STS; /*!< (@ 0x0000002C) QSPI Interrupt Status Register */

    struct {
      __IM unsigned int QSPI_INTR_LVL : 1;                    /*!< [0..0] Interrupt Status bit */
      __IM unsigned int FIFO_AEMPTY_RFIFO_LVL : 1;            /*!< [1..1] NONE */
      __IM unsigned int FIFO_AFULL_RFIFO_LVL : 1;             /*!< [2..2] NONE  */
      __IM unsigned int FIFO_AEMPTY_WFIFO_LVL : 1;            /*!< [3..3] NONE */
      __IM unsigned int FIFO_AFULL_WFIFO_LVL : 1;             /*!< [4..4] NONE  */
      __IM unsigned int FIFO_FULL_WFIFO_LVL : 1;              /*!< [5..5] NONE   */
      __IM unsigned int FIFO_EMPTY_RFIFO_LVL : 1;             /*!< [6..6] NONE  */
      __IM unsigned int AHB_AUTO_WRITE_INTR_LEV : 1;          /*!< [7..7] rising interrupt for any auto
                                           write operation on AHB bus. */
      __IM unsigned int QSPI_AUTO_BASE_ADDR_ERR_INTR_LVL : 1; /*!< [8..8] Rising interrupt
                                                    when no csn is selected using
                                                    programmable auto base
                                                    address. */
      __IM unsigned int M4QSPI_MANUAL_BLOCKED_LVL : 1;        /*!< [9..9] Rising interrupt when M4
                                            QSPI tries to do manual mode
                                            transactions in Common flash mode
                                            (3). */
      __IM unsigned int M4_AUTO_READ_OUT_RANGE_LVL : 1;       /*!< [10..10] Rising interrupt when M4
                                              QSPI tries to read NWP locations
                                                        in Common flash mode (3).
                                            */
      __IM unsigned int RESERVED1 : 21;                       /*!< [31..11] reserved1       */
    } QSPI_INTR_STS_b;
  };

  union {
    __IOM unsigned int QSPI_INTR_ACK; /*!< (@ 0x00000030) QSPI Interrupt Acknowledge
                                      Register                        */

    struct {
      __OM unsigned int QSPI_INTR_ACK : 1;                    /*!< [0..0] Interrupt Status bit */
      __OM unsigned int FIFO_AEMPTY_RFIFO_ACK : 1;            /*!< [1..1] NONE */
      __OM unsigned int FIFO_AFULL_RFIFO_ACK : 1;             /*!< [2..2] NONE  */
      __OM unsigned int FIFO_AEMPTY_WFIFO_ACK : 1;            /*!< [3..3] NONE */
      __OM unsigned int FIFO_AFULL_WFIFO_ACK : 1;             /*!< [4..4] NONE  */
      __OM unsigned int FIFO_FULL_WFIFO_ACK : 1;              /*!< [5..5] NONE   */
      __OM unsigned int FIFO_EMPTY_RFIFO_ACK : 1;             /*!< [6..6] NONE  */
      __OM unsigned int AHB_AUTO_WRITE_INTR_ACK : 1;          /*!< [7..7] Rising interrupt for any auto
                                           write operation on AHB bus. This bit
                                           is an ack for this interrupt. */
      __OM unsigned int QSPI_AUTO_BASE_ADDR_ERR_INTR_ACK : 1; /*!< [8..8] Rising interrupt
                                                    when no csn is selected using
                                                    programmable auto base
                                                    address. This bit is an ack
                                                    for this interrupt. */
      __IOM unsigned int M4QSPI_MANUAL_BLOCKED_INTR_ACK : 1;  /*!< [9..9] Rising interrupt when
                                                  M4 QSPI tries to do manual mode
                                                        transactions in Common
                                                  flash mode (3).  This bit is an
                                                        ack for this interrupt.
                                                */
      __IOM unsigned int M4_AUTO_READ_OUT_RANGE_INTR_ACK : 1; /*!< [10..10] Rising interrupt
                                                   when M4 QSPI tries to read NWP
                                                   locations
                                                        in Common flash mode (3).
                                                   This bit is an ack for this
                                                   interrupt.          */
      __OM unsigned int RESERVED1 : 21;                       /*!< [31..11] reserved1            */
    } QSPI_INTR_ACK_b;
  };

  union {
    __IM unsigned int QSPI_STS_MC; /*!< (@ 0x00000034) QSPI State Machine Monitor
                                   Register                        */

    struct {
      __IM unsigned int BUS_CTRL_PSTATE : 4;     /*!< [3..0] Bus controller present state */
      __IM unsigned int AUTO_CTRL_PSTATE : 3;    /*!< [6..4] Auto controller present state */
      __IM unsigned int QSPI_MASTER_PSTATE : 3;  /*!< [9..7] Qspi master present state */
      __IM unsigned int QSPI_MANUAL_RD_CNT : 15; /*!< [24..10] Qspi manual read
                                                 counter value */
      __IM unsigned int RESERVED1 : 7;           /*!< [31..25] reserved1           */
    } QSPI_STS_MC_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_CONFIG_1_CSN1; /*!< (@ 0x00000038) QSPI Auto Controller
                                     Configuration 1 CSN1 Register         */

    struct {
      __IOM unsigned int QSPI_EXT_BYTE_MODE_CSN1 : 2;   /*!< [1..0] Mode of operation of QSPI in
                                           instruction phase. */
      __IOM unsigned int QSPI_DUMMY_MODE_CSN1 : 2;      /*!< [3..2] Mode of operation of
                                                   QSPI in instruction phase */
      __IOM unsigned int QSPI_ADDR_MODE_CSN1 : 2;       /*!< [5..4] Mode of operation of
                                                   QSPI in instruction phase.  */
      __IOM unsigned int QSPI_CMD_MODE_CSN1 : 2;        /*!< [7..6] Mode of operation of
                                                   QSPI in instruction phase.   */
      __IOM unsigned int QSPI_DATA_MODE_CSN1 : 2;       /*!< [9..8] Mode of operation of
                                                   QSPI in DATA phase.  */
      __IM unsigned int QSPI_EXTRA_BYTE_CSN1 : 8;       /*!< [17..10] Value of the extra byte to be
                                        transmitted, if the extra byte mode is
                                        enabled. */
      __IOM unsigned int QSPI_EXTRA_BYTE_EN_CSN1 : 2;   /*!< [19..18] Mode of operation of QSPI
                                             in DATA phase.   */
      __IOM unsigned int QSPI_WRAP_SIZE : 2;            /*!< [21..20] Qspi auto wrap size */
      __IOM unsigned int RESERVED1 : 1;                 /*!< [22..22] reserved1      */
      __OM unsigned int QSPI_PG_JUMP_CSN1 : 1;          /*!< [23..23] NONE */
      __IM unsigned int QSPI_DUMMY_BYTES_INCR_CSN1 : 4; /*!< [27..24] Specifies the number of
                                            dummy bytes 0 to 7 for the selected
                                            SPI mode. */
      __IM unsigned int QSPI_DUMMY_BYTES_WRAP_CSN1 : 4; /*!< [31..28] Specifies the number of
                                            dummy bytes 0 to 7 for the selected
                                            SPI mode in case of wrap instruction.
                                          */
    } QSPI_AUTO_CONFIG_1_CSN1_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_CONFIG_2_CSN1_REG; /*!< (@ 0x0000003C) QSPI Auto Controller
                                         Configuration 2 CSN1 Register         */

    struct {
      __IOM unsigned int QSPI_RD_SWAP_AUTO_CSN1 : 1;             /*!< [0..0] Swap the read data from the
                                          flash in byte order for chip select1
                                          csn1 in auto mode. */
      __IOM unsigned int QSPI_ADR_SIZE_16BIT_AUTO_MODE_CSN1 : 1; /*!< [1..1] NONE */
      __IOM unsigned int QSPI_CONTI_RD_EN_CSN1 : 1;              /*!< [2..2] Continuous read
                                                      enable bit.   */
      __IOM unsigned int DUMMY_BYTES_WR_RD : 1;                  /*!< [3..3] Dummy bytes to the model
                                                to be read or to be write. */
      __IOM unsigned int QSPI_DUMMY_BYTES_JMP_CSN1 : 4;          /*!< [7..4] Dummy cycles to be selected
                                             in case of JUMP */
      __IOM unsigned int QSPI_RD_INST_CSN1 : 8;                  /*!< [15..8] Read instruction to be
                                                used for the selected SPI modes
                                                 and when wrap is not needed or
                                                supported */
      __IOM unsigned int QSPI_RD_WRAP_INST_CSN1 : 8;             /*!< [23..16] Read instruction to be used
                                          for the selected SPI modes and when
                                          wrap is not needed or supported */
      __IOM unsigned int QSPI_PG_JMP_INST_CSN1 : 8;              /*!< [31..24] Read instruction to be used,
                                         when Page jump is to be used. */
    } QSPI_AUTO_CONFIG_2_CSN1_REG_b;
  };
  __IOM unsigned int QSPI_MANUAL_RDWR_FIFO[16]; /*!< (@ 0x00000040) QSPI FIFOs */

  union {
    __IOM unsigned int QSPI_MANUAL_WRITE_DATA2; /*!< (@ 0x00000080) QSPI Manual
                                                Write Data 2 Register */

    struct {
      __IOM unsigned int QSPI_MANUAL_WRITE_DATA2 : 5; /*!< [4..0] Number of bits to be written
                                           in write mode */
      __IOM unsigned int RESERVED1 : 2;               /*!< [6..5] reserved1    */
      __IOM unsigned int USE_PREV_LENGTH : 1;         /*!< [7..7] Use previous length. */
      __IOM unsigned int QSPI_CLK_ENABLE_HCLK : 1;    /*!< [8..8] reserved2 */
      __IOM unsigned int RESERVED2 : 23;              /*!< [31..9] reserved2           */
    } QSPI_MANUAL_WRITE_DATA2_b;
  };
  __IM unsigned int RESERVED1[3];

  union {
    __IOM unsigned int QSPI_AUTO_CONFIG_3_CSN0_REG; /*!< (@ 0x00000090) QSPI Auto Controller
                                         Configuration 3 CSN0 Register         */

    struct {
      __IOM unsigned int QSPI_DUMMY_BYTE_OR_BIT_CSN0 : 1;          /*!< [0..0] Indicates all above
                                           mention values are dummy bytes or
                                           bits in auto mode. */
      __IOM unsigned int QSPI_DUMMY_BYTES_INCR_CSN0 : 4;           /*!< [4..1] Specifies the number of
                                          dummy bytes for the selected
                                          SPI mode. It contains MS nibble for
                                          byte. */
      __IOM unsigned int QSPI_DUMMY_BYTES_WRAP_CSN0 : 4;           /*!< [8..5] Specifies the number of
                                          dummy bytes for the selected SPI mode
                                          in case of wrap instruction. It
                                          contains MS nibble for byte. */
      __IOM unsigned int RESERVED1 : 3;                            /*!< [11..9] reserved1       */
      __IOM unsigned int QSPI_DDR_CMD_MODE_CSN0 : 1;               /*!< [12..12] DDR Command mode */
      __IOM unsigned int QSPI_DDR_ADDR_MODE_CSN0 : 1;              /*!< [13..13] DDR Address mode */
      __IOM unsigned int QSPI_DDR_DUMMY_MODE_CSN0 : 1;             /*!< [14..14] DDR Address mode */
      __IOM unsigned int QSPI_DDR_EXTRA_MODE_CSN0 : 1;             /*!< [15..15] DDR Address mode */
      __IOM unsigned int QSPI_DDR_DATA_MODE_CSN0 : 1;              /*!< [16..16] DDR Address mode */
      __IOM unsigned int QSPI_AUTO_DDR_CMD_MODE_CSN0 : 1;          /*!< [17..17] DDR data mode. */
      __IOM unsigned int QSPI_CMD_SIZE_16BIT_CSN0 : 1;             /*!< [18..18] Enable for 16 read cmd
                                            size for csn0. */
      __IOM unsigned int QSPI_ADR_SIZE_32BIT_AUTO_MODE : 1;        /*!< [19..19] 32 bit addressing
                                                 support enable. */
      __IOM unsigned int QSPI_RD_DATA_SWAP_WORD_LVL_AUTO_CSN0 : 1; /*!< [20..20] Rd data swap
                                                        at word level in auto
                                                        mode for csn0. It is
                                                        valid for octa mode. */
      __IOM unsigned int RESERVED3 : 3;                            /*!< [23..21] reserved3                 */
      __IOM unsigned int QSPI_RD_INST_CSN0_MSB : 8;                /*!< [31..24] Read instruction MS byte to
                                         be used the selected SPI
                                           modes and when wrap is not needed or
                                         supported.                           */
    } QSPI_AUTO_CONFIG_3_CSN0_REG_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_CONFIG_3_CSN1_REG; /*!< (@ 0x00000094) QSPI Auto Controller
                                         Configuration 3 CSN1 Register         */

    struct {
      __IOM unsigned int QSPI_DUMMY_BYTE_OR_BIT_CSN1 : 1;          /*!< [0..0] Indicates all above
                                           mention values are dummy bytes or
                                           bits in auto mode. */
      __IOM unsigned int QSPI_DUMMY_BYTES_INCR_CSN1 : 4;           /*!< [4..1] Specifies the number of
                                          dummy bytes for the selected
                                          SPI mode. It contains MS nibble for
                                          byte. */
      __IOM unsigned int QSPI_DUMMY_BYTES_WRAP_CSN1 : 4;           /*!< [8..5] Specifies the number of
                                          dummy bytes for the selected SPI mode
                                          in case of wrap instruction. It
                                          contains MS nibble for byte. */
      __IOM unsigned int RESERVED1 : 3;                            /*!< [11..9] reserved1       */
      __IOM unsigned int QSPI_DDR_CMD_MODE_CSN1 : 1;               /*!< [12..12] DDR Command mode */
      __IOM unsigned int QSPI_DDR_ADDR_MODE_CSN1 : 1;              /*!< [13..13] DDR Address mode */
      __IOM unsigned int QSPI_DDR_DUMMY_MODE_CSN1 : 1;             /*!< [14..14] DDR Address mode */
      __IOM unsigned int QSPI_DDR_EXTRA_MODE_CSN1 : 1;             /*!< [15..15] DDR Address mode */
      __IOM unsigned int QSPI_DDR_DATA_MODE_CSN1 : 1;              /*!< [16..16] DDR Address mode */
      __IOM unsigned int QSPI_AUTO_DDR_CMD_MODE_CSN1 : 1;          /*!< [17..17] DDR data mode. */
      __IOM unsigned int QSPI_CMD_SIZE_16BIT_CSN1 : 1;             /*!< [18..18] Enable for 16 read cmd
                                            size for csn1. */
      __IOM unsigned int RESERVED3 : 1;                            /*!< [19..19] RESERVED3     */
      __IOM unsigned int QSPI_RD_DATA_SWAP_WORD_LVL_AUTO_CSN1 : 1; /*!< [20..20] Rd data swap
                                                        at word level in auto
                                                        mode for csn1. It is
                                                        valid for octa mode. */
      __IOM unsigned int RESERVED4 : 3;                            /*!< [23..21] reserved4                 */
      __IOM unsigned int QSPI_RD_INST_CSN1_MSB : 8;                /*!< [31..24] Read instruction MS byte to
                                         be used the selected SPI
                                           modes and when wrap is not needed or
                                         supported.                           */
    } QSPI_AUTO_CONFIG_3_CSN1_REG_b;
  };
  __IM unsigned int RESERVED2[2];

  union {
    __IOM unsigned int QSPI_AUTO_BASE_ADDR_CSN0; /*!< (@ 0x000000A0) none */

    struct {
      __IOM unsigned int QSPI_AUTO_BASE_ADDR_CSN0 : 32; /*!< [31..0] Holds the 32 bit base
                                           address for select chip select0 in
                                           programmable auto csn mode.  It is
                                           valid only programmable
                                           auto csn mode is enabled. */
    } QSPI_AUTO_BASE_ADDR_CSN0_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_BASE_ADDR_CSN1; /*!< (@ 0x000000A4) none */

    struct {
      __IOM unsigned int QSPI_AUTO_BASE_ADDR_CSN1 : 32; /*!< [31..0] Holds the 32 bit base
                                           address for select chip select1 in
                                           programmable auto csn mode.  It is
                                           valid only programmable
                                           auto csn mode is enabled. */
    } QSPI_AUTO_BASE_ADDR_CSN1_b;
  };
  __IM unsigned int RESERVED3[2];

  union {
    __IOM unsigned int OCTASPI_BUS_CONTROLLER; /*!< (@ 0x000000B0) none */

    struct {
      __IOM unsigned int QSPI_D7TOD4_DATA_CSN0 : 4; /*!< [3..0] Value of SPI_IO7,6,5 and 4 in
                                         case of quad/dual/single mode for chip
                                         select1 (cs_n0). It is used both in Auto
                                           and Manual Mode. */
      __IOM unsigned int QSPI_D7TOD4_OEN_CSN0 : 4;  /*!< [7..4] Direction Control for SPI_IO
                                        7,6,5 and 4 in case of quad/dual/single
                                                        mode for chip select0
                                        (cs_n0).  It is used both in Auto and
                                        Manual Mode. */
      __IOM unsigned int QSPI_D7TOD4_DATA_CSN1 : 4; /*!< [11..8] Value of SPI_IO7,6,5 and 4 in
                                         case of quad/dual/single mode for chip
                                         select1 (cs_n1).  It is used both in
                                         Auto and Manual Mode. */
      __IOM unsigned int QSPI_D7TOD4_OEN_CSN1 : 4;  /*!< [15..12] Direction Control for SPI_IO
                                        7,6,5 and 4 in case of quad/dual/single
                                        mode for chip select1 (cs_n1).  It is
                                          used both in Auto and Manual Mode. */
      __IOM unsigned int QSPI_D7TOD4_DATA_CSN2 : 4; /*!< [19..16] Value of SPI_IO7,6,5 and 4 in
                                         case of quad/dual/single mode for chip
                                         select2 (cs_n2).  It is used both in
                                         Auto and Manual Mode. */
      __IOM unsigned int QSPI_D7TOD4_OEN_CSN2 : 4;  /*!< [23..20] Direction Control for SPI_IO
                                        7,6,5 and 4 in case of quad/dual/single
                                        mode for chip select2 (cs_n2).  It is
                                          used both in Auto and Manual Mode. */
      __IOM unsigned int QSPI_D7TOD4_DATA_CSN3 : 4; /*!< [27..24] Value of SPI_IO7,6,5 and 4 in
                                         case of quad/dual/single mode for chip
                                         select3 (cs_n3). It is used both in Auto
                                           and Manual Mode. */
      __IOM unsigned int QSPI_D7TOD4_OEN_CSN3 : 4;  /*!< [31..28] Direction Control for SPI_IO
                                        7,6,5 and 4 in case of quad/dual/single
                                        mode for chip select3 (cs_n3).  It is
                                          used both in Auto and Manual Mode. */
    } OCTASPI_BUS_CONTROLLER_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_BASE_ADDR_UNMASK_CSN0; /*!< (@ 0x000000B4) none */

    struct {
      __IOM unsigned int QSPI_AUTO_BASE_ADDR_UNMASK_CSN0 : 32; /*!< [31..0] Holds the 32 bit
                                                    base address unmask value for
                                                    select chip select0 in
                                                    programmable auto csn mode.
                                                    It is valid
                                                        only programmable auto
                                                    csn mode is enabled. */
    } QSPI_AUTO_BASE_ADDR_UNMASK_CSN0_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_BASE_ADDR_UNMASK_CSN1; /*!< (@ 0x000000B8) none */

    struct {
      __IOM unsigned int QSPI_AUTO_BASE_ADDR_UNMASK_CSN1 : 32; /*!< [31..0] Holds the 32 bit
                                                    base address unmask value for
                                                    select chip select1 in
                                                    programmable auto csn mode.
                                                    It is valid
                                                        only programmable auto
                                                    csn mode is enabled. */
    } QSPI_AUTO_BASE_ADDR_UNMASK_CSN1_b;
  };
  __IM unsigned int RESERVED4[2];

  union {
    __IOM unsigned int OCTASPI_BUS_CONTROLLER_2_REG; /*!< (@ 0x000000C4) none */

    struct {
      __IOM unsigned int SET_IP_MODE : 1;             /*!< [0..0] This bit enables the qspi
                                             interface pins into HiZ mode    */
      __IOM unsigned int AES_NONCE_INIT : 1;          /*!< [1..1] This bit enables the AES
                                             initialization with nonce */
      __IOM unsigned int AES_SEC_ENABLE : 1;          /*!< [2..2] This bit enables the AES
                                             security enable or not */
      __IOM unsigned int DUAL_MODE_EN : 1;            /*!< [3..3] Dual flash mode enable control. */
      __IOM unsigned int CSN0_2_CSN : 2;              /*!< [5..4] Map csn0 to the programmed csn. It is
                              valid for both manual and auto modes */
      __IOM unsigned int CSN1_2_CSN : 2;              /*!< [7..6] Map csn1 to the programmed csn. It is
                              valid for both manual and auto modes */
      __IOM unsigned int CSN2_2_CSN : 2;              /*!< [9..8] Map csn2 to the programmed csn. It is
                              valid for both manual and auto modes */
      __IOM unsigned int CSN3_2_CSN : 2;              /*!< [11..10] Map csn3 to the programmed csn. It is
                              valid for both manual and auto modes */
      __IOM unsigned int AES_SEC_ENABLE_SG1 : 1;      /*!< [12..12] This bit enables the AES
                                      security enable or not for segment 1 */
      __IOM unsigned int AES_SEC_ENABLE_SG2 : 1;      /*!< [13..13] This bit enables the AES
                                      security enable or not for segment 2 */
      __IOM unsigned int AES_SEC_ENABLE_SG3 : 1;      /*!< [14..14] This bit enables the AES
                                      security enable or not for segment 3 */
      __IOM unsigned int AES_SEC_ENABLE_SG4 : 1;      /*!< [15..15] This bit enables the AES
                                      security enable or not for segment 4 */
      __IOM unsigned int DUAL_MODE_SWAP_LINES : 1;    /*!< [16..16] This bit controls the 8 lines
                                        of qspi with 4 bit swap manner */
      __IOM unsigned int AUTO_MODE_IN_DEFAULT_EN : 1; /*!< [17..17] Qspi works in auto mode if
                                           set this is bit by default. */
      __IOM unsigned int OTP_KEY_LOAD : 1;            /*!< [18..18] Enable to load key from OTP/KH */
      __IOM unsigned int DUAL_STAGE_EN_MANUAL : 1;    /*!< [19..19] Dual stage en for
                                                   dual flash mode */
      __IOM unsigned int RESERVED2 : 12;              /*!< [31..20] reserved2           */
    } OCTASPI_BUS_CONTROLLER_2_REG_b;
  };

  union {
    __IOM unsigned int QSPI_AES_CONFIG; /*!< (@ 0x000000C8) QSPI AES CONFIG REG */

    struct {
      __IOM unsigned int QSPI_AES_MODE : 9;      /*!< [8..0] AES mode of decryption CTR/XTS */
      __IOM unsigned int QSPI_AES_DECKEYCAL : 1; /*!< [9..9] Enables pre-calculation of KEY
                                      before decryption operation         */
      __IOM unsigned int FLIP_KEY_FRM_REG : 1;   /*!< [10..10] writing 1 to this Flips the 32-bit
                                    endian key taken from kh */
      __IOM unsigned int FLIP_KEY_FRM_KH : 1;    /*!< [11..11] writing 1 to this Flips the 32-bit
                                   endian key taken from kh */
      __OM unsigned int QSPI_AES_SRST : 1;       /*!< [12..12] Synchronous soft reset for
                                           AES Module. Write only bit. Reading
                                           this bit gives alway 0 */
      __IOM unsigned int RESERVED1 : 19;         /*!< [31..13] reserved1   */
    } QSPI_AES_CONFIG_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY_IV_VALID; /*!< (@ 0x000000CC) QSPI AES KEYS and
                                              IVS VALID */

    struct {
      __IOM unsigned int QSPI_AES_KEY1_VALID : 4; /*!< [3..0] Write enables for AES KEY 1.
                                        Denotes which bytes of key1  is valid  */
      __IOM unsigned int RESERVED1 : 4;           /*!< [7..4] reserved1 */
      __IOM unsigned int QSPI_AES_KEY2_VALID : 4; /*!< [11..8] Write enables for AES KEY 2.
                                        Denotes which bytes of  key2 is valid  */
      __IOM unsigned int RESERVED2 : 4;           /*!< [15..12] reserved2 */
      __IOM unsigned int QSPI_AES_IV1_VALID : 4;  /*!< [19..16] Write enables for AES IV 1.
                                         Denotes which bytes of    IV1 is valid    */
      __IOM unsigned int RESERVED3 : 12;          /*!< [31..20] reserved3 */
    } QSPI_AES_KEY_IV_VALID_b;
  };

  union {
    __IM unsigned int QSPI_CMNFLASH_STS; /*!< (@ 0x000000D0) QSPI Common Flash Status */

    struct {
      __IM unsigned int QSPI_MANUAL_BLOCKED : 1; /*!< [0..0] 1 - Manual read/write transaction
                                       initiated is blocked.0- No manual
                                       transactions */
      __IM unsigned int AUTO_READ_OUT_RANGE : 1; /*!< [1..1] 1- Auto read transaction is out
                                       of M4 Address range 0- Auto read
                                       transaction is in Address range */
      __IM unsigned int QSPI_AUTO_RD_BUSY : 1;   /*!< [2..2] 1 - Auto read transactions in
                                        progress.0 - No Auto read    transactions    */
      __IM unsigned int RESERVED1 : 29;          /*!< [31..3] reserved1 */
    } QSPI_CMNFLASH_STS_b;
  };
  __IM unsigned int RESERVED5[4];

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_1; /*!< (@ 0x000000E4)
                                        QSPI_AES_SEC_SEG_LS_ADDR_1 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_1 : 32; /*!< [31..0] This register specifies
                                               the lower boundary address of
                                                        1st segment */
    } QSPI_AES_SEC_SEG_LS_ADDR_1_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_1; /*!< (@ 0x000000E8)
                                        QSPI_AES_SEC_SEG_MS_ADDR_1 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_1 : 32; /*!< [31..0] This register specifies
                                               the upper boundary address of
                                                        1st segment */
    } QSPI_AES_SEC_SEG_MS_ADDR_1_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_2; /*!< (@ 0x000000EC)
                                        QSPI_AES_SEC_SEG_LS_ADDR_2 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_2 : 32; /*!< [31..0] This register specifies
                                               the lower boundary address of
                                                        2nd segment */
    } QSPI_AES_SEC_SEG_LS_ADDR_2_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_2; /*!< (@ 0x000000F0)
                                        QSPI_AES_SEC_SEG_MS_ADDR_2 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_2 : 32; /*!< [31..0] This register specifies
                                               the upper boundary address of
                                                        2nd segment */
    } QSPI_AES_SEC_SEG_MS_ADDR_2_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_3; /*!< (@ 0x000000F4)
                                        QSPI_AES_SEC_SEG_LS_ADDR_3 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_3 : 32; /*!< [31..0] This register specifies
                                               the lower boundary address of
                                                        3rd segment */
    } QSPI_AES_SEC_SEG_LS_ADDR_3_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_3; /*!< (@ 0x000000F8)
                                        QSPI_AES_SEC_SEG_MS_ADDR_3 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_3 : 32; /*!< [31..0] This register specifies
                                               the upper boundary address of
                                                        3rd segment */
    } QSPI_AES_SEC_SEG_MS_ADDR_3_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_4; /*!< (@ 0x000000FC)
                                        QSPI_AES_SEC_SEG_LS_ADDR_4 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_LS_ADDR_4 : 32; /*!< [31..0] This register specifies
                                               the lower boundary address of
                                                        4th segment */
    } QSPI_AES_SEC_SEG_LS_ADDR_4_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_4; /*!< (@ 0x00000100)
                                        QSPI_AES_SEC_SEG_MS_ADDR_4 */

    struct {
      __IOM unsigned int QSPI_AES_SEC_SEG_MS_ADDR_4 : 32; /*!< [31..0] This register specifies
                                               the upper boundary address of
                                                        4th segment */
    } QSPI_AES_SEC_SEG_MS_ADDR_4_b;
  };

  union {
    __IOM unsigned int QSPI_SRAM_CTRL_CSN_REG[4]; /*!< (@ 0x00000104) QSPI SRAM CTRL CSN */

    struct {
      __IOM unsigned int BIT_8_MODE : 1;      /*!< [0..0] Flash 8bit (1 byte) boundary mode */
      __IOM unsigned int BYTE_32_MODE : 1;    /*!< [1..1] Flash 32 byte boundary mode */
      __IOM unsigned int ADDR_16BIT_MODE : 1; /*!< [2..2] Send only lower 16bits of
                                              Address enable. */
      __IOM unsigned int RESERVED1 : 5;       /*!< [7..3] reserved1       */
      __IOM unsigned int CMD_MODE : 2;        /*!< [9..8] writing cmd mode        */
      __IOM unsigned int ADDR_MODE : 2;       /*!< [11..10] writing address mode       */
      __IOM unsigned int DATA_MODE : 2;       /*!< [13..12] writing address mode       */
      __IOM unsigned int RESERVED2 : 2;       /*!< [15..14] reserved2       */
      __IOM unsigned int WR_CMD : 8;          /*!< [23..16] Command to be used for writing */
      __IOM unsigned int RESERVED3 : 8;       /*!< [31..24] reserved3 */
    } QSPI_SRAM_CTRL_CSN_REG_b[4];
  };
  __IM unsigned int RESERVED6[2];
  __IOM
  unsigned int SEMI_AUTO_MODE_ADDR_REG; /*!< (@ 0x0000011C) Byte address to read
                                            the data from flash in semi auto
                                            mode.  It is valid only semi auto
                                            mode enable bit is asserted */

  union {
    __IOM unsigned int SEMI_AUTO_MODE_CONFIG_REG; /*!< (@ 0x00000120) none */

    struct {
      __IOM unsigned int QSPI_SEMI_AUTO_BSIZE : 8; /*!< [7..0] This is burst size to read data
                                        from flash in semi auto mode */
      __IOM unsigned int QSPI_SEMI_AUTO_HSIZE : 2; /*!< [9..8] Indicates number of bytes valid
                                         in each transaction                */
      __IOM unsigned int RESERVED1 : 22;           /*!< [31..10] reserved1 */
    } SEMI_AUTO_MODE_CONFIG_REG_b;
  };

  union {
    __IOM unsigned int SEMI_AUTO_MODE_CONFIG2_REG; /*!< (@ 0x00000124) none */

    struct {
      __IOM unsigned int QSPI_SEMI_AUTO_RD_CNT : 12; /*!< [11..0] Total number of bytes to be
                                          read flash continuously from the
                                          address given by
                                          SEMI_AUTO_MODE_ADDR_REG */
      __IOM unsigned int QSPI_SEMI_AUTO_MODE_EN : 1; /*!< [12..12] Enable for semi auto mode
                                          read operation. Make sure manual mode
                                          read/write operation is completed
                                          before asserting this bit */
      __IOM unsigned int QSPI_SEMI_AUTO_RD_BUSY : 1; /*!< [13..13] Indicates status of semi
                                          auto mode read status. If it is high,
                                          semi auto mode read operation is
                                          progressing                  */
      __IOM unsigned int RESERVED1 : 18;             /*!< [31..14] reserved1  */
    } SEMI_AUTO_MODE_CONFIG2_REG_b;
  };

  union {
    __IOM unsigned int QSPI_BUS_MODE2_REG; /*!< (@ 0x00000128) none */

    struct {
      __IOM unsigned int PREFETCH_ENBLD_MSTR_ID : 4;      /*!< [3..0] Holds the programmable
                                          prefetch enabled AHB master ID. This is
                                          commonly used for enabling prefetch for
                                          icache master. */
      __IOM unsigned int PREFETCH_EN_FOR_ICACHE_MSTR : 1; /*!< [4..4] Prefetch enable for
                                               icache AHB master. */
      __IOM unsigned int RESERVED1 : 3;                   /*!< [7..5] Reserved for future use */
      __IOM
      unsigned int QSPI_PREFETCH_ENBLD_TRANS_BYTES : 8; /*!< [15..8] Programmable
                                                       prefetch enabled AHB
                                                       master transfer bytes.
                                                            Assume this is used
                                                       for icache and dma ahb
                                                       master access in auto
                                                       mode. */
      __IOM unsigned int RESERVED2 : 16;                /*!< [31..16] Reserved for future use */
    } QSPI_BUS_MODE2_REG_b;
  };

  union {
    __IOM unsigned int QSPI_AES_SEC_KEY_FRM_KH_REG; /*!< (@ 0x0000012C) none */

    struct {
      __OM unsigned int START_LOADING_SEC_KEY_FRM_KH : 1; /*!< [0..0] Start Security key
                                                loading from KH. */
      __IM unsigned int LOADING_SEC_KEY_FRM_KH : 1;       /*!< [1..1] Indicates security key loading
                                          status from KH.                     */
      __IOM unsigned int SEC_KEY_READING_INTERVAL : 4;    /*!< [5..2] Security key
                                                       reading interval */
      __IOM unsigned int RESERVED1 : 26;                  /*!< [31..6] Reserved for future use */
    } QSPI_AES_SEC_KEY_FRM_KH_REG_b;
  };

  union {
    __IOM unsigned int QSPI_AUTO_CONITNUE_FETCH_CTRL_REG; /*!< (@ 0x00000130) none */

    struct {
      __IOM unsigned int CONTINUE_FETCH_WAIT_TIMEOUT_VALUE_FRM_REG : 12; /*!< [11..0] Maximum
                                                        Continue fetch wait time
                                                        between two qspi auto
                                                        reads. */
      __IOM unsigned int CONTINUE_FETCH_EN : 1;                          /*!< [12..12] Continue fetch feature
                                                enable. */
      __IOM unsigned int RESERVED1 : 19;                                 /*!< [31..13] Reserved for future use */
    } QSPI_AUTO_CONITNUE_FETCH_CTRL_REG_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY1_0_3; /*!< (@ 0x00000134) QSPI_AES_KEY1_0_3 */

    struct {
      __IOM unsigned int QSPI_AES_KEY1_0_3 : 32; /*!< [31..0] To hold first 3-0 bytes of aes
                                      key1 as 0 referred as lsb in the key */
    } QSPI_AES_KEY1_0_3_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY1_4_7; /*!< (@ 0x00000138) QSPI_AES_KEY1_4_7 */

    struct {
      __IOM unsigned int QSPI_AES_KEY1_4_7 : 32; /*!< [31..0] To hold first 7-4 bytes of aes
                                      key1 as 0 referred as lsb */
    } QSPI_AES_KEY1_4_7_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY1_8_B; /*!< (@ 0x0000013C) QSPI_AES_KEY1_8_B */

    struct {
      __IOM unsigned int QSPI_AES_KEY1_8_B : 32; /*!< [31..0] To hold first 11-8 bytes of aes
                                      key1 as 0 referred as lsb */
    } QSPI_AES_KEY1_8_B_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY1_C_F; /*!< (@ 0x00000140) QSPI_AES_KEY1_C_F */

    struct {
      __IOM unsigned int QSPI_AES_KEY1_C_F : 32; /*!< [31..0] To hold first 11-8 bytes of aes
                                      key1 as 0 referred as lsb */
    } QSPI_AES_KEY1_C_F_b;
  };
  __IM unsigned int RESERVED7[4];

  union {
    __IOM unsigned int QSPI_AES_KEY2_0_3; /*!< (@ 0x00000154) QSPI_AES_KEY2_0_3 */

    struct {
      __IOM unsigned int QSPI_AES_KEY2_0_3 : 32; /*!< [31..0] To hold first 3-0 bytes of aes
                                      key2 as 0 referred as lsb in the key */
    } QSPI_AES_KEY2_0_3_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY2_4_7; /*!< (@ 0x00000158) QSPI_AES_KEY2_4_7 */

    struct {
      __IOM unsigned int QSPI_AES_KEY2_4_7 : 32; /*!< [31..0] To hold first 7-4 bytes of aes
                                      key2 as 0 referred as lsb */
    } QSPI_AES_KEY2_4_7_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY2_8_B; /*!< (@ 0x0000015C) QSPI_AES_KEY2_8_B */

    struct {
      __IOM unsigned int QSPI_AES_KEY2_8_B : 32; /*!< [31..0] To hold first 11-8 bytes of aes
                                      key2 as 0 referred as lsb */
    } QSPI_AES_KEY2_8_B_b;
  };

  union {
    __IOM unsigned int QSPI_AES_KEY2_C_F; /*!< (@ 0x00000160) QSPI_AES_KEY2_C_F */

    struct {
      __IOM unsigned int QSPI_AES_KEY2_C_F : 32; /*!< [31..0] To hold first 15-12 bytes of aes
                                      key2 as 0 referred as lsb */
    } QSPI_AES_KEY2_C_F_b;
  };
} QSPI_Type; /*!< Size = 356 (0x164) */

/* ================================================================================ */
/* ================                       BOD                      ================ */
/* ================================================================================ */

/**
  * @brief Brown-Out Detector (BOD) peripheral (BOD)
  */

typedef struct { /*!< (@ 0x2405A780) BOD Structure                                          */

  union {
    __IO unsigned int BOD_COMP_MODE_REG; /*!< (@ 0x2405A780) Comparator Mode Register                               */

    struct {
      __IO unsigned int
        CMP_OP_FILTER_EN : 1; /*!< [0..0] Enable signal for filter at comparator output                  */
      __IO unsigned int CMP_SLOT_VALUE : 16;    /*!< [1..16] Slot value after which comparator outputs are sampled
                                                           in auto mode                                                          */
      __IO unsigned int MANUAL_CMP_MUX_SEL : 3; /*!< [17..19] Selecting and fixing the inputs of comparator when
                                                           in manual mode                                                        */
      __IO unsigned int
        MANUAL_CMP_MODE_EN : 1; /*!< [20..20] Manual comparison mode enable                                */
      __IO unsigned int
        AUTO_CMP_MODE_EN : 1;           /*!< [21..21] Enable auto slotting of comparator inputs                    */
      __IO unsigned int RESERVED1 : 10; /*!< [22..31] reserved                                                     */
    } BOD_COMP_MODE_REG_b;              /*!< [32] BitSize                                                          */
  };

  union {
    __IO unsigned int BOD_COMP_SEL_REG; /*!< (@ 0x2405A784) Comparator Selection Register                          */

    struct {
      __I unsigned int RESERVED1 : 1; /*!< [0..0] Reserved                                                       */
      __IO unsigned int
        BATT_SCALE_FACTOR : 6; /*!< [1..6] Programmability for scaling vbatt                              */
      __IO unsigned int
        BANDGAP_SCALE_FACTOR : 4; /*!< [7..10] Programmability for scaling bandgap v1p2                      */
      __IO unsigned int
        CMP_5_POLARITY : 1; /*!< [11..11] Polarity of comparator 5                                     */
      __IO unsigned int
        CMP_4_POLARITY : 1; /*!< [12..12] Polarity of comparator 4                                     */
      __IO unsigned int
        CMP_3_POLARITY : 1; /*!< [13..13] Polarity of comparator 3                                     */
      __IO unsigned int
        CMP_2_POLARITY : 1; /*!< [14..14] Polarity of comparator 2                                     */
      __IO unsigned int
        CMP_1_POLARITY : 1;           /*!< [15..15] Polarity of comparator 1                                     */
      __IO unsigned int CMP_5_EN : 1; /*!< [16..16] Enable signal for BOD detection                              */
      __IO unsigned int CMP_4_EN : 1; /*!< [17..17] Enable signal for comparison 4                               */
      __IO unsigned int CMP_3_EN : 1; /*!< [18..18] Enables comparison 3                                         */
      __IO unsigned int CMP_2_EN : 1; /*!< [19..19] Enables comparison 2                                         */
      __IO unsigned int CMP_1_EN : 1; /*!< [20..20] Enables comparison 1                                         */
      __IO unsigned int
        BUTTON_WAKEUP_EN : 1;              /*!< [21..21] Enable button wakeup                                         */
      __IO unsigned int CMP_EN_REG_WR : 1; /*!< [22..22] Whenever any of the cmp_ens change or this register
                                                           of spi is written CMP_EN_REG_WR = 1                                   */
      __I unsigned int RESERVED2 : 9;      /*!< [23..31] Reserved                                                     */
    } BOD_COMP_SEL_REG_b;                  /*!< [32] BitSize                                                          */
  };

  union {
    __IO unsigned int BOD_BUTTON_REG; /*!< (@ 0x2405A788) Button Register                                        */

    struct {
      __IO unsigned int
        comparator_hysterisis : 2; /*!< [0..1] Comparator hysterisis                                          */
      __IO unsigned int
        BUTTON3_MIN_VALUE : 5; /*!< [2..6] Minimum value for button 3 detect                              */
      __IO unsigned int
        BUTTON2_MIN_VALUE : 5; /*!< [7..11] Minimum value for button 2 detect                             */
      __IO unsigned int
        BUTTON1_MIN_VALUE : 5; /*!< [12..16] Minimum threshold value to detect button 1 wakeup            */
      __IO unsigned int
        BUTTON_MAX_VALUE : 4; /*!< [17..20] Maximum range for each button wakeup detect                  */
      __I unsigned int
        SYNC_RESET_READ : 1;            /*!< [21..21] Read back synced reset with 32KHz fsm clock                  */
      __IO unsigned int RESERVED1 : 10; /*!< [22..31] reserved                                                     */
    } BOD_BUTTON_REG_b;                 /*!< [32] BitSize                                                          */
  };

  union {
    __IO unsigned int
      BOD_TEST_PG_VBATT_STATUS_REG; /*!< (@ 0x2405A78C) Test, Power Gate and VBATT Status Register             */

    struct {
      __I unsigned int VBATT_STATUS : 6; /*!< [0..5] Status of battery, 31 -> full, 0 -> low                        */
      __I unsigned int
        VBATT_STATUS_VALID : 1; /*!< [6..6] Valid signal for reading vbatt status                          */
      __O unsigned int
        CHECK_VBATT_STATUS : 1; /*!< [7..7] Pulse signal, combinational logic to check battery status      */
      __IO unsigned int
        PERIODIC_TRIGGER_EN : 1; /*!< [8..8] Periodic checking for battery status enable                    */
      __IO unsigned int
        BROWN_OUT_INTERRUPT_THRESHOLD : 5; /*!< [9..13] Threshold for brown out detection beyond which interrupt
                                                           is not given to NPSS                                                  */
      __IO unsigned int BLACKOUT_EN : 1;   /*!< [14..14] Unused                                                       */
      __IO unsigned int
        BOD_PWRGATE_EN_N_ULP_BUTTON_CALIB : 1; /*!< [15..15] Powergate enable signal for button calib and vbatt
                                                           status checking block                                                 */
      __I unsigned int BUTTON3_WAKEUP : 1; /*!< [16..16] Button 3 wakeup status                                       */
      __I unsigned int BUTTON2_WAKEUP : 1; /*!< [17..17] Button 2 wakeup status                                       */
      __I unsigned int BUTTON1_WAKEUP : 1; /*!< [18..18] Button 1 wakeup status                                       */
      __IO unsigned int BOD_TEST_SEL : 2;  /*!< [19..20] Select bits for test mux                                     */
      __IO unsigned int
        EN_BOD_TEST_MUX : 1;            /*!< [21..21] Enable test mux to connect to GPIO pad                       */
      __IO unsigned int RESERVED1 : 10; /*!< [22..31] reserved                                                     */
    } BOD_TEST_PG_VBATT_STATUS_REG_b;   /*!< [32] BitSize                                                          */
  };

  union {
    __IO unsigned int BOD_BUTTON_READ; /*!< (@ 0x2405A790) Button Read Register                                   */

    struct {
      __I unsigned int RESERVED1 : 16; /*!< [0..15] Reserved                                                      */
      __IO unsigned int
        READ_BUTTON_WORD : 6; /*!< [16..21] Word captured when button pressed                            */
    } BOD_BUTTON_READ_b;      /*!< [22] BitSize                                                          */
  };
} BOD_Type;

/* ===========================================================================================================================
  */
/* ================                                            CRC
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief CRC is used in all wireless communication as a first data integrity
  * check (CRC)
  */

typedef struct { /*!< (@ 0x45080000) CRC Structure */

  union {
    __IOM unsigned int CRC_GEN_CTRL_SET_REG; /*!< (@ 0x00000000) General control set
                                             register */

    struct {
      __IOM unsigned int SOFT_RST : 1;   /*!< [0..0] Soft reset. This clears the FIFO and settles
                            all the state machines to their IDLE */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use. */
    } CRC_GEN_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int CRC_GEN_CTRL_RESET; /*!< (@ 0x00000004) General control reset
                                           register */

    struct {
      __IOM unsigned int RESERVED1 : 32; /*!< [31..0] Reserved for future use. */
    } CRC_GEN_CTRL_RESET_b;
  };

  union {
    __IM unsigned int CRC_GEN_STS; /*!< (@ 0x00000008) General status register */

    struct {
      __IM unsigned int CALC_DONE : 1;          /*!< [0..0] When the computation of final CRC
                                       with the data out of fifo, this will get
                                       set to 1 otherwise 0 */
      __IM unsigned int DIN_NUM_BYTES_DONE : 1; /*!< [1..1] When number of bytes requested for
                                      computation of final CRC is read from fifo
                                      by internal FSM, this will get set to 1
                                      otherwise 0. */
      __IM unsigned int RESERVED1 : 30;         /*!< [31..2] Reserved for future use. */
    } CRC_GEN_STS_b;
  };

  union {
    __IOM unsigned int CRC_POLYNOMIAL; /*!< (@ 0x0000000C) This register holds the polynomial
                            with which the final CRC is computed. */

    struct {
      __IOM unsigned int POLYNOMIAL : 32; /*!< [31..0] Polynomial register. This register holds
                               the polynomial with which the final CRC is
                               computed.When write Polynomial will be
                               updated.When read read polynomial. */
    } CRC_POLYNOMIAL_b;
  };

  union {
    __IOM unsigned int CRC_POLYNOMIAL_CTRL_SET; /*!< (@ 0x00000010) Polynomial
                                                control set register */

    struct {
      __IOM unsigned int POLYNOMIAL_WIDTH_SET : 5; /*!< [4..0] Polynomial width set. Number of
                                         bits/width of the polynomial  has to be
                                         written here for the computation of final
                                         CRC.  If a new width has to be configured,
                                         clear the existing  length first by
                                         writing 0x1f in polynomial_ctrl_reset
                                         register.  When read, actual polynomial
                                         width is read.  */
      __IOM unsigned int RESERVED1 : 27;           /*!< [31..5] Reserved for future use. */
    } CRC_POLYNOMIAL_CTRL_SET_b;
  };

  union {
    __IOM unsigned int CRC_POLYNOMIAL_CTRL_RESET; /*!< (@ 0x00000014) Polynomial
                                                  control set register */

    struct {
      __IOM unsigned int POLYNOMIAL_WIDTH_SET : 5; /*!< [4..0] Polynomial width reset. If a new
                                        width has to be configured, clear the
                                        existing length first by writing 0x1f.
                                        When read, actual polynomial width is
                                        read. */
      __IOM unsigned int RESERVED1 : 27;           /*!< [31..5] Reserved for future use. */
    } CRC_POLYNOMIAL_CTRL_RESET_b;
  };

  union {
    __IOM unsigned int CRC_LFSR_INIT_VAL; /*!< (@ 0x00000018) LFSR initial value */

    struct {
      __IOM unsigned int LFSR_INIT : 32; /*!< [31..0] This holds LFSR initialization value.
                              When ever LFSR needs to be initialized, this has to
                              be updated with the init value and trigger
                              init_lfsr in LFSR_INIT_CTRL_SET register. For
                              example, in WiFi case, 0xffffffff is used as init
                              value of LFSR. */
    } CRC_LFSR_INIT_VAL_b;
  };

  union {
    __IOM unsigned int CRC_LFSR_INIT_CTRL_SET; /*!< (@ 0x0000001C) LFSR state initialization
                                    control set register             */

    struct {
      __IOM unsigned int CLEAR_LFSR : 1;           /*!< [0..0] Clear LFSR state. When this is
                                         set, LFSR state is cleared to 0 */
      __IOM unsigned int INIT_LFSR : 1;            /*!< [1..1] Initialize LFSR state. When this
                                        is set LFSR state will be initialized
                                        with LFSR_INIT_VAL/bit swapped
                                        LFSR_INIT_VAL in the next cycle */
      __IOM unsigned int USE_SWAPPED_INIT_VAL : 1; /*!< [2..2] Use bit swapped init value. If
                                         this is set bit swapped  version of LFSR
                                         init value will be loaded / initialized
                                            to LFSR state  */
      __IOM unsigned int RESERVED1 : 29;           /*!< [31..3] Reserved for future use. */
    } CRC_LFSR_INIT_CTRL_SET_b;
  };

  union {
    __IOM unsigned int CRC_LFSR_INIT_CTRL_RESET; /*!< (@ 0x00000020) LFSR state initialization
                                      control reset register           */

    struct {
      __IOM unsigned int RESERVED1 : 1;            /*!< [0..0] Reserved for future use. */
      __IOM unsigned int RESERVED2 : 1;            /*!< [1..1] Reserved for future use. */
      __IOM unsigned int USE_SWAPPED_INIT_VAL : 1; /*!< [2..2] Use bit swapped init value. If
                                         this is set bit swapped  version of LFSR
                                         init value will be loaded / initialized
                                            to LFSR state  */
      __IOM unsigned int RESERVED3 : 29;           /*!< [31..3] Reserved for future use. */
    } CRC_LFSR_INIT_CTRL_RESET_b;
  };

  union {
    __OM unsigned int CRC_DIN_FIFO; /*!< (@ 0x00000024) Data input FIFO register */

    struct {
      __OM unsigned int DIN_FIFO : 32; /*!< [31..0] FIFO input port is mapped to this
                             register. Data on which the final CRC has to be
                             computed has to be loaded to this FIFO */
    } CRC_DIN_FIFO_b;
  };

  union {
    __IOM unsigned int CRC_DIN_CTRL_SET; /*!< (@ 0x00000028) Input data control set
                                         register                            */

    struct {
      __IOM unsigned int DIN_WIDTH_REG : 5;         /*!< [4..0] Valid number of bits in the input data
                                 in din_width_from_reg set mode. Before writing a
                                 new value into this,  din_ctrl_reset_reg has to
                                 be written with 0x1f to clear this field as
                                 these are set/clear bits. */
      __IOM unsigned int DIN_WIDTH_FROM_REG : 1;    /*!< [5..5] Valid number of bits in the input
                                      data. In default, number of valid bits in
                                      the input data is taken from ULI (uli_be).
                                           If this is set, whatever is the input
                                      size, only din_ctrl_reg[4:0] is taken as
                                      valid length/width for inout data. */
      __IOM unsigned int DIN_WIDTH_FROM_CNT : 1;    /*!< [6..6] Valid number of bits in the input
                                      data. In default, number of valid bits in
                                      the input data is taken from ULI (uli_be).
                                           If this is set, a mix of ULI length
                                      and number of bytes remaining will form the
                                      valid bits (which ever is less
                                           that will be considered as valid
                                      bits).                                   */
      __IOM unsigned int USE_SWAPPED_DIN : 1;       /*!< [7..7] Use bit swapped input data. If this
                                   is set, input data will be swapped and filled
                                   in to FIFO. Whatever read out from FIFO will
                                   be directly fed to LFSR engine. */
      __IOM unsigned int RESET_FIFO_PTRS : 1;       /*!< [8..8] Reset fifo pointer. This
                                              clears the FIFO.When this is set,
                                              FIFO will be cleared. */
      __IOM unsigned int RESERVED1 : 15;            /*!< [23..9] Reserved for future use. */
      __IOM unsigned int FIFO_AEMPTY_THRESHOLD : 4; /*!< [27..24] FIFO almost empty threshold
                                         value. This has to be cleared by writing
                                         0x0f000000 into din_ctrl_reset before
                                         updating any new value. */
      __IOM unsigned int FIFO_AFULL_THRESHOULD : 4; /*!< [31..28] FIFO almost full threshold
                                         value. This has to be cleared by writing
                                         0xf0000000 into din_ctrl_reset before
                                         updating any new value */
    } CRC_DIN_CTRL_SET_b;
  };

  union {
    __IOM unsigned int CRC_DIN_CTRL_RESET_REG; /*!< (@ 0x0000002C) Input data
                                               control set register */

    struct {
      __IOM unsigned int DIN_WIDTH_REG : 5;         /*!< [4..0] Valid number of bits in the input data
                                 in din_width_from_reg set mode. Before writing a
                                 new value into this,  din_ctrl_reset_reg has to
                                 be written with 0x1f to clear this field as
                                 these are set/clear bits. */
      __IOM unsigned int DIN_WIDTH_FROM_REG : 1;    /*!< [5..5] Valid number of bits in the input
                                      data. In default, number of valid bits in
                                      the input data is taken from ULI (uli_be).
                                           If this is set, whatever is the input
                                      size, only din_ctrl_reg[4:0] is taken as
                                      valid length/width for inout data. */
      __IOM unsigned int DIN_WIDTH_FROM_CNT : 1;    /*!< [6..6] Valid number of bits in the input
                                      data. In default, number of valid bits in
                                      the input data is taken from ULI (uli_be).
                                           If this is set, a mix of ULI length
                                      and number of bytes remaining will form the
                                      valid bits (which ever is less
                                           that will be considered as valid
                                      bits).                                   */
      __IOM unsigned int USE_SWAPPED_DIN : 1;       /*!< [7..7] Use bit swapped input data. If this
                                   is set input data will be swapped and filled
                                   in to FIFO. Whatever read out from FIFO will
                                   be directly fed to LFSR engine. */
      __IOM unsigned int RESERVED1 : 1;             /*!< [8..8] Reserved for future use.  */
      __IOM unsigned int RESERVED2 : 15;            /*!< [23..9] Reserved for future use. */
      __IOM unsigned int FIFO_AEMPTY_THRESHOLD : 4; /*!< [27..24] FIFO almost empty threshold
                                         value. This has to be cleared by writing
                                         0x0f000000 into din_ctrl_reset before
                                         updating any new value. */
      __IOM unsigned int FIFO_AFULL_THRESHOULD : 4; /*!< [31..28] FIFO almost full threshold
                                         value. This has to be cleared by writing
                                         0xf0000000 into din_ctrl_reset before
                                         updating any new value */
    } CRC_DIN_CTRL_RESET_REG_b;
  };

  union {
    __IOM unsigned int CRC_DIN_NUM_BYTES; /*!< (@ 0x00000030) Data input FIFO register */

    struct {
      __IOM unsigned int DIN_NUM_BYTES : 32; /*!< [31..0] in out data number of bytes */
    } CRC_DIN_NUM_BYTES_b;
  };

  union {
    __IM unsigned int CRC_DIN_STS; /*!< (@ 0x00000034) Input data status register */

    struct {
      __IM unsigned int FIFO_EMPTY : 1;  /*!< [0..0] FIFO empty indication status */
      __IM unsigned int FIFO_AEMPTY : 1; /*!< [1..1] FIFO almost empty indication status. */
      __IM unsigned int FIFO_AFULL : 1;  /*!< [2..2] FIFO almost full indication status */
      __IM unsigned int FIFO_FULL : 1;   /*!< [3..3] FIFO full indication status  */
      __IM unsigned int FIFO_OCC : 6;    /*!< [9..4] FIFO occupancy   */
      __IM unsigned int RESERVED1 : 22;  /*!< [31..10] Reserved for future use. */
    } CRC_DIN_STS_b;
  };

  union {
    __IOM unsigned int CRC_LFSR_STATE; /*!< (@ 0x00000038) LFSR state register */

    struct {
      __IOM unsigned int LFSR_STATE : 32; /*!< [31..0] If LFSR dynamic loading is
                                          required this can be used for writing
                                          the LFSR state directly. */
    } CRC_LFSR_STATE_b;
  };
} CRC_Type; /*!< Size = 60 (0x3c) */

/* ===========================================================================================================================
  */
/* ================                                           EFUSE
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The EFUSE controller is used to provide an interface to one time
  * program memory (EFUSE macro) to perform write and read operations (EFUSE)
  */

typedef struct { /*!< (@ 0x4600C000) EFUSE Structure */

  union {
    __IOM unsigned int EFUSE_DA_ADDR_REG; /*!< (@ 0x00000000) Direct Access Registers */

    struct {
      __IOM unsigned int ADDR_BITS : 16; /*!< [15..0] These bits specifies the address to write
                              or read from EFUSE macro model */
      __IOM unsigned int RESERVED1 : 16; /*!< [31..16] reserved1 */
    } EFUSE_DA_ADDR_REG_b;
  };

  union {
    __IOM unsigned int EFUSE_DA_CTRL_SET_REG; /*!< (@ 0x00000004) Direct Access Set
                                              Registers */

    struct {
      __IOM unsigned int PGENB : 1;      /*!< [0..0] Set Program enable      */
      __IOM unsigned int CSB : 1;        /*!< [1..1] Set Chip Enable        */
      __IOM unsigned int STROBE : 1;     /*!< [2..2] Set strobe enable     */
      __IOM unsigned int LOAD : 1;       /*!< [3..3] Set Load enable       */
      __IOM unsigned int RESERVED1 : 12; /*!< [15..4] reserved1 */
      __IOM unsigned int RESERVED2 : 16; /*!< [31..16] reserved2 */
    } EFUSE_DA_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int EFUSE_DA_CTRL_CLEAR_REG; /*!< (@ 0x00000008) Direct Access
                                                Clear Registers */

    struct {
      __IOM unsigned int PGENB : 1;     /*!< [0..0] Clear Program enable     */
      __IOM unsigned int CSB : 1;       /*!< [1..1] Clear Chip Enable       */
      __IM unsigned int RESERVED1 : 1;  /*!< [2..2] reserved1  */
      __IOM unsigned int LOAD : 1;      /*!< [3..3] Clear Load enable      */
      __IM unsigned int RESERVED2 : 12; /*!< [15..4] reserved2 */
      __IM unsigned int RESERVED3 : 16; /*!< [31..16] reserved3 */
    } EFUSE_DA_CTRL_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int EFUSE_CTRL_REG; /*!< (@ 0x0000000C) Control Register */

    struct {
      __IOM unsigned int EFUSE_ENABLE : 1;             /*!< [0..0] This bit specifies whether the EFUSE
                                module is enabled or not */
      __IOM unsigned int EFUSE_DIRECT_PATH_ENABLE : 1; /*!< [1..1] This bit specifies whether
                                            the EFUSE direct path is enabled or
                                            not for direct accessing of the EFUSE
                                            pins                             */
      __IOM unsigned int ENABLE_EFUSE_WRITE : 1;       /*!< [2..2] Controls the switch on
                                                 VDDIQ for eFuse read/write. */
      __IM unsigned int RESERVED1 : 13;                /*!< [15..3] reserved1          */
      __IM unsigned int RESERVED2 : 16;                /*!< [31..16] reserved2          */
    } EFUSE_CTRL_REG_b;
  };

  union {
    __IOM unsigned int EFUSE_READ_ADDR_REG; /*!< (@ 0x00000010) Read address Register */

    struct {
      __IOM unsigned int READ_ADDR_BITS : 13; /*!< [12..0] These bits specifies the
                                              address from which read operation
                                                      has to be performed */
      __IM unsigned int RESERVED1 : 2;        /*!< [14..13] reserved1        */
      __OM unsigned int DO_READ : 1;          /*!< [15..15] Enables read FSM after EFUSE is
                                     enabled                          */
      __IM unsigned int RESERVED2 : 16;       /*!< [31..16] reserved2 */
    } EFUSE_READ_ADDR_REG_b;
  };

  union {
    __IOM unsigned int EFUSE_READ_DATA_REG; /*!< (@ 0x00000014) Read address Register */

    struct {
      __IOM unsigned int READ_DATA_BITS : 8; /*!< [7..0] These bits specifies the data bits
                                  that are read from a given address specified in
                                  the  EFUSE_READ_ADDRESS_REGISTER bits 8:0 */
      __IM unsigned int RESERVED1 : 7;       /*!< [14..8] reserved1     */
      __IM unsigned int READ_FSM_DONE : 1;   /*!< [15..15] Indicates read fsm is done.
                                           After this read data is available in
                                           EFUSE_READ_DATA_REGISTER bits 7:0 */
      __IM unsigned int RESERVED2 : 16;      /*!< [31..16] reserved2    */
    } EFUSE_READ_DATA_REG_b;
  };

  union {
    __IM unsigned int EFUSE_STATUS_REG; /*!< (@ 0x00000018) Read address Register */

    struct {
      __IM unsigned int EFUSE_ENABLED : 1;    /*!< [0..0] This bit specifies whether
                                           the EFUSE is enabled or not */
      __IM unsigned int RESERVED1 : 1;        /*!< [1..1] reserved1     */
      __IM unsigned int EFUSE_DOUT_SYNC : 8;  /*!< [9..2] This bit specifies the 8-bit data
                                   read out from the EFUSE macro. This is
                                   synchronized with pclk */
      __IM unsigned int STROBE_CLEAR_BIT : 1; /*!< [10..10] This bit indicates STROBE signal
                                    goes low after strobe
                                            count value reached '0' */
      __IM unsigned int RESERVED2 : 5;        /*!< [15..11] reserved2  */
      __IM unsigned int RESERVED3 : 16;       /*!< [31..16] reserved3 */
    } EFUSE_STATUS_REG_b;
  };

  union {
    __IOM unsigned int EFUSE_RD_TMNG_PARAM_REG; /*!< (@ 0x0000001C) none */

    struct {
      __IOM unsigned int TSUR_CS : 4;   /*!< [3..0] CSB to STROBE setup time into read mode */
      __IOM unsigned int TSQ : 4;       /*!< [7..4] Q7-Q0 access time from STROBE rising edge */
      __IOM unsigned int THRA : 4;      /*!< [11..8] for 32x8 macro: A4 A0 to STROBE hold
                                   time into Read mode 5122x8 macro: A8 A0 to
                                   STROBE hold time into Read mode */
      __IM unsigned int RESERVED1 : 4;  /*!< [15..12] reserved1  */
      __IM unsigned int RESERVED2 : 16; /*!< [31..16] reserved2 */
    } EFUSE_RD_TMNG_PARAM_REG_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int EFUSE_MEM_MAP_LENGTH_REG; /*!< (@ 0x00000024) none */

    struct {
      __IOM unsigned int EFUSE_MEM_MAP_LEN : 1; /*!< [0..0] 0: 8 bit read 1: 16 bit read    */
      __IM unsigned int RESERVED1 : 15;         /*!< [15..1] reserved1 */
      __IM unsigned int RESERVED2 : 16;         /*!< [31..16] reserved2 */
    } EFUSE_MEM_MAP_LENGTH_REG_b;
  };

  union {
    __IOM
    unsigned int EFUSE_READ_BLOCK_STARTING_LOCATION; /*!< (@ 0x00000028) Starting
                                                    address from which the read
                                                    has to be blocked. Once the
                                                    end address is written, it
                                                    cannot be changed till power
                                                    on reset is given */

    struct {
      __IOM unsigned int EFUSE_READ_BLOCK_STARTING_LOCATION : 16; /*!< [15..0] Starting address
                                                       from which the read has to
                                                       be blocked. Once the end
                                                       address is written, it
                                                       cannot be changed till
                                                        power on reset is given.
                                                     */
      __IM unsigned int RESERVED1 : 16;                           /*!< [31..16] reserved1                */
    } EFUSE_READ_BLOCK_STARTING_LOCATION_b;
  };

  union {
    __IOM unsigned int EFUSE_READ_BLOCK_END_LOCATION; /*!< (@ 0x0000002C) Starting address from
                                           which the read has to be blocked. Once
                                           the end address is written, it cannot
                                           be changed till power on reset is
                                           given */

    struct {
      __IOM unsigned int EFUSE_READ_BLOCK_END_LOCATION : 16; /*!< [15..0] End address till
                                                  which the read has to be
                                                  blocked. Once the end address
                                                  is written , it cannot be
                                                  changed till
                                                        power on reset is given.
                                                */
      __IM unsigned int RESERVED1 : 16;                      /*!< [31..16] reserved1           */
    } EFUSE_READ_BLOCK_END_LOCATION_b;
  };

  union {
    __IOM unsigned int EFUSE_READ_BLOCK_ENABLE_REG; /*!< (@ 0x00000030) The Transmit Poll
                                         Demand register enables the Transmit DMA
                                         to check whether or not the current
                                         descriptor is owned by DMA */

    struct {
      __IOM unsigned int EFUSE_READ_BLOCK_ENABLE : 1; /*!< [0..0] Enable for blocking the read
                                           access from a programmable memory
                                           location */
      __IM unsigned int RESERVED1 : 15;               /*!< [15..1] reserved1    */
      __IM unsigned int RESERVED2 : 16;               /*!< [31..16] reserved2    */
    } EFUSE_READ_BLOCK_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int EFUSE_DA_CLR_STROBE_REG; /*!< (@ 0x00000034) none */

    struct {
      __IOM unsigned int EFUSE_STROBE_CLR_CNT : 9; /*!< [8..0] Strobe signal Clear count in
                                        direct access mode. value
                                           depends on APB clock frequency of
                                        eFuse controller */
      __IOM unsigned int EFUSE_STROBE_ENABLE : 1;  /*!< [9..9] none */
      __IM unsigned int RESERVED1 : 6;             /*!< [15..10] reserved1            */
      __IM unsigned int RESERVED2 : 16;            /*!< [31..16] reserved2           */
    } EFUSE_DA_CLR_STROBE_REG_b;
  };
} EFUSE_Type; /*!< Size = 56 (0x38) */

/* ===========================================================================================================================
  */
/* ================                                           I2S0
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief I2S(Inter-IC Sound) is transferring two-channel digital audio data
  * from one IC device to another (I2S0)
  */

typedef struct { /*!< (@ 0x47050000) I2S0 Structure */

  union {
    __IOM unsigned int I2S_IER; /*!< (@ 0x00000000) I2S Enable Register */

    struct {
      __IOM unsigned int IEN : 1;        /*!< [0..0] Inter Block Enable        */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_IER_b;
  };

  union {
    __IOM unsigned int I2S_IRER; /*!< (@ 0x00000004) I2S Receiver Block Enable Register */

    struct {
      __IOM unsigned int RXEN : 1;       /*!< [0..0] Receive Block Enable, Bit Overrides
                                   any Individual Receive Channel Enables */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_IRER_b;
  };

  union {
    __IOM unsigned int I2S_ITER; /*!< (@ 0x00000008) Transmitter Block Enable */

    struct {
      __IOM unsigned int TXEN : 1;       /*!< [0..0] Transmitter Block Enable, Bit Overrides any
                        Individual Transmit Channel Enables */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_ITER_b;
  };

  union {
    __IOM unsigned int I2S_CER; /*!< (@ 0x0000000C) Clock Enable Register */

    struct {
      __IOM unsigned int CLKEN : 1;      /*!< [0..0] Clock generation enable/disable */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_CER_b;
  };

  union {
    __IOM unsigned int I2S_CCR; /*!< (@ 0x00000010) Clock Configuration Register */

    struct {
      __IOM unsigned int SCLKG : 3;      /*!< [2..0] These bits are used to program the
                                    gating of sclk                  */
      __IOM unsigned int WSS : 2;        /*!< [4..3] These bits are used to program the
                                    number of sclk cycles           */
      __IOM unsigned int RESERVED1 : 27; /*!< [31..5] Reserved for future use */
    } I2S_CCR_b;
  };

  union {
    __OM unsigned int I2S_RXFFR; /*!< (@ 0x00000014) Receiver Block FIFO Reset Register */

    struct {
      __OM unsigned int RXFFR : 1;      /*!< [0..0] Writing a 1 To This Register Flushes
                                   All The RX FIFO's Receiver Block  Must be
                                   Disable Prior to Writing This Bit */
      __OM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_RXFFR_b;
  };

  union {
    __OM unsigned int I2S_TXFFR; /*!< (@ 0x00000018) Transmitter Block FIFO Reset
                                 Register                      */

    struct {
      __OM unsigned int TXFFR : 1;      /*!< [0..0] Writing a 1 To This Register Flushes
                                   All The RX FIFO's Receiver Block  Must be
                                   Disable Prior to Writing This Bit */
      __OM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_TXFFR_b;
  };
  __IM unsigned int RESERVED;
  __IOM I2S0_CHANNEL_CONFIG_Type CHANNEL_CONFIG[4]; /*!< (@ 0x00000020) [0..3] */
  __IM unsigned int RESERVED1[40];

  union {
    __IM unsigned int I2S_RXDMA; /*!< (@ 0x000001C0) Receiver Block DMA Register */

    struct {
      __IM unsigned int RXDMA : 32; /*!< [31..0] Used to cycle repeatedly through the enabled
                          receive channels  Reading stereo data pairs */
    } I2S_RXDMA_b;
  };

  union {
    __OM unsigned int I2S_RRXDMA; /*!< (@ 0x000001C4) Reset Receiver Block DMA Register */

    struct {
      __OM unsigned int RRXDMA : 1;     /*!< [0..0] Writing a 1 to this self-clearing
                                    register resets the RXDMA register */
      __OM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved for future use */
    } I2S_RRXDMA_b;
  };

  union {
    __OM unsigned int I2S_TXDMA; /*!< (@ 0x000001C8) Transmitter Block DMA Register */

    struct {
      __OM unsigned int TXDMA : 32; /*!< [31..0] Used to cycle repeatedly through
                                    the enabled transmit channels  allow to
                                    writing of stereo data pairs */
    } I2S_TXDMA_b;
  };

  union {
    __OM unsigned int I2S_RTXDMA; /*!< (@ 0x000001CC) Reset Transmitter Block DMA
                                  Register                       */

    struct {
      __OM unsigned int RTXDMA : 1;     /*!< [0..0] Writing a 1 to this self-clearing
                                    register resets the TXDMA register */
      __OM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved1 */
    } I2S_RTXDMA_b;
  };
  __IM unsigned int RESERVED2[8];

  union {
    __IM unsigned int I2S_COMP_PARAM_2; /*!< (@ 0x000001F0) Component Parameter 2
                                        Register                             */

    struct {
      __IM unsigned int I2S_RX_WORDSIZE_0 : 3; /*!< [2..0] On Read returns the value
                                               of word size of receiver channel
                                                      0 */
      __IM unsigned int I2S_RX_WORDSIZE_1 : 3; /*!< [5..3] On Read returns the value
                                               of word size of receiver channel
                                                      1 */
      __IM unsigned int RESERVED1 : 1;         /*!< [6..6] Reserved1         */
      __IM unsigned int I2S_RX_WORDSIZE_2 : 3; /*!< [9..7] On Read returns the value
                                               of word size of receiver channel
                                                      2 */
      __IM unsigned int I2S_RX_WORDSIZE_3 : 3; /*!< [12..10] On Read returns the value of word
                                     size of receiver channel 3 */
      __IM unsigned int RESERVED2 : 19;        /*!< [31..13] Reserved2 */
    } I2S_COMP_PARAM_2_b;
  };

  union {
    __IM unsigned int I2S_COMP_PARAM_1; /*!< (@ 0x000001F4) Component Parameter 1
                                        Register                             */

    struct {
      __IM unsigned int APB_DATA_WIDTH : 2;        /*!< [1..0] Width of APB data bus */
      __IM unsigned int I2S_FIFO_DEPTH_GLOBAL : 2; /*!< [3..2] Determines FIFO depth
                                                   for all channels */
      __IM unsigned int I2S_FIFO_MODE_EN : 1;      /*!< [4..4] Determines whether component act as
                                    Master or Slave                */
      __IM unsigned int I2S_TRANSMITTER_BLOCK : 1; /*!< [5..5] Shows the presence of
                                                   the transmitter block */
      __IM unsigned int I2S_RECEIVER_BLOCK : 1;    /*!< [6..6] Shows the presence of
                                                   the receiver block    */
      __IM unsigned int I2S_RX_CHANNELS : 2;       /*!< [8..7] Returns the number of
                                                   receiver channels       */
      __IM unsigned int I2S_TX_CHANNELS : 2;       /*!< [10..9] Returns the number of
                                               transmitter channels   */
      __IM unsigned int RESERVED1 : 5;             /*!< [15..11] Reserved1         */
      __IM unsigned int I2S_TX_WORDSIZE_0 : 3;     /*!< [18..16] Returns the value of
                                               word size of transmitter channel
                                                      0 */
      __IM unsigned int I2S_TX_WORDSIZE_1 : 3;     /*!< [21..19] Returns the value of
                                               word size of transmitter channel
                                                      1 */
      __IM unsigned int I2S_TX_WORDSIZE_2 : 3;     /*!< [24..22] Returns the value of
                                               word size of transmitter channel
                                                      2 */
      __IM unsigned int I2S_TX_WORDSIZE_3 : 3;     /*!< [27..25] Returns the value of
                                               word size of transmitter channel
                                                      3 */
      __IM unsigned int RESERVED2 : 4;             /*!< [31..28] Reserved2         */
    } I2S_COMP_PARAM_1_b;
  };

  union {
    __IM unsigned int I2S_COMP_VERSION_REG; /*!< (@ 0x000001F8) Component Version ID */

    struct {
      __IM unsigned int I2S_COMP_VERSION : 32; /*!< [31..0] Return the component
                                               version(1.02) */
    } I2S_COMP_VERSION_REG_b;
  };

  union {
    __IM unsigned int I2S_COMP_TYPE_REG; /*!< (@ 0x000001FC) Component Type */

    struct {
      __IM unsigned int I2S_COMP_TYPE : 32; /*!< [31..0] Return the component type */
    } I2S_COMP_TYPE_REG_b;
  };
} I2S0_Type; /*!< Size = 512 (0x200) */

/* ===========================================================================================================================
  */
/* ================                                          IID_AES
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The AES module provides AES encoding and decoding functionality. It
  * can be used in a microprocessor based environment (IID_AES)
  */

typedef struct { /*!< (@ 0x20480500) IID_AES Structure */

  union {
    __IOM unsigned int AES_KCR; /*!< (@ 0x00000000) AES Key Control register */

    struct {
      __IOM unsigned int AES_KEY_CHNG_REQ : 1; /*!< [0..0] Programming 1 clears the current key
                                    and starts a request a for a new key
                                    Auto-reverts to 0 as soon as the request is
                                    accepted */
      __IOM unsigned int AES_KEY_SIZE : 1;     /*!< [1..1] Size of the AES key 0:
                                           128-bit 1: 256-bit */
      __IM unsigned int : 5;
      __IOM unsigned int AES_KEY_SRC : 1; /*!< [7..7] Source of the AES key 0:
                                          Interface 1: Register */
      __IM unsigned int : 24;
    } AES_KCR_b;
  };

  union {
    __IOM unsigned int AES_MODE_REG; /*!< (@ 0x00000004) AES Mode register */

    struct {
      __IOM unsigned int AES_MODE : 8; /*!< [7..0] The AES Mode register defines
                                       which mode of AES is used.           */
      __IM unsigned int : 24;
    } AES_MODE_REG_b;
  };

  union {
    __IOM unsigned int AES_ACT_REG; /*!< (@ 0x00000008) AES Action register */

    struct {
      __IOM unsigned int AES_ACTION : 2; /*!< [1..0] The AES Mode register defines
                                         which mode of AES is used.           */
      __IM unsigned int : 30;
    } AES_ACT_REG_b;
  };
  __IM unsigned int RESERVED[5];

  union {
    __IM unsigned int AES_SR_REG; /*!< (@ 0x00000020) AES Status register */

    struct {
      __IM unsigned int AES_BUSY : 1; /*!< [0..0] Indicates that the AES core is
                                      processing data                     */
      __IM unsigned int : 1;
      __IM unsigned int AES_CLEAR_DONE : 1;  /*!< [2..2] Indicates that the Clear
                                             action is finished  */
      __IM unsigned int AES_KEY_PRESENT : 1; /*!< [3..3] Indicates that the Clear
                                             action is finished */
      __IM unsigned int : 1;
      __IM unsigned int AES_KEY_REQ : 1;  /*!< [5..5] Indicates that a key must be
                                          provided                              */
      __IM unsigned int AES_DATA_REQ : 1; /*!< [6..6] Indicates that data must be
                                          provided */
      __IM unsigned int AES_DATA_AV : 1;  /*!< [7..7] Indicates that data is available */
      __IM unsigned int : 24;
    } AES_SR_REG_b;
  };
  __IM unsigned int RESERVED1[7];

  union {
    __OM unsigned int AES_KEY_REG; /*!< (@ 0x00000040) The AES Key register is used
                                   to program a key into the AES module. */

    struct {
      __OM unsigned int AES_KEY : 32; /*!< [31..0] 4 writes of 32 bits make up the 128-bit key
                            for AES, 8 writes make up the 256-bit key */
    } AES_KEY_REG_b;
  };

  union {
    __OM unsigned int AES_DIN_REG; /*!< (@ 0x00000044) AES Data In register */

    struct {
      __OM unsigned int AES_DIN : 32; /*!< [31..0] Data for encoding or decoding, 4 writes of
                            32 bits make up a 128-bit data word */
    } AES_DIN_REG_b;
  };

  union {
    __IM unsigned int AES_DOUT_REG; /*!< (@ 0x00000048) AES Data out register */

    struct {
      __IM unsigned int AES_DOUT : 32; /*!< [31..0] Result from encoding or decoding, 4 reads
                             of 32 bits make up a 128-bit data word */
    } AES_DOUT_REG_b;
  };
  __IM unsigned int RESERVED2[36];

  union {
    __OM unsigned int AES_IF_SR_C_REG; /*!< (@ 0x000000DC) AES Interface Status
                                       Clear register                        */

    struct {
      __OM unsigned int IFB_ERROR : 1; /*!< [0..0] Clears the if_error bit */
      __IM unsigned int : 31;
    } AES_IF_SR_C_REG_b;
  };

  union {
    __IM unsigned int AES_IF_SR_REG; /*!< (@ 0x000000E0) AES Interface Status register */

    struct {
      __IM unsigned int IF_ERROR : 1; /*!< [0..0] Indicates that an interface error
                                      has occurred                     */
      __IM unsigned int : 31;
    } AES_IF_SR_REG_b;
  };

  union {
    __IOM unsigned int AES_TEST_REG; /*!< (@ 0x000000E4) AES Test register */

    struct {
      __IOM unsigned int AES_BIST_ENABLE : 1; /*!< [0..0] Isolates the iid_aes
                                              module and runs a BIST */
      __IM unsigned int : 3;
      __IOM unsigned int AES_BIST_RUNNING : 1; /*!< [4..4] BIST is in progress or
                                               finishing up */
      __IOM unsigned int AES_BIST_ACTIVE : 1;  /*!< [5..5] Indicates that the BIST is
                                              running */
      __IOM unsigned int AES_BIST_OK : 1;      /*!< [6..6] Indicates that the BIST has passed */
      __IOM unsigned int AES_BIST_ERROR : 1;   /*!< [7..7] Indicates that the BIST has
                                             failed */
      __IM unsigned int : 24;
    } AES_TEST_REG_b;
  };
  __IM unsigned int RESERVED3[6];

  union {
    __IM unsigned int AES_VER_REG; /*!< (@ 0x00000100) AES Version register */

    struct {
      __IM unsigned int AES_VERSION : 32; /*!< [31..0] Version of iid_aes */
    } AES_VER_REG_b;
  };
} IID_AES_Type; /*!< Size = 260 (0x104) */

/* ===========================================================================================================================
  */
/* ================                                          IID_QK
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The purpose of Quiddikey is to provide secure key storage without
  * storing the key. (IID_QK)
  */

typedef struct { /*!< (@ 0x20480600) IID_QK Structure */

  union {
    __OM unsigned int QK_CR_REG; /*!< (@ 0x00000000) Quiddikey Control register.The
                                 Quiddikey Control register defines which command
                                 must be executed next. */

    struct {
      __OM unsigned int QK_ZEROIZE : 1; /*!< [0..0] Begin Zeroize operation and go
                                        to Error state                      */
      __OM unsigned int QK_ENROLL : 1;  /*!< [1..1] Begin Enroll operation  */
      __OM unsigned int QK_START : 1;   /*!< [2..2] Begin Start operation   */
      __OM unsigned int QK_SET_IK : 1;  /*!< [3..3] Begin Set Intrinsic Key operation */
      __OM unsigned int QK_SET_UK : 1;  /*!< [4..4] Begin Set User Key operation */
      __OM unsigned int QK_SET_XK : 1;  /*!< [5..5] Begin Set External Key operation */
      __OM unsigned int QK_GET_KEY : 1; /*!< [6..6] Begin Get Key operation */
      __IM unsigned int : 25;
    } QK_CR_REG_b;
  };

  union {
    __IOM unsigned int QK_KIDX_REG; /*!< (@ 0x00000004) The Quiddikey Key Index register
                         defines the key index for the next set_key command */

    struct {
      __IOM unsigned int QK_KEY_INDEX : 4; /*!< [3..0] Key index for Set Key operations */
      __IM unsigned int : 28;
    } QK_KIDX_REG_b;
  };

  union {
    __IOM unsigned int QK_KSZ_REG; /*!< (@ 0x00000008) Quiddikey Key Size register */

    struct {
      __IOM unsigned int QK_KEY_SIZE : 6; /*!< [5..0] Key size for Set Key operations */
      __IM unsigned int : 26;
    } QK_KSZ_REG_b;
  };

  union {
    __IOM unsigned int QK_KT_REG; /*!< (@ 0x0000000C) Quiddikey Key Size register */

    struct {
      __IOM unsigned int QK_KEY_TARGET : 1; /*!< [0..0] Target of reconstructed key */
      __IM unsigned int : 31;
    } QK_KT_REG_b;
  };
  __IM unsigned int RESERVED[4];

  union {
    __IM unsigned int QK_SR_REG; /*!< (@ 0x00000020) Quiddikey Status register */

    struct {
      __IM unsigned int QK_BUSY : 1;   /*!< [0..0] Indicates that operation is in progress */
      __IM unsigned int QK_OK : 1;     /*!< [1..1] Last operation was successful    */
      __IM unsigned int QK_ERROR : 1;  /*!< [2..2] Quiddikey is in the Error state
                                      and no operations can be performed */
      __IM unsigned int QK_XO_AV : 1;  /*!< [3..3] Next part of XKPD is available            */
      __IM unsigned int QK_KI_REQ : 1; /*!< [4..4] Request for next part of key */
      __IM unsigned int QK_KO_AV : 1;  /*!< [5..5] Next part of key is available  */
      __IM unsigned int QK_CI_REQ : 1; /*!< [6..6] Request for next part of AC/KC */
      __IM unsigned int QK_CO_AV : 1;  /*!< [7..7] Next part of AC/KC is available */
      __IM unsigned int : 24;
    } QK_SR_REG_b;
  };
  __IM unsigned int RESERVED1;

  union {
    __IM unsigned int QK_AR_REG; /*!< (@ 0x00000028) Quiddikey allow register */

    struct {
      __IM unsigned int QK_ALLOW_ENROLL : 1;  /*!< [0..0] Enroll operation is allowed */
      __IM unsigned int QK_ALLOW_START : 1;   /*!< [1..1] Start operation is allowed */
      __IM unsigned int QK_ALLOW_SET_KEY : 1; /*!< [2..2] Set Key operations are allowed */
      __IM unsigned int QK_ALLOW_GET_KEY : 1; /*!< [3..3] Get Key operation is allowed */
      __IM unsigned int : 3;
      __IM unsigned int QK_ALLOW_BIST : 1; /*!< [7..7] BIST is allowed to be started */
      __IM unsigned int : 24;
    } QK_AR_REG_b;
  };
  __IM unsigned int RESERVED2[5];

  union {
    __IOM unsigned int QK_KI_REG; /*!< (@ 0x00000040) Quiddikey Key Input register */

    struct {
      __IOM unsigned int QK_KI : 32; /*!< [31..0] Key input data */
    } QK_KI_REG_b;
  };

  union {
    __IOM unsigned int QK_CI_REG; /*!< (@ 0x00000044) Quiddikey Code Input register */

    struct {
      __IOM unsigned int QK_CI : 32; /*!< [31..0] AC/KC input data */
    } QK_CI_REG_b;
  };

  union {
    __IM unsigned int QK_CO_REG; /*!< (@ 0x00000048) Quiddikey Code Output register */

    struct {
      __IM unsigned int QK_CO : 32; /*!< [31..0] AC/KC output data */
    } QK_CO_REG_b;
  };

  union {
    __IM unsigned int QK_XO_REG; /*!< (@ 0x0000004C) Quiddikey XKPD Output register */

    struct {
      __IM unsigned int QK_XO : 32; /*!< [31..0] XKPD output data */
    } QK_XO_REG_b;
  };
  __IM unsigned int RESERVED3[4];

  union {
    __IM unsigned int QK_KO_IDX_REG; /*!< (@ 0x00000060) Quiddikey Key Output Index
                                     register                        */

    struct {
      __IM unsigned int qk_ko_index : 4; /*!< [3..0] Key index for the key that is currently
                               output via the Key Output register */
      __IM unsigned int : 28;
    } QK_KO_IDX_REG_b;
  };

  union {
    __IM unsigned int QK_KO_REG; /*!< (@ 0x00000064) Quiddikey Code Output register */

    struct {
      __IM unsigned int QK_KO : 32; /*!< [31..0] Key output data */
    } QK_KO_REG_b;
  };
  __IM unsigned int RESERVED4[29];

  union {
    __IM unsigned int QK_IF_SR_C_REG; /*!< (@ 0x000000DC) Quiddikey Interface Status
                                      register                        */

    struct {
      __IM unsigned int IF_ERROR : 1; /*!< [0..0] Clears the if_error bit */
      __IM unsigned int : 31;
    } QK_IF_SR_C_REG_b;
  };

  union {
    __IM unsigned int QK_IF_SR_REG; /*!< (@ 0x000000E0) Quiddikey Interface Status
                                    register                        */

    struct {
      __IM unsigned int IF_ERROR : 1; /*!< [0..0] Indicates that an interface error
                                      has occurred                     */
      __IM unsigned int : 31;
    } QK_IF_SR_REG_b;
  };

  union {
    __IOM unsigned int QK_TEST_REG; /*!< (@ 0x000000E4) QK Test register */

    struct {
      __IOM unsigned int QK_BIST_ENABLE : 1; /*!< [0..0] Isolates the iid_quiddikey
                                             module and runs a BIST */
      __IM unsigned int : 3;
      __IOM unsigned int QK_BIST_RUNNING : 1; /*!< [4..4] BIST is in progress or
                                              finishing up */
      __IOM unsigned int QK_BIST_ACTIVE : 1;  /*!< [5..5] Indicates that the BIST is
                                              running  */
      __IOM unsigned int QK_BIST_OK : 1;      /*!< [6..6] Indicates that the BIST has passed */
      __IOM unsigned int QK_BIST_ERROR : 1;   /*!< [7..7] Indicates that the BIST has failed */
      __IM unsigned int : 24;
    } QK_TEST_REG_b;
  };
  __IM unsigned int RESERVED5[6];

  union {
    __IM unsigned int QK_VER_REG; /*!< (@ 0x00000100) QK Version register */

    struct {
      __IM unsigned int QK_VERSION : 32; /*!< [31..0] Version of iid_qk */
    } QK_VER_REG_b;
  };
} IID_QK_Type; /*!< Size = 260 (0x104) */

/* ===========================================================================================================================
  */
/* ================                                         IID_RPINE
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief none (IID_RPINE)
  */

typedef struct { /*!< (@ 0x20480400) IID_RPINE Structure */

  union {
    __IOM unsigned int IID_BIST_CTRL_REG; /*!< (@ 0x00000000) Quiddikey Control register.The
                               Quiddikey Control register defines which command
                               must be executed next. */

    struct {
      __IOM unsigned int QK_BIST_ENABLE : 1;  /*!< [0..0] none  */
      __IOM unsigned int AES_BIST_ENABLE : 1; /*!< [1..1] none */
      __IOM unsigned int KH_BIST_ENABLE : 1;  /*!< [2..2] none  */
      __IM unsigned int : 29;
    } IID_BIST_CTRL_REG_b;
  };

  union {
    __IOM unsigned int IID_BIST_STATUS_REG; /*!< (@ 0x00000004) none */

    struct {
      __IOM unsigned int QK_BIST_ACTIVE : 1;   /*!< [0..0] none */
      __IOM unsigned int QK_BIST_ERROR : 1;    /*!< [1..1] Indicates that the BIST has failed */
      __IOM unsigned int QK_BIST_OK : 1;       /*!< [2..2] Indicates that the BIST has passed */
      __IOM unsigned int QK_BIST_RUNNING : 1;  /*!< [3..3] Indicates that the BIST is
                                              running */
      __IOM unsigned int AES_BIST_ACTIVE : 1;  /*!< [4..4] none */
      __IOM unsigned int AES_BIST_ERROR : 1;   /*!< [5..5] none  */
      __IOM unsigned int AES_BIST_OK : 1;      /*!< [6..6] Indicates that the BIST has passed */
      __IOM unsigned int AES_BIST_RUNNING : 1; /*!< [7..7] Indicates that the BIST
                                               is running */
      __IOM unsigned int KH_BIST_STATUS : 1;   /*!< [8..8] none   */
      __IM unsigned int : 23;
    } IID_BIST_STATUS_REG_b;
  };

  union {
    __IOM unsigned int IID_CTRL_REG; /*!< (@ 0x00000008) none */

    struct {
      __IOM unsigned int AES_MAX_KEY_SIZE : 1; /*!< [0..0] 1 256 bit key, 0 128 bit key */
      __IOM unsigned int SOURCE_KEY_KH : 1;    /*!< [1..1] When set KH will source the key to AES
                                 engine. When this is not QK key output is
                                 connected to AES key input */
      __IOM unsigned int LATCH_KEY_KH : 1;     /*!< [2..2] When set KH will latch the key given by
                                QK. When this is not QK key output is connected
                                to AES key input                        */
      __IOM unsigned int KH_RESET_N : 1;       /*!< [3..3] 0 KH will be in reset 1 Out of reset */
      __IOM unsigned int KH_KEY_SIZE : 1;      /*!< [4..4] 0 128 bit key 1 256 bit key
                                          This is used by KH */
      __IOM unsigned int KH_CLOCK_RATIO : 3;   /*!< [7..5] Indicates the division factor to be
                                  used for generating kh_clk. */
      __IM unsigned int : 24;
    } IID_CTRL_REG_b;
  };

  union {
    __IOM unsigned int WKE_CTRL_REG; /*!< (@ 0x0000000C) none */

    struct {
      __IOM unsigned int ENABLE_WKE : 1;    /*!< [0..0] When WKE will be enabled. This
                                         is a self clearing bit.
                                                    Once enabled WKE can not be
                                         disabled till process is done */
      __IOM unsigned int WKE_KEY_SIZE : 1;  /*!< [1..1] 0 128 bit size 1 256 bit size */
      __IOM unsigned int WKE_FLUSH : 1;     /*!< [2..2] When set, WKE will flush out the data from
                             AES. When WEK is active,  firmware reads to AES
                             engine are masked. This gets cleared once four
                             dwords are read from AES                      */
      __IOM unsigned int WKE_COMPARE : 1;   /*!< [3..3] When set, WKE will compare the data from
                               AES engine with the data provided by firmware */
      __IOM unsigned int WKE_SET_KEY : 1;   /*!< [4..4] This has to be set after key
                                            available from AES   */
      __IOM unsigned int KEY_CODE_DONE : 1; /*!< [5..5] This has to be set after
                                            reading key code */
      __IM unsigned int : 26;
    } WKE_CTRL_REG_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int IID_AES_CTRL_REG; /*!< (@ 0x00000014) none */

    struct {
      __IOM unsigned int KEY_REQ_IN_DMA_PATH : 1;         /*!< [0..0] Include key req in dma path. With
                                      this KEY Also can be loaded using DMA. */
      __IOM unsigned int AES_MAX_KEY_SIZE_FRM_REG : 1;    /*!< [1..1] This is valid
                                                      only when
                                                      aes_max_key_size_frm_reg_en
                                                      is set. */
      __IOM unsigned int AES_MAX_KEY_SIZE_FRM_REG_EN : 1; /*!< [2..2] When set, WKE will flush
                                           out the data from AES. When WEK is
                                           active,  firmware reads to AES engine
                                           are masked. This gets cleared once
                                           four dwords are read from AES */
      __IOM unsigned int OTP_KEY_LOADING : 1;             /*!< [3..3] When set, WKE will compare
                                              the data from AES engine with the
                                              data provided by firmware */
      __IM unsigned int : 28;
    } IID_AES_CTRL_REG_b;
  };

  union {
    __IM unsigned int IID_AES_STS_REG; /*!< (@ 0x00000018) none */

    struct {
      __IM unsigned int DIN_FIFO_FULL : 1;   /*!< [0..0] Input data fifo full indication */
      __IM unsigned int DOUT_FIFO_EMPTY : 1; /*!< [1..1] Output data fifo empty
                                             indication */
      __IM unsigned int : 30;
    } IID_AES_STS_REG_b;
  };
  __IM unsigned int RESERVED1;

  union {
    __IOM unsigned int WKE_STATUS_REG; /*!< (@ 0x00000020) none */

    struct {
      __IOM unsigned int WKE_ACTIVE : 1;               /*!< [0..0] Will be high when WKE is active */
      __IOM unsigned int WKE_KEY_FEED_IN_PROGRESS : 1; /*!< [1..1] Will be high when WKE is
                                            feeding key to AES engine */
      __IOM unsigned int WKE_FLUSH_IN_PROGRESS : 1;    /*!< [2..2] Will be high when WKE flushing
                                         out the data from AES               */
      __IOM unsigned int WKE_COMPARE_IN_PROGRESS : 1;  /*!< [3..3] Will be high when WKE is
                                           comparing the data from AES */
      __IOM unsigned int WKE_SET_KEY_IN_PROGRESS : 1;  /*!< [4..4] Will be high when WKE is
                                            doing set key operation with  QK  */
      __IOM unsigned int WKE_KEY_READY : 1;            /*!< [5..5] Firmware has to load the
                                            authentication, which will be
                                            compared with AES output, when this
                                            bit is low */
      __IOM unsigned int WKE_CMP_DATA_READY : 1;       /*!< [6..6] Firmware has to load
                                                 the authentication, which will
                                                 be compared with AES output,
                                                 when this bit is low */
      __IOM unsigned int WKE_COMPARE_FAIL : 1;         /*!< [7..7] This bit will be set when
                                    authentication data comparison fails */
      __IM unsigned int : 24;
    } WKE_STATUS_REG_b;
  };
  __IM unsigned int RESERVED2;
  __IOM unsigned int WKE_DATA_REG; /*!< (@ 0x00000028) none */
} IID_RPINE_Type;                  /*!< Size = 44 (0x2c)              */

/* ===========================================================================================================================
  */
/* ================                                            CT0
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief Configurable timer is used in counting clocks, events and states with
   reference clock external clock and system clock (CT0)
   */

typedef struct { /*!< (@ 0x45060000) CT0 Structure */

  union {
    __IOM unsigned int CT_GEN_CTRL_SET_REG; /*!< (@ 0x00000000) General control set
                                            register */

    struct {
      __IOM unsigned int COUNTER_IN_32_BIT_MODE : 1;        /*!< [0..0] Counter_1 and Counter_0 will
                                          be merged and used as a single 32 bit
                                          counter */
      __IOM unsigned int SOFT_RESET_COUNTER_0_FRM_REG : 1;  /*!< [1..1] This is applied to 32
                                                bits of counter only when the
                                                counter is in 32 bit counter mode
                                                otherwise this will be applied
                                                        to only lower 16 bits of
                                                counter */
      __IOM unsigned int PERIODIC_EN_COUNTER_0_FRM_REG : 1; /*!< [2..2] This is applied to 32
                                                 bits of counter only when the
                                                 counter is in 32 bit counter
                                                 mode  otherwise this will be
                                                 applied
                                                        to only lower 16 bits of
                                                 counter */
      __IOM unsigned int COUNTER_0_TRIG_FRM_REG : 1;        /*!< [3..3] This enables the
                                                     counter to run/active */
      __IOM unsigned int COUNTER_0_UP_DOWN : 2;             /*!< [5..4] This enables the counter to run in
                                     up/down/up-down/down-up directions */
      __IOM unsigned int COUNTER_0_SYNC_TRIG : 1;           /*!< [6..6] This is applied to 32 bits of
                                           counter only when the counter     is in 32 bit
                                           counter mode otherwise this will be
                                           applied     to only lower 16 bits of counter.
                                           This enables the counter     to run/active
                                           when sync is found.     */
      __IOM unsigned int BUF_REG_0_EN : 1;                  /*!< [7..7] Buffer register gets enabled
                                           for MATCH REG. MATCH_BUF_REG is always
                                           available and whenever this bit is set
                                           only, gets copied to MATCH REG. */
      __IOM unsigned int RESERVED1 : 9;                     /*!< [16..8] Reserved1    */
      __IOM unsigned int SOFT_RESET_COUNTER_1_FRM_REG : 1;  /*!< [17..17] This resets the
                                                counter on the write */
      __IOM unsigned int PERIODIC_EN_COUNTER_1_FRM_REG : 1; /*!< [18..18] This resets the
                                                 counter on the write */
      __IOM unsigned int COUNTER_1_TRIG_FRM : 1;            /*!< [19..19] This enables the
                                                 counter to run/active */
      __IOM unsigned int COUNTER_1_UP_DOWN : 2;             /*!< [21..20] This enables the counter to run
                                     in upward direction              */
      __IOM unsigned int COUNTER_1_SYNC_TRIG : 1;           /*!< [22..22] This is applied to 32 bits of
                                       counter only when the counter is in 32 bit
                                       counter mode otherwise this will be
                                           applied to only lower 16 bits of
                                       counter. This enables the counter to
                                       run/active when sync is found. */
      __IOM unsigned int BUF_REG_1_EN : 1;                  /*!< [23..23] Buffer register gets enabled for MATCH
                                REG. MATCH_BUF_REG is always available and
                                whenever this bit is set only, gets copied to
                                MATCH REG. */
      __IOM unsigned int RESERVED2 : 8;                     /*!< [31..24] Reserved2 */
    } CT_GEN_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int CT_GEN_CTRL_RESET_REG; /*!< (@ 0x00000004) General control
                                              reset register */

    struct {
      __IOM unsigned int COUNTER_IN_32_BIT_MODE : 1;        /*!< [0..0] Counter_1 and Counter_0 will
                                          be merged and used as a single 32 bit
                                          counter */
      __IM unsigned int RESERVED1 : 1;                      /*!< [1..1] Reserved1    */
      __IOM unsigned int PERIODIC_EN_COUNTER_0_FRM_REG : 1; /*!< [2..2] This is applied to 32
                                                 bits of counter only when the
                                                 counter is in 32 bit counter
                                                 mode  otherwise this will be
                                                 applied
                                                        to only lower 16 bits of
                                                 counter */
      __IM unsigned int RESERVED2 : 1;                      /*!< [3..3] Reserved2           */
      __IOM unsigned int COUNTER_0_UP_DOWN : 2;             /*!< [5..4] This enables the counter to run in
                                     up/down/up-down/down-up directions */
      __IM unsigned int RESERVED3 : 1;                      /*!< [6..6] Reserved3     */
      __IOM unsigned int BUF_REG_0_EN : 1;                  /*!< [7..7] Buffer register gets enabled
                                           for MATCH REG. MATCH_BUF_REG is always
                                           available and whenever this bit is set
                                           only, gets copied to MATCH REG. */
      __IM unsigned int RESERVED4 : 9;                      /*!< [16..8] Reserved4     */
      __IM unsigned int RESERVED5 : 1;                      /*!< [17..17] Reserved5     */
      __IOM unsigned int PERIODIC_EN_COUNTER_1_FRM_REG : 1; /*!< [18..18] This resets the
                                                 counter on the write */
      __IM unsigned int RESERVED6 : 1;                      /*!< [19..19] Reserved6           */
      __IOM unsigned int COUNTER_1_UP_DOWN : 2;             /*!< [21..20] This enables the counter to run
                                       in upward direction              */
      __IM unsigned int RESERVED7 : 1;                      /*!< [22..22] Reserved7 */
      __IOM unsigned int BUF_REG_1_EN : 1;                  /*!< [23..23] Buffer register gets enabled for MATCH
                                REG. MATCH_BUF_REG is always available and
                                whenever this bit is set only, gets copied to
                                MATCH REG. */
      __IM unsigned int RESERVED8 : 8;                      /*!< [31..24] Reserved8 */
    } CT_GEN_CTRL_RESET_REG_b;
  };

  union {
    __IM unsigned int CT_INTR_STS; /*!< (@ 0x00000008) Interrupt status */

    struct {
      __IM unsigned int INTR_0_L : 1;            /*!< [0..0] Indicates the FIFO full signal of
                                      channel-0                        */
      __IM unsigned int FIFO_0_FULL_L : 1;       /*!< [1..1] Indicates the FIFO full
                                                 signal of channel-0       */
      __IM unsigned int COUNTER_0_IS_ZERO_L : 1; /*!< [2..2] Counter 0 hit zero in
                                                 active mode. */
      __IM unsigned int COUNTER_0_IS_PEAK_L : 1; /*!< [3..3] Counter 0 hit peak
                                                 (MATCH) in active mode. */
      __IM unsigned int RESERVED1 : 12;          /*!< [15..4] Reserved1          */
      __IM unsigned int INTR_1_L : 1;            /*!< [16..16] Indicates the FIFO full signal
                                      of channel-1                      */
      __IM unsigned int FIFO_1_FULL_L : 1;       /*!< [17..17] Indicates the FIFO full
                                           signal of channel-1 */
      __IM unsigned int COUNTER_1_IS_ZERO_L : 1; /*!< [18..18] Counter 1 hit zero in
                                                 active mode. */
      __IM unsigned int COUNTER_1_IS_PEAK_L : 1; /*!< [19..19] Counter 1 hit peak
                                                 (MATCH) in active mode. */
      __IM unsigned int RESERVED2 : 12;          /*!< [31..20] Reserved2          */
    } CT_INTR_STS_b;
  };

  union {
    __IOM unsigned int CT_INTR_MASK; /*!< (@ 0x0000000C) Interrupts mask */

    struct {
      __IOM unsigned int INTR_0_L : 1;            /*!< [0..0] Interrupt mask signal.      */
      __IOM unsigned int FIFO_0_FULL_L : 1;       /*!< [1..1] Interrupt mask signal. */
      __IOM unsigned int COUNTER_0_IS_ZERO_L : 1; /*!< [2..2] Interrupt mask signal. */
      __IOM unsigned int COUNTER_0_IS_PEAK_L : 1; /*!< [3..3] Interrupt mask signal.   */
      __IOM unsigned int RESERVED1 : 12;          /*!< [15..4] Reserved1 */
      __IOM unsigned int INTR_1_L : 1;            /*!< [16..16] Interrupt mask signal.   */
      __IOM unsigned int FIFO_1_FULL_L : 1;       /*!< [17..17] Interrupt mask signal. */
      __IOM unsigned int COUNTER_1_IS_ZERO_L : 1; /*!< [18..18] Interrupt mask signal. */
      __IOM unsigned int COUNTER_1_IS_PEAK_L : 1; /*!< [19..19] Interrupt mask signal.   */
      __IOM unsigned int RESERVED2 : 12;          /*!< [31..20] Reserved2 */
    } CT_INTR_MASK_b;
  };

  union {
    __IOM unsigned int CT_INTER_UNMASK; /*!< (@ 0x00000010) Interrupts unmask */

    struct {
      __IOM unsigned int INTR_0_L : 1;            /*!< [0..0] Interrupt unmask signal. */
      __IOM unsigned int FIFO_0_FULL_L : 1;       /*!< [1..1] Interrupt unmask signal. */
      __IOM unsigned int COUNTER_0_IS_ZERO_L : 1; /*!< [2..2] Interrupt unmask signal. */
      __IOM unsigned int COUNTER_0_IS_PEAK_L : 1; /*!< [3..3] Interrupt unmask signal.  */
      __IM unsigned int RESERVED1 : 12;           /*!< [15..4] Reserved1 */
      __IOM unsigned int INTR_1_L : 1;            /*!< [16..16] Interrupt unmask signal.  */
      __IOM unsigned int FIFO_1_FULL_L : 1;       /*!< [17..17] Interrupt unmask signal */
      __IOM unsigned int COUNTER_1_IS_ZERO_L : 1; /*!< [18..18] Interrupt unmask signal. */
      __IOM unsigned int COUNTER_1_IS_PEAK_L : 1; /*!< [19..19] Interrupt unmask signal.  */
      __IM unsigned int RESERVED2 : 12;           /*!< [31..20] Reserved2 */
    } CT_INTER_UNMASK_b;
  };

  union {
    __IOM unsigned int CT_INTR_ACK; /*!< (@ 0x00000014) Interrupt clear/ack register */

    struct {
      __IOM unsigned int INTR_0_L : 1;            /*!< [0..0] Interrupt ack signal.      */
      __IOM unsigned int FIFO_0_FULL_L : 1;       /*!< [1..1] Interrupt ack signal. */
      __IOM unsigned int COUNTER_0_IS_ZERO_L : 1; /*!< [2..2] Interrupt ack signal. */
      __IOM unsigned int COUNTER_0_IS_PEAK_L : 1; /*!< [3..3] Interrupt ack signal.      */
      __IM unsigned int RESERVED1 : 12;           /*!< [15..4] Reserved1     */
      __IOM unsigned int INTR_1_L : 1;            /*!< [16..16] Interrupt ack signal.      */
      __IOM unsigned int FIFO_1_FULL_L : 1;       /*!< [17..17] Interrupt ack signal. */
      __IOM unsigned int COUNTER_1_IS_ZERO_L : 1; /*!< [18..18] Interrupt ack signal. */
      __IOM unsigned int COUNTER_1_IS_PEAK_L : 1; /*!< [19..19] Interrupt ack signal.   */
      __IOM unsigned int RESERVED2 : 12;          /*!< [31..20] Reserved2 */
    } CT_INTR_ACK_b;
  };

  union {
    __IOM unsigned int CT_MATCH_REG; /*!< (@ 0x00000018) Match value register */

    struct {
      __IOM unsigned int COUNTER_0_MATCH : 16; /*!< [15..0] This will be used as
                                               lower match */
      __IOM unsigned int COUNTER_1_MATCH : 16; /*!< [31..16] This will be used as
                                               upper match */
    } CT_MATCH_REG_b;
  };

  union {
    __IOM unsigned int CT_MATCH_BUF_REG; /*!< (@ 0x0000001C) Match Buffer register */

    struct {
      __IOM unsigned int COUNTER_0_MATCH_BUF : 16; /*!< [15..0] This gets copied to MATCH
                                        register if bug_reg_0_en is set.  Copying
                                        is done when counter 0 is active and hits
                                           0. */
      __IOM unsigned int COUNTER_1_MATCH_BUF : 16; /*!< [31..16] This gets copied to MATCH
                                        register if bug_reg_1_en is set. Copying
                                        is done when counter 1 is active and hits
                                           0. */
    } CT_MATCH_BUF_REG_b;
  };

  union {
    __IM unsigned int CT_CAPTURE_REG; /*!< (@ 0x00000020) Capture Register */

    struct {
      __IM unsigned int COUNTER_0_CAPTURE : 16; /*!< [15..0] This is a latched value of
                                      counter lower part when the selected
                                      capture_event occurs */
      __IM unsigned int COUNTER_1_CAPTURE : 16; /*!< [31..16] This is a latched value of
                                      counter upper part when the selected
                                      capture_event occurs */
    } CT_CAPTURE_REG_b;
  };

  union {
    __IOM unsigned int CT_COUNTER_REG; /*!< (@ 0x00000024) Counter Register */

    struct {
      __IM unsigned int COUNTER0 : 16; /*!< [15..0] This holds the value of counter-0 */
      __IM unsigned int COUNTER1 : 16; /*!< [31..16] This holds the value of counter-1 */
    } CT_COUNTER_REG_b;
  };

  union {
    __IOM unsigned int CT_OCU_CTRL_REG; /*!< (@ 0x00000028) OCU control register */

    struct {
      __IOM unsigned int OUTPUT_IS_OCU_0 : 1;        /*!< [0..0] Indicates whether the output is in
                                   OCU mode or not for channel-0 */
      __IOM unsigned int SYNC_WITH_0 : 3;            /*!< [3..1] Indicates whether the other channel is in
                               sync with this channel */
      __IOM unsigned int OCU_0_DMA_MODE : 1;         /*!< [4..4] Indicates whether the OCU DMA mode is
                                  active or not for channel 0 */
      __IOM unsigned int OCU_0_MODE_8_16 : 1;        /*!< [5..5] Indicates whether entire 16 bits or
                                   only 8-bits of the channel 0 are used in OCU
                                   mode */
      __IOM unsigned int MAKE_OUTPUT_0_HIGH_SEL : 3; /*!< [8..6] Check counter ocus for
                                          possibilities. When this is hit
                                           output will be made high. */
      __IOM unsigned int MAKE_OUTPUT_0_LOW_SEL : 3;  /*!< [11..9] Check counter ocus for
                                         possibilities. When this is hit output
                                         will be made low. */
      __IOM unsigned int RESERVED1 : 4;              /*!< [15..12] Reserved1  */
      __IOM unsigned int OUTPUT_1_IS_OCU : 1;        /*!< [16..16] Indicates whether the output is in
                                   OCU mode or not for channel 1 */
      __IOM unsigned int SYNC_WITH_1 : 3;            /*!< [19..17] Indicates whether the other channel is
                               in sync with this channel */
      __IOM unsigned int OCU_1_DMA_MODE : 1;         /*!< [20..20] Indicates whether the OCU DMA mode
                                  is active or not for channel 1 */
      __IOM unsigned int OCU_1_MODE_8_16_MODE : 1;   /*!< [21..21] Indicates whether entire 16
                                        bits or only 8-bits of
                                         the channel 1 are used in OCU mode */
      __IOM unsigned int MAKE_OUTPUT_1_HIGH_SEL : 3; /*!< [24..22] Check counter ocus for
                                          possibilities. When this is
                                           hit output will be made high. */
      __IOM unsigned int MAKE_OUTPUT_1_LOW_SEL : 3;  /*!< [27..25] Check counter ocus for
                                         possibilities. When this is hit output
                                         will be made low. */
      __IOM unsigned int RESERVED2 : 4;              /*!< [31..28] Reserved2  */
    } CT_OCU_CTRL_REG_b;
  };

  union {
    __IOM unsigned int CT_OCU_COMPARE_REG; /*!< (@ 0x0000002C) OCU Compare Register */

    struct {
      __IOM unsigned int OCU_COMPARE_0_REG : 16; /*!< [15..0] Holds the threshold value of
                                      present OCU period which denotes the number
                                      of clock  cycles for which the OCU output
                                           should be considered (counter 0) */
      __IOM unsigned int OCU_COMPARE_1_REG : 16; /*!< [31..16] Holds the threshold value of
                                      present OCU period which denotes the number
                                      of clock  cycles for which the OCU output
                                           should be considered (counter 1) */
    } CT_OCU_COMPARE_REG_b;
  };

  union {
    __IOM unsigned int CT_OCU_COMPARE2_REG; /*!< (@ 0x00000030) OCU Compare2 Register */

    struct {
      __IOM unsigned int OCU_COMPARE2_0_REG : 16; /*!< [15..0] Holds the threshold
                                                  value of present OCU period2
                                                  which denotes the number of
                                                  clock  cycles for which the OCU
                                                  output should be considered
                                                  (counter 0) */
      __IOM unsigned int OCU_COMPARE2_1_REG : 16; /*!< [31..16] Holds the threshold
                                                  value of present OCU period2
                                                  which denotes the number of
                                                  clock  cycles for which the OCU
                                                  output should be considered
                                                  (counter 1) */
    } CT_OCU_COMPARE2_REG_b;
  };

  union {
    __IOM unsigned int CT_OCU_SYNC_REG; /*!< (@ 0x00000034) OCU Synchronization Register */

    struct {
      __IOM unsigned int OCU_SYNC_CHANNEL0_REG : 16; /*!< [15..0] Starting point of channel 0
                                          for synchronization purpose */
      __IOM unsigned int OCU_SYNC_CHANNEL1_REG : 16; /*!< [31..16] Starting point of channel 1
                                          for synchronization purpose          */
    } CT_OCU_SYNC_REG_b;
  };

  union {
    __IOM unsigned int CT_OCU_COMPARE_NXT_REG; /*!< (@ 0x00000038) PWM compare next
                                               register */

    struct {
      __IOM unsigned int OCU_COMPARE_NXT_COUNTER1 : 16; /*!< [15..0] OCU output should be high
                                             for counter 1 */
      __IOM unsigned int OCU_COMPARE_NXT_COUNTER0 : 16; /*!< [31..16] PWM output should be high
                                             for counter 0 */
    } CT_OCU_COMPARE_NXT_REG_b;
  };

  union {
    __IOM unsigned int CT_WFG_CTRL_REG; /*!< (@ 0x0000003C) WFG control register */

    struct {
      __IOM unsigned int MAKE_OUTPUT_0_TGL_0_SEL : 3; /*!< [2..0] Check the counter ocus
                                           possibilities for description for
                                           channel 0. */
      __IOM unsigned int MAKE_OUTPUT_0_TGL_1_SEL : 3; /*!< [5..3] Check the counter ocus
                                                 possibilities for description       for
                                                 channel 0.       */
      __IOM unsigned int RESERVED1 : 2;               /*!< [7..6] Reserved1          */
      __IOM unsigned int WFG_TGL_CNT_0_PEAK : 8;      /*!< [15..8] WFG mode output toggle
                                                 count clock for channel 0. */
      __IOM unsigned int MAKE_OUTPUT_1_TGL_0_SEL : 3; /*!< [18..16] Check the counter ocus
                                           possibilities for description for
                                           channel 1. */
      __IOM unsigned int MAKE_OUTPUT_1_TGL_1_SEL : 3; /*!< [21..19] Check the counter ocus
                                           possibilities for description for
                                           channel 1. */
      __IOM unsigned int RESERVED2 : 2;               /*!< [23..22] Reserved2    */
      __IOM unsigned int WFG_TGL_CNT_1_PEAK : 8;      /*!< [31..24] WFG mode output toggle count
                                      clock for channel 1                 */
    } CT_WFG_CTRL_REG_b;
  };

  union {
    __IOM unsigned int CT_OCU_COMPARE2_NXT_REG; /*!< (@ 0x00000040) PWM compare next
                                                register */

    struct {
      __IOM unsigned int OCU_COMPARE2_NXT_COUNTER0 : 16; /*!< [15..0] OCU output should be high
                                              for counter 1 */
      __IOM unsigned int OCU_COMPARE2_NXT_COUNTER1 : 16; /*!< [31..16] PWM output should be
                                              high for counter 0 */
    } CT_OCU_COMPARE2_NXT_REG_b;
  };
  __IM unsigned int RESERVED[3];

  union {
    __IOM unsigned int CT_START_COUNTER_EVENT_SEL; /*!< (@ 0x00000050) Start counter
                                                   event select register */

    struct {
      __IOM unsigned int START_COUNTER_0_EVENT_SEL : 6; /*!< [5..0] For two 16 bit counters
                                           mode: Event select for starting the
                                           Counter 0  For 32 bit counter mode:
                                           Event select for starting counter */
      __IOM unsigned int RESERVED1 : 10;                /*!< [15..6] Reserved1     */
      __IOM unsigned int START_COUNTER_1_EVENT_SEL : 6; /*!< [21..16] For two 16 bit counters
                                             mode: Event select for starting the
                                             Counter 1.  For 32 bit counter mode:
                                             Invalid. Please refer to events
                                             table for description */
      __IM unsigned int RESERVED2 : 10;                 /*!< [31..22] Reserved2      */
    } CT_START_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_START_COUNTER_AND_EVENT; /*!< (@ 0x00000054) Start counter
                                                   AND event register */

    struct {
      __IOM unsigned int START_COUNTER_0_AND_EVENT : 4; /*!< [3..0] For two 16 bit counter
                                           mode: AND expression valids for AND
                                           event in start Counter 0 event For 32
                                           bit counter mode
                                           AND expression valids for AND event in
                                           start counter event                */
      __IOM unsigned int RESERVED1 : 4;                 /*!< [7..4] Reserved1      */
      __IOM unsigned int START_COUNTER_0_AND_VLD : 4;   /*!< [11..8] none */
      __IM unsigned int RESERVED2 : 4;                  /*!< [15..12] Reserved2                */
      __IOM unsigned int START_COUNTER_1_AND_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: AND expression valids for AND
                                           event in start counter event For 32
                                           bit counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                  /*!< [23..20] Reserved3       */
      __IOM unsigned int START_COUNTER_1_AND_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                  /*!< [31..28] Reserved4                */
    } CT_START_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM unsigned int CT_START_COUNTER_OR_EVENT; /*!< (@ 0x00000058) Start counter
                                                  OR event register */

    struct {
      __IOM unsigned int START_COUNTER_0_OR_EVENT : 4; /*!< [3..0] For two 16 bit counter mode:
                                           OR expression valids for OR event in
                                           start Counter 0 event For 32 bit
                                           counter mode
                                           OR expression valids for OR event in
                                           start counter event */
      __IOM unsigned int RESERVED1 : 4;                /*!< [7..4] Reserved1     */
      __IOM unsigned int START_COUNTER_0_OR_VLD : 4;   /*!< [11..8] none */
      __IOM unsigned int RESERVED2 : 4;                /*!< [15..12] Reserved2              */
      __IOM unsigned int START_COUNTER_1_OR_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: OR expression valids for OR
                                           event in start counter event For 32
                                           bit counter mode : Invalid. */
      __IM unsigned int RESERVED3 : 4;                 /*!< [23..20] Reserved3      */
      __IOM unsigned int START_COUNTER_1_OR_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                 /*!< [31..28] Reserved4               */
    } CT_START_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM unsigned int CT_CONTINUE_COUNTER_EVENT_SEL; /*!< (@ 0x0000005C) Continue counter
                                           event select register */

    struct {
      __IOM unsigned int CONTINUE_COUNTER_0_EVENT_SEL : 6; /*!< [5..0] For two 16 bit counters
                                                mode: Event select for continuing
                                                        the Counter 0 For 32 bit
                                                counter mode: Event select for
                                                        continuing counter */
      __IOM unsigned int RESERVED1 : 10;                   /*!< [15..6] Reserved1        */
      __IOM unsigned int CONTINUE_COUNTER_1_EVENT_SEL : 6; /*!< [21..16] For two 16 bit
                                                counters mode: Event select for
                                                continuing
                                                        the Counter 1  For 32 bit
                                                counter mode: Invalid. */
      __IM unsigned int RESERVED2 : 10;                    /*!< [31..22] Reserved2         */
    } CT_CONTINUE_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_CONTINUE_COUNTER_AND_EVENT; /*!< (@ 0x00000060) Continue counter AND
                                           event register */

    struct {
      __IOM unsigned int CONTINUE_COUNTER_0_AND_EVENT : 4; /*!< [3..0] For two 16 bit counter
                                                mode: AND expression valids for
                                                        AND event in continue
                                                Counter 0 event For 32 bit
                                                counter mode AND expression
                                                valids for AND event in continue
                                                counter event. */
      __IOM unsigned int RESERVED1 : 4;                    /*!< [7..4] Reserved1         */
      __IOM unsigned int CONTINUE_COUNTER_0_AND_VLD : 4;   /*!< [11..8] none */
      __IOM unsigned int RESERVED2 : 4;                    /*!< [15..12] Reserved2                  */
      __IOM unsigned int CONTINUE_COUNTER_1_AND_EVENT : 4; /*!< [19..16] For two 16 bit
                                                counters mode: AND expression
                                                valids for AND event in continue
                                                counter event For 32 bit counter
                                                        mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                     /*!< [23..20] Reserved3          */
      __IOM unsigned int CONTINUE_COUNTER_1_AND_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                     /*!< [31..28] Reserved4                   */
    } CT_CONTINUE_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM unsigned int CT_CONTINUE_COUNTER_OR_EVENT; /*!< (@ 0x00000064) Continue counter OR
                                          event register */

    struct {
      __IOM unsigned int CONTINUE_COUNTER_0_OR_EVENT : 4; /*!< [3..0] For two 16 bit counter
                                           mode: OR expression valids for OR
                                           event in continue Counter 0 event For
                                           32 bit counter mode OR expression
                                           valids for OR event in continue
                                           counter event */
      __IOM unsigned int RESERVED1 : 4;                   /*!< [7..4] Reserved1        */
      __IOM unsigned int CONTINUE_COUNTER_0_OR_VLD : 4;   /*!< [11..8] none */
      __IOM unsigned int RESERVED2 : 4;                   /*!< [15..12] Reserved2                 */
      __IOM unsigned int CONTINUE_COUNTER_1_OR_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: OR expression valids for OR
                                           event in continue counter event  For
                                           32 bit counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                    /*!< [23..20] Reserved3         */
      __IOM unsigned int CONTINUE_COUNTER_1_OR_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                    /*!< [31..28] Reserved4                  */
    } CT_CONTINUE_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM unsigned int CT_STOP_COUNTER_EVENT_SEL; /*!< (@ 0x00000068) Stop counter
                                                  event select register */

    struct {
      __IOM unsigned int STOP_COUNTER_0_EVENT_SEL : 6; /*!< [5..0] For two 16 bit counters
                                           mode: Event select for Stopping the
                                           Counter 0 For 32 bit counter mode:
                                           Event select for Stopping counter */
      __IOM unsigned int RESERVED1 : 10;               /*!< [15..6] Reserved1    */
      __IOM unsigned int STOP_COUNTER_1_EVENT_SEL : 6; /*!< [21..16] For two 16 bit counters
                                            mode: Event select for Stopping the
                                            Counter 1 For 32 bit counter mode:
                                            Invalid                            */
      __IM unsigned int RESERVED2 : 10;                /*!< [31..22] Reserved2     */
    } CT_STOP_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_STOP_COUNTER_AND_EVENT; /*!< (@ 0x0000006C) Stop counter
                                                  AND event register */

    struct {
      __IOM unsigned int STOP_COUNTER_0_AND_EVENT : 4; /*!< [3..0] For two 16 bit counter mode:
                                           AND expression valids for AND event in
                                           stop Counter 0 event  For 32 bit
                                           counter mode
                                           AND expression valids for AND event in
                                           stop counter event                 */
      __IOM unsigned int RESERVED1 : 4;                /*!< [7..4] Reserved1     */
      __IOM unsigned int STOP_COUNTER_0_AND_VLD : 4;   /*!< [11..8] Indicates which
                                                     bits in 3:0 are valid for
                                                     considering AND event */
      __IOM unsigned int RESERVED2 : 4;                /*!< [15..12] Reserved2              */
      __IOM unsigned int STOP_COUNTER_1_AND_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: AND expression valids for AND
                                           event in stop counter event For 32 bit
                                           counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                 /*!< [23..20] Reserved3      */
      __IOM unsigned int STOP_COUNTER_1_AND_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                 /*!< [31..28] Reserved4               */
    } CT_STOP_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM unsigned int CT_STOP_COUNTER_OR_EVENT; /*!< (@ 0x00000070) Stop counter OR
                                                 event register */

    struct {
      __IOM unsigned int STOP_COUNTER_0_OR_EVENT : 4; /*!< [3..0] For two 16 bit counter mode:
                                           OR expression valids for OR event in
                                           Stop Counter 0 event For 32 bit
                                           counter mode
                                           OR expression valids for OR event in
                                           Stop counter event */
      __IOM unsigned int RESERVED1 : 4;               /*!< [7..4] Reserved1    */
      __IOM unsigned int STOP_COUNTER_0_OR_VLD : 4;   /*!< [11..8] none */
      __IOM unsigned int RESERVED2 : 4;               /*!< [15..12] Reserved2             */
      __IOM unsigned int STOP_COUNTER_1_OR_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: OR expression valids for OR
                                           event in Stop counter event For 32 bit
                                           counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                /*!< [23..20] Reserved3     */
      __IOM unsigned int STOP_COUNTER_1_OR_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                /*!< [31..28] Reserved4              */
    } CT_STOP_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM unsigned int CT_HALT_COUNTER_EVENT_SEL; /*!< (@ 0x00000074) Halt counter
                                                  event select register */

    struct {
      __IOM unsigned int HALT_COUNTER_0_EVENT_SEL : 6;  /*!< [5..0] For two 16 bit counters
                                          mode: Event select for Halting the
                                          Counter 0 For 32 bit counter mode:
                                          Event select for Halting counter */
      __OM unsigned int RESUME_FROM_HALT_COUNTER_0 : 1; /*!< [6..6] For two 16 bit counters
                                            mode: Event select for Halting the
                                            Counter 0 For 32 bit counter mode:
                                            Event select for Halting counter */
      __IM unsigned int RESERVED1 : 9;                  /*!< [15..7] Reserved1        */
      __IOM unsigned int HALT_COUNTER_1_EVENT_SEL : 6;  /*!< [21..16] For two 16 bit counters
                                           mode: Event select for Halting the
                                           Counter 1 For 32 bit counter mode:
                                           Invalid                            */
      __OM unsigned int RESUME_FROM_HALT_COUNTER_1 : 1; /*!< [22..22] For two 16 bit
                                                  counters mode: Event select for
                                                  Halting the Counter 0 For 32
                                                  bit counter mode: Event select
                                                  for Halting counter */
      __IM unsigned int RESERVED2 : 9;                  /*!< [31..23] Reserved2            */
    } CT_HALT_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_HALT_COUNTER_AND_EVENT; /*!< (@ 0x00000078) Halt counter
                                                  AND event register */

    struct {
      __IOM unsigned int HALT_COUNTER_0_AND_EVENT : 4; /*!< [3..0] For two 16 bit counter mode:
                                           AND expression valids for AND event in
                                           stop Counter 0 event For 32 bit
                                           counter mode
                                           AND expression valids for AND event in
                                           stop counter event                 */
      __IOM unsigned int RESERVED1 : 4;                /*!< [7..4] Reserved1     */
      __IOM unsigned int HALT_COUNTER_0_AND_VLD : 4;   /*!< [11..8] Indicates which
                                                     bits in 3:0 are valid for
                                                     considering AND event */
      __IM unsigned int RESERVED2 : 4;                 /*!< [15..12] Reserved2               */
      __IOM unsigned int HALT_COUNTER_1_AND_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: AND expression valids for AND
                                           event in stop counter event For 32 bit
                                           counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                 /*!< [23..20] Reserved3      */
      __IOM unsigned int HALT_COUNTER_1_AND_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                 /*!< [31..28] Reserved4               */
    } CT_HALT_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM unsigned int CT_HALT_COUNTER_OR_EVENT; /*!< (@ 0x0000007C) Halt counter OR
                                                 event register */

    struct {
      __IOM unsigned int HALT_COUNTER_0_OR_EVENT : 4; /*!< [3..0] For two 16 bit counter mode:
                                           OR expression valids for OR event in
                                           Halt Counter 0 event For 32 bit
                                           counter mode
                                           OR expression valids for OR event in
                                           Halt counter event */
      __IOM unsigned int RESERVED1 : 4;               /*!< [7..4] Reserved1    */
      __IOM unsigned int HALT_COUNTER_0_OR_VLD : 4;   /*!< [11..8] none */
      __IM unsigned int RESERVED2 : 4;                /*!< [15..12] Reserved2              */
      __IOM unsigned int HALT_COUNTER_1_OR_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: OR expression valids for OR
                                           event in Halt counter event For 32 bit
                                           counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                /*!< [23..20] Reserved3     */
      __IOM unsigned int HALT_COUNTER_1_OR_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                /*!< [31..28] Reserved4              */
    } CT_HALT_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM unsigned int CT_INCREMENT_COUNTER_EVENT_SEL; /*!< (@ 0x00000080) Increment counter
                                            event select register */

    struct {
      __IOM unsigned int INCREMENT_COUNTER_0_EVENT_SEL : 6; /*!< [5..0] For two 16 bit counters
                                                 mode: Event select for
                                                 Incrementing the Counter 0 For
                                                 32 bit counter mode: Event
                                                 select for
                                                        Incrementing counter */
      __IM unsigned int RESERVED1 : 10;                     /*!< [15..6] Reserved1          */
      __IOM unsigned int INCREMENT_COUNTER_1_EVENT_SEL : 6; /*!< [21..16] For two 16
                                                            bit counters mode:
                                                            Event select for
                                                            Incrementing the
                                                            Counter 1 For 32 bit
                                                            counter mode: Invalid
                                                          */
      __IM unsigned int RESERVED2 : 10;                     /*!< [31..22] Reserved2 */
    } CT_INCREMENT_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_INCREMENT_COUNTER_AND_EVENT; /*!< (@ 0x00000084) Increment counter
                                            AND event register */

    struct {
      __IOM unsigned int INCREMENT_COUNTER_0_AND_EVENT : 4; /*!< [3..0] For two 16 bit counter
                                                 mode: AND expression valids for
                                                        AND event in stop Counter
                                                 0 event For 32 bit counter mode
                                                        AND expression valids for
                                                 AND event in stop counter event
                                               */
      __IOM unsigned int RESERVED1 : 4;                     /*!< [7..4] Reserved1          */
      __IOM unsigned int INCREMENT_COUNTER_0_AND_VLD : 4;   /*!< [11..8] none */
      __IM unsigned int RESERVED2 : 4;                      /*!< [15..12] Reserved2                    */
      __IOM unsigned int INCREMENT_COUNTER_1_AND_EVENT : 4; /*!< [19..16] For two 16 bit
                                                 counters mode: AND expression
                                                 valids for AND event in stop
                                                 counter event For 32 bit counter
                                                        mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                      /*!< [23..20] Reserved3           */
      __IOM unsigned int INCREMENT_COUNTER_1_AND_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                      /*!< [31..28] Reserved4                    */
    } CT_INCREMENT_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM unsigned int CT_INCREMENT_COUNTER_OR_EVENT; /*!< (@ 0x00000088) Increment counter OR
                                           event register */

    struct {
      __IOM unsigned int INCREMENT_COUNTER_0_OR_EVENT : 4; /*!< [3..0] For two 16 bit counter
                                                mode: OR expression valids for OR
                                                event in Increment Counter 0
                                                event For 32 bit counter mode OR
                                                expression valids for OR event in
                                                Increment counter event */
      __IOM unsigned int RESERVED1 : 4;                    /*!< [7..4] Reserved1         */
      __IOM unsigned int INCREMENT_COUNTER_0_OR_VLD : 4;   /*!< [11..8] none */
      __IM unsigned int RESERVED2 : 4;                     /*!< [15..12] Reserved2                   */
      __IOM unsigned int INCREMENT_COUNTER_1_OR_EVENT : 4; /*!< [19..16] For two 16 bit
                                           counters mode: OR expression valids
                                           for OR event in Increment counter
                                           event For 32 bit counter mode :
                                           Invalid */
      __IM unsigned int RESERVED4 : 4;                     /*!< [23..20] Reserved4          */
      __IOM unsigned int INCREMENT_COUNTER_1_OR_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED5 : 4;                     /*!< [31..28] Reserved5                   */
    } CT_INCREMENT_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM unsigned int CT_CAPTURE_COUNTER_EVENT_SEL; /*!< (@ 0x0000008C) Capture counter event
                                          select register                      */

    struct {
      __IOM unsigned int CAPTURE_COUNTER_0_EVENT_SEL : 6; /*!< [5..0] For two 16 bit counters
                                               mode: Event select for Capturing
                                                        the Counter 0 For 32 bit
                                               counter mode: Event select for
                                                        Capturing counter */
      __IM unsigned int RESERVED1 : 10;                   /*!< [15..6] Reserved1        */
      __IOM unsigned int CAPTURE_COUNTER_1_EVENT_SEL : 6; /*!< [21..16] For two 16 bit counters
                                               mode: Event select for Capturing
                                                        the Counter 1 For 32 bit
                                               counter mode : Invalid */
      __IM unsigned int RESERVED2 : 10;                   /*!< [31..22] Reserved2        */
    } CT_CAPTURE_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_CAPTURE_COUNTER_AND_EVENT; /*!< (@ 0x00000090) Capture counter AND
                                          event register */

    struct {
      __IOM
      unsigned int CAPTURE_COUNTER_0_AND_EVENT : 4;       /*!< [3..0] For two 16 bit
                                                        counter mode: AND expression
                                                        valids for      AND event in stop
                                                        Counter 0 event For 32 bit
                                                        counter mode      AND expression
                                                        valids for AND event in stop
                                                        counter event      */
      __IOM unsigned int RESERVED1 : 4;                   /*!< [7..4] Reserved1                 */
      __IOM unsigned int CAPTURE_COUNTER_0_AND_VLD : 4;   /*!< [11..8] none */
      __IM unsigned int RESERVED2 : 4;                    /*!< [15..12] Reserved2                  */
      __IOM unsigned int CAPTURE_COUNTER_1_AND_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: AND expression valids for AND
                                           event in stop counter event For 32 bit
                                           counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                    /*!< [23..20] Reserved3         */
      __IOM unsigned int CAPTURE_COUNTER_1_AND_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                    /*!< [31..28] Reserved4                  */
    } CT_CAPTURE_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM unsigned int CT_CAPTURE_COUNTER_OR_EVENT; /*!< (@ 0x00000094) Capture counter OR
                                         event register */

    struct {
      __IOM unsigned int CAPTURE_COUNTER_0_OR_EVENT : 4; /*!< [3..0] For two 16 bit counter
                                           mode: OR expression valids for OR
                                           event in Capture Counter 0 event For
                                           32 bit counter mode OR expression
                                           valids for OR event in Capture counter
                                           event */
      __IOM unsigned int RESERVED1 : 4;                  /*!< [7..4] Reserved1       */
      __IOM unsigned int CAPTURE_COUNTER_0_OR_VLD : 4;   /*!< [11..8] none */
      __IM unsigned int RESERVED2 : 4;                   /*!< [15..12] Reserved2                 */
      __IOM unsigned int CAPTURE_COUNTER_1_OR_EVENT : 4; /*!< [19..16] For two 16 bit counters
                                           mode: OR expression valids for OR
                                           event in Capture counter event For 32
                                           bit counter mode : Invalid */
      __IM unsigned int RESERVED3 : 4;                   /*!< [23..20] Reserved3        */
      __IOM unsigned int CAPTURE_COUNTER_1_OR_VLD : 4;   /*!< [27..24] none */
      __IM unsigned int RESERVED4 : 4;                   /*!< [31..28] Reserved4                 */
    } CT_CAPTURE_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM unsigned int CT_OUTPUT_EVENT_SEL; /*!< (@ 0x00000098) Output event select
                                            register */

    struct {
      __IOM unsigned int OUTPUT_EVENT_SEL_0 : 6; /*!< [5..0] For two 16 bit counters mode:
                                        Event select for output   event from Counter
                                        0 For 32 bit counter mode: Event select
                                             for output event   */
      __IM unsigned int RESERVED1 : 10;          /*!< [15..6] Reserved1 */
      __IOM unsigned int OUTPUT_EVENT_SEL_1 : 6; /*!< [21..16] For two 16 bit counters mode:
                                        Event select for output
                                             event from counter 1 For 32 bit
                                        counter mode : Invalid   */
      __IM unsigned int RESERVED2 : 10;          /*!< [31..22] Reserved2 */
    } CT_OUTPUT_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_OUTPUT_AND_EVENT_REG; /*!< (@ 0x0000009C) Output AND event
                                                Register */

    struct {
      __IOM unsigned int OUTPUT_0_AND_EVENT : 4; /*!< [3..0] AND expression for AND event in
                                        output Counter_0 event.            */
      __IOM unsigned int RESERVED1 : 4;          /*!< [7..4] Reserved1 */
      __IOM unsigned int OUTPUT_0_AND_VLD : 4;   /*!< [11..8] AND expression for AND event in
                                       output Counter_0 event.           */
      __IM unsigned int RESERVED2 : 4;           /*!< [15..12] Reserved2 */
      __IOM unsigned int OUTPUT_1_AND_EVENT : 4; /*!< [19..16] AND expression for AND event in
                                       output Counter_1 event.          */
      __IM unsigned int RESERVED3 : 4;           /*!< [23..20] Reserved3 */
      __IOM unsigned int OUTPUT_1_AND_VLD : 4;   /*!< [27..24] AND expression for AND event in
                                       output Counter_1 event.          */
      __IM unsigned int RESERVED4 : 4;           /*!< [31..28] Reserved4 */
    } CT_OUTPUT_AND_EVENT_REG_b;
  };

  union {
    __IOM unsigned int CT_OUTPUT_OR_EVENT; /*!< (@ 0x000000A0) Output OR event Register */

    struct {
      __IOM unsigned int OUTPUT_0_OR_EVENT : 4; /*!< [3..0] OR expression for OR event in
                                        output Counter_0 event               */
      __IOM unsigned int RESERVED1 : 4;         /*!< [7..4] Reserved1 */
      __IOM unsigned int OUTPUT_0_OR_VLD : 4;   /*!< [11..8] Indicates which bits in 3:0 are
                                       valid for considering     OR event     */
      __IM unsigned int RESERVED2 : 4;          /*!< [15..12] Reserved2 */
      __IOM unsigned int OUTPUT_1_OR_EVENT : 4; /*!< [19..16] OR expression for OR event in
                                       output Counter_0 event             */
      __IM unsigned int RESERVED3 : 4;          /*!< [23..20] Reserved3 */
      __IOM unsigned int OUTPUT_1_OR_VLD : 4;   /*!< [27..24] Indicates which bits in 3:0 are
                                       valid for considering     OR event     */
      __IM unsigned int RESERVED4 : 4;          /*!< [31..28] Reserved4 */
    } CT_OUTPUT_OR_EVENT_b;
  };

  union {
    __IOM unsigned int CT_INTR_EVENT_SEL; /*!< (@ 0x000000A4) Interrupt Event Select
                                          Register                            */

    struct {
      __IOM unsigned int INTR_EVENT_SEL_0 : 6; /*!< [5..0] For two 16 bit counters mode: Event
                                    select for interrupt event from Counter 0 For
                                    32 bit counter mode: Event select
                                           for output event */
      __IM unsigned int RESERVED1 : 10;        /*!< [15..6] Reserved1        */
      __IOM unsigned int INTR_EVENT_SEL_1 : 6; /*!< [21..16] For two 16 bit counters
                                               mode: Event select for interrupt
                                                      event from counter 1 For 32
                                               bit counter mode : Invalid */
      __IM unsigned int RESERVED2 : 10;        /*!< [31..22] Reserved2        */
    } CT_INTR_EVENT_SEL_b;
  };

  union {
    __IOM unsigned int CT_INTR_AND_EVENT; /*!< (@ 0x000000A8) Interrupt AND Event Register */

    struct {
      __IOM unsigned int INTR_0_AND_EVENT : 4; /*!< [3..0] None */
      __IOM unsigned int RESERVED1 : 4;        /*!< [7..4] Reserved1        */
      __IOM unsigned int INTR_0_AND_VLD : 4;   /*!< [11..8] None   */
      __IM unsigned int RESERVED2 : 4;         /*!< [15..12] Reserved2         */
      __IOM unsigned int INTR_1_AND_EVENT : 4; /*!< [19..16] None */
      __IM unsigned int RESERVED3 : 4;         /*!< [23..20] Reserved3         */
      __IOM unsigned int INTR_1_AND_VLD : 4;   /*!< [27..24] None   */
      __IM unsigned int RESERVED4 : 4;         /*!< [31..28] Reserved4         */
    } CT_INTR_AND_EVENT_b;
  };

  union {
    __IOM unsigned int CT_INTR_OR_EVENT_REG; /*!< (@ 0x000000AC) Interrupt OR Event
                                             Register */

    struct {
      __IOM unsigned int INTR_0_OR_EVENT : 4; /*!< [3..0] None */
      __IOM unsigned int RESERVED1 : 4;       /*!< [7..4] Reserved1       */
      __IOM unsigned int INTR_0_OR_VLD : 4;   /*!< [11..8] None   */
      __IM unsigned int RESERVED2 : 4;        /*!< [15..12] Reserved2        */
      __IOM unsigned int INTR_1_OR_EVENT : 4; /*!< [19..16] None */
      __IM unsigned int RESERVED3 : 4;        /*!< [23..20] Reserved3        */
      __IOM unsigned int INTR_1_OR_VLD : 4;   /*!< [27..24] None   */
      __IM unsigned int RESERVED4 : 4;        /*!< [31..28] Reserved4        */
    } CT_INTR_OR_EVENT_REG_b;
  };
} CT0_Type; /*!< Size = 176 (0xb0) */

/* ===========================================================================================================================
  */
/* ================                                        CT_MUX_REG
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief Configurable timer is used in counting clocks, events and states with
   reference clock external clock and system clock (CT_MUX_REG)
   */

typedef struct { /*!< (@ 0x4506F000) CT_MUX_REG Structure */

  union {
    __IOM unsigned int CT_MUX_SEL_0_REG; /*!< (@ 0x00000000) MUX_SEL_0_REG Register */

    struct {
      __IOM unsigned int MUX_SEL_0 : 4;  /*!< [3..0] Select value to select first output value
                             fifo_0_full[0] out of all the fifo_0_full_muxed
                             signals of counter 0                     */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved1 */
    } CT_MUX_SEL_0_REG_b;
  };

  union {
    __IOM unsigned int CT_MUX_SEL_1_REG; /*!< (@ 0x00000004) MUX_SEL_1_REG Register */

    struct {
      __IOM unsigned int MUX_SEL_1 : 4;  /*!< [3..0] Select value to select first output value
                             fifo_0_full[1] out of all the fifo_0_full_muxed
                             signals of counter 0                     */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved1 */
    } CT_MUX_SEL_1_REG_b;
  };

  union {
    __IOM unsigned int CT_MUX_SEL_2_REG; /*!< (@ 0x00000008) MUX_SEL_2_REG Register */

    struct {
      __IOM unsigned int MUX_SEL_2 : 4;  /*!< [3..0] Select value to select first output value
                             fifo_1_full[0] out of all the fifo_1_full_muxed
                             signals of counter 1                     */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved1 */
    } CT_MUX_SEL_2_REG_b;
  };

  union {
    __IOM unsigned int CT_MUX_SEL_3_REG; /*!< (@ 0x0000000C) MUX_SEL_3_REG Register */

    struct {
      __IOM unsigned int MUX_SEL_3 : 4;  /*!< [3..0] Select value to select first output value
                             fifo_1_full[1] out of all the fifo_1_full_muxed
                             signals of counter 1                     */
      __IOM unsigned int RESERVED1 : 28; /*!< [31..4] Reserved1 */
    } CT_MUX_SEL_3_REG_b;
  };
  __IM unsigned int RESERVED[2];

  union {
    __IOM unsigned int CT_OUTPUT_EVENT1_ADC_SEL; /*!< (@ 0x00000018) OUTPUT_EVENT_ADC_SEL
                                      Register                              */

    struct {
      __IOM
      unsigned int OUTPUT_EVENT_ADC_SEL : 4; /*!< [3..0] Select signals to select one
                                            output event out of all the output
                                            events output_event_0 output_event_1,
                                            output_event_2, output_event_3 to
                                            enable ADC module */
      __IOM unsigned int RESERVED1 : 28;     /*!< [31..4] Reserved1    */
    } CT_OUTPUT_EVENT1_ADC_SEL_b;
  };

  union {
    __IOM unsigned int CT_OUTPUT_EVENT2_ADC_SEL; /*!< (@ 0x0000001C) OUTPUT_EVENT_ADC_SEL
                                      Register                              */

    struct {
      __IOM
      unsigned int OUTPUT_EVENT_ADC_SEL : 4; /*!< [3..0] Select signals to select one
                                            output event out of all the output
                                            events output_event_0 output_event_1,
                                            output_event_2, output_event_3 to
                                            enable ADC module */
      __IOM unsigned int RESERVED1 : 28;     /*!< [31..4] Reserved1    */
    } CT_OUTPUT_EVENT2_ADC_SEL_b;
  };
} CT_MUX_REG_Type; /*!< Size = 32 (0x20) */

/* ===========================================================================================================================
  */
/* ================                                           EGPIO
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief ENHANCED GENERAL PERPOSE INPUT/OUTPUT (EGPIO)
  */

typedef struct {                              /*!< (@ 0x46130000) EGPIO Structure */
  __IOM EGPIO_PIN_CONFIG_Type PIN_CONFIG[80]; /*!< (@ 0x00000000) [0..79] */
  __IM unsigned int RESERVED[704];
  __IOM EGPIO_PORT_CONFIG_Type PORT_CONFIG[6]; /*!< (@ 0x00001000) [0..5] */
  __IM unsigned int RESERVED1[80];
  __IOM EGPIO_INTR_Type INTR[8];                   /*!< (@ 0x00001200) [0..7] */
  __IOM EGPIO_GPIO_GRP_INTR_Type GPIO_GRP_INTR[4]; /*!< (@ 0x00001240) [0..3] */
} EGPIO_Type;                                      /*!< Size = 4704 (0x1260)           */

/* ===========================================================================================================================
  */
/* ================                                           SDIO0
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The Secure Digital I/O (SDIO) Slave module implements the
  * functionality of the SDIO card based on the SDIO specifications version 2.0.
  * (SDIO0)
  */

typedef struct { /*!< (@ 0x40000000) SDIO0 Structure */

  union {
    __IOM unsigned int SDIO_INTR_FN1_STATUS_CLEAR_REG; /*!< (@ 0x00000000) SDIO Function1
                                            Interrupt Enable Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_CLR : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt. =1
                                                      Interrupt is enabled =0 -
                                              Interrupt is disabled */
      __IOM unsigned int SDIO_RD_INT_CLR : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_INT_CLR : 1;     /*!< [2..2] This bit is used to
                                                 enable CMD53 CSA interrupt   */
      __IOM unsigned int SDIO_CMD52_INT_CLR : 1;   /*!< [3..3] This bit is used to
                                                 enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_CLR : 1; /*!< [4..4] This bit is used to enable power
                                        level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_CLR : 1; /*!< [5..5] This bit is used to enable CRC
                                        error interrupt                     */
      __IOM unsigned int SDIO_ABORT_INT_CLR : 1;   /*!< [6..6] This bit is used to
                                                 enable abort interrupt */
      __IOM unsigned int SDIO_TOUT_INT_CLR : 1;    /*!< [7..7] This bit is used to enable ?read
                                     FIFO wait time over? interrupt */
      __IOM unsigned int SDIO_WR_RDZ : 1;          /*!< [8..8] SDIO_WR_RDZ */
      __IOM unsigned int SDIO_CSA_ACCESS : 1;      /*!< [9..9] csa_window_access When set, indicates
                                   that current request is for CSA window
                                   register. This is only status signal */
      __IOM unsigned int RES : 22;                 /*!< [31..10] reserved1 */
    } SDIO_INTR_FN1_STATUS_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN1_ENABLE_REG; /*!< (@ 0x00000004) SDIO Function1
                                                 Interrupt Enable Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_EN : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt.  */
      __IOM unsigned int SDIO_RD_INT_EN : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt  */
      __IOM unsigned int SDIO_CSA_INT_EN : 1;     /*!< [2..2] This bit is used to enable
                                              CMD53 CSA interrupt */
      __IOM unsigned int SDIO_CMD52_INT_EN : 1;   /*!< [3..3] This bit is used to
                                                enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_EN : 1; /*!< [4..4] This bit is used to enable power
                                       level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_EN : 1; /*!< [5..5] This bit is used to
                                                  enable CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_INT_EN : 1;   /*!< [6..6] This bit is used to
                                                  enable abort interrupt   */
      __IOM unsigned int SDIO_TOUT_INT_EN : 1;    /*!< [7..7] This bit is used to enable ?read
                                    FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                /*!< [31..8] reserved5  */
    } SDIO_INTR_FN1_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN1_MASK_REG; /*!< (@ 0x00000008) SDIO Function1
                                               Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_MSK : 1;  /*!< [0..0] This bit is used to mask
                                              CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_MSK : 1;  /*!< [1..1] This bit is used to mask
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_MSK : 1;     /*!< [2..2] This bit is used to mask CMD53 CSA
                                interrupt.Setting this bit will mask the
                                interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_MSK : 1;   /*!< [3..3] This bit is used to mask
                                               CMD52 interrupt   */
      __IOM unsigned int SDIO_PWR_LEV_MSK : 1; /*!< [4..4] This bit is used to mask
                                               power level change interrupt */
      __IOM unsigned int SDIO_CRC_ERR_MSK : 1; /*!< [5..5] This bit is used to mask
                                               CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_MSK : 1;   /*!< [6..6] This bit is used to mask abort
                                  interrupt Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_TOUT_MSK : 1;    /*!< [7..7] This bit is used to mask read FIFO wait
                                 time over interrupt        */
      __IOM unsigned int RES : 24;             /*!< [31..8] reserved5 */
    } SDIO_INTR_FN1_MASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN1_UNMASK_REG; /*!< (@ 0x0000000C) SDIO Function1
                                                 Interrupt UnMask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_UNMSK : 1;  /*!< [0..0] This bit is used to
                                                unmask CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_UNMSK : 1;  /*!< [1..1] This bit is used to
                                                unmask CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_UNMSK : 1;     /*!< [2..2] This bit is used to unmask CMD53 CSA
                                  interrupt.Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_UNMSK : 1;   /*!< [3..3] This bit is used to
                                               unmask CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_UNMSK : 1; /*!< [4..4] This bit is used to unmask power
                                      level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_UNMSK : 1; /*!< [5..5] This bit is used to
                                                 unmask CRC error interrupt */
      __IOM
      unsigned int SDIO_ABORT_UNMSK : 1;      /*!< [6..6] This bit is used to unmask
                                             abort interrupt Setting this bit
                                             will mask the interrupt Clearing
                                             this bit has no effect */
      __IOM unsigned int SDIO_TOUT_UNMSK : 1; /*!< [7..7] This bit is used to unmask read FIFO
                                   wait time over interrupt      */
      __IOM unsigned int RES : 24;            /*!< [31..8] reserved5 */
    } SDIO_INTR_FN1_UNMASK_REG_b;
  };

  union {
    __IM unsigned int SDIO_BLK_LEN_REG; /*!< (@ 0x00000010) SDIO Block Length Register */

    struct {
      __IM unsigned int SDIO_BLK_LEN : 12; /*!< [11..0] Length of each block for the
                                           last received CMD53 */
      __IM unsigned int RES : 20;          /*!< [31..12] reserved5          */
    } SDIO_BLK_LEN_REG_b;
  };

  union {
    __IM unsigned int SDIO_BLK_CNT_REG; /*!< (@ 0x00000014) SDIO Block Length Register */

    struct {
      __IM unsigned int SDIO_BLK_CNT : 9; /*!< [8..0] Block count for the last
                                          received CMD53 */
      __IM unsigned int RES : 23;         /*!< [31..9] reserved5         */
    } SDIO_BLK_CNT_REG_b;
  };

  union {
    __IM unsigned int SDIO_ADDRESS_REG; /*!< (@ 0x00000018) SDIO Address Register */

    struct {
      __IM unsigned int SDIO_ADDR : 16; /*!< [15..0] Lower 16-bits of the 17-bit address field
                              in the last received CMD53 */
      __IM unsigned int RES : 16;       /*!< [31..16] reserved5 */
    } SDIO_ADDRESS_REG_b;
  };
  __IOM unsigned int SDIO_CMD52_RDATA_REGISTER; /*!< (@ 0x0000001C) SDIO CMD52 RDATA
                                                Register */
  __IOM unsigned int SDIO_CMD52_WDATA_REGISTER; /*!< (@ 0x00000020) SDIO CMD52 WDATA
                                                Register */

  union {
    __IM unsigned int SDIO_INTR_STATUS_REG; /*!< (@ 0x00000024) SDIO Interrupt
                                            Status Register */

    struct {
      __IM unsigned int SDIO_INT_ERROR : 1; /*!< [0..0] Interrupt is pending because of error
                                  condition from any of the functions */
      __IM unsigned int SDIO_INT_FN1 : 1;   /*!< [1..1] Interrupt is pending for function1 */
      __IM unsigned int SDIO_INT_FN2 : 1;   /*!< [2..2] Interrupt is pending for function2 */
      __IM unsigned int SDIO_INT_FN3 : 1;   /*!< [3..3] Interrupt is pending for function3 */
      __IM unsigned int SDIO_INT_FN4 : 1;   /*!< [4..4] Interrupt is pending for function4 */
      __IM unsigned int SDIO_INT_FN5 : 1;   /*!< [5..5] Interrupt is pending for function5   */
      __IM unsigned int RES : 26;           /*!< [31..6] reserved5 */
    } SDIO_INTR_STATUS_REG_b;
  };

  union {
    __IM unsigned int SDIO_INTR_FN_NUMBER_REG; /*!< (@ 0x00000028) SDIO Interrupt
                                               Function Number Register */

    struct {
      __IM unsigned int SDIO_INTR_FN_NUM : 3; /*!< [2..0] Indicates the function number to
                                    which interrupt is pending.       */
      __IM unsigned int RES : 29;             /*!< [31..3] reserved5   */
    } SDIO_INTR_FN_NUMBER_REG_b;
  };

  union {
    __IM unsigned int SDIO_FIFO_STATUS_REG; /*!< (@ 0x0000002C) SDIO FIFO Status Register */

    struct {
      __IM unsigned int SDIO_WFIFO_FULL : 1;        /*!< [0..0] When set, indicates that
                                              WFIFO is full WFIFO is used
                                                       in SDIO reads from host for
                                              sending data from AHB to Host  */
      __IM unsigned int SDIO_WFIFO_AFULL : 1;       /*!< [1..1] When set, indicates that
                                              WFIFO is almost full */
      __IM unsigned int SDIO_RFIFO_EMPTY : 1;       /*!< [2..2] When set, indicates that RFIFO is
                                    empty RFIFO is used in SDIO writes from host
                                    for sending data from host to AHB */
      __IM unsigned int SDIO_RFIFO_AEMPTY : 1;      /*!< [3..3] When set, indicates that
                                               RFIFO is almost empty */
      __IM unsigned int SDIO_CURRENT_FN_NUM : 3;    /*!< [6..4] Indicates the function number of
                                       the last received command         */
      __IM unsigned int SDIO_BUS_CONTROL_STATE : 5; /*!< [11..7] Indicates the function number
                                          of the last received command        */
      __IM unsigned int RES : 20;                   /*!< [31..12] reserved5         */
    } SDIO_FIFO_STATUS_REG_b;
  };

  union {
    __IM unsigned int SDIO_FIFO_OCC_REG; /*!< (@ 0x00000030) SDIO FIFO Occupancy Register */

    struct {
      __IM unsigned int SDIO_WFIFO_OCC : 8;   /*!< [7..0] Indicates the occupancy
                                              level of the write FIFO   */
      __IM unsigned int SDIO_RFIFO_AVAIL : 8; /*!< [15..8] Indicates the available
                                              space in the read FIFO */
      __IM unsigned int RES : 16;             /*!< [31..16] reserved5             */
    } SDIO_FIFO_OCC_REG_b;
  };

  union {
    __IOM unsigned int SDIO_HOST_INTR_SET_REG; /*!< (@ 0x00000034) SDIO Host
                                               Interrupt Set Register */

    struct {
      __IOM unsigned int SDIO_INTSET_FN2 : 1; /*!< [0..0] This bit is used to raise an
                                   interrupt to host for function2. Setting this
                                   bit will raise the interrupt Clearing this
                                           bit has no effect */
      __IOM unsigned int SDIO_INTSET_FN3 : 1; /*!< [1..1] This bit is used to raise an
                                   interrupt to host for function3. Setting this
                                   bit will raise the interrupt Clearing this
                                           bit has no effect */
      __IOM unsigned int SDIO_INTSET_FN4 : 1; /*!< [2..2] This bit is used to raise an
                                   interrupt to host for function4. Setting this
                                   bit will raise the interrupt Clearing this
                                           bit has no effect */
      __IOM unsigned int SDIO_INTSET_FN5 : 1; /*!< [3..3] This bit is used to raise an
                                   interrupt to host for function5. Setting this
                                   bit will raise the interrupt Clearing this
                                           bit has no effect */
      __IOM unsigned int RES : 28;            /*!< [31..4] reserved5 */
    } SDIO_HOST_INTR_SET_REG_b;
  };

  union {
    __IOM unsigned int SDIO_HOST_INTR_CLEAR_REG; /*!< (@ 0x00000038) SDIO Host
                                                 Interrupt Clear Register */

    struct {
      __IOM unsigned int SDIO_INTCLR_FN2 : 1; /*!< [0..0] This bit is used to clear the
                                   interrupt to host for function2. Setting this
                                   bit will clear the interrupt Clearing this
                                           bit has no effect */
      __IOM unsigned int SDIO_INTCLR_FN3 : 1; /*!< [1..1] This bit is used to clear the
                                   interrupt to host for function3. Setting this
                                   bit will clear the interrupt Clearing this
                                           bit has no effect */
      __IOM unsigned int SDIO_INTCLR_FN4 : 1; /*!< [2..2] This bit is used to clear the
                                   interrupt to host for function4. Setting this
                                   bit will clear the interrupt Clearing this
                                           bit has no effectt */
      __IOM unsigned int SDIO_INTCLR_FN5 : 1; /*!< [3..3] This bit is used to clear the
                                   interrupt to host for function5. Setting this
                                   bit will clear the interrupt Clearing this
                                           bit has no effect */
      __IOM unsigned int RES : 28;            /*!< [31..4] reserved5 */
    } SDIO_HOST_INTR_CLEAR_REG_b;
  };
  __IM unsigned int RESERVED;

  union {
    __OM unsigned int SDIO_RFIFO_DATA_REG[16]; /*!< (@ 0x00000040) SDIO Read FIFO
                                               Data Register */

    struct {
      __OM unsigned int SDIO_RFIFO : 32; /*!< [31..0] Data to be written into SDIO Read FIFO
                               has to be written in this register. */
    } SDIO_RFIFO_DATA_REG_b[16];
  };

  union {
    __IM unsigned int SDIO_WFIFO_DATA_REG[16]; /*!< (@ 0x00000080) SDIO Write FIFO
                                               Data Register */

    struct {
      __IM unsigned int SDIO_WFIFO : 32; /*!< [31..0] SDIO Write FIFO data can be
                                         read through this register.           */
    } SDIO_WFIFO_DATA_REG_b[16];
  };

  union {
    __IOM unsigned int SDIO_INTR_FN2_STATUS_CLEAR_REG; /*!< (@ 0x000000C0) SDIO Function2
                                            Status Clear Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_CLR : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt. =1
                                                      Interrupt is enabled =0 -
                                              Interrupt is disabled */
      __IOM unsigned int SDIO_RD_INT_CLR : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_INT_CLR : 1;     /*!< [2..2] This bit is used to
                                                 enable CMD53 CSA interrupt   */
      __IOM unsigned int SDIO_CMD52_INT_CLR : 1;   /*!< [3..3] This bit is used to
                                                 enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_CLR : 1; /*!< [4..4] This bit is used to enable power
                                        level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_CLR : 1; /*!< [5..5] This bit is used to enable CRC
                                        error interrupt                     */
      __IOM unsigned int SDIO_ABORT_INT_CLR : 1;   /*!< [6..6] This bit is used to
                                                 enable abort interrupt */
      __IOM unsigned int SDIO_TOUT_INT_CLR : 1;    /*!< [7..7] This bit is used to enable ?read
                                     FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                 /*!< [31..8] reserved5   */
    } SDIO_INTR_FN2_STATUS_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN2_ENABLE_REG; /*!< (@ 0x000000C4) SDIO Function1
                                                 Interrupt Enable Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_EN : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt.  */
      __IOM unsigned int SDIO_RD_INT_EN : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt  */
      __IOM unsigned int SDIO_CSA_INT_EN : 1;     /*!< [2..2] This bit is used to enable
                                              CMD53 CSA interrupt */
      __IOM unsigned int SDIO_CMD52_INT_EN : 1;   /*!< [3..3] This bit is used to
                                                enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_EN : 1; /*!< [4..4] This bit is used to enable power
                                       level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_EN : 1; /*!< [5..5] This bit is used to
                                                  enable CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_INT_EN : 1;   /*!< [6..6] This bit is used to
                                                  enable abort interrupt   */
      __IOM unsigned int SDIO_TOUT_INT_EN : 1;    /*!< [7..7] This bit is used to enable ?read
                                    FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                /*!< [31..8] reserved5  */
    } SDIO_INTR_FN2_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN2_MASK_REG; /*!< (@ 0x000000C8) SDIO Function2
                                               Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_MSK : 1;  /*!< [0..0] This bit is used to mask
                                              CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_MSK : 1;  /*!< [1..1] This bit is used to mask
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_MSK : 1;     /*!< [2..2] This bit is used to mask CMD53 CSA
                                interrupt.Setting this bit will mask the
                                interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_MSK : 1;   /*!< [3..3] This bit is used to mask
                                               CMD52 interrupt   */
      __IOM unsigned int SDIO_PWR_LEV_MSK : 1; /*!< [4..4] This bit is used to mask
                                               power level change interrupt */
      __IOM unsigned int SDIO_CRC_ERR_MSK : 1; /*!< [5..5] This bit is used to mask
                                               CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_MSK : 1;   /*!< [6..6] This bit is used to mask abort
                                  interrupt Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_TOUT_MSK : 1;    /*!< [7..7] This bit is used to mask read FIFO wait
                                 time over interrupt        */
      __IOM unsigned int RES : 24;             /*!< [31..8] reserved5 */
    } SDIO_INTR_FN2_MASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN2_UNMASK_REG; /*!< (@ 0x000000CC) SDIO Function2
                                                 Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_UNMSK : 1;  /*!< [0..0] This bit is used to
                                                unmask CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_UNMSK : 1;  /*!< [1..1] This bit is used to
                                                unmask CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_UNMSK : 1;     /*!< [2..2] This bit is used to unmask CMD53 CSA
                                  interrupt.Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_UNMSK : 1;   /*!< [3..3] This bit is used to
                                               unmask CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_UNMSK : 1; /*!< [4..4] This bit is used to unmask power
                                      level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_UNMSK : 1; /*!< [5..5] This bit is used to
                                                 unmask CRC error interrupt */
      __IOM
      unsigned int SDIO_ABORT_UNMSK : 1;      /*!< [6..6] This bit is used to unmask
                                             abort interrupt Setting this bit
                                             will mask the interrupt Clearing
                                             this bit has no effect */
      __IOM unsigned int SDIO_TOUT_UNMSK : 1; /*!< [7..7] This bit is used to unmask read FIFO
                                   wait time over interrupt      */
      __IOM unsigned int RES : 24;            /*!< [31..8] reserved5 */
    } SDIO_INTR_FN2_UNMASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN3_STATUS_CLEAR_REG; /*!< (@ 0x000000D0) SDIO Function3
                                            Status Clear Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_CLR : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt. =1
                                                      Interrupt is enabled =0 -
                                              Interrupt is disabled */
      __IOM unsigned int SDIO_RD_INT_CLR : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_INT_CLR : 1;     /*!< [2..2] This bit is used to
                                                 enable CMD53 CSA interrupt   */
      __IOM unsigned int SDIO_CMD52_INT_CLR : 1;   /*!< [3..3] This bit is used to
                                                 enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_CLR : 1; /*!< [4..4] This bit is used to enable power
                                        level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_CLR : 1; /*!< [5..5] This bit is used to enable CRC
                                        error interrupt                     */
      __IOM unsigned int SDIO_ABORT_INT_CLR : 1;   /*!< [6..6] This bit is used to
                                                 enable abort interrupt */
      __IOM unsigned int SDIO_TOUT_INT_CLR : 1;    /*!< [7..7] This bit is used to enable ?read
                                     FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                 /*!< [31..8] reserved5   */
    } SDIO_INTR_FN3_STATUS_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN3_ENABLE_REG; /*!< (@ 0x000000D4) SDIO Function3
                                                 Interrupt Enable Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_EN : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt.  */
      __IOM unsigned int SDIO_RD_INT_EN : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt  */
      __IOM unsigned int SDIO_CSA_INT_EN : 1;     /*!< [2..2] This bit is used to enable
                                              CMD53 CSA interrupt */
      __IOM unsigned int SDIO_CMD52_INT_EN : 1;   /*!< [3..3] This bit is used to
                                                enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_EN : 1; /*!< [4..4] This bit is used to enable power
                                       level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_EN : 1; /*!< [5..5] This bit is used to
                                                  enable CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_INT_EN : 1;   /*!< [6..6] This bit is used to
                                                  enable abort interrupt   */
      __IOM unsigned int SDIO_TOUT_INT_EN : 1;    /*!< [7..7] This bit is used to enable ?read
                                    FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                /*!< [31..8] reserved5  */
    } SDIO_INTR_FN3_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN3_MASK_REG; /*!< (@ 0x000000D8) SDIO Function3
                                               Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_MSK : 1;  /*!< [0..0] This bit is used to mask
                                              CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_MSK : 1;  /*!< [1..1] This bit is used to mask
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_MSK : 1;     /*!< [2..2] This bit is used to mask CMD53 CSA
                                interrupt.Setting this bit will mask the
                                interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_MSK : 1;   /*!< [3..3] This bit is used to mask
                                               CMD52 interrupt   */
      __IOM unsigned int SDIO_PWR_LEV_MSK : 1; /*!< [4..4] This bit is used to mask
                                               power level change interrupt */
      __IOM unsigned int SDIO_CRC_ERR_MSK : 1; /*!< [5..5] This bit is used to mask
                                               CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_MSK : 1;   /*!< [6..6] This bit is used to mask abort
                                  interrupt Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_TOUT_MSK : 1;    /*!< [7..7] This bit is used to mask read FIFO wait
                                 time over interrupt        */
      __IOM unsigned int RES : 24;             /*!< [31..8] reserved5 */
    } SDIO_INTR_FN3_MASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN3_UNMASK_REG; /*!< (@ 0x000000DC) SDIO Function3
                                                 Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_UNMSK : 1;  /*!< [0..0] This bit is used to
                                                unmask CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_UNMSK : 1;  /*!< [1..1] This bit is used to
                                                unmask CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_UNMSK : 1;     /*!< [2..2] This bit is used to unmask CMD53 CSA
                                  interrupt.Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_UNMSK : 1;   /*!< [3..3] This bit is used to
                                               unmask CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_UNMSK : 1; /*!< [4..4] This bit is used to unmask power
                                      level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_UNMSK : 1; /*!< [5..5] This bit is used to
                                                 unmask CRC error interrupt */
      __IOM
      unsigned int SDIO_ABORT_UNMSK : 1;      /*!< [6..6] This bit is used to unmask
                                             abort interrupt Setting this bit
                                             will mask the interrupt Clearing
                                             this bit has no effect */
      __IOM unsigned int SDIO_TOUT_UNMSK : 1; /*!< [7..7] This bit is used to unmask read FIFO
                                   wait time over interrupt      */
      __IOM unsigned int RES : 24;            /*!< [31..8] reserved5 */
    } SDIO_INTR_FN3_UNMASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN4_STATUS_CLEAR_REG; /*!< (@ 0x000000E0) SDIO Function4
                                            Status Clear Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_CLR : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt. =1
                                                      Interrupt is enabled =0 -
                                              Interrupt is disabled */
      __IOM unsigned int SDIO_RD_INT_CLR : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_INT_CLR : 1;     /*!< [2..2] This bit is used to
                                                 enable CMD53 CSA interrupt   */
      __IOM unsigned int SDIO_CMD52_INT_CLR : 1;   /*!< [3..3] This bit is used to
                                                 enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_CLR : 1; /*!< [4..4] This bit is used to enable power
                                        level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_CLR : 1; /*!< [5..5] This bit is used to enable CRC
                                        error interrupt                     */
      __IOM unsigned int SDIO_ABORT_INT_CLR : 1;   /*!< [6..6] This bit is used to
                                                 enable abort interrupt */
      __IOM unsigned int SDIO_TOUT_INT_CLR : 1;    /*!< [7..7] This bit is used to enable ?read
                                     FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                 /*!< [31..8] reserved5   */
    } SDIO_INTR_FN4_STATUS_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN4_ENABLE_REG; /*!< (@ 0x000000E4) SDIO Function4
                                                 Interrupt Enable Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_EN : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt.  */
      __IOM unsigned int SDIO_RD_INT_EN : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt  */
      __IOM unsigned int SDIO_CSA_INT_EN : 1;     /*!< [2..2] This bit is used to enable
                                              CMD53 CSA interrupt */
      __IOM unsigned int SDIO_CMD52_INT_EN : 1;   /*!< [3..3] This bit is used to
                                                enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_EN : 1; /*!< [4..4] This bit is used to enable power
                                       level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_EN : 1; /*!< [5..5] This bit is used to
                                                  enable CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_INT_EN : 1;   /*!< [6..6] This bit is used to
                                                  enable abort interrupt   */
      __IOM unsigned int SDIO_TOUT_INT_EN : 1;    /*!< [7..7] This bit is used to enable ?read
                                    FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                /*!< [31..8] reserved5  */
    } SDIO_INTR_FN4_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN4_MASK_REG; /*!< (@ 0x000000E8) SDIO Function4
                                               Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_MSK : 1;  /*!< [0..0] This bit is used to mask
                                              CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_MSK : 1;  /*!< [1..1] This bit is used to mask
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_MSK : 1;     /*!< [2..2] This bit is used to mask CMD53 CSA
                                interrupt.Setting this bit will mask the
                                interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_MSK : 1;   /*!< [3..3] This bit is used to mask
                                               CMD52 interrupt   */
      __IOM unsigned int SDIO_PWR_LEV_MSK : 1; /*!< [4..4] This bit is used to mask
                                               power level change interrupt */
      __IOM unsigned int SDIO_CRC_ERR_MSK : 1; /*!< [5..5] This bit is used to mask
                                               CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_MSK : 1;   /*!< [6..6] This bit is used to mask abort
                                  interrupt Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_TOUT_MSK : 1;    /*!< [7..7] This bit is used to mask read FIFO wait
                                 time over interrupt        */
      __IOM unsigned int RES : 24;             /*!< [31..8] reserved5 */
    } SDIO_INTR_FN4_MASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN4_UNMASK_REG; /*!< (@ 0x000000EC) SDIO Function4
                                                 Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_UNMSK : 1;  /*!< [0..0] This bit is used to
                                                unmask CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_UNMSK : 1;  /*!< [1..1] This bit is used to
                                                unmask CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_UNMSK : 1;     /*!< [2..2] This bit is used to unmask CMD53 CSA
                                  interrupt.Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_UNMSK : 1;   /*!< [3..3] This bit is used to
                                               unmask CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_UNMSK : 1; /*!< [4..4] This bit is used to unmask power
                                      level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_UNMSK : 1; /*!< [5..5] This bit is used to
                                                 unmask CRC error interrupt */
      __IOM
      unsigned int SDIO_ABORT_UNMSK : 1;      /*!< [6..6] This bit is used to unmask
                                             abort interrupt Setting this bit
                                             will mask the interrupt Clearing
                                             this bit has no effect */
      __IOM unsigned int SDIO_TOUT_UNMSK : 1; /*!< [7..7] This bit is used to unmask read FIFO
                                   wait time over interrupt      */
      __IOM unsigned int RES : 24;            /*!< [31..8] reserved5 */
    } SDIO_INTR_FN4_UNMASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN5_STATUS_CLEAR_REG; /*!< (@ 0x000000F0) SDIO Function5
                                            Status Clear Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_CLR : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt. =1
                                                      Interrupt is enabled =0 -
                                              Interrupt is disabled */
      __IOM unsigned int SDIO_RD_INT_CLR : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_INT_CLR : 1;     /*!< [2..2] This bit is used to
                                                 enable CMD53 CSA interrupt   */
      __IOM unsigned int SDIO_CMD52_INT_CLR : 1;   /*!< [3..3] This bit is used to
                                                 enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_CLR : 1; /*!< [4..4] This bit is used to enable power
                                        level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_CLR : 1; /*!< [5..5] This bit is used to enable CRC
                                        error interrupt                     */
      __IOM unsigned int SDIO_ABORT_INT_CLR : 1;   /*!< [6..6] This bit is used to
                                                 enable abort interrupt */
      __IOM unsigned int SDIO_TOUT_INT_CLR : 1;    /*!< [7..7] This bit is used to enable ?read
                                     FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                 /*!< [31..8] reserved5   */
    } SDIO_INTR_FN5_STATUS_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN5_ENABLE_REG; /*!< (@ 0x000000F4) SDIO Function5
                                                 Interrupt Enable Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_EN : 1;      /*!< [0..0] This bit is used to enable
                                              CMD53 write interrupt.  */
      __IOM unsigned int SDIO_RD_INT_EN : 1;      /*!< [1..1] This bit is used to enable
                                              CMD53 read interrupt  */
      __IOM unsigned int SDIO_CSA_INT_EN : 1;     /*!< [2..2] This bit is used to enable
                                              CMD53 CSA interrupt */
      __IOM unsigned int SDIO_CMD52_INT_EN : 1;   /*!< [3..3] This bit is used to
                                                enable CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_INT_EN : 1; /*!< [4..4] This bit is used to enable power
                                       level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_INT_EN : 1; /*!< [5..5] This bit is used to
                                                  enable CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_INT_EN : 1;   /*!< [6..6] This bit is used to
                                                  enable abort interrupt   */
      __IOM unsigned int SDIO_TOUT_INT_EN : 1;    /*!< [7..7] This bit is used to enable ?read
                                    FIFO wait time over? interrupt */
      __IOM unsigned int RES : 24;                /*!< [31..8] reserved5  */
    } SDIO_INTR_FN5_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN5_MASK_REG; /*!< (@ 0x000000F8) SDIO Function5
                                               Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_MSK : 1;  /*!< [0..0] This bit is used to mask
                                              CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_MSK : 1;  /*!< [1..1] This bit is used to mask
                                              CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_MSK : 1;     /*!< [2..2] This bit is used to mask CMD53 CSA
                                interrupt.Setting this bit will mask the
                                interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_MSK : 1;   /*!< [3..3] This bit is used to mask
                                               CMD52 interrupt   */
      __IOM unsigned int SDIO_PWR_LEV_MSK : 1; /*!< [4..4] This bit is used to mask
                                               power level change interrupt */
      __IOM unsigned int SDIO_CRC_ERR_MSK : 1; /*!< [5..5] This bit is used to mask
                                               CRC error interrupt */
      __IOM unsigned int SDIO_ABORT_MSK : 1;   /*!< [6..6] This bit is used to mask abort
                                  interrupt Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_TOUT_MSK : 1;    /*!< [7..7] This bit is used to mask read FIFO wait
                                 time over interrupt        */
      __IOM unsigned int RES : 24;             /*!< [31..8] reserved5 */
    } SDIO_INTR_FN5_MASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_INTR_FN5_UNMASK_REG; /*!< (@ 0x000000FC) SDIO Function5
                                                 Interrupt Mask Register */

    struct {
      __IOM unsigned int SDIO_WR_INT_UNMSK : 1;  /*!< [0..0] This bit is used to
                                                unmask CMD53 write interrupt */
      __IOM unsigned int SDIO_RD_INT_UNMSK : 1;  /*!< [1..1] This bit is used to
                                                unmask CMD53 read interrupt */
      __IOM unsigned int SDIO_CSA_UNMSK : 1;     /*!< [2..2] This bit is used to unmask CMD53 CSA
                                  interrupt.Setting this bit will mask the
                                  interrupt Clearing this bit has no effect */
      __IOM unsigned int SDIO_CMD52_UNMSK : 1;   /*!< [3..3] This bit is used to
                                               unmask CMD52 interrupt */
      __IOM unsigned int SDIO_PWR_LEV_UNMSK : 1; /*!< [4..4] This bit is used to unmask power
                                      level change interrupt            */
      __IOM unsigned int SDIO_CRC_ERR_UNMSK : 1; /*!< [5..5] This bit is used to
                                                 unmask CRC error interrupt */
      __IOM
      unsigned int SDIO_ABORT_UNMSK : 1;      /*!< [6..6] This bit is used to unmask
                                             abort interrupt Setting this bit
                                             will mask the interrupt Clearing
                                             this bit has no effect */
      __IOM unsigned int SDIO_TOUT_UNMSK : 1; /*!< [7..7] This bit is used to unmask read FIFO
                                   wait time over interrupt      */
      __IOM unsigned int RES : 24;            /*!< [31..8] reserved5 */
    } SDIO_INTR_FN5_UNMASK_REG_b;
  };

  union {
    __IOM unsigned int SDIO_ERROR_COND_CHK_ENABLE_REG; /*!< (@ 0x00000100) SDIO error condition
                                            check enable register */

    struct {
      __IOM unsigned int SDIO_CRC_EN : 1;               /*!< [0..0] When set, stops the DMA from doing data
                             accesses till CRC error interrupt is cleared */
      __IOM unsigned int SDIO_ABORT_EN : 1;             /*!< [1..1] When set, stops the DMA from doing data
                               accesses till ABORT interrupt is cleared */
      __IOM unsigned int SDIO_SPI_RD_DATA_ERROR_EN : 1; /*!< [2..2] When set, stops the DMA
                                           from doing data accesses till read
                                           data error interrupt is cleared in SPI
                                           mode                          */
      __IOM unsigned int RES : 29;                      /*!< [31..3] reserved5           */
    } SDIO_ERROR_COND_CHK_ENABLE_REG_b;
  };

  union {
    __IOM unsigned int SDIO_ERROR_COND_STATE_REG; /*!< (@ 0x00000104) SDIO error
                                                  condition state register */

    struct {
      __IOM unsigned int SDIO_ERROR_BYTE_CNT : 12; /*!< [11..0] Indicates byte count when one
                                        of the error condition occurred */
      __IOM unsigned int RESERVED1 : 4;            /*!< [15..12] RESERVED1 */
      __IOM unsigned int SDIO_ERROR_BLK_CNT : 7;   /*!< [22..16] Indicates block count when one
                                        of error condition occurred       */
      __IOM unsigned int RESERVED2 : 9;            /*!< [31..23] RESERVED2 */
    } SDIO_ERROR_COND_STATE_REG_b;
  };

  union {
    __IM unsigned int SDIO_BOOT_CONFIG_VALS_0_REG; /*!< (@ 0x00000108) SDIO Boot
                                                   Config Values Register 0 */

    struct {
      __IM unsigned int OCR_R : 24;     /*!< [23..0] Operating conditions. The value
                                        written by bootloader     can be read here.     */
      __IM unsigned int CSA_MSBYTE : 8; /*!< [31..24] MS byre of CSA address.  Lower
                                        24 bits of CSA will come through SDIO CSA
                                        registers.  Whenever CSA access is
                                                      done, 32-bit address will
                                        be prepared using these fields. */
    } SDIO_BOOT_CONFIG_VALS_0_REG_b;
  };

  union {
    __IM unsigned int SDIO_BOOT_CONFIG_VALS_1_REG; /*!< (@ 0x0000010C) SDIO Boot
                                                   Config Values Register 1 */

    struct {
      __IM unsigned int NO_OF_IO_FUNCTIONS : 3;              /*!< [2..0] Indicates number functions
                                      supported. The value written
                                            by bootloader can be read here. */
      __IM unsigned int COMBOCARD : 1;                       /*!< [3..3] When set, combo mode will be enabled. */
      __IM unsigned int SDMEM_IGNOTRE_SDMEM_PRESENT : 1;     /*!< [4..4] When set, sdmem_present
                                            signal, coming from GPIO, will be
                                            ignored. */
      __IM unsigned int SDMEM_DRIVE_HIZ_MB_READ : 1;         /*!< [5..5] When set, High will be driven
                                        in the second cycle of interrupt period
                                        during sd memory mb read transfer */
      __IM unsigned int SDMEM_DISABLE_INTERRUPT_MB_READ : 1; /*!< [6..6] When set,
                                                             interrupt will be
                                                             not be driven during
                                                             sd memory mb read
                                                             transfer */
      __IM unsigned int IGNORE_DISABLE_HS : 1;               /*!< [7..7] if ignore_disable_hs is set,
                                        sdmem_disable_high_speed_switching    coming
                                        from combo mode module is ignored    */
      __IM unsigned int RESERVED2 : 24;                      /*!< [31..8] RESERVED2 */
    } SDIO_BOOT_CONFIG_VALS_1_REG_b;
  };
} SDIO0_Type; /*!< Size = 272 (0x110) */

/* ===========================================================================================================================
  */
/* ================                                         SPI_SLAVE
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief The SPI Interface is a full duplex serial host interface, which
   supports 8-bit and 32-bit data granularity. It also supports gated mode of SPI
   clock and both the low and the high frequency modes (SPI_SLAVE)
   */

typedef struct { /*!< (@ 0x20200000) SPI_SLAVE Structure */

  union {
    __IOM uint8_t SPI_HOST_INTR; /*!< (@ 0x00000000) SPI Host interupt resgister. */

    struct {
      __IOM uint8_t SPI_HOST_INTR : 8; /*!< [7..0] These bits indicate the interrupt
                                 vector value coming from system side. */
    } SPI_HOST_INTR_b;
  };
  __IM uint8_t RESERVED;

  union {
    __IOM uint8_t SPI_RFIFO_START; /*!< (@ 0x00000002) SPI FIFO start Level Register. */

    struct {
      __IOM uint8_t SPI_RFIFO_ST : 8; /*!< [7..0] These bits indicate the interrupt vector
                                value coming from system side. */
    } SPI_RFIFO_START_b;
  };
  __IM uint8_t RESERVED1;

  union {
    __IOM uint8_t SPI_RFIFO_AFULL_LEV; /*!< (@ 0x00000004) SPI RFIFO AFULL Level
                                           Register. */

    struct {
      __IOM uint8_t SPI_RFIFO_AFULL_LEV : 8; /*!< [7..0] These bits are used to program
                                       the FIFO occupancy level to trigger the
                                       Almost Full indication. */
    } SPI_RFIFO_AFULL_LEV_b;
  };
  __IM uint8_t RESERVED2;

  union {
    __IOM uint8_t SPI_RFIFO_AEMPTY_LEV; /*!< (@ 0x00000006) SPI WFIFO Almost
                                            Empty Register. */

    struct {
      __IOM uint8_t SPI_RFIFO_AEMPTY_LEV : 8; /*!< [7..0] These bits are used to
                                                  program the occupancy level to
                                                      trigger the Almost Empty
                                                  indication. */
    } SPI_RFIFO_AEMPTY_LEV_b;
  };
  __IM uint8_t RESERVED3;

  union {
    __IOM uint8_t SPI_MODE; /*!< (@ 0x00000008) SPI Mode Register. */

    struct {
      __IOM uint8_t SPI_OP_MODE : 1; /*!< [0..0] This bit is used to program the
                                         mode of working of SPI Interface. */
      __IOM uint8_t SPI_FIX_EN : 1;  /*!< [1..1] This bit is used to enable the
                                         fix made for bus_ctrl_busy  being asserted
                                         when success_state is being asserted
                                         getting  deasserted when FSM has decided
                                         to move to BUSY_STATE or  not.  */
      __IOM uint8_t VHS_EN : 1;      /*!< [2..2] This bit is used to enable Very high
                                    speed mode (120Mhz).          */
      __IOM uint8_t BYPASS_INIT : 1; /*!< [3..3] This bit is used to bypass the
                                         SPI initialization.0 -
                                                      Doesn't bypass,1 - bypasses
                                         SPI initialization */
      __IOM uint8_t RESERVED1 : 4;   /*!< [7..4] reserved1   */
    } SPI_MODE_b;
  };
  __IM uint8_t RESERVED4;

  union {
    __IOM uint16_t SPI_INTR_STATUS; /*!< (@ 0x0000000A) SPI interrupt status register. */

    struct {
      __IOM uint16_t SPI_WR_REQ : 1;      /*!< [0..0] Write request received. */
      __IOM uint16_t SPI_RD_REQ : 1;      /*!< [1..1] Read request received. */
      __IOM uint16_t SPI_CS_DEASSERT : 1; /*!< [2..2] SPI chip deassert interrupt.       */
      __IOM uint16_t RESERVED1 : 13;      /*!< [15..3] reserved1 */
    } SPI_INTR_STATUS_b;
  };

  union {
    __IOM uint16_t SPI_INTR_EN; /*!< (@ 0x0000000C) SPI interrupt enable register. */

    struct {
      __IOM uint16_t SPI_WR_INT_EN : 1;          /*!< [0..0] This bit is used to enable
                                            the write interrupt. */
      __IOM uint16_t SPI_RD_INT_EN : 1;          /*!< [1..1] This bit is used to enable
                                            the read interrupt. */
      __IOM uint16_t SPI_CS_DEASSERT_INT_EN : 1; /*!< [2..2] This bit is used to enable the
                                          interrupt due to wrong deassertion of
                                          CS. */
      __IOM uint16_t RESERVED1 : 13;             /*!< [15..3] reserved1  */
    } SPI_INTR_EN_b;
  };

  union {
    __IOM uint16_t SPI_INTR_MASK; /*!< (@ 0x0000000E) SPI interrupt Mask register */

    struct {
      __IOM uint16_t SPI_WR_INTR_MSK : 1;         /*!< [0..0] This bit is used to mask
                                              the write interrupt. */
      __IOM uint16_t SPI_RD_INTR_MSK : 1;         /*!< [1..1] This bit is used to mask
                                              the read interrupt. */
      __IOM uint16_t SPI_CS_DEASSERT_INT_MSK : 1; /*!< [2..2] This bit is used to mask the
                                           CS deassertion interrupt. */
      __IOM uint16_t RESERVED1 : 13;              /*!< [15..3] reserved1   */
    } SPI_INTR_MASK_b;
  };

  union {
    __IOM uint16_t SPI_INTR_UNMASK; /*!< (@ 0x00000010) SPI interrupt unmask register */

    struct {
      __IOM uint16_t SPI_WR_INT_UNMASK : 1;         /*!< [0..0] This bit is used to
                                                unmask the write interrupt. */
      __IOM uint16_t SPI_RD_INTR_UNMSK : 1;         /*!< [1..1] This bit is used to
                                                unmask the read interrupt. */
      __IOM uint16_t SPI_CS_DEASSERT_INT_UNMSK : 1; /*!< [2..2] This bit is used to unmask
                                             the CS deassertion interrupt. */
      __IOM uint16_t RESERVED1 : 13;                /*!< [15..3] reserved1     */
    } SPI_INTR_UNMASK_b;
  };

  union {
    __IM uint16_t SPI_LENGTH; /*!< (@ 0x00000012) SPI Length Register */

    struct {
      __IM uint16_t SPI_LEN : 16; /*!< [15..0] These bit indicate the length of
                                      the transfer as transmitted
                                                      in the Commands C3 and C4.
                                    */
    } SPI_LENGTH_b;
  };

  union {
    __IM uint16_t SPI_COMMAND; /*!< (@ 0x00000014) SPI Command Register */

    struct {
      __IM uint16_t SPI_C1 : 8; /*!< [7..0] These bits store the received command C1. */
      __IM uint16_t SPI_C2 : 8; /*!< [15..8] These bits store the received
                                    command C2.                         */
    } SPI_COMMAND_b;
  };

  union {
    __IM uint16_t SPI_DEV_ID; /*!< (@ 0x00000016) SPI Device ID Register */

    struct {
      __IM uint16_t SPI_DEVID : 16; /*!< [15..0] These bits store the Device ID
                                        information.                       */
    } SPI_DEV_ID_b;
  };

  union {
    __IM uint16_t SPI_VERSION; /*!< (@ 0x00000018) SPI Device ID Register */

    struct {
      __IM uint16_t SPI_VERNO : 8; /*!< [7..0] These bits store the version number. */
      __IM uint16_t RESERVED1 : 8; /*!< [15..8] reserved1 */
    } SPI_VERSION_b;
  };

  union {
    __IM uint16_t SPI_STATUS; /*!< (@ 0x0000001A) SPI Status Register */

    struct {
      __IM uint16_t SPI_RFIFO_FULL : 1;   /*!< [0..0] This bit indicates if the
                                             read FIFO is almost full.  */
      __IM uint16_t SPI_RFIFO_AFULL : 1;  /*!< [1..1] This bit indicates if the
                                             read FIFO is almost full. */
      __IM uint16_t SPI_WFIFO_EMPTY : 1;  /*!< [2..2] This bit indicates if write
                                             FIFO is empty. */
      __IM uint16_t SPI_WFIFO_AEMPTY : 1; /*!< [3..3] This bit indicates if
                                              write FIFO is almost empty. */
      __IM uint16_t SPI_RFIFO_EMPTY : 1;  /*!< [4..4] This bit indicates if read FIFO is
                                   empty (Read from SOC to host). */
      __IM uint16_t SPI_RFIFO_AEMPTY : 1; /*!< [5..5] This bit indicates if read FIFO is
                                    empty (Read from SOC to host). */
      __IM uint16_t SPI_WFIFO_FULL : 1;   /*!< [6..6] This bit indicates if write FIFO is
                                  full (Write from Host to SOC). */
      __IM uint16_t SPI_WFIFO_AFULL : 1;  /*!< [7..7] This bit indicates if write FIFO is
                                   full (Write from Host to SOC). */
      __IM uint16_t RESERVED1 : 8;        /*!< [15..8] reserved1 */
    } SPI_STATUS_b;
  };

  union {
    __IM uint16_t SPI_BC_STATE; /*!< (@ 0x0000001C) SPI Bus Controller State Register */

    struct {
      __IM uint16_t SPI_BC : 14;   /*!< [13..0] These bits indicate the Bus
                                       Controller FSM state.                 */
      __IM uint16_t RESERVED1 : 2; /*!< [15..14] reserved1 */
    } SPI_BC_STATE_b;
  };
  __IM uint16_t RESERVED5;
  __IM unsigned int RESERVED6[23];

  union {
    __IOM uint16_t SPI_SYS_RESET_REQ; /*!< (@ 0x0000007C) SPI SYS Reset Req Register */

    struct {
      __IOM
      uint16_t SPI_SYS_RESET_REQ : 1; /*!< [0..0] When set generates system reset
                                         request to reset controller. This gets
                                         reset once, reset controller generates
                                         reset. Host should not reset this bit.
                                         With this reset request, reset
                                         controller generates non por reset. */
      __IOM uint16_t RESERVED1 : 15;  /*!< [15..1] reserved1 */
    } SPI_SYS_RESET_REQ_b;
  };

  union {
    __IOM uint16_t SPI_WAKE_UP; /*!< (@ 0x0000007E) SPI Wakeup Register */

    struct {
      __IOM uint16_t SPI_WAKEUP : 1;        /*!< [0..0] Wakeup Interrupt,Interrupt for waking up
                              the system from Deep Sleep. */
      __IOM uint16_t SPI_DEEP_SLEEP_ST : 1; /*!< [1..1] Deep Sleep Start,Indicates the
                                         device to enter Deep Sleep
                                               state for maximum power save.     */
      __IOM uint16_t RESERVED1 : 14;        /*!< [15..2] reserved1 */
    } SPI_WAKE_UP_b;
  };
  __IM unsigned int RESERVED7[192];

  union {
    __IM unsigned int SPI_RFIFO_DATA; /*!< (@ 0x00000380) SPI RFIFO Data Register */

    struct {
      __IM unsigned int SPI_RFIFO : 32; /*!< [31..0] These bits store the data
                                        received from the host */
    } SPI_RFIFO_DATA_b;
  };
  __IM unsigned int RESERVED8[15];

  union {
    __OM unsigned int SPI_WFIFO_DATA; /*!< (@ 0x000003C0) SPI WFIFO Data Register */

    struct {
      __OM unsigned int SPI_WFIFO : 32; /*!< [31..0] These bits are used to write,
                                        the data to be sent to the host. */
    } SPI_WFIFO_DATA_b;
  };
} SPI_SLAVE_Type; /*!< Size = 964 (0x3c4) */

/* ===========================================================================================================================
  */
/* ================                                           M4CLK
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief MCU HP (High Performance) domain contains the Cortex-M4F Processor,
  * FPU, Debugger, MCU High Speed Interfaces, MCU HP Peripherals, MCU HP DMA and
  * MCU/SZP shareable Interfaces (M4CLK)
  */

typedef struct { /*!< (@ 0x46000000) M4CLK Structure */

  union {
    __IOM unsigned int CLK_ENABLE_SET_REG1; /*!< (@ 0x00000000) Clock Enable Set
                                            Register 1 */

    struct {
      __IOM unsigned int USART1_PCLK_ENABLE_b : 1;          /*!< [0..0] Static Clock gating Enable for
                                        usart1 pclk1'b1 => Clock
                                           is enabled 1'b0 => Invalid */
      __IOM unsigned int USART1_SCLK_ENABLE_b : 1;          /*!< [1..1] Static Clock gating Enable for
                                        usart1 sclk1'b1 => Clock
                                           is enabled 1'b0 => Invalid */
      __IOM unsigned int USART2_PCLK_ENABLE_b : 1;          /*!< [2..2] Static Clock gating Enable for
                                        usart2 pclk1'b1 => Clock
                                           is enabled 1'b0 => Invalid */
      __IOM unsigned int USART2_SCLK_ENABLE_b : 1;          /*!< [3..3] Static Clock gating Enable for
                                        usart2 sclk1'b1 => Clock
                                           is enabled 1'b0 => Invalid */
      __IOM unsigned int Reserved1 : 5;                     /*!< [8..4] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int CT_CLK_ENABLE_b : 1;               /*!< [9..9] Static Clock gating Enable
                                             for sct clk1'b1 => Clock is
                                             enabled 1'b0 => Invalid. */
      __IOM unsigned int CT_PCLK_ENABLE_b : 1;              /*!< [10..10] Static Clock gating
                                              Enable for sct pclk1'b1 => Clock
                                              is enabled 1'b0 => Invalid. */
      __IOM unsigned int ICACHE_CLK_ENABLE_b : 1;           /*!< [11..11] Static Clock gating Enable for
                                      icache clk1'b1 => Clock
                                      is enabled 1'b0 => Invalid. */
      __IOM unsigned int ICACHE_CLK_2X_ENABLE_b : 1;        /*!< [12..12] Static Clock gating Enable
                                         for icache 2x clk1'b1 => Clock is
                                         enabled 1'b0 => Invalid. */
      __IOM unsigned int RPDMA_HCLK_ENABLE_b : 1;           /*!< [13..13] Static Clock gating Enable for
                                      rpdma hclk1'b1 => Clock
                                      is enabled 1'b0 => Invalid. */
      __IOM unsigned int SOC_PLL_SPI_CLK_ENABLE_b : 1;      /*!< [14..14] Static Clock gating Enable
                                           for soc pll spi clk1'b1
                                           => Clock is enabled 1'b0 => Invalid.
                                         */
      __IOM unsigned int Reserved2 : 1;                     /*!< [15..15] It is recommended to write
                                            these bits to 0.                      */
      __IOM unsigned int IID_CLK_ENABLE_b : 1;              /*!< [16..16] Static Clock gating
                                               Enable for iid clk1'b1 => Clock
                                                is enabled 1'b0 => Invalid. */
      __IOM unsigned int SDIO_SYS_HCLK_ENABLE_b : 1;        /*!< [17..17] Static Clock gating Enable
                                          for sdio sys hclk1'b1 => Clock is
                                          enabled 1'b0 => Invalid */
      __IOM unsigned int CRC_CLK_ENABLE_b : 1;              /*!< [18..18] Static Clock gating
                                               Enable for crc clk1'b1 => Clock
                                                is enabled 1'b0 => Invalid */
      __IOM unsigned int Reserved3 : 3;                     /*!< [21..19] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int HWRNG_PCLK_ENABLE_b : 1;           /*!< [22..22] Static Clock gating Enable for
                                       HWRNG pclk1'b1 => Clock
                                         is enabled 1'b0 => Invalid. */
      __IOM unsigned int GNSS_MEM_CLK_ENABLE_b : 1;         /*!< [23..23] Static Clock gating Enable
                                         for GNSS mem clk1'b1 =>
                                           Clock is enabled 1'b0 => Invalid */
      __IOM unsigned int Reserved4 : 3;                     /*!< [26..24] It is recommended to write
                                         these bits to 0.                      */
      __IOM unsigned int MASK_HOST_CLK_WAIT_FIX_b : 1;      /*!< [27..27] This bit decides whether
                                           to wait for a fixed number of xtal
                                           clock cycles(based on
                                           mask31_host_clk_cnt) or wait for a
                                           internally generated signal to come
                                           out of WAIT state in host mux FSM 1'b1
                                           => Wait for fixed number of xtal clk
                                           cycles 1'b0 => Invalid This bit along
                                           with mask_host_clk_available_fix and
                                           mask31_host_clk_cnt are to take care
                                           in case of any bugs. */
      __IOM unsigned int MASK31_HOST_CLK_CNT_b : 1;         /*!< [28..28] When mask_host_clk_wait_fix
                                        is 1'b1, this bit decides whether to
                                        count for 32 0r 16 xtal clock cycles to
                                        come out of WAIT state in host mux FSM
                                        1'b1 => Wait for 32 clock cycles 1'b0 =>
                                        Invalid This bit along with
                                        mask_host_clk_available_fix and
                                        mask_host_clk_wait_fix are to take care
                                        in case of any bugs. */
      __IOM unsigned int Reserved5 : 1;                     /*!< [29..29] It is recommended to write
                                         these bits to 0.                      */
      __IOM unsigned int MASK_HOST_CLK_AVAILABLE_FIX_b : 1; /*!< [30..30] This bit decides
                                                 whether to consider negedge of
                                                 host_clk_available in the
                                                 generation of clock enable for
                                                 host_clk gate in host mux 1'b1
                                                 => Don't consider 1'b0 =>
                                                 Invalid This bit along with
                                                 mask_host_clk_wait_fix and
                                                 mask31_host_clk_cnt
                                                        are to take care in case
                                                 of any bugs. */
      __IOM unsigned int ULPSS_CLK_ENABLE_b : 1;            /*!< [31..31] Static Clock gating Enable for
                                      m4 soc_clk to ulpss1'b1
                                           => Clock is enabled 1'b0 => Invalid.
                                    */
    } CLK_ENABLE_SET_REG1_b;
  };

  union {
    __IOM unsigned int CLK_ENABLE_CLEAR_REG1; /*!< (@ 0x00000004) Clock Enable Clear
                                              Register 1 */

    struct {
      __IOM unsigned int USART1_PCLK_ENABLE_b : 1;          /*!< [0..0] Static Clock Clear
                                                   for usart1 pclk1'b1 => Clock
                                                   is Clear 1'b0 => Invalid */
      __IOM unsigned int USART1_SCLK_ENABLE_b : 1;          /*!< [1..1] Static Clock Clear
                                                   for usart1 sclk1'b1 => Clock
                                                   is Clear 1'b0 => Invalid */
      __IOM unsigned int USART2_PCLK_ENABLE_b : 1;          /*!< [2..2] Static Clock Clear
                                                   for usart2 pclk 1'b1 => Clock
                                                   is Clear 1'b0 => Invalid */
      __IOM unsigned int USART2_SCLK_ENABLE_b : 1;          /*!< [3..3] Static Clock Clear
                                                   for usart2 sclk1'b1 => Clock
                                                   is Clear 1'b0 => Invalid */
      __IOM unsigned int Reserved1 : 5;                     /*!< [8..4] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int CT_CLK_ENABLE_b : 1;               /*!< [9..9] Static Clock Clear for sct clk1'b1 =>
                                   Clock is Clear 1'b0 => Invalid. */
      __IOM unsigned int CT_PCLK_ENABLE_b : 1;              /*!< [10..10] Static Clock Clear for
                                               sct pclk1'b1 => Clock is Clear
                                                1'b0 => Invalid. */
      __IOM unsigned int ICACHE_CLK_ENABLE_b : 1;           /*!< [11..11] Static Clock Clear
                                                  for icache clk1'b1 => Clock is
                                                  Clear 1'b0 => Invalid. */
      __IOM unsigned int ICACHE_CLK_2X_ENABLE_b : 1;        /*!< [12..12] Static Clock Clear for
                                          icache 2x clk1'b1 => Clock is Clear
                                          1'b0 => Invalid. */
      __IOM unsigned int RPDMA_HCLK_ENABLE_b : 1;           /*!< [13..13] Static Clock Clear
                                                  for rpdma hclk1'b1 => Clock is
                                                  Clear 1'b0 => Invalid. */
      __IOM unsigned int SOC_PLL_SPI_CLK_ENABLE_b : 1;      /*!< [14..14] Static Clock Clear for soc
                                           pll spi clk1'b1 => Clock
                                           is Clear 1'b0 => Invalid. */
      __IOM unsigned int Reserved2 : 1;                     /*!< [15..15] It is recommended to write
                                            these bits to 0.                      */
      __IOM unsigned int IID_CLK_ENABLE_b : 1;              /*!< [16..16] Static Clock Clear for iid clk1'b1
                                    => Clock is Clear 1'b0 => Invalid. */
      __IOM unsigned int SDIO_SYS_HCLK_ENABLE_b : 1;        /*!< [17..17] Static Clock Clear for sdio
                                          sys hclk1'b1 => Clock is
                                           Clear 1'b0 => Invalid */
      __IOM unsigned int CRC_CLK_ENABLE_b : 1;              /*!< [18..18] Static Clock Clear for crc clk1'b1
                                    => Clock is Clear 1'b0 => Invalid */
      __IOM unsigned int Reserved3 : 3;                     /*!< [21..19] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int HWRNG_PCLK_ENABLE_b : 1;           /*!< [22..22] Static Clock Clear
                                                    for HWRNG pclk1'b1 => Clock is
                                                    Clear   1'b0 => Invalid.   */
      __IOM unsigned int GNSS_MEM_CLK_ENABLE_b : 1;         /*!< [23..23] Static Clock Clear
                                                    for GNSS mem clk1'b1 => Clock
                                                    is Clear 1'b0 => Invalid */
      __IOM unsigned int Reserved4 : 3;                     /*!< [26..24] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int MASK_HOST_CLK_WAIT_FIX_b : 1;      /*!< [27..27] This bit decides whether
                                           to wait for a fixed number of xtal
                                           clock cycles(based on
                                           mask31_host_clk_cnt) or wait for a
                                           internally generated signal to come
                                           out of WAIT state in host mux FSM 1'b1
                                           => Wait for fixed number of xtal clk
                                           cycles 1'b0 => Invalid This bit along
                                           with mask_host_clk_available_fix and
                                           mask31_host_clk_cnt are to take care
                                           in case of any bugs. */
      __IOM unsigned int MASK31_HOST_CLK_CNT_b : 1;         /*!< [28..28] When mask_host_clk_wait_fix
                                        is 1'b1, this bit decides whether to
                                        count for 32 0r 16 xtal clock cycles to
                                        come out of WAIT state in host mux FSM
                                        1'b1 => Wait for 32 clock cycles 1'b0 =>
                                        Invalid This bit along with
                                        mask_host_clk_available_fix and
                                        mask_host_clk_wait_fix are to take care
                                        in case of any bugs. */
      __IOM unsigned int Reserved5 : 1;                     /*!< [29..29] It is recommended to write
                                         these bits to 0.                      */
      __IOM unsigned int MASK_HOST_CLK_AVAILABLE_FIX_b : 1; /*!< [30..30] This bit decides
                                                 whether to consider negedge of
                                                 host_clk_available in the
                                                 generation of clock enable for
                                                 host_clk gate in host mux 1'b1
                                                 => Don't consider 1'b0 =>
                                                 Invalid This bit along with
                                                 mask_host_clk_wait_fix and
                                                 mask31_host_clk_cnt
                                                        are to take care in case
                                                 of any bugs. */
      __IOM unsigned int ULPSS_CLK_ENABLE_b : 1;            /*!< [31..31] Static Clock gating Enable for
                                      m4 soc_clk to ulpss1'b1
                                           => Clock is enabled 1'b0 => Invalid.
                                    */
    } CLK_ENABLE_CLEAR_REG1_b;
  };

  union {
    __IOM unsigned int CLK_ENABLE_SET_REG2; /*!< (@ 0x00000008) Clock Enable Set
                                            Register 2 */

    struct {
      __IOM unsigned int GEN_SPI_MST1_HCLK_ENABLE_b : 1; /*!< [0..0] Static Clock gating Enable
                                              for gen spi master1 hclk 1'b1
                                                        => Clock is enabled 1'b0
                                              => Invalid */
      __IOM unsigned int Reserved1 : 5;                  /*!< [5..1] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int UDMA_HCLK_ENABLE_b : 1;         /*!< [6..6] Static Clock gating Enable for
                                      udma hclk 1'b1 => Clock
                                      is enabled 1'b0 => Invalid. */
      __IOM unsigned int I2C_BUS_CLK_ENABLE_b : 1;       /*!< [7..7] Static Clock gating Enable for
                                        i2c-1 bus clk1'b1 => Clock is enabled
                                        1'b0 => Invalid. */
      __IOM unsigned int I2C_2_BUS_CLK_ENABLE_b : 1;     /*!< [8..8] Static Clock gating Enable for
                                          i2c-2 bus clk 1'b1 =>
                                          Clock is enabled 1'b0 => Invalid. */
      __IOM unsigned int SSI_SLV_PCLK_ENABLE_b : 1;      /*!< [9..9] Static Clock gating Enable for
                                         ssi slave pclk 1'b1 =>
                                         Clock is enabled 1'b0 => Invalid. */
      __IOM
      unsigned int SSI_SLV_SCLK_ENABLE_b : 1;         /*!< [10..10] Static Clock gating
                                                  Enable for ssi slave sclk 1'b1
                                                  => Clock is enabled 1'b0 =>
                                                  Invalid. */
      __IOM unsigned int QSPI_CLK_ENABLE_b : 1;       /*!< [11..11] Static Clock gating
                                                  Enable for qspi clk 1'b1 => Clock
                                                  is enabled 1'b0 => Invalid.   */
      __IOM unsigned int QSPI_HCLK_ENABLE_b : 1;      /*!< [12..12] Static Clock gating Enable for
                                      qspi hclk 1'b1 => Clock
                                      is enabled 1'b0 => Invalid. */
      __IOM unsigned int I2SM_SCLK_ENABLE_b : 1;      /*!< [13..13] Static Clock gating Enable for
                                      sclk of I2S at Root Clock generation 1'b1
                                      => Clock is enabled 1'b0 => Invalid. */
      __IOM unsigned int I2SM_INTF_SCLK_ENABLE_b : 1; /*!< [14..14] Static Clock gating Enable
                                           for i2s interface sclk 1'b1
                                           => Clock is enabled 1'b0 => Invalid.
                                         */
      __IOM unsigned int I2SM_PCLK_ENABLE_b : 1;      /*!< [15..15] Static Clock gating Enable for
                                        i2s master pclk 1'b1
                                        => Clock is enabled 1'b0 => Invalid.   */
      __IOM unsigned int Reserved2 : 1;               /*!< [16..16] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int QE_PCLK_ENABLE_b : 1;        /*!< [17..17] Static Clock gating Enable for qe
                                    pclk 1'b1 => Clock is enabled 1'b0 =>
                                    Invalid. */
      __IOM unsigned int MCPWM_PCLK_ENABLE_b : 1;     /*!< [18..18] Static Clock gating Enable for
                                        mcpwm pclk 1'b1 => Clock  is enabled 1'b0
                                        => Invalid.  */
      __IOM unsigned int Reserved3 : 1;               /*!< [19..19] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int SGPIO_PCLK_ENABLE_b : 1;     /*!< [20..20] Static Clock gating Enable for
                                       sgpio pclk 1'b1 => Clock is enabled 1'b0
                                       => Invalid. */
      __IOM unsigned int EGPIO_PCLK_ENABLE_b : 1;     /*!< [21..21] Static Clock gating Enable for
                                       egpio pclk 1'b1 => Clock is enabled 1'b0
                                       => Invalid. */
      __IOM unsigned int ARM_CLK_ENABLE_b : 1;        /*!< [22..22] Static Clock gating
                                               Enable for arm clk 1'b1 => Clock
                                                 is enabled 1'b0 => Invalid. */
      __IOM unsigned int SSI_MST_PCLK_ENABLE_b : 1;   /*!< [23..23] Static Clock gating Enable
                                         for ssi master pclk 1'b1
                                           => Clock is enabled 1'b0 => Invalid.
                                       */
      __IOM unsigned int SSI_MST_SCLK_ENABLE_b : 1;   /*!< [24..24] Static Clock gating Enable
                                         for ssi master sclk 1'b1
                                           => Clock is enabled 1'b0 => Invalid.
                                       */
      __IOM unsigned int Reserved4 : 1;               /*!< [25..25] It is recommended to write
                                         these bits to 0.                      */
      __IOM unsigned int MEM_CLK_ULP_ENABLE_b : 1;    /*!< [26..26] Static Clock gating Enable for
                                        mem ulp clk 1'b1 =>
                                          Clock is enabled 1'b0 => Invalid. */
      __IOM unsigned int ROM_CLK_ENABLE_b : 1;        /*!< [27..27] Static Clock gating
                                               Enable for rom clk 1'b1 => Clock
                                                 is enabled 1'b0 => Invalid. */
      __IOM unsigned int PLL_INTF_CLK_ENABLE_b : 1;   /*!< [28..28] Static Clock gating Enable
                                         for pll intf clk 1'b1 => Clock is
                                         enabled 1'b0 => Invalid. */
      __IOM unsigned int Reserved5 : 3;               /*!< [31..29] It is recommended to write
                                         these bits to 0.                      */
    } CLK_ENABLE_SET_REG2_b;
  };

  union {
    __IOM unsigned int CLK_ENABLE_CLEAR_REG2; /*!< (@ 0x0000000C) Clock Enable Clear
                                              Register 2 */

    struct {
      __IOM unsigned int GEN_SPI_MST1_HCLK_ENABLE_b : 1; /*!< [0..0] Static Clock Clear for gen
                                              spi master1 hclk 1'b1 => Clock is
                                              Clear 1'b0 => Invalid */
      __IOM unsigned int Reserved1 : 5;                  /*!< [5..1] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int UDMA_HCLK_ENABLE_b : 1;         /*!< [6..6] Static Clock Clear for
                                                 udma hclk 1'b1 => Clock is Clear
                                                  1'b0 => Invalid. */
      __IOM unsigned int I2C_BUS_CLK_ENABLE_b : 1;       /*!< [7..7] Static Clock Clear
                                                   for i2c-1 bus clk1'b1 => Clock
                                                   is Clear 1'b0 => Invalid. */
      __IOM unsigned int I2C_2_BUS_CLK_ENABLE_b : 1;     /*!< [8..8] Static Clock Clear for i2c-2
                                          bus clk 1'b1 => Clock is
                                           Clear 1'b0 => Invalid. */
      __IOM unsigned int SSI_SLV_PCLK_ENABLE_b : 1;      /*!< [9..9] Static Clock Clear for ssi
                                         slave pclk 1'b1 => Clock is Clear 1'b0
                                         => Invalid. */
      __IOM unsigned int SSI_SLV_SCLK_ENABLE_b : 1;      /*!< [10..10] Static Clock Clear for ssi
                                         slave sclk 1'b1 => Clock is Clear 1'b0
                                         => Invalid. */
      __IOM unsigned int QSPI_CLK_ENABLE_b : 1;          /*!< [11..11] Static Clock Clear for qspi clk
                                     1'b1 => Clock is Clear 1'b0 => Invalid. */
      __IOM unsigned int QSPI_HCLK_ENABLE_b : 1;         /*!< [12..12] Static Clock Clear
                                                 for qspi hclk 1'b1 => Clock is
                                                 Clear 1'b0 => Invalid. */
      __IOM unsigned int I2SM_SCLK_ENABLE_b : 1;         /*!< [13..13] Static Clock Clear
                                                 for sclk of I2S at Root Clock
                                                 generation 1'b1 => Clock is
                                                 Clear 1'b0 => Invalid. */
      __IOM unsigned int I2SM_INTF_SCLK_ENABLE_b : 1;    /*!< [14..14] Static Clock Clear for i2s
                                           interface sclk 1'b1 => Clock
                                           is Clear 1'b0 => Invalid. */
      __IOM unsigned int I2SM_PCLK_ENABLE_b : 1;         /*!< [15..15] Static Clock Clear for i2s
                                        master pclk 1'b1 => Clock
                                        is Clear 1'b0 => Invalid.   */
      __IOM unsigned int Reserved2 : 1;                  /*!< [16..16] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int QE_PCLK_ENABLE_b : 1;           /*!< [17..17] Static Clock Clear for qe pclk
                                    1'b1 => Clock is Clear 1'b0 => Invalid. */
      __IOM unsigned int MCPWM_PCLK_ENABLE_b : 1;        /*!< [18..18] Static Clock Clear
                                                  for mcpwm pclk 1'b1 => Clock is
                                                      Clear 1'b0 => Invalid. */
      __IOM unsigned int Reserved3 : 1;                  /*!< [19..19] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int SGPIO_PCLK_ENABLE_b : 1;        /*!< [20..20] Static Clock Clear
                                                  for sgpio pclk 1'b1 => Clock is
                                                    Clear 1'b0 => Invalid. */
      __IOM unsigned int EGPIO_PCLK_ENABLE_b : 1;        /*!< [21..21] Static Clock Clear
                                                  for egpio pclk 1'b1 => Clock is
                                                    Clear 1'b0 => Invalid. */
      __IOM unsigned int ARM_CLK_ENABLE_b : 1;           /*!< [22..22] Static Clock Clear for arm clk
                                    1'b1 => Clock is Clear 1'b0 => Invalid. */
      __IOM unsigned int SSI_MST_PCLK_ENABLE_b : 1;      /*!< [23..23] Static Clock Clear for ssi
                                         master pclk 1'b1 => Clock
                                           is Clear 1'b0 => Invalid. */
      __IOM unsigned int SSI_MST_SCLK_ENABLE_b : 1;      /*!< [24..24] Static Clock Clear for ssi
                                         master sclk 1'b1 => Clock
                                           is Clear 1'b0 => Invalid. */
      __IOM unsigned int Reserved4 : 1;                  /*!< [25..25] It is recommended to write
                                         these bits to 0.                      */
      __IOM unsigned int MEM_CLK_ULP_ENABLE_b : 1;       /*!< [26..26] Static Clock Clear
                                                   for mem ulp clk 1'b1 => Clock
                                                   is Clear 1'b0 => Invalid. */
      __IOM unsigned int ROM_CLK_ENABLE_b : 1;           /*!< [27..27] Static Clock Clear for rom clk
                                    1'b1 => Clock is Clear 1'b0 => Invalid. */
      __IOM unsigned int PLL_INTF_CLK_ENABLE_b : 1;      /*!< [28..28] Static Clock Clear for pll
                                         intf clk 1'b1 => Clock is
                                           Clear 1'b0 => Invalid. */
      __IOM unsigned int Reserved5 : 3;                  /*!< [31..29] It is recommended to write
                                         these bits to 0.                      */
    } CLK_ENABLE_CLEAR_REG2_b;
  };

  union {
    __IOM unsigned int CLK_ENABLE_SET_REG3; /*!< (@ 0x00000010) Clock Enable Set
                                            Register 3 */

    struct {
      __IOM unsigned int BUS_CLK_ENABLE_b : 1;                   /*!< [0..0] Static Clock gating
                                               Enable for bus clk 1'b1 => Clock
                                                  is enabled 1'b0 => Invalid */
      __IOM unsigned int M4_CORE_CLK_ENABLE_b : 1;               /*!< [1..1] Static Clock gating Enable for
                                        M4 Core clk 1'b1 => Clock
                                           is enabled 1'b0 => Invalid. */
      __IOM unsigned int CM_BUS_CLK_ENABLE_b : 1;                /*!< [2..2] Static Clock gating Enable for cm
                                       bus clk1'b1 => Clock is enabled1'b0 =>
                                       Invalid. */
      __IOM unsigned int Reserved1 : 1;                          /*!< [3..3] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int MISC_CONFIG_PCLK_ENABLE_b : 1;          /*!< [4..4] Static Clock gating Enable
                                             for misc config regs clk 1'b1
                                                  => Clock is enabled 1'b0 =>
                                             Invalid. */
      __IOM unsigned int EFUSE_CLK_ENABLE_b : 1;                 /*!< [5..5] Static Clock gating Enable for
                                      efuse clk 1'b1 => Clock
                                           is enabled 1'b0 => Invalid. */
      __IOM unsigned int ICM_CLK_ENABLE_b : 1;                   /*!< [6..6] Static Clock gating Enable for icm
                                    clk 1'b1 => Clock
                                         is enabled 1'b0 => Invalid. */
      __IOM unsigned int Reserved2 : 6;                          /*!< [12..7] It is recommended to write
                                        these bits to 0.                       */
      __IOM unsigned int QSPI_CLK_ONEHOT_ENABLE_b : 1;           /*!< [13..13] Static Clock gating Enable
                                           for QSPI clock generated from the
                                           dynamic mux 1b1 - Clock is enabled 1b0
                                           - Invalid.                */
      __IOM unsigned int QSPI_M4_SOC_SYNC_b : 1;                 /*!< [14..14] Specifies whether QSPI clock is
                                      in sync with Soc clock. Before enabling
                                      this make sure that qspi_clk_onehot_enable
                                           is 1b0 to enable glitch free switching
                                      1b1 - QSPI clock is in sync with M4 clock
                                      1b0 - Invalid. */
      __IOM unsigned int Reserved3 : 1;                          /*!< [15..15] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int EGPIO_CLK_ENABLE_b : 1;                 /*!< [16..16] Static Clock gating enable for
                                      Enhanced-GPIO 1b1 -
                                          Clock is enabled 1b0 - Invalid. */
      __IOM unsigned int I2C_CLK_ENABLE_b : 1;                   /*!< [17..17] Static Clock gating enable for
                                    I2C-1 Module 1b1 - Clock is enabled 1b0 -
                                    Invalid. */
      __IOM unsigned int I2C_2_CLK_ENABLE_b : 1;                 /*!< [18..18] Static Clock gating enable for
                                      I2C-2 Module 1b1 - Clock is enabled 1b0 -
                                      Invalid. */
      __IOM unsigned int EFUSE_PCLK_ENABLE_b : 1;                /*!< [19..19] Static Clock gating
                                                  enable for EFUSE APB Interface
                                                      1b1 - Clock is enabled 1b0
                                                  - Invalid. */
      __IOM unsigned int SGPIO_CLK_ENABLE_b : 1;                 /*!< [20..20] Static Clock gating enable for
                                      SIO Module 1b1 - Clock is enabled 1b0 -
                                      Invalid. */
      __IOM unsigned int TASS_M4SS_64K_SWITCH_CLK_ENABLE_b : 1;  /*!< [21..21] Unused. */
      __IOM unsigned int TASS_M4SS_128K_SWITCH_CLK_ENABLE_b : 1; /*!< [22..22] Unused. */
      __IOM unsigned int TASS_M4SS_SDIO_SWITCH_CLK_ENABLE_b : 1; /*!< [23..23] Unused. */
      __IOM unsigned int Reserved4 : 1;                          /*!< [24..24] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int ROM_MISC_STATIC_ENABLE_b : 1;           /*!< [25..25] Static Clock gating enable
                                           for rom ahb Clock 1b1 - Clock is
                                           enabled 1b0 - Invalid. */
      __IOM unsigned int M4_SOC_CLK_FOR_OTHER_ENABLE_b : 1;      /*!< [26..26] Static Clock gating
                                                 enable for M4-SOC Other Clock
                                                 1b1
                                                        - Clock is enabled 1b0 -
                                                 Invalid. */
      __IOM unsigned int ICACHE_ENABLE_b : 1;                    /*!< [27..27] Static Clock gating enable for
                                        Icache. This has to      be enable for Icache
                                        operations. 1b1 - Clock is enabled      1b0 -
                                        Invalid.      */
      __IOM unsigned int Reserved5 : 4;                          /*!< [31..28] It is recommended to write
                                        these bits to 0.                      */
    } CLK_ENABLE_SET_REG3_b;
  };

  union {
    __IOM unsigned int CLK_ENABLE_CLEAR_REG3; /*!< (@ 0x00000014) Clock Enable Clear
                                              Register 3 */

    struct {
      __IOM unsigned int BUS_CLK_ENABLE_b : 1;                   /*!< [0..0] Static Clock Clear for bus clk 1'b1
                                    => Clock is Clear 1'b0 => Invalid */
      __IOM unsigned int M4_CORE_CLK_ENABLE_b : 1;               /*!< [1..1] Static Clock Clear
                                                   for M4 Core clk 1'b1 => Clock
                                                   is Clear 1'b0 => Invalid. */
      __IOM unsigned int CM_BUS_CLK_ENABLE_b : 1;                /*!< [2..2] Static Clock gating Enable for cm
                                       bus clk1'b1 => Clock is enabled1'b0 =>
                                       Invalid. */
      __IOM unsigned int Reserved1 : 1;                          /*!< [3..3] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int MISC_CONFIG_PCLK_ENABLE_b : 1;          /*!< [4..4] Static Clock Clear for misc
                                             config regs clk 1'b1 => Clock is
                                             Clear 1'b0 => Invalid. */
      __IOM unsigned int EFUSE_CLK_ENABLE_b : 1;                 /*!< [5..5] Static Clock Clear for
                                                 efuse clk 1'b1 => Clock is Clear
                                                      1'b0 => Invalid. */
      __IOM unsigned int ICM_CLK_ENABLE_b : 1;                   /*!< [6..6] Static Clock Clear for icm clk 1'b1
                                    => Clock is Clear 1'b0 => Invalid. */
      __IOM unsigned int Reserved2 : 6;                          /*!< [12..7] It is recommended to write
                                        these bits to 0.                       */
      __IOM unsigned int QSPI_CLK_ONEHOT_ENABLE_b : 1;           /*!< [13..13] Static Clock Clear for
                                           QSPI clock generated from the
                                           dynamic mux 1b1 - Clock is Gated 1b0 -
                                           Invalid.                           */
      __IOM unsigned int QSPI_M4_SOC_SYNC_b : 1;                 /*!< [14..14] Specifies whether QSPI clock is
                                      in sync with Soc clock. Before enabling
                                      this make sure that qspi_clk_onehot_enable
                                           is 1b0 to enable glitch free switching
                                      1b1 - QSPI clock is in sync with M4 clock
                                      1b0 - Invalid. */
      __IOM unsigned int Reserved3 : 1;                          /*!< [15..15] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int EGPIO_CLK_ENABLE_b : 1;                 /*!< [16..16] Static Clock Disable for
                                      Enhanced-GPIO 1b1 - Clock
                                          is Disable 1b0 - Invalid. */
      __IOM unsigned int I2C_CLK_ENABLE_b : 1;                   /*!< [17..17] Static Clock Disable
                                                 for I2C-1 Module 1b1 - Clock is
                                                     Disable 1b0 - Invalid.   */
      __IOM unsigned int I2C_2_CLK_ENABLE_b : 1;                 /*!< [18..18] Static Clock Disable
                                                 for I2C-2 Module 1b1 - Clock is
                                                     Disable 1b0 - Invalid. */
      __IOM unsigned int EFUSE_PCLK_ENABLE_b : 1;                /*!< [19..19] Static Clock Disable for EFUSE
                                       APB Interface 1b1 -
                                           Clock is Disable 1b0 - Invalid. */
      __IOM unsigned int SGPIO_CLK_ENABLE_b : 1;                 /*!< [20..20] Static Clock gating enable for
                                      SIO Module 1b1 - Clock is enabled 1b0 -
                                      Invalid. */
      __IOM unsigned int TASS_M4SS_64K_SWITCH_CLK_ENABLE_b : 1;  /*!< [21..21] Unused. */
      __IOM unsigned int TASS_M4SS_128K_SWITCH_CLK_ENABLE_b : 1; /*!< [22..22] Unused. */
      __IOM unsigned int TASS_M4SS_SDIO_SWITCH_CLK_ENABLE_b : 1; /*!< [23..23] Unused. */
      __IOM unsigned int Reserved4 : 1;                          /*!< [24..24] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int ROM_MISC_STATIC_ENABLE_b : 1;           /*!< [25..25] Static Clock Disable for
                                           rom ahb Clock 1b1 - Clock is Disable
                                           1b0 - Invalid. */
      __IOM unsigned int M4_SOC_CLK_FOR_OTHER_ENABLE_b : 1;      /*!< [26..26] Static Clock Disable
                                                 for M4-SOC Other Clock 1b1 -
                                                 Clock
                                                        is Disable 1b0 - Invalid.
                                               */
      __IOM unsigned int ICACHE_ENABLE_b : 1;                    /*!< [27..27] Static Clock Disable for Icache.
                                   This has to be enable for Icache operations.
                                   1b1 - Clock is Disable 1b0 - Invalid. */
      __IOM unsigned int Reserved5 : 4;                          /*!< [31..28] It is recommended to write
                                        these bits to 0.                      */
    } CLK_ENABLE_CLEAR_REG3_b;
  };

  union {
    __IOM unsigned int CLK_CONFIG_REG1; /*!< (@ 0x00000018) Clock Config Register 1 */

    struct {
      __IOM unsigned int QSPI_CLK_SEL : 3;     /*!< [2..0] Selects one of the following clocks for
                                ssi master 000
                                   - ULP Ref Clock(generated inside M4SS based on
                                m4ss_ref_clk_sel from NPSS) 001 - Intf PLL Clock
                                Clock (program bypass_intf_pll_clk if the bypass
                                clock has to be selected) 010 - Modem PLL
                                   Clock2(Not Intended for the programmer)
                                (program bypass_modem_pll_clk
                                   if the bypass clock has to be selected) 011 -
                                SoC PLL Clock               */
      __IOM unsigned int QSPI_CLK_DIV_FAC : 6; /*!< [8..3] Clock divison factor for QSPI. If
                                    qspi_clk_enable is 1b0 clock is gated. Else
                                    1)when qspi_clk_swallow_sel is 1b1 and
                                    qspi_odd_div_sel is 1b0 output clock is a
                                    swallowed clock with the following frequency.
                                    6h0,6h1 => clk_out = clk_in >6h1 => clk_out =
                                    clk_in/ qspi_clk_div_fac 2)when
                                       qspi_clk_swallow_sel is 1b0 */
      __IOM
      unsigned int QSPI_CLK_SWALLOW_SEL : 1;           /*!< [9..9] Clock select for clock
                                             swallow or clock divider for QSPI  1b0
                                             => 50% divider is selected with
                                             division factor qspi_clk_div_fac  1b1
                                             => Swallowed clock is selected with
                                             division factor  qspi_clk_div_fac
                                             Before Changing this ensure that the
                                             input  clocks are gated  */
      __IOM unsigned int SLP_RF_CLK_SEL : 1;           /*!< [10..10] clock select for
                                             m4_soc_rf_ref_clk 0 - m4_soc_clk 1
                                                - rf_ref_clk. */
      __IOM unsigned int SSI_MST_SCLK_DIV_FAC : 4;     /*!< [14..11] Clock division factor for
                                        ssi_mst_sclk. If ssi_mst_sclk_enable is
                                        1b0 clock is gated. Else output clock is
                                        a swallowed clock with the following
                                        frequency. 4h0,4h1 => Divider
                                               is bypassed >4h1 => clk_out =
                                        clk_in/ ssi_mst_sclk_div_fac. */
      __IOM unsigned int SSI_MST_SCLK_SEL : 3;         /*!< [17..15] Selects one of the following
                                    clocks for ssi master 000 - ULP Ref
                                    Clock(generated inside M4SS based on
                                    m4ss_ref_clk_sel from NPSS) 001 - SoC PLL
                                    Clock(program bypass_soc_pll_clk if the
                                    bypass clock has to be selected) 010 - Modem
                                    PLL Clock1(Not Intended for the programmer)
                                    (program bypass_modem_pll_clk if the bypass
                                    clock has to be selected) 011 - Intf PLL
                                           Clock(program bypass_intf_pll_clk if
                                    the                                  */
      __IOM unsigned int PLL_INTF_CLK_SEL : 1;         /*!< [18..18] Selects one of the following
                                    clocks for pll intf clock 0 - Intf Pll
                                    Clock(program bypass_intf_pll_clk if the
                                    bypass clock has to be selected) 1 - SoC Pll
                                    Clock(program bypass_soc_pll_clk if the
                                    bypass clock has to be selected) */
      __IOM unsigned int PLL_INTF_CLK_DIV_FAC : 4;     /*!< [22..19] Clock division factor for
                                        pll_intf_clk. If pll_intf_clk_enable is
                                        1b0 clock is gated. Else, when
                                        pll_intf_clk_swallow_sel is 1b1, output
                                        clock is a swallowed clock. when
                                        pll_intf_clk_swallow_sel is 1b0, output
                                        clock is a 50 Per duty cycle clock. */
      __IOM unsigned int PLL_INTF_CLK_SWALLOW_SEL : 1; /*!< [23..23] Clock select for clock
                                           swallow or clock divider for PLL INTF
                                           Clk 1b0 - 50% divider is selected with
                                           division factor 2; 1b1 - Swallowed
                                           clock is selected with division
                                           factor pll_intf_clk_div_fac */
      __IOM unsigned int GEN_SPI_MST1_SCLK_SEL : 3;    /*!< [26..24] Selects one of the following
                                        clocks for USART1 clk 000 -
                                        m4_soc_clk_for_other_clocks 001 - ulp ref
                                        Clock(generated inside M4SS based on
                                        m4ss_ref_clk_sel from NPSS) 010 - SoC PLL
                                        Clock(program bypass_soc_pll_clk if the
                                        bypass clock has to be selected) 011 -
                                        Modem PLL Clock2(Not Intended for the
                                        pragrammer) (program bypass_modem_pll_clk
                                        if the bypass clock has to be sele */
      __IOM unsigned int Reserved1 : 5;                /*!< [31..27] It is recommended to write
                                         these bits to 0.                      */
    } CLK_CONFIG_REG1_b;
  };

  union {
    __IOM unsigned int CLK_CONFIG_REG2; /*!< (@ 0x0000001C) Clock Config Register 1 */

    struct {
      __IOM unsigned int USART1_SCLK_SEL : 3;      /*!< [2..0] Selects one of the following clocks
                                   for USART1 clk 000
                                       - ulp ref Clock(generated inside M4SS
                                   based on m4ss_ref_clk_sel from NPSS) 001 - SoC
                                   PLL Clock(program bypass_soc_pll_clk if the
                                   bypass clock has to be selected) 010 - Modem
                                   PLL Clock2(Not Intended for the
                                   pragrammer)(program bypass_modem_pll_clk if
                                   the bypass clock has to be selected) 011 -
                                   Intf PLL
                                       Clock(program bypass_intf_pll_clk if the b
                                 */
      __IOM unsigned int USART1_SCLK_DIV_FAC : 4;  /*!< [6..3] Clock division factor for USART1
                                       Clock. If usart1_sclk_enable is 1b0 clock
                                       is gated. Else output clock is a swallowed
                                           clock. */
      __IOM unsigned int USART2_SCLK_SEL : 3;      /*!< [9..7] Selects one of the following clocks
                                   for USART2 clk 000
                                       - ulp ref Clock(generated inside M4SS
                                   based on m4ss_ref_clk_sel from NPSS) 001 - SoC
                                   PLL Clock(program bypass_soc_pll_clk if the
                                   bypass clock has to be selected) 010 - Modem
                                   PLL Clock2(Not Intended for the
                                   pragrammer)(program bypass_modem_pll_clk if
                                   the bypass clock has to be selected) 011 -
                                   Intf PLL
                                       Clock(program bypass_intf_pll_clk if the b
                                 */
      __IOM unsigned int USART2_SCLK_DIV_FAC : 4;  /*!< [13..10] Clock division factor for
                                         USART2 Clock. If usart2_sclk_enable   is 1b0
                                         clock is gated. Else output clock is a
                                         swallowed   clock.   */
      __IOM unsigned int Reserved1 : 14;           /*!< [27..14] It is recommended to write
                                         these bits to 0.                      */
      __IOM unsigned int QSPI_ODD_DIV_SEL : 1;     /*!< [28..28] Clock select for clock swallow or
                                    50% even clock divider or 50% odd divider
                                    clock for QSPI 1b1 - 50% odd clock divider
                                       output is selected with division factor
                                    qspi_clk_div_fac 1b0 - 50% even clock divider
                                    output or swallowed is selected
                                       with division factor qspi_clk_div_fac
                                    based on qspi_clk_swallow_sel.      */
      __IOM unsigned int USART1_SCLK_FRAC_SEL : 1; /*!< [29..29] Selects the type of divider
                                        for uart1_clk 1b0 - Clock Swallow is
                                        selected 1b1 - Fractional Divider is
                                        selected.                 */
      __IOM unsigned int USART2_SCLK_FRAC_SEL : 1; /*!< [30..30] Selects the type of divider
                                        for uart2_clk 1b0 - Clock Swallow is
                                        selected 1b1 - Fractional Divider is
                                        selected.                 */
      __IOM unsigned int USART3_SCLK_FRAC_SEL : 1; /*!< [31..31] Selects the type of divider
                                        for uart3_clk 1b0 - Clock Swallow is
                                        selected 1b1 - Fractional Divider is
                                        selected.                 */
    } CLK_CONFIG_REG2_b;
  };

  union {
    __IOM unsigned int CLK_CONFIG_REG3; /*!< (@ 0x00000020) Clock Config Register 3 */

    struct {
      __IOM unsigned int Reserved1 : 8;          /*!< [7..0] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int MCU_CLKOUT_SEL : 4;     /*!< [11..8] Clock Select for the clock
                                             on mcu_clkout (Mapped to GPIO) */
      __IOM unsigned int MCU_CLKOUT_DIV_FAC : 6; /*!< [17..12] Division factor for
                                                 mcu_clkout (Mapped to GPIO) */
      __IOM unsigned int MCU_CLKOUT_ENABLE : 1;  /*!< [18..18] Clock Enable for the clock on
                                         nwp_clkout (Mapped to     GPIO) 1b0 - Clock is
                                         Gated 1b1 - Clock is Enabled     */
      __IOM unsigned int Reserved2 : 13;         /*!< [31..19] It is recommended to write
                                         these bits to 0.                      */
    } CLK_CONFIG_REG3_b;
  };

  union {
    __IOM unsigned int CLK_CONFIG_REG4; /*!< (@ 0x00000024) Clock Config Register 4 */

    struct {
      __IOM unsigned int SOC_PLL_CLK_BYP_SEL : 2;             /*!< [1..0] Selects one of the bypass clocks
                                       for SoC PLL Clock                 */
      __IOM unsigned int I2S_PLL_CLK_BYP_SEL : 2;             /*!< [3..2] Selects one of the bypass clocks
                                       for I2S PLL Clock                 */
      __IOM unsigned int MODEM_PLL_CLK_BYP_SEL : 2;           /*!< [5..4] Selects one of the bypass
                                         clocks for Modem PLL Clock */
      __IOM unsigned int INTF_PLL_CLK_BYP_SEL : 2;            /*!< [7..6] Selects one of the bypass clocks
                                        for Intf PLL Clock                */
      __IOM unsigned int SOC_INTF_PLL_BYPCLK_CLKCLNR_ON : 1;  /*!< [8..8] Clock cleaner ON
                                                  Control for SoC PLL Bypass
                                                  Clock          */
      __IOM unsigned int SOC_INTF_PLL_BYPCLK_CLKCLNR_OFF : 1; /*!< [9..9] Clock cleaner OFF
                                                   Control for SoC PLL Bypass
                                                   Clock        */
      __IOM unsigned int Reserved1 : 2;                       /*!< [11..10] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int I2S_PLL_BYPCLK_CLKCLNR_ON : 1;       /*!< [12..12] Clock cleaner ON Control
                                             for I2S PLL Bypass Clock. */
      __IOM unsigned int I2S_PLL_BYPCLK_CLKCLNR_OFF : 1;      /*!< [13..13] Clock cleaner
                                                         OFF Control for I2S PLL
                                                         Bypass Clock. */
      __IOM unsigned int MODEM_PLL_BYPCLK_CLKCLNR_ON : 1;     /*!< [14..14] Clock cleaner ON
                                               Control for Modem PLL Bypass
                                               Clock.        */
      __IOM unsigned int MODEM_PLL_BYPCLK_CLKCLNR_OFF : 1;    /*!< [15..15] Clock cleaner OFF
                                                Control for Modem PLL Bypass
                                                Clock.      */
      __IOM unsigned int BYPASS_SOC_PLL_CLK : 1;              /*!< [16..16] Select to choose bypass clock or
                                      PLL clock  1b0 - soc_pll_clk 1b1 - One of
                                      the bypass clocks based on
                                      soc_pll_clk_byp_sel.              */
      __IOM unsigned int BYPASS_I2S_PLL_CLK : 1;              /*!< [17..17] Select to choose bypass clock or
                                      PLL clock  1b0 - i2s_pll_clk 1b1 - One of
                                      the bypass clocks based on
                                      soc_pll_clk_byp_sel.              */
      __IOM unsigned int BYPASS_MODEM_PLL_CLK1 : 1;           /*!< [18..18] Select to choose bypass clock
                                         or PLL clock 1b0 - modem_pll_clk1
                                                        1b1 - One of the bypass
                                         clocks based on modem_pll_clk_byp_sel.
                                       */
      __IOM unsigned int BYPASS_MODEM_PLL_CLK2 : 1;           /*!< [19..19] Select to choose bypass clock
                                         or PLL clock  1b0 - modem_pll_clk2
                                                        1b1 - One of the bypass
                                         clocks based on modem_pll_clk_byp_sel.
                                       */
      __IOM unsigned int BYPASS_INTF_PLL_CLK : 1;             /*!< [20..20] Select to choose bypass clock
                                       or PLL clock  1b0 - intf_pll_clk
                                                 1b1 - One of the bypass clocks
                                       based on soc_pll_clk_byp_sel. */
      __IOM unsigned int SLEEP_CLK_SEL : 2;                   /*!< [22..21] Select to choose sleep clk
                                            00 - ulp_32khz_rc_clk 01
                                                      - ulp_32khz_xtal_clk 10 -
                                            Gated 11 - ulp_32khz_ro_clk. */
      __IOM unsigned int Reserved2 : 2;                       /*!< [24..23] It is recommended to write
                                            these bits to 0.                      */
      __IOM unsigned int ULPSS_CLK_DIV_FAC : 6;               /*!< [30..25] Clock division factor for clock
                                     to ULPSS. If ulpss_clk_enable is 1b0 clock
                                     is gated. Else output clock is a divided
                                     clock with the following frequency. 6h0 -
                                     Divider is bypassed > 6h0 - clk_out =
                                     clk_in/ 2* ulpss_clk_div_fac */
      __IOM unsigned int Reserved3 : 1;                       /*!< [31..31] It is recommended to write
                                        these bits to 0.                      */
    } CLK_CONFIG_REG4_b;
  };

  union {
    __IOM unsigned int CLK_CONFIG_REG5; /*!< (@ 0x00000028) Clock Config Register 5 */

    struct {
      __IOM unsigned int M4_SOC_CLK_SEL : 4;      /*!< [3..0] Selects one of the clock sources for
                                  M4 SoC clock. These clocks are selected for
                                  m4_soc_clk when 1)m4_soc_host_clk_sel is 1b0 or
                                  2)when m4_soc_host_clk_sel is 1b1, xtal is
                                  ON(xtal_off from slp_fsm should be zero) and
                                  host_clk_available(from host logic) is 1b0.
                                  0000 - ULP Ref Clock (generated inside M4SS
                                  based on m4ss_ref_clk_sel from NPSS) 0001 -
                                  Reserved 0010 - */
      __IOM unsigned int M4_SOC_CLK_DIV_FAC : 6;  /*!< [9..4] Clock divison factor for NWP SoC
                                      Clock If ta_soc_clk_enable(from NPSS) is
                                      1b0 clock is gated. Else output clock is a
                                      swallowed clock with the following
                                      frequency. 6h0,6h1 - Divider is bypassed
                                      >6h1 - clk_out = clk_in/ ta_soc_clk_div_fac
                                    */
      __IOM unsigned int I2S_CLK_SEL : 1;         /*!< [10..10] Selects one of the following clocks for
                               config timer I2S interface 00/11 - I2S PLL Clock
                               (program bypass_i2s_pll_clk if the bypass clock
                               has to be selected) 01 - I2S PLL Clock_1 (program
                               bypass_i2s_pll_clk_1 if the bypass clock has to
                                       be selected) 10 -
                               m4_soc_clk_for_other_clocks */
      __IOM unsigned int I2S_CLK_DIV_FAC : 6;     /*!< [16..11] Clock division factor for i2s_clk.
                                   Else output clock is a 50% divided clock with
                                   the following frequency. 6h0
                                           - Divider is bypassed >6h0 - clk_out =
                                   clk_in/ 2*i2s_clk_div_fac          */
      __IOM unsigned int CT_CLK_SEL : 3;          /*!< [19..17] Selects one of the following clocks for
                              config timer 000 - ulp ref Clock(generated inside
                              M4SS based on m4ss_ref_clk_sel from NPSS) 001 -
                              Intf PLL Clock(program bypass_intf_pll_clk if the
                              bypass clock has to be selected) 010 - SoC PLL
                              Clock(program bypass_soc_pll_clk if the bypass
                              clock has to be selected) 011 -
                              m4_soc_clk_for_other_clocks 100,110 - Invalid */
      __IOM unsigned int CT_CLK_DIV_FAC : 6;      /*!< [25..20] Clock division factor for sct_clk.
                                  If sct_clk_enable is 1b0 clock is gated. Else
                                  output clock is a 50% divided clock with the
                                  following frequency. 6h0 - Divider is bypassed
                                          >6h0 - clk_out = clk_in/
                                  2*sct_clk_div_fac */
      __IOM unsigned int M4_SOC_HOST_CLK_SEL : 1; /*!< [26..26] Selects the previous muxed
                                        output(xtal_clk) or host_clk  as the clock
                                        source for M4 SoC clock based on the
                                        following  combinations of {xtal_off(from
                                        slp fsm), host_clk_available(from  host
                                        logic),m4_soc_host_clk_sel} XX0 - xtal_clk
                                        001 - After  wait time based on
                                        mask_host_clk_wait_fix ; xtal_clk X11
                                            - host_clk 101 - No Clock  */
      __IOM unsigned int Reserved1 : 1;           /*!< [27..27] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int ULPSS_ODD_DIV_SEL : 1;   /*!< [28..28] Selects the type of divider for
                                     m4_soc_clk_2ulpss 1b0
                                           - Clock Divider(even) is selected 1b1
                                     - Odd Divider is selected. */
      __IOM unsigned int Reserved2 : 2;           /*!< [30..29] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int I2S_CLK_SEL_1 : 1;       /*!< [31..31] Selects one of the following clocks
                                 for config timer for I2S interface 00/11 - I2S
                                 PLL Clock (program bypass_i2s_pll_clk if the
                                 bypass clock has to be selected) 01 - I2S PLL
                                 Clock_1 (program bypass_i2s_pll_clk_1 if the
                                 bypass clock has to be selected) 10 -
                                 m4_soc_clk_for_other_clocks */
    } CLK_CONFIG_REG5_b;
  };
  __IM unsigned int RESERVED[6];

  union {
    __IOM unsigned int DYN_CLK_GATE_DISABLE_REG; /*!< (@ 0x00000044) Dynamic Clock
                                                 Gate Disable Register */

    struct {
      __IOM unsigned int SDIO_SYS_HCLK_DYN_CTRL_DISABLE_b : 1;    /*!< [0..0] Dynamic clock gate
                                                    disable control sdio sys
                                                    clk1'b0 => Dynamic control of
                                                    the clock is disbaled 1'b1 =>
                                                    Dynamic
                                                        control of the clock is
                                                    enabled */
      __IOM unsigned int BUS_CLK_DYN_CTRL_DISABLE_b : 1;          /*!< [1..1] Dynamic clock gate disable
                                              control bus clk1'b0 => Dynamic
                                                        control of the clock is
                                              disbaled 1'b1 => Dynamic control
                                                        of the clock is enabled
                                            */
      __IOM unsigned int Reserved1 : 2;                           /*!< [3..2] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int GPDMA_HCLK_DYN_CTRL_DISABLE_b : 1;       /*!< [4..4] Dynamic clock gate
                                                 disable control gpdma clk1'b0 =>
                                                 Dynamic control of the clock is
                                                 disbaled 1'b1 => Dynamic control
                                                        of the clock is enabled
                                               */
      __IOM unsigned int EGPIO_PCLK_DYN_CTRL_DISABLE_b : 1;       /*!< [5..5] Dynamic clock gate
                                                 disable control egpio clk1'b0 =>
                                                 Dynamic control of the clock is
                                                 disbaled 1'b1 => Dynamic control
                                                        of the clock is enabled
                                               */
      __IOM unsigned int SGPIO_PCLK_DYN_CTRL_DISABLE_b : 1;       /*!< [6..6] Dynamic clock gate
                                                 disable control sgpio clk1'b0 =>
                                                 Dynamic control of the clock is
                                                 disbaled 1'b1 => Dynamic control
                                                        of the clock is enabled
                                               */
      __IOM unsigned int TOT_CLK_DYN_CTRL_DISABLE_b : 1;          /*!< [7..7] Dynamic clock gate disable
                                              control tot clk1'b0 => Dynamic
                                                        control of the clock is
                                              disbaled 1'b1 => Dynamic control
                                                        of the clock is enabled
                                            */
      __IOM unsigned int Reserved2 : 1;                           /*!< [8..8] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int USART1_SCLK_DYN_CTRL_DISABLE_b : 1;      /*!< [9..9] Dynamic clock gate
                                                  disable control usart1 sclk1'b0
                                                  => Dynamic control of the clock
                                                  is disbaled 1'b1 => Dynamic
                                                        control of the clock is
                                                  enabled. */
      __IOM unsigned int USART1_PCLK_DYN_CTRL_DISABLE_b : 1;      /*!< [10..10] Dynamic clock gate
                                                  disable control usart1 pclk1'b0
                                                        => Dynamic control of the
                                                  clock is disbaled 1'b1 =>
                                                  Dynamic
                                                        control of the clock is
                                                  enabled. */
      __IOM unsigned int USART2_SCLK_DYN_CTRL_DISABLE_b : 1;      /*!< [11..11] Dynamic clock gate
                                                  disable control usart2 sclk1'b0
                                                        => Dynamic control of the
                                                  clock is disbaled 1'b1 =>
                                                  Dynamic
                                                        control of the clock is
                                                  enabled. */
      __IOM unsigned int USART2_PCLK_DYN_CTRL_DISABLE_b : 1;      /*!< [12..12] Dynamic clock gate
                                                  disable control usart2 pclk1'b0
                                                        => Dynamic control of the
                                                  clock is disbaled 1'b1 =>
                                                  Dynamic
                                                        control of the clock is
                                                  enabled. */
      __IOM unsigned int Reserved3 : 2;                           /*!< [14..13] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int SSI_SLV_SCLK_DYN_CTRL_DISABLE_b : 1;     /*!< [15..15] Dynamic clock gate
                                                   disable control ssi slave
                                                   sclk1'b0
                                                        => Dynamic control of the
                                                   clock is disbaled 1'b1 =>
                                                   Dynamic control of the clock
                                                   is enabled */
      __IOM unsigned int SSI_SLV_PCLK_DYN_CTRL_DISABLE_b : 1;     /*!< [16..16] Dynamic clock gate
                                                   disable control ssi slave
                                                   pclk1'b0
                                                        => Dynamic control of the
                                                   clock is disbaled 1'b1 =>
                                                   Dynamic control of the clock
                                                   is enabled */
      __IOM unsigned int Reserved4 : 2;                           /*!< [18..17] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int SEMAPHORE_CLK_DYN_CTRL_DISABLE_b : 1;    /*!< [19..19] Dynamic clock gate
                                                    disable control semaphore
                                                    clk1'b0
                                                        => Dynamic control of the
                                                    clock is disbaled 1'b1 =>
                                                    Dynamic
                                                        control of the clock is
                                                    enabled. */
      __IOM unsigned int ARM_CLK_DYN_CTRL_DISABLE_b : 1;          /*!< [20..20] Dynamic clock gate
                                              disable control arm clk1'b0 =>
                                              Dynamic control of the clock is
                                              disbaled 1'b1 => Dynamic control
                                                        of the clock is enabled.
                                            */
      __IOM unsigned int SSI_MST_SCLK_DYN_CTRL_DISABLE_b : 1;     /*!< [21..21] Dynamic clock gate
                                                   disable control ssi mst
                                                   sclk1'b0
                                                        => Dynamic control of the
                                                   clock is disbaled 1'b1 =>
                                                   Dynamic control of the clock
                                                   is enabled. */
      __IOM unsigned int Reserved5 : 2;                           /*!< [23..22] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int MEM_CLK_ULP_DYN_CTRL_DISABLE_b : 1;      /*!< [24..24] Dynamic clock gate
                                                  disable control mem clk1'b0 =>
                                                  Dynamic control of the clock is
                                                  disbaled 1'b1 => Dynamic
                                                  control of the clock is
                                                  enabled. */
      __IOM unsigned int Reserved6 : 3;                           /*!< [27..25] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int SSI_MST_PCLK_DYN_CTRL_DISABLE_b : 1;     /*!< [28..28] Dynamic clock gate
                                                   disable control ssi mst pclk
                                                   1'b0
                                                        => Dynamic control of the
                                                   clock is disbaled 1'b1 =>
                                                   Dynamic control of the clock
                                                   is enabled */
      __IOM unsigned int ICACHE_DYN_GATING_DISABLE_b : 1;         /*!< [29..29] Dynamic clock gate
                                               disable control icache clk1'b0 =>
                                                        Dynamic control of the
                                               clock is disbaled 1'b1 => Dynamic
                                                        control of the clock is
                                               enabled */
      __IOM unsigned int Reserved7 : 1;                           /*!< [30..30] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int MISC_CONFIG_PCLK_DYN_CTRL_DISABLE_b : 1; /*!< [31..31] Dynamic clock
                                                       gate disable control miscn
                                                       config pclk 1'b0 =>
                                                       Dynamic control of the
                                                       clock is disbaled 1'b1 =>
                                                        Dynamic control of the
                                                       clock is enabled. */
    } DYN_CLK_GATE_DISABLE_REG_b;
  };
  __IM unsigned int RESERVED1[2];

  union {
    __IOM unsigned int PLL_ENABLE_SET_REG; /*!< (@ 0x00000050) PLL Enable Set Register */

    struct {
      __IOM unsigned int SOCPLL_SPI_SW_RESET : 1; /*!< [0..0] SPI soft reset for SoC PLL1'b1 =>
                                       soft reset is enabled1'b0
                                           => Invalid */
      __IOM unsigned int Reserved1 : 31;          /*!< [31..1] It is recommended to write
                                         these bits to 0. */
    } PLL_ENABLE_SET_REG_b;
  };

  union {
    __IOM unsigned int PLL_ENABLE_CLEAR_REG; /*!< (@ 0x00000054) PLL Enable Clear Register */

    struct {
      __IOM unsigned int SOCPLL_SPI_SW_RESET : 1; /*!< [0..0] SPI soft reset for SoC PLL1'b1 =>
                                       soft reset is disabled1'b0
                                           => Invalid */
      __IOM unsigned int Reserved1 : 31;          /*!< [31..1] It is recommended to write
                                         these bits to 0. */
    } PLL_ENABLE_CLEAR_REG_b;
  };

  union {
    __IM unsigned int PLL_STAT_REG; /*!< (@ 0x00000058) PLL Status Register */

    struct {
      __IM unsigned int LCDPLL_LOCK : 1;                 /*!< [0..0] Lock Signal from LCD PLL  */
      __IM unsigned int DDRPLL_LOCK : 1;                 /*!< [1..1] Lock Signal from DDR PLL  */
      __IM unsigned int APPLL_LOCK : 1;                  /*!< [2..2] Lock Signal from AP PLL   */
      __IM unsigned int INTFPLL_LOCK : 1;                /*!< [3..3] Lock Signal from INTF PLL */
      __IM unsigned int I2SPLL_LOCK : 1;                 /*!< [4..4] Lock Signal from I2S PLL  */
      __IM unsigned int SOCPLL_LOCK : 1;                 /*!< [5..5] Lock Signal from SoC PLL  */
      __IM unsigned int MODEMPLL_LOCK : 1;               /*!< [6..6] Lock Signal from Modem PLL */
      __IM unsigned int PLL_LOCK_DATA_TRIG : 1;          /*!< [7..7] This is set to 1'b1 when the PLL
                                      Locks are equal to pll_lock_int_data_r
                                                        g */
      __IM unsigned int M4_SOC_CLK_SWITCHED : 1;         /*!< [8..8] Indication from M4 SoC
                                                 Clock Dynamic mux that the
                                                 switching happened */
      __IM unsigned int QSPI_CLK_SWITCHED : 1;           /*!< [9..9] Indication from QSPI Clock Dynamic
                                     mux that the switching happened */
      __IM unsigned int USART1_SCLK_SWITCHED : 1;        /*!< [10..10] Indication from
                                                  USART1 Clock Dynamic mux that
                                                  the switching happened */
      __IM unsigned int USART2_SCLK_SWITCHED : 1;        /*!< [11..11] Indication from
                                                  USART1 Clock Dynamic mux that
                                                  the switching happened */
      __IM unsigned int GEN_SPI_MST1_SCLK_SWITCHED : 1;  /*!< [12..12] Indication from USART2
                                              Clock Dynamic mux that the
                                              switching happened */
      __IM unsigned int SSI_MST_SCLK_SWITCHED : 1;       /*!< [13..13] Indication from SSi
                                                   Master SClock Dynamic mux that
                                                      the switching happened */
      __IM unsigned int Reserved1 : 1;                   /*!< [14..14] It is recommended to write
                                       these bits to 0.                      */
      __IM unsigned int CT_CLK_SWITCHED : 1;             /*!< [15..15] Indication from SCT Clock Dynamic
                                   mux that the switching happened */
      __IM unsigned int M4_TA_SOC_CLK_SWITCHED_SDIO : 1; /*!< [16..16] Indication
                                                         from M4-NWP Soc SDIO
                                                         Clock Dynamic mux that
                                                            the switching
                                                         happened(TBD) */
      __IM unsigned int I2S_CLK_SWITCHED : 1;            /*!< [17..17] Indication from I2S Clock Dynamic
                                    mux that the switching happened */
      __IM unsigned int PLL_INTF_CLK_SWITCHED : 1;       /*!< [18..18] Indication from Pll
                                                   Intf Clock Dynamic mux that
                                                   the switching happened */
      __IM unsigned int Reserved2 : 2;                   /*!< [20..19] It is recommended to write
                                       these bits to 0.                      */
      __IM unsigned int SLEEP_CLK_SWITCHED : 1;          /*!< [21..21] Indication from Sleep
                                                 clcok Dynamic mux that the
                                                 switching  happened  */
      __IM unsigned int MCU_CLKOUT_SWITCHED : 1;         /*!< [22..22] Indication from
                                                 mcu_clkout Dynamic mux that the
                                                 switching happened */
      __IM unsigned int QSPI_2_CLK_SWITCHED : 1;         /*!< [23..23] Indication from QSPI
                                                 Clock Dynamic mux that the
                                                 switching happened */
      __IM unsigned int TASS_M4SS_64K_CLK_SWITCHED : 1;  /*!< [24..24] Indication when NWP
                                                 accessing 2nd memory chunk of M4,
                                                 clock to Dynamic mux switching
                                                 happened      */
      __IM unsigned int CC_CLOCK_MUX_SWITCHED : 1;       /*!< [25..25] Indication from cc
                                                   clock Dynamic mux that the
                                                   switching happened */
      __IM unsigned int TASS_M4SS_192K_CLK_SWITCHED : 1; /*!< [26..26] Indication when NWP
                                              accessing 0th memory chunk of M4,
                                              clock to Dynamic mux switching
                                              happened   */
      __IM unsigned int USART1_CLK_SWITCHED : 1;         /*!< [27..27] Indication from
                                              usart1 sclk or pclk Dynamic mux
                                              that the switching happened */
      __IM unsigned int USART2_CLK_SWITCHED : 1;         /*!< [28..28] Indication from
                                              usart2 sclk or pclk Dynamic mux
                                              that the switching happened */
      __IM unsigned int TASS_M4SS_64K0_CLK_SWITCHED : 1; /*!< [29..29] Indication when NWP
                                            accessing 1st memory chunk of M4,
                                            clock to Dynamic mux switching
                                            happened */
      __IM unsigned int CLK_FREE_OR_SLP_SWITCHED : 1;    /*!< [30..30] Indication from
                                               clk_free_or_slp Dynamic mux that the
                                               switching happened       */
      __IM unsigned int ULP_REF_CLK_SWITCHED : 1;        /*!< [31..31] Indication from
                                                  ulp_ref_clk Dynamic mux that
                                                  the switching happened */
    } PLL_STAT_REG_b;
  };

  union {
    __IOM unsigned int PLL_LOCK_INT_MASK_REG; /*!< (@ 0x0000005C) PLL Lock Interrupt
                                              Mask Register */

    struct {
      __IOM unsigned int LCD_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;           /*!< [0..0] 1'b1 =>
                                                        Masked;1'b0 => Not Masked
                                                      */
      __IOM unsigned int DDR_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;           /*!< [1..1] 1'b1 =>
                                                        Masked;1'b0 => Not Masked
                                                      */
      __IOM unsigned int AP_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;            /*!< [2..2] 1'b1 =>
                                                       Masked;1'b0 => Not Masked
                                                     */
      __IOM unsigned int INTF_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;          /*!< [3..3] 1'b1 =>
                                                         Masked;1'b0 => Not
                                                         Masked */
      __IOM unsigned int I2S_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;           /*!< [4..4] 1'b1 =>
                                                        Masked;1'b0 => Not Masked
                                                      */
      __IOM unsigned int SOC_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;           /*!< [5..5] 1'b1 =>
                                                        Masked;1'b0 => Not Masked
                                                      */
      __IOM unsigned int MODEM_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;         /*!< [6..6] 1'b1 =>
                                                          Masked;1'b0 => Not
                                                          Masked */
      __IOM unsigned int PLL_LOCK_DATA_TRIGGER_MASK_BIT_OF_RISING_EDGE : 1;  /*!< [7..7] 1'b1 =>
                                                                 Masked;1'b0 =>
                                                                 Not Masked */
      __IOM unsigned int LCD_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;          /*!< [8..8] 1'b1 =>
                                                         Masked;1'b0 => Not
                                                         Masked */
      __IOM unsigned int DDR_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;          /*!< [9..9] 1'b1 =>
                                                         Masked;1'b0 => Not
                                                         Masked */
      __IOM unsigned int AP_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;           /*!< [10..10] 1'b1 =>
                                                        Masked;1'b0 => Not Masked
                                                      */
      __IOM unsigned int INTF_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;         /*!< [11..11] 1'b1 =>
                                                          Masked;1'b0 => Not
                                                          Masked */
      __IOM unsigned int I2S_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;          /*!< [12..12] 1'b1 =>
                                                         Masked;1'b0 => Not
                                                         Masked */
      __IOM unsigned int SOC_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;          /*!< [13..13] 1'b1 =>
                                                         Masked;1'b0 => Not
                                                         Masked */
      __IOM unsigned int MODEM_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;        /*!< [14..14] 1'b1 =>
                                                           Masked;1'b0 => Not
                                                           Masked */
      __IOM unsigned int PLL_LOCK_DATA_TRIGGER_MASK_BIT_OF_FALLING_EDGE : 1; /*!< [15..15] 1'b1
                                                                  => Masked;1'b0
                                                                  => Not Masked
                                                                */
      __IOM unsigned int Reserved1 : 16;                                     /*!< [31..16] It is recommended to write
                                         these bits to 0.                      */
    } PLL_LOCK_INT_MASK_REG_b;
  };

  union {
    __IOM unsigned int PLL_LOCK_INT_CLR_REG; /*!< (@ 0x00000060) PLL Lock Interrupt
                                             Clear Register */

    struct {
      __IOM unsigned int LCD_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;           /*!< [0..0] 1'b0 => Not
                                                         Cleared 1'b1 => Cleared
                                                       */
      __IOM unsigned int DDR_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;           /*!< [1..1] 1'b0 => Not
                                                         Cleared 1'b1 => Cleared
                                                       */
      __IOM unsigned int AP_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;            /*!< [2..2] 1'b0 => Not
                                                        Cleared 1'b1 => Cleared
                                                      */
      __IOM unsigned int INTF_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;          /*!< [3..3] 1'b0 => Not
                                                          Cleared 1'b1 => Cleared
                                                        */
      __IOM unsigned int I2S_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;           /*!< [4..4] 1'b0 => Not
                                                         Cleared 1'b1 => Cleared
                                                       */
      __IOM unsigned int SOC_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;           /*!< [5..5] 1'b0 => Not
                                                         Cleared 1'b1 => Cleared
                                                       */
      __IOM unsigned int MODEM_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;         /*!< [6..6] 1'b0 => Not
                                                           Cleared 1'b1 =>
                                                           Cleared */
      __IOM unsigned int PLL_LOCK_DATA_TRIGGER_CLEAR_BIT_OF_RISING_EDGE : 1;  /*!< [7..7] 1'b0
                                                                  => Not Cleared
                                                                  1'b1 => Cleared
                                                                */
      __IOM unsigned int LCD_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;          /*!< [8..8] 1'b0 => Not
                                                          Cleared 1'b1 => Cleared
                                                        */
      __IOM unsigned int DDR_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;          /*!< [9..9] 1'b0 => Not
                                                          Cleared 1'b1 => Cleared
                                                        */
      __IOM unsigned int AP_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;           /*!< [10..10] 1'b0 => Not
                                                         Cleared 1'b1 => Cleared
                                                       */
      __IOM unsigned int INTF_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;         /*!< [11..11] 1'b0 => Not
                                                           Cleared 1'b1 =>
                                                           Cleared             */
      __IOM unsigned int I2S_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;          /*!< [12..12] 1'b0 => Not
                                                          Cleared 1'b1 => Cleared
                                                        */
      __IOM unsigned int SOC_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;          /*!< [13..13] 1'b0 => Not
                                                          Cleared 1'b1 => Cleared
                                                        */
      __IOM unsigned int MODEM_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;        /*!< [14..14] 1'b0 =>
                                                            Not Cleared 1'b1 =>
                                                            Cleared            */
      __IOM unsigned int PLL_LOCK_DATA_TRIGGER_CLEAR_BIT_OF_FALLING_EDGE : 1; /*!<
                                                                              [15..15]
                                                                              1'b0
                                                                              =>
                                                                              Not
                                                                              Cleared
                                                                              1'b1
                                                                              =>
                                                                              Cleared
                                                                            */
      __IOM unsigned int Reserved1 : 16;                                      /*!< [31..16] It is recommended to write
                                         these bits to 0.                      */
    } PLL_LOCK_INT_CLR_REG_b;
  };

  union {
    __IOM unsigned int PLL_LOCK_INT_DATA_REG; /*!< (@ 0x00000064) PLL Lock Interrupt
                                              DATA Register */

    struct {
      __IOM unsigned int LCD_PLL_LOCK : 1;   /*!< [0..0] 1'b1 => LCD PLL Lock has to be used as
                                trigger1'b0 => LCD PLL Lock not to be used as
                                trigger                                    */
      __IOM unsigned int DDR_PLL_LOCK : 1;   /*!< [1..1] 1'b1 => DDR PLL Lock has to be used as
                                trigger1'b0 => DDR PLL Lock not to be used as
                                trigger                                    */
      __IOM unsigned int AP_PLL_LOCK : 1;    /*!< [2..2] 1'b1 => AP PLL Lock has to be used as
                               trigger1'b0 => Ap PLL Lock not to be used as
                               trigger                                     */
      __IOM unsigned int INTF_PLL_LOCK : 1;  /*!< [3..3] 1'b1 => INTF PLL Lock has to
                                            be used as trigger1'b0 => INTF PLL
                                            Lock not to be used as trigger */
      __IOM unsigned int I2S_PLL_LOCK : 1;   /*!< [4..4] 1'b1 => I2S PLL Lock has to be used as
                                trigger1'b0 => I2S PLL Lock not to be used as
                                trigger                                    */
      __IOM unsigned int SOC_PLL_LOCK : 1;   /*!< [5..5] 1'b1 => SoC PLL Lock has to be used as
                                trigger1'b0 => SoC PLL Lock not to be used as
                                trigger                                    */
      __IOM unsigned int MODEM_PLL_LOCK : 1; /*!< [6..6] 1'b1 => Modem PLL Lock has
                                             to be used as trigger1'b0
                                                      => Modem PLL Lock not to be
                                             used as trigger */
      __IOM unsigned int Reserved1 : 25;     /*!< [31..7] It is recommended to write
                                             these bits to 0.     */
    } PLL_LOCK_INT_DATA_REG_b;
  };

  union {
    __IOM unsigned int SLEEP_CALIB_REG; /*!< (@ 0x00000068) Sleep Calib Register */

    struct {
      __IOM unsigned int SLP_CALIB_START_b : 1;     /*!< [0..0] This bit is used to start the
                                     calibration. 1b1 - Start calibration.
                                     slp_calib_duration should be loaded before
                                       this bit is set. This bit is
                                     self-clearing. When read,
                                       if high indicates the completion of
                                     calibration process.                  */
      __IOM unsigned int SLP_CALIB_CYCLES : 2;      /*!< [2..1] These bits are used to program the
                                    number of clock cycles over which clock
                                    calibration is to be done. */
      __IOM unsigned int SLP_CALIB_DURATION_b : 16; /*!< [18..3] Duration of the sleep clock in
                                         terms of processor clocks. This has to
                                         be divided with number of calibration
                                         cycles to get number of clock
                                         cycles(reference clock) in single clock
                                         period). 1b1 - AP PLL Lock has to be
                                         used as trigger1b0
                                           - Ap PLL Lock not to be used as
                                         trigger */
      __IOM unsigned int SLP_CALIB_DONE_b : 1;      /*!< [19..19] Indicates the end of
                                               calibration */
      __IOM unsigned int Reserved1 : 12;            /*!< [31..20] It is recommended to write
                                         these bits to 0.                      */
    } SLEEP_CALIB_REG_b;
  };

  union {
    __IOM unsigned int CLK_CALIB_CTRL_REG1; /*!< (@ 0x0000006C) Clock Calib Control
                                            Register1 */

    struct {
      __IOM unsigned int CC_SOFT_RST_b : 1;        /*!< [0..0] Soft Reset for clock
                                            calibrator 1b1 - reset enabled 1b0
                                                      - reset disabled. */
      __IOM unsigned int CC_START_b : 1;           /*!< [1..1] start clk calibration 1b1 - start */
      __IOM unsigned int CC_CHANGE_TEST_CLK_b : 1; /*!< [2..2] change test clk. Set
                                                   this bit to 1'b1 only when
                                                   test_clk is being changed,
                                                   else this should be 1'b0. */
      __IOM unsigned int CC_CLKIN_SEL_b : 4;       /*!< [6..3] select the clock to be calibrated 4d0
                                  - ulp_ref_clk 4d1
                                     - mems_ref_clk 4d2 - ulp_20mhz_ringosc_clk
                                  4d3 - modem_pll_clk1 4d4 - modem_pll_clk2 4d5 -
                                  intf_pll_clk 4d6 - soc_pll_clk 4d7 -
                                  i2s_pll_clk 4d8 - sleep_clk 4d9 - bus_clkby2_ap
                                */
      __IOM unsigned int Reserved1 : 25;           /*!< [31..7] It is recommended to write
                                         these bits to 0. */
    } CLK_CALIB_CTRL_REG1_b;
  };

  union {
    __IOM unsigned int CLK_CALIB_CTRL_REG2; /*!< (@ 0x00000070) Clock Calib Control
                                            Register2 */

    struct {
      __IOM unsigned int CC_NUM_REF_CLKS : 32; /*!< [31..0] number of ref_clk cycles to be
                                    considered for calibrating.        */
    } CLK_CALIB_CTRL_REG2_b;
  };

  union {
    __IOM unsigned int CLK_CALIB_STS_REG1; /*!< (@ 0x00000074) Clock Calib Status
                                           Register1 */

    struct {
      __IOM unsigned int CC_DONE_b : 1;  /*!< [0..0] indicates clock calibratioon
                                         done1'b1 => done1'b0 =>  none  */
      __IOM unsigned int CC_ERROR_b : 1; /*!< [1..1] indicates clock calibration
                                         error1'b1 => error1'b0 => none */
      __IOM unsigned int Reserved1 : 30; /*!< [31..2] It is recommended to write
                                         these bits to 0. */
    } CLK_CALIB_STS_REG1_b;
  };

  union {
    __IOM unsigned int CLK_CALIB_STS_REG2; /*!< (@ 0x00000078) Clock Calib Status
                                           Register2 */

    struct {
      __IOM unsigned int CC_NUM_TEST_CLKS : 32; /*!< [31..0] number of test clk cycles occurred
                                     for the specified number of ref_clk cycles
                                   */
    } CLK_CALIB_STS_REG2_b;
  };

  union {
    __IOM unsigned int CLK_CONFIG_REG6; /*!< (@ 0x0000007C) Clock Config Register6 */

    struct {
      __IOM unsigned int IID_KH_CLK_DIV_FAC : 3;     /*!< [2..0] Clock division factor
                                                 for iid_clk. */
      __IOM unsigned int Reserved1 : 2;              /*!< [4..3] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int PADCFG_PCLK_DIV_FAC : 4;    /*!< [8..5] Clock division factor
                                                  for pclk_pad_config_m4ss */
      __IOM unsigned int QSPI_2_CLK_SEL : 3;         /*!< [11..9] Selects one of the following clocks
                                  for ssi master 000
                                   - ULP Ref Clock(generated inside M4SS based on
                                  m4ss_ref_clk_sel from NPSS) 001 - Intf PLL
                                  Clock Clock (program bypass_intf_pll_clk if the
                                  bypass clock has to be selected) 010 - Modem
                                  PLL Clock2(Not Intended for the programmer)
                                  (program bypass_modem_pll_clk if the bypass
                                  clock has to be selected) 011 - SoC PLL Clock
                                */
      __IOM unsigned int QSPI_2_CLK_DIV_FAC : 6;     /*!< [17..12] Clock divison factor for QSPI.
                                      If qspi_clk_enable is 1b0 clock is gated.
                                      Else 1)when qspi_clk_swallow_sel is 1b1 and
                                      qspi_odd_div_sel is 1b0 output clock is a
                                      swallowed clock with the following
                                      frequency. 6h0,6h1 => clk_out = clk_in >6h1
                                      => clk_out = clk_in/ qspi_clk_div_fac
                                      2)when qspi_clk_swallow_sel is 1b0 */
      __IOM unsigned int QSPI_2_CLK_SWALLOW_SEL : 1; /*!< [18..18] Clock select for clock
                                          swallow or clock divider for QSPI 1b0
                                          => 50% divider is selected with
                                          division factor qspi_clk_div_fac 1b1 =>
                                          Swallowed clock is selected with
                                          division factor qspi_clk_div_fac Before
                                          Changing this ensure that the input
                                           clocks are gated */
      __IOM unsigned int QSPI_2_ODD_DIV_SEL : 1;     /*!< [19..19] Clock select for clock swallow
                                      or 50% even clock divider or 50% odd
                                      divider clock for QSPI 1b1 - 50% odd clock
                                      divider output is selected with division
                                      factor qspi_clk_div_fac 1b0 - 50% even
                                      clock divider output or swallowed is
                                      selected with division factor
                                      qspi_clk_div_fac based on
                                      qspi_clk_swallow_sel.      */
      __IOM unsigned int Reserved2 : 12;             /*!< [31..20] It is recommended to write
                                         these bits to 0. */
    } CLK_CONFIG_REG6_b;
  };

  union {
    __IOM unsigned int DYN_CLK_GATE_DISABLE_REG2; /*!< (@ 0x00000080) Dynamic Clock
                                                  Gate Disable Register2 */

    struct {
      __IOM unsigned int SOC_PLL_SPI_CLK_DYN_CTRL_DISABLE_b : 1; /*!< [0..0] Dynamic clock gate
                                                      disable control soc pll spi
                                                      clk  1b1
                                                        - Dynamic control of the
                                                      clock is disbaled  1b0 -
                                                      Dynamic control of the
                                                      clock is enabled */
      __IOM unsigned int Reserved1 : 2;                          /*!< [2..1] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int CT_PCLK_DYN_CTRL_DISABLE_b : 1;         /*!< [3..3] Dynamic clock gate disable
                                              control SCT pclk  1b1 - Dynamic
                                                        control of the clock is
                                              disbaled  1b0 - Dynamic control
                                                        of the clock is enabled
                                            */
      __IOM unsigned int Reserved2 : 2;                          /*!< [5..4] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int EFUSE_CLK_DYN_CTRL_DISABLE_b : 1;       /*!< [6..6] Dynamic clock gate
                                                disable control efuse clk  1b1 -
                                                Dynamic control of the clock is
                                                disbaled  1b0 - Dynamic control
                                                        of the clock is enabled
                                              */
      __IOM unsigned int EFUSE_PCLK_DYN_CTRL_DISABLE_b : 1;      /*!< [7..7] Dynamic clock gate
                                                 disable control efuse pclk  1b1
                                                 - Dynamic control of the clock
                                                 is disbaled  1b0 - Dynamic
                                                        control of the clock is
                                                 enabled */
      __IOM unsigned int Reserved3 : 24;                         /*!< [31..8] It is recommended to write
                                         these bits to 0. */
    } DYN_CLK_GATE_DISABLE_REG2_b;
  };

  union {
    __IOM unsigned int PLL_LOCK_INT_STATUS_REG; /*!< (@ 0x00000084) PLL Lock
                                                Interrupt Status Register */

    struct {
      __IOM unsigned int LCD_PLL_LOCK_OF_RISING_EDGE : 1;                /*!< [0..0] 1b0 - No Interrupt; 1b1 -
                                               Interrupt encountered. */
      __IOM unsigned int DDR_PLL_LOCK_OF_RISING_EDGE : 1;                /*!< [1..1] 1'b0 => No Interrupt;1'b1
                                               => Interrupt encountered. */
      __IOM unsigned int AP_PLL_LOCK_OF_RISING_EDGE : 1;                 /*!< [2..2] 1'b0 => No Interrupt;1'b1
                                              => Interrupt encountered. */
      __IOM unsigned int INTF_PLL_LOCK_OF_RISING_EDGE : 1;               /*!< [3..3] 1'b0 => No
                                                Interrupt;1'b1 => Interrupt
                                                encountered.          */
      __IOM unsigned int I2S_PLL_LOCK_OF_RISING_EDGE : 1;                /*!< [4..4] 1'b0 => No Interrupt;1'b1
                                               => Interrupt encountered. */
      __IOM unsigned int SOC_PLL_LOCK_OF_RISING_EDGE : 1;                /*!< [5..5] 1'b0 => No Interrupt;1'b1
                                               => Interrupt encountered. */
      __IOM unsigned int MODEM_PLL_LOCK_OF_RISING_EDGE : 1;              /*!< [6..6] 1'b0 => No
                                                 Interrupt;1'b1 => Interrupt
                                                 encountered.         */
      __IOM unsigned int PLL_LOCK_DATA_TRIGGER_INTR_OF_RISING_EDGE : 1;  /*!< [7..7] 1'b0 => No
                                                             Interrupt;1'b1 =>
                                                             Interrupt
                                                             encountered. */
      __IOM unsigned int LCD_PLL_LOCK_OF_FALLING_EDGE : 1;               /*!< [8..8] 1'b0 => No
                                                Interrupt;1'b1 => Interrupt
                                                encountered.          */
      __IOM unsigned int DDR_PLL_LOCK_OF_FALLING_EDGE : 1;               /*!< [9..9] 1'b0 => No
                                                Interrupt;1'b1 => Interrupt
                                                encountered.          */
      __IOM unsigned int AP_PLL_LOCK_OF_FALLING_EDGE : 1;                /*!< [10..10] 1'b0 => No
                                               Interrupt;1'b1 => Interrupt
                                               encountered.         */
      __IOM unsigned int INTF_PLL_LOCK_OF_FALLING_EDGE : 1;              /*!< [11..11] 1'b0 => No
                                                 Interrupt;1'b1 => Interrupt
                                                 encountered.       */
      __IOM unsigned int I2S_PLL_LOCK_OF_FALLING_EDGE : 1;               /*!< [12..12] 1'b0 => No
                                                Interrupt;1'b1 => Interrupt
                                                encountered.        */
      __IOM unsigned int SOC_PLL_LOCK_OF_FALLING_EDGE : 1;               /*!< [13..13] 1'b0 => No
                                                Interrupt;1'b1 => Interrupt
                                                encountered.        */
      __IOM unsigned int MODEM_PLL_LOCK_OF_FALLING_EDGE : 1;             /*!< [14..14] 1'b0 => No
                                                  Interrupt;1'b1 => Interrupt
                                                  encountered.      */
      __IOM unsigned int PLL_LOCK_DATA_TRIGGER_INTR_OF_FALLING_EDGE : 1; /*!< [15..15] 1'b0 =>
                                                              No Interrupt;1'b1
                                                              => Interrupt
                                                              encountered. */
      __IOM unsigned int Reserved1 : 16;                                 /*!< [31..16] It is recommended to write
                                         these bits to 0.                      */
    } PLL_LOCK_INT_STATUS_REG_b;
  };
} M4CLK_Type; /*!< Size = 136 (0x88) */

/* ===========================================================================================================================
  */
/* ================                                        TIME_PERIOD
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief In this the time periods of 32KHz RC clock, 32KHz RO clock and 32KHz
  * XTAL clock can be calibrated (TIME_PERIOD)
  */

typedef struct { /*!< (@ 0x24048200) TIME_PERIOD Structure */

  union {
    __IOM unsigned int MCU_CAL_RO_TIMEPERIOD_READ; /*!< (@ 0x00000000) RO timeperiod
                                                   read register */

    struct {
      __IM unsigned int TIMEPERIOD_RO : 25; /*!< [24..0] Calibrated RO timeperiod      */
      __IM unsigned int RESERVED1 : 7;      /*!< [31..25] reser */
    } MCU_CAL_RO_TIMEPERIOD_READ_b;
  };

  union {
    __IOM unsigned int MCU_CAL_TIMER_CLOCK_PERIOD; /*!< (@ 0x00000004) MCU calender timer clock
                                        period register                   */

    struct {
      __IOM unsigned int RTC_TIMER_CLK_PERIOD : 25;             /*!< [24..0] RTC timer clock
                                                    period programmed by SOC */
      __IM unsigned int RESERVED1 : 6;                          /*!< [30..25] reser              */
      __IM unsigned int SPI_RTC_TIMER_CLK_PERIOD_APPLIED_b : 1; /*!< [31..31] Indicated SOC
                                                      programmed rtc_timer clock
                                                      period is applied at KHz
                                                      clock domain */
    } MCU_CAL_TIMER_CLOCK_PERIOD_b;
  };

  union {
    __IOM unsigned int MCU_CAL_TEMP_PROG_REG; /*!< (@ 0x00000008) temprature program
                                              register */

    struct {
      __IOM unsigned int BYPASS_CALIB_PG : 1;          /*!< [0..0] To bypass power gating and
                                              keep all the blocks always on */
      __IM unsigned int RESERVED1 : 15;                /*!< [15..1] reser       */
      __IOM unsigned int MAX_TEMP_CHANGE : 5;          /*!< [20..16] maximum temperature change after
                                   which rc calibration must be trigger */
      __IOM unsigned int TEMP_TRIGGER_TIME_SEL : 2;    /*!< [22..21] temperature
                                                       trigger time select    */
      __IOM unsigned int PERIODIC_TEMP_CALIB_EN : 1;   /*!< [23..23] Enable periodic
                                                       checking of temperature   */
      __IOM unsigned int RTC_TIMER_PERIOD_MUX_SEL : 1; /*!< [24..24] rtc timer
                                                       period mux select */
      __IM unsigned int RESERVED2 : 7;                 /*!< [31..25] reser                 */
    } MCU_CAL_TEMP_PROG_REG_b;
  };

  union {
    __IOM unsigned int MCU_CAL_START_REG; /*!< (@ 0x0000000C) mcu cal start register */

    struct {
      __IOM unsigned int ALPHA_RO : 3;               /*!< [2..0] alpha = 1/2^alpha_ro , averaging factor of
                            RO timeperiod T = alpha(t_inst) + (1- alpha )t_prev
                          */
      __IOM unsigned int ALPHA_RC : 3;               /*!< [5..3] alpha = 1/2^alpha_rc , averaging factor of
                            RC timeperiod T = alpha(t_inst) + (1- alpha )t_prev
                          */
      __IOM unsigned int NO_OF_RO_CLKS : 4;          /*!< [9..6] 2^no_of_ro_clks no of clocks
                                            of ro clock counts for no of rc
                                            clocks in that time to measure
                                            timeperiod */
      __IOM unsigned int NO_OF_RC_CLKS : 3;          /*!< [12..10] 2^no_of_rc_clocks = no of
                                            rc clocks used in calibration */
      __IOM unsigned int RC_SETTLE_TIME : 3;         /*!< [15..13] no of clocks of RO for the RC clk to
                                  settle when enabled         */
      __IOM unsigned int RO_TRIGGER_TIME_SEL : 2;    /*!< [17..16] ro trigger time select */
      __IOM unsigned int RC_TRIGGER_TIME_SEL : 3;    /*!< [20..18] rc trigger time select */
      __IOM unsigned int PERIODIC_RO_CALIB_EN : 1;   /*!< [21..21] periodically calibrate RO
                                        timeperiod based ro trigger time sel */
      __IOM unsigned int PERIODIC_RC_CALIB_EN : 1;   /*!< [22..22] periodically calibrate RC
                                        timeperiod based rc trigger time sel */
      __OM unsigned int START_CALIB_RO : 1;          /*!< [23..23] to initiate RO calibration */
      __OM unsigned int START_CALIB_RC : 1;          /*!< [24..24] to initiate RC calibration */
      __IOM unsigned int RC_XTAL_MUX_SEL : 1;        /*!< [25..25] xtal mux select */
      __IOM unsigned int LOW_POWER_TRIGGER_SEL : 1;  /*!< [26..26] power trigger select */
      __IOM unsigned int VBATT_TRIGGER_TIME_SEL : 3; /*!< [29..27] trigger to ipmu block for
                                          checking vbatt status periodicaly */
      __IM unsigned int RESERVED1 : 2;               /*!< [31..30] reser    */
    } MCU_CAL_START_REG_b;
  };

  union {
    __IOM unsigned int MCU_CAL_REF_CLK_SETTLE_REG; /*!< (@ 0x00000010) reference
                                                   clock settle register */

    struct {
      __IOM unsigned int XTAL_SETTLE : 7;        /*!< [6..0] no of 32khz clocks for xtal
                                          40mhz clk to settle */
      __IM unsigned int RESERVED1 : 9;           /*!< [15..7] reser    */
      __IM unsigned int VALID_RC_TIMEPERIOD : 1; /*!< [16..16] Valid signal for reading RC
                                       timeperiod calibrated                */
      __IM unsigned int VALID_RO_TIMEPERIOD : 1; /*!< [17..17] Valid signal for
                                                 reading RO timeperiod */
      __IM unsigned int RESERVED2 : 14;          /*!< [31..18] reser          */
    } MCU_CAL_REF_CLK_SETTLE_REG_b;
  };

  union {
    __IOM unsigned int MCU_CAL_RC_TIMEPERIOD_READ; /*!< (@ 0x00000014) rc timeperiod
                                                   read register */

    struct {
      __IM unsigned int TIMEPERIOD_RC : 25; /*!< [24..0] Calibrated RC timeperiod      */
      __IM unsigned int RESERVED1 : 7;      /*!< [31..25] reser */
    } MCU_CAL_RC_TIMEPERIOD_READ_b;
  };

  union {
    __IOM unsigned int MCU_CAL_REF_CLK_TIEMPERIOD_REG; /*!< (@ 0x00000018) reference clock
                                            timeperiod register */

    struct {
      __IOM unsigned int TIMEPERIOD_REF_CLK : 24; /*!< [23..0] timeperiod of reference clk with
                                       each bit corresponding to granularity of
                                       2^27 = 1us */
      __IM unsigned int RESERVED1 : 8;            /*!< [31..24] reser */
    } MCU_CAL_REF_CLK_TIEMPERIOD_REG_b;
  };
} TIME_PERIOD_Type; /*!< Size = 28 (0x1c) */

/* ===========================================================================================================================
  */
/* ================                                          MCU_WDT
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief A dedicated window watch dog timer for MCU applications (MCU_WDT)
  */

typedef struct { /*!< (@ 0x24048300) MCU_WDT Structure */

  union {
    __IOM unsigned int MCU_WWD_INTERRUPT_TIMER; /*!< (@ 0x00000000) WATCHDOG
                                                interrupt timer register */

    struct {
      __IOM unsigned int WWD_INTERRUPT_TIMER : 5; /*!< [4..0] Watchdog Timer
                                                  programming values */
      __IM unsigned int RESERVED1 : 27;           /*!< [31..5] reser           */
    } MCU_WWD_INTERRUPT_TIMER_b;
  };

  union {
    __IOM unsigned int MCU_WWD_SYSTEM_RESET_TIMER; /*!< (@ 0x00000004) MCU watchdog
                                                   system reset register */

    struct {
      __IOM unsigned int WWD_SYSTEM_RESET_TIMER : 5; /*!< [4..0] Watch dog soc reset delay
                                          timer programming values */
      __IM unsigned int RESERVED1 : 27;              /*!< [31..5] reser   */
    } MCU_WWD_SYSTEM_RESET_TIMER_b;
  };

  union {
    __IOM unsigned int MCU_WWD_WINDOW_TIMER; /*!< (@ 0x00000008) watchdog window
                                             timer register */

    struct {
      __IOM unsigned int WINDOW_TIMER : 4; /*!< [3..0] watchdog window timer */
      __IM unsigned int RESERVED1 : 28;    /*!< [31..4] reser    */
    } MCU_WWD_WINDOW_TIMER_b;
  };

  union {
    __IOM unsigned int MCU_WWD_ARM_STUCK_EN; /*!< (@ 0x0000000C) watchdog arm stuck
                                             enable register */

    struct {
      __IM unsigned int RESERVED1 : 16;                /*!< [15..0] reser */
      __OM unsigned int PROCESSOR_STUCK_RESET_EN : 1;  /*!< [16..16] Enable to reset M4 core on
                                            seeing LOCKUP signal */
      __IM unsigned int RESERVED2 : 7;                 /*!< [23..17] reser      */
      __IM unsigned int PROCESSOR_STUCK_RESET_EN_ : 1; /*!< [24..24] Read signal for processor
                                             stuck reset enable */
      __IM unsigned int RESERVED3 : 7;                 /*!< [31..25] reser       */
    } MCU_WWD_ARM_STUCK_EN_b;
  };

  union {
    __IOM unsigned int MCU_WWD_MODE_AND_RSTART; /*!< (@ 0x00000010) WATCHDOG mode
                                                and restart register */

    struct {
      __IOM unsigned int WWD_MODE_RSTART : 1;    /*!< [0..0] restart pulse to restart
                                              watchdog timer */
      __IM unsigned int RESERVED1 : 15;          /*!< [15..1] reser       */
      __IOM unsigned int WWD_MODE_EN_STATUS : 8; /*!< [23..16] Watchdog timer mode  */
      __IM unsigned int RESERVED2 : 8;           /*!< [31..24] reser */
    } MCU_WWD_MODE_AND_RSTART_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int MCU_WWD_KEY_ENABLE; /*!< (@ 0x00000018) watchdog key enable
                                           register */

    struct {
      __OM unsigned int WWD_KEY_ENABLE : 32; /*!< [31..0] enable access to program
                                             Watch dog registers */
    } MCU_WWD_KEY_ENABLE_b;
  };
} MCU_WDT_Type; /*!< Size = 28 (0x1c) */

/* ===========================================================================================================================
  */
/* ================                                            RTC
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The MCU calender acts as RTC  with time in seconds, minutes, hours,
  * days, months, years and centuries (RTC)
  */

typedef struct { /*!< (@ 0x2404821C) RTC Structure */

  union {
    __IOM unsigned int MCU_CAL_ALARM_PROG_1; /*!< (@ 0x00000000) MCU calender alarm
                                             prog register 1 */

    struct {
      __IOM unsigned int PROG_ALARM_MSEC : 10; /*!< [9..0] milli seconds value of
                                               alarm time */
      __IOM unsigned int PROG_ALARM_SEC : 6;   /*!< [15..10] seconds value of alarm time */
      __IOM unsigned int PROG_ALARM_MIN : 6;   /*!< [21..16] mins value of alarm time */
      __IOM unsigned int PROG_ALARM_HOUR : 5;  /*!< [26..22] hours value of alarm time     */
      __IM unsigned int RESERVED1 : 5;         /*!< [31..27] reser */
    } MCU_CAL_ALARM_PROG_1_b;
  };

  union {
    __IOM unsigned int MCU_CAL_ALARM_PROG_2; /*!< (@ 0x00000004) MCU calender alarm
                                             prog register 2 */

    struct {
      __IOM unsigned int PROG_ALARM_DAY : 5;     /*!< [4..0] day count in alarm time 1-31      */
      __IM unsigned int RESERVED1 : 3;           /*!< [7..5] reser */
      __IOM unsigned int PROG_ALARM_MONTH : 4;   /*!< [11..8] month count in alarm time    */
      __IM unsigned int RESERVED2 : 4;           /*!< [15..12] reser */
      __IOM unsigned int PROG_ALARM_YEAR : 7;    /*!< [22..16] year count in alarm time
                                              0 - 99 */
      __IOM unsigned int PROG_ALARM_CENTURY : 2; /*!< [24..23] century count in alarm time  */
      __IM unsigned int RESERVED3 : 6;           /*!< [30..25] reser */
      __IOM unsigned int ALARM_EN : 1;           /*!< [31..31] alarm function enable for calendar */
    } MCU_CAL_ALARM_PROG_2_b;
  };

  union {
    __IOM unsigned int MCU_CAL_POWERGATE_REG; /*!< (@ 0x00000008) MCU calender
                                              powergate register */

    struct {
      __IOM unsigned int PG_EN_CALENDER : 1;               /*!< [0..0] Start calender block */
      __IOM unsigned int ENABLE_CALENDER_COMBI : 1;        /*!< [1..1] Enable calender
                                                    combitional logic block */
      __IOM unsigned int DISABLE_COMBI_DYN_PWRGATE_EN : 1; /*!< [2..2] Disable option for
                                                     dynamic combo RTC power gate      */
      __IOM unsigned int STATIC_COMBI_RTC_PG_EN : 1;       /*!< [3..3] Enable static combo
                                                     RTC power gate */
      __IM unsigned int RESERVED1 : 28;                    /*!< [31..4] RESERVED1              */
    } MCU_CAL_POWERGATE_REG_b;
  };

  union {
    __IOM unsigned int MCU_CAL_PROG_TIME_1; /*!< (@ 0x0000000C) MCU calendar prog
                                            time 1 register */

    struct {
      __IOM unsigned int PROG_MSEC : 10; /*!< [9..0] Milli seconds value to be programmed to
                              real time in calendar when pro_time_trig is 1 */
      __IOM unsigned int PROG_SEC : 6;   /*!< [15..10] seconds value to be programmed to real
                            time in calendar when pro_time_trig is 1 */
      __IOM unsigned int PROG_MIN : 6;   /*!< [21..16] minutes value to be programmed to real
                            time in calendar when pro_time_trig is 1 */
      __IOM unsigned int PROG_HOUR : 5;  /*!< [26..22] hours value to be programmed to real time
                             in calendar when pro_time_trig is 1 */
      __IM unsigned int RESERVED2 : 5;   /*!< [31..27] reser */
    } MCU_CAL_PROG_TIME_1_b;
  };

  union {
    __IOM unsigned int MCU_CAL_PROG_TIME_2; /*!< (@ 0x00000010) MCU calendar prog
                                            time 2 register */

    struct {
      __IOM unsigned int PROG_DAY : 5;      /*!< [4..0] day count value to be programmed to real
                            time in calendar when pro_time_trig is 1 */
      __IOM unsigned int PROG_WEEK_DAY : 3; /*!< [7..5] program which week day it is */
      __IOM unsigned int PROG_MONTH : 4;    /*!< [11..8] month value to be programmed to real time
                              in calendar when pro_time_trig is 1 */
      __IM unsigned int RES : 4;            /*!< [15..12] reser */
      __IOM unsigned int PROG_YEAR : 7;     /*!< [22..16] year value to be programmed to real time
                             in calendar when pro_time_trig is 1 */
      __IOM unsigned int PROG_CENTURY : 2;  /*!< [24..23] century value to be programmed to real
                                time in calendar when pro_time_trig is 1 */
      __IM unsigned int RESERVED1 : 6;      /*!< [30..25] reser      */
      __OM unsigned int PROG_TIME_TRIG : 1; /*!< [31..31] load the programmed to the
                                            real time in calendar block */
    } MCU_CAL_PROG_TIME_2_b;
  };

  union {
    __IM unsigned int MCU_CAL_READ_TIME_MSB; /*!< (@ 0x00000014) MCU calendar read
                                             time msb */

    struct {
      __IM unsigned int WEEK_DAY : 3;      /*!< [2..0] week day      */
      __IM unsigned int MONTHS_COUNT : 4;  /*!< [6..3] months count  */
      __IM unsigned int YEAR_COUNT : 7;    /*!< [13..7] years count    */
      __IM unsigned int CENTURY_COUNT : 2; /*!< [15..14] century count */
      __IM unsigned int RESERVED1 : 16;    /*!< [31..16] reser    */
    } MCU_CAL_READ_TIME_MSB_b;
  };

  union {
    __IM unsigned int MCU_CAL_READ_TIME_LSB; /*!< (@ 0x00000018) MCU calendar read
                                             time lsb */

    struct {
      __IM unsigned int MILLISECONDS_COUNT : 10; /*!< [9..0] milliseconds count */
      __IM unsigned int SECONDS_COUNT : 6;       /*!< [15..10] seconds count       */
      __IM unsigned int MINS_COUNT : 6;          /*!< [21..16] mins count          */
      __IM unsigned int HOURS_COUNT : 5;         /*!< [26..22] hours count         */
      __IM unsigned int DAYS_COUNT : 5;          /*!< [31..27] days count          */
    } MCU_CAL_READ_TIME_LSB_b;
  };

  union {
    __IM unsigned int MCU_CAL_READ_COUNT_TIMER; /*!< (@ 0x0000001C) MCU calendar
                                                read count timer */

    struct {
      __IM unsigned int READ_COUNT_TIMER : 27; /*!< [26..0] Read timer which increments by
                                       time period value to   reach to count
                                       milliseconds   */
      __IM unsigned int RESERVED1 : 5;         /*!< [31..27] reser */
    } MCU_CAL_READ_COUNT_TIMER_b;
  };

  union {
    __IM unsigned int MCU_CAL_SLEEP_CLK_COUNTERS; /*!< (@ 0x00000020) MCU calendar
                                                  sleep clock counter */

    struct {
      __IM unsigned int SLEEP_CLK_DURATION : 12;       /*!< [11..0] No of sleep clks with respect to
                                       APB clock so far from
                                            the posedge of sleep clk */
      __IM unsigned int RESERVED1 : 4;                 /*!< [15..12] reser */
      __IM unsigned int PCLK_COUNT_WRT_SLEEP_CLK : 12; /*!< [27..16] no. of APB clks in 1
                                             sleep clock duration */
      __IM unsigned int RESERVED2 : 4;                 /*!< [31..28] reser       */
    } MCU_CAL_SLEEP_CLK_COUNTERS_b;
  };

  union {
    __OM unsigned int MCU_CAL_KEY_EANBLE; /*!< (@ 0x00000024) MCU calendar key enable */

    struct {
      __OM unsigned int RTC_KEY : 32; /*!< [31..0] enable access to program Watch
                                      dog registers                      */
    } MCU_CAL_KEY_EANBLE_b;
  };
} RTC_Type; /*!< Size = 40 (0x28) */

/* ===========================================================================================================================
  */
/* ================                                          BATT_FF
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The use of this is to store some information in ULP over wake-ups to
  * reduce wake-up time (BATT_FF)
  */

typedef struct { /*!< (@ 0x24048400) BATT_FF Structure */

  union {
    __IOM unsigned int M4SS_BYPASS_PWRCTRL_REG1; /*!< (@ 0x00000000) M4ss bypass
                                                 power control register 1 */

    struct {
      __IM unsigned int RES : 3;                                    /*!< [2..0] reserved1 */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_M4_ULP_AON_b : 1;   /*!< [3..3] Enables software
                                                       based control of isolation
                                                       and reset
                                                        for ULP AON M4ss */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_EFUSE_b : 1;        /*!< [4..4] Enables software based
                                                  control of isolation and reset
                                                        for ULP EFUSE */
      __IOM unsigned int RESERVED2 : 4;                             /*!< [8..5] reserved2           */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_RPDMA_b : 1;        /*!< [9..9] Enables software based
                                                  control of isolation and reset
                                                        for RPDMA */
      __IOM unsigned int RESERVED3 : 1;                             /*!< [10..10] reserved3           */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_HIF_SDIO_SPI_b : 1; /*!< [11..11] Enables
                                                        software based control of
                                                        isolation and reset for
                                                        HIF SDIO SPI */
      __IOM unsigned int RESERVED4 : 1;                             /*!< [12..12] reserved4                  */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_QSPI_ICACHE_b : 1;  /*!< [13..13] Enables
                                                        software based control of
                                                        isolation and reset for
                                                        ULP quad SPI and ICACHE
                                                      */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_IID_b : 1;          /*!< [14..14] Enables software based
                                                control of isolation and reset
                                                        for ULP IID */
      __IOM unsigned int RESERVED5 : 2;                             /*!< [16..15] reserved5         */
      __IOM
      unsigned int BYPASS_M4SS_PWRCTL_ULP_M4_DEBUG_b : 1;      /*!< [17..17] Enables
                                                         software based control
                                                         of isolation and reset
                                                            for M4ss DEBUG */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_M4_CORE_b : 1; /*!< [18..18] Enables software
                                                    based control of isolation
                                                    and reset for M4ss CORE */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_AON_b : 1;     /*!< [19..19] Enables software based
                                                control of isolation and reset
                                                        for ULP AON */
      __IM unsigned int RESERVED6 : 2;                         /*!< [21..20] reserved6          */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_ROM_b : 1;     /*!< [22..22] Enables software based
                                                control of isolation and reset
                                                        for M4ss ROM */
      __IM unsigned int RESERVED7 : 9;                         /*!< [31..23] reserved7          */
    } M4SS_BYPASS_PWRCTRL_REG1_b;
  };

  union {
    __IOM unsigned int M4SS_BYPASS_PWRCTRL_REG2; /*!< (@ 0x00000004) M4SS bypass
                                                 power control register 2 */

    struct {
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_SRAM_1 : 10; /*!< [9..0] Enables software based
                                                  control of isolation and reset
                                                        for M4ss SRAM 1 */
      __IM unsigned int RESERVED1 : 6;                       /*!< [15..10] reserved1            */
      __IOM unsigned int BYPASS_M4SS_PWRCTL_ULP_SRAM_2 : 4;  /*!< [19..16] Enables software
                                                 based control of isolation and
                                                 reset for M4ss SRAM 2 */
      __IM unsigned int RESERVED2 : 12;                      /*!< [31..20] reserved1          */
    } M4SS_BYPASS_PWRCTRL_REG2_b;
  };

  union {
    __IOM unsigned int M4SS_PWRCTRL_SET_REG; /*!< (@ 0x00000008) M4SS power control
                                             set register */

    struct {
      __IM unsigned int RES : 4;                                   /*!< [3..0] reserved1 */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_EFUSE_b : 1;        /*!< [4..4] Power Gate control for
                                                 EFUSE */
      __IM unsigned int RESERVED2 : 4;                             /*!< [8..5] reserved2           */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_RPDMA_b : 1;        /*!< [9..9] Power Gate control for
                                                 RPDMA */
      __IM unsigned int RESERVED3 : 1;                             /*!< [10..10] reserved3           */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_HIF_SDIO_SPI_b : 1; /*!< [11..11] Power Gate
                                                        control for HIF SDIO SPI
                                                      */
      __IM unsigned int RESERVED4 : 1;                             /*!< [12..12] reserved4                  */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_QSPI_ICACHE_b : 1;  /*!< [13..13] Power Gate
                                                       control for QSPI and
                                                       ICACHE              */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_IID_b : 1;          /*!< [14..14] Power Gate control for
                                           IID Block.If set, powered ON
                                           Clearing this bit has no effect */
      __IM unsigned int RESERVED5 : 2;                             /*!< [16..15] reserved5         */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_M4_DEBUG_b : 1;     /*!< [17..17] Power Gate control
                                                    for M4 DEBUG */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_M4_CORE_b : 1;      /*!< [18..18] Power Gate control
                                                   for M4 CORE */
      __IM unsigned int RESERVED6 : 3;                             /*!< [21..19] reserved6             */
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_ROM_b : 1;      /*!< [22..22] External power gate
                                                   enable signal for ROM */
      __IM unsigned int RESERVED7 : 9;                             /*!< [31..23] reserved7             */
    } M4SS_PWRCTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int M4SS_PWRCTRL_CLEAR_REG; /*!< (@ 0x0000000C) M4SS power
                                               control clear register */

    struct {
      __IM unsigned int RES : 4;                                   /*!< [3..0] reserved1 */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_EFUSE_b : 1;        /*!< [4..4] Power Gate control for
                                                 EFUSE */
      __IM unsigned int RESERVED2 : 4;                             /*!< [8..5] reserved2           */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_RPDMA_b : 1;        /*!< [9..9] Power Gate control for
                                                 RPDMA */
      __IM unsigned int RESERVED3 : 1;                             /*!< [10..10] reserved3           */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_HIF_SDIO_SPI_b : 1; /*!< [11..11] Power Gate
                                                        control for HIF SDIO SPI
                                                      */
      __IM unsigned int RESERVED4 : 1;                             /*!< [12..12] reserved4                  */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_QSPI_ICACHE_b : 1;  /*!< [13..13] Power Gate
                                                       control for QSPI and
                                                       ICACHE              */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_IID_b : 1;          /*!< [14..14] Power Gate control for
                                           IID Block.If set, powered ON
                                           Clearing this bit has no effect */
      __IM unsigned int RESERVED5 : 2;                             /*!< [16..15] reserved5         */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_M4_DEBUG_b : 1;     /*!< [17..17] Power Gate control
                                                    for M4 DEBUG */
      __IOM unsigned int M4SS_PWRGATE_EN_N_ULP_M4_CORE_b : 1;      /*!< [18..18] Power Gate control
                                                   for M4 CORE */
      __IM unsigned int RESERVED6 : 3;                             /*!< [21..19] reserved6             */
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_ROM_b : 1;      /*!< [22..22] External power gate
                                                   enable signal for ROM */
      __IM unsigned int RESERVED7 : 9;                             /*!< [31..23] reserved7             */
    } M4SS_PWRCTRL_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_SET_REG1; /*!< (@ 0x00000010) M4SS power
                                                 control set register 1 */

    struct {
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_1 : 10; /*!< [9..0] Functional Control
                                                     signal for M4SS SRAM */
      __IM unsigned int RESERVED1 : 6;                          /*!< [15..10] reserved1               */
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_2 : 4;  /*!< [19..16] Functional Control
                                                    signal for TASS SRAM shared
                                                    with M4SS */
      __IOM unsigned int RESERVED2 : 12;                        /*!< [31..20] reserved1            */
    } M4_SRAM_PWRCTRL_SET_REG1_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_CLEAR_REG1; /*!< (@ 0x00000014) M4SS power
                                                   control clear register 1 */

    struct {
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_1 : 10; /*!< [9..0] Functional Control
                                                     signal for M4SS SRAM */
      __IOM unsigned int RESERVED1 : 6;                         /*!< [15..10] reserved1              */
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_2 : 4;  /*!< [19..16] Functional Control
                                                    signal for TASS SRAM shared
                                                    with M4SS */
      __IOM unsigned int RESERVED2 : 12;                        /*!< [31..20] reserved1            */
    } M4_SRAM_PWRCTRL_CLEAR_REG1_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_SET_REG2; /*!< (@ 0x00000018) M4SS power
                                                 control set register 2 */

    struct {
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 10; /*!< [9..0] Functional
                                                          Control signal for M4SS
                                                          SRAM Dual Rail pins */
      __IOM unsigned int RESERVED1 : 6;                              /*!< [15..10] reserved1                   */
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_2 : 4;  /*!< [19..16] Functional
                                                        Control signal for TASS
                                                        SRAM Dual Rail pins
                                                        shared with M4SS */
      __IOM unsigned int RESERVED2 : 12;                             /*!< [31..20] reserved1                 */
    } M4_SRAM_PWRCTRL_SET_REG2_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_CLEAR_REG2; /*!< (@ 0x0000001C) M4SS power
                                                   control clear register 2 */

    struct {
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 10; /*!< [9..0] Functional
                                                          Control signal for M4SS
                                                          SRAM Dual Rail pins */
      __IOM unsigned int RESERVED1 : 6;                              /*!< [15..10] reserved1                   */
      __IOM unsigned int M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_2 : 4;  /*!< [19..16] Functional
                                                        Control signal for TASS
                                                        SRAM Dual Rail pins
                                                        shared with M4SS */
      __IOM unsigned int RESERVED2 : 12;                             /*!< [31..20] reserved1                 */
    } M4_SRAM_PWRCTRL_CLEAR_REG2_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_SET_REG3; /*!< (@ 0x00000020) M4SS power
                                                 control set register 3 */

    struct {
      __IOM unsigned int M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_1 : 10; /*!< [9..0] Input
                                                            isolation control for
                                                            M4SS SRAM */
      __IOM unsigned int RESERVED1 : 6;                                /*!< [15..10] reserved1 */
      __IOM unsigned int M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_2 : 4;  /*!< [19..16] Input
                                                           isolation control for
                                                           TASS SRAM shared with
                                                           M4SS */
      __IOM unsigned int RESERVED2 : 12;                               /*!< [31..20] reserved1 */
    } M4_SRAM_PWRCTRL_SET_REG3_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_CLEAR_REG3; /*!< (@ 0x00000024) M4SS power
                                                   control clear register 3 */

    struct {
      __IOM unsigned int M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_1 : 10; /*!< [9..0] Input
                                                            isolation control for
                                                            M4SS SRAM */
      __IOM unsigned int RESERVED1 : 6;                                /*!< [15..10] reserved1 */
      __IOM unsigned int M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_2 : 4;  /*!< [19..16] Input
                                                           isolation control for
                                                           TASS SRAM shared with
                                                           M4SS */
      __IOM unsigned int RESERVED2 : 12;                               /*!< [31..20] reserved1 */
    } M4_SRAM_PWRCTRL_CLEAR_REG3_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_SET_REG4; /*!< (@ 0x00000028) M4SS power
                                                 control set register 4 */

    struct {
      __IOM unsigned int M4SS_SRAM_DS_1 : 24; /*!< [23..0] Deep-Sleep control for
                                              M4SS SRAM */
      __IOM unsigned int RESERVED1 : 8;       /*!< [31..24] reserved1       */
    } M4_SRAM_PWRCTRL_SET_REG4_b;
  };

  union {
    __IOM unsigned int M4_SRAM_PWRCTRL_CLEAR_REG4; /*!< (@ 0x0000002C) M4SS power
                                                   control clear register 4 */

    struct {
      __IOM unsigned int M4SS_SRAM_DS_1 : 24; /*!< [23..0] Deep-Sleep control for
                                              M4SS SRAM */
      __IOM unsigned int RESERVED1 : 8;       /*!< [31..24] reserved1       */
    } M4_SRAM_PWRCTRL_CLEAR_REG4_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int M4SS_TASS_CTRL_SET_REG; /*!< (@ 0x00000034) M4SS_TASS control
                                               set register */

    struct {
      __IOM unsigned int M4SS_CTRL_TASS_AON_PWRGATE_EN : 1;               /*!< [0..0] M4SS controlling Power
                                                 supply for TASS AON domain */
      __IOM unsigned int M4SS_CTRL_TASS_AON_DISABLE_ISOLATION_BYPASS : 1; /*!< [1..1] M4SS
                                                        controlling Power supply
                                                        for TASS AON domains
                                                        isolation enable in
                                                        bypass mode */
      __IOM unsigned int M4SS_CTRL_TASS_AON_PWR_DMN_RST_BYPASS : 1;       /*!< [2..2] M4SS
                                                        controlling Power supply
                                                        for TASS AON domains
                                                        reset
                                                        pin in bypass mode */
      __IOM unsigned int RESERVED1 : 29;                                  /*!< [31..3] reserved1                 */
    } M4SS_TASS_CTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int M4SS_TASS_CTRL_CLEAR_REG; /*!< (@ 0x00000038) M4SS_TASS
                                                 control CLEAR register */

    struct {
      __IOM unsigned int M4SS_CTRL_TASS_AON_PWRGATE_EN : 1;               /*!< [0..0] M4SS controlling Power
                                                 supply for TASS AON domain */
      __IOM unsigned int M4SS_CTRL_TASS_AON_DISABLE_ISOLATION_BYPASS : 1; /*!< [1..1] M4SS
                                                        controlling Power supply
                                                        for TASS AON domains
                                                        isolation enable in
                                                        bypass mode */
      __IOM unsigned int M4SS_CTRL_TASS_AON_PWR_DMN_RST_BYPASS : 1;       /*!< [2..2] M4SS
                                                        controlling Power supply
                                                        for TASS AON domains
                                                        reset
                                                        pin in bypass mode */
      __IOM unsigned int RESERVED1 : 29;                                  /*!< [31..3] reserved1                 */
    } M4SS_TASS_CTRL_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int M4_ULP_MODE_CONFIG; /*!< (@ 0x0000003C) m4 ulp mode config register */

    struct {
      __IOM unsigned int ULPMODE_ISOLATION_CTRL_ULPSS : 1;        /*!< [0..0] Isolation Control for
                                                ULP-Mode non-functional paths for
                                                        ULPSS */
      __IOM unsigned int ULPMODE_ISOLATION_CTRL_M4SS_AON : 1;     /*!< [1..1] Isolation Control for
                                                   ULP-Mode non-functional paths
                                                   for M4SS-AON */
      __IOM unsigned int ULPMODE_ISOLATION_CTRL_M4_ULP : 1;       /*!< [2..2] Isolation Control for
                                                 ULP-Mode non-functional paths
                                                 for M4ULP_AON */
      __IOM unsigned int ULPMODE_ISOLATION_CTRL_M4_CORE : 1;      /*!< [3..3] Isolation Control for
                                                  ULP-Mode non-functional paths
                                                  for M4_CORE */
      __IOM unsigned int ULPMODE_ISOLATION_CTRL_M4_DEBUG_FPU : 1; /*!< [4..4] Isolation Control
                                                       for ULP-Mode
                                                       non-functional paths for
                                                        M4_DEBUG */
      __IOM unsigned int ULPMODE_ISOLATION_CTRL_M4_ROM : 1;       /*!< [5..5] Isolation Control for
                                                 ULP-Mode non-functional paths
                                                 for ROM */
      __IOM unsigned int RES : 26;                                /*!< [31..6] reserved1               */
    } M4_ULP_MODE_CONFIG_b;
  };

  union {
    __IOM unsigned int ULPSS_BYPASS_PWRCTRL_REG; /*!< (@ 0x00000040) ULPSS bypass
                                                 power control register */

    struct {
      __IOM unsigned int RES : 2;                           /*!< [1..0] reserved1 */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_AON : 1; /*!< [2..2] Enables software based
                                                 control of output isolation for
                                                        ULPTASS AON */
      __IOM
      unsigned int BYPASS_ULPSDCSS_PWRCTRL_ULP_AON : 1;      /*!< [3..3] Enables software
                                                       based control of output
                                                       isolation for
                                                            ULPSDCSS AON */
      __IOM unsigned int RESERVED2 : 1;                      /*!< [4..4] reser                */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_MISC : 1; /*!< [5..5] Enables software based
                                                  control of output isolation for
                                                        ULP MISC */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_CAP : 1;  /*!< [6..6] Enables software based
                                                 control of output isolation for
                                                        ULP CAP */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_VAD : 1;  /*!< [7..7] Enables software based
                                                 control of output isolation for
                                                        ULP VAD */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_UART : 1; /*!< [8..8] Enables software based
                                                  control of output isolation for
                                                        ULP UART */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_SSI : 1;  /*!< [9..9] Enables software based
                                                 control of output isolation for
                                                        ULP SSI */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_I2S : 1;  /*!< [10..10] Enables software
                                                 based control of output
                                                 isolation for ULP I2S */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_I2C : 1;  /*!< [11..11] Enables software
                                                 based control of output
                                                 isolation for ULP I2C */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_AUX : 1;  /*!< [12..12] Enables software
                                            based control of output isolation
                                            for ULP AUX */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_IR : 1;   /*!< [13..13] Enables software based
                                           control of output isolation
                                           for ULP IR */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_UDMA : 1; /*!< [14..14] Enables software
                                                  based control of output
                                                  isolation for ULP UDMA */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_FIM : 1;  /*!< [15..15] Enables software
                                                 based control of output
                                                 isolation for ULP FIM */
      __IOM unsigned int RESERVED3 : 3;                      /*!< [18..16] reserved1          */
      __IOM unsigned int BYPASS_ULPTASS_PWRCTL_ULP_SRAM : 4; /*!< [22..19] Enables software
                                                  based control of output
                                                  isolation for ULPTASS SRAM */
      __IOM unsigned int RESERVED4 : 9;                      /*!< [31..23] reserved1           */
    } ULPSS_BYPASS_PWRCTRL_REG_b;
  };

  union {
    __IOM unsigned int ULPSS_PWRCTRL_SET_REG; /*!< (@ 0x00000044) ULPSS power
                                              control set register */

    struct {
      __IOM unsigned int RES : 18;                              /*!< [17..0] reserved1 */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_MISC : 1; /*!< [18..18] Functional
                                                     Control signal for ULPTASS
                                                     MISC            */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_CAP : 1;  /*!< [19..19] Functional Control
                                                    signal for ULPTASS CAP */
      __IOM unsigned int RESERVED2 : 1;                         /*!< [20..20] reserved2             */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_UART : 1; /*!< [21..21] Functional
                                                     Control signal for ULPTASS
                                                     UART            */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_SSI : 1;  /*!< [22..22] Functional Control
                                                    signal for ULPTASS SSI */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_I2S : 1;  /*!< [23..23] Functional Control
                                                    signal for ULPTASS I2S */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_I2C : 1;  /*!< [24..24] Functional Control
                                                    signal for ULPTASS I2C */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_AUX : 1;  /*!< [25..25] Functional Control
                                                    signal for ULPTASS AUX */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_IR : 1;   /*!< [26..26] Functional Control
                                                   signal for ULPTASS IR */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_UDMA : 1; /*!< [27..27] Functional
                                                     Control signal for ULPTASS
                                                     UDMA            */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_FIM : 1;  /*!< [28..28] Functional Control
                                                    signal for ULPTASS FIM */
      __IOM unsigned int RESERVED3 : 3;                         /*!< [31..29] RESERVED3             */
    } ULPSS_PWRCTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int ULPSS_PWRCTRL_CLEAR_REG; /*!< (@ 0x00000048) ULPSS power
                                                control clear register */

    struct {
      __IOM unsigned int RESERVED1 : 18;                        /*!< [17..0] reserved1 */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_MISC : 1; /*!< [18..18] Functional
                                                     Control signal for ULPTASS
                                                     MISC            */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_CAP : 1;  /*!< [19..19] Functional Control
                                                    signal for ULPTASS CAP */
      __IOM unsigned int RESERVED2 : 1;                         /*!< [20..20] reserved2             */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_UART : 1; /*!< [21..21] Functional
                                                     Control signal for ULPTASS
                                                     UART            */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_SSI : 1;  /*!< [22..22] Functional Control
                                                    signal for ULPTASS SSI */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_I2S : 1;  /*!< [23..23] Functional Control
                                                    signal for ULPTASS I2S */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_I2C : 1;  /*!< [24..24] Functional Control
                                                    signal for ULPTASS I2C */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_AUX : 1;  /*!< [25..25] Functional Control
                                                    signal for ULPTASS AUX */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_IR : 1;   /*!< [26..26] Functional Control
                                                   signal for ULPTASS IR */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_UDMA : 1; /*!< [27..27] Functional
                                                     Control signal for ULPTASS
                                                     UDMA            */
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_FIM : 1;  /*!< [28..28] Functional Control
                                                    signal for ULPTASS FIM */
      __IOM unsigned int RESERVED3 : 3;                         /*!< [31..29] RESERVED3             */
    } ULPSS_PWRCTRL_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int ULPSS_RAM_PWRCTRL_SET_REG1; /*!< (@ 0x0000004C) ULPSS ram power control
                                        set register1                      */

    struct {
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM : 4; /*!< [3..0] Functional Control
                                                     signal for ULPSS SRAM pins
                                                   */
      __IOM unsigned int RESERVED1 : 28;                        /*!< [31..4] reserved1             */
    } ULPSS_RAM_PWRCTRL_SET_REG1_b;
  };

  union {
    __IOM unsigned int ULPSS_RAM_PWRCTRL_CLEAR_REG1; /*!< (@ 0x00000050) ULPSS ram power
                                          control clear register1 */

    struct {
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM : 4; /*!< [3..0] Functional Control
                                                     signal for ULPSS SRAM pins
                                                   */
      __IOM unsigned int RESERVED1 : 28;                        /*!< [31..4] reserved1             */
    } ULPSS_RAM_PWRCTRL_CLEAR_REG1_b;
  };

  union {
    __IOM unsigned int ULPSS_RAM_PWRCTRL_SET_REG2; /*!< (@ 0x00000054) ULPSS ram power control
                                        set register2                      */

    struct {
      __IOM unsigned int ULPTASS_SRAM_INPUT_DISABLE_ISOLATION_ULP : 4; /*!< [3..0] Input
                                                            isolation control for
                                                            ULPTASS SRAM */
      __IOM unsigned int RESERVED1 : 12;                               /*!< [15..4] reserved1         */
      __IOM unsigned int SRAM_DS_ULP_PROC_1 : 4;                       /*!< [19..16] Deep-Sleep control
                                                 for ULPTASS SRAM */
      __IOM unsigned int RESERVED2 : 12;                               /*!< [31..20] reserved1         */
    } ULPSS_RAM_PWRCTRL_SET_REG2_b;
  };

  union {
    __IOM unsigned int ULPSS_RAM_PWRCTRL_CLEAR_REG2; /*!< (@ 0x00000058) ULPSS ram power
                                          control clear register2 */

    struct {
      __IOM unsigned int ULPTASS_SRAM_INPUT_DISABLE_ISOLATION_ULP : 4; /*!< [3..0] Input
                                                            isolation control for
                                                            ULPTASS SRAM */
      __IOM unsigned int RESERVED1 : 12;                               /*!< [15..4] reserved1         */
      __IOM unsigned int SRAM_DS_ULP_PROC_1 : 4;                       /*!< [19..16] Deep-Sleep control
                                                 for ULPTASS SRAM */
      __IOM unsigned int RESERVED2 : 12;                               /*!< [31..20] reserved1         */
    } ULPSS_RAM_PWRCTRL_CLEAR_REG2_b;
  };

  union {
    __IOM unsigned int ULPSS_RAM_PWRCTRL_SET_REG3; /*!< (@ 0x0000005C) ULPSS ram power control
                                        set register3                      */

    struct {
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 4; /*!< [3..0] Functional
                                                        Control signal for
                                                        ULPTASS SRAM Dual Rail
                                                        pins */
      __IOM unsigned int RES : 28;                                     /*!< [31..4] reserved1 */
    } ULPSS_RAM_PWRCTRL_SET_REG3_b;
  };

  union {
    __IOM unsigned int ULPSS_RAM_PWRCTRL_CLEAR_REG3; /*!< (@ 0x00000060) ULPSS ram power
                                          control clear register3 */

    struct {
      __IOM unsigned int ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 4; /*!< [3..0] Functional
                                                        Control signal for
                                                        ULPTASS SRAM Dual Rail
                                                        pins */
      __IOM unsigned int RES : 28;                                     /*!< [31..4] reserved1 */
    } ULPSS_RAM_PWRCTRL_CLEAR_REG3_b;
  };

  union {
    __IOM unsigned int MCU_FSM_CTRL_BYPASS; /*!< (@ 0x00000064) MCU FSM control
                                            bypass register */

    struct {
      __IOM unsigned int MCU_XTAL_EN_40MHZ_BYPASS_CTRL : 1;     /*!< [0..0] Xtal 40mhz enable
                                                       bypass from MCU       */
      __IOM unsigned int MCU_XTAL_EN_40MHZ_BYPASS : 1;          /*!< [1..1] Value of Xtal
                                                       Enable in bypass mode */
      __IOM unsigned int MCU_PMU_SHUT_DOWN_BYPASS_CTRL : 1;     /*!< [2..2] Enable bypass mode to
                                                 Control pmu shut down */
      __IOM unsigned int MCU_PMU_SHUT_DOWN_BYPASS : 1;          /*!< [3..3] Value of pmu shutdown in
                                            bypass mode */
      __IOM unsigned int MCU_BUCK_BOOST_ENABLE_BYPASS_CTRL : 1; /*!< [4..4] Enable software
                                                     control for buck boost
                                                     enable           */
      __IOM unsigned int MCU_BUCK_BOOST_ENABLE_BYPASS : 1;      /*!< [5..5] Value for MCU BuckBoost
                                                Enable in bypass mode */
      __IOM unsigned int RES : 26;                              /*!< [31..6] reserved1              */
    } MCU_FSM_CTRL_BYPASS_b;
  };

  union {
    __IOM unsigned int MCU_PMU_LDO_CTRL_SET; /*!< (@ 0x00000068) MCU PMU LD0 control
                                             set register */

    struct {
      __IOM unsigned int MCU_FLASH_LDO_EN : 1; /*!< [0..0] Enable Flash LDO from M4SS */
      __IOM unsigned int MCU_SCO_LDO_EN : 1;   /*!< [1..1] Enable SoC LDO from M4SS         */
      __IOM unsigned int MCU_DCDC_EN : 1;      /*!< [2..2] Enable PMU DCDC from M4SS */
      __IOM unsigned int RESER : 14;           /*!< [16..3] reserved1      */
      __IOM unsigned int MCU_SOC_LDO_LVL : 1;  /*!< [17..17] PMU SOC LDO High and Low
                                              Voltage selection */
      __IOM unsigned int MCU_DCDC_LVL : 1;     /*!< [18..18] PMU DCDC High and Low
                                              Voltage selection    */
      __IOM unsigned int RES : 13;             /*!< [31..19] reserved1            */
    } MCU_PMU_LDO_CTRL_SET_b;
  };

  union {
    __IOM unsigned int MCU_PMU_LDO_CTRL_CLEAR; /*!< (@ 0x0000006C) MCU PMU LD0
                                               control clear register */

    struct {
      __IOM unsigned int MCU_FLASH_LDO_EN : 1; /*!< [0..0] Enable Flash LDO from M4SS */
      __IOM unsigned int MCU_SOC_LDO_EN : 1;   /*!< [1..1] Enable SoC LDO from M4SS         */
      __IOM unsigned int MCU_DCDC_EN : 1;      /*!< [2..2] Enable PMU DCDC from M4SS */
      __IOM unsigned int RESER : 14;           /*!< [16..3] reserved1      */
      __IOM unsigned int MCU_SOC_LDO_LVL : 1;  /*!< [17..17] PMU SOC LDO High and Low
                                              Voltage selection */
      __IOM unsigned int MCU_DCDC_LVL : 1;     /*!< [18..18] PMU DCDC High and Low
                                              Voltage selection    */
      __IOM unsigned int RES : 13;             /*!< [31..19] reserved1            */
    } MCU_PMU_LDO_CTRL_CLEAR_b;
  };
  __IM unsigned int RESERVED1[4];

  union {
    __IOM unsigned int PLLCCI_PWRCTRL_REG; /*!< (@ 0x00000080) PLL CCI power control
                                           register */

    struct {
      __IOM unsigned int I2SPLL_ISO_EN : 1;          /*!< [0..0] Enables software based control of
                                 isolation and reset for I2SPLL */
      __IOM unsigned int I2SPLL_BYPASS_ISO_GEN : 1;  /*!< [1..1] Isolation value */
      __IOM unsigned int INTFPLL_ISO_EN : 1;         /*!< [2..2] Enables software based control of
                                  isolation and reset for INTF PLL */
      __IOM unsigned int INTFPLL_BYPASS_ISO_GEN : 1; /*!< [3..3] Isolation value */
      __IOM unsigned int SOCPLL_ISO_ENABLE : 1;      /*!< [4..4] Enables software based control of
                                     isolation and reset for SOCPLL */
      __IOM unsigned int SOCPLL_BYPASS_ISO_GEN : 1;  /*!< [5..5] Isolation value */
      __IOM unsigned int SOCPLL_SPI_PG_EN : 1;       /*!< [6..6] SOCPLL SPI Power Control */
      __IOM unsigned int SOCPLL_VDD13_ISO_EN : 1;    /*!< [7..7] SOCPLL MACRO POC Control */
      __IOM unsigned int RES : 24;                   /*!< [31..8] reserved1     */
    } PLLCCI_PWRCTRL_REG_b;
  };

  union {
    __IOM unsigned int DLL_PWRCTRL_REG; /*!< (@ 0x00000084) DLL power control register */

    struct {
      __IOM unsigned int QSPI_DLL_RX_ISO_ENABLE : 1;     /*!< [0..0] Enables software based control
                                          of isolation and reset
                                           for QSPI_DLL_TX */
      __IOM unsigned int QSPI_DLL_RX_BYPASS_ISO_GEN : 1; /*!< [1..1] Isolation value */
      __IOM unsigned int QSPI_DLL_RX_PG_EN_N : 1;        /*!< [2..2] QPSI DLL RX Power Control */
      __IOM unsigned int RESER : 1;                      /*!< [3..3] reserved1    */
      __IOM unsigned int QSPI_DLL_TX_ISO_ENABLE : 1;     /*!< [4..4] Enables software based control
                                          of isolation and reset
                                           for QSPI_DLL_TX */
      __IOM unsigned int QSPI_DLL_TX_BYPASS_ISO_GEN : 1; /*!< [5..5] Isolation value */
      __IOM unsigned int QSPI_DLL_TX_PG_EN_N : 1;        /*!< [6..6] QPSI DLL TX Power Control   */
      __IOM unsigned int RESERVED1 : 25;                 /*!< [31..7] reserved1 */
    } DLL_PWRCTRL_REG_b;
  };
} BATT_FF_Type; /*!< Size = 136 (0x88) */

/* ===========================================================================================================================
  */
/* ================                                          MCU_FSM
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief This is explain the Sleep FSM registers. (MCU_FSM)
  */

typedef struct { /*!< (@ 0x24048100) MCU_FSM Structure */

  union {
    __IOM unsigned int MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE; /*!< (@ 0x00000000) Sleep Control
                                                 Signals and Wakeup source
                                                 selection   */

    struct {
      __IOM unsigned int MCUFSM_SHUTDOWN_ENABLE : 1;         /*!< [0..0] shutdown enable pulse. */
      __IOM unsigned int Reserved1 : 1;                      /*!< [1..1] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int LP_SLEEP_MODE_b : 1;                /*!< [2..2] setting this bit enables retention of
                                   TASS-RAM, M4SS-RAM in PS2 Active/Sleep state
                                 */
      __IOM unsigned int M4SS_RAM_RETENTION_MODE_EN : 1;     /*!< [3..3] shutdown enable
                                                         pulse. */
      __IOM unsigned int M4ULP_RAM_RETENTION_MODE_EN_b : 1;  /*!< [4..4] RAM retention enable
                                                 for ULP M4 ram during deep sleep
                                               */
      __IOM unsigned int TA_RAM_RETENTION_MODE_EN : 1;       /*!< [5..5] RAM retention enable for NWP
                                            ram during deep sleep */
      __IOM unsigned int ULPSS_RAM_RETENTION_MODE_EN : 1;    /*!< [6..6] RAM retention enable for
                                               ulpss ram during deep sleep */
      __IOM unsigned int M4ULP_RAM16K_RETENTION_MODE_EN : 1; /*!< [7..7] To enable retention
                                                  mode for m4ulp 16k RAM */
      __IOM unsigned int LDO_SOC_ON_b : 1;                   /*!< [8..8] ON ldo soc during deep sleep */
      __IOM unsigned int LDO_FLASH_ON_b : 1;                 /*!< [9..9] ON flash ldo during deep sleep */
      __IOM unsigned int PMU_DCDC_ON_b : 1;                  /*!< [10..10] 1: PMU DCDC(BUCK) ON,0:
                                            PMU DCDC(BUCK) OFF. */
      __IOM unsigned int SKIP_XTAL_WAIT_TIME : 1;            /*!< [11..11] 1 : Skips Xtal Good
                                                  Delay wait time. */
      __IOM unsigned int Reserved2 : 2;                      /*!< [13..12] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int MCUFSM_WAKEUP_NWPFSM : 1;           /*!< [14..14] When Set, mcufsm wakeup enable
                                        will wakeup both NWP FSM and MCU
                                        FSM.Clear this BIT if this feature is not
                                        required..          */
      __IOM unsigned int SLEEP_WAKEUP : 1;                   /*!< [15..15] Wakeup indication from Processor */
      __IOM unsigned int TIMER_BASED_WAKEUP_b : 1;           /*!< [16..16] wakeup enable after deep sleep
                                        counter elapses                   */
      __IOM unsigned int HOST_BASED_WAKEUP_b : 1;            /*!< [17..17] host based wakeup enable */
      __IOM unsigned int WIRELESS_BASED_WAKEUP_b : 1;        /*!< [18..18] wireless based
                                                      wakeup enable */
      __IOM unsigned int M4_PROC_BASED_WAKEUP_b : 1;         /*!< [19..19] wakeup based on
                                                      m4 processor enable  */
      __IOM unsigned int GPIO_BASED_WAKEUP_b : 1;            /*!< [20..20] wakeup on gpio interrupt enable
                                       based in configuration in GPIO WAKEUP
                                       REGISTER */
      __IOM unsigned int COMPR_BASED_WAKEUP_b : 1;           /*!< [21..21] compartor based
                                                   wakeup enable, either of any 6
                                                   comparator interrupts */
#if defined(SLI_SI917B0)
      __IOM unsigned int SYSRTC_BASED_WAKEUP_b : 1; /*!< [22..22] SYSRTC Based Wakeup */
#else
      __IOM unsigned int Reserved3 : 1;                /*!< [22..22] It is recommended to write
                                        these bits to 0.                      */
#endif
      __IOM unsigned int WIC_BASED_WAKEUP_b : 1;      /*!< [23..23] WIC based wakeup mask */
      __IOM unsigned int ULPSS_BASED_WAKEUP_b : 1;    /*!< [24..24] ULPSS peripheral
                                                   based wakeup */
      __IOM unsigned int SDCSS_BASED_WAKEUP_b : 1;    /*!< [25..25] Sensor Data
                                                   collector based wakeup */
      __IOM unsigned int ALARM_BASED_WAKEUP_b : 1;    /*!< [26..26] Alarm Based wakeup */
      __IOM unsigned int SEC_BASED_WAKEUP_b : 1;      /*!< [27..27] Second Pulse Based wakeup */
      __IOM unsigned int MSEC_BASED_WAKEUP_b : 1;     /*!< [28..28] Millisecond Pulse
                                                  Based wakeup */
      __IOM unsigned int WDT_INTR_BASED_WAKEUP_b : 1; /*!< [29..29] Millisecond
                                                      Pulse Based wakeup */
      __IOM unsigned int ULPSS_BASED_SLEEP : 1;       /*!< [30..30] ULPSS initiated DeepSleep. */
      __IOM unsigned int SDCSS_BASED_SLEEP : 1;       /*!< [31..31] SDCSS initiated DeepSleep. */
    } MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b;
  };

  union {
    __IOM unsigned int MCU_FSM_PERI_CONFIG_REG; /*!< (@ 0x00000004) Configuration
                                                for Ultra Low-Power Mode of the
                                                                     processor
                                                (PS2 State) */

    struct {
      __IOM unsigned int ULP_MCU_MODE_EN : 1;                  /*!< [0..0] Enables voltage switching
                                              for PS2-PS4/PS3 and PS4/PS3-PS2
                                                      state transitions. */
      __IOM unsigned int M4SS_CONTEXT_SWITCH_TOP_ULP_MODE : 2; /*!< [2..1] Enable functional
                                                    switching for PS2-PS4/PS3 and
                                                    PS4/PS3-PS2
                                                        state transitions */
      __IOM unsigned int WICENREQ : 1;                         /*!< [3..3] Its enable or disable maximum of 32KB of
                            LP-SRAM for operation in PS2 state */
      __IOM unsigned int Reserved1 : 12;                       /*!< [15..4] It is recommended to write
                                         these bits to 0. */
      __IOM unsigned int BGPMU_SAMPLING_EN_R : 1;              /*!< [16..16] Controls the mode of Band-Gap
                                         for DC-DC 1.35 during   PS2 state.   */
      __IOM unsigned int Reserved2 : 15;                       /*!< [31..17] It is recommended to write
                                         these bits to 0.                      */
    } MCU_FSM_PERI_CONFIG_REG_b;
  };

  union {
    __IOM unsigned int GPIO_WAKEUP_REGISTER; /*!< (@ 0x00000008) GPIO Wakeup Register */

    struct {
      __IOM unsigned int GPIO_0_WAKEUP : 1;           /*!< [0..0] Enable gpio 0 based wakeup. */
      __IOM unsigned int GPIO_1_WAKEUP : 1;           /*!< [1..1] Enable gpio 1 based wakeup */
      __IOM unsigned int GPIO_2_WAKEUP : 1;           /*!< [2..2] Enable gpio 2 based wakeup */
      __IOM unsigned int GPIO_3_WAKEUP : 1;           /*!< [3..3] Enable gpio 3 based wakeup */
      __IOM unsigned int GPIO_4_WAKEUP : 1;           /*!< [4..4] Enable gpio 3 based wakeup         */
      __IOM unsigned int Reserved1 : 11;              /*!< [15..5] It is recommended to write
                                         these bits to 0. */
      __IOM unsigned int CONTINIOUS_START : 1;        /*!< [16..16] Trigger Deep sleep
                                               timer to start counting. */
      __IOM unsigned int CONTINIOUS_TIMER_ENABLE : 1; /*!< [17..17] Enable Deep sleep timer
                                           mode continuous. */
      __IOM unsigned int DS_TIMER_SOFT_RESET : 1;     /*!< [18..18] Enable Deep sleep
                                                  timer mode continuous. */
      __IOM unsigned int Reserved2 : 13;              /*!< [31..19] It is recommended to write
                                         these bits to 0.                      */
    } GPIO_WAKEUP_REGISTER_b;
  };

  union {
    __IOM unsigned int MCU_FSM_DEEP_SLEEP_DURATION_LSB_REG; /*!< (@ 0x0000000C) MCU FSM DEEP
                                                 SLEEP DURATION LSB Register */

    struct {
      __IOM
      unsigned int MCUFSM_DEEPSLEEP_DURATION_COUNT : 32; /*!< [31..0] LSB bits of
                                                        deep sleep duration
                                                        counter after which
                                                            system wakes up is
                                                        timeout wakeup is
                                                        enabled. */
    } MCU_FSM_DEEP_SLEEP_DURATION_LSB_REG_b;
  };

  union {
    __IOM unsigned int MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG; /*!< (@ 0x00000010) MCU FSM XTAL
                                                 AND PMU GOOD COUNT Register */

    struct {
      __IOM unsigned int MCUFSM_PMU_POWERGOOD_DURATION_COUNT : 7; /*!< [6..0] Wait Delay for
                                                       PMU POWER GOOD 0 - 5us 1 -
                                                       10us 2 - 12.5us
                                                        3 - 25us 4 - 50us 5 -
                                                       75us. */
      __IOM unsigned int Reserved1 : 9;                           /*!< [15..7] It is recommended to write
                                         these bits to 0. */
      __IOM unsigned int MCUFSM_XTAL_GOODTIME_DURATION_COUNT : 7; /*!< [22..16] Wait Delay for
                                                       XTAL GOOD Time 0 - 5us 1 -
                                                       10us.     */
      __IOM unsigned int Reserved2 : 9;                           /*!< [31..23] It is recommended to write
                                         these bits to 0.                      */
    } MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG_b;
  };

  union {
    __IOM unsigned int MCU_FSM_POWER_CTRL_AND_DELAY; /*!< (@ 0x00000014) Power Control and
                                          Delay Configuration for Ultra
                                                          Low-Power Mode of the
                                          processor (PS2 State)                */

    struct {
      __IOM unsigned int PS2_PMU_LDO_OFF_DELAY : 5;        /*!< [4..0] PMU BUCK And LDO
                                                    Turn-OFF Delay. */
      __IOM unsigned int Reserved1 : 3;                    /*!< [7..5] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int PS4_SOCLDO_ON_DELAY : 4;          /*!< [11..8] PMU SOCLDO Turn-ON Delay. */
      __IOM unsigned int PG4_BUCK_ON_DELAY : 4;            /*!< [15..12] PMU Buck Turn-ON Delay. */
      __IOM unsigned int FSM_PERI_SOC_LDO_EN : 1;          /*!< [16..16] Enable SOCLDO in
                                                  Peri mode. */
      __IOM unsigned int FSM_PERI_DCDC_EN : 1;             /*!< [17..17] Enable DCDC in Peri mode.     */
      __IOM unsigned int Reserved2 : 1;                    /*!< [18..18] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int POWER_MUX_SEL_ULPSS : 1;          /*!< [19..19] Select value for
                                                   ULPSS(Peripherals) Power Mux             */
      __IOM unsigned int POWER_MUX_SEL_M4_ULP : 2;         /*!< [21..20] Select value for M4
                                                   ULP (Peripherals + Cortex Core
                                                      )Power Mux. */
      __IOM unsigned int POWER_MUX_SEL_M4_ULP_RAM_16K : 2; /*!< [23..22] Select value for M4
                                                ULP RAM 16K Power Mux */
      __IOM unsigned int POWER_MUX_SEL_M4_ULP_RAM : 2;     /*!< [25..24] Select value for M4 ULP
                                            RAM Power Mux. */
      __IOM unsigned int POWER_MUX_SEL_ULPSS_RAM : 2;      /*!< [27..26] Select value for
                                                      ULPSS RAM Power Mux. */
      __IOM unsigned int Reserved3 : 4;                    /*!< [31..28] It is recommended to write
                                        these bits to 0.                      */
    } MCU_FSM_POWER_CTRL_AND_DELAY_b;
  };

  union {
    __IOM unsigned int MCU_FSM_CLKS_REG; /*!< (@ 0x00000018) MCU FSM Clocks Register */

    struct {
      __IOM unsigned int Reserved1 : 2;                /*!< [1..0] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int HF_FSM_CLK_SELECT : 3;        /*!< [4..2] Disable signal for m4ss
                                                reference clock. */
      __IOM unsigned int Reserved2 : 10;               /*!< [14..5] It is recommended to write
                                         these bits to 0. */
      __IOM unsigned int HF_FSM_CLK_SWITCHED_SYNC : 1; /*!< [15..15] If high freq fsm clock
                                              select is modified.   */
      __IOM unsigned int HF_FSM_CLK_FREQ : 6;          /*!< [21..16] High Frequency Source
                                              Clock value in MHz. */
      __IOM unsigned int US_DIV_COUNT : 2;             /*!< [23..22] One Micro second division factor.
                                Program value to
                                           3. If hf_fsm_gen_2mhz is 0 Program
                                value to 1. If hf_fsm_gen_2mhz is 1. */
      __IOM unsigned int HF_FSM_GEN_2MHZ : 1;          /*!< [24..24] Enable 2Mhz clock for FSM 1 -Enable
                                   2Mhz option 0- Enable 4MHz option. */
      __IOM unsigned int HF_FSM_CLK_EN : 1;            /*!< [25..25] high frequency mcu fsm
                                            clock enable. */
      __IOM unsigned int Reserved3 : 6;                /*!< [31..26] It is recommended to write
                                            these bits to 0.                      */
    } MCU_FSM_CLKS_REG_b;
  };

  union {
    __IOM unsigned int MCU_FSM_REF_CLK_REG; /*!< (@ 0x0000001C) MCU FSM Clocks Register */

    struct {
      __IOM unsigned int M4SS_REF_CLK_SEL : 3;            /*!< [2..0] Dynamic Reference Clock Mux select
                                    of M4SS 0 - Clock will be gated at dynamic
                                    mux output of M4SS 1 - ulp_mhz_rc_byp_clk
                                           2 - ulp_mhz_rc_clk 3 - rf_ref_clk 4
                                    - mems_ref_clk 5
                                           - ulp_20mhz_ringosc_clk 6 -
                                    ulp_doubler_clk 7 - ref_byp_clk to TASS. */
      __IOM unsigned int Reserved1 : 4;                   /*!< [6..3] It is recommended to write these
                                        bits to 0.                        */
      __IOM unsigned int M4SS_REF_CLK_CLEANER_OFF_b : 1;  /*!< [7..7] Disable signal for m4ss
                                              reference clock. */
      __IOM unsigned int M4SS_REF_CLK_CLEANER_ON_b : 1;   /*!< [8..8] Enable clk cleaner for m4ss
                                             reference clock. */
      __IOM unsigned int Reserved2 : 3;                   /*!< [11..9] It is recommended to write
                                             these bits to 0.                       */
      __IOM unsigned int TASS_REF_CLK_SEL : 3;            /*!< [14..12] Dynamic Reference Clock Mux select
                                    of TASS controlled by M4. 0 : Clock will be
                                    gated at dynamic mux output of TASS 1 :
                                    ulp_mhz_rc_byp_clk 2 : ulp_mhz_rc_clk 3 :
                                           rf_ref_clk 4 : mems_ref_clk 5 :
                                    ulp_20mhz_ringosc_clk 6 : ref_byp_clk to
                                    TASS. */
      __IOM unsigned int Reserved3 : 1;                   /*!< [15..15] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int ULPSS_REF_CLK_SEL_b : 3;         /*!< [18..16] Dynamic Reference Clock Mux
                                        select of TASS controlled  by M4. 0 : Clock
                                        will be gated at dynamic mux output of
                                            TASS 1 : ulp_mhz_rc_byp_clk 2 :
                                        ulp_mhz_rc_clk 3 :  rf_ref_clk 4 :
                                        mems_ref_clk 5 : ulp_20mhz_ringosc_clk 6
                                            : ref_byp_clk to TASS.  */
      __IOM unsigned int Reserved4 : 4;                   /*!< [22..19] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int ULPSS_REF_CLK_CLEANER_OFF_b : 1; /*!< [23..23] Clock cleaner Off
                                               signal for ulpss ref clock. */
      __IOM unsigned int ULPSS_REF_CLK_CLEANER_ON_b : 1;  /*!< [24..24] Clock cleaner Off signal
                                              for ulpss ref clock. */
      __IOM unsigned int Reserved5 : 3;                   /*!< [27..25] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int SDCSS_CLK_SEL_b : 2;             /*!< [29..28] select between RC / RO
                                              32KHz clk in sdcss 01 - MHz RC
                                              Clock 10- 20MHz RO Clock. */
      __IOM unsigned int SDCSS_CLK_EN_b : 1;              /*!< [30..30] To enable dynamic clock
                                              for sdcss  */
      __IOM unsigned int SDCSS_STATIC_CLK_EN_b : 1;       /*!< [31..31] To enable static clk for
                                         sensor data collector subsystem */
    } MCU_FSM_REF_CLK_REG_b;
  };

  union {
    __IOM unsigned int MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP; /*!< (@ 0x00000020) MCU FSM
                                                         And First Bootup */

    struct {
      __IOM unsigned int FIRST_BOOTUP_MCU_N_b : 1;       /*!< [0..0] Indication for S/W to
                                        distinguish b/w First Power or ULP
                                        wakeup.S/W need to set this Bit after
                                        first power ..                  */
      __IM unsigned int RAM_RETENTION_STATUS_M4SS_b : 1; /*!< [1..1] Indicates to S/W that
                                               RAM's were in retention mode
                                               during Sleep time. 1 - RAM are in
                                               retention mode during sleep. 0 -
                                               RAM are not in retention mode
                                               during sleep.Domain is OFF.. */
      __IOM unsigned int RETENTION_DOMAIN_ON_b : 1;      /*!< [2..2] Indicates to S/W that Retention
                                         domain is ON. 1 - Domain is ON. 0 -
                                         Domain is OFF.. */
      __IOM unsigned int CHIP_MODE_VALID_b : 1;          /*!< [3..3] Indicates to S/W that ChipMode
                                     programming are valid and need not read
                                     EFUSE. 1 - ChipMode are Valid. 0 - ChipModes
                                       are invalid. */
      __IOM unsigned int STORAGE_DOMAIN_ON_b : 1;        /*!< [4..4] Indicates to S/W that
                                                  MCU Data Storage 1 domain is
                                                  ON. 1 - Domain is ON. 0 -
                                                  Domain is OFF.. */
#if !defined(SLI_SI917B0)
      __IOM unsigned int Reserved1 : 10; /*!< [14..5] It is recommended to write
                                         these bits to 0. */
#else
      __IOM unsigned int Reserved1 : 9;                /*!< [13..5] It is recommended to write
                                        these bits to 0.                       */
      __IOM unsigned int MCU_ULP_1KHZ_RC_CLK_EN_b : 1; /*!< [14..14] Enables ULP 1KHz Rc Clock
                                            (For SYSRTC and MCU WWD). */
#endif
      __IOM unsigned int MCU_FSM_RESET_N_SYNC_b : 1;          /*!< [15..15] Indicated MCU FSM is out of
                                          reset. 1 : Indicated MCU
                                           FSM is out of reset 0 : Indicated MCU
                                          FSM is in reset.                    */
      __IOM unsigned int MCU_ULP_32KHZ_RC_CLK_EN_b : 1;       /*!< [16..16] Enables ULP
                                                          32KHz Rc Clock.   */
      __IOM unsigned int MCU_ULP_32KHZ_RO_CLK_EN_b : 1;       /*!< [17..17] Enables ULP
                                                          32KHz RO Clock.   */
      __IOM unsigned int MCU_ULP_32KHZ_XTAL_CLK_EN_b : 1;     /*!< [18..18] Enables ULP
                                                          32KHz Xtal Clock. */
      __IOM unsigned int MCU_ULP_MHZ_RC_CLK_EN_b : 1;         /*!< [19..19] Enables ULP
                                                          MHz RC Clock.   */
      __IOM unsigned int MCU_ULP_20MHZ_RING_OSC_CLK_EN_b : 1; /*!< [20..20] Enables ULP 20mhz
                                                   RO Clock. */
      __IOM unsigned int MCU_ULP_DOUBLER_CLK_EN_b : 1;        /*!< [21..21] Enables ULP
                                                       Doubler Clock. */
      __IOM unsigned int MCU_ULP_40MHZ_CLK_EN_b : 1;          /*!< [22..22] Enables 40MHz
                                                       XTAL clock.   */
      __IOM unsigned int Reserved2 : 9;                       /*!< [31..23] It is recommended to write
                                        these bits to 0.                      */
    } MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b;
  };

  union {
    __IOM unsigned int MCU_FSM_CRTL_PDM_AND_ENABLES; /*!< (@ 0x00000024) Power Domains
                                          Controlled by Sleep FSM. */

    struct {
      __IOM unsigned int ENABLE_WDT_IN_SLEEP_b : 1;          /*!< [0..0] Its enable or disable WDT
                                         during Sleep/Shutdown states. */
      __IOM unsigned int ENABLE_WURX_DETECTION_b : 1;        /*!< [1..1] Its enable or disable
                                           detection of On-Air Pattern using
                                           Wake-Fi Rx. */
      __IOM unsigned int RESET_MCU_BBF_DM_EN_b : 1;          /*!< [2..2] Its enable or disable reset of
                                         Power Domain Control Battery FF's on
                                         wakeup. */
      __IOM unsigned int DISABLE_TURNOFF_SRAM_PERI_b : 1;    /*!< [3..3] Enable MCU SRAM periphery
                                           during Deepsleep 1 - Enable SRAM
                                           periphery during Deepsleep 0 - Disable
                                           SRAM periphery during Deepsleep. */
      __IOM unsigned int ENABLE_SRAM_DS_CRTL_b : 1;          /*!< [4..4] Enable signal for controlling
                                     Deepsleep signal of all SRAM used by M4 1-
                                     Enable SRAM Deepsleep Signal 0- Disable
                                     SRAM Deepsleep Signal. */
      __IOM unsigned int Reserved1 : 11;                     /*!< [15..5] It is recommended to write
                                         these bits to 0. */
      __IOM unsigned int POWER_ENABLE_FSM_PERI_b : 1;        /*!< [16..16] Its enable or disable Power
                                           to Low-Power FSM.                   */
      __IOM unsigned int POWER_ENABLE_TIMESTAMPING_b : 1;    /*!< [17..17] Its enable or disable
                                               Power to TIMESTAMP. */
      __IOM unsigned int POWER_ENABLE_DEEPSLEEP_TIMER_b : 1; /*!< [18..18] Its enable or
                                                  disable Power to DEEP SLEEP
                                                  Timer.         */
      __IOM unsigned int POWER_ENABLE_RETENTION_DM_b : 1;    /*!< [19..19] Its enable or disable
                                               Power to Retention Flops during
                                                        SHIP state.These Flops
                                               are used for storing Chip
                                               Configuration.           */
      __IOM unsigned int Reserved2 : 12;                     /*!< [31..20] It is recommended to write
                                         these bits to 0.                      */
    } MCU_FSM_CRTL_PDM_AND_ENABLES_b;
  };

  union {
    __IOM unsigned int MCU_GPIO_TIMESTAMPING_CONFIG; /*!< (@ 0x00000028) MCU GPIO
                                                     TIMESTAMPING CONFIG. */

    struct {
      __IOM unsigned int ENABLE_GPIO_TIMESTAMPING_b : 1; /*!< [0..0] Enable GPIO time stamping
                                              Feature.. */
      __IOM unsigned int TIMESTAMPING_ON_GPIO0_b : 1;    /*!< [1..1] Enable GPIO time
                                                      stamping on GPIO0. */
      __IOM unsigned int TIMESTAMPING_ON_GPIO1_b : 1;    /*!< [2..2] Enable GPIO time
                                                      stamping on GPIO1. */
      __IOM unsigned int TIMESTAMPING_ON_GPIO2_b : 1;    /*!< [3..3] Enable GPIO time
                                                      stamping on GPIO2. */
      __IOM unsigned int TIMESTAMPING_ON_GPIO3_b : 1;    /*!< [4..4] Enable GPIO time
                                                      stamping on GPIO3. */
      __IOM unsigned int TIMESTAMPING_ON_GPIO4_b : 1;    /*!< [5..5] Enable GPIO time
                                                      stamping on GPIO4. */
      __IOM unsigned int Reserved1 : 26;                 /*!< [31..6] It is recommended to write
                                         these bits to 0. */
    } MCU_GPIO_TIMESTAMPING_CONFIG_b;
  };

  union {
    __IM unsigned int MCU_GPIO_TIMESTAMP_READ; /*!< (@ 0x0000002C) MCU GPIO
                                               TIMESTAMPING READ. */

    struct {
      __IM unsigned int GPIO_EVENT_COUNT_PARTIAL : 11; /*!< [10..0] Counter value indicating
                                            the duration from GPIO going high to
                                            first Sleep clock( MCU FSM Clock)
                                            posedge from GPIO going high with
                                            respect to MHz RC clock. */
      __IM unsigned int Reserved1 : 5;                 /*!< [15..11] It is recommended to write
                                       these bits to 0.                      */
      __IM unsigned int GPIO_EVENT_COUNT_FULL : 11;    /*!< [26..16] Counter value indicating
                                          number for RC MHz clock present in 1
                                          Sleep clock (MCU FSM Clock). */
      __IM unsigned int Reserved2 : 5;                 /*!< [31..27] It is recommended to write
                                          these bits to 0.                      */
    } MCU_GPIO_TIMESTAMP_READ_b;
  };

  union {
    __IOM unsigned int MCU_SLEEPHOLD_REQ; /*!< (@ 0x00000030) MCU SLEEP HOLD REQ. */

    struct {
      __IOM unsigned int SLEEPHOLDREQn : 1;   /*!< [0..0] Sleepholdreq when enable
                                            will gate the clock to M4. 1
                                                      - Sleepholdreq is Disabled.
                                            0 - Sleepholdreq is Enabled. */
      __IM unsigned int SLEEPHOLDACKn : 1;    /*!< [1..1] SLEEPHOLDACK response to
                                            SLEEPHOLDREQ.  */
      __IOM unsigned int Reserved1 : 14;      /*!< [15..2] It is recommended to write
                                            these bits to 0.    */
      __IOM unsigned int SELECT_FSM_MODE : 1; /*!< [16..16] Enable for selecting secondary FSM.
                                   1 - Select Secondary FSM 0 - Select Primary
                                   FSM. */
      __IOM unsigned int Reserved2 : 15;      /*!< [31..17] It is recommended to write
                                         these bits to 0.                      */
    } MCU_SLEEPHOLD_REQ_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int MCU_FSM_WAKEUP_STATUS_REG; /*!< (@ 0x00000038) MCU FSM Wakeup
                                                  Status Register. */

    struct {
      __IOM unsigned int WAKEUP_STATUS : 11;            /*!< [10..0] To know the wakeup source.       */
      __IOM unsigned int Reserved1 : 5;                 /*!< [15..11] It is recommended to write
                                        these bits to 0.                      */
      __IOM unsigned int MCU_FIRST_POWERUP_POR : 1;     /*!< [16..16] Indication to Processor that
                                         system came out first power up. */
      __IOM unsigned int MCU_FIRST_POWERUP_RESET_N : 1; /*!< [17..17] Indication to Processor
                                             that system came out of Reset. */
      __IOM unsigned int Reserve2 : 14;                 /*!< [31..18] It is recommended to write
                                        these bits to 0.                      */
    } MCU_FSM_WAKEUP_STATUS_REG_b;
  };

  union {
    __IOM unsigned int MCU_FSM_WAKEUP_STATUS_CLEAR; /*!< (@ 0x0000003C) MCU FSM
                                                    Wakeup Status Clear. */

    struct {
      __IOM unsigned int WWD_INTERRUPT_STATUS_CLEAR_b : 1;          /*!< [0..0] To Clear WatchDog
                                                Interrupt status indication. */
      __IOM unsigned int MILLI_SEC_BASED_STATUS_CLEAR_b : 1;        /*!< [1..1] To Clear Milli-Second
                                                  Wakeup status indication. */
      __IOM unsigned int RTC_SEC_BASED_STATUS_CLEAR_b : 1;          /*!< [2..2] To Clear Second Tick
                                                wakeup status indication. */
      __IOM unsigned int RTC_ALARM_BASED_WAKEUP_STATUS_CLEAR_b : 1; /*!< [3..3] To Clear RTC
                                                         Alarm wakeup status
                                                         indicaition.       */
      __IOM unsigned int COMP1_BASED_WAKEUP_STATUS_CLEAR_b : 1;     /*!< [4..4] To Clear comp1
                                                     wakeup (Analog IP1 and
                                                     Analog IP2) status
                                                        indication. */
      __IOM unsigned int COMP2_BASED_WAKEUP_STATUS_CLEAR_b : 1;     /*!< [5..5] To Clear comp2
                                                     wakeup (Analog IP1 and
                                                     BandGap Scale) status
                                                     indication. */
      __IOM
      unsigned int COMP3_BASED_WAKEUP_STATUS_CLEAR_b : 1; /*!< [6..6] To Clear comp3
                                                         wakeup (Analog IP1 and
                                                         VBatt Scale) status
                                                            indication. */
      __IOM
      unsigned int COMP4_BASED_WAKEUP_STATUS_CLEAR_b : 1;       /*!< [7..7] To Clear Comp4
                                                         wakeup (Bandgap En and
                                                         VBatt Scale) status
                                                            indication. */
      __IOM unsigned int COMP5_BASED_WAKEUP_STATUS_CLEAR_b : 1; /*!< [8..8] To Clear BOD Wakeup
                                                     status indication. */
      __IOM unsigned int COMP6_BASED_WAKEUP_STATUS_CLEAR_b : 1; /*!< [9..9] To Clear
                                                     Button-wake status
                                                     indication. */
#if !defined(SLI_SI917B0)
      __IOM unsigned int RF_WAKEUP_CLEAR_b : 1; /*!< [10..10] To Clear WuRX status
                                                indication. */
#else
      __IOM unsigned int SYSRTC_WAKEUP_CLEAR_b : 1;    /*!< [10..10] To Clear SYSRTC Wakeup status
                                         indication. */
#endif
      __IOM unsigned int Reserved1 : 21; /*!< [31..11] It is recommended to write
                                         these bits to 0.                      */
    } MCU_FSM_WAKEUP_STATUS_CLEAR_b;
  };

  union {
    __IOM unsigned int MCU_FSM_PMU_STATUS_REG; /*!< (@ 0x00000040) MCU FSM PMU
                                               Status Register. */

    struct {
      __IOM unsigned int SCDCDC_LP_MODE_EN : 1;  /*!< [0..0] SCDC in LP mode. */
      __IOM unsigned int BGPMU_SLEEP_EN_R_b : 1; /*!< [1..1] Sleep en for BG PMU.    */
      __IOM unsigned int Reserved1 : 15;         /*!< [16..2] It is recommended to write
                                         these bits to 0. */
      __IOM unsigned int STANDBY_LDORF_R : 1;    /*!< [17..17] Standby state for LDO RF. */
      __IOM unsigned int STANDBY_LDOSOC_R : 1;   /*!< [18..18] Standby state for LDO soc. */
      __IOM unsigned int STANDBY_DC1P3_R : 1;    /*!< [19..19] Standby state for DC1p3. */
      __IM unsigned int POWERGOOD_LDOSOC : 1;    /*!< [20..20] Powergood signal from ldosoc. */
      __IM unsigned int LEVEL_OK_DC1P3 : 1;      /*!< [21..21] Powergood signal from LDORF. */
      __IM unsigned int CL_FLAG_DC1P3 : 1;       /*!< [22..22] Powergood signal read for
                                           DC 1.3V. */
      __IOM unsigned int Reserved2 : 9;          /*!< [31..23] It is recommended to write
                                           these bits to 0.                      */
    } MCU_FSM_PMU_STATUS_REG_b;
  };

  union {
    __IOM unsigned int MCU_FSM_PMUX_CTRLS_RET; /*!< (@ 0x00000044) MCU FSM PMUX
                                               Controls Retention. */

    struct {
      __IOM unsigned int POWER_SW_CTRL_TASS_RAM_IN_RETAIN : 1;     /*!< [0..0] Select value for
                                                    TASS RAM Power Mux In
                                                    Retention mode    */
      __IOM unsigned int POWER_SW_CTRL_M4SS_RAM_IN_RETAIN : 1;     /*!< [1..1] Select value for
                                                    M4SS RAM Power Mux In
                                                    Retention mode    */
      __IOM unsigned int POWER_SW_CTRL_M4ULP_RAM_IN_RETAIN : 2;    /*!< [3..2] Select value for
                                                     M4ULP RAM Power Mux In
                                                     Retention mode  */
      __IOM unsigned int POWER_SW_CTRL_M4ULP_RAM16K_IN_RETAIN : 2; /*!< [5..4] Select value for
                                                        M4ULP 16K RAM Power Mux
                                                        In Retention mode */
      __IOM unsigned int POWER_SW_CTRL_ULPSS_RAM_IN_RETAIN : 2;    /*!< [7..6] Select value for
                                                     ULPSS RAM Power Mux In
                                                     Retention mode  */
      __IOM unsigned int Reserved1 : 24;                           /*!< [31..8] It is recommended to write
                                         these bits to 0. */
    } MCU_FSM_PMUX_CTRLS_RET_b;
  };

  union {
    __IOM unsigned int MCU_FSM_TOGGLE_COUNT; /*!< (@ 0x00000048) MCU FSM Toggle Count. */

    struct {
      __OM unsigned int TOGGLE_COUNT_RSTART : 1; /*!< [0..0] Start counting GIPO Toggles.   */
      __IOM unsigned int Reserved1 : 14;         /*!< [14..1] It is recommended to write
                                         these bits to 0. */
      __OM unsigned int LATCH_TOGGLE_DATA : 1;   /*!< [15..15] Trigger indication to
                                               read GPIO toggle data. */
      __IM unsigned int GPIO_TOGGLE_COUNT : 12;  /*!< [27..16] GPIO toogle data count.   */
      __IOM unsigned int Reserved2 : 3;          /*!< [30..28] It is recommended to write
                                        these bits to 0.                      */
      __IM unsigned int TOGGLE_DATA_READY : 1;   /*!< [31..31] GPIO toogle data count. */
    } MCU_FSM_TOGGLE_COUNT_b;
  };
} MCU_FSM_Type; /*!< Size = 76 (0x4c) */

/* ===========================================================================================================================
  */
/* ================                                     MCU_ProcessSensor
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief The process sensor module, count the process clock (high frequency
   ring clock) over one MCU FSM clock and divide this clock by programmable
   value. (MCU_ProcessSensor)
   */

typedef struct { /*!< (@ 0x24048540) MCU_ProcessSensor Structure */

  union {
    __IOM unsigned int PROCESS_SENSOR_ENABLE_AND_READ; /*!< (@ 0x00000000) Process sensor
                                            enable and read for operation */

    struct {
      __IOM unsigned int PROCESS_SENSOR_EN : 1; /*!< [0..0] enable or on the process sensor,if
                                     this bit is set the sensor enable else
                                     sensor is disable. */
      __IOM unsigned int PS_RING_CLK_START : 1; /*!< [1..1] Start Ring-Oscillator clock for
                                     estimating process corner.         */
      __IOM unsigned int PS_CLK_SW_ON : 1;      /*!< [2..2] Clock cleaner on signal to clock cleaner
                                block on clock generated by delay chain. */
      __IOM unsigned int PS_CLK_SW_OFF : 1;     /*!< [3..3] Clock cleaner off signal to clock
                                 cleaner block on clock
                                           generated by delay chain. */
      __IOM unsigned int NUM_CYCLES : 2;        /*!< [5..4] Number of MCU FSM clock(32KHz)for which
                              measurement need to be done.if bits is 1 then 1
                              clock, 2 then 2 clocks,3 then 3 clocks,4 then 4
                              clocks. */
      __IM unsigned int PS_MEAS_DONE_SYNC : 1;  /*!< [6..6] Processor sensor
                                               measurement done. */
      __IOM unsigned int RESERVED1 : 9;         /*!< [15..7] Reserved1        */
      __IM unsigned int PS_COUNT : 16;          /*!< [31..16] Processor sensor read back */
    } PROCESS_SENSOR_ENABLE_AND_READ_b;
  };
} MCU_ProcessSensor_Type; /*!< Size = 4 (0x4) */

/* ===========================================================================================================================
  */
/* ================                                          MCU_RET
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief NPSS has Retention domain logic which is a power  domain .
                This domain consisted all logic which will turned off if none of
   the  M4 memories are retained. (MCU_RET)
   */

typedef struct { /*!< (@ 0x24048600) MCU_RET Structure */

  union {
    __IOM unsigned int MCURET_QSPI_WR_OP_DIS; /*!< (@ 0x00000000) MCURET QSPI WR OP DIS */

    struct {
      __IOM unsigned int M4SS_QSPI_WRSR_WR_OP_DISABLE : 1; /*!< [0..0] M4SS Write operation
                                                disable to Flash. 1 - Write
                                                Operation to Flash is not
                                                allowed. 0 - Write Operation to
                                                Flash is allowed. */
      __IM unsigned int TASS_QSPI_WRSR_WR_OP_DISABLE : 1;  /*!< [1..1] TASS Write operation
                                                disable to Flash. 1 - Write
                                                Operation to Flash is not
                                                allowed. 0 - Write Operation to
                                                Flash is allowed. */
      __IOM unsigned int RESERVED1 : 30;                   /*!< [31..2] Reserved1        */
    } MCURET_QSPI_WR_OP_DIS_b;
  };

  union {
    __IM unsigned int MCURET_BOOTSTATUS; /*!< (@ 0x00000004) MCURET BOOT Status */

    struct {
      __IM unsigned int BOOT_STATUS : 1; /*!< [0..0] Boot Status/Configuration
                                         information to MCU */
      __IM unsigned int RESERVED1 : 31;  /*!< [31..1] Reserved1  */
    } MCURET_BOOTSTATUS_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IM unsigned int CHIP_CONFIG_MCU_READ; /*!< (@ 0x0000000C) MCURET BOOT Status */

    struct {
      __IM unsigned int DISABLE_M4 : 1;                       /*!< [0..0] When set, disables the M4 by
                                        clock gating and putting M4 in reset */
      __IM unsigned int LIMIT_M4_FREQ_110MHZ_b : 1;           /*!< [1..1] When set, limits the M4SS SoC
                                          clock to Max clock/2                 */
      __IM unsigned int DISABLE_M4_ULP_MODE : 1;              /*!< [2..2] When set, limits the M4SS SoC
                                       clock to Max clock/2                 */
      __IM unsigned int RESERVED1 : 7;                        /*!< [9..3] Reserved1 */
      __IM unsigned int M4_FLASH_SIZE : 3;                    /*!< [12..10] 0xx - Unrestricted 100 - Auto mode
                                 accesses to flash are restricted to 4 MBit 101 -
                                 Auto mode accesses to flash are restricted to 8
                                 MBit 110 - Auto mode accesses to flash are
                                 restricted to 16 MBit 111 - Auto mode accesses
                                 to flash are restricted to 32 MBit */
      __IM unsigned int DISABLE_FIM_COP : 1;                  /*!< [13..13] When set, disable FIMV         */
      __IM unsigned int DISABLE_VAP : 1;                      /*!< [14..14] When set, disables VAD   */
      __IM unsigned int DISABLE_TOUCH : 1;                    /*!< [15..15] When set, disables touch
                                           interface */
      __IM unsigned int RESERVED2 : 1;                        /*!< [16..16] Reserved2     */
      __IM unsigned int DISABLE_ANALOG_PERIPH : 1;            /*!< [17..17] When set, disables
                                                   analog peripherals */
      __IM unsigned int DISABLE_JTAG : 1;                     /*!< [18..18] When set, disable JTAG
                                          interface(both M4 and NWP) */
      __IM unsigned int DISABLE_M4SS_KH_ACCESS : 1;           /*!< [19..19] When set, disables
                                                    access to key in the key
                                                    holder from M4SS QSPI */
      __IM unsigned int DISABLE_M4SS_ACCESS_FRM_TASS_SEC : 1; /*!< [20..20] When set, M4 can
                                                    not access TASS memory or
                                                    registers except for host
                                                    communication registers */
      __IM unsigned int RESERVED3 : 11;                       /*!< [31..21] Reserved3             */
    } CHIP_CONFIG_MCU_READ_b;
  };

  union {
    __IOM unsigned int MCUAON_CTRL_REG4; /*!< (@ 0x00000010) MCURET Control Register4 */

    struct {
      __IOM unsigned int RESERVED1 : 16;                   /*!< [15..0] Reserved1 */
      __IOM unsigned int ULP_GPIO_2_TEST_MODE_OUT_SEL : 4; /*!< [19..16] NPSS Test modes */
      __IOM unsigned int ULP_GPIO_1_TEST_MODE_OUT_SEL : 4; /*!< [23..20] NPSS Test modes */
      __IOM unsigned int ULP_GPIO_0_TEST_MODE_OUT_SEL : 4; /*!< [27..24] NPSS Test modes */
      __IOM unsigned int ULP_GPIOS_IN_TEST_MODE : 1;       /*!< [28..28] NPSS Test modes */
      __IOM unsigned int RESERVED2 : 3;                    /*!< [31..29] Reserved2   */
    } MCUAON_CTRL_REG4_b;
  };
  __IM unsigned int RESERVED1[2];
  __IOM MCU_RET_NPSS_GPIO_CNTRL_Type NPSS_GPIO_CNTRL[5]; /*!< (@ 0x0000001C) [0..4] */
} MCU_RET_Type;                                          /*!< Size = 48 (0x30)           */

/* ===========================================================================================================================
  */
/* ================                                         MCU_TEMP
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief The temperature sensor is used to read the temperature by using APB
   registers, which is access through direct to ULPSS system. (MCU_TEMP)
   */

typedef struct { /*!< (@ 0x24048500) MCU_TEMP Structure */

  union {
    __IOM unsigned int TS_ENABLE_AND_TEMPERATURE_DONE; /*!< (@ 0x00000000) Temperature sensor
                                            enable and measurement calculation
                                                                       done
                                            indication register */

    struct {
      __OM unsigned int TEMP_SENS_EN : 1;          /*!< [0..0] Temperature sensing
                                          enable,self clearing register */
      __IOM unsigned int REF_CLK_SEL : 1;          /*!< [1..1] if this bit is zero then reference RO
                               clock from analog,else this bit is one then MCU
                               FSM clock */
      __IOM unsigned int CONT_COUNT_FREEZE : 10;   /*!< [11..2] Count of reference clock on which
                                      ptat clock counts               */
      __IM unsigned int TEMP_MEASUREMENT_DONE : 1; /*!< [12..12] temperature measurement done
                                         indication.                         */
      __IOM unsigned int RESERVED1 : 19;           /*!< [31..13] reserved1 */
    } TS_ENABLE_AND_TEMPERATURE_DONE_b;
  };

  union {
    __IOM unsigned int TS_SLOPE_SET; /*!< (@ 0x00000004) temperature sensor slope set(slope
                          will be change with respect to temperature change) */

    struct {
      __IOM unsigned int SLOPE : 10;           /*!< [9..0] This is one time measurement for one package
                          after chip arrives from fab,this is signed bit. */
      __IOM unsigned int RESERVED1 : 6;        /*!< [15..10] Reserved1        */
      __IOM unsigned int TEMPERATURE_SPI : 11; /*!< [26..16] temperature known */
      __OM unsigned int TEMP_UPDATED : 1;      /*!< [27..27] temperature updated signal for the reg
                                to capture this temperature. */
      __IOM unsigned int BJT_BASED_TEMP : 1;   /*!< [28..28] Temperature is updated through which
                                  is calculated using bjt based if bit is high(1)
                                  through spi and bit is low(0) then through
                                  calculation RO based */
      __IOM unsigned int RESERVED2 : 3;        /*!< [31..29] Reserved2 */
    } TS_SLOPE_SET_b;
  };

  union {
    __IOM unsigned int TS_FE_COUNTS_NOMINAL_SETTINGS; /*!< (@ 0x00000008) determine
                                                      calibrated temperature */

    struct {
      __IOM unsigned int F2_NOMINAL : 10;         /*!< [9..0] ptat clock count during calibration,This
                               will vary with chip to chip. */
      __IOM unsigned int RESERVED1 : 6;           /*!< [15..10] Reserved1 */
      __IOM unsigned int NOMINAL_TEMPERATURE : 7; /*!< [22..16] calibrated temperature  */
      __IOM unsigned int RESERVED2 : 9;           /*!< [31..23] Reserved2 */
    } TS_FE_COUNTS_NOMINAL_SETTINGS_b;
  };

  union {
    __IM unsigned int TS_COUNTS_READ; /*!< (@ 0x0000000C) temperature sensor count read. */

    struct {
      __IM unsigned int COUNT_F2 : 10; /*!< [9..0] COUNT_F2 */
      __IM unsigned int RESERVED1 : 6; /*!< [15..10] Reserved1 */
      __IM unsigned int COUNT_F1 : 10; /*!< [25..16] COUNT_F1 */
      __IM unsigned int RESERVED2 : 6; /*!< [31..26] Reserved2 */
    } TS_COUNTS_READ_b;
  };

  union {
    __IOM unsigned int TEMPERATURE_READ; /*!< (@ 0x00000010) read the temperature */

    struct {
      __IM unsigned int TEMPERATURE_RD : 11; /*!< [10..0] Temperature value for read
                                             in signed format */
      __IOM unsigned int RES10 : 21;         /*!< [31..11] reserved10         */
    } TEMPERATURE_READ_b;
  };
} MCU_TEMP_Type; /*!< Size = 20 (0x14) */

/* ===========================================================================================================================
  */
/* ================                                          MCU_AON
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief NPSS has always ON domain logic which is not power gatable  Which
   consistes of power, reset, isolation controls for different power domains in
   NPSS. (MCU_AON)
   */

typedef struct { /*!< (@ 0x24048000) MCU_AON Structure */

  union {
    __IOM unsigned int MCUAON_NPSS_PWRCTRL_SET_REG; /*!< (@ 0x00000000) This register used for
                                         NPSS power control set register. */

    struct {
      __IOM unsigned int RES : 1;                            /*!< [0..0] bit is reserved */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUBFFS : 1; /*!< [1..1] MCU domain battery
                                                  FF's power gate enable.If
                                                  set,Power
                                                        Supply is On clearing
                                                  this bit has no effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUFSM : 1;  /*!< [2..2] MCU FSM power gate
                                                 enable,If set power supply is on
                                                 clearing
                                                        this bit has no effect.
                                               */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCURTC : 1;  /*!< [3..3] MCU RTC power gate
                                                 enable if set,power supply is on
                                                 clearing
                                                        this bit has no effect.
                                               */
      __IOM
      unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUWDT : 1; /*!< [4..4] MCU WDT power
                                                          gate enable if
                                                          set,power supply is on
                                                          clearing this bit has
                                                          no effect */
      __IOM
      unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUPS : 1;                 /*!< [5..5] MCU PS power gate
                                                    enable.if set,power supply is
                                                    on clearing this bit has no
                                                    effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUTS : 1;           /*!< [6..6] MCU temperature sensor
                                                power gate enable if set,power
                                                        supply is on.clearing
                                                this bit has no effect */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUSTORE1 : 1;       /*!< [7..7] MCU Storage 1 power
                                                    gate enable for 64-bit.if
                                                    set,power supply is
                                                    on,clearing this bit has no
                                                    effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUSTORE2 : 1;       /*!< [8..8] MCU Storage 2 power
                                                    gate enable for 64-bit.if
                                                    set,power supply is
                                                    on,clearing this bit has no
                                                    effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUSTORE3 : 1;       /*!< [9..9] MCU Storage 3 power
                                                    gate enable for 64-bit.if
                                                    set,power supply is
                                                    on,clearing this bit has no
                                                    effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_TIMEPERIOD : 1;      /*!< [10..10] TIMEPERIOD power
                                                     gate enable. */
      __IOM unsigned int RESERVED1 : 5;                              /*!< [15..11] reserved1              */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_NWPAPB_MCU_CTRL : 1; /*!< [16..16] NWPAPB MCU
                                                          control power gate
                                                          enable             */
      __IOM unsigned int RESERVED2 : 15;                             /*!< [31..17] reserved2                  */
    } MCUAON_NPSS_PWRCTRL_SET_REG_b;
  };

  union {
    __IOM unsigned int MCUAON_NPSS_PWRCTRL_CLEAR_REG; /*!< (@ 0x00000004) This register used
                                           for NPSS power control clear
                                                          register. */

    struct {
      __IOM unsigned int RES : 1;                                    /*!< [0..0] bit is reserved */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUBFFS : 1;         /*!< [1..1] MCU domain battery
                                                  FF's power gate enable.If
                                                  set,Power Supply is OFF
                                                  clearing this bit has no
                                                  effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUFSM : 1;          /*!< [2..2] MCU FSM power gate
                                                 enable,If set power supply is
                                                 OFF clearing this bit has no
                                                 effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCURTC : 1;          /*!< [3..3] MCU RTC power gate
                                                 enable if set,power supply is
                                                 OFF clearing this bit has no
                                                 effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUWDT : 1;          /*!< [4..4] MCU WDT power gate
                                                 enable if set,power supply is
                                                 OFF clearing this bit has no
                                                 effect */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUPS : 1;           /*!< [5..5] MCU PS power gate
                                                enable.if set,power supply is OFF
                                                clearing
                                                        this bit has no effect.
                                              */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUTS : 1;           /*!< [6..6] MCU temperature sensor
                                                power gate enable if set,power
                                                        supply is OFF.clearing
                                                this bit has no effect */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUSTORE1 : 1;       /*!< [7..7] MCU Storage 1 power
                                                    gate enable for 64-bit.if
                                                    set,power supply is
                                                    OFF,clearing this bit has no
                                                    effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUSTORE2 : 1;       /*!< [8..8] MCU Storage 2 power
                                                    gate enable for 64-bit.if
                                                    set,power supply is
                                                    OFF,clearing this bit has no
                                                    effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_MCUSTORE3 : 1;       /*!< [9..9] MCU Storage 3 power
                                                    gate enable for 64-bit.if
                                                    set,power supply is
                                                    OFF,clearing this bit has no
                                                    effect. */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_TIMEPERIOD : 1;      /*!< [10..10] TIMEPERIOD power
                                                     gate enable. */
      __IOM unsigned int RESERVED1 : 5;                              /*!< [15..11] reserved1              */
      __IOM unsigned int SLPSS_PWRGATE_EN_N_ULP_NWPAPB_MCU_CTRL : 1; /*!< [16..16] NWPAPB MCU
                                                          control power gate
                                                          enable             */
      __IOM unsigned int RESERVED2 : 15;                             /*!< [31..17] reserved2                  */
    } MCUAON_NPSS_PWRCTRL_CLEAR_REG_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int MCUAON_IPMU_RESET_CTRL; /*!< (@ 0x0000000C) This register used for ipmu
                                    reset control register         */

    struct {
      __IOM unsigned int ULP_ANALOG_SPI_RESET_N : 1; /*!< [0..0] ULP Analog SPI Reset Signal,
                                          if bit is 1 then outoff reset,else in
                                          reset */
      __IOM unsigned int IPMU_SPI_RESET_N : 1;       /*!< [1..1] IPMU SPI Reset Signal,if bit is 1
                                    then outoff reset,else in reset */
      __IOM unsigned int RESERVED1 : 30;             /*!< [31..2] reserved1 */
    } MCUAON_IPMU_RESET_CTRL_b;
  };

  union {
    __IOM unsigned int MCUAON_SHELF_MODE; /*!< (@ 0x00000010) This register used for
                                          control shelf mode.                 */

    struct {
      __OM unsigned int ENTER_SHELF_MODE : 16;        /*!< [15..0] Program 0xAAAA for
                                                   entering shelf mode.     */
      __IOM unsigned int SHUTDOWN_WAKEUP_MODE : 2;    /*!< [17..16] GPIO based wakeup
                                                   mode configuration. */
      __IOM unsigned int SHELF_MODE_GPIOBASED : 1;    /*!< [18..18] GPIO based shelf mode
                                        entering,If set 1 by processor, On
                                        Falling edge of GPIO (Based on the option
                                        used in shutdown_wakeup_mode register)
                                        chip will enter Shelf mode. */
      __IOM unsigned int SHELF_MODE_WAKEUP_DELAY : 3; /*!< [21..19] Programmable delay for
                                           resetting Chip during exit phase of
                                           shelf mode. */
      __IOM unsigned int RESERVED1 : 10;              /*!< [31..22] reserved1   */
    } MCUAON_SHELF_MODE_b;
  };

  union {
    __IOM unsigned int MCUAON_GEN_CTRLS; /*!< (@ 0x00000014) This register used for
                                         MCUON gen control mode.             */

    struct {
      __IOM unsigned int XTAL_CLK_FROM_GPIO : 1;       /*!< [0..0] Select external 32KHz clock from
                                     NPSS GPIO's,if bit is 1 then select XTAL
                                     clock from GPIO Pins.  Please refer to NPSS
                                     GPIO Pin muxing for configuration.else
                                     select XTAL clock from IPMU clock sources.
                                   */
      __IOM unsigned int ULP_ANALOG_WAKEUP_ACCESS : 1; /*!< [1..1] ULP analog wakeup Source
                                           Access,if bit is 1 then TASS
                                           else bit is 0 then M4SS. */
      __IOM unsigned int RES : 14;                     /*!< [15..2] reser          */
      __IOM unsigned int ENABLE_PDO : 1;               /*!< [16..16] Enable turning Off POD power
                                         domain when SOC_LDO EN is low,When Set
                                         to 1, Up on SoC LDO Enable going low, IO
                                                      supply (3.3v)to SOC Pads
                                         will be tuned-off. */
      __IOM unsigned int NPSS_SUPPLY_0P9 : 1;          /*!< [17..17] keep npss supply always at 0.9V,if
                                   bit is 1 then npss supply always at 0.9V else
                                   bit is zero then npss supply will switch from
                                   0.6V to 0.9V  based on high frequency
                                           enables. */
      __IOM unsigned int RESERVED1 : 14;               /*!< [31..18] reser */
    } MCUAON_GEN_CTRLS_b;
  };

  union {
    __IOM unsigned int MCUAON_PDO_CTRLS; /*!< (@ 0x00000018) This register used for
                                         MCUON PDO control mode.             */

    struct {
      __IOM
      unsigned int SOC_B_IO_DOMAIN_EN_B : 1;       /*!< [0..0] Turn-Off IO supply of SOC
                                            domain on bottom side,if bit is 1
                                            then turn-off and 0 then turn on */
      __IOM unsigned int SOC_L_IO_DOMAIN_EN_B : 1; /*!< [1..1] Turn-Off IO supply of SOC domain
                                        on left side,if bit is 1  then turn-off
                                        and 0 then turn on */
      __IOM unsigned int SOC_T_IO_DOMAIN_EN_B : 1; /*!< [2..2] Turn-Off IO supply of SOC domain
                                        on top side,if bit is 1  then turn-off
                                        and 0 then turn on */
      __IOM unsigned int QSPI_IO_DOMAIN_EN_B : 1;  /*!< [3..3] Turn-Off IO supply of QSPI
                                       domain,if bit is 1 then  turn-off and 0
                                       then turn on */
      __IOM unsigned int SDIO_IO_DOMAIN_EN_B : 1;  /*!< [4..4] Turn-Off IO supply of SDIO
                                       domain.,if bit is 1 then turn-off and 0
                                       then turn on */
      __IOM unsigned int RES : 27;                 /*!< [31..5] reser     */
    } MCUAON_PDO_CTRLS_b;
  };

  union {
    __IOM unsigned int MCUAON_WDT_CHIP_RST; /*!< (@ 0x0000001C) This register used
                                            for wdt chip reset purpose. */

    struct {
      __IOM unsigned int MCU_WDT_BASED_CHIP_RESET : 1; /*!< [0..0] When cleared, Up on host
                                           reset request.Power-On Reset (POR)
                                           will be generated */
      __IOM unsigned int RESERVED1 : 31;               /*!< [31..1] reserved1    */
    } MCUAON_WDT_CHIP_RST_b;
  };

  union {
    __IOM unsigned int MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS; /*!< (@ 0x00000020) This register
                                                 used for khz clock select and
                                                 reset status */

    struct {
      __IOM unsigned int AON_KHZ_CLK_SEL : 3;               /*!< [2..0] NPSS AON KHz clock selection,if 001
                                   Khz RO clock select,else
                                           if 010 - Khz RC clock select,else 100
                                   Khz Xtal clock select               */
      __IM unsigned int AON_KHZ_CLK_SEL_CLOCK_SWITCHED : 1; /*!< [3..3] If Khz clock mux
                                                  select is modified,please poll
                                                  this bit and wait  till it
                                                  becomes one. */

#if defined(SLI_SI917B0)
      __IOM unsigned int AON_KHZ_CLK_SEL_WWD : 4;                  /* [4 .. 7] NPSS AON KHz clock
                                                  selection for WWD */
      __IM unsigned int AON_KHZ_CLK_SEL_CLOCK_SWITCHED_WWD : 1;    /*!< [8..8] If Khz clock mux
                                                      select is modified for
                                                      wwd,please poll this bit
                                                      and wait  till it becomes
                                                      one. */
      __IOM unsigned int AON_KHZ_CLK_SEL_SYSRTC : 4;               /* [9 .. 12] NPSS AON KHz clock
                                                     selection for SYSRTC */
      __IM unsigned int AON_KHZ_CLK_SEL_CLOCK_SWITCHED_SYSRTC : 1; /*!< [13..13] If Khz clock
                                                         mux select is modified
                                                         for sysrtc,please poll
                                                         this
                                                                        bit and
                                                         wait  till it becomes
                                                         one. */
      __IOM unsigned int RES : 2;                                  /*!< [14..15] reserved */
#else
      __IOM unsigned int RES : 12;                     /*!< [15..4] reser       */
#endif

      __IOM unsigned int MCU_FIRST_POWERUP_POR : 1;     /*!< [16..16] Program this bit to '1' upon
                                         power_up.It will be clear when Vbatt
                                         power is removed */
      __IOM unsigned int MCU_FIRST_POWERUP_RESET_N : 1; /*!< [17..17] Program this bit to '1'
                                             upon power_up,It will be clear
                                                        when reset pin is pulled
                                             low. */
#if defined(SLI_SI917B0)
      __IOM unsigned int SYSRTC_32KHZ_RC_CLK_DIV_FACTOR : 6; /* [18..23] Clock division factor
                                                  for 32Khz_rc_clk (Used in
                                                  SYSRTC and MCU WWD) */
      __IOM unsigned int RESERVED1 : 8;                      /*!< [24..31] reserved1 */
#else
      __IOM unsigned int RESERVED1 : 14;               /*!< [31..18] reserved1 */
#endif
    } MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b;
  };
} MCU_AON_Type; /*!< Size = 36 (0x24) */

/* ===========================================================================================================================
  */
/* ================                                          ULPCLK
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief This block provides programming support for miscellaneous blocks in
   the chip. Various features in the chip are enabled using this. (ULPCLK)
   */

typedef struct { /*!< (@ 0x24041400) ULPCLK Structure */

  union {
    __IOM unsigned int ULP_MISC_SOFT_SET_REG; /*!< (@ 0x00000000) ULP MISC soft
                                              register set. */

    struct {
      __IOM unsigned int PCM_ENABLE_b : 1;                  /*!< [0..0] Used in pcm      */
      __IOM unsigned int PCM_FSYNC_START_b : 1;             /*!< [1..1] Used in pcm */
      __IOM unsigned int BIT_RES : 2;                       /*!< [3..2] Used in pcm           */
      __IOM unsigned int IR_PCLK_EN_b : 1;                  /*!< [4..4] Static clock enable for IR
                                           APB Interface */
      __IOM unsigned int PCLK_ENABLE_I2C_b : 1;             /*!< [5..5] This bit is used as Static enable
                                     for APB clock to I2C module,if bit is zero
                                     then clock is disabled else bit is
                                           one then clock is enabled. */
      __IOM unsigned int CLK_ENABLE_I2S_b : 1;              /*!< [6..6] This bit is used to enable clock to
                                    I2S module if bit is set(1)then clock is
                                    enabled is bit is zero then clock disabled.
                                  */
      __IOM unsigned int PCLK_ENABLE_SSI_MASTER_b : 1;      /*!< [7..7] This bit is used to enable
                                            APB bus clock to SSI master,if bit is
                                            zero clock  will be available only
                                            when the request from the module is
                                            present.else bit  is one then clock
                                                        is enabled. */
      __IOM unsigned int SCLK_ENABLE_SSI_MASTER_b : 1;      /*!< [8..8] This bit is used to enable
                                           clock serial clock to SSI master,if
                                           bit is zero clock  will be available
                                           only when the request from the module
                                           is present.else bit  is one
                                           then clock is enabled. */
      __IOM unsigned int PCLK_ENABLE_UART_b : 1;            /*!< [9..9] This bit is used to enable
                                      peripheral bus clock to UART4,if bit zero
                                      then clock will be available only when the
                                      request from the module is present or a
                                      transaction is pending
                                           on the APB bus,else bit is one then
                                      clock is enabled.                     */
      __IOM unsigned int SCLK_ENABLE_UART_b : 1;            /*!< [10..10] This bit is used to enable
                                      asynchronous serial clock to UART4,if bit
                                      is zero clock  will be available only when
                                           the request from the module is
                                      present.else bit  is one then clock is
                                      enabled. */
      __IOM unsigned int FIM_PCLK_ENABLE_b : 1;             /*!< [11..11] This bit is used to enable clock
                                     to FIM reg file,if this bit is zero then
                                     clock will be available only when the
                                     request from the module is present  else bit
                                     is set(1)then clock is enabled. */
      __IOM unsigned int VAD_PCLK_ENABLE_b : 1;             /*!< [12..12] This bit is used to enable clock
                                     to FIM reg file,if this bit is zero then
                                     clock will be available only when the
                                     request from the module is present  else bit
                                     is set(1)then clock is enabled. */
      __IOM unsigned int CLK_ENABLE_TIMER_b : 1;            /*!< [13..13] This bit is used to enable clock
                                      to Timer,if this bit is zero then clock
                                      will be available only when the request
                                           from the module is present  else bit
                                      is set(1)then clock is enabled. */
      __IOM unsigned int EGPIO_CLK_EN_b : 1;                /*!< [14..14] This bit is used to enable clock to
                                  gpio,if this bit is zero then clock will be
                                  available only when the request from the module
                                  is present  else bit is set(1)then clock is
                                  enabled. */
      __IOM unsigned int REG_ACCESS_SPI_CLK_EN_b : 1;       /*!< [15..15] This bit is used to enable
                                           clock to register access spi,if this
                                           bit is zero then clock will be
                                           available only when the request from
                                           the module is present  else bit is
                                           set(1)then clock is enabled. */
      __IOM unsigned int FIM_CLK_EN_b : 1;                  /*!< [16..16] This bit is used to enable clock to
                                FIM module,if this bit is zero then clock will be
                                gated,else bit is one then clock is enabled. */
      __IOM unsigned int VAD_CLK_EN_b : 1;                  /*!< [17..17] This bit is used to enable clock to
                                vad module,if this bit is zero then clock will be
                                gated,else bit is one then clock is enabled. */
      __IOM unsigned int CLK_ENABLE_ULP_MEMORIES_b : 1;     /*!< [18..18] This bit is used to
                                           enable clock to memories,if this bit
                                           is zero then clock will be available
                                           only when the request from the module
                                           is present  else bit is set(1)then
                                           clock is enabled. */
      __IOM unsigned int EGPIO_PCLK_DYN_CTRL_DISABLE_b : 1; /*!< [19..19] This bit is used to
                                                disable dynamic clock gating on
                                                APB clock to egpio */
      __IOM unsigned int EGPIO_PCLK_ENABLE_b : 1;           /*!< [20..20] This bit is used to enable
                                      static clock to egpio APB interface */
      __IOM unsigned int TIMER_PCLK_EN_b : 1;               /*!< [21..21] This bit is used to enable static
                                  clock to Timer APB Interface */
      __IOM unsigned int AUX_ULP_EXT_TRIG_1_SEL_b : 1;      /*!< [22..22] aux adc dac controller
                                           external trigger2 mux select, to
                                           choose between  ulp gpio aux ext
                                           trigger2 and timer interrupt. */
      __IOM unsigned int AUX_ULP_EXT_TRIG_2_SEL_b : 1;      /*!< [23..23] aux adc dac controller
                                           external trigger2 mux select, to
                                           choose between ulp  gpio aux ext
                                           trigger2 and timer interrupt. */
      __IOM unsigned int AUX_SOC_EXT_TRIG_1_SEL_b : 1;      /*!< [24..24] aux adc dac controller
                                           external trigger3 mux select, to
                                           choose between soc aux  ext
                                           trigger1and soc aux ext trigger3. */
      __IOM unsigned int AUX_SOC_EXT_TRIG_2_SEL_b : 1;      /*!< [25..25] aux adc dac controller
                                           external trigger4 mux select, to
                                           choose between soc aux ext trigger2and
                                           soc aux ext trigger4.           */
      __IOM unsigned int ULPSS_M4SS_SLV_SEL_b : 1;          /*!< [26..26] select slave */
      __IOM unsigned int ULPSS_TASS_QUASI_SYNC_b : 1;       /*!< [27..27] TASS quasi sync         */
      __IOM unsigned int RESERVED1 : 2;                     /*!< [29..28] reserved1            */
      __IOM unsigned int FIM_AHB_CLK_ENABLE_b : 1;          /*!< [30..30] static clock enable
                                                   for FIM AHB interface */
      __IOM unsigned int TOUCH_SENSOR_PCLK_ENABLE_b : 1;    /*!< [31..31] Static clock enable for
                                              touch APB interface */
    } ULP_MISC_SOFT_SET_REG_b;
  };

  union {
    __IOM unsigned int ULP_TA_PERI_ISO_REG; /*!< (@ 0x00000004) ULP NWP isolation register. */

    struct {
      __IOM unsigned int UDMA_ISO_CNTRL_b : 1;      /*!< [0..0] UDMA module isolation enable,if bit
                                    is set(1) then enable else bit is zero then
                                    disable. */
      __IOM unsigned int IR_ISO_CNTRL_b : 1;        /*!< [1..1] IR module isolation enable,if bit is
                                  set(1) then enable else bit is zero then
                                  disable. */
      __IOM unsigned int I2C_ISO_CNTRL_b : 1;       /*!< [2..2] I2C module isolation enable,if bit is
                                   set(1) then enable else bit is zero then
                                   disable. */
      __IOM unsigned int I2S_ISO_CNTRL_b : 1;       /*!< [3..3] I2S module isolation enable,if bit is
                                   set(1) then enable else bit is zero then
                                   disable. */
      __IOM unsigned int SSI_ISO_CNTRL_b : 1;       /*!< [4..4] SSI module isolation enable ,if bit
                                   is set(1) then enable else bit is zero then
                                   disable. */
      __IOM unsigned int UART_ISO_CNTRL_b : 1;      /*!< [5..5] UART module isolation enable,if bit
                                    is set(1) then enable else bit is zero then
                                    disable. */
      __IOM unsigned int AUX_A2D_ISO_CNTRL_b : 1;   /*!< [6..6] AUX a2d module isolation
                                       enable,if bit is set(1) then enable else
                                       bit is zero then disable. */
      __IOM unsigned int VAD_ISO_CNTRL_b : 1;       /*!< [7..7] VAD module isolation enable,if bit is
                                   set(1) then enable else bit is zero then
                                   disable. */
      __IOM unsigned int TOUCH_ISO_CNTRL_b : 1;     /*!< [8..8] CAP sensor module isolation
                                     enable,if bit is set(1) then
                                       enable else bit is zero then disable. */
      __IOM unsigned int PROC_MISC_ISO_CNTRL_b : 1; /*!< [9..9] mis top(TOT, semaphore,
                                         interrupt cntrl, Timer) module isolation
                                         enable ,if bit is set(1) then enable
                                         else bit is zero then disable. */
      __IOM unsigned int RESERVED0 : 1;             /*!< [10..10] reserved0  */
      __IOM unsigned int RESERVED1 : 1;             /*!< [11..11] reserved1  */
      __IOM unsigned int RESERVED2 : 1;             /*!< [12..12] reserved2  */
      __IOM unsigned int RESERVED3 : 1;             /*!< [13..13] reserved3  */
      __IOM unsigned int FIM_ISO_CNTRL_b : 1;       /*!< [14..14] FIM module isolation enable ,if bit
                                   is set(1) then enable else bit is zero then
                                   disable. */
      __IOM unsigned int MEM_2K_1_ISO_CNTRL_b : 1;  /*!< [15..15] 2k SRAM memory isolation
                                        enable ,if bit is set(1) then
                                           enable else bit is zero then disable.
                                      */
      __IOM unsigned int MEM_2K_2_ISO_CNTRL_b : 1;  /*!< [16..16] 2k SRAM memory isolation
                                        enable ,if bit is set(1) then
                                           enable else bit is zero then disable.
                                      */
      __IOM unsigned int MEM_2K_3_ISO_CNTRL_b : 1;  /*!< [17..17] 2k SRAM memory isolation
                                        enable ,if bit is set(1) then
                                           enable else bit is zero then disable.
                                      */
      __IOM unsigned int MEM_2K_4_ISO_CNTRL_b : 1;  /*!< [18..18] 2k SRAM memory isolation
                                         enable ,if bit is set(1) then
                                            enable else bit is zero then disable.
                                       */
      __IOM unsigned int RESERVED4 : 13;            /*!< [31..19] reserved4 */
    } ULP_TA_PERI_ISO_REG_b;
  };

  union {
    __IOM unsigned int ULP_TA_PERI_RESET_REG; /*!< (@ 0x00000008) ULP NWP peri reset
                                              register. */

    struct {
      __IOM unsigned int UDMA_SOFT_RESET_CNTRL_b : 1;      /*!< [0..0] UDMA module soft reset
                                           enable,if bit is set(1) then out of
                                           soft reset else bit is zero then in
                                           reset.                             */
      __IOM unsigned int IR_SOFT_RESET_CNTRL_b : 1;        /*!< [1..1] IR module soft reset enable,if
                                         bit is set(1) then out of soft reset
                                         else bit is zero then in reset. */
      __IOM unsigned int I2C_SOFT_RESET_CNTRL_b : 1;       /*!< [2..2] I2C module soft reset enable
                                          ,if bit is set(1) then out of soft
                                          reset else bit is zero then in reset.
                                        */
      __IOM unsigned int I2S_SOFT_RESET_CNTRL_b : 1;       /*!< [3..3] I2S module soft reset enable
                                          ,if bit is set(1) then out of soft
                                          reset else bit is zero then in reset.
                                        */
      __IOM unsigned int SSI_SOFT_RESET_CNTRL_b : 1;       /*!< [4..4] SSI module soft reset enable
                                          ,if bit is set(1) then out of soft
                                          reset else bit is zero then in reset.
                                        */
      __IOM unsigned int UART_SOFT_RESET_CNTRL_b : 1;      /*!< [5..5] UART module soft reset enable
                                           ,if bit is set(1) then out of soft
                                           reset else bit is zero then in reset.
                                         */
      __IOM unsigned int AUX_A2D_SOFT_RESET_CNTRL_b : 1;   /*!< [6..6] AUX a2d module soft reset
                                           enable,if bit is set(1) then out of
                                           soft reset else bit is zero then in
                                           reset.                         */
      __IOM unsigned int VAD_SOFT_RESET_CNTRL_b : 1;       /*!< [7..7] VAD module soft reset
                                       enable,if bit is set(1) then out of soft
                                       reset else bit is zero then in reset. */
      __IOM unsigned int TOUCH_SOFT_RESET_CNTRL_b : 1;     /*!< [8..8] CAP Sensor module soft reset
                                         enable,if bit is set(1)
                                         then out of soft reset else bit is zero
                                         then in reset.                    */
      __IOM unsigned int PROC_MISC_SOFT_RESET_CNTRL_b : 1; /*!< [9..9] mis top(TOT, semaphore,
                                                interrupt control, Timer) module
                                                   soft reset enable,if bit is
                                                set(1) then out of soft reset
                                                   else bit is zero then in reset
                                              */
      __IOM unsigned int COMP1_OUTPUT_CNTRL_b : 1;         /*!< [10..10] This is ULP comparator1
                                        interrupt unmasking signal. 0 means
                                        comparator1 interrupt is masked and 1
                                        means unmasking.
                                           It is masked at power-on time. */
      __IOM unsigned int COMP2_OUTPUT_CNTRL_b : 1;         /*!< [11..11] This is ULP comparator2
                                        interrupt unmasking signal. 0 means
                                        comparator2 interrupt is masked and 1
                                        means unmasking.
                                           It is masked at power-on time. */
      __IOM unsigned int RESERVED1 : 2;                    /*!< [13..12] reserved1 */
      __IOM unsigned int FIM_SOFT_RESET_CNTRL_b : 1;       /*!< [14..14] FIM module soft reset
                                          enable,if bit is set(1) then out of
                                          soft reset else bit is  zero then in
                                          reset                         */
      __IOM unsigned int RESERVED2 : 17;                   /*!< [31..15] reserved2  */
    } ULP_TA_PERI_RESET_REG_b;
  };
  __IM unsigned int RESERVED[2];

  union {
    __IOM unsigned int ULP_TA_CLK_GEN_REG; /*!< (@ 0x00000014) ULP NWP clock
                                           generation register. */

    struct {
      __IOM unsigned int ULP2M4_A2A_BRDG_CLK_EN_b : 1; /*!< [0..0] Clock enable for ULP-M4SS
                                            AHB-AHB bridge,if bit is set(1)
                                                        then enable else bit is
                                            zero then in disable */
      __IOM unsigned int ULP_PROC_CLK_SEL : 4;         /*!< [4..1] ulp bus clock select. */
      __IOM unsigned int ULP_PROC_CLK_DIV_FACTOR : 8;  /*!< [12..5] ulp bus clock
                                                      division factor */
      __IOM unsigned int RES : 19;                     /*!< [31..13] reserved1                    */
    } ULP_TA_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int ULP_I2C_SSI_CLK_GEN_REG; /*!< (@ 0x00000018) ULP I2C SSI
                                                clock generation register. */

    struct {
      __IOM unsigned int ULP_I2C_CLK_EN_b : 1;       /*!< [0..0] ulp i2c clock enable,if bit is
                                        set(1) then enable else     bit is zero then in
                                        disable     */
      __IOM unsigned int RESERVED1 : 4;              /*!< [4..1] reserved1 */
      __IOM unsigned int RESERVED2 : 8;              /*!< [12..5] reserved2 */
      __IOM unsigned int RESERVED3 : 3;              /*!< [15..13] reserved3 */
      __IOM unsigned int ULP_SSI_CLK_EN_b : 1;       /*!< [16..16] ssi clk enable if set(1) then
                                    enable else bit is zero then disable */
      __IOM unsigned int ULP_SSI_CLK_DIV_FACTOR : 7; /*!< [23..17] ssi clk enable if
                                                     set(1) then enable else bit
                                                     is zero then disable */
      __IOM unsigned int RESERVED4 : 4;              /*!< [27..24] reserved4              */
      __IOM unsigned int ULP_SSI_CLK_SEL : 4;        /*!< [31..28] Ulp ssi clock select. */
    } ULP_I2C_SSI_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int ULP_I2S_CLK_GEN_REG; /*!< (@ 0x0000001C) ULP I2S clock
                                            generation register. */

    struct {
      __IOM unsigned int ULP_I2S_CLK_EN_b : 1;                /*!< [0..0] ulp i2s clk enable,if bit is set(1)
                                    then enable else
                                           bit is zero then in disable */
      __IOM unsigned int ULP_I2S_CLK_SEL_b : 4;               /*!< [4..1] ulp i2s clock select. */
      __IOM unsigned int ULP_I2S_CLKDIV_FACTOR : 8;           /*!< [12..5] ulp i2s clock
                                                    division factor. */
      __IOM unsigned int ULP_I2S_MASTER_SLAVE_MODE_b : 1;     /*!< [13..13] i2s master slave mode
                                               decide field. */
      __IOM unsigned int ULP_I2S_SCLK_DYN_CTRL_DISABLE_b : 1; /*!< [14..14] Disable dynamic
                                                       clock gating of System clock
                                                       in I2S     */
      __IOM unsigned int RESERVED1 : 1;                       /*!< [15..15] reserved1                */
      __IOM unsigned int ULP_I2S_LOOP_BACK_MODE_b : 1;        /*!< [16..16] Enables loop
                                                       back mode in I2S. */
      __IOM unsigned int ULP_I2S_PCLK_DYN_CTRL_DISABLE_b : 1; /*!< [17..17] Disable dynamic
                                                   clock gating of APB clock in
                                                   I2S        */
      __IOM unsigned int ULP_I2S_PCLK_EN_b : 1;               /*!< [18..18] Static clock enable
                                                   for APB clock in I2S    */
      __IOM unsigned int RESERVED2 : 13;                      /*!< [31..19] reserved2           */
    } ULP_I2S_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int ULP_UART_CLK_GEN_REG; /*!< (@ 0x00000020) ulp uart clock
                                             generation register. */

    struct {
      __IOM unsigned int ULP_UART_FRAC_CLK_SEL_b : 1; /*!< [0..0] ulp uart clk selection,if bit
                                           is set(1) then fractional divider
                                           output is  selected else swallow
                                           divider output is selected */
      __IOM unsigned int ULP_UART_CLK_SEL : 4;        /*!< [4..1] ulp uart clock select. */
      __IOM unsigned int ULP_UART_CLKDIV_FACTOR : 3;  /*!< [7..5] ulp uart clock
                                                     division factor */
      __IOM unsigned int RESERVED1 : 24;              /*!< [31..8] reserved1             */
    } ULP_UART_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int M4LP_CTRL_REG; /*!< (@ 0x00000024) m4 ulp control register */

    struct {
      __IOM unsigned int RESERVED0 : 2;                          /*!< [1..0] reserved0                */
      __IOM unsigned int ULP_M4_CORE_CLK_ENABLE_b : 1;           /*!< [2..2] Static clock
                                                      enable m4 core in ULP
                                                      mode,if bit is set(1)
                                                      then clock enable else
                                                      clock is disable */
      __IOM unsigned int ULP_MEM_CLK_ULP_ENABLE_b : 1;           /*!< [3..3] Static clock enable for M4
                                           memories in ULP mode,if bit
                                           is set(1) then clock enable else
                                           dynamic control */
      __IOM unsigned int ULP_MEM_CLK_ULP_DYN_CTRL_DISABLE_b : 1; /*!< [4..4] Disable the
                                                      dynamic clock gating for M4
                                                      memories in ULP mode,if bit
                                                      is set(1) then  dynamic
                                                      control disabled else
                                                        dynamic control enabled.
                                                    */
      __IOM unsigned int RESERVED1 : 27;                         /*!< [31..5] reserved1              */
    } M4LP_CTRL_REG_b;
  };

  union {
    __IOM unsigned int CLOCK_STAUS_REG; /*!< (@ 0x00000028) read clock status register */

    struct {
      __IM unsigned int CLOCK_SWITCHED_UART_CLK_b : 1;         /*!< [0..0] status of clock mux for
                                            uart,if bit is set(1) then clock is
                                            switched,else bit is zero then clock
                                            not switched.                     */
      __IM unsigned int CLOCK_SWITCHED_I2S_CLK_b : 1;          /*!< [1..1] Status of clock mux for
                                           i2s,if bit is set(1) then clock is
                                           switched,else bit is zero then clock
                                           not switched.                     */
      __IM unsigned int CLOCK_SWITCHED_CORTEX_SLEEP_CLK_b : 1; /*!< [2..2] Status of clock mux
                                                    for m4 sleep clk,if bit is
                                                    set(1) then clock is
                                                    switched,else bit is zero
                                                    then clock not switched. */
      __IM unsigned int CLOCK_SWITCHED_PROC_CLK_b : 1;         /*!< [3..3] Status of clock mux for
                                            pclk,if bit is set(1) then clock is
                                            switched,else bit is zero then clock
                                            not switched.                     */
      __IM unsigned int CLOCK_SWITCHED_I2C_b : 1;              /*!< [4..4] Status of clock mux for i2c,if
                                       bit is set(1) then clock
                                       is switched,else bit is zero then clock
                                       not switched.                     */
      __IM unsigned int CLOCK_SWITCHED_SSI_b : 1;              /*!< [5..5] Status of clock mux for ssi,if
                                       bit is set(1) then clock
                                       is switched,else bit is zero then clock
                                       not switched.                     */
      __IM unsigned int CLOCK_SWITCHED_VAD_b : 1;              /*!< [6..6] Status of clock mux for vad,if
                                       bit is set(1) then clock
                                       is switched,else bit is zero then clock
                                       not switched.                     */
      __IM unsigned int CLOCK_SWITCHED_AUXADC_b : 1;           /*!< [7..7] Status of clock mux for aux
                                           adc dac clock,if bit is set(1) then
                                           clock is switched,else bit is zero
                                           then clock not switched. */
      __IM unsigned int CLOCK_SWITCHED_TIMER_b : 1;            /*!< [8..8] Status of clock mux for async
                                          timers,if bit is set(1) then clock is
                                          switched,else bit is zero then clock
                                          not switched. */
      __IM unsigned int CLOCK_SWITCHED_TOUCH_SENSOR_b : 1;     /*!< [9..9] Status of clock mux for
                                                 touch sensor,if bit is set(1)
                                                        then clock is
                                                 switched,else bit is zero then
                                                 clock not switched. */
      __IM unsigned int CLOCK_SWITCHED_FCLK_VAD_b : 1;         /*!< [10..10] Status of clock mux for
                                             vad fast clock,if bit is set(1) then
                                             clock is switched,else bit is zero
                                             then clock not switched. */
      __IM unsigned int CLOCK_SWITCHED_SCLK_VAD_b : 1;         /*!< [11..11] Status of clock mux for
                                             vad slow clock,if bit is set(1) then
                                             clock is switched,else bit is zero
                                             then clock not switched. */
      __IM unsigned int CLOCK_SWITCHED_SYSTICK_b : 1;          /*!< [12..12] Status of clock mux for
                                            systick clock,if bit is set(1) then
                                            clock is switched,else bit is zero
                                            then clock not switched. */
      __IOM unsigned int RESERVED1 : 19;                       /*!< [31..13] reserved1    */
    } CLOCK_STAUS_REG_b;
  };

  union {
    __IOM unsigned int ULP_TOUCH_CLK_GEN_REG; /*!< (@ 0x0000002C) ULP touch clock
                                              generation register */

    struct {
      __IOM unsigned int ULP_TOUCH_CLK_EN_b : 1;      /*!< [0..0] ulp touch clk enable,if bit is
                                      set(1) then enable,else bit is zero then
                                      disable. */
      __IOM unsigned int ULP_TOUCH_CLK_SEL : 4;       /*!< [4..1] ulp touch clock select. */
      __IOM unsigned int ULP_TOUCH_CLKDIV_FACTOR : 8; /*!< [12..5] ulp touch clock
                                                      division factor. */
      __IOM unsigned int RESERVED1 : 19;              /*!< [31..13] reserved1              */
    } ULP_TOUCH_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int ULP_TIMER_CLK_GEN_REG; /*!< (@ 0x00000030) ULP clock
                                              generation for timer */

    struct {
      __IOM unsigned int RESERVED1 : 1;           /*!< [0..0] reserved1 */
      __IOM unsigned int ULP_TIMER_CLK_SEL : 4;   /*!< [4..1] ulp timer clock select.    */
      __IOM unsigned int RESERVED2 : 8;           /*!< [12..5] reserved2 */
      __IOM unsigned int ULP_TIMER_IN_SYNC_b : 1; /*!< [13..13] Ulp timer in synchronous mode
                                         to ULPSS pclk                      */
      __IOM unsigned int RESERVED3 : 18;          /*!< [31..14] reserved3 */
    } ULP_TIMER_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int ULP_AUXADC_CLK_GEN_REG; /*!< (@ 0x00000034) ULP AUX clock
                                               generation register */

    struct {
      __IOM unsigned int ULP_AUX_CLK_EN_b : 1; /*!< [0..0] ulp aux clk enable,if bit is one
                                    then clock enable else
                                           bit is zero then clock disable. */
      __IOM unsigned int ULP_AUX_CLK_SEL : 4;  /*!< [4..1] ulp aux clock select. */
      __IOM unsigned int RESERVED1 : 27;       /*!< [31..5] reserved1      */
    } ULP_AUXADC_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int ULP_VAD_CLK_GEN_REG; /*!< (@ 0x00000038) ULP vad clock
                                            generation register */

    struct {
      __IOM unsigned int ULP_VAD_CLK_EN_b : 1;      /*!< [0..0] ulp vad clk enable ,if bit is one
                                    then clock enable else
                                           bit is zero then clock disable. */
      __IOM unsigned int ULP_VAD_CLK_SEL : 3;       /*!< [3..1] ulp vad clock select. */
      __IOM unsigned int ULP_VAD_FCLK_EN : 1;       /*!< [4..4] Enables Fast clock to VAD. */
      __IOM unsigned int ULP_VAD_FCLK_SEL : 4;      /*!< [8..5] ulp vad Fast clock select. */
      __IOM unsigned int ULP_VAD_CLKDIV_FACTOR : 8; /*!< [16..9] ulp vad clock
                                                    division factor */
      __IOM unsigned int RESERVED1 : 15;            /*!< [31..17] reserved1            */
    } ULP_VAD_CLK_GEN_REG_b;
  };

  union {
    __IOM unsigned int BYPASS_I2S_CLK_REG; /*!< (@ 0x0000003C) bypass i2s clock register */

    struct {
      __IOM unsigned int BYPASS_I2S_PLL_SEL : 1;         /*!< [0..0] Bypass_I2S PLL clock,if
                                                 bit is one bypass clock is used
                                                      else bit is zero then I2S
                                                 Clock is used. */
      __IOM unsigned int BYPASS_I2S_PLL_CLK_CLN_ON : 1;  /*!< [1..1] I2S PLL Bypass
                                                         clock cleaner ON  */
      __IOM unsigned int BYPASS_I2S_PLL_CLK_CLN_OFF : 1; /*!< [2..2] I2S PLL Bypass
                                                         clock cleaner OFF */
      __IOM unsigned int RESERVED3 : 29;                 /*!< [31..3] reserved3                 */
    } BYPASS_I2S_CLK_REG_b;
  };
  __IM unsigned int RESERVED1;

  union {
    __IOM unsigned int ULP_RM_RME_REG; /*!< (@ 0x00000044) ulp rm rem register */

    struct {
      __IOM unsigned int ULP_MEM_RME_b : 1;      /*!< [0..0] RM enable signal for memories internal
                                 tp peripherals. This needs to be programmed when
                                 the peripheral memories are not active. */
      __IOM unsigned int ULP_MEM_RM : 2;         /*!< [2..1] RM ports for memories internal to
                                       peripheral. This needs          to be programmed when the
                                       peripheral memories are not active.             */
      __IM unsigned int RESERVED1 : 1;           /*!< [3..3] reserved1 */
      __IOM unsigned int ULP_MEM_RME_SRAM_b : 1; /*!< [4..4] RM enable signal for
                                                 sram memories. This needs to be
                                                      programmed when the SRAM is
                                                 not active. */
      __IOM unsigned int ULP_MEM_RM_SRAM : 2;    /*!< [6..5] RM ports for sram memories. This
                                   needs to be programmed when the SRAM  is not
                                   active */
      __IOM unsigned int RESERVED2 : 25;         /*!< [31..7] reserved2 */
    } ULP_RM_RME_REG_b;
  };

  union {
    __IOM unsigned int ULP_CLK_ENABLE_REG; /*!< (@ 0x00000048) ulp clock enable register. */

    struct {
      __IOM unsigned int ULP_32KHZ_RO_CLK_EN_PROG_b : 1;   /*!< [0..0] Static Clock enable to
                                           iPMU for 32KHz RO Clock,if bit is
                                           one(set) then clock enable else not
                                           enable.                            */
      __IOM unsigned int ULP_32KHZ_RC_CLK_EN_PROG_b : 1;   /*!< [1..1] Static Clock enable to
                                           iPMU for 32KHz RC Clock,if bit is
                                           one(set) then clock enable else not
                                           enable.                            */
      __IOM unsigned int ULP_32KHZ_XTAL_CLK_EN_PROG_b : 1; /*!< [2..2] Static Clock enable to
                                              iPMU for 32KHz XTAL Clock,if bit
                                              is one(set) then clock enable else
                                              not enable. */
      __IOM unsigned int ULP_DOUBLER_CLK_EN_PROG_b : 1;    /*!< [3..3] Static Clock
                                                      enable to iPMU for Doubler
                                                      Clock,if bit is one(set)
                                                      then clock enable else not
                                                      enable. */
      __IOM unsigned int ULP_20MHZ_RO_CLK_EN_PROG_b : 1;   /*!< [4..4] Static Clock enable to
                                           iPMU for 20MHz RO clock,if bit is
                                           one(set) then clock enable else not
                                           enable.                            */
      __IOM unsigned int ULP_MHZ_RC_CLK_EN_PROG_b : 1;     /*!< [5..5] Static Clock enable to
                                           iPMU for MHz RC Clock,if bit is
                                           one(set) then clock enable else not
                                           enable.                            */
      __IOM unsigned int SOC_CLK_EN_PROG_b : 1;            /*!< [6..6] Static Clock enable to iPMU for
                                  PLL-500 Clock,if bit is one(set) then clock
                                  enable else not enable. */
      __IOM unsigned int I2S_PLLCLK_EN_PROG_b : 1;         /*!< [7..7] Static clock enable
                                                to iPMU for I2S-PLL Clock,if bit
                                                is one(set) then clock enable
                                                else not enable. */
      __IOM unsigned int REF_CLK_EN_IPS_PROG_b : 1;        /*!< [8..8] Static Clock enable to iPMU for
                                         REF Clock,if bit is one(set)
                                                      then clock enable else not
                                         enable. */
      __IOM unsigned int RESERVED1 : 23;                   /*!< [31..9] reserved1 */
    } ULP_CLK_ENABLE_REG_b;
  };
  __IM unsigned int RESERVED2;

  union {
    __IOM unsigned int SYSTICK_CLK_GEN_REG; /*!< (@ 0x00000050) sys tick clock
                                            generation register. */

    struct {
      __IOM unsigned int SYSTICK_CLK_EN_b : 1;      /*!< [0..0] sys tick clock enable ,if bit is
                                    one(set) then clock enable else not enable.
                                  */
      __IOM unsigned int SYSTICK_CLK_SEL : 4;       /*!< [4..1] sys tick clock select */
      __IOM unsigned int SYSTICK_CLKDIV_FACTOR : 8; /*!< [12..5] sys tick clock
                                                    division factor */
      __IOM unsigned int RESERVED1 : 19;            /*!< [31..13] reserved1            */
    } SYSTICK_CLK_GEN_REG_b;
  };
  __IM unsigned int RESERVED3[3];
  __IOM ULPCLK_ULP_SOC_GPIO_MODE_REG_Type ULP_SOC_GPIO_MODE_REG[16]; /*!< (@ 0x00000060) [0..15] */

  union {
    __IOM unsigned int ULP_DYN_CLK_CTRL_DISABLE; /*!< (@ 0x000000A0) this register used for ULP
                                      dynamic clock control disable. */

    struct {
      __IOM unsigned int I2C_PCLK_DYN_CTRL_DISABLE_b : 1;           /*!< [0..0] Dynamic clock control
                                            disable for APB interface in i2c
                                            module,if bit is one(set)  then
                                            dynamic control disabled
                                            else bit is zero then Dynamic control
                                            enabled. */
      __IOM unsigned int I2S_CLK_DYN_CTRL_DISABLE_b : 1;            /*!< [1..1] Dynamic clock control
                                           disable for i2s module,if bit is
                                           one(set)  then dynamic control
                                           disabled else bit is zero
                                           then Dynamic control enabled. */
      __IOM unsigned int SSI_MST_PCLK_DYN_CTRL_DISABLE_b : 1;       /*!< [2..2] Dynamic clock control
                                                   disable for pclk ssi module,if
                                                        bit is one(set)  then
                                                   dynamic control disabled else
                                                   bit is zero then Dynamic
                                                   control enabled. */
      __IOM unsigned int SSI_MST_SCLK_DYN_CTRL_DISABLE_b : 1;       /*!< [3..3] Dynamic clock control
                                                   disable for ssi module,if bit
                                                   is one(set)  then dynamic
                                                   control disabled else bit is
                                                   zero then Dynamic control
                                                   enabled. */
      __IOM unsigned int UART_CLK_DYN_CTRL_DISABLE_b : 1;           /*!< [4..4] Dynamic clock control
                                               disable for pclk uart module ,if
                                                        bit is one(set)  then
                                               dynamic control disabled else bit
                                                        is zero then Dynamic
                                               control enabled. */
      __IOM unsigned int UART_SCLK_DYN_CTRL_DISABLE_b : 1;          /*!< [5..5] Dynamic clock
                                                      control disable for uart
                                                      module,if bit is one(set)
                                                      then dynamic control
                                                      disabled else bit is zero
                                                      then Dynamic control
                                                      enabled. */
      __IOM unsigned int TIMER_PCLK_DYN_CTRL_DISABLE_b : 1;         /*!< [6..6] Dynamic clock control
                                                 disable for timer pclk module,if
                                                        bit is one(set)  then
                                                 dynamic control disabled else
                                                 bit
                                                        is zero then Dynamic
                                                 control enabled. */
      __IOM unsigned int TIMER_SCLK_DYN_CTRL_DISABLE_b : 1;         /*!< [7..7] Dynamic clock control
                                                 disable for timer sclk module,if
                                                        bit is one(set)  then
                                                 dynamic control disabled else
                                                 bit
                                                        is zero then Dynamic
                                                 control enabled. */
      __IOM unsigned int REG_ACCESS_SPI_CLK_DYN_CTRL_DISABLE_b : 1; /*!< [8..8] Dynamic clock
                                                        control disable for reg
                                                        access spi module,if bit
                                                        is one(set)  then dynamic
                                                        control disabled else bit
                                                        is zero then Dynamic
                                                        control enabled. */
      __IOM unsigned int FIM_CLK_DYN_CTRL_DISABLE_b : 1;            /*!< [9..9] Dynamic clock control
                                           disable for fim module,if bit is
                                           one(set)  then dynamic control
                                           disabled else bit is zero
                                           then Dynamic control enabled. */
      __IOM unsigned int VAD_CLK_DYN_CTRL_DISABLE_b : 1;            /*!< [10..10] Dynamic clock
                                                      control disable for vad
                                                      module,if bit is one(set)
                                                      then dynamic control
                                                      disabled else bit is zero
                                                      then Dynamic control
                                                      enabled. */
      __IOM unsigned int AUX_PCLK_EN_b : 1;                         /*!< [11..11] Static Enable for Aux adc pclk. */
      __IOM unsigned int AUX_CLK_EN_b : 1;                          /*!< [12..12] Static Enable for Aux adc clk. */
      __IOM unsigned int AUX_MEM_EN_b : 1;                          /*!< [13..13] Static Enable for Aux adc mem. */
      __IOM unsigned int AUX_PCLK_DYN_CTRL_DISABLE_b : 1;           /*!< [14..14] Dynamic clock control
                                           disable for aux adc module,if bit is
                                           one(set)  then dynamic control
                                           disabled else bit
                                           is zero then Dynamic control enabled.
                                         */
      __IOM unsigned int AUX_CLK_DYN_CTRL_DISABLE_b : 1;            /*!< [15..15] Dynamic clock control
                                          disable for aux adc module,if bit is
                                          one(set)  then dynamic control disabled
                                          else bit is zero then Dynamic control
                                          enabled. */
      __IOM unsigned int AUX_CLK_MEM_DYN_CTRL_DISABLE_b : 1;        /*!< [16..16] Dynamic clock
                                                  control disable for aux adc
                                                  mem,if bit is one(set)  then
                                                  dynamic control disabled else
                                                  bit is zero then Dynamic
                                                  control enabled. */
      __IOM unsigned int UDMA_CLK_ENABLE_b : 1;                     /*!< [17..17] Static Enable for UDMA. */
      __IOM unsigned int IR_CLK_ENABLE_b : 1;                       /*!< [18..18] Static Enable for IR. */
      __IOM
      unsigned int IR_CLK_DYN_CTRL_DISABLE_b : 1; /*!< [19..19] Dynamic clock control
                                               disable for ir module ,if bit is
                                               one(set)  then dynamic control
                                               disabled else bit is zero then
                                               Dynamic control enabled. */
      __IOM unsigned int RESERVED1 : 12;          /*!< [31..20] reserved1         */
    } ULP_DYN_CLK_CTRL_DISABLE_b;
  };

  union {
    __IOM unsigned int SLP_SENSOR_CLK_REG; /*!< (@ 0x000000A4) this register used
                                           for SLP sensor clock register. */

    struct {
      __IOM unsigned int DIVISON_FACTOR : 8; /*!< [7..0] Division factor for apb interface
                                  clock to sleep sensor subsystem. */
      __IOM unsigned int ENABLE_b : 1;       /*!< [8..8] Enable for APB clock to SLPSS */
      __IOM unsigned int RESERVED1 : 23;     /*!< [31..9] reserved1 */
    } SLP_SENSOR_CLK_REG_b;
  };
} ULPCLK_Type; /*!< Size = 168 (0xa8) */

/* ===========================================================================================================================
  */
/* ================                                            FIM
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief FIM support fixed point Multiplications implemented through
  * programmable shifting. (FIM)
  */

typedef struct { /*!< (@ 0x24070000) FIM Structure */

  union {
    __IOM unsigned int FIM_MODE_INTERRUPT; /*!< (@ 0x00000000) Configuration for FIM Operation
                                Mode and Interrupt Control */

    struct {
      __IOM unsigned int LATCH_MODE : 1; /*!< [0..0] Enable latch mode */
      __IOM unsigned int OPER_MODE : 8;  /*!< [8..1] Indicates the Mode of Operation
                                         to be performed.                   */
      __IM unsigned int RESERVED1 : 1;   /*!< [9..9] reserved1   */
      __OM unsigned int INTR_CLEAR : 1;  /*!< [10..10] Writing 1 to this bit clears
                                         the interrupt                       */
      __IM unsigned int RESERVED2 : 21;  /*!< [31..11] reserved2  */
    } FIM_MODE_INTERRUPT_b;
  };

  union {
    __IOM unsigned int FIM_INP1_ADDR; /*!< (@ 0x00000004) This register used for COP
                                      input address for 0 register. */

    struct {
      __IOM unsigned int INP1_ADDR : 32; /*!< [31..0] Indicates the Start Address of
                                         1st Input Data for FIM Operations */
    } FIM_INP1_ADDR_b;
  };

  union {
    __IOM unsigned int FIM_INP2_ADDR; /*!< (@ 0x00000008) This register used for COP
                                      input address for 1 register */

    struct {
      __IOM unsigned int INP2_ADDR : 32; /*!< [31..0] Indicates the Start Address of
                                         2nd Input Data for FIM Operations */
    } FIM_INP2_ADDR_b;
  };

  union {
    __IOM unsigned int FIM_OUT_ADDR; /*!< (@ 0x0000000C) Memory Offset Address for
                                     Output from FIM Operations       */

    struct {
      __IOM unsigned int OUT_ADDR : 32; /*!< [31..0] Indicates the Start Address of
                                        Output Data for FIM Operations     */
    } FIM_OUT_ADDR_b;
  };

  union {
    __IOM unsigned int FIM_SCALAR_POLE_DATA1; /*!< (@ 0x00000010) Indicates the Input Scalar
                                   Data for Scalar Operations indicates the
                                   feedback coefficient for IIR Operations */

    struct {
      __IOM unsigned int SCALAR_POLE_DATA1 : 32; /*!< [31..0] Pole 0/Scalar Value */
    } FIM_SCALAR_POLE_DATA1_b;
  };

  union {
    __IOM unsigned int FIM_POLE_DATA2; /*!< (@ 0x00000014) Feedback coefficient for
                                       IIR filter operation              */

    struct {
      __IOM unsigned int POLE_DATA2 : 32; /*!< [31..0] Indicates the feedback
                                          coefficient for IIR Operations */
    } FIM_POLE_DATA2_b;
  };

  union {
    __IOM unsigned int FIM_SAT_SHIFT; /*!< (@ 0x00000018) Configuration for precision of Output
                           Data for FIM Operations */

    struct {
      __IOM unsigned int SAT_VAL : 5;   /*!< [4..0] Indicates the number of MSB's to
                                        be saturated for Output   Data   */
      __IOM unsigned int TRUNCATE : 5;  /*!< [9..5] Truncate  */
      __IOM unsigned int SHIFT_VAL : 6; /*!< [15..10] Indicates the number of bits
                                        to be right-shifted for Output Data */
      __IOM unsigned int ROUND : 2;     /*!< [17..16] Round     */
      __IOM unsigned int SAT_EN : 1;    /*!< [18..18] Saturation enable bit    */
      __IM unsigned int RESERVED2 : 13; /*!< [31..19] reserved2 */
    } FIM_SAT_SHIFT_b;
  };

  union {
    __IOM unsigned int FIM_CONFIG_REG1; /*!< (@ 0x0000001C) Configuration Register
                                        for FIM Operations.                 */

    struct {
      __IOM unsigned int MAT_LEN : 6;   /*!< [5..0] Indicates the number of columns in 1st input
                           for Matrix Multiplication. This is same as number of
                           rows in 2nd input for Matrix Multiplication. */
      __IOM unsigned int INP1_LEN : 10; /*!< [15..6] Indicates the length of 1st input for FIM
                             Operations other than filtering (FIR, IIR) and
                             Interpolation                         */
      __IOM unsigned int INP2_LEN : 10; /*!< [25..16] Indicates the length of 2nd input for FIM
                             Operations other than filtering (FIR, IIR) and
                             Interpolation.                        */
      __IOM unsigned int DECIM_FAC : 6; /*!< [31..26] Decimation Factor */
    } FIM_CONFIG_REG1_b;
  };

  union {
    __IOM unsigned int FIM_CONFIG_REG2; /*!< (@ 0x00000020) Configuration Register
                                        for FIM Operations                  */

    struct {
      __OM unsigned int START_OPER : 1;         /*!< [0..0] Start trigger for the FIM operations,this
                              is reset upon write register */
      __IOM unsigned int INSTR_BUFF_ENABLE : 1; /*!< [1..1] Instruction buffer enable */
      __IM unsigned int RES : 6;                /*!< [7..2] reserved5     */
      __IOM unsigned int CPLX_FLAG : 2;         /*!< [9..8] Complex Flag,not valid in matrix mode */
      __IOM unsigned int COL_M2 : 6;            /*!< [15..10] Indicates the number of columns
                                     in 2nd input for Matrix Multiplication */
      __IOM unsigned int ROW_M1 : 6;            /*!< [21..16] Indicates the number of rows in
                                     1st input for Matrix Multiplication */
      __IOM unsigned int INTRP_FAC : 6;         /*!< [27..22] Indicates the Interpolation Factor */
      __IM unsigned int RESERVED1 : 4;          /*!< [31..28] reserved1 */
    } FIM_CONFIG_REG2_b;
  };
} FIM_Type; /*!< Size = 36 (0x24) */

/* ===========================================================================================================================
  */
/* ================                                          NWP_FSM
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief NWP FSM one register Structure (NWP_FSM)
  */

typedef struct { /*!< (@ 0x41300110) NWP_FSM Structure */

  union {
    __IOM unsigned int TASS_REF_CLOCK_SELECT; /*!< (@ 0x00000000) TASS REF CLOCK SELECT */

    struct {
      __IOM unsigned int M4SS_REF_CLK_SEL_NWP : 3;         /*!< [2..0] M4SS REF CLK SEL NWP */
      __IOM unsigned int RESERVED1 : 1;                    /*!< [3..3] reserved1 */
      __IOM unsigned int ULPSS_REF_CLK_SEL_NWP : 3;        /*!< [6..4] ULPSS REF CLK SEL NWP */
      __IOM unsigned int RESERVED2 : 9;                    /*!< [15..7] reserved2  */
      __IOM unsigned int TASS_REF_CLK_SEL_NWP : 3;         /*!< [18..16] TASS REF CLK SEL NWP */
      __IOM unsigned int RESERVED3 : 3;                    /*!< [21..19] reserved3 */
      __IOM unsigned int TASS_REF_CLK_CLEANER_OFF_NWP : 1; /*!< [22..22] TASS REF CLK CLEANER
                                                OFF NWP */
      __IOM unsigned int TASS_REF_CLK_CLEANER_ON_NWP : 1;  /*!< [23..23] TASS REF CLK
                                                          CLEANER ON NWP */
      __IOM unsigned int RESERVED4 : 8;                    /*!< [31..24] reserved4                   */
    } TASS_REF_CLOCK_SELECT_b;
  };
} NWP_FSM_Type; /*!< Size = 4 (0x4) */

/* ===========================================================================================================================
  */
/* ================                                           OPAMP
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief The opamps top consists of 3 general purpose Operational Amplifiers
  * (OPAMP) offering rail-to-rail inputs and outputs (OPAMP)
  */

typedef struct { /*!< (@ 0x24043A14) OPAMP Structure */

  union {
    __IOM unsigned int OPAMP_1; /*!< (@ 0x00000000) Programs opamp1 */

    struct {
      __IOM unsigned int OPAMP1_ENABLE : 1;         /*!< [0..0] To enable opamp 1 */
      __IOM unsigned int OPAMP1_LP_MODE : 1;        /*!< [1..1] Enable or disable low power mode */
      __IOM unsigned int OPAMP1_R1_SEL : 2;         /*!< [3..2] Programmability to select
                                            resister bank R1 */
      __IOM unsigned int OPAMP1_R2_SEL : 3;         /*!< [6..4] Programmability to select
                                            resister bank R2 */
      __IOM unsigned int OPAMP1_EN_RES_BANK : 1;    /*!< [7..7] enables the resistor bank 1 for
                                      enable 0 for disable               */
      __IOM unsigned int OPAMP1_RES_MUX_SEL : 3;    /*!< [10..8] selecting input for
                                                 registor bank */
      __IOM unsigned int OPAMP1_RES_TO_OUT_VDD : 1; /*!< [11..11] connect resistor bank to out
                                         or vdd i.e 0-out and 1-vdd          */
      __IOM unsigned int OPAMP1_OUT_MUX_EN : 1;     /*!< [12..12] out mux enable */
      __IOM unsigned int OPAMP1_INN_SEL : 3;        /*!< [15..13] selecting -ve input of opamp */
      __IOM unsigned int OPAMP1_INP_SEL : 4;        /*!< [19..16] selecting +ve input of opamp */
      __IOM unsigned int OPAMP1_OUT_MUX_SEL : 1;    /*!< [20..20] to connect opamp1
                                                 output to pad */
      __IOM unsigned int MEMS_RES_BANK_EN : 1;      /*!< [21..21] enables mems res bank        */
      __IOM unsigned int VREF_MUX_EN : 4;           /*!< [25..22] vref mux enable  */
      __IOM unsigned int MUX_EN : 1;                /*!< [26..26] Mux Enable       */
      __IOM unsigned int VREF_MUX_SEL : 4;          /*!< [30..27] vref mux enable */
      __IOM unsigned int OPAMP1_DYN_EN : 1;         /*!< [31..31] dynamic enable for opamp1 */
    } OPAMP_1_b;
  };

  union {
    __IOM unsigned int OPAMP_2; /*!< (@ 0x00000004) Programs opamp2 */

    struct {
      __IOM unsigned int OPAMP2_ENABLE : 1;         /*!< [0..0] enables the opamp2 */
      __IOM unsigned int OPAMP2_LP_MODE : 1;        /*!< [1..1] select the power mode 0-normal mode
                                  and 1-low power mode           */
      __IOM unsigned int OPAMP2_R1_SEL : 2;         /*!< [3..2] Programmability to select
                                            resister bank R1 */
      __IOM unsigned int OPAMP2_R2_SEL : 3;         /*!< [6..4] Programmability to select
                                            resister bank R2 */
      __IOM unsigned int OPAMP2_EN_RES_BANK : 1;    /*!< [7..7] enables the resistor bank 1 for
                                      enable 0 for disable               */
      __IOM unsigned int OPAMP2_RES_MUX_SEL : 3;    /*!< [10..8] selecting input for
                                                 registor bank */
      __IOM unsigned int OPAMP2_RES_TO_OUT_VDD : 2; /*!< [12..11] connect resistor bank to out
                                         or vdd or gnd or DAC i.e
                                           0-out and 1-vdd 2-DAC 3-gnd */
      __IOM unsigned int OPAMP2_OUT_MUX_EN : 1;     /*!< [13..13] out mux enable */
      __IOM unsigned int OPAMP2_INN_SEL : 2;        /*!< [15..14] selecting -ve input of opamp */
      __IOM unsigned int OPAMP2_INP_SEL : 3;        /*!< [18..16] selecting +ve input of opamp2 */
      __IOM unsigned int OPAMP2_DYN_EN : 1;         /*!< [19..19] dynamic enable for opamp2         */
      __IOM unsigned int RESERVED1 : 12;            /*!< [31..20] res */
    } OPAMP_2_b;
  };

  union {
    __IOM unsigned int OPAMP_3; /*!< (@ 0x00000008) Programs opamp3 */

    struct {
      __IOM unsigned int OPAMP3_ENABLE : 1;         /*!< [0..0] enables the opamp3 1 for
                                            enable 0 for disable */
      __IOM unsigned int OPAMP3_LP_MODE : 1;        /*!< [1..1] select the power mode 0-normal mode
                                  and 1-low power mode           */
      __IOM unsigned int OPAMP3_R1_SEL : 2;         /*!< [3..2] Programmability to select
                                            resister bank R1 */
      __IOM unsigned int OPAMP3_R2_SEL : 3;         /*!< [6..4] Programmability to select
                                            resister bank R2 */
      __IOM unsigned int OPAMP3_EN_RES_BANK : 1;    /*!< [7..7] enables the resistor bank 1 for
                                      enable 0 for disable               */
      __IOM unsigned int OPAMP3_RES_MUX_SEL : 3;    /*!< [10..8] selecting input for
                                                 registor bank */
      __IOM unsigned int OPAMP3_RES_TO_OUT_VDD : 1; /*!< [11..11] connect resistor bank to out
                                         or vdd i.e 0-out and 1-vdd          */
      __IOM unsigned int OPAMP3_OUT_MUX_EN : 1;     /*!< [12..12] out mux enable */
      __IOM unsigned int OPAMP3_INN_SEL : 2;        /*!< [14..13] selecting -ve input of opamp */
      __IOM unsigned int OPAMP3_INP_SEL : 3;        /*!< [17..15] selecting +ve input of opamp */
      __IOM unsigned int OPAMP3_DYN_EN : 1;         /*!< [18..18] dynamic enable for opamp2         */
      __IOM unsigned int RESERVED1 : 13;            /*!< [31..19] res */
    } OPAMP_3_b;
  };
} OPAMP_Type; /*!< Size = 12 (0xc) */

/* ===========================================================================================================================
  */
/* ================                                     AUX_ADC_DAC_COMP
  * ================ */
/* ===========================================================================================================================
  */

/**
   * @brief The ADC-DAC Controller works on a ADC with a resolution of 12bits at
   5Mega sample per second when ADC reference Voltage is greater than 2.8v or
   5Mega sample per second when ADC reference Voltage is less than 2.8v.
   (AUX_ADC_DAC_COMP)
   */

typedef struct { /*!< (@ 0x24043800) AUX_ADC_DAC_COMP Structure */

  union {
    __IOM unsigned int AUXDAC_CTRL_1; /*!< (@ 0x00000000) Control register1 for DAC */

    struct {
      __IOM unsigned int ENDAC_FIFO_CONFIG : 1;   /*!< [0..0] This bit activates the
                                                DAC path in Aux ADC-DAC
                                                controller. Data samples will be
                                                played on  DAC only when this bit
                                                      is set. */
      __IOM unsigned int DAC_STATIC_MODE : 1;     /*!< [1..1] This bit is used to select
                                              non-FIFO mode in DAC. */
      __IOM unsigned int DAC_FIFO_FLUSH : 1;      /*!< [2..2] This bit is used to flush
                                              the DAC FIFO.  */
      __IOM unsigned int DAC_FIFO_THRESHOLD : 3;  /*!< [5..3] These bits control the DAC FIFO
                                      threshold. When used by DMA, this will act
                                      as almost full threshold. For NWP, it acts
                                      as almost empty threshold */
      __IOM unsigned int DAC_ENABLE_F : 1;        /*!< [6..6] This bit is used to enable
                                            AUX DAC controller ,valid  only when
                                            DAC enable is happpen  */
      __IOM unsigned int DAC_WORD_MODE : 1;       /*!< [7..7] This bit is used to select
                                            the data size valid on the APB */
      __IOM unsigned int AUX_DAC_MAC_MUX_SEL : 1; /*!< [8..8] It is recommended to
                                                  write these bits to 0 */

      __IOM unsigned int DAC_FIFO_AEMPTY_THRESHOLD : 4; /*!< [12..9] It is recommended to write
                                             these bits to 0 */

      __IOM unsigned int DAC_FIFO_AFULL_THRESHOLD : 4; /*!< [16..13] It is recommended to write
                                            these bits to 0 */

      __IOM unsigned int RESERVED1 : 15; /*!< [31..9] Reserved1 */
    } AUXDAC_CTRL_1_b;
  };

  union {
    __IOM unsigned int AUXADC_CTRL_1; /*!< (@ 0x00000004) Control register1 for ADC */

    struct {
      __IOM unsigned int ADC_ENABLE : 1;               /*!< [0..0] This bits activates the ADC
                                         path in Aux ADC-DAC controller. */
      __IOM unsigned int ADC_STATIC_MODE : 1;          /*!< [1..1] This bit is used to select
                                              non-FIFO mode in ADC. */
      __IOM unsigned int ADC_FIFO_FLUSH : 1;           /*!< [2..2] This bit is used to flush
                                              the ADC FIFO  */
      __IOM unsigned int RESERVED1 : 3;                /*!< [5..3] RESERVED1       */
      __IOM unsigned int ADC_MULTIPLE_CHAN_ACTIVE : 1; /*!< [6..6] This bit is used to control
                                            the auxadc sel signal going
                                            to the Aux ADC.  */
      __IOM unsigned int ADC_CH_SEL_MSB : 2;           /*!< [8..7] It is recommended to write
                                             these bits to 0 */
      __IOM unsigned int BYPASS_NOISE_AVG : 1;         /*!< [9..9] ADC in Bypass noise avg mode. */
      __IOM unsigned int EN_ADC_CLK : 1;               /*!< [10..10] Enable AUX ADC Divider output clock */
      __IOM unsigned int ENDIFF : 1;                   /*!< [11..11] Control to the Aux ADC */
      __IOM unsigned int ADC_CH_SEL_LS : 2;            /*!< [13..12] Aux ADC channel number from which the
                                 data has to be sampled This is valid only when
                                 adc multiple channel active is zero. When
                                 channel number is greater than three, upper bits
                                 should also be programmed ADC CHANNEL SELECT MS
                                 to bits in this register */
      __IOM unsigned int ADC_WORD_MODE : 1;            /*!< [14..14] This bit is used to select the read
                                 data size valid on the APB */
      __IOM unsigned int AUX_ADC_MAC_MUX_SEK : 1;      /*!< [15..15] When set, AUX-ADC control is
                                       handed over to Aux ADC-ADC controller. By
                                       default, AUX-ADC is under the control of
                                           baseband. */
      __IOM
      unsigned int OVERRUN_DMA : 1;            /*!< [16..16] overrun bit in dma mode to
                                        enable the over-writing of buffer from
                                        beginning when buffer is full. */
      __IOM unsigned int RESERVED2 : 4;        /*!< [20..17] Reserved2 */
      __IOM unsigned int ADC_WAKE_UP_TIME : 5; /*!< [25..21] overrun bit in dma mode to enable
                                    the over-writing of buffer from beginning
                                    when buffer is wake up time (number
                                           of clock cycles) , dependant upon AUX
                                    ADC latency.                        */
      __IOM unsigned int EN_ADC_TURN_OFF : 1;  /*!< [26..26] Enable power save mode to turn off
                                   AUX ADC when sampling clock is idle and enable
                                   it before sampling event, programmed by
                                   adc_wake_up_time */
      __IOM unsigned int ADC_NUM_PHASE : 1;    /*!< [27..27] ADC number of phase */
      __IOM unsigned int RESERVED3 : 4;        /*!< [31..28] Reserved3     */
    } AUXADC_CTRL_1_b;
  };

  union {
    __IOM unsigned int AUXDAC_CLK_DIV_FAC; /*!< (@ 0x00000008) DAC clock division register */

    struct {
      __IOM unsigned int DAC_CLK_DIV_FAC : 10; /*!< [9..0] These bits control the
                                               DAC clock division factor */
      __IOM unsigned int RESERVED1 : 22;       /*!< [31..10] Reserved1       */
    } AUXDAC_CLK_DIV_FAC_b;
  };

  union {
    __IOM unsigned int AUXADC_CLK_DIV_FAC; /*!< (@ 0x0000000C) ADC clock division register */

    struct {
      __IOM unsigned int ADC_CLK_DIV_FAC : 10; /*!< [9..0] These bits control the
                                             Total-Duration of the ADC clock          */
      __IOM unsigned int RESERVED1 : 6;        /*!< [15..10] Reserved1      */
      __IOM unsigned int ADC_CLK_ON_DUR : 9;   /*!< [24..16] These bits control the
                                             On-Duration of the ADC clock */
      __IOM unsigned int RESERVED2 : 7;        /*!< [31..25] Reserved2      */
    } AUXADC_CLK_DIV_FAC_b;
  };

  union {
    __IOM unsigned int AUXDAC_DATA_REG; /*!< (@ 0x00000010) Writing to this register will fill
                             DAC FIFO for streaming Data to DAC */

    struct {
      __IOM unsigned int AUXDAC_DATA : 10; /*!< [9..0] Writing to this register will fill DAC
                                FIFO for streaming Data to DAC */
      __IOM unsigned int RESERVED1 : 22;   /*!< [31..10] Reserved1 */
    } AUXDAC_DATA_REG_b;
  };

  union {
    __IOM unsigned int AUXADC_DATA; /*!< (@ 0x00000014) AUXADC Data Read through Register. */

    struct {
      __IM unsigned int AUXADC_DATA : 12; /*!< [11..0] AUXADC Data Read through Register */
      __IM unsigned int AUXADC_CH_ID : 4; /*!< [15..12] AUXADC Channel ID */
      __IOM unsigned int RESERVED1 : 16;  /*!< [31..16] Reserved1  */
    } AUXADC_DATA_b;
  };

  union {
    __IOM unsigned int ADC_DET_THR_CTRL_0; /*!< (@ 0x00000018) ADC detection
                                           threshold control 0 */

    struct {
      __IOM unsigned int ADC_INPUT_DETECTION_THRESHOLD_0 : 8; /*!< [7..0] The value against
                                                   which the ADC output has to be
                                                   compared is to be programmed
                                                   in this register */
      __IOM unsigned int COMP_LESS_THAN_EN : 1;               /*!< [8..8] When set, Aux ADC-DAC controller
                                     raises an interrupt to processor when the
                                     Aux ADC output falls below the  programmed
                                     Aux ADC detection threshold. */
      __IOM
      unsigned int COMP_GRTR_THAN_EN : 1;                     /*!< [9..9] When set, Aux ADC-DAC
                                         controller raises an interrupt to
                                         processor when the Aux ADC output is
                                         greater than the programmed Aux ADC
                                         detection threshold.. */
      __IOM unsigned int COMP_EQ_EN : 1;                      /*!< [10..10] When set, Aux ADC-DAC controller raises
                              an interrupt to processor when  the Aux ADC output
                              is equal to the programmed Aux ADC detection
                              threshold */
      __IOM unsigned int RANGE_COMPARISON_ENABLE : 1;         /*!< [11..11] When set, Aux ADC-DAC
                                           controller raises an interrupt to
                                           processor when the Aux ADC output
                                           falls within the range specified in
                                           AUX ADC Detection threshold0  and AUX
                                           ADC Detection threshold1 */
      __IOM unsigned int ADC_INPUT_DETECTION_THRESHOLD_1 : 4; /*!< [15..12] Carries upper four
                                                   bits of ADC detection
                                                   threshold      */
      __IOM unsigned int RESERVED1 : 16;                      /*!< [31..16] Reserved1           */
    } ADC_DET_THR_CTRL_0_b;
  };

  union {
    __IOM unsigned int ADC_DET_THR_CTRL_1; /*!< (@ 0x0000001C) ADC detection
                                           threshold control 1 */

    struct {
      __IOM unsigned int ADC_INPUT_DETECTION_THRESHOLD_2 : 8;      /*!< [7..0] The value against
                                                  which the ADC output has to be
                                                  compared is to be programmed
                                                  in this register. */
      __IOM unsigned int COMP_LESS_THAN_EN : 1;                    /*!< [8..8] When set, Aux ADC-DAC controller
                                    raises an interrupt to NWP when the Aux ADC
                                    output  falls below the programmed Aux ADC
                                    detection threshold. */
      __IOM unsigned int COMP_GRTR_THAN_EN : 1;                    /*!< [9..9] When set, Aux ADC-DAC controller
                                    raises an interrupt to NWP when the Aux ADC
                                    output is greater than the programmed Aux
                                    ADC detection threshold. */
      __IOM unsigned int COMP_EQ_EN : 1;                           /*!< [10..10] When set, Aux ADC-DAC controller raises
                             an interrupt to NWP when the  Aux ADC output is
                             equal to the programmed Aux ADC detection
                             threshold. */
      __IOM unsigned int ADC_DETECTION_THRESHOLD_4_UPPER_BITS : 4; /*!< [14..11] Upper 4 bits
                                                        of ADC detection
                                                        threshold 2 for ADC  */
      __IOM unsigned int RESERVED1 : 17;                           /*!< [31..15] Reserved1                */
    } ADC_DET_THR_CTRL_1_b;
  };

  union {
    __IOM unsigned int INTR_CLEAR_REG; /*!< (@ 0x00000020) ADC detection threshold
                                       control 1                          */

    struct {
      __IOM unsigned int CLR_INTR : 1;        /*!< [0..0] This bit is used to clear
                                              threshold detection interrupt        */
      __IOM unsigned int RESERVED1 : 7;       /*!< [7..1] Reserved1       */
      __IOM unsigned int INTR_CLEAR_REG : 16; /*!< [23..8] If enabled, corresponding
                                              first_mem_switch_intr bits
                                                      will be cleared. */
      __IOM unsigned int RESERVED2 : 8;       /*!< [31..24] Reserved2       */
    } INTR_CLEAR_REG_b;
  };

  union {
    __IOM unsigned int INTR_MASK_REG; /*!< (@ 0x00000024) Mask interrupt register */

    struct {
      __IOM unsigned int THRESHOLD_DETECTION_INTR_EN : 1;    /*!< [0..0] When Cleared, threshold
                                               detection interrupt will be
                                               unmasked  */
      __IOM unsigned int DAC_FIFO_EMPTY_INTR_MASK : 1;       /*!< [1..1] When Cleared, dac_FIFO_empty
                                            interrupt will be unmasked */
      __IOM unsigned int DAC_FIFO_AEMPTY_INTR_MASK : 1;      /*!< [2..2] When Cleared, adc FIFO full
                                             interrupt will be unmasked */
      __IOM unsigned int ADC_FIFO_FULL_INTR_MASK : 1;        /*!< [3..3] When Cleared, adc FIFO full
                                           interrupt will be unmasked */
      __IOM unsigned int ADC_FIFO_AFULL_INTR_MASK : 1;       /*!< [4..4] When Cleared, adc FIFO afull
                                            interrupt will be unmasked */
      __IOM unsigned int ADC_FIFO_OVERFLOW_INTR_MASK : 1;    /*!< [5..5] When Cleared, dac FIFO
                                               underrun interrupt will be
                                               unmasked    */
      __IOM unsigned int DAC_FIFO_UNDERRUN_INTR_MASK : 1;    /*!< [6..6] When Cleared, dac FIFO
                                               underrun interrupt will be
                                               unmasked    */
      __IOM unsigned int FIRST_MEM_SWITCH_INTR_MASK : 16;    /*!< [22..7] When Cleared,
                                                          first_mem_switch_intr
                                                          will be unmasked */
      __IOM unsigned int ADC_STATIC_MODE_DATA_INTR_MASK : 1; /*!< [23..23] When Cleared, adc
                                                  static_mode_data_intr will be
                                                  unmasked */
      __IOM unsigned int DAC_STATIC_MODE_DATA_INTR_MASK : 1; /*!< [24..24] When Cleared, dac
                                                  static_mode_data_intr will be
                                                  unmasked */
      __IOM unsigned int RESERVED1 : 7;                      /*!< [31..25] Reserved1           */
    } INTR_MASK_REG_b;
  };

  union {
    __IM unsigned int INTR_STATUS_REG; /*!< (@ 0x00000028) Status interrupt register */

    struct {
      __IM unsigned int ADC_THRESHOLD_DETECTION_INTR : 1; /*!< [0..0] This bit is set when ADC
                                                threshold matches with the
                                                programmed conditions This will
                                                be be cleared as soon as this
                                                interrupt is acknowledged by
                                                processor */
      __IM unsigned int DAC_FIFO_EMPTY : 1;               /*!< [1..1] Set when DAC FIFO is empty. This bit
                                  gets cleared when the DAC FIFO at least a
                                  single sample is available in DAC FIFO */
      __IM unsigned int DAC_FIFO_AEMPTY : 1;              /*!< [2..2] Set when the FIFO occupancy grater
                                   than or equal to DAC FIFO threshold. */
      __IM unsigned int ADC_FIFO_FULL : 1;                /*!< [3..3] Set when ADC FIFO is full,This bit gets
                                 cleared when data is read from the FIFO */
      __IM unsigned int ADC_FIFO_AFULL : 1;               /*!< [4..4] Set when ADC FIFO occupancy less than
                                  or equal to ADC FIFO threshold */
      __IM unsigned int ADC_FIFO_OVERFLOW : 1;            /*!< [5..5] Set when a write attempt is made to
                                     ADC FIFO when the FIFO is already full */
      __IM unsigned int DAC_FIFO_UNDERRUN : 1;            /*!< [6..6] Set when a read is done on DAC FIFO
                                     when the FIFO is empty */
      __IM unsigned int FIRST_MEM_SWITCH_INTR : 16;       /*!< [22..7] Interrupt
                                                    indicating the first memory
                                                    has been filled and the DMA
                                                    write is being shifted to
                                                    second memory chunk for
                                                    ping-pong operation */
      __IM unsigned int ADC_STATIC_MODE_DATA_INTR : 1;    /*!< [23..23] Set when a proper data
                                             packet is ready to read in static
                                                        mode for ADC */
      __IM unsigned int DAC_STATIC_MODE_DATA_INTR : 1;    /*!< [24..24] Set when a proper data
                                             packet is ready to read in static
                                                        mode for DAC */
      __IM unsigned int RESERVED1 : 7;                    /*!< [31..25] Reserved1       */
    } INTR_STATUS_REG_b;
  };

  union {
    __IM unsigned int INTR_MASKED_STATUS_REG; /*!< (@ 0x0000002C) Interrupt masked
                                              status register */

    struct {
      __IM unsigned int ADC_THRESHOLD_DETECTION_INTR_MASKED : 1; /*!< [0..0] Masked Interrupt.
                                                       This bit is set when ADC
                                                       threshold matches with
                                                       the programmed conditions
                                                     */
      __IM unsigned int DAC_FIFO_EMPTY_MASKED : 1;               /*!< [1..1] Masked Interrupt.Set
                                                   when DAC FIFO is empty */
      __IM unsigned int DAC_FIFO_AEMPTY_MASKED : 1;              /*!< [2..2] Masked Interrupt. Set when the
                                          FIFO occupancy less than equal to DAC
                                          FIFO threshold. */
      __IM unsigned int ADC_FIFO_FULL_MASKED : 1;                /*!< [3..3] Masked Interrupt. Set
                                                  when ADC FIFO is full. */
      __IM unsigned int ADC_FIFO_AFULL_MASKED : 1;               /*!< [4..4] Masked Interrupt. Set when ADC
                                         FIFO occupancy greater than ADC FIFO
                                         threshold */
      __IM unsigned int ADC_FIFO_OVERFLOW_MASKED : 1;            /*!< [5..5] Masked Interrupt. Set when a
                                            write attempt is made to ADC FIFO
                                            when the FIFO is already full. */
      __IM unsigned int DAC_FIFO_UNDERRUN_MASKED : 1;            /*!< [6..6] Masked Interrupt. Set when a
                                            read is done on DAC FIFO when the
                                            FIFO is empty. */
      __IM unsigned int FIRST_MEM_SWITCH_INTR_MASKED : 16;       /*!< [22..7] Masked Interrupt
                                            status indicating the first memory
                                            has been filled and the DMA write is
                                            being shifted  to
                                            second memory chunk for ping-pong
                                            operation */
      __IM unsigned int ADC_STATIC_MODE_DATA_INTR_MASKED : 1;    /*!< [23..23] Masked Interrupt.
                                                    Set when a proper data packet
                                                    is ready to read in static
                                                    mode for ADC */
      __IM unsigned int DAC_STATIC_MODE_DATA_INTR_MASKED : 1;    /*!< [24..24] Masked Interrupt.
                                                    Set when a proper data packet
                                                    is ready to read in static
                                                    mode for DAC */
      __IM unsigned int RESERVED1 : 7;                           /*!< [31..25] Reserved1              */
    } INTR_MASKED_STATUS_REG_b;
  };

  union {
    __IM unsigned int FIFO_STATUS_REG; /*!< (@ 0x00000030) Interrupt masked status
                                       register                           */

    struct {
      __IM unsigned int DAC_FIFO_FULL : 1;   /*!< [0..0] Set when DAC FIFO is full. In
                                          word mode, FIFO will be shown as full
                                          unless there is space for 16-bits. */
      __IM unsigned int DAC_FIFO_AFULL : 1;  /*!< [1..1] Set when DAC FIFO occupancy
                                            greater than FIFO threshold */
      __IM unsigned int ADC_FIFO_EMPTY : 1;  /*!< [2..2] Set when FIFO is empty. This bit gets
                                  cleared when the ADC FIFO is not empty. */
      __IM unsigned int ADC_FIFO_AEMPTY : 1; /*!< [3..3] Set when the FIFO occupancy
                                             less than ADC FIFO threshold */
      __IM unsigned int DAC_FIFO_EMPTY : 1;  /*!< [4..4] Set when FIFO is empty. This bit gets
                                  cleared when the DAC FIFO is not empty. */
      __IM unsigned int DAC_FIFO_AEMPTY : 1; /*!< [5..5] Set when the FIFO occupancy
                                             less than DAC FIFO threshold */
      __IM unsigned int ADC_FIFO_FULL : 1;   /*!< [6..6] Set when ADC FIFO is full.
                                             This bit gets cleared when   data is
                                             read from the FIFO.   */
      __IM unsigned int ADC_FIFO_AFULL : 1;  /*!< [7..7] Set when ADC FIFO occupancy
                                             greater than ADC FIFO threshold.  */
      __IM unsigned int RESERVED1 : 24;      /*!< [31..8] Reserved1      */
    } FIFO_STATUS_REG_b;
  };

  union {
    __IOM unsigned int ADC_CTRL_REG_2; /*!< (@ 0x00000034) ADC Control register2 */

    struct {
      __IOM unsigned int EXT_TRIG_DETECT_1 : 2; /*!< [1..0] Condition to detect event on
                                     external trigger 1 00: None (trigger
                                     disabled) 01: Positive edge 10: Negative
                                     edge 11: Positive or negative edge. */
      __IOM unsigned int EXT_TRIG_DETECT_2 : 2; /*!< [3..2] Condition to detect event on
                                     external trigger 2 00: None (trigger
                                     disabled) 01: Positive edge 10: Negative
                                     edge 11: Positive or negative edge. */
      __IOM unsigned int EXT_TRIG_DETECT_3 : 2; /*!< [5..4] Condition to detect event on
                                     external trigger 3 00: None (trigger
                                     disabled) 01: Positive edge 10: Negative
                                     edge 11: Positive or negative edge. */
      __IOM unsigned int EXT_TRIG_DETECT_4 : 2; /*!< [7..6] Condition to detect event on
                                     external trigger 4 00: None (trigger
                                     disabled) 01: Positive edge 10: Negative
                                     edge 11: Positive or negative edge. */
      __IOM unsigned int EXT_TRIGGER_SEL_4 : 4; /*!< [11..8] 4-bit Channel ID corresponding to
                                     external trigger 4.             */
      __IOM unsigned int EXT_TRIGGER_SEL_3 : 4; /*!< [15..12] 4-bit Channel ID corresponding to
                                     external trigger 3. */
      __IOM unsigned int EXT_TRIGGER_SEL_2 : 4; /*!< [19..16] Enable bit corresponding to
                                     channel id selected for trigger 2. */
      __IOM unsigned int EXT_TRIGGER_SEL_1 : 4; /*!< [23..20] 4-bit Channel ID corresponding to
                                     external trigger 1. */
      __IOM unsigned int TRIG_1_MATCH : 1;      /*!< [24..24] indicating trigger 1 is matched. Write
                                1 to clear this bit. */
      __IOM unsigned int TRIG_2_MATCH : 1;      /*!< [25..25] indicating trigger 2 is matched. Write
                                1 to clear this bit. */
      __IOM unsigned int TRIG_3_MATCH : 1;      /*!< [26..26] indicating trigger 3 is matched. Write
                                1 to clear this bit. */
      __IOM unsigned int TRIG_4_MATCH : 1;      /*!< [27..27] indicating trigger 4 is matched. Write
                                1 to clear this bit. */
      __IOM unsigned int RESERVED1 : 4;         /*!< [31..28] Reserved1 */
    } ADC_CTRL_REG_2_b;
  };
  __IOM AUX_ADC_DAC_COMP_ADC_CH_BIT_MAP_CONFIG_Type ADC_CH_BIT_MAP_CONFIG[16]; /*!< (@ 0x00000038) [0..15] */

  union {
    __IOM unsigned int ADC_CH_OFFSET[16]; /*!< (@ 0x00000138) This Register specifies initial
                               offset value with respect  to AUX_ADC clock after
                               which Channel(0-16)should be sampled. */

    struct {
      __IOM unsigned int CH_OFFSET : 16; /*!< [15..0] This Register field specifies initial
                              offset value with respect  to AUX_ADC clock after
                              which Channel(0-16)should be sampled. */
      __IOM unsigned int RESERVED1 : 16; /*!< [31..16] Reserved1 */
    } ADC_CH_OFFSET_b[16];
  };

  union {
    __IOM unsigned int ADC_CH_FREQ[16]; /*!< (@ 0x00000178) This register specifies Sampling
                             frequency rate at which AUX ADC Date is sampled for
                             Channel(1 to 16 ) */

    struct {
      __IOM unsigned int CH_FREQ_VALUE : 16; /*!< [15..0] This register specifies Sampling
                                  frequency rate at which AUX ADC Date is sampled
                                  for Channel all respective channel (1-16) */
      __IOM unsigned int RESERVED1 : 16;     /*!< [31..16] Reserved1 */
    } ADC_CH_FREQ_b[16];
  };

  union {
    __IOM unsigned int ADC_CH_PHASE_1; /*!< (@ 0x000001B8) ADC Channel Phase 1 */

    struct {
      __IOM unsigned int CH1_PHASE : 4; /*!< [3..0] Phase corresponding to channel-1 */
      __IOM unsigned int CH2_PHASE : 4; /*!< [7..4] Phase corresponding to channel-2 */
      __IOM unsigned int CH3_PHASE : 4; /*!< [11..8] Phase corresponding to channel-3 */
      __IOM unsigned int CH4_PHASE : 4; /*!< [15..12] Phase corresponding to channel-4 */
      __IOM unsigned int CH5_PHASE : 4; /*!< [19..16] Phase corresponding to channel-5 */
      __IOM unsigned int CH6_PHASE : 4; /*!< [23..20] Phase corresponding to channel-6 */
      __IOM unsigned int CH7_PHASE : 4; /*!< [27..24] Phase corresponding to channel-7 */
      __IOM unsigned int CH8_PHASE : 4; /*!< [31..28] Phase corresponding to channel-8 */
    } ADC_CH_PHASE_1_b;
  };

  union {
    __IOM unsigned int ADC_CH_PHASE_2; /*!< (@ 0x000001BC) ADC Channel Phase 2 */

    struct {
      __IOM unsigned int CH9_PHASE : 4;  /*!< [3..0] Phase corresponding to channel-9 */
      __IOM unsigned int CH10_PHASE : 4; /*!< [7..4] Phase corresponding to channel-10 */
      __IOM unsigned int CH11_PHASE : 4; /*!< [11..8] Phase corresponding to channel-11 */
      __IOM unsigned int CH12_PHASE : 4; /*!< [15..12] Phase corresponding to channel-12 */
      __IOM unsigned int CH13_PHASE : 4; /*!< [19..16] Phase corresponding to channel-13 */
      __IOM unsigned int CH14_PHASE : 4; /*!< [23..20] Phase corresponding to channel-14 */
      __IOM unsigned int CH15_PHASE : 4; /*!< [27..24] Phase corresponding to channel-15 */
      __IOM unsigned int CH16_PHASE : 4; /*!< [31..28] Phase corresponding to channel-16 */
    } ADC_CH_PHASE_2_b;
  };
  __IM unsigned int RESERVED;

  union {
    __IOM unsigned int ADC_SINGLE_CH_CTRL_1; /*!< (@ 0x000001C4) ADC SINGLE Channel
                                             Configuration */

    struct {
      __IOM unsigned int ADC_CH_INDEX_SINGLE_CHAN_1 : 32; /*!< [31..0] [31:0]out of total 48
                                               bits of bit map for single channel
                                                        mode of a particular
                                               channel. */
    } ADC_SINGLE_CH_CTRL_1_b;
  };

  union {
    __IOM unsigned int ADC_SINGLE_CH_CTRL_2; /*!< (@ 0x000001C8) ADC SINGLE Channel
                                             Configuration */

    struct {
      __IOM unsigned int ADC_CH_INDEX_SINGLE_CHAN_2 : 16; /*!< [15..0] [47:32] out of total 48
                                               bits of bit map for single channel
                                                        mode of a particular
                                               channel. */
      __IOM unsigned int ADC_INTERPOL_SINGLE_CHAN : 2;    /*!< [17..16] Interpolation angle for
                                            the particular channel in single
                                                        channel mode whose bit
                                            sequence has been written to
                                            adc_ch_index_single_c an. */
      __IOM unsigned int RESERVED1 : 14;                  /*!< [31..18] Reserved1    */
    } ADC_SINGLE_CH_CTRL_2_b;
  };

  union {
    __IOM unsigned int ADC_SEQ_CTRL; /*!< (@ 0x000001CC) This register explain
                                     configuration parameter for AUXADC */

    struct {
      __IOM unsigned int ADC_SEQ_CTRL_PING_PONG : 16; /*!< [15..0] To enable/disable per
                                           channel DAM mode (One-hot coding) */
      __IOM unsigned int ADC_SEQ_CTRL_DMA_MODE : 16;  /*!< [31..16] To enable/disable per
                                          channel ping-pong operation (One-hot
                                                        coding). */
    } ADC_SEQ_CTRL_b;
  };

  union {
    __IOM unsigned int VAD_BBP_ID; /*!< (@ 0x000001D0) This register explain VDD BBP ID */

    struct {
      __IOM unsigned int BPP_ID : 4;          /*!< [3..0] Channel id for bbp samples. */
      __IOM unsigned int BPP_EN : 1;          /*!< [4..4] Enables Aux-ADC samples to BBP */
      __IOM unsigned int AUX_ADC_BPP_EN : 1;  /*!< [5..5] Enable Indication for BBP        */
      __IOM unsigned int RESERVED1 : 10;      /*!< [15..6] RESERVED1 */
      __IOM unsigned int DISCONNET_MODE : 16; /*!< [31..16] Per channel discontinuous mode
                                   enable signal. When discontinuous mode is
                                   enabled, data is sampled only once from that
                                   channel and the enable bit is reset to 0. */
    } VAD_BBP_ID_b;
  };

  union {
    __IOM unsigned int ADC_INT_MEM_1; /*!< (@ 0x000001D4) This register explain start address
                           of first/second buffer corresponding to the channel
                           location ADC INT MEM 2 */

    struct {
      __IOM unsigned int PROG_WR_DATA : 32; /*!< [31..0] These 32-bits specifies the
                                            start address of first/second
                                                      buffer corresponding to the
                                            channel  location ADC INT MEM */
    } ADC_INT_MEM_1_b;
  };

  union {
    __IOM unsigned int ADC_INT_MEM_2; /*!< (@ 0x000001D8) This register explain ADC
                                      INT MEM2.                        */

    struct {
      __IOM unsigned int PROG_WR_DATA : 10; /*!< [9..0] These 10-bits specify the buffer length
                                 of first/second buffer corresponding to the
                                 channel location ADC INT MEM2                 */
      __IOM unsigned int PROG_WR_ADDR : 5;  /*!< [14..10] These bits correspond to
                                           the address of the internal memory
                                           basing on the channel number,  whose
                                           information we want to program */
      __IOM unsigned int PROG_WR_DATA1 : 1; /*!< [15..15] Valid bit for first/second buffers
                                 corresponding to ADC INT MEM2 */
      __IOM unsigned int RESERVED3 : 16;    /*!< [31..16] Reserved3 */
    } ADC_INT_MEM_2_b;
  };

  union {
    __IOM unsigned int INTERNAL_DMA_CH_ENABLE; /*!< (@ 0x000001DC) This register is
                                               internal channel enable */

    struct {
      __IOM unsigned int PER_CHANNEL_ENABLE : 16; /*!< [15..0] Enable bit for Each
                                                  channel,like channel0 for bit0
                                                  to channel15 for bit15 etc */
      __IOM unsigned int RESERVED3 : 15;          /*!< [30..16] Reserved3          */
      __IOM unsigned int INTERNAL_DMA_ENABLE : 1; /*!< [31..31] When Set, Internal DMA will be
                                       used for reading ADC
                                           samples from ADC FIFO and  writing
                                       them to ULP SRAM Memories.             */
    } INTERNAL_DMA_CH_ENABLE_b;
  };

  union {
    __IOM unsigned int TS_PTAT_ENABLE; /*!< (@ 0x000001E0) This register is enable
                                       PTAT for temperature sensor */

    struct {
      __IOM unsigned int TS_PTAT_EN : 1; /*!< [0..0] BJT based Temperature sensor            */
      __IOM unsigned int RESERVED1 : 31; /*!< [31..1] Reserved1 */
    } TS_PTAT_ENABLE_b;
  };

  union {
    __OM unsigned int ADC_FIFO_THRESHOLD; /*!< (@ 0x000001E4) Configured FIFO to ADC */

    struct {
      __OM unsigned int ADC_FIFO_AEMPTY_THRESHOLD : 4; /*!< [3..0] FIFO almost empty
                                                       threshold for ADC */
      __OM unsigned int ADC_FIFO_AFULL_THRESHOLD : 4;  /*!< [7..4] FIFO almost full
                                                       threshold for ADC  */
      __OM unsigned int RESERVED1 : 24;                /*!< [31..8] Reserved1                */
    } ADC_FIFO_THRESHOLD_b;
  };
  __IM unsigned int RESERVED1[6];

  union {
    __IOM unsigned int BOD; /*!< (@ 0x00000200) Programs resistor bank, reference
                            buffer and scaler */

    struct {
      __IOM unsigned int EN_BOD_TEST_MUX : 1; /*!< [0..0] 1 - To enable test mux         */
      __IOM unsigned int BOD_TEST_SEL : 2;    /*!< [2..1] Select bits for test mux */
      __IOM unsigned int REFBUF_EN : 1;       /*!< [3..3] Reference buffer configuration 1
                                        for enable 0 for disable          */
      __IOM unsigned int REFBUF_VOLT_SEL : 4; /*!< [7..4] selection of voltage of
                                              reference buffer */
      __IOM unsigned int BOD_RES_EN : 1;      /*!< [8..8] configuration of register bank
                                         1 for enable and 0 for disable */
      __IOM unsigned int BOD_THRSH : 5;       /*!< [13..9] Programmability for resistor bank */
      __IOM unsigned int RESERVED2 : 18;      /*!< [31..14] Reserved2 */
    } BOD_b;
  };

  union {
    __IOM unsigned int COMPARATOR1; /*!< (@ 0x00000204) Programs comparators1 and
                                    comparators2                     */

    struct {
      __IOM unsigned int CMP1_EN : 1;        /*!< [0..0] To enable comparator1 */
      __IOM unsigned int CMP1_EN_FILTER : 1; /*!< [1..1] To enable filter for comparator 1 */
      __IOM unsigned int CMP1_HYST : 2;      /*!< [3..2] Programmability to control
                                             hysteresis of comparator1      */
      __IOM unsigned int CMP1_MUX_SEL_P : 4; /*!< [7..4] Select for positive input
                                             of comparator_1 */
      __IOM unsigned int CMP1_MUX_SEL_N : 4; /*!< [11..8] Select for negative input
                                             of comparator_1 */
      __IOM unsigned int CMP2_EN : 1;        /*!< [12..12] To enable comparator 2        */
      __IOM unsigned int CMP2_EN_FILTER : 1; /*!< [13..13] To enable filter for
                                             comparator 2 */
      __IOM unsigned int CMP2_HYST : 2;      /*!< [15..14] Programmability to control
                                        hysteresis of comparator2             */
      __IOM unsigned int CMP2_MUX_SEL_P : 4; /*!< [19..16] Select for positive input
                                             of comparator_2 */
      __IOM unsigned int CMP2_MUX_SEL_N : 4; /*!< [23..20] Select for negative input
                                             of comparator_2 */
      __IOM unsigned int COM_DYN_EN : 1;     /*!< [24..24] Dynamic enable for registers */
      __IOM unsigned int RESERVED1 : 7;      /*!< [31..25] Reserved1 */
    } COMPARATOR1_b;
  };

  union {
    __IOM unsigned int AUXADC_CONFIG_2; /*!< (@ 0x00000208) This register is AUX-ADC
                                        config2                           */

    struct {
      __IOM unsigned int AUXADC_INP_SEL : 5;    /*!< [4..0] Mux select for positive
                                             input of adc */
      __IOM unsigned int AUXADC_INN_SEL : 4;    /*!< [8..5] Mux select for negetive
                                             input of adc */
      __IOM unsigned int AUXADC_DIFF_MODE : 1;  /*!< [9..9] AUX ADC Differential Mode         */
      __IOM unsigned int AUXADC_ENABLE : 1;     /*!< [10..10] Static Enable */
      __IOM unsigned int AUXADC_DYN_ENABLE : 1; /*!< [11..11] Aux ADC Configuration Enable */
      __IOM unsigned int RESERVED2 : 20;        /*!< [31..12] Reserved2 */
    } AUXADC_CONFIG_2_b;
  };

  union {
    __IOM unsigned int AUXDAC_CONIG_1; /*!< (@ 0x0000020C) This register is AUX-DAC
                                       config1                           */

    struct {
      __IOM unsigned int AUXDAC_EN_S : 1;        /*!< [0..0] Enable signal DAC       */
      __IOM unsigned int AUXDAC_OUT_MUX_EN : 1;  /*!< [1..1] Aux OUT mux Enable */
      __IOM unsigned int AUXDAC_OUT_MUX_SEL : 1; /*!< [2..2] AUXDAC OUT MUX SELECT Enable   */
      __IOM unsigned int RESERVED1 : 1;          /*!< [3..3] Reserved1 */
      __IOM unsigned int AUXDAC_DATA_S : 10;     /*!< [13..4] Satatic AUX Dac Data */
      __IOM unsigned int AUXDAC_DYN_EN : 1;      /*!< [14..14] Satatic AUX Dac Data  */
      __IOM unsigned int RESERVED2 : 17;         /*!< [31..15] RESERVED2     */
    } AUXDAC_CONIG_1_b;
  };

  union {
    __IOM unsigned int AUX_LDO; /*!< (@ 0x00000210) This register is AUX-LDO configuration */

    struct {
      __IOM unsigned int LDO_CTRL : 4;         /*!< [3..0] Enable ldo control field */
      __IOM unsigned int LDO_DEFAULT_MODE : 1; /*!< [4..4] ldo default mode enable      */
      __IOM unsigned int BYPASS_LDO : 1;       /*!< [5..5] bypass the LDO */
      __IOM unsigned int ENABLE_LDO : 1;       /*!< [6..6] Turn LDO */
      __IOM unsigned int DYN_EN : 1;           /*!< [7..7] Dynamic Enable     */
      __IOM unsigned int RESERVED1 : 24;       /*!< [31..8] It is recommended to write
                                         these bits to 0. */
    } AUX_LDO_b;
  };
} AUX_ADC_DAC_COMP_Type; /*!< Size = 532 (0x214) */

/* ===========================================================================================================================
  */
/* ================                                            IR
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief IR Decoder are used for the decoding the external ir sensor input.
  * (IR)
  */

typedef struct { /*!< (@ 0x24040C00) IR Structure */

  union {
    __IOM unsigned int IR_OFF_TIME_DURATION; /*!< (@ 0x00000000) This register used for IR
                                  sleep duration timer value. */

    struct {
      __IOM unsigned int IR_OFF_TIME_DURATION : 17; /*!< [16..0] This field define
                                                    ir off time */
      __IM unsigned int RES : 15;                   /*!< [31..17] reserved5                   */
    } IR_OFF_TIME_DURATION_b;
  };

  union {
    __IOM unsigned int IR_ON_TIME_DURATION; /*!< (@ 0x00000004) This register used for IR
                                 Detection duration timer value. */

    struct {
      __IOM unsigned int IR_ON_TIME_DURATION : 12; /*!< [11..0] This field define ir on time
                                        for ir detection on                  */
      __IM unsigned int RES : 20;                  /*!< [31..12] reserved5       */
    } IR_ON_TIME_DURATION_b;
  };

  union {
    __IOM unsigned int IR_FRAME_DONE_THRESHOLD; /*!< (@ 0x00000008) This register used count
                                     with respect to 32KHz clock after not more
                                     toggle are expected to a given pattern. */

    struct {
      __IOM unsigned int IR_FRAME_DONE_THRESHOLD : 15; /*!< [14..0] count with respect to 32KHz
                                           clock after not more toggle are
                                           expected to a given pattern */
      __IM unsigned int RES : 17;                      /*!< [31..15] reserved5           */
    } IR_FRAME_DONE_THRESHOLD_b;
  };

  union {
    __IOM unsigned int IR_DET_THRESHOLD; /*!< (@ 0x0000000C) This register used Minimum Number
                              of edges to detected during on-time failing which
                              IR detection is re-stated. */

    struct {
      __IOM unsigned int IR_DET_THRESHOLD : 7; /*!< [6..0] Minimum Number of edges to detected
                                    during on-time failing
                                           which IR detection is re-stated. */
      __IM unsigned int RES : 25;              /*!< [31..7] reserved5   */
    } IR_DET_THRESHOLD_b;
  };

  union {
    __IOM unsigned int IR_CONFIG; /*!< (@ 0x00000010) This register used to configure the ir
                       structure for application purpose. */

    struct {
      __IOM unsigned int EN_IR_DET : 1;      /*!< [0..0] Enable IR detection logic bit if bit 1 then
                             detection enable if 0 then not enable. */
      __IOM unsigned int IR_DET_RSTART : 1;  /*!< [1..1] Enable IR detection re-start logic bit
                                 if bit 1 then re-start. */
      __IOM unsigned int EN_CLK_IR_CORE : 1; /*!< [2..2] Enable 32KHz clock to IR Core bit ,if
                                  bit 1 then clock gating disable and bit is 0
                                  then clock gating Enable */
      __IM unsigned int RES : 5;             /*!< [7..3] reserved5  */
      __IOM unsigned int EN_CONT_IR_DET : 1; /*!< [8..8] This bit is Enable continues IR
                                  detection,When enabled there will be no power
                                  cycling on External IR Sensor. */
      __IM unsigned int RES1 : 7;            /*!< [15..9] reserved6 */
      __IOM unsigned int SREST_IR_CORE : 1;  /*!< [16..16] This bit is used soft
                                            reset IR core block */
      __IM unsigned int RES2 : 15;           /*!< [31..17] reserved7          */
    } IR_CONFIG_b;
  };

  union {
    __IOM unsigned int IR_MEM_ADDR_ACCESS; /*!< (@ 0x00000014) This register used to access
                                memory address for application purpose. */

    struct {
      __IOM unsigned int IR_MEM_ADDR : 7;         /*!< [6..0] This field is used to IR read
                                           address.                             */
      __IOM unsigned int RES : 1;                 /*!< [7..7] reserved5          */
      __IOM unsigned int IR_MEM_WR_EN : 1;        /*!< [8..8] IR memory write enable. */
      __IOM unsigned int IR_MEM_RD_EN : 1;        /*!< [9..9] This field used to IR memory
                                           read enable. */
      __IOM unsigned int RES1 : 6;                /*!< [15..10] reserved1         */
      __IOM unsigned int IR_MEM_WR_TEST_MODE : 1; /*!< [16..16] IR memory write
                                                  enable in test mode.. */
      __IOM unsigned int RES2 : 15;               /*!< [31..17] reserved2               */
    } IR_MEM_ADDR_ACCESS_b;
  };

  union {
    __IM unsigned int IR_MEM_READ; /*!< (@ 0x00000018) This register used to IR Read
                                   data from memory.            */

    struct {
      __IM unsigned int IR_MEM_DATA_OUT : 16;  /*!< [15..0] This field is used to IR
                                              Read data from memory. */
      __IM unsigned int RES : 8;               /*!< [23..16] reserved5              */
      __IM unsigned int IR_DATA_MEM_DEPTH : 7; /*!< [30..24] This field used to indicated
                                     valid number of IR Address
                                            in the memory to be read. */
      __IM unsigned int RES1 : 1;              /*!< [31..31] reserved6    */
    } IR_MEM_READ_b;
  };
} IR_Type; /*!< Size = 28 (0x1c) */

/* ===========================================================================================================================
  */
/* ================                                        MISC_CONFIG
  * ================ */
/* ===========================================================================================================================
  */

/**
  * @brief MISC CONFIG Register (MISC_CONFIG)
  */

typedef struct { /*!< (@ 0x46008000) MISC_CONFIG Structure */

  union {
    __IOM unsigned int MISC_CFG_HOST_INTR_MASK; /*!< (@ 0x00000000) MISC CFG HOST
                                                INTR MASK */

    struct {
      __IOM unsigned int HOST_INTR_MSK : 8;                 /*!< [7..0] Writing 1 in any bit masks
                                            the corresponding interrupt in
                                            HOST_INTR_STATUS. */
      __IOM unsigned int HOST_SPI_INTR_OPEN_DRAIN_MODE : 1; /*!< [8..8] Writing 1 to this bit
                                                 configures the host SPI
                                                 interrupt in open drain mode.
                                                 When open drain mode is enabled
                                                 and interrupt is configured in
                                                 active high mode,  external
                                                        PULLDOWN has to be used
                                                 on the board. */
      __IOM
      unsigned int HOST_SPI_INTR_ACTIVE_LOW_MODE : 1; /*!< [9..9] Writing 1 to this
                                                     bit configures the host SPI
                                                     interrupt in active low
                                                     mode. By default, it will be
                                                     active high. */
      __IOM unsigned int RESERVED1 : 22;              /*!< [31..10] reserved1             */
    } MISC_CFG_HOST_INTR_MASK_b;
  };
} MISC_CONFIG_Type; /*!< Size = 4 (0x4) */

#if defined(SLI_SI917B0)
/**************************************************************************/ /**
                                                                               * @defgroup RSI_DEVICE_SYSRTC SYSRTC
                                                                               * @{
                                                                               * @brief RSI_DEVICE SYSRTC Register Declaration.
                                                                               *****************************************************************************/

/** SYSRTC Register Declaration. */
typedef struct {
  __IM unsigned int IPVERSION;           /**< IP VERSION */
  __IOM unsigned int EN;                 /**< Module Enable Register                             */
  __IOM unsigned int SWRST;              /**< Software Reset Register */
  __IOM unsigned int CFG;                /**< Configuration Register                             */
  __IOM unsigned int CMD;                /**< Command Register                                   */
  __IM unsigned int STATUS;              /**< Status register */
  __IOM unsigned int CNT;                /**< Counter Value Register                             */
  __IM unsigned int SYNCBUSY;            /**< Synchronization Busy Register            */
  __IOM unsigned int LOCK;               /**< Configuration Lock Register               */
  unsigned int RESERVED0[3U];            /**< Reserved for future use            */
  __IOM unsigned int FAILDETCTRL;        /**< Failure Detection        */
  __IOM unsigned int FAILDETLOCK;        /**< FAILDET Lock Register        */
  unsigned int RESERVED1[2U];            /**< Reserved for future use            */
  __IOM unsigned int GRP0_IF;            /**< Group Interrupt Flags            */
  __IOM unsigned int GRP0_IEN;           /**< Group Interrupt Enables           */
  __IOM unsigned int GRP0_CTRL;          /**< Group Control Register          */
  __IOM unsigned int GRP0_CMP0VALUE;     /**< Compare 0 Value Register     */
  __IOM unsigned int GRP0_CMP1VALUE;     /**< Compare 1 Value Register     */
  __IM unsigned int GRP0_CAP0VALUE;      /**< Capture 0 Value Register      */
  __IM unsigned int GRP0_SYNCBUSY;       /**< Synchronization busy Register       */
  unsigned int RESERVED2[1U];            /**< Reserved for future use            */
  __IOM unsigned int GRP1_IF;            /**< Group Interrupt Flags            */
  __IOM unsigned int GRP1_IEN;           /**< Group Interrupt Enables           */
  __IOM unsigned int GRP1_CTRL;          /**< Group Control Register          */
  __IOM unsigned int GRP1_CMP0VALUE;     /**< Compare 0 Value Register     */
  __IOM unsigned int GRP1_CMP1VALUE;     /**< Compare 1 Value Register     */
  __IM unsigned int GRP1_CAP0VALUE;      /**< Capture 0 Value Register      */
  __IM unsigned int GRP1_SYNCBUSY;       /**< Synchronization busy Register       */
  unsigned int RESERVED3[33U];           /**< GRP2 - GRP7,Reserved for future use           */
  __IM unsigned int IPVERSION_SET;       /**< IP VERSION       */
  __IOM unsigned int EN_SET;             /**< Module Enable Register             */
  __IOM unsigned int SWRST_SET;          /**< Software Reset Register          */
  __IOM unsigned int CFG_SET;            /**< Configuration Register            */
  __IOM unsigned int CMD_SET;            /**< Command Register            */
  __IM unsigned int STATUS_SET;          /**< Status register          */
  __IOM unsigned int CNT_SET;            /**< Counter Value Register            */
  __IM unsigned int SYNCBUSY_SET;        /**< Synchronization Busy Register        */
  __IOM unsigned int LOCK_SET;           /**< Configuration Lock Register           */
  unsigned int RESERVED4[3U];            /**< Reserved for future use            */
  __IOM unsigned int FAILDETCTRL_SET;    /**< Failure Detection    */
  __IOM unsigned int FAILDETLOCK_SET;    /**< FAILDET Lock Register    */
  unsigned int RESERVED5[2U];            /**< Reserved for future use            */
  __IOM unsigned int GRP0_IF_SET;        /**< Group Interrupt Flags        */
  __IOM unsigned int GRP0_IEN_SET;       /**< Group Interrupt Enables       */
  __IOM unsigned int GRP0_CTRL_SET;      /**< Group Control Register      */
  __IOM unsigned int GRP0_CMP0VALUE_SET; /**< Compare 0 Value Register */
  __IOM unsigned int GRP0_CMP1VALUE_SET; /**< Compare 1 Value Register */
  __IM unsigned int GRP0_CAP0VALUE_SET;  /**< Capture 0 Value Register  */
  __IM unsigned int GRP0_SYNCBUSY_SET;   /**< Synchronization busy Register   */
  unsigned int RESERVED6[1U];            /**< Reserved for future use            */
  __IOM unsigned int GRP1_IF_SET;        /**< Group Interrupt Flags        */
  __IOM unsigned int GRP1_IEN_SET;       /**< Group Interrupt Enables       */
  __IOM unsigned int GRP1_CTRL_SET;      /**< Group Control Register      */
  __IOM unsigned int GRP1_CMP0VALUE_SET; /**< Compare 0 Value Register */
  __IOM unsigned int GRP1_CMP1VALUE_SET; /**< Compare 1 Value Register */
  __IM unsigned int GRP1_CAP0VALUE_SET;  /**< Capture 0 Value Register  */
  __IM unsigned int GRP1_SYNCBUSY_SET;   /**< Synchronization busy Register   */
  unsigned int RESERVED7[33U];           /**< Reserved for future use           */
  __IM unsigned int IPVERSION_CLR;       /**< IP VERSION       */
  __IOM unsigned int EN_CLR;             /**< Module Enable Register             */
  __IOM unsigned int SWRST_CLR;          /**< Software Reset Register          */
  __IOM unsigned int CFG_CLR;            /**< Configuration Register            */
  __IOM unsigned int CMD_CLR;            /**< Command Register            */
  __IM unsigned int STATUS_CLR;          /**< Status register          */
  __IOM unsigned int CNT_CLR;            /**< Counter Value Register            */
  __IM unsigned int SYNCBUSY_CLR;        /**< Synchronization Busy Register        */
  __IOM unsigned int LOCK_CLR;           /**< Configuration Lock Register           */
  unsigned int RESERVED8[3U];            /**< Reserved for future use            */
  __IOM unsigned int FAILDETCTRL_CLR;    /**< Failure Detection    */
  __IOM unsigned int FAILDETLOCK_CLR;    /**< FAILDET Lock Register    */
  unsigned int RESERVED9[2U];            /**< Reserved for future use            */
  __IOM unsigned int GRP0_IF_CLR;        /**< Group Interrupt Flags        */
  __IOM unsigned int GRP0_IEN_CLR;       /**< Group Interrupt Enables       */
  __IOM unsigned int GRP0_CTRL_CLR;      /**< Group Control Register      */
  __IOM unsigned int GRP0_CMP0VALUE_CLR; /**< Compare 0 Value Register */
  __IOM unsigned int GRP0_CMP1VALUE_CLR; /**< Compare 1 Value Register */
  __IM unsigned int GRP0_CAP0VALUE_CLR;  /**< Capture 0 Value Register  */
  __IM unsigned int GRP0_SYNCBUSY_CLR;   /**< Synchronization busy Register   */
  unsigned int RESERVED10[1U];           /**< Reserved for future use           */
  __IOM unsigned int GRP1_IF_CLR;        /**< Group Interrupt Flags        */
  __IOM unsigned int GRP1_IEN_CLR;       /**< Group Interrupt Enables       */
  __IOM unsigned int GRP1_CTRL_CLR;      /**< Group Control Register      */
  __IOM unsigned int GRP1_CMP0VALUE_CLR; /**< Compare 0 Value Register */
  __IOM unsigned int GRP1_CMP1VALUE_CLR; /**< Compare 1 Value Register */
  __IM unsigned int GRP1_CAP0VALUE_CLR;  /**< Capture 0 Value Register  */
  __IM unsigned int GRP1_SYNCBUSY_CLR;   /**< Synchronization busy Register   */
  unsigned int RESERVED11[33U];          /**< Reserved for future use          */
  __IM unsigned int IPVERSION_TGL;       /**< IP VERSION       */
  __IOM unsigned int EN_TGL;             /**< Module Enable Register             */
  __IOM unsigned int SWRST_TGL;          /**< Software Reset Register          */
  __IOM unsigned int CFG_TGL;            /**< Configuration Register            */
  __IOM unsigned int CMD_TGL;            /**< Command Register            */
  __IM unsigned int STATUS_TGL;          /**< Status register          */
  __IOM unsigned int CNT_TGL;            /**< Counter Value Register            */
  __IM unsigned int SYNCBUSY_TGL;        /**< Synchronization Busy Register        */
  __IOM unsigned int LOCK_TGL;           /**< Configuration Lock Register           */
  unsigned int RESERVED12[3U];           /**< Reserved for future use           */
  __IOM unsigned int FAILDETCTRL_TGL;    /**< Failure Detection    */
  __IOM unsigned int FAILDETLOCK_TGL;    /**< FAILDET Lock Register    */
  unsigned int RESERVED13[2U];           /**< Reserved for future use           */
  __IOM unsigned int GRP0_IF_TGL;        /**< Group Interrupt Flags        */
  __IOM unsigned int GRP0_IEN_TGL;       /**< Group Interrupt Enables       */
  __IOM unsigned int GRP0_CTRL_TGL;      /**< Group Control Register      */
  __IOM unsigned int GRP0_CMP0VALUE_TGL; /**< Compare 0 Value Register */
  __IOM unsigned int GRP0_CMP1VALUE_TGL; /**< Compare 1 Value Register */
  __IM unsigned int GRP0_CAP0VALUE_TGL;  /**< Capture 0 Value Register  */
  __IM unsigned int GRP0_SYNCBUSY_TGL;   /**< Synchronization busy Register   */
  unsigned int RESERVED14[1U];           /**< Reserved for future use           */
  __IOM unsigned int GRP1_IF_TGL;        /**< Group Interrupt Flags        */
  __IOM unsigned int GRP1_IEN_TGL;       /**< Group Interrupt Enables       */
  __IOM unsigned int GRP1_CTRL_TGL;      /**< Group Control Register      */
  __IOM unsigned int GRP1_CMP0VALUE_TGL; /**< Compare 0 Value Register */
  __IOM unsigned int GRP1_CMP1VALUE_TGL; /**< Compare 1 Value Register */
  __IM unsigned int GRP1_CAP0VALUE_TGL;  /**< Capture 0 Value Register  */
  __IM unsigned int GRP1_SYNCBUSY_TGL;   /**< Synchronization busy Register   */
  unsigned int RESERVED15[32U];          /**< Reserved for future use          */
  __IOM unsigned int MCUSYSRTC_REG1;     /**< input and output configuration     */

} SYSRTC_TypeDef;

/* =========================================================================================================================== */
/* ================                                            SDC                                            ================ */
/* =========================================================================================================================== */

/**
   * @brief SDC_SDC_DATA_REG [SDC_DATA_REG] (SDC Data registers (0-15))
   */
typedef struct {
  union {
    __IM unsigned int DATA_REG; /*!< (@ 0x00000000) SDC Data register                                          */

    struct {
      __IM unsigned int
        SDC_DATA_SAMPLE : 12;          /*!< [11..0] Sample 0 collected from Sensor through Aux ADC.                   */
      __IM unsigned int SMP_ID_CH : 2; /*!< [13..12] Channel iD for sample                                            */
      __IM unsigned int
        RESERVED1 : 18; /*!< [31..14] reserved1                                                        */
    } DATA_REG_b;
  };
} SDC_SDC_DATA_REG_Type;

/**
   * @brief Sensor Data Collector Register structure (SDC)
   */

typedef struct { /*!< (@ 0x24042400) SDC Structure                                              */

  union {
    __IOM unsigned int
      SDC_GEN_CONFIG_0; /*!< (@ 0x00000000) SDC general configuration 0                                */

    struct {
      __IOM unsigned int INTR_STATUS_CLEAR : 1; /*!< [0..0] Writing 1 clears interrupt, reading gives SDC Interrupt
                                                      status                                                                    */
      __IOM unsigned int
        RESERVED1 : 31; /*!< [31..1] Reserevd                                                          */
    } SDC_GEN_CONFIG_0_b;
  };

  union {
    __IOM unsigned int
      SDC_GEN_CONFIG_1; /*!< (@ 0x00000004) SDC general configuration 1                                */

    struct {
      __IOM unsigned int RST_WRT_PTR : 1; /*!< [0..0] Writing 1 will resets the write pointer so that new samples
                                                      can be filled in Buffer.                                                  */
      __IM unsigned int WRT_PTR : 4; /*!< [4..1] Write pointer Value                                                */
      __IOM unsigned int SAMP_THRESH : 4; /*!< [8..5] Number of data sampled to be collected from Aux-ADC and
                                                      stored in Buffer before interrupt is raised/wakeup is initialed           */
      __IOM unsigned int
        RESERVED1 : 23; /*!< [31..9] Reserevd                                                          */
    } SDC_GEN_CONFIG_1_b;
  };

  union {
    __IOM unsigned int
      SDC_GEN_CONFIG_2; /*!< (@ 0x00000008) SDC general configuration 2                                */

    struct {
      __IOM unsigned int
        SDC_SAMP_EN : 1; /*!< [0..0] SDC Data Sampling mode                                             */
      __IOM unsigned int
        NUM_CH_SEL : 3; /*!< [3..1] Number of Channels to be used                                      */
      __IOM unsigned int
        RESERVED1 : 28; /*!< [31..4] Reserevd                                                          */
    } SDC_GEN_CONFIG_2_b;
  };

  union {
    __IOM unsigned int
      SDC_GEN_CONFIG_3; /*!< (@ 0x00000014) SDC general configuration 3                                */

    struct {
      __IOM unsigned int
        SAMP_TRIG_SEL : 1; /*!< [0..0] select the trigger event on which AUX-ADC Data is sampled          */
      __IOM unsigned int
        CNT_TRIG_EVNT : 10; /*!< [10..1] which trigger event AUX-ADC Data will sampled                     */
      __IOM unsigned int
        SDC_CLK_DIV : 10; /*!< [20..11] SDCSS clock division factor                                      */
      __IOM unsigned int
        RESERVED1 : 11; /*!< [31..21] Reserevd                                                         */
    } SDC_GEN_CONFIG_3_b;
  };
  __IM unsigned int RESERVED[2];
  union {
    __IOM unsigned int
      SDC_AUXADC_CONFIG_1; /*!< (@ 0x00000018) SDC AUX ADC configuration 1                                */

    struct {
      __IOM unsigned int
        SDC_AUXADC_INPUT_P_SEL_CH1 : 5; /*!< [4..0] AUXADC's Positive Input Mux Select for Channel-1               */
      __IOM unsigned int
        SDC_AUXADC_INPUT_N_SEL_CH1 : 4; /*!< [8..5] AUXADC's Negative Input Mux Select for Channel-1               */
      __IOM unsigned int
        SDC_AUXADC_DIFF_MODE_CH1 : 1; /*!< [9..9] Enable Differential Mode in AUX ADC for Channel -1               */
      __IOM unsigned int
        SDC_AUXADC_EN : 1; /*!< [10..10] AUXADC Enable from SDC Block                                     */
      __IOM unsigned int SDC_ADC_CONFIG_EN : 1; /*!< [11..11] On Enabling this register, SDC ADC Configuration will
                                                      be Applied.                                                               */
      __IOM unsigned int
        RESERVED1 : 20; /*!< [31..12] Reserevd                                                         */
    } SDC_AUXADC_CONFIG_1_b;
  };

  union {
    __IOM unsigned int
      SDC_AUXDAC_CONFIG_1; /*!< (@ 0x0000001C) SDC AUX DAC configuration 1                                */

    struct {
      __IOM unsigned int
        SDC_DAC_EN : 1; /*!< [0..0] Enable signal DAC                                                  */
      __IOM unsigned int
        SDC_DAC_OUT_MUX_EN : 1; /*!< [1..1] Enable signal for Connecting DAC Output to GPIO                    */
      __IOM unsigned int SDC_DAC_OUT_MUX_SEL : 1; /*!< [2..2] Programming register for choosing GPIO in which DAC Output
                                                     is connected                                                              */
      __IOM unsigned int
        RESERVED1 : 1; /*!< [3..3] Reserved                                                           */
      __IOM unsigned int
        SDC_DAC_DATA : 10; /*!< [13..4] SDC Aux DAC Data                                                  */
      __IOM unsigned int SDC_DAC_CONFIG_EN : 1; /*!< [14..14] On Enabling this register, SDC DAC Configuration will
                                                      be Applied.                                                               */
      __IOM unsigned int
        RESERVED2 : 17; /*!< [31..15] Reserevd                                                         */
    } SDC_AUXDAC_CONFIG_1_b;
  };

  union {
    __IOM unsigned int
      SDC_AUXLDO_CONFIG; /*!< (@ 0x00000020) SDC AUX LDO configuration                                  */

    struct {
      __IOM unsigned int
        SDC_AUXLDO_VOLT_CTRL : 4; /*!< [3..0] SDC AUX LDO Voltage Control Selection                              */
      __IOM unsigned int
        RESERVED1 : 1; /*!< [4..4] RESERVED                                                           */
      __IOM unsigned int SDC_AUXLDO_BYP_EB : 1; /*!< [5..5] Configure AUXLDO in Buypass mode.When Enabled, Ouput
                                                      supply of LDO will be same as Input supply.                               */
      __IOM unsigned int
        SDC_AUXLDO_EN : 1; /*!< [6..6] Turn-On AUX LDO                                                    */
      __IOM unsigned int
        SDC_AUXLDO_CONFIG_EN : 1; /*!< [7..7] SDC Aux LDO Configuration Control Enable                           */
      __IOM unsigned int
        RESERVED2 : 24; /*!< [31..8] Reserved                                                          */
    } SDC_AUXLDO_CONFIG_b;
  };

  union {
    __IOM unsigned int
      SDC_AUXOPAMP_CONFIG_1; /*!< (@ 0x00000024) SDC AUX OPAMP configuration 1                              */

    struct {
      __IOM unsigned int SDC_OPAMP_EN_CH1 : 1; /*!< [0..0] Enable signal for turning OPAMP to used for Channel-1
                                                      Operation                                                                 */
      __IOM unsigned int
        SDC_OPAMP_LP_MODE : 1; /*!< [1..1] Configuration of OPAMP1 Operation mode                             */
      __IOM unsigned int
        SDC_OPAMP_R1_SEL : 2; /*!< [3..2] Configuration for Resistor Ladder R1 of OPAMP1 for controlling
                                                      it gain.                                                                  */
      __IOM unsigned int
        SDC_OPAMP_R2_SEL : 3; /*!< [6..4] Configuration for Resistor Ladder R2 of OPAMP1 for controlling
                                                      it gain.                                                                  */
      __IOM unsigned int SDC_OPAMP_RES_BACK_EN : 1; /*!< [7..7] Configuration register for controlling Resistor Bank
                                                      of OPAMP                                                                  */
      __IOM unsigned int
        SDC_OPAMP_RES_MUX_SEL : 3; /*!< [10..8] Configuration register for Connecting R1 Resistor Ladder
                                                      input                                                                     */
      __IOM unsigned int
        SDC_OPAMP_RES_TO_OUT_VDD : 1; /*!< [11..11] Configuration register for Connecting R2 Resistor Ladder
                                                      input                                                                     */
      __IOM unsigned int
        SDC_OPAMP_OUT_MUX_EN : 1; /*!< [12..12] Configur this register to OPAMP1 Output will be connected
                                                      to GPIO                                                                   */
      __IOM unsigned int
        SDC_OPAMP_IN_N_SEL : 3; /*!< [15..13] Configuration register for selecting N Input of OPAMP1.          */
      __IOM unsigned int
        SDC_OPAMP_IN_P_SEL_CH1 : 4; /*!< [19..16] Configuration register for selecting P Input of OPAMP1.,for
                                                      CH1                                                                       */
      __IOM unsigned int SDC_OPAMP_OUT_MUX_SEL : 1; /*!< [20..20] Configuration register for connecting OPAMP1 output
                                                      to GPIO                                                                   */
      __IM unsigned int RESERVED1 : 1; /*!< [21..21] Reserved                                                         */
      __IOM unsigned int SDC_VREF_MUX_1_EN : 1; /*!< [22..22] Connect Low Drive Strength voltage reference for ULP
                                                      GPIO 1 For external use                                                   */
      __IOM unsigned int SDC_VREF_MUX_2_EN : 1; /*!< [23..23] Connect Low Drive Strength voltage reference for ULP
                                                      GPIO 3 For external use                                                   */
      __IOM unsigned int SDC_VREF_MUX_3_EN : 1; /*!< [24..24] Connect Low Drive Strength voltage reference for ULP
                                                      GPIO 4 For external use                                                   */
      __IOM unsigned int SDC_VREF_MUX_4_EN : 1; /*!< [25..25] Connect Low Drive Strength voltage reference for ULP
                                                      GPIO 15 For external use                                                  */
      __IOM unsigned int
        RESERVED2 : 1; /*!< [26..26] Reserved                                                         */
      __IOM unsigned int SDC_VREF_MUX_1_SEL : 1;  /*!< [27..27] Selection register for choosing Voltage reference to
                                                      external use on ULP_GPIO_1                                                */
      __IOM unsigned int SDC_VREF_MUX_2_SEL : 1;  /*!< [28..28] Selection register for choosing Voltage reference to
                                                      external use on ULP_GPIO_3                                                */
      __IOM unsigned int SDC_VREF_MUX_3_SEL : 1;  /*!< [29..29] Selection register for choosing Voltage reference to
                                                      external use on ULP_GPIO_4                                                */
      __IOM unsigned int SDC_VREF_MUX_4_SEL : 1;  /*!< [30..30] Selection register for choosing Voltage reference to
                                                      external use on ULP_GPIO_15                                               */
      __IOM unsigned int SDC_OPAMP_CONFIG_EN : 1; /*!< [31..31] On Enabling this register, SDC OPAMP Configuration
                                                      will be Applied.                                                          */
    } SDC_AUXOPAMP_CONFIG_1_b;
  };

  union {
    __IOM unsigned int
      SDC_AUXADC_CONFIG_2; /*!< (@ 0x00000028) SDC AUX ADC configuration 2                                */

    struct {
      __IOM unsigned int
        SDC_AUXADC_INPUT_P_SEL_CH2 : 5; /*!< [4..0] AUXADC's Positive Input Mux Select for Channel-2               */
      __IOM unsigned int
        SDC_AUXADC_INPUT_N_SEL_CH2 : 4; /*!< [8..5] AUXADC's Negative Input Mux Select for Channel-2               */
      __IOM unsigned int
        SDC_AUXADC_DIFF_MODE_CH2 : 1; /*!< [9..9] 1-AUX ADC Differencial mode, 0 - Single Ended Mode               */
      __IOM unsigned int
        RESERVED1 : 22; /*!< [31..10] Reserevd                                                         */
    } SDC_AUXADC_CONFIG_2_b;
  };

  union {
    __IOM unsigned int
      SDC_AUXADC_CONFIG_3; /*!< (@ 0x0000002C) SDC AUX ADC configuration 3                                */

    struct {
      __IOM unsigned int
        SDC_AUXADC_INPUT_P_SEL_CH3 : 5; /*!< [4..0] AUXADC's Positive Input Mux Select for Channel-3               */
      __IOM unsigned int
        SDC_AUXADC_INPUT_N_SEL_CH3 : 4; /*!< [8..5] AUXADC's Negative Input Mux Select for Channel-3               */
      __IOM unsigned int
        SDC_AUXADC_DIFF_MODE_CH3 : 1; /*!< [9..9] 1-AUX ADC Differencial mode, 0 - Single Ended Mode               */
      __IOM unsigned int
        RESERVED1 : 22; /*!< [31..10] Reserved                                                         */
    } SDC_AUXADC_CONFIG_3_b;
  };

  union {
    __IOM unsigned int
      SDC_AUXADC_CONFIG_4; /*!< (@ 0x00000030) SDC AUX ADC configuration 4                                */

    struct {
      __IOM unsigned int
        SDC_AUXADC_INPUT_P_SEL_CH4 : 5; /*!< [4..0] AUXADC's Positive Input Mux Select for Channel-4               */
      __IOM unsigned int
        SDC_AUXADC_INPUT_N_SEL_CH4 : 4; /*!< [8..5] AUXADC's Negative Input Mux Select for Channel-4               */
      __IOM unsigned int
        SDC_AUXADC_DIFF_MODE_CH4 : 1; /*!< [9..9] 1-AUX ADC Differencial mode, 0 - Single Ended Mode               */
      __IOM unsigned int
        RESERVED1 : 22; /*!< [31..10] Reserved                                                         */
    } SDC_AUXADC_CONFIG_4_b;
  };

  union {
    __IOM unsigned int
      SDC_AUXOPAMP_CONFIG_2; /*!< (@ 0x00000034) SDC AUX OPAMP Configuration 2                              */

    struct {
      __IOM unsigned int SDC_OPAMP_EN_CH2 : 1;       /*!< [0..0] Enable signal for turning OPAMP to used for Channel-2
                                                      Operation                                                                 */
      __IOM unsigned int SDC_OPAMP_IN_P_SEL_CH2 : 4; /*!< [4..1] Configuration register for selecting P Input of OPAMP1
                                                      for Channel-2                                                             */
      __IOM unsigned int SDC_OPAMP_EN_CH3 : 1;       /*!< [5..5] Enable signal for turning OPAMP to used for Channel-4
                                                      Operation                                                                 */
      __IOM unsigned int SDC_OPAMP_IN_P_SEL_CH3 : 4; /*!< [9..6] Configuration register for selecting P Input of OPAMP1
                                                     for Channel-3                                                             */
      __IOM unsigned int SDC_OPAMP_EN_CH4 : 1;       /*!< [10..10] Enable signal for turning OPAMP to used for Channel-4
                                                     Operation                                                                 */
      __IOM unsigned int
        SDC_OPAMP_IN_P_SEL_CH4 : 1; /*!< [11..11] Configuration register for selecting P Input of OPAMP1
                                                      for Channel-4                                                             */
      unsigned int : 3;
      __IOM unsigned int
        RESERVED1 : 17; /*!< [31..15] Reserved                                                         */
    } SDC_AUXOPAMP_CONFIG_2_b;
  };
  __IOM SDC_SDC_DATA_REG_Type
    SDC_DATA_REG[16]; /*!< (@ 0x00000038) SDC Data registers (0-15)                                  */
} SDC_Type;
/** @} End of group RSI_DEVICE_SYSRTC */

#include "si91x_mvp.h"

#endif

/** @} */ /* End of group Device_Peripheral_peripherals */

/* ===========================================================================================================================
  */
/* ================                          Device Specific Peripheral Address
  * Map                           ================ */
/* ===========================================================================================================================
  */

/** @addtogroup Device_Peripheral_peripheralAddr
 * @{
 */
#define BOD_BASE               0x2405A780UL
#define I2C0_BASE              0x44010000UL
#define I2C1_BASE              0x47040000UL
#define I2C2_BASE              0x24040000UL
#define MCPWM_BASE             0x47070000UL
#define UDMA0_BASE             0x44030000UL
#define UDMA1_BASE             0x24078000UL
#define GPDMA_G_BASE           0x21080000UL
#define GPDMA_C_BASE           0x21081004UL
#define HWRNG_BASE             0x45090000UL
#define TIMERS_BASE            0x24042000UL
#define QEI_BASE               0x47060000UL
#define USART0_BASE            0x44000100UL
#define UART0_BASE             0x44000000UL
#define UART1_BASE             0x45020000UL
#define ULP_UART_BASE          0x24041800UL
#define GSPI0_BASE             0x45030000UL
#define SSI0_BASE              0x44020000UL
#define SSISlave_BASE          0x45010000UL
#define SSI2_BASE              0x24040800UL
#define SIO_BASE               0x47000000UL
#define QSPI_BASE              0x12000000UL
#define CRC_BASE               0x45080000UL
#define EFUSE_BASE             0x4600C000UL
#define I2S0_BASE              0x47050000UL
#define I2S1_BASE              0x24040400UL
#define IID_AES_BASE           0x20480500UL
#define IID_QK_BASE            0x20480600UL
#define IID_RPINE_BASE         0x20480400UL
#define CT0_BASE               0x45060000UL
#define CT1_BASE               0x45060100UL
#define CT2_BASE               0x45061000UL
#define CT3_BASE               0x45061100UL
#define CT_MUX_REG_BASE        0x4506F000UL
#define EGPIO_BASE             0x46130000UL
#define EGPIO1_BASE            0x2404C000UL
#define SDIO0_BASE             0x20200000UL
#define SPI_SLAVE_BASE         0x20200000UL
#define M4CLK_BASE             0x46000000UL
#define TIME_PERIOD_BASE       0x24048200UL
#define MCU_WDT_BASE           0x24048300UL
#define RTC_BASE               0x2404821CUL
#define BATT_FF_BASE           0x24048400UL
#define MCU_FSM_BASE           0x24048100UL
#define MCU_ProcessSensor_BASE 0x24048540UL
#define MCU_RET_BASE           0x24048600UL
#define MCU_TEMP_BASE          0x24048500UL
#define MCU_AON_BASE           0x24048000UL
#define ULPCLK_BASE            0x24041400UL
#define SDC_BASE               0x24042400UL

#if defined(SLI_SI917B0)
#define SYSRTC_BASE 0x24048C00UL

#define MVP_S_BASE  (0x24000000UL) /* MVP_S base address */
#define MVP_NS_BASE (0x24000000UL) /* MVP_NS base address */

#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_MVP_S)) \
     || (defined(SL_TRUSTZONE_PERIPHERAL_MVP_S) && (SL_TRUSTZONE_PERIPHERAL_MVP_S == 1)))
#define MVP_BASE (MVP_S_BASE) /* MVP base address */
#else
#define MVP_BASE (MVP_NS_BASE) /* MVP base address */
#endif                         /* SL_TRUSTZONE_PERIPHERAL_MVP_S */
#endif

#if ((defined ENHANCED_FIM) && (defined INST_BUFF_ENABLE))
/* Asign some random ULPSS memory when instruction buffer is enabled */
#define FIM_BASE 0x24060100UL
#else
#define FIM_BASE 0x24070000UL
#endif
#define NWP_FSM_BASE          0x41300110UL
#define OPAMP_BASE            0x24043A14UL
#define AUX_ADC_DAC_COMP_BASE 0x24043800UL
#define IR_BASE               0x24040C00UL
#define MISC_CONFIG_BASE      0x46008000UL

/** @} */ /* End of group Device_Peripheral_peripheralAddr */

/* ===========================================================================================================================
  */
/* ================                                  Peripheral declaration
  * ================ */
/* ===========================================================================================================================
  */

/** @addtogroup Device_Peripheral_declaration
  * @{
  */

#define BOD               ((BOD_Type *)BOD_BASE)
#define I2C0              ((I2C0_Type *)I2C0_BASE)
#define I2C1              ((I2C0_Type *)I2C1_BASE)
#define I2C2              ((I2C0_Type *)I2C2_BASE)
#define MCPWM             ((MCPWM_Type *)MCPWM_BASE)
#define UDMA0             ((UDMA0_Type *)UDMA0_BASE)
#define UDMA1             ((UDMA0_Type *)UDMA1_BASE)
#define GPDMA_G           ((GPDMA_G_Type *)GPDMA_G_BASE)
#define GPDMA_C           ((GPDMA_C_Type *)GPDMA_C_BASE)
#define HWRNG             ((HWRNG_Type *)HWRNG_BASE)
#define TIMERS            ((TIMERS_Type *)TIMERS_BASE)
#define QEI               ((QEI_Type *)QEI_BASE)
#define USART0            ((USART0_Type *)USART0_BASE)
#define UART0             ((USART0_Type *)UART0_BASE)
#define UART1             ((USART0_Type *)UART1_BASE)
#define ULP_UART          ((USART0_Type *)ULP_UART_BASE)
#define GSPI0             ((GSPI0_Type *)GSPI0_BASE)
#define SSI0              ((SSI0_Type *)SSI0_BASE)
#define SSISlave          ((SSI0_Type *)SSISlave_BASE)
#define SSI2              ((SSI0_Type *)SSI2_BASE)
#define SIO               ((SIO_Type *)SIO_BASE)
#define QSPI              ((QSPI_Type *)QSPI_BASE)
#define CRC               ((CRC_Type *)CRC_BASE)
#define EFUSE             ((EFUSE_Type *)EFUSE_BASE)
#define I2S0              ((I2S0_Type *)I2S0_BASE)
#define I2S1              ((I2S0_Type *)I2S1_BASE)
#define IID_AES           ((IID_AES_Type *)IID_AES_BASE)
#define IID_QK            ((IID_QK_Type *)IID_QK_BASE)
#define IID_RPINE         ((IID_RPINE_Type *)IID_RPINE_BASE)
#define CT0               ((CT0_Type *)CT0_BASE)
#define CT1               ((CT0_Type *)CT1_BASE)
#define CT2               ((CT0_Type *)CT2_BASE)
#define CT3               ((CT0_Type *)CT3_BASE)
#define CT_MUX_REG        ((CT_MUX_REG_Type *)CT_MUX_REG_BASE)
#define EGPIO             ((EGPIO_Type *)EGPIO_BASE)
#define EGPIO1            ((EGPIO_Type *)EGPIO1_BASE)
#define SDIO0             ((SDIO0_Type *)SDIO0_BASE)
#define SPI_SLAVE         ((SPI_SLAVE_Type *)SPI_SLAVE_BASE)
#define M4CLK             ((M4CLK_Type *)M4CLK_BASE)
#define TIME_PERIOD       ((TIME_PERIOD_Type *)TIME_PERIOD_BASE)
#define MCU_WDT           ((MCU_WDT_Type *)MCU_WDT_BASE)
#define RTC               ((RTC_Type *)RTC_BASE)
#define BATT_FF           ((BATT_FF_Type *)BATT_FF_BASE)
#define MCU_FSM           ((MCU_FSM_Type *)MCU_FSM_BASE)
#define MCU_ProcessSensor ((MCU_ProcessSensor_Type *)MCU_ProcessSensor_BASE)
#define MCU_RET           ((MCU_RET_Type *)MCU_RET_BASE)
#define MCU_TEMP          ((MCU_TEMP_Type *)MCU_TEMP_BASE)
#define MCU_AON           ((MCU_AON_Type *)MCU_AON_BASE)
#define ULPCLK            ((ULPCLK_Type *)ULPCLK_BASE)
#define FIM               ((FIM_Type *)FIM_BASE)
#define NWP_FSM           ((NWP_FSM_Type *)NWP_FSM_BASE)
#define OPAMP             ((OPAMP_Type *)OPAMP_BASE)
#define AUX_ADC_DAC_COMP  ((AUX_ADC_DAC_COMP_Type *)AUX_ADC_DAC_COMP_BASE)
#define IR                ((IR_Type *)IR_BASE)
#define MISC_CONFIG       ((MISC_CONFIG_Type *)MISC_CONFIG_BASE)
#define SDC               ((SDC_Type *)SDC_BASE)
#define ULP_I2C           I2C2 // Renaming I2C2 base address as ULP_I2C
#if defined(SLI_SI917B0)
#define SYSRTC0 ((SYSRTC_TypeDef *)SYSRTC_BASE)
#define MVP     ((MVP_TypeDef *)MVP_BASE) /**< MVP base pointer */
#endif
/** @} */ /* End of group Device_Peripheral_declaration */

/* =========================================  End of section using anonymous
  * unions  ========================================= */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__ICCARM__)
                                                       /* leave anonymous unions enabled */
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
#pragma warning restore
#elif defined(__CSMC__)
/* anonymous unions are enabled by default */
#endif

#ifdef __cplusplus
}
#endif

#endif /* RS1XXXX_H */

/** @} */ /* End of group RS1xxxx */

/** @} */ /* End of group Silicon Lab Inc. */
