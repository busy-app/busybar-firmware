/******************************************************************************
* @file  RS1xxxx.h
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

/** @addtogroup Redpine Signals Inc.
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

/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum {
  /* -------------------  Cortex-M4 Processor Exceptions Numbers  ------------------- */
  Reset_IRQn            = -15, /*!<   1  Reset Vector, invoked on Power up and warm reset                 */
  NonMaskableInt_IRQn   = -14, /*!<   2  Non maskable Interrupt, cannot be stopped or preempted           */
  HardFault_IRQn        = -13, /*!<   3  Hard Fault, all classes of Fault                                 */
  MemoryManagement_IRQn = -12, /*!<   4  Memory Management, MPU mismatch, including Access Violation and No Match */
  BusFault_IRQn     = -11, /*!<   5  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory related Fault */
  UsageFault_IRQn   = -10, /*!<   6  Usage Fault, i.e. Undef Instruction, Illegal State Transition   */
  SVCall_IRQn       = -5,  /*!<  11  System Service Call via SVC instruction                          */
  DebugMonitor_IRQn = -4,  /*!<  12  Debug Monitor                                                    */
  PendSV_IRQn       = -2,  /*!<  14  Pendable request for system service                              */
  SysTick_IRQn      = -1,  /*!<  15  System Tick Timer                                                */
  /* ---------------------  RS1xxxx Specific Interrupt Numbers  --------------------- */
  VAD_INTR_PING_IRQn               = 0, /*!<   VAD ping interrupt                                                   */
  VAD_INTR_PONG_IRQn               = 1, /*!<   VAD pong interrupt                                                   */
  TIMER0_IRQn                      = 2, /*!<   1  TIMER0                                                            */
  TIMER1_IRQn                      = 3, /*!<   2  TIMER1                                                            */
  TIMER2_IRQn                      = 4, /*!<   3  TIMER2                                                            */
  TIMER3_IRQn                      = 5,
  CAP_SENSOR_IRQn                  = 6,
  COMP2_IRQn                       = 7,  /*!< 7  COMP2                                                               */
  COMP1_IRQn                       = 8,  /*!< 8  COMP1                                                               */
  UDMA1_IRQn                       = 10, /*!< 10  UDMA1                                                              */
  ADC_IRQn                         = 11, /*!< 10  UDMA1                                                             */
  ULPSS_UART_IRQn                  = 12, /*!<  12  ULPSS USART/UART                                                 */
  I2C2_IRQn                        = 13, /*!<  13  I2C2                                                             */
  I2S1_IRQn                        = 14, /*!<  14  I2S2 							     */
  IR_DECODER_IRQ                   = 15,
  SSI2_IRQn                        = 16, /*!<  14  SSI2                                                             */
  FIM_IRQn                         = 17,
  ULP_EGPIO_PIN_IRQn               = 18,
  ULP_EGPIO_GROUP_IRQn             = 19,
  NPSS_TO_MCU_WDT_INTR_IRQn        = 20, /*!<  20 WDT interrupt */
  NPSS_TO_MCU_GPIO_INTR_IRQn       = 21,
  NPSS_TO_MCU_CMP_RF_WKP_INTR_IRQn = 22,
  NPSS_TO_MCU_BOD_INTR_IRQn        = 23,
  NPSS_TO_MCU_BUTTON_INTR_IRQn     = 24,
  NPSS_TO_MCU_SDC_INTR_IRQn        = 25,
  NPSS_TO_MCU_WIRELESS_INTR_IRQn   = 26,
  NPSS_MCU_INTR_IRQn               = 27,
  MCU_CAL_ALARM_IRQn               = 28, /*!<  Alarm interrupt 						     */
  MCU_CAL_RTC_IRQn                 = 29, /*!<  Alarm interrupt 						     */
  GPDMA_IRQn                       = 31, /*!<  31  RPDMA                                                            */
  UDMA0_IRQn                       = 33, /*!<  33  UDMA0                                                            */
  CT_IRQn                          = 34, /*!<  34  CT                                                               */
  HIF0_IRQn                        = 35, /*!<  35  HIF0                                                             */
  HIF1_IRQn                        = 36, /*!<  36  HIF1                                                             */
  SIO_IRQn                         = 37, /*!<  37  SIO                                                              */
  USART0_IRQn                      = 38, /*!<  38  USART0                                                           */
  UART1_IRQn                       = 39, /*!<  39  USART1  							     */
  EGPIO_WAKEUP_IRQn                = 41, /*!<  41  EGPIO_WAKEUP						     */
  I2C0_IRQn                        = 42, /*!<  42  I2C0                                                             */
  SSI1_IRQn                        = 44, /*!<  44  SSI1                                                             */
  GSPI0_IRQn                       = 46, /*!<  46  GSPI0                                                            */
  SSI0_IRQn                        = 47, /*!<  47  SSI0                                                             */
  MCPWM_IRQn                       = 48, /*!<  48  MCPWM                                                            */
  QEI_IRQn                         = 49, /*!<  23  QEI                  					     */
  EGPIO_GROUP_0_IRQn               = 50, /*!<  50  EGPIO_GROUP_0                                                    */
  EGPIO_GROUP_1_IRQn               = 51, /*!<  51  EGPIO_GROUP_1                                                    */
  EGPIO_PIN_0_IRQn                 = 52, /*!<  52  EGPIO_PIN_0                                                      */
  EGPIO_PIN_1_IRQn                 = 53, /*!<  53  EGPIO_PIN_1                                                      */
  EGPIO_PIN_2_IRQn                 = 54, /*!<  54  EGPIO_PIN_2                                                      */
  EGPIO_PIN_3_IRQn                 = 55, /*!<  55  EGPIO_PIN_3                                                      */
  EGPIO_PIN_4_IRQn                 = 56, /*!<  56  EGPIO_PIN_4                                                      */
  EGPIO_PIN_5_IRQn                 = 57, /*!<  57  EGPIO_PIN_5                                                      */
  EGPIO_PIN_6_IRQn                 = 58, /*!<  58  EGPIO_PIN_6                                                      */
  EGPIO_PIN_7_IRQn                 = 59, /*!<  59  EGPIO_PIN_7                                                      */
  QSPI_IRQn                        = 60, /*!<  60  QSPI                                                             */
  I2C1_IRQn                        = 61, /*!<  61  I2C1                                                             */
  ETHERNET_IRQn                    = 62, /*!<  62  ETHERNET                                                         */
  ETHERNET_PMT_IRQn                = 63, /*!<  63  ETHERNET PMT                                                     */
  I2S0_IRQn                        = 64,
  CAN1_IRQn                        = 66, /*!<  67  CAN1                                                             */
  SDMEM_IRQn                       = 68, /*!<  68  SDMEM interrput						     */
  PLL_CLOCK_IRQn                   = 69, /*!<  69  PLL CLOCK INTERRUPT 					     */
  CCI_IRQn                         = 71, /*!<  71  CCI                                                              */
  USB_IRQn                         = 73, /*!<  73  USB                                                              */
  TASS_P2P_IRQn                    = 74  /*!<  NWP to M4 Interrupt						     */
} IRQn_Type;

/** @addtogroup Configuration_of_CMSIS
 * @{
 */

/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* ----------------Configuration of the Cortex-M4 Processor and Core Peripherals---------------- */
#define __CM4_REV              0x0100 /*!< Cortex-M4 Core Revision                                               */
#define __MPU_PRESENT          1      /*!< MPU present or not                                                    */
#define __NVIC_PRIO_BITS       4      /*!< Number of Bits used for Priority Levels                               */
#define __Vendor_SysTickConfig 0      /*!< Set to 1 if different SysTick Config is used                          */
#define __FPU_PRESENT          1      /*!< FPU present or not                                                    */
/** @} */                             /* End of group Configuration_of_CMSIS */

#include "core_cm4.h"     /*!< Cortex-M4 processor and core peripherals                              */
#include "system_si91x.h" /*!< Si91x System                                                        */

/* ================================================================================ */
/* ================       Device Specific Peripheral Section       ================ */
/* ================================================================================ */

/** @addtogroup Device_Peripheral_Registers
 * @{
 */

/* -------------------  Start of section using anonymous unions  ------------------ */
#if defined(__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
#pragma warning 586
#else
#warning Not supported compiler type
#endif

/* =========================================================================================================================== */
/* ================                                          GPDMA_C                                          ================ */
/* =========================================================================================================================== */

/**
  * @brief GPDMA_C_CHANNEL_CONFIG [CHANNEL_CONFIG] ([0..7])
  */
typedef struct {
  union {
    __IOM uint32_t LINK_LIST_PTR_REGS; /*!< (@ 0x00000000) Link List Register for channel 0 to 7                      */

    struct {
      __IOM uint32_t LINK_LIST_PTR_REG_CHNL : 32; /*!< [31..0] This is the address of the memory location from which
                                                     we get our next descriptor                                                */
    } LINK_LIST_PTR_REGS_b;
  };

  union {
    __IOM uint32_t SRC_ADDR_REG_CHNL; /*!< (@ 0x00000004) Source Address Register for channel 0 to 7                 */

    struct {
      __IOM uint32_t SRC_ADDR : 32; /*!< [31..0] This is the address of the memory location from which
                                                     we get our next descriptor                                                */
    } SRC_ADDR_REG_CHNL_b;
  };

  union {
    __IOM uint32_t DEST_ADDR_REG_CHNL; /*!< (@ 0x00000008) Source Address Register for channel 0 to 7                 */

    struct {
      __IOM uint32_t DEST_ADDR : 32; /*!< [31..0] This is the destination address to whih the data is
                                                     sent                                                                      */
    } DEST_ADDR_REG_CHNL_b;
  };

  union {
    __IOM uint32_t
      CHANNEL_CTRL_REG_CHNL; /*!< (@ 0x0000000C) Channel Control Register for channel 0 to 7                */

    struct {
      __IOM uint32_t DMA_BLK_SIZE : 12; /*!< [11..0] This is data to be transmitted. Loaded at the beginning
                                                     of the DMA transfer and decremented at every dma transaction.             */
      __IOM uint32_t TRNS_TYPE : 2; /*!< [13..12] DMA transfer type                                                */
      __IOM uint32_t
        DMA_FLOW_CTRL : 2; /*!< [15..14] DMA flow control                                                 */
      __IOM uint32_t
        MSTR_IF_FETCH_SEL : 1; /*!< [16..16] This selects the MASTER IF from which data to be fetched         */
      __IOM uint32_t
        MSTR_IF_SEND_SEL : 1; /*!< [17..17] This selects the MASTER IF from which data to be sent            */
      __IOM uint32_t
        DEST_DATA_WIDTH : 2; /*!< [19..18] Data transfer to destination.                                    */
      __IOM uint32_t
        SRC_DATA_WIDTH : 2;            /*!< [21..20] Data transfer from source.                                       */
      __IOM uint32_t SRC_ALIGN : 1;    /*!< [22..22] Reserved.Value set to 0 We do not do any singles. 
                                                     We just do burst, save first 3 bytes in to residue buffer
                                                     in one cycle,  In the next cycle send 4 bytes to fifo,
                                                     save 3 bytes in to residue. This continues on.                            */
      __IOM uint32_t LINK_LIST_ON : 1; /*!< [23..23] This mode is set, when we do link listed operation               */
      __IOM uint32_t
        LINK_LIST_MSTR_SEL : 1; /*!< [24..24] This mode is set, when we do link listed operation               */
      __IOM uint32_t
        SRC_ADDR_CONTIGUOUS : 1; /*!< [25..25] Indicates Address is contiguous from previous                    */
      __IOM uint32_t
        DEST_ADDR_CONTIGUOUS : 1; /*!< [26..26] Indicates Address is contiguous from previous                    */
      __IOM uint32_t RETRY_ON_ERROR : 1; /*!< [27..27] When this bit is set, if we recieve HRESPERR, We will
                                                     retry the DMA for that channel.                                           */
      __IOM uint32_t LINK_INTERRUPT : 1; /*!< [28..28] This bit is set in link list descriptor.Hard ware will
                                                     send an interrupt when the DMA transfer is done for the
                                                     corresponding link list address                                           */
      __IOM uint32_t SRC_FIFO_MODE : 1;  /*!< [29..29] If set to 1; source address will not be incremented(means
                                                     fifo mode for source)                                                     */
      __IOM uint32_t DEST_FIFO_MODE : 1; /*!< [30..30] If set to 1; destination address will not be incremented(means
                                                     fifo mode for destination)                                                */
      __IM uint32_t RESERVED1 : 1; /*!< [31..31] Reserved1                                                        */
    } CHANNEL_CTRL_REG_CHNL_b;
  };

  union {
    __IOM uint32_t
      MISC_CHANNEL_CTRL_REG_CHNL; /*!< (@ 0x00000010) Misc Channel Control Register for channel 0                */

    struct {
      __IOM uint32_t
        AHB_BURST_SIZE : 3; /*!< [2..0] Burst size                                                         */
      __IOM uint32_t DEST_DATA_BURST : 6; /*!< [8..3] Burst writes in beats to destination.(000000-64 beats
                                                     .....111111-63 beats)                                                     */
      __IOM uint32_t SRC_DATA_BURST : 6;  /*!< [14..9] Burst writes in beats from source(000000-64 beats .....111111-63
                                                     beats)                                                                    */
      __IOM uint32_t DEST_CHNL_ID : 6;    /*!< [20..15] This is the destination channel Id to which the data
                                                     is sent. Must be set up prior to DMA_CHANNEL_ENABLE                       */
      __IOM uint32_t SRC_CHNL_ID : 6;     /*!< [26..21] This is the source channel Id, from which the data
                                                     is fetched. must be set up prior to DMA_CHANNEL_ENABLE                    */
      __IOM uint32_t DMA_PROT : 3;        /*!< [29..27] Protection level to go with the data. It will be concatenated
                                                     with 1 b1 as there will be no opcode fetching and directly
                                                     assign to hprot in AHB interface                                          */
      __IOM uint32_t
        MEM_FILL_ENABLE : 1;           /*!< [30..30] Enable for memory filling with either 1s or 0s.                  */
      __IOM uint32_t MEM_ONE_FILL : 1; /*!< [31..31] Select for memory filling with either 1s or 0s.                  */
    } MISC_CHANNEL_CTRL_REG_CHNL_b;
  };

  union {
    __IOM uint32_t FIFO_CONFIG_REGS; /*!< (@ 0x00000014) FIFO Configuration Register for channel 1                  */

    struct {
      __IOM uint32_t
        FIFO_STRT_ADDR : 6;         /*!< [5..0] Starting row address of channel                                    */
      __IOM uint32_t FIFO_SIZE : 6; /*!< [11..6] Channel size                                                      */
      __IM uint32_t RESERVED1 : 20; /*!< [31..12] Reserved1                                                        */
    } FIFO_CONFIG_REGS_b;
  };

  union {
    __IOM uint32_t PRIORITY_CHNL_REGS; /*!< (@ 0x00000018) Priority Register for channel 0 to 7                       */

    struct {
      __IOM uint32_t PRIORITY_CH : 2; /*!< [1..0] Set a value between 2 b00 to 2 b11. The channel having
                                                     highest number is the highest priority channel.                           */
      __IM uint32_t RESERVED1 : 30;   /*!< [31..2] Reserved1                                                         */
    } PRIORITY_CHNL_REGS_b;
  };
  __IM uint32_t RESERVED[57];
} GPDMA_C_CHANNEL_CONFIG_Type; /*!< Size = 256 (0x100)                                                        */

/**
  * @brief GPDMA_G_GLOBAL [GLOBAL] (GLOBAL)
  */
typedef struct {
  union {
    __IOM uint32_t INTERRUPT_REG; /*!< (@ 0x00000000) Interrupt Register                                         */

    struct {
      __IOM uint32_t
        GPDMAC_INT_STAT : 8;        /*!< [7..0] Interrupt Status                                                   */
      __IM uint32_t RESERVED1 : 24; /*!< [31..8] reserved1                                                         */
    } INTERRUPT_REG_b;
  };

  union {
    __IOM uint32_t INTERRUPT_MASK_REG; /*!< (@ 0x00000004) Interrupt Mask Register                                    */

    struct {
      __IOM uint32_t RESERVED1 : 8; /*!< [7..0] reserved1                                                          */
      __IOM uint32_t LINK_LIST_FETCH_MASK : 8; /*!< [15..8] Linked list fetch done interrupt bit mask control. By
                                                     default, descriptor fetch done interrupt is masked.                       */
      __IOM uint32_t
        TFR_DONE_MASK : 8;          /*!< [23..16] Transfer done interrupt bit mask control.                        */
      __IOM uint32_t RESERVED2 : 8; /*!< [31..24] reserved2                                                        */
    } INTERRUPT_MASK_REG_b;
  };

  union {
    __IOM uint32_t INTERRUPT_STAT_REG; /*!< (@ 0x00000008) Interrupt status register                                  */

    struct {
      __IOM uint32_t HRESP_ERR0 : 1; /*!< [0..0] DMA error bit                                                      */
      __IOM uint32_t LINK_LIST_FETCH_DONE0 : 1; /*!< [1..1] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 0                                                  */
      __IOM uint32_t TFR_DONE0 : 1;             /*!< [2..2] This bit indicates the status of DMA transfer done interrupt
                                                     for channel 0                                                             */
      __IOM uint32_t GPDMAC_ERR0 : 1; /*!< [3..3] transfer size or burst size or h size mismatch error               */
      __IOM uint32_t HRESP_ERR1 : 1;  /*!< [4..4] HRESP error bit                                                    */
      __IOM uint32_t LINK_LIST_FETCH_DONE1 : 1; /*!< [5..5] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 1                                                  */
      __IOM uint32_t TFR_DONE1 : 1;             /*!< [6..6] This bit indicates the status of DMA transfer done interrupt
                                                     for channel 1.                                                            */
      __IOM uint32_t GPDMAC_ERR1 : 1; /*!< [7..7] transfer size or burst size or h size mismatch error               */
      __IOM uint32_t HRESP_ERR2 : 1;  /*!< [8..8] HRESP error bit                                                    */
      __IOM uint32_t LINK_LIST_FETCH_DONE2 : 1; /*!< [9..9] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 2.                                                 */
      __IOM uint32_t TFR_DONE2 : 1;             /*!< [10..10] This bit indicates the status of DMA transfer done
                                                     interrupt for channel 2.                                                  */
      __IOM uint32_t GPDMAC_ERR2 : 1; /*!< [11..11] transfer size or burst size or h size mismatch error             */
      __IOM uint32_t HRESP_ERR3 : 1;  /*!< [12..12] HRESP error bit                                                  */
      __IOM uint32_t LINK_LIST_FETCH_DONE3 : 1; /*!< [13..13] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 3.                                                 */
      __IOM uint32_t TFR_DONE3 : 1;             /*!< [14..14] This bit indicates the status of DMA transfer done
                                                     interrupt for channel 3.                                                  */
      __IOM uint32_t GPDMAC_ERR3 : 1; /*!< [15..15] transfer size or burst size or h size mismatch error             */
      __IOM uint32_t HRESP_ERR4 : 1;  /*!< [16..16] HRESP error bit                                                  */
      __IOM uint32_t LINK_LIST_FETCH_DONE4 : 1; /*!< [17..17] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 4.                                                 */
      __IOM uint32_t TFR_DONE4 : 1;             /*!< [18..18] This bit indicates the status of DMA transfer done
                                                     interrupt for channel 4.                                                  */
      __IOM uint32_t GPDMAC_ERR4 : 1; /*!< [19..19] transfer size or burst size or h size mismatch error             */
      __IOM uint32_t HRESP_ERR5 : 1;  /*!< [20..20] HRESP error bit                                                  */
      __IOM uint32_t LINK_LIST_FETCH_DONE5 : 1; /*!< [21..21] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 5.                                                 */
      __IOM uint32_t TFR_DONE5 : 1;             /*!< [22..22] This bit indicates the status of DMA transfer done
                                                     interrupt for channel 5.                                                  */
      __IOM uint32_t GPDMAC_ERR5 : 1; /*!< [23..23] transfer size or burst size or h size mismatch error             */
      __IM uint32_t HRESP_ERR6 : 1;   /*!< [24..24] HRESP error bit                                                  */
      __IOM uint32_t LINK_LIST_FETCH_DONE6 : 1; /*!< [25..25] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 6.                                                 */
      __IOM uint32_t TFR_DONE6 : 1;             /*!< [26..26] This bit indicates the status of DMA transfer done
                                                     interrupt for channel 6.                                                  */
      __IOM uint32_t GPDMAC_ERR6 : 1; /*!< [27..27] transfer size or burst size or h size mismatch error             */
      __IOM uint32_t HRESP_ERR7 : 1;  /*!< [28..28] HRESP error bit                                                  */
      __IOM uint32_t LINK_LIST_FETCH_DONE7 : 1; /*!< [29..29] This bit indicates the status of linked list descriptor
                                                     fetch done for channel 7.                                                 */
      __IOM uint32_t TFR_DONE7 : 1;             /*!< [30..30] This bit indicates the status of DMA transfer done
                                                     interrupt for channel 7.                                                  */
      __IOM uint32_t GPDMAC_ERR7 : 1; /*!< [31..31] transfer size or burst size or h size mismatch error             */
    } INTERRUPT_STAT_REG_b;
  };

  union {
    __IOM uint32_t
      DMA_CHNL_ENABLE_REG; /*!< (@ 0x0000000C) This register used for enable DMA channel                  */

    struct {
      __IOM uint32_t CH_ENB : 8;    /*!< [7..0] CWhen a bit is set to one, it indicates, corresponding
                                                     channel is enabled for dma operation                                      */
      __IM uint32_t RESERVED1 : 24; /*!< [31..8] Reserved1                                                         */
    } DMA_CHNL_ENABLE_REG_b;
  };

  union {
    __IOM uint32_t
      DMA_CHNL_SQUASH_REG; /*!< (@ 0x00000010) This register used for enable DMA channel squash           */

    struct {
      __IOM uint32_t CH_DIS : 8;    /*!< [7..0] CPU Will be masked to write zeros, CPU is allowed write
                                                     1 only                                                                    */
      __IM uint32_t RESERVED1 : 24; /*!< [31..8] Reserved1                                                         */
    } DMA_CHNL_SQUASH_REG_b;
  };

  union {
    __IOM uint32_t DMA_CHNL_LOCK_REG; /*!< (@ 0x00000014) This register used for enable DMA channel squash           */

    struct {
      __IOM uint32_t CHNL_LOCK : 8; /*!< [7..0] When set entire DMA block transfer is done, before other
                                                     DMA request is serviced                                                   */
      __IM uint32_t RESERVED1 : 24; /*!< [31..8] Reserved1                                                         */
    } DMA_CHNL_LOCK_REG_b;
  };
} GPDMA_G_GLOBAL_Type; /*!< Size = 24 (0x18)                                                          */

/* =========================================================================================================================== */
/* ================                                          GPDMA_C                                          ================ */
/* =========================================================================================================================== */

/**
  * @brief GPDMAC (dma controller) is an AMBA complaint peripheral unit supports 8-channels (GPDMA_C)
  */

typedef struct { /*!< (@ 0x21081004) GPDMA_C Structure                                          */
  __IOM GPDMA_C_CHANNEL_CONFIG_Type
    CHANNEL_CONFIG[8]; /*!< (@ 0x00000000) [0..7]                                               */
} GPDMA_C_Type;        /*!< Size = 2048 (0x800)                                                       */

/* =========================================================================================================================== */
/* ================                                          GPDMA_G                                          ================ */
/* =========================================================================================================================== */

/**
  * @brief GPDMA is an AMBA complaint peripheral unit supports 8-channels (GPDMA_G)
  */

typedef struct { /*!< (@ 0x21080000) GPDMA_G Structure                                          */
  __IM uint32_t RESERVED[1057];
  __IOM GPDMA_G_GLOBAL_Type GLOBAL; /*!< (@ 0x00001084) GLOBAL                                                     */
} GPDMA_G_Type;                     /*!< Size = 4252 (0x109c)                                                      */

/**
  * @brief TIMERS_MATCH_CTRL [MATCH_CTRL] ([0..3])
  */
typedef struct {
  union {
    __IOM uint32_t MCUULP_TMR_MATCH; /*!< (@ 0x00000000) Timer Match Register                                       */

    struct {
      __IOM uint32_t TMR_MATCH : 32; /*!< (@ 0x00000000) This bits are used to program the lower significant
                                                                    16-bits of timer time out value in millisecond
                                                                    or number of system clocks                                 */
    } MCUULP_TMR_MATCH_b;
  };

  union {
    __IOM uint32_t MCUULP_TMR_CNTRL; /*!< (@ 0x00000004) Timer Control Register                                     */

    struct {
      __OM uint32_t TMR_START : 1;    /*!< (@ 0x00000000) This Bit are Used to start the timer timer gets
                                                                    reset upon setting this bit                                */
      __OM uint32_t TMR_INTR_CLR : 1; /*!< (@ 0x00000001) This Bit are Used to clear the timer                       */
      __IOM uint32_t
        TMR_INTR_ENABLE : 1;         /*!< (@ 0x00000002) This Bit are Used to enable the time out interrupt         */
      __IOM uint32_t TMR_TYPE : 2;   /*!< (@ 0x00000003) This Bit are Used to select the type of timer              */
      __IOM uint32_t TMR_MODE : 1;   /*!< (@ 0x00000005) This Bit are Used to select the mode working
                                                                    of timer                                                   */
      __OM uint32_t TMR_STOP : 1;    /*!< (@ 0x00000006) This Bit are Used to stop the timer                        */
      __IOM uint32_t COUNTER_UP : 1; /*!< (@ 0x00000007) For reading/tracking counter in up counting this
                                                                    bit has to be set                                          */
      __IOM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } MCUULP_TMR_CNTRL_b;
  };
} TIMERS_MATCH_CTRL_Type;

/**
  * @brief EGPIO_PIN_CONFIG [PIN_CONFIG] ([0..79])
  */
typedef struct {
  union {
    __IOM uint32_t GPIO_CONFIG_REG; /*!< (@ 0x00000000) GPIO Configuration Register                                */

    struct {
      __IOM uint32_t DIRECTION : 1; /*!< (@ 0x00000000) Direction of the GPIO pin                                  */
      __IOM uint32_t PORTMASK : 1;  /*!< (@ 0x00000001) Port mask value                                            */
      __IOM uint32_t MODE : 4;      /*!< (@ 0x00000002) GPIO Pin Mode Used for GPIO Pin Muxing                     */
      __IOM uint32_t RESERVED1 : 2; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t
        GROUP_INTERRUPT1_ENABLE : 1; /*!< (@ 0x00000008) When set, the corresponding GPIO is pin is selected
                                                                    for group intr 1 generation                                */
      __IOM uint32_t
        GROUP_INTERRUPT1_POLARITY : 1; /*!< (@ 0x00000009) Decides the active value of the pin to be considered
                                                                    for group interrupt 1 generation                           */
      __IOM uint32_t
        GROUP_INTERRUPT2_ENABLE : 1; /*!< (@ 0x0000000A) When set, the corresponding GPIO is pin is selected
                                                                    for group intr 2 generation                                */
      __IOM uint32_t
        GROUP_INTERRUPT2_POLARITY : 1; /*!< (@ 0x0000000B) Decides the active value of the pin to be considered
                                                                    for group interrupt 2 generation                           */
      __IOM uint32_t RESERVED2 : 4;    /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t RESERVED3 : 16;   /*!< (@ 0x00000010) Reserved3                                                  */
    } GPIO_CONFIG_REG_b;
  };

  union {
    __IOM uint32_t BIT_LOAD_REG; /*!< (@ 0x00000004) Bit Load                                                   */

    struct {
      __IOM uint32_t BIT_LOAD : 1;   /*!< (@ 0x00000000) Loads 0th bit on to the pin on write. And reads
                                                                    the value on pin on read into 0th bit                      */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved1                                                  */
    } BIT_LOAD_REG_b;
  };

  union {
    __IOM uint32_t WORD_LOAD_REG; /*!< (@ 0x00000008) Word Load                                                  */

    struct {
      __IOM uint32_t WORD_LOAD : 16; /*!< (@ 0x00000000) Loads 1 on the pin when any of the bit in load
                                                                    value is 1. On read pass the bit status
                                                                    into all bits.                                             */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) Reserved1                                                  */
    } WORD_LOAD_REG_b;
  };
  __IM uint32_t RESERVED;
} EGPIO_PIN_CONFIG_Type; /*!< Size = 16 (0x10)                                                          */

/**
  * @brief EGPIO_PORT_CONFIG [PORT_CONFIG] ([0..5])
  */
typedef struct {
  union {
    __IOM uint32_t PORT_LOAD_REG; /*!< (@ 0x00000000) Port Load                                                  */

    struct {
      __IOM uint32_t PORT_LOAD : 16; /*!< (@ 0x00000000) Loads the value on to pin on write. And reads
                                                                    the value of load register on read                         */
      __IM uint32_t RES : 16;        /*!< (@ 0x00000010) RES                                                        */
    } PORT_LOAD_REG_b;
  };

  union {
    __OM uint32_t PORT_SET_REG; /*!< (@ 0x00000004) Port Set Register                                          */

    struct {
      __OM uint32_t PORT_SET : 16;  /*!< (@ 0x00000000) Sets the pin when corresponding bit is high.
                                                                    Writing zero has no effect.                                */
      __OM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) Reserved1                                                  */
    } PORT_SET_REG_b;
  };

  union {
    __OM uint32_t PORT_CLEAR_REG; /*!< (@ 0x00000008) Port Clear Register                                        */

    struct {
      __OM uint32_t PORT_CLEAR : 16; /*!< (@ 0x00000000) Clears the pin when corresponding bit is high.
                                                                    Writing zero has no effect.                                */
      __OM uint32_t RESERVED1 : 16;  /*!< (@ 0x00000010) Reserved1                                                  */
    } PORT_CLEAR_REG_b;
  };

  union {
    __OM uint32_t
      PORT_MASKED_LOAD_REG; /*!< (@ 0x0000000C) Port Masked Load Register                                  */

    struct {
      __OM uint32_t PORT_MASKED_LOAD : 16; /*!< (@ 0x00000000) Only loads into pins which are not masked. On
                                                                    read, pass only status unmasked pins                       */
      __OM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) Reserved1                                                  */
    } PORT_MASKED_LOAD_REG_b;
  };

  union {
    __OM uint32_t PORT_TOGGLE_REG; /*!< (@ 0x00000010) Port Toggle Register                                       */

    struct {
      __OM uint32_t PORT_TOGGLE : 16; /*!< (@ 0x00000000) Toggles the pin when corresponding bit is high.
                                                                    Writing zero has not effect.                               */
      __OM uint32_t RESERVED1 : 16;   /*!< (@ 0x00000010) Reserved1                                                  */
    } PORT_TOGGLE_REG_b;
  };

  union {
    __IM uint32_t PORT_READ_REG; /*!< (@ 0x00000014) Port Read Register                                         */

    struct {
      __IM uint32_t PORT_READ : 16; /*!< (@ 0x00000000) Reads the value on GPIO pins irrespective of
                                                                    the pin mode.                                              */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) Reserved1                                                  */
    } PORT_READ_REG_b;
  };
  __IM uint32_t RESERVED[2];
} EGPIO_PORT_CONFIG_Type; /*!< Size = 32 (0x20)                                                          */

/**
  * @brief EGPIO_INTR [INTR] ([0..5])
  */
typedef struct {
  union {
    __IOM uint32_t GPIO_INTR_CTRL; /*!< (@ 0x00000000) GPIO Interrupt Control Register                            */

    struct {
      __IOM uint32_t LEVEL_HIGH_ENABLE : 1; /*!< (@ 0x00000000) enables interrupt generation when pin level is
                                                                    1                                                          */
      __IOM uint32_t LEVEL_LOW_ENABLE : 1;  /*!< (@ 0x00000001) enables interrupt generation when pin level is
                                                                    0                                                          */
      __IOM uint32_t RISE_EDGE_ENABLE : 1;  /*!< (@ 0x00000002) enables interrupt generation when rising edge
                                                                    is detected on pin                                         */
      __IOM uint32_t FALL_EDGE_ENABLE : 1;  /*!< (@ 0x00000003) enables interrupt generation when Falling edge
                                                                    is detected on pin                                         */
      __IOM uint32_t MASK : 1;              /*!< (@ 0x00000004) Masks the interrupt. Interrupt will still be
                                                                    seen in status register when enabled                       */
      __IOM uint32_t RESERVED1 : 3;   /*!< (@ 0x00000005) Reserved1                                                  */
      __IOM uint32_t PIN_NUMBER : 4;  /*!< (@ 0x00000008) GPIO Pin to be chosen for interrupt generation             */
      __IOM uint32_t PORT_NUMBER : 2; /*!< (@ 0x0000000C) GPIO Port to be chosen for interrupt generation            */
      __IOM uint32_t RESERVED2 : 18;  /*!< (@ 0x0000000E) Reserved2                                                  */
    } GPIO_INTR_CTRL_b;
  };

  union {
    __IOM uint32_t GPIO_INTR_STATUS; /*!< (@ 0x00000004) GPIO Interrupt Status Register                             */

    struct {
      __IOM uint32_t
        INTERRUPT_STATUS : 1; /*!< (@ 0x00000000) Gets set when interrupt is enabled and occurs.             */
      __IOM uint32_t
        RISE_EDGE_STATUS : 1; /*!< (@ 0x00000001) Gets set when rise edge is enabled and occurs.             */
      __IOM uint32_t
        FALL_EDGE_STATUS : 1;        /*!< (@ 0x00000002) Gets set when Fall edge is enabled and occurs.             */
      __OM uint32_t MASK_SET : 1;    /*!< (@ 0x00000003) Mask set                                                   */
      __OM uint32_t MASK_CLEAR : 1;  /*!< (@ 0x00000004) Mask Clear                                                 */
      __IOM uint32_t RESERVED1 : 27; /*!< (@ 0x00000005) Reserved1                                                  */
    } GPIO_INTR_STATUS_b;
  };
} EGPIO_INTR_Type; /*!< Size = 8 (0x8)                                                            */

/**
  * @brief EGPIO_GPIO_GRP_INTR [GPIO_GRP_INTR] ([0..3])
  */
typedef struct {
  union {
    __IOM uint32_t
      GPIO_GRP_INTR_CTRL_REG; /*!< (@ 0x00000000) GPIO Interrupt 0 Control Register                          */

    struct {
      __IOM uint32_t AND_OR : 1;     /*!< (@ 0x00000000) AND/OR                                                     */
      __IOM uint32_t LEVEL_EDGE : 1; /*!< (@ 0x00000001) Level/Edge                                                 */
      __IOM uint32_t ENABLE_WAKEUP : 1; /*!< (@ 0x00000002) For wakeup generation, actual pin status has
                                                                    to be seen(before double ranking point)                    */
      __IOM uint32_t
        ENABLE_INTERRUPT : 1;        /*!< (@ 0x00000003) Enable Interrupt                                           */
      __IOM uint32_t MASK : 1;       /*!< (@ 0x00000004) Mask                                                       */
      __IOM uint32_t RESERVED1 : 27; /*!< (@ 0x00000005) Reserved1                                                  */
    } GPIO_GRP_INTR_CTRL_REG_b;
  };

  union {
    __IOM uint32_t GPIO_GRP_INTR_STS; /*!< (@ 0x00000004) GPIO Interrupt 0 Status Register                           */

    struct {
      __IOM uint32_t INTERRUPT_STATUS : 1; /*!< (@ 0x00000000) Interrupt status is available in this bit when
                                                                    interrupt is enabled and generated. When
                                                                    1 is written, interrupt gets cleared.                      */
      __IM uint32_t WAKEUP : 1;            /*!< (@ 0x00000001) Double ranked version of wakeup. Gets set when
                                                                    wakeup is enabled and occurs. When 1 is
                                                                    written it gets cleared                                    */
      __IOM uint32_t RESERVED1 : 1;  /*!< (@ 0x00000002) Reserved1                                                  */
      __IOM uint32_t MASK_SET : 1;   /*!< (@ 0x00000003) Gives zero on read                                         */
      __IOM uint32_t MASK_CLEAR : 1; /*!< (@ 0x00000004) Gives zero on read                                         */
      __IOM uint32_t RESERVED2 : 27; /*!< (@ 0x00000005) Reserved2                                                  */
    } GPIO_GRP_INTR_STS_b;
  };
} EGPIO_GPIO_GRP_INTR_Type;
/* ================================================================================ */
/* ================                      I2C0                      ================ */
/* ================================================================================ */

/**
 * @brief Inter Integrated Circuit(I2C) is programmable control bus that provides support for the communications link between integrated circuits in a system  (I2C0)
 */

/**
 * @brief Inter Integrated Circuit(I2C) is programmable control bus that provides support for the communications link between integrated circuits in a system  (I2C0)
 */

typedef struct { /*!< (@ 0x44010000) I2C1 Structure                                             */

  union {
    __IOM uint32_t IC_CON; /*!< (@ 0x00000000) This register can be written only when the i2c
                                                                    is disabled, which corresponds to IC_ENABLE[0]
                                                                    being set to 0. Writes at other times have
                                                                    no effect.                                                 */

    struct {
      __IOM uint32_t MASTER_MODE : 1; /*!< (@ 0x00000000) This bit controls whether the I2C master is enabled.       */
      __IOM uint32_t SPEED : 2;       /*!< (@ 0x00000001) These bits control at which speed the I2C operates.
                                                                    Hardware protects against illegal values
                                                                    being programmed by software.                              */
      __IOM uint32_t IC_10BITADDR_SLAVE : 1; /*!< (@ 0x00000003) When acting as a slave, this bit controls whether
                                                                    the I2C responds to 7- or 10-bit addresses.                */
      __IM uint32_t IC_10BITADDR_MASTER_RD_ONLY : 1; /*!< (@ 0x00000004) the function of this bit is handled by bit 12
                                                                    of IC_TAR register, and becomes a read-only
                                                                    copy called IC_10BITADDR_MASTER_rd_only                    */
      __IOM uint32_t IC_RESTART_EN : 1;              /*!< (@ 0x00000005) Determines whether RESTART conditions may be
                                                                    sent when acting as a master                               */
      __IOM uint32_t
        IC_SLAVE_DISABLE : 1; /*!< (@ 0x00000006) This bit controls whether I2C has its slave disabled       */
      __IOM uint32_t STOP_DET_IFADDRESSED : 1; /*!< (@ 0x00000007) The STOP DETECTION interrupt is generated only
                                                                    when the transmitted address matches the
                                                                    slave address of SAR                                       */
      __IOM uint32_t TX_EMPTY_CTRL : 1;        /*!< (@ 0x00000008) This bit controls the generation of the TX EMPTY
                                                                    interrupt, as described in the IC RAW INTR
                                                                    STAT register.                                             */
      __IM uint32_t RESERVED1 : 1; /*!< (@ 0x00000009) reserved1                                                  */
      __IOM uint32_t
        STOP_DET_IF_MASTER_ACTIVE : 1; /*!< (@ 0x0000000A) In Master mode.                                         */
      __IOM uint32_t
        BUS_CLEAR_FEATURE_CTRL : 1;  /*!< (@ 0x0000000B) In Master mode.                                            */
      __IOM uint32_t RESERVED2 : 20; /*!< (@ 0x0000000C) reserved2                                                  */
    } IC_CON_b;
  };

  union {
    __IOM uint32_t IC_TAR; /*!< (@ 0x00000004) I2C Target Address Register                                */

    struct {
      __IOM uint32_t IC_TAR : 10;     /*!< (@ 0x00000000) This is the target address for any master transaction      */
      __IOM uint32_t GC_OR_START : 1; /*!< (@ 0x0000000A) If bit 11 (SPECIAL) is set to 1 and bit 13 (Device-ID)
                                                                    is set to 0, then this bit indicates whether
                                                                    a General Call or START byte command is
                                                                    to be performed by the I2C                                 */
      __IOM uint32_t SPECIAL : 1;     /*!< (@ 0x0000000B) This bit indicates whether software performs
                                                                    a General Call or START BYTE command                       */
      __IOM uint32_t IC_10BITADDR_MASTER : 1; /*!< (@ 0x0000000C) This bit controls whether the i2c starts its
                                                                    transfers in 7-or 10-bit addressing mode
                                                                    when acting as a master                                    */
      __IOM uint32_t DEVICE_ID : 1;           /*!< (@ 0x0000000D) If bit 11 (SPECIAL) is set to 1, then this bit
                                                                    indicates whether a Device-ID of a particular
                                                                    slave mentioned in IC_TAR[6:0] is to be
                                                                    performed by the I2C Master                                */
      __IM uint32_t RESERVED1 : 18; /*!< (@ 0x0000000E) reserved1                                                  */
    } IC_TAR_b;
  };

  union {
    __IOM uint32_t IC_SAR; /*!< (@ 0x00000008) I2C Slave Address Register                                 */

    struct {
      __IOM uint32_t IC_SAR : 10;   /*!< (@ 0x00000000) The IC_SAR holds the slave address when the I2C
                                                                    is operating as a slave. For 7-bit addressing,
                                                                    only IC_SAR[6:0] is used.                                  */
      __IM uint32_t RESERVED1 : 22; /*!< (@ 0x0000000A) reserved1                                                  */
    } IC_SAR_b;
  };

  union {
    __IOM uint32_t IC_HS_MADDR; /*!< (@ 0x0000000C) I2C High Speed Master Mode Code Address Register           */

    struct {
      __IOM uint32_t IC_HS_MAR : 3; /*!< (@ 0x00000000) This bit field holds the value of the I2C HS
                                                                    mode master code                                           */
      __IM uint32_t RESERVED1 : 29; /*!< (@ 0x00000003) reserved1                                                  */
    } IC_HS_MADDR_b;
  };

  union {
    __IOM uint32_t IC_DATA_CMD; /*!< (@ 0x00000010) I2C Rx/Tx Data Buffer and Command Register                 */

    struct {
      __IOM uint32_t DAT : 8;            /*!< (@ 0x00000000) This register contains the data to be transmitted
                                                                    or received on the I2C bus                                 */
      __OM uint32_t CMD : 1;             /*!< (@ 0x00000008) This bit controls whether a read or a write is
                                                                    performed                                                  */
      __OM uint32_t STOP : 1;            /*!< (@ 0x00000009) This bit controls whether a STOP is issued after
                                                                    the byte is sent or received                               */
      __OM uint32_t RESTART : 1;         /*!< (@ 0x0000000A) This bit controls whether a RESTART is issued
                                                                    before the byte is sent or received                        */
      __IM uint32_t FIRST_DATA_BYTE : 1; /*!< (@ 0x0000000B) Indicates the first data byte received after
                                                                    the address phase for receive transfer
                                                                    in Master receiver or Slave receiver mode                  */
      __IM uint32_t RESERVED1 : 20; /*!< (@ 0x0000000C) reserved1                                                  */
    } IC_DATA_CMD_b;
  };

  union {
    __IOM uint32_t IC_SS_SCL_HCNT; /*!< (@ 0x00000014) Standard Speed I2C Clock SCL High Count Register           */

    struct {
      __IOM uint32_t IC_SS_SCL_HCNT : 16; /*!< (@ 0x00000000) This register must be set before any I2C bus
                                                                    transaction can take place to ensure proper
                                                                    I/O timing                                                 */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } IC_SS_SCL_HCNT_b;
  };

  union {
    __IOM uint32_t IC_SS_SCL_LCNT; /*!< (@ 0x00000018) Standard Speed I2C Clock SCL Low Count Register            */

    struct {
      __IOM uint32_t IC_SS_SCL_LCNT : 16; /*!< (@ 0x00000000) This register must be set before any I2C bus
                                                                    transaction can take place to ensure proper
                                                                    I/O timing                                                 */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } IC_SS_SCL_LCNT_b;
  };

  union {
    __IOM uint32_t IC_FS_SCL_HCNT; /*!< (@ 0x0000001C) Fast Speed I2C Clock SCL High Count Register               */

    struct {
      __IOM uint32_t IC_FS_SCL_HCNT : 16; /*!< (@ 0x00000000) This register must be set before any I2C bus
                                                                    transaction can take place to ensure proper
                                                                    I/O timing                                                 */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } IC_FS_SCL_HCNT_b;
  };

  union {
    __IOM uint32_t IC_FS_SCL_LCNT; /*!< (@ 0x00000020) Fast Speed I2C Clock SCL Low Count Register                */

    struct {
      __IOM uint32_t IC_FS_SCL_LCNT : 16; /*!< (@ 0x00000000) This register must be set before any I2C bus
                                                                    transaction can take place to ensure proper
                                                                    I/O timing                                                 */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } IC_FS_SCL_LCNT_b;
  };

  union {
    __IOM uint32_t IC_HS_SCL_HCNT; /*!< (@ 0x00000024) High Speed I2C Clock SCL High Count Register               */

    struct {
      __IOM uint32_t IC_HS_SCL_HCNT : 16; /*!< (@ 0x00000000) This register must be set before any I2C bus
                                                                    transaction can take place to ensure proper
                                                                    I/O timing                                                 */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } IC_HS_SCL_HCNT_b;
  };

  union {
    __IOM uint32_t IC_HS_SCL_LCNT; /*!< (@ 0x00000028) High Speed I2C Clock SCL Low Count Register                */

    struct {
      __IOM uint32_t IC_HS_SCL_LCNT : 16; /*!< (@ 0x00000000) This register must be set before any I2C bus
                                                                    transaction can take place to ensure proper
                                                                    I/O timing                                                 */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } IC_HS_SCL_LCNT_b;
  };

  union {
    __IM uint32_t IC_INTR_STAT; /*!< (@ 0x0000002C) I2C Interrupt Status Register                              */

    struct {
      __IM uint32_t R_RX_UNDER : 1;         /*!< (@ 0x00000000) Set if the processor attempts to read the receive
                                                                    buffer when it is empty by reading from
                                                                    the IC_DATA_CMD register                                   */
      __IM uint32_t R_RX_OVER : 1;          /*!< (@ 0x00000001) Set if the receive buffer is completely filled
                                                                    to IC_RX_BUFFER_DEPTH and an additional
                                                                    byte is received from an external I2C device               */
      __IM uint32_t R_RX_FULL : 1;          /*!< (@ 0x00000002) Set when the receive buffer reaches or goes above
                                                                    the RX_TL threshold in the IC_RX_TL register.              */
      __IM uint32_t R_TX_OVER : 1;          /*!< (@ 0x00000003) Set during transmit if the transmit buffer is
                                                                    filled to IC_TX_BUFFER_DEPTH and the processor
                                                                    attempts to issue another I2C command by
                                                                    writing to the IC_DATA_CMD register.                       */
      __IM uint32_t R_TX_EMPTY : 1;         /*!< (@ 0x00000004) This bit is set to 1 when the transmit buffer
                                                                    is at or below the threshold value set
                                                                    in the IC_TX_TL register.                                  */
      __IM uint32_t R_RD_REQ : 1;           /*!< (@ 0x00000005) This bit is set to 1 when DW_apb_i2c is acting
                                                                    as a slave and another I2C master is attempting
                                                                    to read data from DW_apb_i2c.                              */
      __IM uint32_t R_TX_ABRT : 1;          /*!< (@ 0x00000006) This bit indicates if DW_apb_i2c, as an I2C transmitter,
                                                                    is unable to complete the intended actions
                                                                    on the contents of the transmit FIFO                       */
      __IM uint32_t R_RX_DONE : 1;          /*!< (@ 0x00000007) When the DW_apb_i2c is acting as a slave-transmitter,
                                                                    this bit is set to 1 if the master does
                                                                    not acknowledge a transmitted byte                         */
      __IM uint32_t R_ACTIVITY : 1;         /*!< (@ 0x00000008) This bit captures DW_apb_i2c activity and stays
                                                                    set until it is cleared                                    */
      __IM uint32_t R_STOP_DET : 1;         /*!< (@ 0x00000009) Indicates whether a STOP condition has occurred
                                                                    on the I2C interface regardless of whether
                                                                    DW_apb_i2c is operating in slave or master
                                                                    mode.                                                      */
      __IM uint32_t R_START_DET : 1;        /*!< (@ 0x0000000A) Indicates whether a START or RESTART condition
                                                                    has occurred on the I2C interface regardless
                                                                    of whether DW_apb_i2c is operating in slave
                                                                    or master mode.                                            */
      __IM uint32_t R_GEN_CALL : 1;         /*!< (@ 0x0000000B) Set only when a General Call address is received
                                                                    and it is acknowledged                                     */
      __IM uint32_t R_RESTART_DET : 1;      /*!< (@ 0x0000000C) Indicates whether a RESTART condition has occurred
                                                                    on the I2C interface when DW_apb_i2c is
                                                                    operating in slave mode and the slave is
                                                                    the addressed slave                                        */
      __IM uint32_t R_MST_ON_HOLD : 1;      /*!< (@ 0x0000000D) Indicates whether a master is holding the bus
                                                                    and the Tx FIFO is empty.                                  */
      __IM uint32_t M_SCL_STUCK_AT_LOW : 1; /*!< (@ 0x0000000E) Indicates whether the SCL Line is stuck at low
                                                                    for the IC_SCL_STUCK_LOW_TIMOUT number
                                                                    of ic_clk periods                                          */
      __IM uint32_t RESERVED1 : 17; /*!< (@ 0x0000000F) reserved1                                                  */
    } IC_INTR_STAT_b;
  };

  union {
    __IOM uint32_t IC_INTR_MASK; /*!< (@ 0x00000030) I2C Interrupt Mask Register                                */

    struct {
      __IOM uint32_t M_RX_UNDER : 1;        /*!< (@ 0x00000000) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_RX_OVER : 1;         /*!< (@ 0x00000001) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_RX_FULL : 1;         /*!< (@ 0x00000002) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_TX_OVER : 1;         /*!< (@ 0x00000003) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register                          */
      __IOM uint32_t M_TX_EMPTY : 1;        /*!< (@ 0x00000004) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_RD_REQ : 1;          /*!< (@ 0x00000005) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_TX_ABRT : 1;         /*!< (@ 0x00000006) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_RX_DONE : 1;         /*!< (@ 0x00000007) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_ACTIVITY : 1;        /*!< (@ 0x00000008) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_STOP_DET : 1;        /*!< (@ 0x00000009) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_START_DET : 1;       /*!< (@ 0x0000000A) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_GEN_CALL : 1;        /*!< (@ 0x0000000B) This bit mask their corresponding interrupt status
                                                                    bits in the IC_INTR_STAT register.                         */
      __IOM uint32_t M_RESTART_DET : 1;     /*!< (@ 0x0000000C) Indicates whether a RESTART condition has occurred
                                                                    on the I2C interface when DW_apb_i2c is
                                                                    operating in slave mode and the slave is
                                                                    the addressed slave                                        */
      __IOM uint32_t M_MST_ON_HOLD : 1;     /*!< (@ 0x0000000D) Indicates whether a master is holding the bus
                                                                    and the Tx FIFO is empty.                                  */
      __IM uint32_t M_SCL_STUCK_AT_LOW : 1; /*!< (@ 0x0000000E) Indicates whether the SCL Line is stuck at low
                                                                    for the IC_SCL_STUCK_LOW_TIMOUT number
                                                                    of ic_clk periods                                          */
      __IM uint32_t RESERVED1 : 17; /*!< (@ 0x0000000F) reserved1                                                  */
    } IC_INTR_MASK_b;
  };

  union {
    __IM uint32_t IC_RAW_INTR_STAT; /*!< (@ 0x00000034) I2C Raw Interrupt Status Register                          */

    struct {
      __IM uint32_t RX_UNDER : 1;         /*!< (@ 0x00000000) Set if the processor attempts to read the receive
                                                                    buffer when it is empty by reading from
                                                                    the IC_DATA_CMD register                                   */
      __IM uint32_t RX_OVER : 1;          /*!< (@ 0x00000001) Set if the receive buffer is completely filled
                                                                    to IC_RX_BUFFER_DEPTH and an additional
                                                                    byte is received from an external I2C device               */
      __IM uint32_t RX_FULL : 1;          /*!< (@ 0x00000002) Set when the receive buffer reaches or goes above
                                                                    the RX_TL threshold in the IC_RX_TL register.              */
      __IM uint32_t TX_OVER : 1;          /*!< (@ 0x00000003) Set during transmit if the transmit buffer is
                                                                    filled to IC_TX_BUFFER_DEPTH and the processor
                                                                    attempts to issue another I2C command by
                                                                    writing to the IC_DATA_CMD register.                       */
      __IM uint32_t TX_EMPTY : 1;         /*!< (@ 0x00000004) This bit is set to 1 when the transmit buffer
                                                                    is at or below the threshold value set
                                                                    in the IC_TX_TL register.                                  */
      __IM uint32_t RD_REQ : 1;           /*!< (@ 0x00000005) This bit is set to 1 when DW_apb_i2c is acting
                                                                    as a slave and another I2C master is attempting
                                                                    to read data from DW_apb_i2c.                              */
      __IM uint32_t TX_ABRT : 1;          /*!< (@ 0x00000006) This bit indicates if DW_apb_i2c, as an I2C transmitter,
                                                                    is unable to complete the intended actions
                                                                    on the contents of the transmit FIFO                       */
      __IM uint32_t RX_DONE : 1;          /*!< (@ 0x00000007) When the DW_apb_i2c is acting as a slave-transmitter,
                                                                    this bit is set to 1 if the master does
                                                                    not acknowledge a transmitted byte                         */
      __IM uint32_t ACTIVITY : 1;         /*!< (@ 0x00000008) This bit captures DW_apb_i2c activity and stays
                                                                    set until it is cleared                                    */
      __IM uint32_t STOP_DET : 1;         /*!< (@ 0x00000009) Indicates whether a STOP condition has occurred
                                                                    on the I2C interface regardless of whether
                                                                    DW_apb_i2c is operating in slave or master
                                                                    mode.                                                      */
      __IM uint32_t START_DET : 1;        /*!< (@ 0x0000000A) Indicates whether a START or RESTART condition
                                                                    has occurred on the I2C interface regardless
                                                                    of whether DW_apb_i2c is operating in slave
                                                                    or master mode.                                            */
      __IM uint32_t GEN_CALL : 1;         /*!< (@ 0x0000000B) Set only when a General Call address is received
                                                                    and it is acknowledged                                     */
      __IM uint32_t RESTART_DET : 1;      /*!< (@ 0x0000000C) Indicates whether a RESTART condition has occurred
                                                                    on the I2C interface when DW_apb_i2c is
                                                                    operating in slave mode and the slave is
                                                                    the addressed slave                                        */
      __IM uint32_t MST_ON_HOLD : 1;      /*!< (@ 0x0000000D) Indicates whether a master is holding the bus
                                                                    and the Tx FIFO is empty.                                  */
      __IM uint32_t SCL_STUCK_AT_LOW : 1; /*!< (@ 0x0000000E) Indicates whether the SCL Line is stuck at low
                                                                    for the IC_SCL_STUCK_LOW_TIMOUT number
                                                                    of ic_clk periods                                          */
      __IM uint32_t RESERVED1 : 17; /*!< (@ 0x0000000F) reserved1                                                  */
    } IC_RAW_INTR_STAT_b;
  };

  union {
    __IOM uint32_t IC_RX_TL; /*!< (@ 0x00000038) I2C Receive FIFO Threshold Register                        */

    struct {
      __IOM uint32_t RX_TL : 8;     /*!< (@ 0x00000000) Receive FIFO Threshold Level                               */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } IC_RX_TL_b;
  };

  union {
    __IOM uint32_t IC_TX_TL; /*!< (@ 0x0000003C) I2C Transmit FIFO Threshold Register                       */

    struct {
      __IOM uint32_t TX_TL : 8;     /*!< (@ 0x00000000) Transmit FIFO Threshold Level                              */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } IC_TX_TL_b;
  };

  union {
    __IM uint32_t IC_CLR_INTR; /*!< (@ 0x00000040) Clear Combined and Individual Interrupt Register           */

    struct {
      __IM uint32_t CLR_INTR : 1;   /*!< (@ 0x00000000) Read this register to clear the combined interrupt,
                                                                    all individual interrupts, and the IC_TXABRT_SOURCE
                                                                    register                                                   */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_INTR_b;
  };

  union {
    __IM uint32_t IC_CLR_RX_UNDER; /*!< (@ 0x00000044) Clear RX_UNDER Interrupt Register                          */

    struct {
      __IM uint32_t CLR_RX_UNDER : 1; /*!< (@ 0x00000000) Read this register to clear the RX_UNDER interrupt
                                                                    (bit 0) of the IC_RAW_INTR_STAT register.                  */
      __IM uint32_t RESERVED1 : 31;   /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_RX_UNDER_b;
  };

  union {
    __IM uint32_t IC_CLR_RX_OVER; /*!< (@ 0x00000048) Clear RX_OVER Interrupt Register                           */

    struct {
      __IM uint32_t CLR_RX_OVER : 1; /*!< (@ 0x00000000) Read this register to clear the RX_OVER interrupt
                                                                    (bit 1) of the IC_RAW_INTR_STAT register                   */
      __IM uint32_t RESERVED1 : 31;  /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_RX_OVER_b;
  };

  union {
    __IM uint32_t IC_CLR_TX_OVER; /*!< (@ 0x0000004C) Clear TX_OVER Interrupt Register                           */

    struct {
      __IM uint32_t CLR_TX_OVER : 1; /*!< (@ 0x00000000) Read this register to clear the TX_OVER interrupt
                                                                    (bit 3) of the IC_RAW_INTR_STAT register.                  */
      __IM uint32_t RESERVED1 : 31;  /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_TX_OVER_b;
  };

  union {
    __IM uint32_t IC_CLR_RD_REQ; /*!< (@ 0x00000050) Clear RD_REQ Interrupt Register                            */

    struct {
      __IM uint32_t CLR_RD_REQ : 1; /*!< (@ 0x00000000) Read this register to clear the RD_REQ interrupt
                                                                    (bit 5) of the IC_RAW_INTR_STAT register.                  */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_RD_REQ_b;
  };

  union {
    __IM uint32_t IC_CLR_TX_ABRT; /*!< (@ 0x00000054) Clear TX_ABRT Interrupt Register                           */

    struct {
      __IM uint32_t CLR_TX_ABRT : 1; /*!< (@ 0x00000000) Read this register to clear the TX_ABRT interrupt
                                                                    (bit 6) of the C_RAW_INTR_STAT register,
                                                                    and the IC_TX_ABRT_SOURCE register                         */
      __IM uint32_t RESERVED1 : 31;  /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_TX_ABRT_b;
  };

  union {
    __IM uint32_t IC_CLR_RX_DONE; /*!< (@ 0x00000058) Clear RX_DONE Interrupt Register                           */

    struct {
      __IM uint32_t CLR_RX_DONE : 1; /*!< (@ 0x00000000) Read this register to clear the RX_DONE interrupt
                                                                    (bit 7) of the IC_RAW_INTR_STAT register                   */
      __IM uint32_t RESERVED1 : 31;  /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_RX_DONE_b;
  };

  union {
    __IM uint32_t IC_CLR_ACTIVITY; /*!< (@ 0x0000005C) Clear ACTIVITY Interrupt Register                          */

    struct {
      __IM uint32_t CLR_ACTIVITY : 1; /*!< (@ 0x00000000) Reading this register clears the ACTIVITY interrupt
                                                                    if the I2C is not active any more                          */
      __IM uint32_t RESERVED1 : 31;   /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_ACTIVITY_b;
  };

  union {
    __IM uint32_t IC_CLR_STOP_DET; /*!< (@ 0x00000060) Clear STOP_DET Interrupt Register                          */

    struct {
      __IM uint32_t CLR_STOP_DET : 1; /*!< (@ 0x00000000) Read this register to clear the STOP_DET interrupt
                                                                    (bit 9) of the IC_RAW_INTR_STAT register.                  */
      __IM uint32_t RESERVED1 : 31;   /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_STOP_DET_b;
  };

  union {
    __IM uint32_t IC_CLR_START_DET; /*!< (@ 0x00000064) Clear START_DET Interrupt Register                         */

    struct {
      __IM uint32_t CLR_START_DET : 1; /*!< (@ 0x00000000) Read this register to clear the START_DET interrupt
                                                                    (bit 10) of the IC_RAW_INTR_STAT register                  */
      __IM uint32_t RESERVED1 : 31;    /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_START_DET_b;
  };

  union {
    __IM uint32_t IC_CLR_GEN_CALL; /*!< (@ 0x00000068) Clear GEN_CALL Interrupt Register                          */

    struct {
      __IM uint32_t CLR_GEN_CALL : 1; /*!< (@ 0x00000000) Read this register to clear the GEN_CALL interrupt
                                                                    (bit 11) of IC_RAW_INTR_STAT register                      */
      __IM uint32_t RESERVED1 : 31;   /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_GEN_CALL_b;
  };

  union {
    __IOM uint32_t IC_ENABLE; /*!< (@ 0x0000006C) Clear GEN_CALL Interrupt Register                          */

    struct {
      __IOM uint32_t EN : 1;           /*!< (@ 0x00000000) Controls whether the DW_apb_i2c is enabled                 */
      __IOM uint32_t ABORT : 1;        /*!< (@ 0x00000001) When set, the controller initiates the transfer
                                                                    abort                                                      */
      __IOM uint32_t TX_CMD_BLOCK : 1; /*!< (@ 0x00000002) none                                                       */
      __IOM uint32_t SDA_STUCK_RECOVERY_ENABLE : 1;
      __IM uint32_t RESERVED1 : 12; /*!< (@ 0x00000004) reserved1                                                  */
      __IM uint32_t RESERVED2 : 16; /*!< (@ 0x00000010) reserved2                                                  */
    } IC_ENABLE_b;
  };

  union {
    __IM uint32_t IC_STATUS; /*!< (@ 0x00000070) I2C Status Register                                        */

    struct {
      __IM uint32_t ACTIVITY : 1;     /*!< (@ 0x00000000) I2C Activity Status                                        */
      __IM uint32_t TFNF : 1;         /*!< (@ 0x00000001) Transmit FIFO Not Full                                     */
      __IM uint32_t TFE : 1;          /*!< (@ 0x00000002) Transmit FIFO Completely Empty                             */
      __IM uint32_t RFNE : 1;         /*!< (@ 0x00000003) Receive FIFO Not Empty                                     */
      __IM uint32_t RFF : 1;          /*!< (@ 0x00000004) Receive FIFO Completely Full                               */
      __IM uint32_t MST_ACTIVITY : 1; /*!< (@ 0x00000005) Master FSM Activity Status                                 */
      __IM uint32_t SLV_ACTIVITY : 1; /*!< (@ 0x00000006) Slave FSM Activity Status                                  */
      __IM uint32_t MST_HOLD_TX_FIFO_EMPTY : 1;  /*!< (@ 0x00000007) The I2C master stalls the write transfer when
                                                                    Tx FIFO is empty, and the the last byte
                                                                    does not have the Stop bit set.                            */
      __IM uint32_t MST_HOLD_RX_FIFO_FULL : 1;   /*!< (@ 0x00000008) This bit indicates the BUS Hold in Master mode
                                                                    due to Rx FIFO is Full and additional byte
                                                                    has been received.                                         */
      __IM uint32_t SLV_HOLD_TX_FIFO_EMPTY : 1;  /*!< (@ 0x00000009) This bit indicates the BUS Hold in Slave mode
                                                                    for the Read request when the Tx FIFO is
                                                                    empty.                                                     */
      __IM uint32_t SLV_HOLD_RX_FIFO_FULL : 1;   /*!< (@ 0x0000000A) This bit indicates the BUS Hold in Slave mode
                                                                    due to the Rx FIFO being Full and an additional
                                                                    byte being received.                                       */
      __IM uint32_t SDA_STUCK_NOT_RECOVERED : 1; /*!< (@ 0x0000000B) This bit indicates that an SDA stuck at low is
                                                                    not recovered after the recovery mechanism.                */
      __IM uint32_t RESERVED1 : 20; /*!< (@ 0x0000000C) reserved1                                                  */
    } IC_STATUS_b;
  };

  union {
    __IM uint32_t IC_TXFLR; /*!< (@ 0x00000074) I2C Transmit FIFO Level Register                           */

    struct {
      __IM uint32_t TXFLR : 4;      /*!< (@ 0x00000000) Contains the number of valid data entries in
                                                                    the transmit FIFO.                                         */
      __IM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) reserved1                                                  */
    } IC_TXFLR_b;
  };

  union {
    __IM uint32_t IC_RXFLR; /*!< (@ 0x00000078) I2C Receive FIFO Level Register                            */

    struct {
      __IM uint32_t RXFLR : 4;      /*!< (@ 0x00000000) Receive FIFO Level. Contains the number of valid
                                                                    data entries in the receive FIFO                           */
      __IM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) reserved1                                                  */
    } IC_RXFLR_b;
  };

  union {
    __IOM uint32_t IC_SDA_HOLD; /*!< (@ 0x0000007C) I2C SDA Hold Time Length Register                          */

    struct {
      __IOM uint32_t IC_SDA_TX_HOLD : 16; /*!< (@ 0x00000000) Sets the required SDA hold time in units of ic_clk
                                                                    period,when I2C acts as a transmitter.                     */
      __IOM uint32_t IC_SDA_RX_HOLD : 8;  /*!< (@ 0x00000010) Sets the required SDA hold time in units of ic_clk
                                                                    period,when I2C acts as a receiver.                        */
      __IM uint32_t RESERVED1 : 8; /*!< (@ 0x00000018) reserved1                                                  */
    } IC_SDA_HOLD_b;
  };

  union {
    __IM uint32_t IC_TX_ABRT_SOURCE; /*!< (@ 0x00000080) I2C Transmit Abort Source Register                         */

    struct {
      __IM uint32_t ABRT_7B_ADDR_NOACK : 1;  /*!< (@ 0x00000000) 1: Master is in 7-bit addressing mode and the
                                                                    address sent was not acknowledged by any
                                                                    slave                                                      */
      __IM uint32_t ABRT_10ADDR1_NOACK : 1;  /*!< (@ 0x00000001) 1: Master is in 10-bit address mode and the first
                                                                    10-bit address byte was not acknowledged
                                                                    by any slave                                               */
      __IM uint32_t ABRT_10ADDR2_NOACK : 1;  /*!< (@ 0x00000002) 1: Master is in 10-bit address mode and the second
                                                                    address byte of the 10-bit address was
                                                                    not acknowledged by any slave                              */
      __IM uint32_t ABRT_TXDATA_NOACK : 1;   /*!< (@ 0x00000003) 1: This is a master-mode only bit. Master has
                                                                    received an acknowledgement for the address,
                                                                    but when it sent data byte(s) following
                                                                    the address, it did not receive an acknowledge
                                                                    from the remote slave(s)                                   */
      __IM uint32_t ABRT_GCALL_NOACK : 1;    /*!< (@ 0x00000004) 1: DW_apb_i2c in master mode sent a General Call
                                                                    and no slave on the bus acknowledged the
                                                                    General Call                                               */
      __IM uint32_t ABRT_GCALL_READ : 1;     /*!< (@ 0x00000005) 1: DW_apb_i2c in master mode sent a General Call
                                                                    but the user programmed the byte following
                                                                    the General Call to be a read from the
                                                                    bus (IC_DATA_CMD[9] is set to 1)                           */
      __IM uint32_t ABRT_HS_ACKDET : 1;      /*!< (@ 0x00000006) 1: Master is in High Speed mode and the High
                                                                    Speed Master code was acknowledged                         */
      __IM uint32_t ABRT_SBYTE_ACKDET : 1;   /*!< (@ 0x00000007) 1: Master has sent a START Byte and the START
                                                                    Byte was acknowledged (wrong behavior)                     */
      __IM uint32_t ABRT_HS_NORSTRT : 1;     /*!< (@ 0x00000008) 1: The restart is disabled (IC_RESTART_EN bit
                                                                    (IC_CON[5]) = 0) and the user is trying
                                                                    to use the master to transfer data in High
                                                                    Speed mode                                                 */
      __IM uint32_t ABRT_SBYTE_NORSTRT : 1;  /*!< (@ 0x00000009) 1: The restart is disabled (IC_RESTART_EN bit
                                                                    (IC_CON[5]) = 0) and the user is trying
                                                                    to send a START Byte                                       */
      __IM uint32_t ABRT_10B_RD_NORSTRT : 1; /*!< (@ 0x0000000A) 1: The restart is disabled (IC_RESTART_EN bit
                                                                    (IC_CON[5]) = 0) and the master sends a
                                                                    read command in 10-bit addressing mode                     */
      __IM uint32_t ABRT_MASTER_DIS : 1;     /*!< (@ 0x0000000B) 1: User tries to initiate a Master operation
                                                                    with the Master mode disabled                              */
      __IM uint32_t ARB_LOST : 1; /*!< (@ 0x0000000C) 1: Master has lost arbitration, or if IC_TX_ABRT_SOURCE[14]
                                                                    is also set, then the slave transmitter
                                                                    has lost arbitration                                       */
      __IM uint32_t ABRT_SLVFLUSH_TXFIFO : 1;  /*!< (@ 0x0000000D) 1: Slave has received a read command and some
                                                                    data exists in the TX FIFO so the slave
                                                                    issues a TX_ABRT interrupt to flush old
                                                                    data in TX FIFO                                            */
      __IM uint32_t ABRT_SLV_ARBLOST : 1;      /*!< (@ 0x0000000E) 1: Slave lost the bus while transmitting data
                                                                    to a remote master. IC_TX_ABRT_SOURCE[12]
                                                                    is set at the same time                                    */
      __IM uint32_t ABRT_SLVRD_INTX : 1;       /*!< (@ 0x0000000F) 1: When the processor side responds to a slave
                                                                    mode request for data to be transmitted
                                                                    to a remote master and user writes a 1
                                                                    in CMD (bit 8) of IC_DATA_CMD register                     */
      __IM uint32_t ABRT_USER_ABRT : 1;        /*!< (@ 0x00000010) This is a master-mode-only bit. Master has detected
                                                                    the transfer abort (IC_ENABLE[1]).                         */
      __IM uint32_t ABRT_SDA_STUCK_AT_LOW : 1; /*!< (@ 0x00000011) Master detects the SDA is Stuck at low for the
                                                                    IC_SDA_STUCK_AT_LOW_TIMEOUT value of ic_clks               */
      __IM uint32_t ABRT_DEVICE_NOACK : 1;     /*!< (@ 0x00000012) Master initiates the DEVICE_ID transfer and the
                                                                    device ID sent is not acknowledged by any
                                                                    slave                                                      */
      __IM uint32_t ABRT_DEVICE_SLVADDR_NOACK : 1; /*!< (@ 0x00000013) Master is initiating the DEVICE_ID transfer and
                                                                    the slave address sent was not acknowledged
                                                                    by any slave                                               */
      __IM uint32_t ABRT_DEVICE_WRITE : 1;         /*!< (@ 0x00000014) Master is initiating the DEVICE_ID transfer and
                                                                    the Tx- FIFO consists of write commands.                   */
      __IM uint32_t RESERVED1 : 2;    /*!< (@ 0x00000015) reserved1                                                  */
      __IM uint32_t TX_FLUSH_CNT : 9; /*!< (@ 0x00000017) This field indicates the number of Tx FIFO data
                                                                    commands that are flushed due to TX_ABRT
                                                                    interrupt                                                  */
    } IC_TX_ABRT_SOURCE_b;
  };

  union {
    __IOM uint32_t
      IC_SLV_DATA_NACK_ONLY; /*!< (@ 0x00000084) Generate Slave Data NACK Register                          */

    struct {
      __IOM uint32_t NACK : 1;      /*!< (@ 0x00000000) Generate NACK. This NACK generation only occurs
                                                                    when DW_apb_i2c is a slave receiver.                       */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) reserved1                                                  */
    } IC_SLV_DATA_NACK_ONLY_b;
  };

  union {
    __IOM uint32_t IC_DMA_CR; /*!< (@ 0x00000088) DMA Control Register                                       */

    struct {
      __IOM uint32_t RDMAE : 1;     /*!< (@ 0x00000000) Receive DMA Enable                                         */
      __IOM uint32_t TDMAE : 1;     /*!< (@ 0x00000001) Transmit DMA Enable.This bit enables/disables
                                                                    the transmit FIFO DMA channel                              */
      __IM uint32_t RESERVED1 : 30; /*!< (@ 0x00000002) reserved1                                                  */
    } IC_DMA_CR_b;
  };

  union {
    __IOM uint32_t IC_DMA_TDLR; /*!< (@ 0x0000008C) DMA Transmit Data Level Register                           */

    struct {
      __IOM uint32_t DMATDL : 4;    /*!< (@ 0x00000000) This bit field controls the level at which a
                                                                    DMA request is made by the transmit logic                  */
      __IM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) reserved1                                                  */
    } IC_DMA_TDLR_b;
  };

  union {
    __IOM uint32_t IC_DMA_RDLR; /*!< (@ 0x00000090) I2C Receive Data Level Register                            */

    struct {
      __IOM uint32_t DMARDL : 4;    /*!< (@ 0x00000000) This bit field controls the level at which a
                                                                    DMA request is made by the receive logic                   */
      __IM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) reserved1                                                  */
    } IC_DMA_RDLR_b;
  };

  union {
    __IOM uint32_t IC_SDA_SETUP; /*!< (@ 0x00000094) I2C SDA Setup Register                                     */

    struct {
      __IOM uint32_t SDA_SETUP : 8; /*!< (@ 0x00000000) This register controls the amount of time delay
                                                                    (in terms of number of ic_clk clock periods)               */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } IC_SDA_SETUP_b;
  };

  union {
    __IOM uint32_t
      IC_ACK_GENERAL_CALL; /*!< (@ 0x00000098) I2C ACK General Call Register                              */

    struct {
      __IOM uint32_t ACK_GEN_CALL : 1; /*!< (@ 0x00000000) ACK General Call                                           */
      __IM uint32_t RESERVED1 : 31;    /*!< (@ 0x00000001) reserved1                                                  */
    } IC_ACK_GENERAL_CALL_b;
  };

  union {
    __IOM uint32_t IC_ENABLE_STATUS; /*!< (@ 0x0000009C) I2C Enable Status Register                                 */

    struct {
      __IOM uint32_t ACK_GEN_CALL : 1; /*!< (@ 0x00000000) ACK General Call                                           */
      __IM uint32_t SLV_DISABLED_WHILE_BUSY : 1; /*!< (@ 0x00000001) This bit indicates if a potential or active Slave
                                                                    operation has been aborted due to the setting
                                                                    of the IC_ENABLE register from 1 to 0                      */
      __IM uint32_t
        SLV_RX_DATA_LOST : 1;        /*!< (@ 0x00000002) Slave Received Data Lost                                   */
      __IOM uint32_t RESERVED1 : 29; /*!< (@ 0x00000003) reserved1                                                  */
    } IC_ENABLE_STATUS_b;
  };

  union {
    __IOM uint32_t IC_FS_SPKLEN; /*!< (@ 0x000000A0) I2C SS and FS Spike Suppression Limit Register             */

    struct {
      __IOM uint32_t IC_FS_SPKLEN : 8; /*!< (@ 0x00000000) This register sets the duration, measured in
                                                                    ic_clk cycles, of the longest spike in
                                                                    the SCL or SDA lines that are filtered
                                                                    out by the spike suppression logic                         */
      __IM uint32_t RESERVED1 : 24;    /*!< (@ 0x00000008) reserved1                                                  */
    } IC_FS_SPKLEN_b;
  };

  union {
    __IOM uint32_t IC_HS_SPKLEN; /*!< (@ 0x000000A4) I2C HS Spike Suppression Limit Register                    */

    struct {
      __IOM uint32_t IC_HS_SPKLEN : 8; /*!< (@ 0x00000000) This register sets the duration, measured in
                                                                    ic_clk cycles, of the longest spike in
                                                                    the SCL or SDA lines that are filtered
                                                                    out by the spike suppression logic                         */
      __IM uint32_t RESERVED1 : 24;    /*!< (@ 0x00000008) reserved1                                                  */
    } IC_HS_SPKLEN_b;
  };

  union {
    __IM uint32_t IC_CLR_RESTART_DET; /*!< (@ 0x000000A8) Clear RESTART_DET Interrupt Register                       */

    struct {
      __IM uint32_t CLR_RESTART_DET : 1; /*!< (@ 0x00000000) Read this register to clear the RESTART_DET interrupt
                                                                    (bit 12) of the IC_RAW_INTR_STAT registe                   */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_RESTART_DET_b;
  };

  union {
    __IOM uint32_t
      IC_SCL_STUCK_AT_LOW_TIMEOUT; /*!< (@ 0x000000AC) I2C SCL Stuck at Low Timeout                               */

    struct {
      __IOM uint32_t IC_SCL_STUCK_LOW_TIMEOUT : 32; /*!< (@ 0x00000000) Generates the interrupt to indicate SCL stuck
                                                                    at low if it detects the SCL stuck at low
                                                                    for the IC_SCL_STUCK_LOW_TIMEOUT in units
                                                                    of ic_clk period                                           */
    } IC_SCL_STUCK_AT_LOW_TIMEOUT_b;
  };

  union {
    __IOM uint32_t
      IC_SDA_STUCK_AT_LOW_TIMEOUT; /*!< (@ 0x000000B0) I2C SDA Stuck at Low Timeout                               */

    struct {
      __IOM uint32_t
        IC_SDA_STUCK_LOW_TIMEOUT : 32; /*!< (@ 0x00000000) Initiates the recovery of SDA line , if it detects
                                                                    the SDA stuck at low for the IC_SDA_STUCK_LOW_TIMEOUT
                                                                    in units of ic_clk period.                                 */
    } IC_SDA_STUCK_AT_LOW_TIMEOUT_b;
  };

  union {
    __IM uint32_t
      IC_CLR_SCL_STUCK_DET; /*!< (@ 0x000000B4) Clear SCL Stuck at Low Detect Interrupt Register           */

    struct {
      __IM uint32_t CLR_SCL_STUCK : 1; /*!< (@ 0x00000000) Read this register to clear the SCL_STUCK_DET
                                                                    interrupt                                                  */
      __IM uint32_t RESERVED1 : 31;    /*!< (@ 0x00000001) reserved1                                                  */
    } IC_CLR_SCL_STUCK_DET_b;
  };

  union {
    __IM uint32_t IC_DEVICE_ID; /*!< (@ 0x000000B8) I2C Device ID                                              */

    struct {
      __IM uint32_t DEVICE_ID : 24; /*!< (@ 0x00000000) Contains the Device-ID of the component assigned
                                                                    through the configuration parameter                        */
      __IM uint32_t RESERVED1 : 8;  /*!< (@ 0x00000018) reserved1                                                  */
    } IC_DEVICE_ID_b;
  };

  union {
    __IOM uint32_t
      IC_SMBUS_CLOCK_LOW_SEXT; /*!< (@ 0x000000BC) SMBUS Slave Clock Extend Timeout Register                  */

    struct {
      __IOM uint32_t
        SMBUS_CLK_LOW_SEXT_TIMEOUT : 32; /*!< (@ 0x00000000) The values in this register are in units of ic_clk
                                                                    period.                                                    */
    } IC_SMBUS_CLOCK_LOW_SEXT_b;
  };

  union {
    __IOM uint32_t
      IC_SMBUS_CLOCK_LOW_MEXT; /*!< (@ 0x000000C0) SMBUS Master extend clock Timeout Register                 */

    struct {
      __IOM uint32_t
        SMBUS_CLK_LOW_MEXT_TIMEOUT : 32; /*!< (@ 0x00000000) The values in this register are in units of ic_clk
                                                                    period..                                                   */
    } IC_SMBUS_CLOCK_LOW_MEXT_b;
  };

  union {
    __IOM uint32_t
      IC_SMBUS_THIGH_MAX_IDLE_COUNT; /*!< (@ 0x000000C4) SMBus Thigh MAX Bus-Idle count Register                   */

    struct {
      __IOM uint32_t
        SMBUS_THIGH_MAX_BUS_IDLE_CNT : 16; /*!< (@ 0x00000000) The values in this register are in units of ic_clk
                                                                    period.                                                    */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) Reserved1                                                  */
    } IC_SMBUS_THIGH_MAX_IDLE_COUNT_b;
  };

  union {
    __IOM uint32_t IC_SMBUS_INTR_STAT; /*!< (@ 0x000000C8) SMBUS Interrupt Status Register                            */

    struct {
      __IOM uint32_t RESERVED1 : 32; /*!< (@ 0x00000000) Reserved1                                                  */
    } IC_SMBUS_INTR_STAT_b;
  };

  union {
    __IOM uint32_t IC_SMBUS_INTR_MASK; /*!< (@ 0x000000CC) Interrupt Mask Register                                    */

    struct {
      __IOM uint32_t RESERVED1 : 32; /*!< (@ 0x00000000) Reserved1                                                  */
    } IC_SMBUS_INTR_MASK_b;
  };

  union {
    __IOM uint32_t
      IC_SMBUS_INTR_RAW_STATUS; /*!< (@ 0x000000D0) SMBUS Raw Interrupt Status Register                        */

    struct {
      __IOM uint32_t RESERVED1 : 32; /*!< (@ 0x00000000) Reserved1.                                                 */
    } IC_SMBUS_INTR_RAW_STATUS_b;
  };

  union {
    __IOM uint32_t IC_CLR_SMBUS_INTR; /*!< (@ 0x000000D4) Clear SMBUS Interrupt Register                             */

    struct {
      __IOM uint32_t RESERVED1 : 32; /*!< (@ 0x00000000) RESERVED1                                                  */
    } IC_CLR_SMBUS_INTR_b;
  };

  union {
    __IOM uint32_t IC_OPTIONAL_SAR; /*!< (@ 0x000000D8) Optional Slave Address Register                            */

    struct {
      __IOM uint32_t RESERVED1 : 32; /*!< (@ 0x00000000) Reserved1.                                                 */
    } IC_OPTIONAL_SAR_b;
  };

  union {
    __IOM uint32_t IC_SMBUS_UDID_LSB; /*!< (@ 0x000000DC) SMBUS ARP UDID LSB Register                                */

    struct {
      __IOM uint32_t IC_SMBUS_ARP_UDID_LSB : 32; /*!< (@ 0x00000000) This field is used to store the LSB 32 bit value
                                                                    of slave unique device identifier used
                                                                    in Address Resolution Protocol.                            */
    } IC_SMBUS_UDID_LSB_b;
  };
  __IM uint32_t RESERVED[5];

  union {
    __IM uint32_t IC_COMP_PARAM_1; /*!< (@ 0x000000F4) I2C HS Spike Suppression Limit Register                    */

    struct {
      __IM uint32_t CLR_RESTART_DET : 1; /*!< (@ 0x00000000) Read this register to clear the RESTART_DET interrupt
                                                                    (bit 12) of the IC_RAW_INTR_STAT registe                   */
      __IM uint32_t : 1;
      __IM uint32_t
        MAX_SPEED_MODE : 2; /*!< (@ 0x00000002) Maximum Speed Mode                                         */
      __IM uint32_t
        HC_COUNT_VALUES : 1;     /*!< (@ 0x00000004) Hard Code the count values                                 */
      __IM uint32_t INTR_IO : 1; /*!< (@ 0x00000005) Single Interrupt Output port                               */
      __IM uint32_t HAS_DMA : 1; /*!< (@ 0x00000006) DMA Handshake Interface signal                             */
      __IM uint32_t
        ADD_ENCODED_PARAMS : 1; /*!< (@ 0x00000007) Add Encoded Parameters                                     */
      __IM uint32_t RX_BUFFER_DEPTH : 8; /*!< (@ 0x00000008) Depth of receive buffer;the buffer is 8 bits
                                                                    wide;2 to 256                                              */
      __IM uint32_t TX_BUFFER_DEPTH : 8; /*!< (@ 0x00000010) Depth of Transmit buffer;the buffer is 8 bits
                                                                    wide;2 to 256                                              */
      __IM uint32_t RESERVED1 : 8; /*!< (@ 0x00000018) reserved1                                                  */
    } IC_COMP_PARAM_1_b;
  };

  union {
    __IM uint32_t IC_COMP_VERSION; /*!< (@ 0x000000F8) I2C Component Version Register                             */

    struct {
      __IM uint32_t
        IC_COMP_VERSION : 32; /*!< (@ 0x00000000) Signifies the component version                            */
    } IC_COMP_VERSION_b;
  };

  union {
    __IM uint32_t IC_COMP_TYPE; /*!< (@ 0x000000FC) I2C Component Type Register                                */

    struct {
      __IM uint32_t IC_COMP_TYPE : 32; /*!< (@ 0x00000000) Design ware Component Type number = 0x44_57_01_40          */
    } IC_COMP_TYPE_b;
  };
} I2C0_Type;

typedef struct {
  union {
    __IOM uint32_t PWM_DEADTIME_A; /*!< NONE                                                                  */

    struct {
      __IOM uint32_t DEADTIME_A_CH : 6; /*!< Dead time A value to load into dead time counter A of channel0        */
      uint32_t : 26;
    } PWM_DEADTIME_A_b; /*!< BitSize                                                               */
  };

  union {
    __IOM uint32_t PWM_DEADTIME_B; /*!< NONE                                                                  */

    struct {
      __IOM uint32_t DEADTIME_B_CH : 6; /*!< Dead time B value to load into deadtime counter B of channel0         */
      uint32_t : 26;
    } PWM_DEADTIME_B_b; /*!< BitSize                                                               */
  };
} MCPWM_PWM_DEADTIME_Type;

/* ================================================================================ */
/* ================                      MCPWM                     ================ */
/* ================================================================================ */

/**
 * @brief The Motor Control PWM (MCPWM) controller is used to generate a periodic pulse waveform, which is useful in motor control and power control applications (MCPWM)
 */

typedef struct { /*!< (@ 0x47070000) MCPWM Structure                                            */

  union {
    __IM uint32_t PWM_INTR_STS; /*!< (@ 0x00000000) PWM Interrupt Status Register                              */

    struct {
      __IM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_INTR_CH0 : 1; /*!< (@ 0x00000000) This time base interrupt for 0th channel without
                                                                    considering postscaler                                     */
      __IM uint32_t PWM_TIME_PRD_MATCH_INTR_CH0 : 1; /*!< (@ 0x00000001) This time base interrupt for 0th channel, which
                                                                    considers postscaler value                                 */
      __IM uint32_t FLT_A_INTR : 1; /*!< (@ 0x00000002) When the fault A pin is driven low, this interrupt
                                                                    is raised.                                                 */
      __IM uint32_t FLT_B_INTR : 1; /*!< (@ 0x00000003) When the fault B pin is driven low, this interrupt
                                                                    is raised.                                                 */
      __IM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_INTR_CH1 : 1; /*!< (@ 0x00000004) This time base interrupt for 1st channel without
                                                                    considering postscaler value                               */
      __IM uint32_t PWM_TIME_PRD_MATCH_INTR_CH1 : 1; /*!< (@ 0x00000005) This time base interrupt for 1st channel, which
                                                                    considers postscaler value.                                */
      __IM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_INTR_CH2 : 1; /*!< (@ 0x00000006) This time base interrupt for 2nd channel without
                                                                    considering postscaler value.                              */
      __IM uint32_t PWM_TIME_PRD_MATCH_INTR_CH2 : 1; /*!< (@ 0x00000007) This time base interrupt for 2nd channel, which
                                                                    considers postscaler value                                 */
      __IM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_INTR_CH3 : 1; /*!< (@ 0x00000008) This time base interrupt for 3rd channel without
                                                                    considering postscaler value.                              */
      __IM uint32_t PWM_TIME_PRD_MATCH_INTR_CH3 : 1; /*!< (@ 0x00000009) This time base interrupt for 3rd channel, which
                                                                    considers postscaler value.                                */
      __IM uint32_t RESERVED1 : 22; /*!< (@ 0x0000000A) reserved1                                                  */
    } PWM_INTR_STS_b;
  };

  union {
    __IOM uint32_t PWM_INTR_UNMASK; /*!< (@ 0x00000004) PWM Interrupt Unmask Register                              */

    struct {
      __IOM uint32_t
        PWM_INTR_UNMASK : 16;        /*!< (@ 0x00000000) Interrupt Unmask                                           */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_INTR_UNMASK_b;
  };

  union {
    __IOM uint32_t PWM_INTR_MASK; /*!< (@ 0x00000008) PWM Interrupt mask Register                                */

    struct {
      __IOM uint32_t
        PWM_INTR_UNMASK : 16;        /*!< (@ 0x00000000) Interrupt Mask                                             */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_INTR_MASK_b;
  };

  union {
    __IOM uint32_t PWM_INTR_ACK; /*!< (@ 0x0000000C) PWM Interrupt Acknowledgement Register                     */

    struct {
      __OM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_CH0_ACK : 1; /*!< (@ 0x00000000) pwm time period match interrupt for 0th channel
                                                                    will be cleared.                                           */
      __OM uint32_t
        PWM_TIME_PRD_MATCH_INTR_CH0_ACK : 1; /*!< (@ 0x00000001) pwm time period match interrupt for 0th channel
                                                                    will be cleared                                            */
      __OM uint32_t
        FLT_A_INTR_ACK : 1; /*!< (@ 0x00000002) pwm fault A interrupt will be cleared.                     */
      __OM uint32_t
        FLT_B_INTR_ACK : 1; /*!< (@ 0x00000003) pwm fault B interrupt will be cleared.                     */
      __OM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_CH1_ACK : 1; /*!< (@ 0x00000004) pwm time period match interrupt for 1st channel
                                                                    will be cleared                                            */
      __OM uint32_t
        PWM_TIME_PRD_MATCH_INTR_CH1_ACK : 1; /*!< (@ 0x00000005) pwm time period match interrupt for 1st channel
                                                                    will be cleared.                                           */
      __OM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_CH2_ACK : 1; /*!< (@ 0x00000006) pwm time period match interrupt for 2nd channel
                                                                    will be cleared.                                           */
      __OM uint32_t
        PWM_TIME_PRD_MATCH_INTR_CH2_ACK : 1; /*!< (@ 0x00000007) pwm time period match interrupt for 2nd channel
                                                                    will be cleared.                                           */
      __OM uint32_t
        RISE_PWM_TIME_PERIOD_MATCH_CH3_ACK : 1; /*!< (@ 0x00000008) pwm time period match interrupt for 3rd channel
                                                                    will be cleared.                                           */
      __OM uint32_t
        PWM_TIME_PRD_MATCH_INTR_CH3_ACK : 1; /*!< (@ 0x00000009) pwm time period match interrupt for 3rd channel
                                                                    will be cleared.                                           */
      __IOM uint32_t RESERVED1 : 22; /*!< (@ 0x0000000A) reserved1                                                  */
    } PWM_INTR_ACK_b;
  };
  __IM uint32_t RESERVED[6];

  union {
    __IOM uint32_t
      PWM_TIME_PRD_WR_REG_CH0; /*!< (@ 0x00000028) Base timer period register of channel 0                    */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_REG_WR_VALUE_CH0 : 16; /*!< (@ 0x00000000) Value to update the base timer period register
                                                                    of channel 0                                               */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_WR_REG_CH0_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_CNTR_WR_REG_CH0; /*!< (@ 0x0000002C) Base time counter initial value register for
                                                                    channel 0                                                  */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_WR_REG_CH0 : 16; /*!< (@ 0x00000000) To update the base time counter initial value
                                                                    for channel 0                                              */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_CNTR_WR_REG_CH0_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_PARAM_REG_CH0; /*!< (@ 0x00000030) Base time period config parameter's register
                                                                    for channel0                                               */

    struct {
      __IOM uint32_t
        TMR_OPEARATING_MODE_CH0 : 3; /*!< (@ 0x00000000) Base timer operating mode for channel0                    */
      __IOM uint32_t RESERVED1 : 1;  /*!< (@ 0x00000003) reserved1                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_PRE_SCALAR_VALUE_CH0 : 3; /*!< (@ 0x00000004) Base timer input clock pre scale select value
                                                                    for channel0.                                              */
      __IOM uint32_t RESERVED2 : 1; /*!< (@ 0x00000007) reserved2                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_POST_SCALAR_VALUE_CH0 : 4; /*!< (@ 0x00000008) Time base output post scale bits for channel0  */
      __IOM uint32_t RESERVED3 : 20; /*!< (@ 0x0000000C) reserved3                                                  */
    } PWM_TIME_PRD_PARAM_REG_CH0_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_CTRL_REG_CH0; /*!< (@ 0x00000034) Base time counter initial value register for
                                                                    channel 0                                                  */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< (@ 0x00000000) Time period counter soft reset                      */
      __IOM uint32_t
        PWM_TIME_BASE_EN_FRM_REG_CH0 : 1; /*!< (@ 0x00000001) Base timer enable for channnel0                      */
      __IOM uint32_t PWM_SFT_RST : 1; /*!< (@ 0x00000002) MC PWM soft reset                                          */
      __IM uint32_t RESERVED1 : 29;   /*!< (@ 0x00000003) reserved1                                                  */
    } PWM_TIME_PRD_CTRL_REG_CH0_b;
  };

  union {
    __IM uint32_t
      PWM_TIME_PRD_STS_REG_CH0; /*!< (@ 0x00000038) Base time period status register for channel0              */

    struct {
      __IM uint32_t
        PWM_TIME_PRD_DIR_STS_CH0 : 1; /*!< (@ 0x00000000) Time period counter direction status for channel0        */
      __IM uint32_t RESERVED1 : 31;   /*!< (@ 0x00000001) reserved1                                                  */
    } PWM_TIME_PRD_STS_REG_CH0_b;
  };

  union {
    __IM uint32_t PWM_TIME_PRD_CNTR_VALUE_CH0; /*!< (@ 0x0000003C) Base Time period counter current value register
                                                                    for channel0                                               */

    struct {
      __IM uint32_t
        PWM_TIME_PRD_CNTR_VALUE_CH0 : 16; /*!< (@ 0x00000000) Time period counter current value for channel0       */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_CNTR_VALUE_CH0_b;
  };
  __IM uint32_t RESERVED1[4];

  union {
    __IOM uint32_t
      PWM_DUTYCYCLE_CTRL_SET_REG; /*!< (@ 0x00000050) Duty cycle Control Set Register                            */

    struct {
      __IOM uint32_t
        IMDT_DUTYCYCLE_UPDATE_EN : 4; /*!< (@ 0x00000000) Enable to update the duty cycle immediately              */
      __IOM uint32_t DUTYCYCLE_UPDATE_DISABLE : 4; /*!< (@ 0x00000004) Duty cycle register updation disable. There is
                                                                    a separate bit for each channel                            */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_DUTYCYCLE_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_DUTYCYCLE_CTRL_RESET_REG; /*!< (@ 0x00000054) Duty cycle Control Reset Register                          */

    struct {
      __IOM uint32_t
        IMDT_DUTYCYCLE_UPDATE_EN : 4; /*!< (@ 0x00000000) Enable to update the duty cycle immediately              */
      __IOM uint32_t DUTYCYCLE_UPDATE_DISABLE : 4; /*!< (@ 0x00000004) Duty cycle register updation disable. There is
                                                                    a separate bit for each channel.                           */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_DUTYCYCLE_CTRL_RESET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_DUTYCYCLE_REG_WR_VALUE[4]; /*!< (@ 0x00000058) Duty cycle Value Register for Channel0 to channel3        */

    struct {
      __IOM uint32_t
        PWM_DUTYCYCLE_REG_WR_VALUE_CH : 16; /*!< (@ 0x00000000) Duty cycle value for channel0 to channel3          */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_DUTYCYCLE_REG_WR_VALUE_b[4];
  };
  __IM uint32_t RESERVED2[4];

  union {
    __IOM uint32_t
      PWM_DEADTIME_CTRL_SET_REG; /*!< (@ 0x00000078) Dead time Control Set Register                             */

    struct {
      __IOM uint32_t
        DEADTIME_SELECT_ACTIVE : 4; /*!< (@ 0x00000000) Dead time select bits for PWM going active                 */
      __IOM uint32_t
        DEADTIME_SELECT_INACTIVE : 4; /*!< (@ 0x00000004) Dead time select bits for PWM going inactive             */
      __IOM uint32_t
        DEADTIME_DISABLE_FRM_REG : 4; /*!< (@ 0x00000008) Dead time counter soft reset for each channel.           */
      __IM uint32_t RESERVED1 : 20;   /*!< (@ 0x0000000C) reserved1                                                  */
    } PWM_DEADTIME_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_DEADTIME_CTRL_RESET_REG; /*!< (@ 0x0000007C) Dead time Control Reset Register                           */

    struct {
      __IOM uint32_t
        DEADTIME_SELECT_ACTIVE : 4; /*!< (@ 0x00000000) Dead time select bits for PWM going active                 */
      __IOM uint32_t
        DEADTIME_SELECT_INACTIVE : 4; /*!< (@ 0x00000004) Dead time select bits for PWM going inactive             */
      __IOM uint32_t
        DEADTIME_DISABLE_FRM_REG : 4; /*!< (@ 0x00000008) Dead time counter soft reset for each channel.           */
      __IM uint32_t RESERVED1 : 20;   /*!< (@ 0x0000000C) reserved1                                                  */
    } PWM_DEADTIME_CTRL_RESET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_DEADTIME_PRESCALE_SELECT_A; /*!< (@ 0x00000080) Dead time Prescale Select Register for A                 */

    struct {
      __IOM uint32_t
        DEADTIME_PRESCALE_SELECT_A : 8; /*!< (@ 0x00000000) Dead time prescale selection bits for unit A.          */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_DEADTIME_PRESCALE_SELECT_A_b;
  };

  union {
    __IOM uint32_t
      PWM_DEADTIME_PRESCALE_SELECT_B; /*!< (@ 0x00000084) Dead time Prescale Select Register for B                 */

    struct {
      __IOM uint32_t
        DEADTIME_PRESCALE_SELECT_B : 8; /*!< (@ 0x00000000) Dead time prescale selection bits for unit B           */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_DEADTIME_PRESCALE_SELECT_B_b;
  };
  __IOM MCPWM_PWM_DEADTIME_Type
    PWM_DEADTIME[4]; /*!< (@ 0x00000088) [0..3]                                                     */
  __IM uint32_t RESERVED3[8];

  union {
    __IOM uint32_t
      PWM_OP_OVERRIDE_CTRL_SET_REG; /*!< (@ 0x000000C8) output override control set register                       */

    struct {
      __IOM uint32_t OP_OVERRIDE_SYNC : 1; /*!< (@ 0x00000000) Output override is synced with pwm time period
                                                                    depending on operating mode                                */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) reserved1                                                  */
    } PWM_OP_OVERRIDE_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_OP_OVERRIDE_CTRL_RESET_REG; /*!< (@ 0x000000CC) output override control reset register                   */

    struct {
      __IOM uint32_t OP_OVERRIDE_SYNC : 1; /*!< (@ 0x00000000) Output override is synced with pwm time period
                                                                    depending on operating mode                                */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) reserved1                                                  */
    } PWM_OP_OVERRIDE_CTRL_RESET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_OP_OVERRIDE_ENABLE_SET_REG; /*!< (@ 0x000000D0) output override enable set register                      */

    struct {
      __IOM uint32_t
        PWM_OP_OVERRIDE_ENABLE_REG : 8; /*!< (@ 0x00000000) Pwm output over ride enable                            */
      __IOM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_OP_OVERRIDE_ENABLE_SET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_OP_OVERRIDE_ENABLE_RESET_REG; /*!< (@ 0x000000D4) output override enable reset register                  */

    struct {
      __IOM uint32_t
        PWM_OP_OVERRIDE_ENABLE_REG : 8; /*!< (@ 0x00000000) Pwm output over ride enable                            */
      __IOM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_OP_OVERRIDE_ENABLE_RESET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_OP_OVERRIDE_VALUE_SET_REG; /*!< (@ 0x000000D8) output override value set register                        */

    struct {
      __IOM uint32_t
        OP_OVERRIDE_VALUE : 8;       /*!< (@ 0x00000000) Pwm output over ride value.                                */
      __IOM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_OP_OVERRIDE_VALUE_SET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_OP_OVERRIDE_VALUE_RESET_REG; /*!< (@ 0x000000DC) output override enable reset register                   */

    struct {
      __IOM uint32_t
        OP_OVERRIDE_VALUE : 8;       /*!< (@ 0x00000000) Pwm output over ride value.                                */
      __IOM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_OP_OVERRIDE_VALUE_RESET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_FLT_OVERRIDE_CTRL_SET_REG; /*!< (@ 0x000000E0) fault override control set register                       */

    struct {
      __IOM uint32_t FLT_A_MODE : 1; /*!< (@ 0x00000000) Fault A mode                                               */
      __IOM uint32_t FLT_B_MODE : 1; /*!< (@ 0x00000001) Fault B mode                                               */
      __IOM uint32_t OP_POLARITY_H : 1; /*!< (@ 0x00000002) Ouput polarity for high (H3, H2, H1, H0) side
                                                                    signals                                                    */
      __IOM uint32_t OP_POLARITY_L : 1; /*!< (@ 0x00000003) Ouput polarity for low (L3, L2, L1, L0) side
                                                                    signals.                                                   */
      __IOM uint32_t FLT_A_ENABLE : 4;  /*!< (@ 0x00000004) Fault A enable. Separate enable bit is present
                                                                    for channel                                                */
      __IOM uint32_t FLT_B_ENABLE : 4;  /*!< (@ 0x00000008) Fault B enable. Separate enable bit is present
                                                                    for channel                                                */
      __IOM uint32_t
        COMPLEMENTARY_MODE : 4;      /*!< (@ 0x0000000C) PWM I/O pair mode                                          */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_FLT_OVERRIDE_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_FLT_OVERRIDE_CTRL_RESET_REG; /*!< (@ 0x000000E4) fault override control reset register                   */

    struct {
      __IOM uint32_t FLT_A_MODE : 1; /*!< (@ 0x00000000) Fault B mode                                               */
      __IOM uint32_t FLT_B_MODE : 1; /*!< (@ 0x00000001) Fault B mode                                               */
      __IOM uint32_t OP_POLARITY_H : 1; /*!< (@ 0x00000002) Ouput polarity for high (H3, H2, H1, H0) side
                                                                    signals                                                    */
      __IOM uint32_t OP_POLARITY_L : 1; /*!< (@ 0x00000003) Ouput polarity for low (L3, L2, L1, L0) side
                                                                    signals.                                                   */
      __IOM uint32_t FLT_A_ENABLE : 4;  /*!< (@ 0x00000004) Fault A enable. Separate enable bit is present
                                                                    for channel                                                */
      __IOM uint32_t FLT_B_ENABLE : 4;  /*!< (@ 0x00000008) Fault B enable. Separate enable bit is present
                                                                    for channel                                                */
      __IOM uint32_t
        COMPLEMENTARY_MODE : 4;      /*!< (@ 0x0000000C) PWM I/O pair mode                                          */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_FLT_OVERRIDE_CTRL_RESET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_FLT_A_OVERRIDE_VALUE_REG; /*!< (@ 0x000000E8) Fault input A PWM override value                           */

    struct {
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_L0 : 1; /*!< (@ 0x00000000) 0 bit for L0                                          */
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_L1 : 1; /*!< (@ 0x00000001) 1 bit for L1                                          */
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_L2 : 1; /*!< (@ 0x00000002) 2 bit for L2                                          */
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_L3 : 1; /*!< (@ 0x00000003) 3 bit for L3                                          */
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_H0 : 1; /*!< (@ 0x00000004) 4 bit for H0                                          */
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_H1 : 1; /*!< (@ 0x00000005) 5 bit for H1                                          */
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_H2 : 1; /*!< (@ 0x00000006) 6 bit for H2                                          */
      __IOM uint32_t
        PWM_FLT_A_OVERRIDE_VALUE_H3 : 1; /*!< (@ 0x00000007) 7 bit for H3                                          */
      __IOM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_FLT_A_OVERRIDE_VALUE_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_FLT_B_OVERRIDE_VALUE_REG; /*!< (@ 0x000000EC) Fault input B PWM override value                           */

    struct {
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_L0 : 1; /*!< (@ 0x00000000) 0 bit for L0                                          */
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_L1 : 1; /*!< (@ 0x00000001) 1 bit for L1                                          */
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_L2 : 1; /*!< (@ 0x00000002) 2 bit for L2                                          */
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_L3 : 1; /*!< (@ 0x00000003) 3 bit for L3                                          */
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_H0 : 1; /*!< (@ 0x00000004) 4 bit for H0                                          */
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_H1 : 1; /*!< (@ 0x00000005) 5 bit for H1                                          */
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_H2 : 1; /*!< (@ 0x00000006) 6 bit for H2                                          */
      __IOM uint32_t
        PWM_FLT_B_OVERRIDE_VALUE_H3 : 1; /*!< (@ 0x00000007) 7 bit for H3                                          */
      __IOM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } PWM_FLT_B_OVERRIDE_VALUE_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_SVT_CTRL_SET_REG; /*!< (@ 0x000000F0) NONE                                                       */

    struct {
      __IOM uint32_t SVT_ENABLE_FRM : 1; /*!< (@ 0x00000000) Special event trigger enable. This is used to
                                                                    enable generation special event trigger                    */
      __IOM uint32_t
        SVT_DIRECTION_FRM : 1;       /*!< (@ 0x00000001) Special event trigger for time base direction              */
      __IOM uint32_t RESERVED1 : 30; /*!< (@ 0x00000002) reserved1                                                  */
    } PWM_SVT_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_SVT_CTRL_RESET_REG; /*!< (@ 0x000000F4) Special event control reset register                       */

    struct {
      __IOM uint32_t SVT_ENABLE_FRM : 1; /*!< (@ 0x00000000) Special event trigger enable. This is used to
                                                                    enable generation special event trigger                    */
      __IOM uint32_t
        SVT_DIRECTION_FRM : 1;       /*!< (@ 0x00000001) Special event trigger for time base direction              */
      __IOM uint32_t RESERVED1 : 30; /*!< (@ 0x00000002) reserved1                                                  */
    } PWM_SVT_CTRL_RESET_REG_b;
  };

  union {
    __IOM uint32_t PWM_SVT_PARAM_REG; /*!< (@ 0x000000F8) Special event parameter register                           */

    struct {
      __IOM uint32_t SVT_POSTSCALER_SELECT : 4; /*!< (@ 0x00000000) PWM special event trigger output postscale select
                                                                    bits                                                       */
      __IOM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) reserved1                                                  */
    } PWM_SVT_PARAM_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_SVT_COMPARE_VALUE_REG; /*!< (@ 0x000000FC) Special event compare value register                       */

    struct {
      __IOM uint32_t PWM_SVT_COMPARE_VALUE : 16; /*!< (@ 0x00000000) Special event compare value. This is used to
                                                                    compare with pwm time period counter to
                                                                    generate special event trigger                             */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_SVT_COMPARE_VALUE_REG_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_WR_REG_CH1; /*!< (@ 0x00000100) Base timer period register of channel1                     */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_REG_WR_VALUE_CH1 : 16; /*!< (@ 0x00000000) Value to update the base timer period register
                                                                    of channel 1                                               */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_WR_REG_CH1_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_CNTR_WR_REG_CH1; /*!< (@ 0x00000104) Base time counter initial value register for
                                                                    channel1                                                   */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_WR_REG_CH1 : 16; /*!< (@ 0x00000000) To update the base time counter initial value
                                                                    for channel 1                                              */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_CNTR_WR_REG_CH1_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_PARAM_REG_CH1; /*!< (@ 0x00000108) NONE                                                       */

    struct {
      __IOM uint32_t
        TMR_OPEARATING_MODE_CH1 : 3; /*!< (@ 0x00000000) Base timer operating mode for channel1                    */
      __IOM uint32_t RESERVED1 : 1;  /*!< (@ 0x00000003) reserved1                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_PRE_SCALAR_VALUE_CH1 : 3; /*!< (@ 0x00000004) Base timer input clock prescale select value
                                                                    for channel1.                                              */
      __IOM uint32_t RESERVED2 : 1; /*!< (@ 0x00000007) reserved2                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_POST_SCALAR_VALUE_CH1 : 4; /*!< (@ 0x00000008) Time base output post scale bits for channel1  */
      __IOM uint32_t RESERVED3 : 20; /*!< (@ 0x0000000C) reserved3                                                  */
    } PWM_TIME_PRD_PARAM_REG_CH1_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_CTRL_REG_CH1; /*!< (@ 0x0000010C) Base time period control register for channel1             */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< (@ 0x00000000) Time period counter soft reset                      */
      __IOM uint32_t
        PWM_TIME_BASE_EN_FRM_REG_CH1 : 1; /*!< (@ 0x00000001) Base timer enable for channnel1                      */
      __IOM uint32_t PWM_SFT_RST : 1; /*!< (@ 0x00000002) MC PWM soft reset                                          */
      __IOM uint32_t RESERVED1 : 29;  /*!< (@ 0x00000003) reserved1                                                  */
    } PWM_TIME_PRD_CTRL_REG_CH1_b;
  };

  union {
    __IM uint32_t
      PWM_TIME_PRD_STS_REG_CH1; /*!< (@ 0x00000110) Base time period status register for channel1              */

    struct {
      __IM uint32_t
        PWM_TIME_PRD_DIR_STS_CH1 : 1; /*!< (@ 0x00000000) Time period counter direction status for channel1.       */
      __IM uint32_t RESERVED1 : 31;   /*!< (@ 0x00000001) reserved1                                                  */
    } PWM_TIME_PRD_STS_REG_CH1_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_CNTR_VALUE_CH1; /*!< (@ 0x00000114) Time period counter current value for channel1             */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_VALUE_CH1 : 1; /*!< (@ 0x00000000) Time period counter current value for channel1        */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) reserved1                                                  */
    } PWM_TIME_PRD_CNTR_VALUE_CH1_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_WR_REG_CH2; /*!< (@ 0x00000118) Base timer period register of channel2                     */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_REG_WR_VALUE_CH2 : 16; /*!< (@ 0x00000000) Value to update the base timer period register
                                                                    of channel 2                                               */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_WR_REG_CH2_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_CNTR_WR_REG_CH2; /*!< (@ 0x0000011C) Base time counter initial value register for
                                                                    channal2                                                   */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_WR_REG_CH2 : 16; /*!< (@ 0x00000000) To update the base time counter initial value
                                                                    for channel 2                                              */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_CNTR_WR_REG_CH2_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_PARAM_REG_CH2; /*!< (@ 0x00000120) Base time period config parameter's register
                                                                    for channel2                                               */

    struct {
      __IOM uint32_t
        TMR_OPEARATING_MODE_CH2 : 3; /*!< (@ 0x00000000) Base timer operating mode for channel2                    */
      __IOM uint32_t RESERVED1 : 1;  /*!< (@ 0x00000003) reserved1                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_PRE_SCALAR_VALUE_CH2 : 3; /*!< (@ 0x00000004) Base timer input clock pre scale select value
                                                                    for channel2.                                              */
      __IOM uint32_t RESERVED2 : 1; /*!< (@ 0x00000007) reserved2                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_POST_SCALAR_VALUE_CH2 : 4; /*!< (@ 0x00000008) Time base output post scale bits for channel2  */
      __IOM uint32_t RESERVED3 : 20; /*!< (@ 0x0000000C) reserved3                                                  */
    } PWM_TIME_PRD_PARAM_REG_CH2_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_CTRL_REG_CH2; /*!< (@ 0x00000124) Base time period control register for channel2             */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< (@ 0x00000000) Time period counter soft reset                      */
      __IOM uint32_t
        PWM_TIME_BASE_EN_FRM_REG_CH2 : 1; /*!< (@ 0x00000001) Base timer enable for channnel2                      */
      __IOM uint32_t PWM_SFT_RST : 1; /*!< (@ 0x00000002) MC PWM soft reset                                          */
      __IOM uint32_t RESERVED1 : 29;  /*!< (@ 0x00000003) reserved1                                                  */
    } PWM_TIME_PRD_CTRL_REG_CH2_b;
  };

  union {
    __IM uint32_t
      PWM_TIME_PRD_STS_REG_CH2; /*!< (@ 0x00000128) Base time period status register for channel2              */

    struct {
      __IM uint32_t
        PWM_TIME_PRD_DIR_STS_CH2 : 1; /*!< (@ 0x00000000) Time period counter direction status for channel2.       */
      __IM uint32_t RESERVED1 : 31;   /*!< (@ 0x00000001) reserved1                                                  */
    } PWM_TIME_PRD_STS_REG_CH2_b;
  };

  union {
    __IM uint32_t PWM_TIME_PRD_CNTR_VALUE_CH2; /*!< (@ 0x0000012C) Time period counter current value register for
                                                                    channel2                                                   */

    struct {
      __IM uint32_t
        PWM_TIME_PRD_CNTR_VALUE_CH2 : 1; /*!< (@ 0x00000000) Time period counter current value for channel2        */
      __IM uint32_t RESERVED1 : 11; /*!< (@ 0x00000001) reserved1                                                  */
      __IM uint32_t RESERVED2 : 20; /*!< (@ 0x0000000C) reserved2                                                  */
    } PWM_TIME_PRD_CNTR_VALUE_CH2_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_WR_REG_CH3; /*!< (@ 0x00000130) Base timer period register of channel3                     */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_REG_WR_VALUE_CH3 : 16; /*!< (@ 0x00000000) To update the base time counter initial value
                                                                    for channel 3                                              */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_WR_REG_CH3_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_CNTR_WR_REG_CH3; /*!< (@ 0x00000134) Base time counter initial value register for
                                                                    channel3                                                   */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_WR_REG_CH3 : 16; /*!< (@ 0x00000000) Value to update the base timer period register
                                                                    of channel 3                                               */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_CNTR_WR_REG_CH3_b;
  };

  union {
    __IOM uint32_t PWM_TIME_PRD_PARAM_REG_CH3; /*!< (@ 0x00000138) Base time period config parameter's register
                                                                    for channel3                                               */

    struct {
      __IOM uint32_t
        TMR_OPEARATING_MODE_CH3 : 3; /*!< (@ 0x00000000) Base timer operating mode for channel3                    */
      __IOM uint32_t RESERVED1 : 1;  /*!< (@ 0x00000003) reserved1                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_PRE_SCALAR_VALUE_CH3 : 3; /*!< (@ 0x00000004) Base timer input clock pre scale select value
                                                                    for channel2.                                              */
      __IOM uint32_t RESERVED2 : 1; /*!< (@ 0x00000007) reserved2                                                  */
      __IOM uint32_t
        PWM_TIME_PRD_POST_SCALAR_VALUE_CH3 : 4; /*!< (@ 0x00000008) Time base output post scale bits for channel3  */
      __IOM uint32_t RESERVED3 : 20; /*!< (@ 0x0000000C) reserved3                                                  */
    } PWM_TIME_PRD_PARAM_REG_CH3_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_CTRL_REG_CH3; /*!< (@ 0x0000013C) Base time period control register for channel3             */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_CNTR_RST_FRM_REG : 1; /*!< (@ 0x00000000) Time period counter soft reset                      */
      __IOM uint32_t
        PWM_TIME_BASE_EN_FRM_REG_CH3 : 1; /*!< (@ 0x00000001) Base timer enable for channnel3                      */
      __IOM uint32_t PWM_SFT_RST : 1; /*!< (@ 0x00000002) MC PWM soft reset                                          */
      __IOM uint32_t RESERVED1 : 29;  /*!< (@ 0x00000003) reserved1                                                  */
    } PWM_TIME_PRD_CTRL_REG_CH3_b;
  };

  union {
    __IM uint32_t
      PWM_TIME_PRD_STS_REG_CH3; /*!< (@ 0x00000140) Base time period status register for channel3              */

    struct {
      __IM uint32_t
        PWM_TIME_PRD_DIR_STS_CH3 : 1; /*!< (@ 0x00000000) Time period counter direction status for channel3.       */
      __IM uint32_t RESERVED1 : 15;   /*!< (@ 0x00000001) reserved1                                                  */
      __IM uint32_t RESERVED2 : 16;   /*!< (@ 0x00000010) reserved2                                                  */
    } PWM_TIME_PRD_STS_REG_CH3_b;
  };

  union {
    __IM uint32_t PWM_TIME_PRD_CNTR_VALUE_CH3; /*!< (@ 0x00000144) Time period counter current value register for
                                                                    channel3                                                   */

    struct {
      __IM uint32_t
        PWM_TIME_PRD_CNTR_VALUE_CH3 : 16; /*!< (@ 0x00000000) Time period counter current value for channe3        */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } PWM_TIME_PRD_CNTR_VALUE_CH3_b;
  };

  union {
    __IOM uint32_t
      PWM_TIME_PRD_COMMON_REG; /*!< (@ 0x00000148) Time period common register                                */

    struct {
      __IOM uint32_t
        PWM_TIME_PRD_USE_0TH_TIMER_ONLY : 1; /*!< (@ 0x00000000) Instead of use four base timers for four channels,
                                                                    use only one base timer for all channels.                  */
      __IOM uint32_t
        PWM_TIME_PRD_COMMON_TIMER_VALUE : 2;         /*!< (@ 0x00000001) Base timers select to generate special event
                                                                    trigger                                                    */
      __IOM uint32_t USE_EXT_TIMER_TRIG_FRM_REG : 1; /*!< (@ 0x00000003) Enable to use external trigger for base time
                                                                    counter increment or decrement.                            */
      __IOM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) reserved1                                                  */
    } PWM_TIME_PRD_COMMON_REG_b;
  };
} MCPWM_Type; /*!< Size = 332 (0x14c)                                                        */

/* ================================================================================ */
/* ================                      UDMA0                     ================ */
/* ================================================================================ */

/**
 * @brief DMA Performs data transfers along with Addresses and control information (UDMA0)
 */
typedef struct { /*!< (@ 0x44030000) UDMA0 Structure                                            */

  union {
    __IM uint32_t DMA_STATUS; /*!< (@ 0x00000000) UDMA Status Register                                       */

    struct {
      __IM uint32_t MASTER_ENABLE : 1; /*!< (@ 0x00000000) Enable status of controller                                */
      __IM uint32_t RESERVED1 : 3;     /*!< (@ 0x00000001) Reserved1                                                  */
      __IM uint32_t STATE : 4;         /*!< (@ 0x00000004) Current state of the control state machine                 */
      __IM uint32_t RESERVED2 : 8;     /*!< (@ 0x00000008) Reserved2                                                  */
      __IM uint32_t CHNLS_MINUS1 : 5;  /*!< (@ 0x00000010) Number of available DMA channels minus one                 */
      __IM uint32_t RESERVED3 : 7;     /*!< (@ 0x00000015) Reserved3                                                  */
      __IM uint32_t TEST_STATUS : 4;   /*!< (@ 0x0000001C) To reduce the gate count you can configure the
                                                                    controller                                                 */
    } DMA_STATUS_b;
  };

  union {
    __OM uint32_t DMA_CFG; /*!< (@ 0x00000004) DMA Configuration                                          */

    struct {
      __OM uint32_t MASTER_ENABLE : 1; /*!< (@ 0x00000000) Enable for the controller                                  */
      __OM uint32_t RESERVED1 : 4;     /*!< (@ 0x00000001) Reserved1                                                  */
      __OM uint32_t CHNL_PROT_CTRL : 3; /*!< (@ 0x00000005) Sets the AHB-Lite protection by controlling the
                                                                    HPROT[3:1]] signal levels as follows Bit[7]-Controls
                                                                    HPROT[3] to indicate if cacheable access
                                                                    is occurring Bit[6]-Controls HPROT[2] to
                                                                    indicate if cacheable access is occurring
                                                                    Bit[5]-Controls HPROT[1] to indicate if
                                                                    cacheable access is occurring                              */
      __OM uint32_t RESERVED2 : 24; /*!< (@ 0x00000008) Reserved2                                                  */
    } DMA_CFG_b;
  };

  union {
    __IOM uint32_t CTRL_BASE_PTR; /*!< (@ 0x00000008) Channel Control Data Base Pointer                          */

    struct {
      __OM uint32_t RESERVED1 : 10; /*!< (@ 0x00000000) Reserved1                                                  */
      __IOM uint32_t CTRL_BASE_PTR : 22; /*!< (@ 0x0000000A) Pointer to the base address of the primary data
                                                                    structure                                                  */
    } CTRL_BASE_PTR_b;
  };

  union {
    __IM uint32_t ALT_CTRL_BASE_PTR; /*!< (@ 0x0000000C) Channel Alternate Control Data Base Pointer                */

    struct {
      __IM uint32_t
        ALT_CTRL_BASE_PTR : 32; /*!< (@ 0x00000000) Base address of the alternative data structure             */
    } ALT_CTRL_BASE_PTR_b;
  };

  union {
    __IM uint32_t
      DMA_WAITONREQUEST_STATUS; /*!< (@ 0x00000010) Channel Wait on request status register                    */

    struct {
      __IM uint32_t
        DMA_WAITONREQ_STATUS : 32; /*!< (@ 0x00000000) Per Channel wait on request status                         */
    } DMA_WAITONREQUEST_STATUS_b;
  };

  union {
    __OM uint32_t CHNL_SW_REQUEST; /*!< (@ 0x00000014) Channel Software Request                                   */

    struct {
      __OM uint32_t CHNL_SW_REQUEST : 32; /*!< (@ 0x00000000) Set the appropriate bit to generate a software
                                                                    DMA request on the corresponding DMA channel               */
    } CHNL_SW_REQUEST_b;
  };

  union {
    __IOM uint32_t CHNL_USEBURST_SET; /*!< (@ 0x00000018) UDMA Channel use burst set                                 */

    struct {
      __IOM uint32_t CHNL_USEBURST_SET : 32; /*!< (@ 0x00000000) The use burst status, or disables dma_sreq[C]
                                                                    from generating DMA requests.                              */
    } CHNL_USEBURST_SET_b;
  };

  union {
    __OM uint32_t CHNL_USEBURST_CLR; /*!< (@ 0x0000001C) UDMA Channel use burst clear                               */

    struct {
      __OM uint32_t CHNL_USEBURST_CLR : 32; /*!< (@ 0x00000000) Set the appropriate bit to enable dma_sreq[]
                                                                    to generate requests                                       */
    } CHNL_USEBURST_CLR_b;
  };

  union {
    __IOM uint32_t CHNL_REQ_MASK_SET; /*!< (@ 0x00000020) UDMA Channel request mask set Register                     */

    struct {
      __IOM uint32_t CHNL_REQ_MASK_SET : 32; /*!< (@ 0x00000000) Returns the request mask status of dma_req[]
                                                                    and dma_sreq[], or disables the corresponding
                                                                    channel from generating DMA requests                       */
    } CHNL_REQ_MASK_SET_b;
  };

  union {
    __OM uint32_t CHNL_REQ_MASK_CLR; /*!< (@ 0x00000024) UDMA Channel request mask clear                            */

    struct {
      __OM uint32_t CHNL_REQ_MASK_CLR : 32; /*!< (@ 0x00000000) Set the appropriate bit to enable DMA requests
                                                                    for the channel corresponding to dma_req[]
                                                                    and dma_sreq[]                                             */
    } CHNL_REQ_MASK_CLR_b;
  };

  union {
    __IOM uint32_t CHNL_ENABLE_SET; /*!< (@ 0x00000028) UDMA Channel enable register                               */

    struct {
      __IOM uint32_t CHNL_ENABLE_SET : 32; /*!< (@ 0x00000000) This Bits are Used to Load the 16bits of Source
                                                                    address                                                    */
    } CHNL_ENABLE_SET_b;
  };

  union {
    __OM uint32_t CHNL_ENABLE_CLR; /*!< (@ 0x0000002C) UDMA Channel enable clear register                         */

    struct {
      __OM uint32_t CHNL_ENABLE_CLR : 32; /*!< (@ 0x00000000) Set the appropriate bit to disable the corresponding
                                                                    DMA channel                                                */
    } CHNL_ENABLE_CLR_b;
  };

  union {
    __IOM uint32_t CHNL_PRI_ALT_SET; /*!< (@ 0x00000030) UDMA Channel primary or alternate set                      */

    struct {
      __IOM uint32_t CHNL_PRI_ALT_SET : 32; /*!< (@ 0x00000000) Returns the channel control data structure status
                                                                    or selects the alternate data structure
                                                                    for the corresponding DMA channel                          */
    } CHNL_PRI_ALT_SET_b;
  };

  union {
    __OM uint32_t CHNL_PRI_ALT_CLR; /*!< (@ 0x00000034) UDMA Channel primary alternate clear                       */

    struct {
      __OM uint32_t CHNL_PRI_ALT_CLR : 32; /*!< (@ 0x00000000) Set the appropriate bit to select the primary
                                                                    data structure for the corresponding DMA
                                                                    channel                                                    */
    } CHNL_PRI_ALT_CLR_b;
  };

  union {
    __IOM uint32_t CHNL_PRIORITY_SET; /*!< (@ 0x00000038) UDMA Channel Priority Set                                  */

    struct {
      __IOM uint32_t CHNL_PRIORITY_SET : 32; /*!< (@ 0x00000000) Set the appropriate bit to select the primary
                                                                    data structure for the corresponding DMA
                                                                    channel                                                    */
    } CHNL_PRIORITY_SET_b;
  };

  union {
    __OM uint32_t CHNL_PRIORITY_CLR; /*!< (@ 0x0000003C) UDMA Channel Priority Clear                                */

    struct {
      __OM uint32_t CHNL_PRIORITY_CLR : 32; /*!< (@ 0x00000000) Set the appropriate bit to select the default
                                                                    priority level for the specified DMA channel               */
    } CHNL_PRIORITY_CLR_b;
  };
  __IM uint32_t RESERVED[3];

  union {
    __IOM uint32_t ERR_CLR; /*!< (@ 0x0000004C) UDMA Bus Error Clear Register                              */

    struct {
      __IOM uint32_t ERR_CLR : 1;    /*!< (@ 0x00000000) Returns the status of dma_err                              */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved1                                                  */
    } ERR_CLR_b;
  };

  union {
    __IOM uint32_t
      UDMA_SKIP_DESC_FETCH_REG; /*!< (@ 0x00000050) UDMA skip descriptor fetch Register                        */

    struct {
      __IOM uint32_t SKIP_DESC_FETCH : 32; /*!< (@ 0x00000000) improving the performance of transfer and saves
                                                                    bus cycles. This features has to be enabled
                                                                    always.                                                    */
    } UDMA_SKIP_DESC_FETCH_REG_b;
  };
  __IM uint32_t RESERVED1[491];

  union {
    __IOM uint32_t
      UDMA_DONE_STATUS_REG; /*!< (@ 0x00000800) UDMA Done status Register                                  */

    struct {
      __IOM uint32_t
        DONE_STATUS_CHANNEL_0 : 1; /*!< (@ 0x00000000) UDMA done Status of the channel 0                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_1 : 1; /*!< (@ 0x00000001) UDMA done Status of the channel 1                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_2 : 1; /*!< (@ 0x00000002) UDMA done Status of the channel 2                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_3 : 1; /*!< (@ 0x00000003) UDMA done Status of the channel 3                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_4 : 1; /*!< (@ 0x00000004) UDMA done Status of the channel 4                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_5 : 1; /*!< (@ 0x00000005) UDMA done Status of the channel 5                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_6 : 1; /*!< (@ 0x00000006) UDMA done Status of the channel 6                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_7 : 1; /*!< (@ 0x00000007) UDMA done Status of the channel 7                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_8 : 1; /*!< (@ 0x00000008) UDMA done Status of the channel 8                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_9 : 1; /*!< (@ 0x00000009) UDMA done Status of the channel 9                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_10 : 1; /*!< (@ 0x0000000A) UDMA done Status of the channel 10                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_11 : 1; /*!< (@ 0x0000000B) UDMA done Status of the channel 3                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_12 : 1; /*!< (@ 0x0000000C) UDMA done Status of the channel 12                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_13 : 1; /*!< (@ 0x0000000D) UDMA done Status of the channel 13                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_14 : 1; /*!< (@ 0x0000000E) UDMA done Status of the channel 14                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_15 : 1; /*!< (@ 0x0000000F) UDMA done Status of the channel 15                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_16 : 1; /*!< (@ 0x00000010) UDMA done Status of the channel 16                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_17 : 1; /*!< (@ 0x00000011) UDMA done Status of the channel 17                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_18 : 1; /*!< (@ 0x00000012) UDMA done Status of the channel 18                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_19 : 1; /*!< (@ 0x00000013) UDMA done Status of the channel 19                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_20 : 1; /*!< (@ 0x00000014) UDMA done Status of the channel 3                          */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_21 : 1; /*!< (@ 0x00000015) UDMA done Status of the channel 21                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_22 : 1; /*!< (@ 0x00000016) UDMA done Status of the channel 22                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_23 : 1; /*!< (@ 0x00000017) UDMA done Status of the channel 23                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_24 : 1; /*!< (@ 0x00000018) UDMA done Status of the channel 24                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_25 : 1; /*!< (@ 0x00000019) UDMA done Status of the channel 25                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_26 : 1; /*!< (@ 0x0000001A) UDMA done Status of the channel 26                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_27 : 1; /*!< (@ 0x0000001B) UDMA done Status of the channel 27                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_28 : 1; /*!< (@ 0x0000001C) UDMA done Status of the channel 28                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_29 : 1; /*!< (@ 0x0000001D) UDMA done Status of the channel 29                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_30 : 1; /*!< (@ 0x0000001E) UDMA done Status of the channel 30                         */
      __IOM uint32_t
        DONE_STATUS_CHANNEL_31 : 1; /*!< (@ 0x0000001F) UDMA done Status of the channel 31                         */
    } UDMA_DONE_STATUS_REG_b;
  };

  union {
    __IM uint32_t CHANNEL_STATUS_REG; /*!< (@ 0x00000804) Channel status Register                                    */

    struct {
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_0 : 1; /*!< (@ 0x00000000) Reading 1 indicates that the channel 0 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_1 : 1; /*!< (@ 0x00000001) Reading 1 indicates that the channel 1 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_2 : 1; /*!< (@ 0x00000002) Reading 1 indicates that the channel 2 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_3 : 1; /*!< (@ 0x00000003) Reading 1 indicates that the channel 3 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_4 : 1; /*!< (@ 0x00000004) Reading 1 indicates that the channel 4 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_5 : 1; /*!< (@ 0x00000005) Reading 1 indicates that the channel 5 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_6 : 1; /*!< (@ 0x00000006) Reading 1 indicates that the channel 6 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_7 : 1; /*!< (@ 0x00000007) Reading 1 indicates that the channel 7 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_8 : 1; /*!< (@ 0x00000008) Reading 1 indicates that the channel 8 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_9 : 1; /*!< (@ 0x00000009) Reading 1 indicates that the channel 9 is busy     */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_10 : 1; /*!< (@ 0x0000000A) Reading 1 indicates that the channel 10 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_11 : 1; /*!< (@ 0x0000000B) Reading 1 indicates that the channel 11 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_12 : 1; /*!< (@ 0x0000000C) Reading 1 indicates that the channel 12 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_13 : 1; /*!< (@ 0x0000000D) Reading 1 indicates that the channel 13 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_14 : 1; /*!< (@ 0x0000000E) Reading 1 indicates that the channel 14 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_15 : 1; /*!< (@ 0x0000000F) Reading 1 indicates that the channel 15 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_16 : 1; /*!< (@ 0x00000010) Reading 1 indicates that the channel 16 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_17 : 1; /*!< (@ 0x00000011) Reading 1 indicates that the channel 17 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_18 : 1; /*!< (@ 0x00000012) Reading 1 indicates that the channel 18 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_19 : 1; /*!< (@ 0x00000013) Reading 1 indicates that the channel 19 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_20 : 1; /*!< (@ 0x00000014) Reading 1 indicates that the channel 20 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_21 : 1; /*!< (@ 0x00000015) Reading 1 indicates that the channel 21 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_22 : 1; /*!< (@ 0x00000016) Reading 1 indicates that the channel 22 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_23 : 1; /*!< (@ 0x00000017) Reading 1 indicates that the channel 23 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_24 : 1; /*!< (@ 0x00000018) Reading 1 indicates that the channel 24 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_25 : 1; /*!< (@ 0x00000019) Reading 1 indicates that the channel 25 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_26 : 1; /*!< (@ 0x0000001A) Reading 1 indicates that the channel 26 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_27 : 1; /*!< (@ 0x0000001B) Reading 1 indicates that the channel 27 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_28 : 1; /*!< (@ 0x0000001C) Reading 1 indicates that the channel 28 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_29 : 1; /*!< (@ 0x0000001D) Reading 1 indicates that the channel 29 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_30 : 1; /*!< (@ 0x0000001E) Reading 1 indicates that the channel 30 is busy   */
      __IM uint32_t
        BUSY_OR_IDEAL_STATUS_CHANNEL_31 : 1; /*!< (@ 0x0000001F) Reading 1 indicates that the channel 31 is busy   */
    } CHANNEL_STATUS_REG_b;
  };
  __IM uint32_t RESERVED2[8];

  union {
    __IOM uint32_t
      UDMA_CONFIG_CTRL_REG; /*!< (@ 0x00000828) DMA Controller Transfer Length Register                    */

    struct {
      __IOM uint32_t
        SINGLE_REQUEST_ENABLE : 1;   /*!< (@ 0x00000000) Enabled signal for single request                          */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use.                                   */
    } UDMA_CONFIG_CTRL_REG_b;
  };

  union {
    __IOM uint32_t UDMA_INTR_MASK_REG; /*!< DMA Controller Interrupt mask register                               */

    struct {
      __IOM uint32_t UDMA_INTR_MASK : 12; /*!< DMA Controller Interrupt mask                                     */
      uint32_t : 20;
    } UDMA_INTR_MASK_REG_b; /*!< BitSize                                                               */
  };
} UDMA0_Type; /*!< Size = 2092 (0x82c)                                                       */

/* ================================================================================ */
/* ================                      HWRNG                     ================ */
/* ================================================================================ */

/**
 * @brief Random numbers generated are 16-bit random numbers and are generated using either the True random number generator or the Pseudo random number generator. (HWRNG)
 */

typedef struct { /*!< HWRNG Structure                                                       */

  union {
    __IO uint32_t HWRNG_CTRL_REG; /*!< Random Number Generator Control Register                              */

    struct {
      __IO uint32_t HWRNG_RNG_ST : 1;  /*!< This bit is used to start the true number generation.                 */
      __IO uint32_t HWRNG_PRBS_ST : 1; /*!< This bit is used to start the pseudo random number generation         */
      __IO uint32_t SOFT_RESET : 1;
      uint32_t : 29;
    } HWRNG_CTRL_REG_b; /*!< BitSize                                                               */
  };

  union {
    __I uint32_t HWRNG_RAND_NUM_REG; /*!< Hardware Random Number Register                                       */

    struct {
      __I uint32_t HWRNG_RAND_NUM : 32; /*!< Generated random number can be read from this register.               */
    } HWRNG_RAND_NUM_REG_b;             /*!< BitSize                                                               */
  };
} HWRNG_Type;

/* ================================================================================ */
/* ================                     TIMERS                     ================ */
/* ================================================================================ */

/**
 * @brief TIMER can be used to generate various timing events for the software (TIMERS)
 */

typedef struct { /*!< (@ 0x24042000) TIMERS Structure                                           */
  __IOM TIMERS_MATCH_CTRL_Type
    MATCH_CTRL[4]; /*!< (@ 0x00000000) [0..3]                                                     */
  __IM uint32_t RESERVED[24];

  union {
    __IOM uint32_t
      MCUULP_TMR_INTR_STAT; /*!< (@ 0x00000080) Timer Status Register                                      */

    struct {
      __IOM uint32_t TMR0_INTR_STATUS : 1; /*!< (@ 0x00000000) This bit indicates status of the interrupt generated
                                                                    by timer 0                                                 */
      __IOM uint32_t TMR1_INTR_STATUS : 1; /*!< (@ 0x00000001) This bit indicates status of the interrupt generated
                                                                    by timer 1                                                 */
      __IOM uint32_t TMR2_INTR_STATUS : 1; /*!< (@ 0x00000002) This bit indicates status of the interrupt generated
                                                                    by timer 2                                                 */
      __IOM uint32_t TMR3_INTR_STATUS : 1; /*!< (@ 0x00000003) This bit indicates status of the interrupt generated
                                                                    by timer 3                                                 */
      __IM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) reserved1                                                  */
    } MCUULP_TMR_INTR_STAT_b;
  };

  union {
    __IOM uint32_t
      MCUULP_TMR_US_PERIOD_INT; /*!< (@ 0x00000084) Timer micro second period Integral Part Register           */

    struct {
      __IOM uint32_t TMR_US_PERIOD_INT : 16; /*!< (@ 0x00000000) This bits are used to program the integer part
                                                                    of number of clock cycles per microseconds
                                                                    of the system clock used                                   */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } MCUULP_TMR_US_PERIOD_INT_b;
  };

  union {
    __IOM uint32_t
      MCUULP_TMR_US_PERIOD_FRAC; /*!< (@ 0x00000088) Timer microsecond period Fractional Part Register          */

    struct {
      __IOM uint32_t TMR_US_PERIOD_FRAC : 8; /*!< (@ 0x00000000) This bits are used to program the fractional
                                                                    part of number of clock cycles per microseconds
                                                                    of the system clock used                                   */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } MCUULP_TMR_US_PERIOD_FRAC_b;
  };

  union {
    __IOM uint32_t
      MCUULP_TMR_MS_PERIOD_INT; /*!< (@ 0x0000008C) Timer 256 microsecond period Integral Part Register        */

    struct {
      __IOM uint32_t TMR_MS_PERIOD_INT : 16; /*!< (@ 0x00000000) This bits are used to program the integer part
                                                                    of number of clock cycles per 256 microseconds
                                                                    of the system clock used                                   */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } MCUULP_TMR_MS_PERIOD_INT_b;
  };

  union {
    __IOM uint32_t MCUULP_TMR_MS_PERIOD_FRAC; /*!< (@ 0x00000090) Timer 256 microsecond period Fractional Part
                                                                    Register                                                   */

    struct {
      __IOM uint32_t TMR_MS_PERIOD_FRAC : 8; /*!< (@ 0x00000000) This bits are used to program the fractional
                                                                    part of number of clock cycles per 256
                                                                    microseconds of the system clock used                      */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } MCUULP_TMR_MS_PERIOD_FRAC_b;
  };
  __IM uint32_t RESERVED1[2];

  union {
    __IM uint32_t
      MCUULP_TMR_ACTIVE_STATUS; /*!< (@ 0x0000009C) Timer Active Status Register                               */

    struct {
      __IM uint32_t TIMER_ACTIVE : 4; /*!< (@ 0x00000000) Timer active status for each timer. LSB bit specifies
                                                                    the status for 0th timer and so on.                        */
      __IM uint32_t RESERVED1 : 28;   /*!< (@ 0x00000004) reserved1                                                  */
    } MCUULP_TMR_ACTIVE_STATUS_b;
  };
} TIMERS_Type; /*!< Size = 160 (0xa0)                                                         */

/* =========================================================================================================================== */
/* ================                                            CAN                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief The DCAN is a standalone CAN (Controller Area Network) controller widely used in automotive and industrial applications. (CAN)
  */

typedef struct { /*!< (@ 0x45070000) CAN Structure                                              */

  union {
    __IOM uint8_t CAN_MR; /*!< (@ 0x00000000) Mode Register                                              */

    struct {
      __IOM uint8_t AFM : 1;       /*!< (@ 0x00000000) hardware acceptance filter scheme                          */
      __IOM uint8_t LOM : 1;       /*!< (@ 0x00000001) Listen Only Mode                                           */
      __IOM uint8_t RM : 1;        /*!< (@ 0x00000002) Reset Mode                                                 */
      __IOM uint8_t RESERVED1 : 5; /*!< (@ 0x00000003) Reserved1                                                  */
    } CAN_MR_b;
  };

  union {
    __OM uint8_t CAN_CMR; /*!< (@ 0x00000001) Command Register                                           */

    struct {
      __OM uint8_t RESERVED1 : 1; /*!< (@ 0x00000000) Reserved1                                                  */
      __OM uint8_t AT : 1;        /*!< (@ 0x00000001) Abort Transmission                                         */
      __OM uint8_t TR : 1;        /*!< (@ 0x00000002) Transmit Request                                           */
      __IM uint8_t : 1;
      __OM uint8_t RESERVED2 : 4; /*!< (@ 0x00000004) Reserved2                                                  */
    } CAN_CMR_b;
  };

  union {
    __IM uint8_t CAN_SR; /*!< (@ 0x00000002) Status register                                            */

    struct {
      __IM uint8_t BS : 1;        /*!< (@ 0x00000000) Bus Off Status                                             */
      __IM uint8_t ES : 1;        /*!< (@ 0x00000001) Error Status                                               */
      __IM uint8_t TS : 1;        /*!< (@ 0x00000002) Transmit Status                                            */
      __IM uint8_t RS : 1;        /*!< (@ 0x00000003) Receive Status                                             */
      __IM uint8_t RESERVED1 : 1; /*!< (@ 0x00000004) Reserved1                                                  */
      __IM uint8_t TBS : 1;       /*!< (@ 0x00000005) Transmit Buffer Status                                     */
      __IM uint8_t DSO : 1;       /*!< (@ 0x00000006) Data Overrun Status                                        */
      __IM uint8_t RBS : 1;       /*!< (@ 0x00000007) Data Overrun Status                                        */
    } CAN_SR_b;
  };

  union {
    __IOM uint8_t CAN_ISR_IACK; /*!< (@ 0x00000003) Interrupt Status/Acknowledge Register                      */

    struct {
      __IOM uint8_t DOI : 1;       /*!< (@ 0x00000000) Data Overrun Interrupt                                     */
      __IOM uint8_t BEI : 1;       /*!< (@ 0x00000001) Bus Error Interrupt                                        */
      __IOM uint8_t TI : 1;        /*!< (@ 0x00000002) Transmission Interrupt                                     */
      __IOM uint8_t RI : 1;        /*!< (@ 0x00000003) Receive Interrupt                                          */
      __IOM uint8_t EPI : 1;       /*!< (@ 0x00000004) Error Passive Interrupt                                    */
      __IOM uint8_t EWI : 1;       /*!< (@ 0x00000005) Error Warning Interrupt                                    */
      __IOM uint8_t ALI : 1;       /*!< (@ 0x00000006) Arbitration Lost Interrupt                                 */
      __IOM uint8_t RESERVED1 : 1; /*!< (@ 0x00000007) Reserved1                                                  */
    } CAN_ISR_IACK_b;
  };

  union {
    __IOM uint8_t CAN_IMR; /*!< (@ 0x00000004) Interrupt Mask register.Setting appropriate bit
                                                                    in IMR register enables interrupt assigned
                                                                    to it, clearing disables this interrupt                    */

    struct {
      __IOM uint8_t DOIM : 1;      /*!< (@ 0x00000000) mask for DOI interrupt                                     */
      __IOM uint8_t BEIM : 1;      /*!< (@ 0x00000001) mask for BEI interrupt                                     */
      __IOM uint8_t TIM : 1;       /*!< (@ 0x00000002) mask for TI interrupt                                      */
      __IOM uint8_t RIM : 1;       /*!< (@ 0x00000003) mask for RI interrupt                                      */
      __IOM uint8_t EPIM : 1;      /*!< (@ 0x00000004) mask for EPI interrupt                                     */
      __IOM uint8_t EWIM : 1;      /*!< (@ 0x00000005) mask for EWI interrupt                                     */
      __IOM uint8_t ALIM : 1;      /*!< (@ 0x00000006) mask for ALI interrupt                                     */
      __IOM uint8_t RESERVED1 : 1; /*!< (@ 0x00000007) Reserved1                                                  */
    } CAN_IMR_b;
  };

  union {
    __IM uint8_t CAN_RMC; /*!< (@ 0x00000005) Receive Message Counter                                    */

    struct {
      __IM uint8_t RMC : 5;       /*!< (@ 0x00000000) number of stored message frames                            */
      __IM uint8_t RESERVED1 : 3; /*!< (@ 0x00000005) Reserved1                                                  */
    } CAN_RMC_b;
  };

  union {
    __IOM uint8_t CAN_BTIM0; /*!< (@ 0x00000006) BUS TIMING REGISTER 0                                      */

    struct {
      __IOM uint8_t BRP : 6; /*!< (@ 0x00000000) Baud Rate Pre scaler                                       */
      __IOM uint8_t SJW : 2; /*!< (@ 0x00000006) Synchronization Jump Width                                 */
    } CAN_BTIM0_b;
  };

  union {
    __IOM uint8_t CAN_BTIM1; /*!< (@ 0x00000007) BUS TIMING REGISTER 1.define the length of bit
                                                                    period, location of the sample point and
                                                                    number of samples to be taken at each sample
                                                                    point                                                      */

    struct {
      __IOM uint8_t TSEG1 : 4; /*!< (@ 0x00000000) Number of clock cycles per Time Segment 1                  */
      __IOM uint8_t TSEG2 : 3; /*!< (@ 0x00000004) Number of clock cycles per Time Segment 1                  */
      __IOM uint8_t SAM : 1;   /*!< (@ 0x00000007) Number of bus level samples                                */
    } CAN_BTIM1_b;
  };

  union {
    __OM uint32_t CAN_TXBUF; /*!< (@ 0x00000008) TRANSMIT BUFFER REGISTER                                   */

    struct {
      __OM uint32_t TXBUF0 : 8; /*!< (@ 0x00000000) Transmit Buffer Register is used to write CAN
                                                                    frame destined to send over CAN network.                   */
      __OM uint32_t TXBUF1 : 8; /*!< (@ 0x00000008) Transmit Buffer Register is used to write CAN
                                                                    frame destined to send over CAN network.                   */
      __OM uint32_t TXBUF2 : 8; /*!< (@ 0x00000010) Transmit Buffer Register is used to write CAN
                                                                    frame destined to send over CAN network.                   */
      __OM uint32_t TXBUF3 : 8; /*!< (@ 0x00000018) Transmit Buffer Register is used to write CAN
                                                                    frame destined to send over CAN network.                   */
    } CAN_TXBUF_b;
  };

  union {
    __IM uint32_t CAN_RXBUF; /*!< (@ 0x0000000C) RECEIVE BUFFER REGISTER                                    */

    struct {
      __IM uint32_t RXBUF0 : 8; /*!< (@ 0x00000000) Receive Buffer Register is used to read CAN frames
                                                                    received by the DCAN core from CAN network                 */
      __IM uint32_t RXBUF1 : 8; /*!< (@ 0x00000008) Receive Buffer Register is used to read CAN frames
                                                                    received by the DCAN core from CAN network                 */
      __IM uint32_t RXBUF2 : 8; /*!< (@ 0x00000010) Receive Buffer Register is used to read CAN frames
                                                                    received by the DCAN core from CAN network                 */
      __IM uint32_t RXBUF3 : 8; /*!< (@ 0x00000018) Receive Buffer Register is used to read CAN frames
                                                                    received by the DCAN core from CAN network                 */
    } CAN_RXBUF_b;
  };

  union {
    __IOM uint32_t CAN_ACR; /*!< (@ 0x00000010) ACCEPTANCE CODE REGISTER                                   */

    struct {
      __IOM uint32_t ACR0 : 8; /*!< (@ 0x00000000) The acceptance code registers contains bit patterns
                                                                    of messages to be received                                 */
      __IOM uint32_t ACR1 : 8; /*!< (@ 0x00000008) The acceptance code registers contains bit patterns
                                                                    of messages to be received                                 */
      __IOM uint32_t ACR2 : 8; /*!< (@ 0x00000010) The acceptance code registers contains bit patterns
                                                                    of messages to be received                                 */
      __IOM uint32_t ACR3 : 8; /*!< (@ 0x00000018) The acceptance code registers contains bit patterns
                                                                    of messages to be received                                 */
    } CAN_ACR_b;
  };

  union {
    __IOM uint32_t CAN_AMR; /*!< (@ 0x00000014) ACCEPTANCE MASK REGISTER                                   */

    struct {
      __IOM uint32_t AMR0 : 8; /*!< (@ 0x00000000) Acceptance mask registers defines which bit positions
                                                                    will be compared and which ones are do
                                                                    not care. Setting certain AMR bit define
                                                                    corresponding bit in ACR as do not care.                   */
      __IOM uint32_t AMR1 : 8; /*!< (@ 0x00000008) acceptance mask registers defines which bit positions
                                                                    will be compared and which ones are do
                                                                    not care. Setting certain AMR bit define
                                                                    corresponding bit in ACR as do not care.                   */
      __IOM uint32_t AMR2 : 8; /*!< (@ 0x00000010) acceptance mask registers defines which bit positions
                                                                    will be compared and which ones are do
                                                                    not care. Setting certain AMR bit define
                                                                    corresponding bit in ACR as do not care.                   */
      __IOM uint32_t AMR3 : 8; /*!< (@ 0x00000018) acceptance mask registers defines which bit positions
                                                                    will be compared and which ones are do
                                                                    not care. Setting certain AMR bit define
                                                                    corresponding bit in ACR as do not care.                   */
    } CAN_AMR_b;
  };

  union {
    __IM uint8_t CAN_ECC; /*!< (@ 0x00000018) ERROR CODE CAPTURE REGISTER                                */

    struct {
      __IM uint8_t BER : 1;   /*!< (@ 0x00000000) Bit Error occurred                                         */
      __IM uint8_t STFER : 1; /*!< (@ 0x00000001) stuff error occurred                                       */
      __IM uint8_t CRCER : 1; /*!< (@ 0x00000002) CRC error occurred                                         */
      __IM uint8_t FRMER : 1; /*!< (@ 0x00000003) Frame error occurred                                       */
      __IM uint8_t ACKER : 1; /*!< (@ 0x00000004) Acknowledgement error occurred                             */
      __IM uint8_t EDIR : 1;  /*!< (@ 0x00000005) direction of transfer while error occurred                 */
      __IM uint8_t TXWRN : 1; /*!< (@ 0x00000006) set when TXERR counter is greater than or equal
                                                                    to 96                                                      */
      __IM uint8_t RXWRN : 1; /*!< (@ 0x00000007) set when RXERR counter is greater than or equal
                                                                    to 96                                                      */
    } CAN_ECC_b;
  };

  union {
    __IM uint8_t CAN_RXERR; /*!< (@ 0x00000019) RECEIVE ERROR COUNTER REGISTER                             */

    struct {
      __IM uint8_t RXERR : 8; /*!< (@ 0x00000000) The RXERR register reflects current value of
                                                                    the receive error counter                                  */
    } CAN_RXERR_b;
  };

  union {
    __IM uint8_t CAN_TXERR; /*!< (@ 0x0000001A) RECEIVE ERROR COUNTER REGISTER                             */

    struct {
      __IM uint8_t TXERR : 8; /*!< (@ 0x00000000) The TXERR register reflects current value of
                                                                    the transmit error counter                                 */
    } TXERR_b;
  };

  union {
    __IM uint8_t CAN_ALC; /*!< (@ 0x0000001B) ARBITRATION LOST CODE CAPTURE REGISTER                     */

    struct {
      __IM uint8_t ALC : 5;       /*!< (@ 0x00000000) Arbitration Lost Capture                                   */
      __IM uint8_t RESERVED1 : 3; /*!< (@ 0x00000005) Reserved1                                                  */
    } CAN_ALC_b;
  };
} CAN_Type; /*!< Size = 28 (0x1c)                                                          */

/* ================================================================================ */
/* ================                       QEI                      ================ */
/* ================================================================================ */

/**
 * @brief The Motor Control PWM (MCPWM) controller is used to generate a periodic pulse waveform, which is useful in motor control and power control applications (QEI)
 */

typedef struct { /*!< QEI Structure                                                         */

  union {
    __I uint32_t QEI_STATUS_REG; /*!< QEI Status Register                                                   */

    struct {
      __I uint32_t QEI_INDEX : 1;         /*!< This is a direct value from the position signal generator             */
      __I uint32_t QEI_POSITION_B : 1;    /*!< This is a direct value from the position signal generator.Value
                                                         refers to the signal Position_B from the generator.                   */
      __I uint32_t QEI_POSITION_A : 1;    /*!< This is a direct value from the position signal generator.Value
                                                         refers to the signal Position_A from the generator.                   */
      __I uint32_t POSITION_CNTR_ERR : 1; /*!< Count Error Status Flag bit                                           */
      __I uint32_t
        POSITION_CNTR_DIRECTION : 1; /*!< Position Counter Direction Status bit                                 */
    } QEI_STATUS_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_CTRL_REG_SET; /*!< QEI Control Set Register                                              */

    struct {
      __I uint32_t QEI_SFT_RST : 1;        /*!< Quadrature encoder soft reset. It is self reset signal.               */
      __IO uint32_t QEI_SWAP_PHASE_AB : 1; /*!< Phase A and Phase B Input Swap Select bit                             */
      __IO uint32_t
        POS_CNT_RST_WITH_INDEX_EN : 1; /*!< Phase A and Phase B Input Swap Select bit                             */
      uint32_t : 1;
      __IO uint32_t
        POS_CNT_DIRECTION_CTRL : 1; /*!< NONE                                                                  */
      __IO uint32_t
        POS_CNT_DIR_FRM_REG : 1; /*!< Position Counter Direction indication from user                       */
      uint32_t : 2;
      __IO uint32_t INDEX_CNT_RST_EN : 1; /*!< NONE                                                                  */
      __IO uint32_t
        DIGITAL_FILTER_BYPASS : 1;  /*!< NONE                                                                  */
      __IO uint32_t TIMER_MODE : 1; /*!< NONE                                                                  */
      __IO uint32_t
        START_VELOCITY_CNTR : 1;          /*!< Starting the velocity counter. It is self reset bit.                  */
      __IO uint32_t QEI_STOP_IN_IDLE : 1; /*!< NONE                                                                  */
      __IO uint32_t
        QEI_POS_CNT_16_BIT_MODE : 1;   /*!< Qei position counter 16 bit mode enable                               */
      __IO uint32_t POS_CNT_RST : 1;   /*!< 1=position counter is going to reset                                  */
      __IO uint32_t INDEX_CNT_RST : 1; /*!< 1= index counter is going to reset.                                   */
    } QEI_CTRL_REG_SET_b;              /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_CTRL_REG_RESET; /*!< PWM Interrupt mask Register                                           */

    struct {
      __I uint32_t QEI_SFT_RST : 1;        /*!< Quadrature encoder soft reset. It is self reset signal                */
      __IO uint32_t QEI_SWAP_PHASE_AB : 1; /*!< Phase A and Phase B Input Swap Select bit                             */
      __IO uint32_t
        POS_CNT_RST_WITH_INDEX_EN : 1; /*!< Phase A and Phase B Input Swap Select bit                             */
      uint32_t : 1;
      __IO uint32_t
        POS_CNT_DIRECTION_CTRL : 1; /*!< NONE                                                                  */
      __IO uint32_t
        POS_CNT_DIR_FRM_REG : 1; /*!< Position Counter Direction indication from user                       */
      uint32_t : 2;
      __IO uint32_t INDEX_CNT_RST_EN : 1; /*!< NONE                                                                  */
      __IO uint32_t
        DIGITAL_FILTER_BYPASS : 1;  /*!< NONE                                                                  */
      __IO uint32_t TIMER_MODE : 1; /*!< NONE                                                                  */
      __IO uint32_t
        START_VELOCITY_CNTR : 1;          /*!< Starting the velocity counter. It is self reset bit.                  */
      __IO uint32_t QEI_STOP_IN_IDLE : 1; /*!< NONE                                                                  */
      __IO uint32_t
        QEI_POS_CNT_16_BIT_MODE : 1;   /*!< Qei position counter 16 bit mode enable                               */
      __IO uint32_t POS_CNT_RST : 1;   /*!< 1=position counter is going to reset                                  */
      __IO uint32_t INDEX_CNT_RST : 1; /*!< 1= index counter is going to reset.                                   */
    } QEI_CTRL_REG_RESET_REg_b;        /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_CNTLR_INIT_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t QEI_ENCODING_MODE : 2; /*!< NONE                                                                  */
      uint32_t : 2;
      __IO uint32_t INDEX_MATCH_VALUE : 2; /*!< These bits allow user to specify the state of position A and
                                                         B during index pulse generation.                                      */
      __IO uint32_t DF_CLK_DIVIDE_SLT : 4; /*!< Digital Filter Clock Divide Select bits                               */
      __IO uint32_t
        UNIDIRECTIONAL_VELOCITY : 1; /*!< Uni directional velocity enable.                                      */
      __IO uint32_t
        UNIDIRECTIONAL_INDEX : 1;       /*!< Uni directional index enable.                                         */
      __IO uint32_t INDEX_CNT_INIT : 1; /*!< Index counter initial value in unidirectional index enable mode.      */
    } QEI_CNTLR_INIT_REG_b;             /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_INDEX_CNT_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t QEI_INDEX_CNT : 16; /*!< Index counter value.User can initialize/change the index counter
                                                         using this register                                                   */
    } QEI_INDEX_CNT_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_INDEX_MAX_CNT_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t
        QEI_INDEX_MAX_CNT : 16; /*!< Qei index maximum count.                                              */
    } QEI_INDEX_MAX_CNT_REG_b;  /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_POSITION_CNT_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t QEI_POSITION_CNT_WR_VALUE_L : 16; /*!< This is used to program/change the value of position counter
                                                         status[15:0]                                                          */
      __IO uint32_t QEI_POSITION_CNT_WR_VALUE_H : 16; /*!< This is used to program/change the value of position counter
                                                         status[31:16].                                                        */
    } QEI_POSITION_CNT_REG_b; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED;

  union {
    __IO uint32_t
      QEI_POSITION_MAX_CNT_LSW_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t QEI_POSITION_MAX_CNT_L : 16; /*!< Qei position maximum count [15:0].This is a maximum count value
                                                         that is allowed to increment in the position counter                  */
      __IO uint32_t QEI_POSITION_MAX_CNT_H : 16; /*!< Qei position maximum count [31:16].This is a maximum count value
                                                         that is allowed to increment in the position counter                  */
    } QEI_POSITION_MAX_CNT_LSW_REG_b; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED1;

  union {
    __I uint32_t QEI_INTR_STS_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t
        QEI_POSITION_CNT_RESET_INTR_LEV : 1; /*!< This is raised when the position counter reaches it's extremes   */
      __I uint32_t QEI_INDEX_CNT_MATCH_INTR_LEV : 1; /*!< This is raised when index counter reaches max value loaded in
                                                         to index_max_cnt register.                                            */
      __I uint32_t POSITION_CNTR_ERR_INTR_LEV : 1; /*!< Whenever number of possible positions are mismatched with actual
                                                         positions are received between two index pulses this will raised      */
      __I uint32_t
        VELOCITY_LESS_THAN_INTR_LEV : 1; /*!< When velocity count is less than the value given in velocity_value_to_comp
                                                         are register, interrupt is raised                                     */
      __I uint32_t
        QEI_POSITION_CNT_MATCH_INTR_LEV : 1; /*!< This is raised when the position counter reaches position match
                                                         value, which is programmable.                                         */
    } QEI_INTR_STS_REG_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_INTR_ACK_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t
        QEI_POSITION_CNT_RESET_INTR_LEV : 1; /*!< Qei_position_cnt_reset_intr_ack                                  */
      __IO uint32_t
        QEI_INDEX_CNT_MATCH_INTR_LEV : 1; /*!< NONE                                                                */
      __IO uint32_t
        POSITION_CNTR_ERR_INTR_LEV : 1; /*!< Position_cntr_err_intr_ack                                            */
      __IO uint32_t
        VELOCITY_LESS_THAN_INTR_LEV : 1; /*!< Velocity_less_than_intr_ack                                          */
      __IO uint32_t
        QEI_POSITION_CNT_MATCH_INTR_LEV : 1; /*!< Qei_position_cnt_match_intr_ack                                  */
    } QEI_INTR_ACK_REG_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_INTR_MASK_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t
        QEI_POSITION_CNT_RESET_INTR_MASK : 1; /*!< Qei_position_cnt_reset_intr_mask                                */
      __IO uint32_t
        QEI_INDEX_CNT_MATCH_INTR_MASK : 1; /*!< Qei_index_cnt_match_intr_mask                                      */
      __IO uint32_t
        POSITION_CNTR_ERR_INTR_MASK : 1; /*!< Position_cntr_err_intr_mask                                          */
      __IO uint32_t
        VELOCITY_LESS_THAN_INTR_MASK : 1; /*!< Velocity_less_than_intr_mask                                        */
      __IO uint32_t
        QEI_POSITION_CNT_MATCH_INTR_MASK : 1; /*!< Qei_position_cnt_match_intr_mask                                */
    } QEI_INTR_MASK_REG_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_INTR_UNMASK_REg; /*!< NONE                                                                  */

    struct {
      __IO uint32_t
        QEI_POSITION_CNT_RESET_INTR_UNMASK : 1; /*!< Qei_position_cnt_reset_intr_unmask                            */
      __IO uint32_t
        QEI_INDEX_CNT_MATCH_INTR_UNMASK : 1; /*!< Qei_index_cnt_match_intr_unmask                                  */
      __IO uint32_t
        POSITION_CNTR_ERR_INTR_UNMASK : 1; /*!< Position_cntr_err_intr_unmask                                      */
      __IO uint32_t
        VELOCITY_LESS_THAN_INTR_UNMASK : 1; /*!< Velocity_less_than_intr_unmask                                    */
      __IO uint32_t
        QEI_POSITION_CNT_MATCH_INTR_UNMASK : 1; /*!< Qei_position_cnt_match_intr_unmask                            */
    } QEI_INTR_UNMASK_REg_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_CLK_FREQ_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t QEI_CLK_FREQ : 9; /*!< Indication of clock frequency on which QEI controller is running.     */
    } QEI_CLK_FREQ_REG_b;             /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QEI_DELTA_TIME_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t
        DELTA_TIME_FOR_VELOCITY : 20; /*!< Delta time LSW to compute velocity                                    */
    } QEI_DELTA_TIME_REG_b;           /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED2;

  union {
    __IO uint32_t QEI_VELOCITY_REG; /*!< NONE                                                                  */

    struct {
      __I uint32_t VELOCITY_VALUE_TO_COMPARE_L : 16; /*!< If read :Velocity value to compare with velocity count LSW If
                                                         write :Velocity LSW count to compare using NWP firmware                */
      __O uint32_t VELOCITY_VALUE_TO_COMPARE_H : 16; /*!< If write :Velocity value to compare with velocity count MSW.
                                                         If read :Velocity MSW count to compare using NWP firmware              */
    } QEI_VELOCITY_REG_b; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED3;

  union {
    __IO uint32_t QEI_POSITION_MATCH_REG; /*!< NONE                                                                  */

    struct {
      __IO uint32_t
        POSTION_MATCH_VALUE_L : 16; /*!< Position match value to compare the position counter.                 */
      __IO uint32_t
        POSTION_MATCH_VALUE_H : 16; /*!< Position match value to compare the position counter.                 */
    } QEI_POSITION_MATCH_REG_b;     /*!< BitSize                                                               */
  };
} QEI_Type;

/* ================================================================================ */
/* ================                      UART0                     ================ */
/* ================================================================================ */

/**
 * @brief Universal Asynchronous Receiver/Transmitter is for serial communication with peripherals,modems and datasets (UART0)
 */

typedef struct { /*!< UART0 Structure                                                       */

  union {
    union {
      __IO uint32_t DLL; /*!< Divisor Latch Low                                                     */

      struct {
        __IO uint32_t DLL : 8; /*!< Lower 8-bits of a 16-bit, read/write, Divisor Latch register
                                                         that contains the baud ratedivisor for the UART                       */
      } DLL_b;                 /*!< BitSize                                                               */
    };

    union {
      __O uint32_t THR; /*!< Transmit Holding Register                                             */

      struct {
        __O uint32_t THR : 8; /*!< Data to be transmitted on serial output port                          */
      } THR_b;                /*!< BitSize                                                               */
    };

    union {
      __I uint32_t RBR; /*!< Receive Buffer Register                                               */

      struct {
        __I uint32_t RBR : 8; /*!< Receive Buffer Field                                                  */
      } RBR_b;                /*!< BitSize                                                               */
    };
  };

  union {
    union {
      __IO uint32_t IER; /*!< Interrupt Enable Register                                             */

      struct {
        __IO uint32_t ERBFI : 1; /*!< Enable Received Data Available Interrupt                              */
        __IO uint32_t ETBEI : 1; /*!< Enable Transmit Holding Register Empty Interrupt                      */
        __IO uint32_t ELSI : 1;  /*!< Enable Receiver Line Status Interrupt                                 */
        __IO uint32_t EDSSI : 1; /*!< Enable Modem Status Interrupt                                         */
        uint32_t : 3;
        __IO uint32_t PTIME : 1; /*!< Programmable THRE Interrupt Mode Enable                               */
      } IER_b;                   /*!< BitSize                                                               */
    };

    union {
      __IO uint32_t DLH; /*!< Divisor Latch High                                                    */

      struct {
        __IO uint32_t DLH : 8; /*!< Upper 8-bits of a 16-bit, read/write, Divisor Latch register
                                                         that contains the baud ratedivisor for the UART                       */
      } DLH_b;                 /*!< BitSize                                                               */
    };
  };

  union {
    union {
      __O uint32_t FCR; /*!< FIFO Control Register                                                 */

      struct {
        __O uint32_t FIFOE : 1;  /*!< This enables/disables the transmit (XMIT) and receive (RCVR)
                                                         FIFOs                                                                 */
        __O uint32_t RFIFOR : 1; /*!< RCVR FIFO Reset                                                       */
        __O uint32_t XFIFOR : 1; /*!< XMIT FIFO Reset                                                       */
        __O uint32_t DMAM : 1;   /*!< DMA signalling mode                                                   */
        __O uint32_t TET : 2;    /*!< TX Empty Trigger                                                      */
        __O uint32_t RT : 2;     /*!< This is used to select the trigger level in the receiver FIFO
                                                         at which the Received Data Available Interrupt is generated
				 */
      } FCR_b;                   /*!< BitSize                                                               */
    };

    union {
      __I uint32_t IIR; /*!< Interrupt Identity Register                                           */

      struct {
        __I uint32_t IID : 4; /*!< Interrupt ID                                                          */
        uint32_t : 2;
        __I uint32_t FIFOSE : 2; /*!< This is used to indicate whether the FIFOs are enabled or disabled.
				 */
      } IIR_b;                   /*!< BitSize                                                               */
    };
  };

  union {
    __IO uint32_t LCR; /*!< Line Control Register                                                 */

    struct {
      __IO uint32_t DLS : 2;          /*!< Data Length Select,This is used to select the number of data
                                                         bits per character that the peripheral transmits and receives
			 */
      __IO uint32_t STOP : 1;         /*!< This is used to select the number of stop bits per character
                                                         that the peripheral transmits and receives                            */
      __IO uint32_t PEN : 1;          /*!< This bit is used to enable and disable parity generation and
                                                         detection in transmitted and received serial character                */
      __IO uint32_t EPS : 1;          /*!< This is used to select between even and odd parity                    */
      __IO uint32_t STICK_PARITY : 1; /*!< This bit is used to force parity value                                */
      __IO uint32_t BC : 1;           /*!< This is used to cause a break condition to be transmitted to
                                                         the receiving device                                                  */
      __IO uint32_t DLAB : 1;         /*!< This bit is used to enable reading and writing of the Divisor
                                                         Latch register to set the baud rate of the UART                       */
    } LCR_b;                          /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MCR; /*!< Modem Control Register                                                */

    struct {
      __IO uint32_t DTR : 1;  /*!< This is used to directly control the Data Terminal Ready (dtr_n)
                                                         output                                                                */
      __IO uint32_t RTS : 1;  /*!< This is used to directly control the Request to Send (rts_n)
                                                         output                                                                */
      __IO uint32_t OUT1 : 1; /*!< This is used to directly control the user-designated Output1
                                                         (out1_n) output                                                       */
      __IO uint32_t OUT2 : 1; /*!< This is used to directly control the user-designated Output2
                                                         (out2_n) output                                                       */
      __IO uint32_t LB : 1;   /*!< This is used to put the UART into a diagnostic mode for test
                                                         purposes                                                              */
      __IO uint32_t AFCE : 1; /*!< This is used to directly control the user-designated Output2
                                                         (out2_n) output                                                       */
      __IO uint32_t SIRE : 1; /*!< This is used to enable/disable the IrDA SIR Mode features             */
    } MCR_b;                  /*!< BitSize                                                               */
  };

  union {
    __I uint32_t LSR; /*!< Line Status Register                                                  */

    struct {
      __I uint32_t DR : 1;   /*!< This is used to indicate that the receiver contains at least
                                                         one character in the RBR or the receiver FIFO                         */
      __I uint32_t OE : 1;   /*!< This is used to indicate the occurrence of an overrun error           */
      __I uint32_t PE : 1;   /*!< This is used to indicate the occurrence of a parity error in
                                                         the receiver if the Parity Enable (PEN) bit (LCR[3]) is set
			 */
      __I uint32_t FE : 1;   /*!< This is used to indicate the occurrence of a framing error in
                                                         the receiver                                                          */
      __I uint32_t BI : 1;   /*!< his is used to indicate the detection of a break sequence on
                                                         the serial input data                                                 */
      __I uint32_t THRE : 1; /*!< Transmit Holding Register Empty bit                                   */
      __I uint32_t TEMT : 1; /*!< Transmitter Empty bit                                                 */
      __I uint32_t RFE : 1;  /*!< This is used to indicate if there is at least one parity error,framing
                                                         error, or break indication in the FIFO                                */
    } LSR_b;                 /*!< BitSize                                                               */
  };

  union {
    __I uint32_t MSR; /*!< Modem Status Register                                                 */

    struct {
      __I uint32_t DCTS : 1; /*!< This is used to indicate that the modem control line cts_n has
                                                         changed since the last time the MSR was read                          */
      __I uint32_t DDSR : 1; /*!< This is used to indicate that the modem control line dsr_n has
                                                         changed since the last time the MSR was read                          */
      __I uint32_t TERI : 1; /*!< This is used to indicate that a change on the input ri_n(from
                                                         an active-low to an inactive-high state) has occurred since
                                                          the last time the MSR was read                                       */
      __I uint32_t DDCD : 1; /*!< This is used to indicate that the modem control line dcd_n has
                                                         changed since the last time the MSR was read                          */
      __I uint32_t CTS : 1;  /*!< This is used to indicate the current state of the modem control
                                                         line cts_n                                                            */
      __I uint32_t DSR : 1;  /*!< This is used to indicate the current state of the modem control
                                                         line dsr_n                                                            */
      __I uint32_t RI : 1;   /*!< This is used to indicate the current state of the modem control
                                                         line ri_n                                                             */
      __I uint32_t DCD : 1;  /*!< This is used to indicate the current state of the modem control
                                                         line dcd_n                                                            */
    } MSR_b;                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t SCR; /*!< Scratch pad Register                                                  */

    struct {
      __IO uint32_t SCRATCH_PAD : 8; /*!< This register is for programmers to use as a temporary storage
                                                         space. It has no defined purpose                                      */
    } SCR_b;                         /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t LPDLL; /*!< Low Power Divisor Latch Low Register                                  */

    struct {
      __IO uint32_t LOW_POWER_DLL : 8; /*!< This register makes up the lower 8-bits of a 16-bit, read/write,
                                                         Low Power Divisor Latch register that contains the baud rate
                                                          divisor for the UART, which must give a baud rate of 115.2K
			 */
    } LPDLL_b;                         /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t LPDLH; /*!< Low Power Divisor Latch High Register                                 */

    struct {
      __IO uint32_t LOW_POWER_DLH : 8; /*!< This register makes up the upper 8-bits of a 16-bit, read/write,
                                                         Low Power Divisor Latch register that contains the baud rate
                                                          divisor for the UART, which must give a baud rate of 115200
			 */
    } LPDLH_b;                         /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED[6];

  union {
    __IO uint32_t HDEN; /*!< none                                                                  */

    struct {
      __IO uint32_t FULL_DUPLEX_MODE : 1; /*!< none                                                                  */
      __IO uint32_t TX_MODE_RX_MODE : 1;  /*!< This signal is valid when full_duplex_mode is disabled                */
    } HDEN_b;                             /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED1[5];

  union {
    __IO uint32_t SMCR; /*!< none                                                                  */

    struct {
      __IO uint32_t SYNC_MODE : 1; /*!< none                                                                  */
      __IO uint32_t MST_MODE : 1;  /*!< none                                                                  */
      uint32_t : 2;
      __IO uint32_t CONTI_CLK_MODE : 1; /*!< none                                                                  */
      __IO uint32_t START_STOP_EN : 1;  /*!< none                                                                  */
    } SMCR_b;                           /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED2[5];

  union {
    __IO uint32_t FAR; /*!< none                                                                  */

    struct {
      __IO uint32_t SYNC_MODE : 1; /*!< none                                                                  */
    } FAR_b;                       /*!< BitSize                                                               */
  };

  union {
    __I uint32_t TFR; /*!< none                                                                  */

    struct {
      __IO uint32_t TX_FIFO_RD : 8; /*!< Transmit FIFO Read                                                    */
    } TFR_b;                        /*!< BitSize                                                               */
  };

  union {
    __I uint32_t RFW; /*!< none                                                                  */

    struct {
      __IO uint32_t RFWD : 8; /*!< Receive FIFO Write Data                                               */
      __IO uint32_t RFPE : 1; /*!< Receive FIFO Parity Error                                             */
      __IO uint32_t RFFE : 1; /*!< Receive FIFO Framing Error                                            */
    } RFW_b;                  /*!< BitSize                                                               */
  };

  union {
    __I uint32_t USR; /*!< UART Status Register                                                  */

    struct {
      __I uint32_t BUSY : 1; /*!< Indicates that a serial transfer is in progress                       */
      __I uint32_t TFNF : 1; /*!< To Indicate that the transmit FIFO is not full                        */
      __I uint32_t TFE : 1;  /*!< To Indicate that the transmit FIFO is completely empty                */
      __I uint32_t RFNE : 1; /*!< To Indicate that the receive FIFO contains one or more entries        */
      __I uint32_t RFE : 1;  /*!< To Indicate that the receive FIFO is completely full                  */
    } USR_b;                 /*!< BitSize                                                               */
  };

  union {
    __I uint32_t TFL; /*!< Transmit FIFO Level                                                   */

    struct {
      __I uint32_t FIFO_ADDR_WIDTH : 30; /*!< Transmit FIFO Level. This is indicates the number of data entries
                                                         in the transmit FIFO.                                                 */
    } TFL_b;                             /*!< BitSize                                                               */
  };

  union {
    __I uint32_t RFL; /*!< Receive FIFO Level                                                    */

    struct {
      __I uint32_t FIFO_ADDR_WIDTH : 30; /*!< Receive FIFO Level. This is indicates the number of data entries
                                                         in the receive FIFO.                                                  */
    } RFL_b;                             /*!< BitSize                                                               */
  };

  union {
    __O uint32_t SRR; /*!< Software Reset Register                                               */

    struct {
      __O uint32_t UR : 1;  /*!< UART Reset                                                            */
      __O uint32_t RFR : 1; /*!< RCVR FIFO Reset                                                       */
      __O uint32_t XFR : 1; /*!< XMIT FIFO Reset                                                       */
    } SRR_b;                /*!< BitSize                                                               */
  };

  union {
    __I uint32_t SRTS; /*!< Shadow Request to Send                                                */

    struct {
      __I uint32_t SRTS : 1; /*!< Shadow Request to Send.                                               */
    } SRTS_b;                /*!< BitSize                                                               */
  };

  union {
    __I uint32_t SBCR; /*!< Shadow Break Control Register                                         */

    struct {
      __I uint32_t SBCR : 1; /*!< Shadow Break Control Bit                                              */
    } SBCR_b;                /*!< BitSize                                                               */
  };

  union {
    __I uint32_t SDMAM; /*!< Shadow DMA Mode                                                       */

    struct {
      __I uint32_t SDMAM : 1; /*!< Shadow DMA Mode                                                       */
    } SDMAM_b;                /*!< BitSize                                                               */
  };

  union {
    __I uint32_t SFE; /*!< Shadow FIFO Enable                                                    */

    struct {
      __I uint32_t SFE : 1; /*!< Shadow FIFO Enable                                                    */
    } SFE_b;                /*!< BitSize                                                               */
  };

  union {
    __I uint32_t SRT; /*!< Shadow RCVR Trigger                                                   */

    struct {
      __I uint32_t SRT : 2; /*!< Shadow RCVR Trigger                                                   */
    } SRT_b;                /*!< BitSize                                                               */
  };

  union {
    __I uint32_t STET; /*!< Shadow TX Empty Trigger                                               */

    struct {
      __I uint32_t STET : 2; /*!< Shadow TX Empty Trigger                                               */
    } STET_b;                /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t HTX; /*!< Halt Transmit                                                         */

    struct {
      __IO uint32_t HALT_TX : 1; /*!< This register is use to halt transmissions for testing                */
    } HTX_b;                     /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t DMASA; /*!< DMA Software Acknowledge                                              */

    struct {
      __O uint32_t DMA_SOFTWARE_ACK : 1; /*!< This register is use to perform a DMA software acknowledge if
                                                         a transfer needs to be terminated due to an error condition
			 */
    } DMASA_b;                           /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t TCR; /*!< Transceiver Control Register                                          */

    struct {
      __IO uint32_t RS485_EN : 1;  /*!< RS485 Transfer Enable                                                 */
      __IO uint32_t RE_POL : 1;    /*!< Receiver Enable Polarity                                              */
      __IO uint32_t DE_POL : 1;    /*!< Driver Enable Polarity                                                */
      __IO uint32_t XFER_MODE : 2; /*!< Transfer Mode                                                         */
    } TCR_b;                       /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t DE_EN; /*!< Driver Output Enable Register                                         */

    struct {
      __IO uint32_t DE_EN : 1; /*!< DE Enable control                                                     */
    } DE_EN_b;                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t RE_EN; /*!< Receiver Output Enable Register                                       */

    struct {
      __IO uint32_t RE_EN : 1; /*!< RE Enable control                                                     */
    } RE_EN_b;                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t DET; /*!< Driver Output Enable Timing Register                                  */

    struct {
      __IO uint32_t DE_ASSERT_TIME : 8; /*!< Driver enable assertion time.                                         */
      uint32_t : 8;
      __IO uint32_t DE_DE_ASSERT_TIME : 8; /*!< Driver enable de-assertion time.                                      */
    } DET_b;                               /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t TAT; /*!< TurnAround Timing Register                                            */

    struct {
      __IO uint32_t DE_RE : 16; /*!< Driver Enable to Receiver Enable TurnAround time.                     */
      __IO uint32_t RE_DE : 16; /*!< Receiver Enable to Driver Enable TurnAround time.                     */
    } TAT_b;                    /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t DLF; /*!< Divisor Latch Fraction Register                                       */

    struct {
      __IO uint32_t DLF : 6; /*!< Fractional part of divisor.                                           */
    } DLF_b;                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t RAR; /*!< Receive Address Register                                              */

    struct {
      __IO uint32_t RAR : 8; /*!< This is an address matching register during receive mode              */
    } RAR_b;                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t TAR; /*!< Transmit Address Register                                             */

    struct {
      __IO uint32_t TAR : 8; /*!< This is an address matching register during transmit mode             */
    } TAR_b;                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t LCR_EXT; /*!< Line Extended Control Register                                        */

    struct {
      __IO uint32_t DLS_E : 1;         /*!< Extension for DLS                                                     */
      __IO uint32_t ADDR_MATCH : 1;    /*!< Address Match Mode                                                    */
      __IO uint32_t SEND_ADDR : 1;     /*!< Send address control bit.                                             */
      __IO uint32_t TRANSMIT_MODE : 1; /*!< Transmit mode control bit                                             */
    } LCR_EXT_b;                       /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED3[9];

  union {
    __I uint32_t CPR; /*!< Component Parameter Register                                          */

    struct {
      __I uint32_t APB_DATA_WIDTH : 2; /*!< none                                                                  */
      uint32_t : 2;
      __I uint32_t AFCE_MODE : 1;       /*!< none                                                                  */
      __I uint32_t THRE_MODE : 1;       /*!< none                                                                  */
      __I uint32_t SIR_MODE : 1;        /*!< none                                                                  */
      __I uint32_t SIR_LP_MODE : 1;     /*!< none                                                                  */
      __I uint32_t ADDITIONAL_FEAT : 1; /*!< none                                                                  */
      __I uint32_t FIFO_ACCESS : 1;     /*!< none                                                                  */
      __I uint32_t FIFO_STAT : 1;       /*!< none                                                                  */
      __I uint32_t SHADOW : 1;          /*!< none                                                                  */
      __I uint32_t
        UART_ADD_ENCODED_PARAMS : 1; /*!< none                                                                  */
      __I uint32_t DMA_EXTRA : 1;    /*!< none                                                                  */
      uint32_t : 2;
      __I uint32_t FIFO_MODE : 8; /*!< none                                                                  */
    } CPR_b;                      /*!< BitSize                                                               */
  };

  union {
    __I uint32_t UCV; /*!< UART Component Version                                                */

    struct {
      __I uint32_t UART_COMP_VER : 32; /*!< ASCII value for each number in the version, followed by *             */
    } UCV_b;                           /*!< BitSize                                                               */
  };

  union {
    __I uint32_t CTR; /*!< Component Type Register                                               */

    struct {
      __I uint32_t UART_COMP_VER : 32; /*!< This register contains the peripherals identification code.           */
    } CTR_b;                           /*!< BitSize                                                               */
  };
} USART0_Type;

/* =========================================================================================================================== */
/* ================                                           GSPI0                                           ================ */
/* =========================================================================================================================== */

/**
  * @brief GSPI, or Generic SPI, is a module which has been derived from QSPI. GSPI can act only as a master (GSPI0)
  */

typedef struct { /*!< (@ 0x45030000) GSPI0 Structure                                            */

  union {
    __IOM uint32_t GSPI_CLK_CONFIG; /*!< (@ 0x00000000) GSPI Clock Configuration Register                          */

    struct {
      __IOM uint32_t GSPI_CLK_SYNC : 1; /*!< (@ 0x00000000) If the clock frequency to FLASH (spi_clk) and
                                                                    SOC clk is same.                                           */
      __IOM uint32_t GSPI_CLK_EN : 1; /*!< (@ 0x00000001) GSPI clock enable                                          */
      __IOM uint32_t RESERVED1 : 30;  /*!< (@ 0x00000002) reserved for future use                                    */
    } GSPI_CLK_CONFIG_b;
  };

  union {
    __IOM uint32_t GSPI_BUS_MODE; /*!< (@ 0x00000004) GSPI Bus Mode Register                                     */

    struct {
      __IOM uint32_t GSPI_DATA_SAMPLE_EDGE : 1; /*!< (@ 0x00000000) Samples MISO data on clock edges. This should
                                                                    be ZERO for mode3 clock                                    */
      __IOM uint32_t
        GSPI_CLK_MODE_CSN0 : 1; /*!< (@ 0x00000001) NONE                                                       */
      __IOM uint32_t
        GSPI_CLK_MODE_CSN1 : 1; /*!< (@ 0x00000002) NONE                                                       */
      __IOM uint32_t
        GSPI_CLK_MODE_CSN2 : 1; /*!< (@ 0x00000003) NONE                                                       */
      __IOM uint32_t
        GSPI_CLK_MODE_CSN3 : 1; /*!< (@ 0x00000004) NONE                                                       */
      __IOM uint32_t
        GSPI_GPIO_MODE_ENABLES : 6; /*!< (@ 0x00000005) These bits are used to map GSPI on GPIO pins               */
      __IOM uint32_t SPI_HIGH_PERFORMANCE_EN : 1; /*!< (@ 0x0000000B) High performance features are enabled when this
                                                                    bit is set to one                                          */
      __IOM uint32_t RESERVED1 : 20; /*!< (@ 0x0000000C) reserved for future use                                    */
    } GSPI_BUS_MODE_b;
  };
  __IM uint32_t RESERVED[2];

  union {
    __IOM uint32_t GSPI_CONFIG1; /*!< (@ 0x00000010) GSPI Configuration 1 Register                              */

    struct {
      __IOM uint32_t
        GSPI_MANUAL_CSN : 1; /*!< (@ 0x00000000) SPI CS in manual mode                                      */
      __IOM uint32_t
        GSPI_MANUAL_WR : 1; /*!< (@ 0x00000001) Write enable for manual mode when CS is low.               */
      __IOM uint32_t
        GSPI_MANUAL_RD : 1; /*!< (@ 0x00000002) Read enable for manual mode when CS is low                 */
      __IOM uint32_t
        GSPI_MANUAL_RD_CNT : 10; /*!< (@ 0x00000003) Indicates total number of bytes to be read                 */
      __IOM uint32_t GSPI_MANUAL_CSN_SELECT : 2; /*!< (@ 0x0000000D) Indicates which CSn is valid. Can be programmable
                                                                    in manual mode                                             */
      __IOM uint32_t
        SPI_FULL_DUPLEX_EN : 1;      /*!< (@ 0x0000000F) Full duplex mode enable                                    */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved for future use                                    */
    } GSPI_CONFIG1_b;
  };

  union {
    __IOM uint32_t GSPI_CONFIG2; /*!< (@ 0x00000014) GSPI Manual Configuration 2 Register                       */

    struct {
      __IOM uint32_t GSPI_WR_DATA_SWAP_MNL_CSN0 : 1; /*!< (@ 0x00000000) Swap the write data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t GSPI_WR_DATA_SWAP_MNL_CSN1 : 1; /*!< (@ 0x00000001) Swap the write data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t GSPI_WR_DATA_SWAP_MNL_CSN2 : 1; /*!< (@ 0x00000002) Swap the write data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t GSPI_WR_DATA_SWAP_MNL_CSN3 : 1; /*!< (@ 0x00000003) Swap the write data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t GSPI_RD_DATA_SWAP_MNL_CSN0 : 1; /*!< (@ 0x00000004) Swap the read data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t GSPI_RD_DATA_SWAP_MNL_CSN1 : 1; /*!< (@ 0x00000005) Swap the read data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t GSPI_RD_DATA_SWAP_MNL_CSN2 : 1; /*!< (@ 0x00000006) Swap the read data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t GSPI_RD_DATA_SWAP_MNL_CSN3 : 1; /*!< (@ 0x00000007) Swap the read data inside the GSPI controller
                                                                    it-self.                                                   */
      __IOM uint32_t
        GSPI_MANUAL_SIZE_FRM_REG : 1; /*!< (@ 0x00000008) Manual reads and manual writes                           */
      __IOM uint32_t RESERVED1 : 1;   /*!< (@ 0x00000009) reserved for future use                                    */
      __IOM uint32_t
        TAKE_GSPI_MANUAL_WR_SIZE_FRM_REG : 1; /*!< (@ 0x0000000A) NONE                                             */
      __IOM uint32_t
        MANUAL_GSPI_MODE : 1;        /*!< (@ 0x0000000B) Internally the priority is given to manual mode            */
      __IOM uint32_t RESERVED2 : 20; /*!< (@ 0x0000000C) reserved for future use                                    */
    } GSPI_CONFIG2_b;
  };

  union {
    __IOM uint32_t GSPI_WRITE_DATA2; /*!< (@ 0x00000018) GSPI Write Data 2 Register                                 */

    struct {
      __IOM uint32_t
        GSPI_MANUAL_WRITE_DATA2 : 4; /*!< (@ 0x00000000) Number of bits to be written in write mode                */
      __IOM uint32_t RESERVED1 : 3;  /*!< (@ 0x00000004) reserved for future use                                    */
      __IOM uint32_t
        USE_PREV_LENGTH : 1;         /*!< (@ 0x00000007) Use previous length                                        */
      __IOM uint32_t RESERVED2 : 24; /*!< (@ 0x00000008) reserved for future use                                    */
    } GSPI_WRITE_DATA2_b;
  };

  union {
    __IOM uint32_t GSPI_FIFO_THRLD; /*!< (@ 0x0000001C) GSPI FIFO Threshold Register                               */

    struct {
      __IOM uint32_t
        FIFO_AEMPTY_THRLD : 4; /*!< (@ 0x00000000) FIFO almost empty threshold                                */
      __IOM uint32_t
        FIFO_AFULL_THRLD : 4;         /*!< (@ 0x00000004) FIFO almost full threshold                                 */
      __IOM uint32_t WFIFO_RESET : 1; /*!< (@ 0x00000008) Write FIFO reset                                           */
      __IOM uint32_t RFIFO_RESET : 1; /*!< (@ 0x00000009) read FIFO reset                                            */
      __IOM uint32_t RESERVED1 : 22;  /*!< (@ 0x0000000A) reserved for future use                                    */
    } GSPI_FIFO_THRLD_b;
  };

  union {
    __IM uint32_t GSPI_STATUS; /*!< (@ 0x00000020) GSPI Status Register                                       */

    struct {
      __IM uint32_t GSPI_BUSY : 1; /*!< (@ 0x00000000) State of Manual mode                                       */
      __IM uint32_t
        FIFO_FULL_WFIFO_S : 1; /*!< (@ 0x00000001) Full status indication for Wfifo in manual mode            */
      __IM uint32_t FIFO_AFULL_WFIFO_S : 1; /*!< (@ 0x00000002) Almost full status indication for Wfifo in manual
                                                                    mode                                                       */
      __IM uint32_t
        FIFO_EMPTY_WFIFO : 1;      /*!< (@ 0x00000003) Empty status indication for Wfifo in manual mode           */
      __IM uint32_t RESERVED1 : 1; /*!< (@ 0x00000004) reserved for future use                                    */
      __IM uint32_t
        FIFO_FULL_RFIFO : 1;       /*!< (@ 0x00000005) Full status indication for Rfifo in manual mode            */
      __IM uint32_t RESERVED2 : 1; /*!< (@ 0x00000006) reserved for future use                                    */
      __IM uint32_t
        FIFO_EMPTY_RFIFO_S : 1; /*!< (@ 0x00000007) Empty status indication for Rfifo in manual mode           */
      __IM uint32_t FIFO_AEMPTY_RFIFO_S : 1; /*!< (@ 0x00000008) Aempty status indication for Rfifo in manual
                                                                    mode                                                       */
      __IM uint32_t
        GSPI_MANUAL_RD_CNT : 1; /*!< (@ 0x00000009) This is a result of 10 bits ORing counter                  */
      __IM uint32_t
        GSPI_MANUAL_CSN : 1;        /*!< (@ 0x0000000A) Provide the status of chip select signal                   */
      __IM uint32_t RESERVED3 : 21; /*!< (@ 0x0000000B) reserved for future use                                    */
    } GSPI_STATUS_b;
  };

  union {
    __IOM uint32_t GSPI_INTR_MASK; /*!< (@ 0x00000024) GSPI Interrupt Mask Register                               */

    struct {
      __IOM uint32_t
        GSPI_INTR_MASK : 1; /*!< (@ 0x00000000) GSPI Interrupt mask bit                                    */
      __IOM uint32_t
        FIFO_AEMPTY_RFIFO_MASK : 1; /*!< (@ 0x00000001) NONE                                                       */
      __IOM uint32_t
        FIFO_AFULL_RFIFO_MASK : 1; /*!< (@ 0x00000002) NONE                                                       */
      __IOM uint32_t
        FIFO_AEMPTY_WFIFO_MASK : 1; /*!< (@ 0x00000003) NONE                                                       */
      __IOM uint32_t
        FIFO_AFULL_WFIFO_MASK : 1; /*!< (@ 0x00000004) NONE                                                       */
      __IOM uint32_t
        FIFO_FULL_WFIFO_MASK : 1; /*!< (@ 0x00000005) NONE                                                       */
      __IOM uint32_t
        FIFO_EMPTY_RFIFO_MASK : 1;   /*!< (@ 0x00000006) NONE                                                       */
      __IOM uint32_t RESERVED1 : 25; /*!< (@ 0x00000007) reserved for future use                                    */
    } GSPI_INTR_MASK_b;
  };

  union {
    __IOM uint32_t GSPI_INTR_UNMASK; /*!< (@ 0x00000028) GSPI Interrupt Unmask Register                             */

    struct {
      __IOM uint32_t
        GSPI_INTR_UNMASK : 1; /*!< (@ 0x00000000) GSPI Interrupt unmask bit                                  */
      __IOM uint32_t
        FIFO_AEMPTY_RFIFO_UNMASK : 1; /*!< (@ 0x00000001) NONE                                                     */
      __IOM uint32_t
        FIFO_AFULL_RFIFO_UNMASK : 1; /*!< (@ 0x00000002) NONE                                                      */
      __IOM uint32_t
        FIFO_AEMPTY_WFIFO_UNMASK : 1; /*!< (@ 0x00000003) NONE                                                     */
      __IOM uint32_t
        FIFO_AFULL_WFIFO_UNMASK : 1; /*!< (@ 0x00000004) NONE                                                      */
      __IOM uint32_t
        FIFO_FULL_WFIFO_UNMASK : 1; /*!< (@ 0x00000005) NONE                                                       */
      __IOM uint32_t
        FIFO_EMPTY_RFIFO_UNMASK : 1; /*!< (@ 0x00000006) NONE                                                      */
      __IOM uint32_t RESERVED1 : 25; /*!< (@ 0x00000007) reserved for future use                                    */
    } GSPI_INTR_UNMASK_b;
  };

  union {
    __IM uint32_t GSPI_INTR_STS; /*!< (@ 0x0000002C) GSPI Interrupt Status Register                             */

    struct {
      __IM uint32_t GSPI_INTR_LVL : 1; /*!< (@ 0x00000000) GSPI Interrupt status bit                                  */
      __IM uint32_t
        FIFO_AEMPTY_RFIFO_LVL : 1; /*!< (@ 0x00000001) NONE                                                       */
      __IM uint32_t RESERVED1 : 2; /*!< (@ 0x00000002) reserved for future use                                    */
      __IM uint32_t
        FIFO_AFULL_WFIFO_LVL : 1; /*!< (@ 0x00000004) NONE                                                       */
      __IM uint32_t
        FIFO_FULL_WFIFO_LVL : 1; /*!< (@ 0x00000005) NONE                                                       */
      __IM uint32_t
        FIFO_EMPTY_RFIFO_LVL : 1;   /*!< (@ 0x00000006) NONE                                                       */
      __IM uint32_t RESERVED2 : 25; /*!< (@ 0x00000007) reserved for future use                                    */
    } GSPI_INTR_STS_b;
  };

  union {
    __OM uint32_t GSPI_INTR_ACK; /*!< (@ 0x00000030) GSPI Interrupt Acknowledge Register                        */

    struct {
      __OM uint32_t GSPI_INTR_ACK : 1; /*!< (@ 0x00000000) GSPI Interrupt status bit                                  */
      __OM uint32_t
        FIFO_AEMPTY_RFIFO_ACK : 1; /*!< (@ 0x00000001) NONE                                                       */
      __OM uint32_t RESERVED1 : 2; /*!< (@ 0x00000002) reserved for future use                                    */
      __OM uint32_t
        FIFO_AFULL_WFIFO_ACK : 1; /*!< (@ 0x00000004) NONE                                                       */
      __OM uint32_t
        FIFO_FULL_WFIFO_ACK : 1; /*!< (@ 0x00000005) NONE                                                       */
      __OM uint32_t
        FIFO_EMPTY_RFIFO_ACK : 1;   /*!< (@ 0x00000006) NONE                                                       */
      __OM uint32_t RESERVED2 : 25; /*!< (@ 0x00000007) reserved1                                                  */
    } GSPI_INTR_ACK_b;
  };

  union {
    __IM uint32_t GSPI_STS_MC; /*!< (@ 0x00000034) GSPI State Machine Monitor Register                        */

    struct {
      __IM uint32_t
        BUS_CTRL_PSTATE : 3;         /*!< (@ 0x00000000) Provides SPI bus controller present state                  */
      __IM uint32_t SPI_RD_CNT : 13; /*!< (@ 0x00000003) number of pending bytes to be read by device               */
      __IM uint32_t RESERVED1 : 16;  /*!< (@ 0x00000010) reserved1                                                  */
    } GSPI_STS_MC_b;
  };

  union {
    __IM uint32_t GSPI_CLK_DIV; /*!< (@ 0x00000038) GSPI Clock Division Factor Register                        */

    struct {
      __IOM uint32_t GSPI_CLK_DIV_FACTOR : 8; /*!< (@ 0x00000000) Provides GSPI clock division factor to the clock
                                                                    divider, which takes SOC clock as input
                                                                    clock and generates required clock according
                                                                    to division factor                                         */
      __IM uint32_t RESERVED1 : 24; /*!< (@ 0x00000008) reserved1                                                  */
    } GSPI_CLK_DIV_b;
  };

  union {
    __IOM uint32_t GSPI_CONFIG3; /*!< (@ 0x0000003C) GSPI Configuration 3 Register                              */

    struct {
      __IOM uint32_t SPI_MANUAL_RD_LNTH_TO_BC : 15; /*!< (@ 0x00000000) Bits are used to indicate the total number of
                                                                    bytes to read from flash during read operation             */
      __IOM uint32_t RESERVED1 : 17; /*!< (@ 0x0000000F) reserved1                                                  */
    } GSPI_CONFIG3_b;
  };
  __IM uint32_t RESERVED1[16];

  union {
    union {
      __OM uint32_t
        GSPI_WRITE_FIFO[16]; /*!< (@ 0x00000080) GSPI fifo                                                  */

      struct {
        __OM uint32_t WRITE_FIFO : 32; /*!< (@ 0x00000000) FIFO data is write to this address space                   */
      } GSPI_WRITE_FIFO_b[16];
    };

    union {
      __IM uint32_t
        GSPI_READ_FIFO[16]; /*!< (@ 0x00000080) GSPI READ FIFO                                             */

      struct {
        __IM uint32_t READ_FIFO : 32; /*!< (@ 0x00000000) FIFO data is read from this address space                  */
      } GSPI_READ_FIFO_b[16];
    };
  };
} GSPI0_Type; /*!< Size = 192 (0xc0)                                                         */

/* ================================================================================ */
/* ================                      SSI0                      ================ */
/* ================================================================================ */

/**
  * @brief Synchronous Serial Interface(SSI) (SSI0)
  */

typedef struct { /*!< (@ 0x44020000) SSI0 Structure                                             */

  union {
    __IOM uint32_t CTRLR0; /*!< (@ 0x00000000) Control Register 0                                         */

    struct {
      __IOM uint32_t DFS : 4;       /*!< [3..0] Select the data frame length (4-bit to 16-bit serial
                                                     data transfers)                                                           */
      __IOM uint32_t FRF : 2;       /*!< [5..4] Frame Format, Selects which serial protocol transfers
                                                     the data                                                                  */
      __IOM uint32_t SCPH : 1;      /*!< [6..6] Serial Clock Phase. Valid when the frame format (FRF)
                                                     is set to Motorola SPI                                                    */
      __IOM uint32_t SCPOL : 1;     /*!< [7..7] Serial Clock Polarity. Valid when the frame format (FRF)
                                                     is set to Motorola SPI                                                    */
      __IOM uint32_t TMOD : 2;      /*!< [9..8] Selects the mode of transfer for serial communication              */
      __IOM uint32_t SLV_OE : 1;    /*!< [10..10] DW_apb_ssi is configured as a serial-slave device                */
      __IOM uint32_t SRL : 1;       /*!< [11..11] Shift Register Loop Used for testing purposes only               */
      __IOM uint32_t CFS : 4;       /*!< [15..12] Control Frame Size Selects the length of the control
                                                     word for the Micro wire frame format                                      */
      __IOM uint32_t DFS_32 : 5;    /*!< [20..16] Selects the data frame length                                    */
      __IOM uint32_t SPI_FRF : 2;   /*!< [22..21] Selects data frame format for transmitting or receiving
                                                     data                                                                      */
      __IOM uint32_t RESERVED1 : 9; /*!< [31..23] Reserved for future use                                          */
    } CTRLR0_b;
  };

  union {
    __IOM uint32_t CTRLR1; /*!< (@ 0x00000004) Control Register 1                                         */

    struct {
      __IOM uint32_t NDF : 16;       /*!< [15..0] Number of Data Frames.When TMOD = 10 or TMOD = 11, this
                                                     register field sets the number of data frames to be continuously
                                                     received by the ssi_master                                                */
      __IOM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use.                                         */
    } CTRLR1_b;
  };

  union {
    __IOM uint32_t SSIENR; /*!< (@ 0x00000008) SSI Enable Register                                        */

    struct {
      __IOM uint32_t SSI_EN : 1;     /*!< [0..0] Enables and disables all ssi operations                            */
      __IOM uint32_t RESERVED1 : 31; /*!< [31..1] Reserved for future use                                           */
    } SSIENR_b;
  };

  union {
    __IOM uint32_t MWCR; /*!< (@ 0x0000000C) Micro wire Control Register                                */

    struct {
      __IOM uint32_t MWMOD : 1;      /*!< [0..0] The Micro wire transfer is sequential or non-sequential            */
      __IOM uint32_t MDD : 1;        /*!< [1..1] The direction of the data word when the Micro wire serial
                                                     protocol is used                                                          */
      __IOM uint32_t MHS : 1;        /*!< [2..2] Microwire Handshaking. Used to enable and disable the
                                                     busy/ready handshaking  interface for the Microwire protocol              */
      __IOM uint32_t RESERVED1 : 29; /*!< [31..3] Reserved for future use                                           */
    } MWCR_b;
  };

  union {
    __IOM uint32_t SER; /*!< (@ 0x00000010) SLAVE ENABLE REGISTER                                      */

    struct {
      __IOM uint32_t SER : 4;        /*!< [3..0] Each bit in this register corresponds to a slave select
                                                     line (ss_x_n) from the SSI master.                                        */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved for future use                                           */
    } SER_b;
  };

  union {
    __IOM uint32_t BAUDR; /*!< (@ 0x00000014) Baud Rate Select Register                                  */

    struct {
      __IOM uint32_t SCKDV : 16;     /*!< [15..0] SSI Clock Divider.The LSB for this field is always set
                                                     to 0 and is unaffected by a write operation, which ensures
                                                     an even value is held in this register                                    */
      __IOM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } BAUDR_b;
  };

  union {
    __IOM uint32_t TXFTLR; /*!< (@ 0x00000018) Transmit FIFO Threshold Level Register                     */

    struct {
      __IOM uint32_t TFT : 4;        /*!< [3..0] Controls the level of entries (or below) at which the
                                                     transmit  FIFO controller triggers an interrupt                           */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved for future use                                           */
    } TXFTLR_b;
  };

  union {
    __IOM uint32_t RXFTLR; /*!< (@ 0x0000001C) Receive FIFO Threshold Level                               */

    struct {
      __IOM uint32_t RFT : 4;        /*!< [3..0] Controls the level of entries (or above) at which the
                                                     receive FIFO controller triggers an interrupt                             */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved for future use                                           */
    } RXFTLR_b;
  };

  union {
    __IM uint32_t TXFLR; /*!< (@ 0x00000020) Transmit FIFO Level Register                               */

    struct {
      __IM uint32_t TXTFL : 5;      /*!< [4..0] Contains the number of valid data entries in the transmit
                                                     FIFO                                                                      */
      __IM uint32_t RESERVED1 : 27; /*!< [31..5] Reserved for future use                                           */
    } TXFLR_b;
  };

  union {
    __IM uint32_t RXFLR; /*!< (@ 0x00000024) Receive FIFO Level Register                                */

    struct {
      __IM uint32_t RXTFL : 5;      /*!< [4..0] Contains the number of valid data entries in the receive
                                                     FIFO                                                                      */
      __IM uint32_t RESERVED1 : 27; /*!< [31..5] Reserved for future use                                           */
    } RXFLR_b;
  };

  union {
    __IM uint32_t SR; /*!< (@ 0x00000028) Status Register                                            */

    struct {
      __IM uint32_t BUSY : 1;       /*!< [0..0] indicates that a serial transfer is in progress                    */
      __IM uint32_t TFNF : 1;       /*!< [1..1] Set when the transmit FIFO contains one or more empty
                                                     locations and is cleared when the FIFO is full                            */
      __IM uint32_t TFE : 1;        /*!< [2..2] When the transmit FIFO is completely empty this bit is
                                                     set                                                                       */
      __IM uint32_t RFNE : 1;       /*!< [3..3] Set when the receive FIFO contains one or more entries
                                                     and is  cleared when the receive FIFO is empty                            */
      __IM uint32_t RFF : 1;        /*!< [4..4] When the receive FIFO is completely full this bit is
                                                     set                                                                       */
      __IM uint32_t TXE : 1;        /*!< [5..5] This bit is cleared when read                                      */
      __IM uint32_t DCOL : 1;       /*!< [6..6] This bit is set if the ss_in_n input is asserted by another
                                                     master, while the ssi master is in the middle of the transfer             */
      __IM uint32_t RESERVED1 : 25; /*!< [31..7] Reserved for future use                                           */
    } SR_b;
  };

  union {
    __IOM uint32_t IMR; /*!< (@ 0x0000002C) Interrupt Mask Register                                    */

    struct {
      __IOM uint32_t TXEIM : 1;     /*!< [0..0] Transmit FIFO Empty Interrupt Mask                                 */
      __IOM uint32_t TXOIM : 1;     /*!< [1..1] Transmit FIFO Overflow Interrupt Mask                              */
      __IOM uint32_t RXUIM : 1;     /*!< [2..2] Receive FIFO Underflow Interrupt Mask                              */
      __IOM uint32_t RXOIM : 1;     /*!< [3..3] Receive FIFO Overflow Interrupt Mask                               */
      __IOM uint32_t RXFIM : 1;     /*!< [4..4] Receive FIFO Full Interrupt Mask                                   */
      __IOM uint32_t MSTIM : 1;     /*!< [5..5] Multi-Master Contention Interrupt Mask                             */
      __IM uint32_t RESERVED1 : 26; /*!< [31..6] Reserved for future use                                           */
    } IMR_b;
  };

  union {
    __IM uint32_t ISR; /*!< (@ 0x00000030) Interrupt Status Register                                  */

    struct {
      __IM uint32_t TXEIS : 1;      /*!< [0..0] Transmit FIFO Empty Interrupt Status                               */
      __IM uint32_t TXOIS : 1;      /*!< [1..1] Transmit FIFO Overflow Interrupt Status                            */
      __IM uint32_t RXUIS : 1;      /*!< [2..2] Receive FIFO Underflow Interrupt Status                            */
      __IM uint32_t RXOIS : 1;      /*!< [3..3] Receive FIFO Overflow Interrupt Status                             */
      __IM uint32_t RXFIS : 1;      /*!< [4..4] Receive FIFO Full Interrupt Status                                 */
      __IM uint32_t MSTIS : 1;      /*!< [5..5] Multi-Master Contention Interrupt Status                           */
      __IM uint32_t RESERVED1 : 26; /*!< [31..6] Reserved for future use                                           */
    } ISR_b;
  };

  union {
    __IM uint32_t RISR; /*!< (@ 0x00000034) Raw Interrupt Status Register                              */

    struct {
      __IM uint32_t TXEIR : 1;      /*!< [0..0] Transmit FIFO Empty Raw Interrupt Status                           */
      __IM uint32_t TXOIR : 1;      /*!< [1..1] Transmit FIFO Overflow Raw Interrupt Status                        */
      __IM uint32_t RXUIR : 1;      /*!< [2..2] Receive FIFO Underflow Raw Interrupt Status                        */
      __IM uint32_t RXOIR : 1;      /*!< [3..3] Receive FIFO Overflow Raw Interrupt Status                         */
      __IM uint32_t RXFIR : 1;      /*!< [4..4] Receive FIFO Full Raw Interrupt Status                             */
      __IM uint32_t MSTIR : 1;      /*!< [5..5] Multi-Master Contention Raw Interrupt Status                       */
      __IM uint32_t RESERVED1 : 26; /*!< [31..6] Reserved for future use                                           */
    } RISR_b;
  };

  union {
    __IM uint32_t TXOICR; /*!< (@ 0x00000038) Transmit FIFO Overflow Interrupt Clear Register            */

    struct {
      __IM uint32_t TXOICR : 1;     /*!< [0..0] Clear Transmit FIFO Overflow Interrupt This register
                                                     reflects the status of the interrupt                                      */
      __IM uint32_t RESERVED1 : 31; /*!< [31..1] Reserved for future use                                           */
    } TXOICR_b;
  };

  union {
    __IM uint32_t RXOICR; /*!< (@ 0x0000003C) Receive FIFO Overflow Interrupt Clear Register             */

    struct {
      __IM uint32_t RXOICR : 1;     /*!< [0..0] This register reflects the status of the interrupt A
                                                     read from this  register clears the ssi_rxo_intr interrupt                */
      __IM uint32_t RESERVED1 : 31; /*!< [31..1] Reserved for future use                                           */
    } RXOICR_b;
  };

  union {
    __IM uint32_t RXUICR; /*!< (@ 0x00000040) Receive FIFO Underflow Interrupt Clear Register            */

    struct {
      __IM uint32_t RXUICR : 1;     /*!< [0..0] This register reflects the status of the interrupt A
                                                     read from this  register clears the ssi_rxu_intr interrupt                */
      __IM uint32_t RESERVED1 : 31; /*!< [31..1] Reserved for future use                                           */
    } RXUICR_b;
  };

  union {
    __IM uint32_t MSTICR; /*!< (@ 0x00000044) Multi-Master Interrupt Clear Register                      */

    struct {
      __IM uint32_t MSTICR : 1;     /*!< [0..0] This register reflects the status of the interrupt A
                                                     read from this  register clears the ssi_mst_intr interrupt                */
      __IM uint32_t RESERVED1 : 31; /*!< [31..1] Reserved for future use                                           */
    } MSTICR_b;
  };

  union {
    __IM uint32_t ICR; /*!< (@ 0x00000048) Interrupt Clear Register                                   */

    struct {
      __IM uint32_t ICR : 1;        /*!< [0..0] This register is set if any of the interrupts below are
                                                     active A read clears the ssi_txo_intr, ssi_rxu_intr, ssi_rxo_intr,
                                                     and the ssi_mst_intr interrupts                                           */
      __IM uint32_t RESERVED1 : 31; /*!< [31..1] Reserved for future use                                           */
    } ICR_b;
  };

  union {
    __IOM uint32_t DMACR; /*!< (@ 0x0000004C) DMA Control Register                                       */

    struct {
      __IOM uint32_t RDMAE : 1;     /*!< [0..0] This bit enables/disables the receive FIFO DMA channel             */
      __IOM uint32_t TDMAE : 1;     /*!< [1..1] This bit enables/disables the transmit FIFO DMA channel            */
      __IM uint32_t RESERVED1 : 30; /*!< [31..2] Reserved for future use                                           */
    } DMACR_b;
  };

  union {
    __IOM uint32_t DMATDLR; /*!< (@ 0x00000050) DMA Transmit Data Level                                    */

    struct {
      __IOM uint32_t DMATDL : 4;    /*!< [3..0] This bit field controls the level at which a DMA request
                                                     is made by the transmit logic                                             */
      __IM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved for future use                                           */
    } DMATDLR_b;
  };

  union {
    __IOM uint32_t DMARDLR; /*!< (@ 0x00000054) DMA Receive Data Level Register                            */

    struct {
      __IOM uint32_t DMARDL : 4;     /*!< [3..0] This bit field controls the level at which a DMA request
                                                     is made by the receive logic                                              */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved for future use                                           */
    } DMARDLR_b;
  };

  union {
    __IM uint32_t IDR; /*!< (@ 0x00000058) Identification Register                                    */

    struct {
      __IM uint32_t IDCODE : 32; /*!< [31..0] This register contains the peripherals identification
                                                     code                                                                      */
    } IDR_b;
  };

  union {
    __IM uint32_t SSI_COMP_VERSION; /*!< (@ 0x0000005C) coreKit version ID register                                */

    struct {
      __IM uint32_t SSI_COMP_VERSION : 32; /*!< [31..0] Contains the hex representation of the Synopsys component
                                                     version                                                                   */
    } SSI_COMP_VERSION_b;
  };

  union {
    __IOM uint32_t DR; /*!< (@ 0x00000060) Data Register                                              */

    struct {
      __IOM uint32_t DR : 32; /*!< [31..0] When writing to this register must right-justify the
                                                     data                                                                      */
    } DR_b;
  };
  __IM uint32_t RESERVED[35];

  union {
    __IOM uint32_t RX_SAMPLE_DLY; /*!< (@ 0x000000F0) Rx Sample Delay Register                                   */

    struct {
      __IOM uint32_t RSD : 8;        /*!< [7..0] Receive Data (rxd) Sample Delay. This register is used
                                                     to delay the sample of the rxd input signal.                              */
      __IOM uint32_t RESERVED1 : 24; /*!< [31..8] Reserved for future use                                           */
    } RX_SAMPLE_DLY_b;
  };

  union {
    __IOM uint32_t SPI_CTRLR0; /*!< (@ 0x000000F4) SPI control Register                                       */

    struct {
      __IOM uint32_t TRANS_TYPE : 2;  /*!< [1..0] Address and instruction transfer format                            */
      __IOM uint32_t ADDR_L : 4;      /*!< [5..2] This bit defines length of address to be transmitted,
                                                     The transfer begins only after these many bits are programmed
                                                     into the FIFO                                                             */
      __IM uint32_t RESERVED1 : 2;    /*!< [7..6] Reserved for future use                                            */
      __IOM uint32_t INST_L : 2;      /*!< [9..8] DUAL/QUAD length in bits                                           */
      __IM uint32_t RESERVED2 : 1;    /*!< [10..10] Reserved for future use                                          */
      __IOM uint32_t WAIT_CYCLES : 4; /*!< [14..11] This bit defines the wait cycles in dual/quad mode
                                                     between control frames transmit and data reception, Specified
                                                     as number of SPI clock cycles                                             */
      __IM uint32_t RESERVED3 : 17;   /*!< [31..15] Reserved for future use                                          */
    } SPI_CTRLR0_b;
  };
} SSI0_Type; /*!< Size = 248 (0xf8)                                                         */

/* =========================================================================================================================== */
/* ================                                            SIO                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief SERIAL GENERAL PERPOSE INPUT/OUTPUT (SIO)
  */

typedef struct { /*!< (@ 0x47000000) SIO Structure                                              */

  union {
    __IOM uint32_t SIO_ENABLE_REG; /*!< (@ 0x00000000) ENABLE REGISTER                                            */

    struct {
      __IOM uint32_t
        SIO_OPERATION_ENABLE : 16;  /*!< [15..0] Contains the Enables for all SIO                                  */
      __IM uint32_t RESERVED3 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_ENABLE_REG_b;
  };

  union {
    __IOM uint32_t SIO_PAUSE_REG; /*!< (@ 0x00000004) PAUSE REGISTER                                             */

    struct {
      __IOM uint32_t
        SIO_POSITION_COUNTER_DISABLE : 16; /*!< [15..0] Contains sio position counter disable for all SIOs         */
      __IM uint32_t RESERVED3 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_PAUSE_REG_b;
  };

  union {
    __IM uint32_t SIO_GPIO_IN_REG; /*!< (@ 0x00000008) GPIO Input Register                                        */

    struct {
      __IM uint32_t IN_VALUE : 32; /*!< [31..0] GPIO input pin status                                             */
    } SIO_GPIO_IN_REG_b;
  };

  union {
    __IOM uint32_t SIO_GPIO_OUT_REG; /*!< (@ 0x0000000C) GPIO Output Register                                       */

    struct {
      __IOM uint32_t OUT_VALUE : 32; /*!< [31..0] Value to be loaded on GPIO out pins                               */
    } SIO_GPIO_OUT_REG_b;
  };

  union {
    __IOM uint32_t SIO_GPIO_OEN_REG; /*!< (@ 0x00000010) GPIO Output enable Register                                */

    struct {
      __IOM uint32_t OEN_VALUE : 32; /*!< [31..0] OEN for the GPIO pins                                             */
    } SIO_GPIO_OEN_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_GPIO_INTR_EN_SET_REG; /*!< (@ 0x00000014) GPIO Interrupt Enable Set Register                         */

    struct {
      __OM uint32_t
        INTR_ENABLE_SET : 16;       /*!< [15..0] gpio interrupt enable set register for all SIOs                   */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_GPIO_INTR_EN_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_GPIO_INTR_EN_CLEAR_REG; /*!< (@ 0x00000018) GPIO Interrupt Enable Clear Register                       */

    struct {
      __OM uint32_t
        INTR_ENABLE_CLEAR : 16;     /*!< [15..0] gpio interrupt enable Clear register for all SIOs                 */
      __OM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_GPIO_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_GPIO_INTR_MASK_SET_REG; /*!< (@ 0x0000001C) GPIO Interrupt Enable Clear Register                       */

    struct {
      __IOM uint32_t
        INTR_MASK_SET : 16;         /*!< [15..0] Common gpio interrupt mask set register for all SIOs              */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_GPIO_INTR_MASK_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_GPIO_INTR_MASK_CLEAR_REG; /*!< (@ 0x00000020) GPIO Interrupt Enable Clear Register                       */

    struct {
      __OM uint32_t
        INTR_MASK_CLEAR : 16;       /*!< [15..0] gpio interrupt mask clear register for all SIOs                   */
      __OM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_GPIO_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_GPIO_INTR_STATUS_REG; /*!< (@ 0x00000024) GPIO Interrupt Status Register                             */

    struct {
      __OM uint32_t
        INTR_MASK_SET : 16;         /*!< [15..0] Common gpio interrupt status register for all SIOs                */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_GPIO_INTR_STATUS_REG_b;
  };

  union {
    __IM uint32_t
      SIO_SHIFT_COUNTER[16]; /*!< (@ 0x00000028) Shift counter register                                     */

    struct {
      __IM uint32_t
        SHIFT_COUNTER : 14;         /*!< [13..0] shift counter current value                                       */
      __IM uint32_t RESERVED1 : 18; /*!< [31..14] Reserved for future use                                          */
    } SIO_SHIFT_COUNTER_b[16];
  };

  union {
    __IOM uint32_t SIO_BUFFER_REG[16]; /*!< (@ 0x00000068) Buffer Register                                            */

    struct {
      __IOM uint32_t DATA : 32; /*!< [31..0] Data to load into the shift register                              */
    } SIO_BUFFER_REG_b[16];
  };

  union {
    __IOM uint32_t
      SIO_SHIFT_COUNT_PRELOAD_REG[16]; /*!< (@ 0x000000A8) Shift counter Reload Register                           */

    struct {
      __IOM uint32_t
        RELOAD_VALUE : 14;             /*!< [13..0] division factor required to generate shift clock                  */
      __IM uint32_t RESERVED1 : 1;     /*!< [14..14] Reserved for future use                                          */
      __IOM uint32_t REVERSE_LOAD : 1; /*!< [15..15] When set, the data on APB is loaded to buffer is reverse
                                                     order                                                                     */
      __IM uint32_t RESERVED2 : 16;    /*!< [31..16] Reserved for future use                                          */
    } SIO_SHIFT_COUNT_PRELOAD_REG_b[16];
  };

  union {
    __IOM uint32_t
      SIO_DATA_POS_COUNT_REG[16]; /*!< (@ 0x000000E8) Data Position Counter Register                             */

    struct {
      __IOM uint32_t RELOAD_VALUE : 8; /*!< [7..0] No. of shifts to happen before reloading the shift register
                                                     with data/ pausing the operation                                          */
      __IOM uint32_t
        POSITION_COUNTER : 8;       /*!< [15..8] The position counter can be loaded via AHB                        */
      __IM uint32_t RESERVED3 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_DATA_POS_COUNT_REG_b[16];
  };

  union {
    __IOM uint32_t SIO_CONFIG_REG[16]; /*!< (@ 0x00000128) Configuration Register                                     */

    struct {
      __IOM uint32_t FULL_ENABLE : 1;  /*!< [0..0] When set, fifo full indication would be asserted when
                                                     internal buffer is full                                                   */
      __IOM uint32_t EMPTY_ENABLE : 1; /*!< [1..1] When set, fifo full indication would be asserted when
                                                     internal buffer is empty                                                  */
      __IOM uint32_t EDGE_SEL : 1;     /*!< [2..2] edge selection                                                     */
      __IOM uint32_t CLK_SEL : 1;      /*!< [3..3] clock selection                                                    */
      __IOM uint32_t
        IGNORE_FIRST_SHIFT_CONDITION : 1; /*!< [4..4] data shift condition                                         */
      __IOM uint32_t
        FLOW_CONTROL_ENABLED : 1; /*!< [5..5] flow control                                                       */
      __IOM uint32_t
        PATTERN_MATCH_ENABLE : 1; /*!< [6..6] pattern match                                                      */
      __IOM uint32_t
        QUALIFIER_MODE : 1; /*!< [7..7] qualifier mode                                                     */
      __IOM uint32_t
        QUALIFY_CLOCK : 1;             /*!< [8..8] qualify clock                                                      */
      __IOM uint32_t INVERT_CLOCK : 1; /*!< [9..9] invert clock                                                       */
      __IOM uint32_t
        PARALLEL_MODE : 2; /*!< [11..10] No. of bits to shift/capture at valid clk edge                   */
      __IOM uint32_t
        PIN_DETECTION_MODE : 2;       /*!< [13..12] Pin mode to be considered for gpio interrupt                     */
      __IOM uint32_t SET_CLK_OUT : 1; /*!< [14..14] When high sets the sio clock_out port. This is used
                                                     only when sio is not enabled                                              */
      __IOM uint32_t RESET_CLK_OUT : 1; /*!< [15..15] When high resets the sio clock_out port. This is used
                                                     only when sio is not enabled                                              */
      __IOM uint32_t
        LOAD_DATA_POS_CNTR_VIA_APB : 1; /*!< [16..16] When set, data position counter can be loaded via APB        */
      __IM uint32_t RESERVED1 : 15; /*!< [31..17] Reserved for future use                                          */
    } SIO_CONFIG_REG_b[16];
  };

  union {
    __IOM uint32_t
      PATTERN_MATCH_MASK_REG_SLICE_0; /*!< (@ 0x00000168) Pattern Match Mask Register 0                            */

    struct {
      __IOM uint32_t
        MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower 16 bits                                        */
    } PATTERN_MATCH_MASK_REG_SLICE_0_b;
  };

  union {
    __IOM uint32_t
      PATTERN_MATCH_MASK_REG_slice_1; /*!< (@ 0x0000016C) Pattern Match Mask Register Slice 1                      */

    struct {
      __IOM uint32_t
        MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower 16 bits                                        */
    } PATTERN_MATCH_MASK_REG_slice_1_b;
  };

  union {
    __IOM uint32_t
      PATTERN_MATCH_MASK_REG_SLICE_2; /*!< (@ 0x00000170) Pattern Match Mask Register Slice 2                      */

    struct {
      __IOM uint32_t
        MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower 16 bits                                        */
    } PATTERN_MATCH_MASK_REG_SLICE_2_b;
  };
  __IM uint32_t RESERVED[5];

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_MASK_REG_SLICE_8; /*!< (@ 0x00000188) Pattern Match Mask Register Slice 8                  */

    struct {
      __IOM uint32_t
        MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower 16 bits                                        */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_8_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_MASK_REG_SLICE_9; /*!< (@ 0x0000018C) Pattern Match Mask Register Slice 9                  */

    struct {
      __IOM uint32_t
        MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower 16 bits                                        */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_9_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_MASK_REG_SLICE_10; /*!< (@ 0x00000190) Pattern Match Mask Register Slice 10                */

    struct {
      __IOM uint32_t
        MATCH_MASK_LOWER16_BITS : 32; /*!< [31..0] Enable for lower 16 bits                                        */
    } SIO_PATTERN_MATCH_MASK_REG_SLICE_10_b;
  };
  __IM uint32_t RESERVED1[5];

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_REG_SLICE_0; /*!< (@ 0x000001A8) Pattern Match Mask Register Slice 0                       */

    struct {
      __IOM uint32_t
        PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16-bits of pattern to be detected                      */
    } SIO_PATTERN_MATCH_REG_SLICE_0_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_REG_SLICE_1; /*!< (@ 0x000001AC) Pattern Match Mask Register Slice 1                       */

    struct {
      __IOM uint32_t
        PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16-bits of pattern to be detected                      */
    } SIO_PATTERN_MATCH_REG_SLICE_1_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_REG_SLICE_2; /*!< (@ 0x000001B0) Pattern Match Mask Register Slice 2                       */

    struct {
      __IOM uint32_t
        PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16-bits of pattern to be detected                      */
    } SIO_PATTERN_MATCH_REG_SLICE_2_b;
  };
  __IM uint32_t RESERVED2[5];

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_REG_SLICE_8; /*!< (@ 0x000001C8) Pattern Match Mask Register Slice 8                       */

    struct {
      __IOM uint32_t
        PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16 bits of pattern to be detected                      */
    } SIO_PATTERN_MATCH_REG_SLICE_8_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_REG_SLICE_9; /*!< (@ 0x000001CC) Pattern Match Mask Register Slice 9                       */

    struct {
      __IOM uint32_t
        PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16 bits of pattern to be detected                      */
    } SIO_PATTERN_MATCH_REG_SLICE_9_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_REG_SLICE_10; /*!< (@ 0x000001D0) Pattern Match Mask Register Slice 10                     */

    struct {
      __IOM uint32_t
        PATTERN_MATCH_LOWER16_BITS : 32; /*!< [31..0] Lower 16 bits of pattern to be detected                      */
    } SIO_PATTERN_MATCH_REG_SLICE_10_b;
  };
  __IM uint32_t RESERVED3[7];

  union {
    __IOM uint32_t
      SIO_SHIFT_INTR_EN_SET_REG; /*!< (@ 0x000001F0) Shift Interrupt Enable Set Register                        */

    struct {
      __IOM uint32_t
        INTR_ENABLE_SET : 16;       /*!< [15..0] Common shift interrupt enable set register for all SIOs           */
      __IM uint32_t RESERVED3 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SHIFT_INTR_EN_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_SHIFT_INTR_EN_CLEAR_REG; /*!< (@ 0x000001F4) Shift Interrupt Enable Clear Register                      */

    struct {
      __OM uint32_t INRT_ENABLE_CLEAR : 16; /*!< [15..0] Common shift interrupt enable Clear register for all
                                                     SIOs                                                                      */
      __OM uint32_t RESERVED3 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SHIFT_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_SHIFT_INTR_MASK_SET_REG; /*!< (@ 0x000001F8) Shift Interrupt Mask Set Register                          */

    struct {
      __IOM uint32_t
        INTR_MASK_SET : 16;         /*!< [15..0] Common shift interrupt enable Set register for all SIOs           */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SHIFT_INTR_MASK_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_SHIFT_INTR_MASK_CLEAR_REG; /*!< (@ 0x000001FC) Shift Interrupt Mask Clear Register                       */

    struct {
      __OM uint32_t
        INTR_MASK_CLEAR : 16;       /*!< [15..0] Common shift interrupt mask clear register for all SIOs           */
      __OM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SHIFT_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_SHIFT_INTR_STATUS_REG; /*!< (@ 0x00000200) Shift Interrupt Status Register                            */

    struct {
      __IOM uint32_t
        INTR_ENABLE_SET : 16;       /*!< [15..0] Common shift interrupt mask clear register for all SIOs           */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SHIFT_INTR_STATUS_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_SWAP_INTR_EN_SET_REG; /*!< (@ 0x00000204) Swap Interrupt Enable Set Register                         */

    struct {
      __IOM uint32_t
        INTR_ENABLE_SET : 16;       /*!< [15..0] Swap interrupt enable set register for all SIOs                   */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SWAP_INTR_EN_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_SWAP_INTR_EN_CLEAR_REG; /*!< (@ 0x00000208) Swap Interrupt Enable Clear Register                       */

    struct {
      __OM uint32_t
        INTR_ENABLE_CLEAR : 16;     /*!< [15..0] Swap interrupt enable Clear register for all SIOs                 */
      __OM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SWAP_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_SWAP_INTR_MASK_SET_REG; /*!< (@ 0x0000020C) Swap Interrupt Mask Set Register                           */

    struct {
      __IOM uint32_t
        INTR_MASK_SET : 16;         /*!< [15..0] Common swap interrupt mask set register for all SIOs              */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SWAP_INTR_MASK_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_SWAP_INTR_MASK_CLEAR_REG; /*!< (@ 0x00000210) Swap Interrupt Mask Clear Register                         */

    struct {
      __OM uint32_t
        INTR_MASK_CLEAR : 16;       /*!< [15..0] Common swap interrupt mask Clear register for all SIOs            */
      __OM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SWAP_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_SWAP_INTR_STATUS_REG; /*!< (@ 0x00000214) Swap Interrupt Statusr Register                            */

    struct {
      __IOM uint32_t
        INTR_ENABLE_SET : 16;       /*!< [15..0] Common swap interrupt status register for all SIOs                */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_SWAP_INTR_STATUS_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_INTR_EN_SET_REG; /*!< (@ 0x00000218) Pattern Match Interrupt Enable Set Register           */

    struct {
      __IOM uint32_t INTR_ENABLE_SET : 16; /*!< [15..0] Common pattern or buffer under run interrupt enable
                                                     set register for all SIOs. Each bit corresponds to one
                                                     SIO                                                                       */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_PATTERN_MATCH_INTR_EN_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_PATTERN_MATCH_INTR_EN_CLEAR_REG; /*!< (@ 0x0000021C) Pattern Match Interrupt Enable Clear Register       */

    struct {
      __OM uint32_t INRT_ENABLE_CLEAR : 16; /*!< [15..0] Common pattern or buffer under run interrupt enable
                                                     clear register for all SIOs                                               */
      __OM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_PATTERN_MATCH_INTR_EN_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_INTR_MASK_SET_REG; /*!< (@ 0x00000220) Pattern Match Interrupt Mask Set Register           */

    struct {
      __IOM uint32_t INTR_MASK_SET : 16; /*!< [15..0] Common pattern or buffer under run interrupt mask set
                                                     register for all SIOs                                                     */
      __IM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_PATTERN_MATCH_INTR_MASK_SET_REG_b;
  };

  union {
    __OM uint32_t
      SIO_PATTERN_MATCH_INTR_MASK_CLEAR_REG; /*!< (@ 0x00000224) Pattern Match Interrupt Mask Clear Register       */

    struct {
      __OM uint32_t INTR_MASK_CLEAR : 16; /*!< [15..0] Common pattern or buffer under run interrupt mask clear
                                                     register for all SIOs                                                     */
      __OM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved for future use                                          */
    } SIO_PATTERN_MATCH_INTR_MASK_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_PATTERN_MATCH_INTR_STATUS_REG; /*!< (@ 0x00000228) Pattern Match Interrupt Status Register               */

    struct {
      __IOM uint32_t INTR_STATUS : 16; /*!< [15..0] Common pattern interrupt status register for all SIOs             */
      __IM uint32_t RESERVED3 : 16;    /*!< [31..16] Reserved for future use                                          */
    } SIO_PATTERN_MATCH_INTR_STATUS_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_BUFFER_INTR_STATUS_REG; /*!< (@ 0x0000022C) Buffer Interrupt Status Register                           */

    struct {
      __IOM uint32_t INTR_STATUS : 16; /*!< [15..0] Common pattern interrupt status register for all SIOs             */
      __IM uint32_t RESERVED1 : 16;    /*!< [31..16] Reserved for future use                                          */
    } SIO_BUFFER_INTR_STATUS_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_OUT_MUX_REG[16]; /*!< (@ 0x00000230) Output muxing Register                                     */

    struct {
      __IOM uint32_t DOUT_OEN_SEL : 3; /*!< [2..0] OEN select for GPIO pin 0                                          */
      __IOM uint32_t DOUT_SEL : 3;     /*!< [5..3] Output mux select for GPIO pin 0                                   */
      __IM uint32_t RESERVED1 : 26;    /*!< [31..6] Reserved for future use                                           */
    } SIO_OUT_MUX_REG_b[16];
  };

  union {
    __IOM uint32_t
      SIO_INPUT_MUX_REG[16]; /*!< (@ 0x00000270) Input muxing Register                                      */

    struct {
      __IOM uint32_t CLK_SEL : 3; /*!< [2..0] Input clock select for SIO 0                                       */
      __IOM uint32_t
        QUALIFIER_SELECT : 2; /*!< [4..3] qualifier select                                                   */
      __IOM uint32_t
        QUALIFIER_MODE : 2;         /*!< [6..5] qualifier mode                                                     */
      __IOM uint32_t DIN_SEL : 3;   /*!< [9..7] Data in mux select                                                 */
      __IM uint32_t RESERVED1 : 22; /*!< [31..10] Reserved for future use                                          */
    } SIO_INPUT_MUX_REG_b[16];
  };

  union {
    __IOM uint32_t SIO_FIFO_WR_RD_REG; /*!< (@ 0x000002B0) FIFO READ/WRITE Register                                   */

    struct {
      __IOM uint32_t FIFO_DATA_REGISTER : 32; /*!< [31..0] Writes and read into this register will be written into
                                                     SIO buffer register                                                       */
    } SIO_FIFO_WR_RD_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_FIFO_WR_OFFSET_START_REG; /*!< (@ 0x000002B4) Points to start slice number forming the FIFO              */

    struct {
      __IOM uint32_t SIO_START_SLICE_NUMBER : 32; /*!< [31..0] Points to start slice number forming the FIFO,On write,
                                                     FIFO_WR_OFFSET_CNT_REG  will also be reset to the value
                                                     pointed written into this register                                        */
    } SIO_FIFO_WR_OFFSET_START_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_FIFO_WR_OFFSET_END_REG; /*!< (@ 0x000002B8) SIO last slice no indication Register                      */

    struct {
      __IOM uint32_t
        SIO_END_SLICE_NUMBER : 32; /*!< [31..0] points to last slice no forming fifo                              */
    } SIO_FIFO_WR_OFFSET_END_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_FIFO_WR_OFFSET_CNT_REG; /*!< (@ 0x000002BC) Points to current slice number forming the FIFO            */

    struct {
      __IOM uint32_t SIO_CURRENT_SLICE_NUMBER : 32; /*!< [31..0] Next FIFO operation will happen to buffer in the slice
                                                     pointed by this register                                                  */
    } SIO_FIFO_WR_OFFSET_CNT_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_FIFO_RD_OFFSET_START_REG; /*!< (@ 0x000002C0) Points to start slice number forming the FIFO              */

    struct {
      __IOM uint32_t
        SIO_START_SLICE_NUMBER : 32; /*!< [31..0] Points to start slice number forming the FIFO                    */
    } SIO_FIFO_RD_OFFSET_START_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_FIFO_RD_OFFSET_END_REG; /*!< (@ 0x000002C4) Points to last slice number forming the FIFO               */

    struct {
      __IOM uint32_t
        SIO_END_SLICE_NUMBER : 32; /*!< [31..0] Points to last slice number forming the FIFO                      */
    } SIO_FIFO_RD_OFFSET_END_REG_b;
  };

  union {
    __IOM uint32_t
      SIO_FIFO_RD_OFFSET_CNT_REG; /*!< (@ 0x000002C8) Points to start current number forming the FIFO            */

    struct {
      __IOM uint32_t SIO_CURRENT_SLICE_NUMBER : 32; /*!< [31..0] Next FIFO operation will happen to buffer in the slice
                                                     pointed by this register This register has to be set to
                                                     zero before starting fresh DMA operation                                  */
    } SIO_FIFO_RD_OFFSET_CNT_REG_b;
  };
} SIO_Type; /*!< Size = 716 (0x2cc)                                                        */

/* ================================================================================ */
/* ================                      QSPI                      ================ */
/* ================================================================================ */

/**
 * @brief The queued serial peripheral interface module provides a serial peripheral interface with queued transfer capability  (QSPI)
 */

typedef struct { /*!< QSPI Structure                                                        */

  union {
    __IO uint32_t QSPI_CLK_CONFIG; /*!< QSPI Clock Configuration Register                                     */

    struct {
      __IO uint32_t QSPI_AUTO_CSN_HIGH_CNT : 5; /*!< Minimum SOC clock cycles, during which QSPI auto csn should
                                                         be high between consecutive CSN assertions                            */
      __IO uint32_t QSPI_CLK_SYNC : 1;          /*!< If the clock frequency to FLASH(spi_clk) and QSPI(hclk) controller
                                                         is same, this bit can be set to one to by-pass the syncros results
                                                          in time consumption                                                  */
      uint32_t : 2;
      __IO uint32_t QSPI_CLK_EN : 1; /*!< QSPI clock enable                                                     */
    } QSPI_CLK_CONFIG_b;             /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_BUS_MODE; /*!< QSPI Bus Mode Register                                                */

    struct {
      __IO uint32_t
        QSPI_9116_FEATURE_EN : 1; /*!< 9115 specific features are enabled with this enable                   */
      __IO uint32_t QSPI_MAN_MODE_CONF_CSN0 : 2; /*!< Configures the QSPI flash for Single/Dual/Quad mode operation
                                                         in manual mode                                                        */
      __IO uint32_t AUTO_MODE_RESET : 1;  /*!< QSPI Auto controller reset. This is not a Self clearing bit           */
      __IO uint32_t QSPI_PREFETCH_EN : 1; /*!< Pre-fetch of data from the model which is connected to QSPI,
                                                         automatically with out reading on AHB and is supplied to AHB,
                                                          when address is matched with AHB read transaction address            */
      __IO uint32_t QSPI_WRAP_EN : 1;     /*!< Model wrap is considered with this bit and uses wrap instruction
                                                         to read from FLASH                                                    */
      __IO uint32_t
        QSPI_AUTO_MODE_FRM_REG : 1; /*!< QSPI Mode of Operation                                                */
      uint32_t : 1;
      __IO uint32_t QSPI_D2_OEN_CSN0 : 1;  /*!< Direction Control for SPI_IO2 in case of dual/single mode for
                                                         chip select0 csn0. It is used both in Auto and Manual Mode            */
      __IO uint32_t QSPI_D3_OEN_CSN0 : 1;  /*!< Direction Control for SPI_IO3 in case of dual/single mode for
                                                         chip select0 csn0. It is used both in Auto and Manual Mode.           */
      __IO uint32_t QSPI_D2_DATA_CSN0 : 1; /*!< Value of SPI_IO2 in case of dual/single mode for chip select0
                                                         csn0. It is used both in Auto and Manual Mode.                        */
      __IO uint32_t QSPI_D3_DATA_CSN0 : 1; /*!< Value of SPI_IO3 in case of dual/single mode for chip select0
                                                         csn0. It is used both in Auto and Manual Mode                         */
      __IO uint32_t QSPI_D2_OEN_CSN1 : 1;  /*!< Direction Control for SPI_IO2 in case of dual/single mode for
                                                         chip select1 csn1                                                     */
      __IO uint32_t QSPI_D3_OEN_CSN1 : 1;  /*!< Direction Control for SPI_IO3 in case of dual/single mode for
                                                         chip select1 csn1                                                     */
      __IO uint32_t QSPI_D2_DATA_CSN1 : 1; /*!< Direction Control for SPI_IO3 in case of dual/single mode for
                                                         chip select1 csn1                                                     */
      __IO uint32_t QSPI_D3_DATA_CSN1 : 1; /*!< Value of SPI_IO3 in case of dual/single mode for chip select1
                                                         csn1                                                                  */
      __IO uint32_t
        QSPI_DATA_SAMPLE_EDGE : 1; /*!< Samples MISO data on clock edges                                      */
      __IO uint32_t
        QSPI_CLK_MODE_CSN0 : 1; /*!< QSPI Clock Mode                                                       */
      __IO uint32_t
        QSPI_CLK_MODE_CSN1 : 1; /*!< QSPI Clock Mode                                                       */
      __IO uint32_t
        QSPI_CLK_MODE_CSN2 : 1; /*!< QSPI Clock Mode                                                       */
      __IO uint32_t
        QSPI_CLK_MODE_CSN3 : 1; /*!< QSPI Clock Mode                                                       */
      uint32_t : 3;
      __IO uint32_t QSPI_D2_OEN_CSN2 : 1;  /*!< Direction Control for SPI_IO2 in case of dual/single mode for
                                                         chip select2 csn2                                                     */
      __IO uint32_t QSPI_D3_OEN_CSN2 : 1;  /*!< Direction Control for SPI_IO3 in case of dual/single mode for
                                                         chip select2 csn2                                                     */
      __IO uint32_t QSPI_D2_DATA_CSN2 : 1; /*!< Value of SPI_IO2 in case of dual/single mode for chip select2
                                                         csn2                                                                  */
      __IO uint32_t QSPI_D3_DATA_CSN2 : 1; /*!< Value of SPI_IO3 in case of dual/single mode for chip select2
                                                         csn2                                                                  */
      __IO uint32_t QSPI_D2_OEN_CSN3 : 1;  /*!< Direction Control for SPI_IO2 in case of dual/single mode for
                                                         chip select3 csn3                                                     */
      __IO uint32_t QSPI_D3_OEN_CSN3 : 1;  /*!< Direction Control for SPI_IO3 in case of dual/single mode for
                                                         chip select3 csn3                                                     */
      __IO uint32_t QSPI_D2_DATA_CSN3 : 1; /*!< Value of SPI_IO2 in case of dual/single mode for chip select3
                                                         csn3                                                                  */
      __IO uint32_t QSPI_D3_DATA_CSN3 : 1; /*!< Value of SPI_IO3 in case of dual/single mode for chip select3
                                                         csn3                                                                  */
    } QSPI_BUS_MODE_b;                     /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_AUTO_CONFIG_1; /*!< QSPI Auto Controller Configuration 1 Register                         */

    struct {
      __IO uint32_t
        QSPI_EXT_BYTE_MODE_CSN0 : 2; /*!< Mode of operation of QSPI in the extra byte phase                     */
      __IO uint32_t
        QSPI_DUMMY_MODE_CSN0 : 2; /*!< Mode of operation of QSPI in instruction phase                        */
      __IO uint32_t
        QSPI_ADDR_MODE_CSN0 : 2; /*!< Mode of operation of QSPI in instruction phase                        */
      __IO uint32_t
        QSPI_CMD_MODE_CSN0 : 2; /*!< Mode of operation of QSPI in instruction phase                        */
      __IO uint32_t
        QSPI_DATA_MODE_CSN0 : 2; /*!< Mode of operation of QSPI in DATA phase                               */
      __IO uint32_t QSPI_EXTRA_BYTE_CSN0 : 8;    /*!< Value of the extra byte to be transmitted, if the extra byte
                                                         mode is enabled                                                       */
      __IO uint32_t QSPI_EXTRA_BYTE_EN_CSN0 : 2; /*!< Value of the extra byte to be transmitted, if the extra byte
                                                         mode is enabled                                                       */
      uint32_t : 3;
      __IO uint32_t QSPI_PG_JUMP_CSN0 : 1; /*!< NONE                                                                  */
      __IO uint32_t QSPI_DUMMY_BYTES_INCR_CSN0 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode                                                              */
      __IO uint32_t QSPI_DUMMY_BYTES_WRAP_CSN0 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode in case of wrap instruction                                  */
    } QSPI_AUTO_CONFIG_1_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_AUTO_CONFIG_2; /*!< QSPI Auto Controller Configuration 2 Register                         */

    struct {
      __IO uint32_t
        QSPI_RD_DATA_SWAP_AUTO_CSN0 : 1; /*!< NONE                                                                 */
      __IO uint32_t
        QSPI_ADR_SIZE_16_BIT_AUTO_MODE_CSN0 : 1; /*!< NONE                                                         */
      __IO uint32_t
        QSPI_CONTI_RD_EN_CSN0 : 1; /*!< NONE                                                                  */
      __IO uint32_t
        DUMMY_BYTES_WR_RD_CSN0 : 1; /*!< Dummy bytes to the model to be read or to be write                    */
      __IO uint32_t
        QSPI_DUMMY_BYTES_JMP_CSN : 4;      /*!< Dummy cycles to be selected in case of JUMP                           */
      __IO uint32_t QSPI_RD_INST_CSN0 : 8; /*!< Read instruction to be used for the selected SPI modes and when
                                                         wrap                                                                  */
      __IO uint32_t QSPI_RD_WRAP_INT_CSN0 : 8; /*!< Read instruction to be used, when wrap mode is supported by
                                                         QSPI flash                                                            */
      __IO uint32_t
        QSPI_PG_JUMP_INST_CSN0 : 8; /*!< Read instruction to be used, when Page jump is to be used             */
    } QSPI_AUTO_CONFIG_2_b;         /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_MANUAL_CONFIG1; /*!< QSPI Manual Configuration 1 Register                                  */

    struct {
      __IO uint32_t QSPI_MANUAL_CSN : 1; /*!< SPI CS in manual mode                                                 */
      __IO uint32_t QSPI_MANUAL_WR : 1;  /*!< Write enable for manual mode when CS is low                           */
      __IO uint32_t QSPI_MANUAL_RD : 1;  /*!< Read enable for manual mode when CS is low                            */
      __IO uint32_t QSPI_MANUAL_RD_CNT : 10; /*!< Indicates total number of bytes to be read along with 31:27
                                                         bits of this register.Maximum length supported is 32k bytes           */
      __IO uint32_t
        QSPI_MANUAL_CSN_SELECT : 2; /*!< Indicates which CSn is valid                                          */
      uint32_t : 4;
      __IO uint32_t
        QSPI_MANUAL_SIZE_FRM_REG : 2; /*!< Manual reads and manual writes follow this size                       */
      __IO uint32_t
        TAKE_QSPI_MANUAL_WR_SIZE_FRM_REG : 1; /*!< NONE                                                            */
      __IO uint32_t
        QSPI_FULL_DUPLEX_EN : 1; /*!< Full duplex mode enable.                                              */
      uint32_t : 2;
      __IO uint32_t
        HW_CTRLD_QSPI_MODE_CTRL : 1; /*!< Hardware controlled qspi mode in between AUTO and manual              */
      __IO uint32_t
        QSPI_MANUAL_QSPI_MODE : 1; /*!< Internally the priority is given to manual mode                       */
    } QSPI_MANUAL_CONFIG1_b;       /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_MANUAL_CONFIG2; /*!< QSPI Manual Configuration 2 Register                                  */

    struct {
      __IO uint32_t
        QSPI_WR_DATA_SWAP_MNL_CSN0 : 1; /*!< Swap the write data inside the QSPI controller it-self                */
      __IO uint32_t
        QSPI_WR_DATA_SWAP_MNL_CSN1 : 1; /*!< Swap the write data inside the QSPI controller it-self.               */
      __IO uint32_t
        QSPI_WR_DATA_SWAP_MNL_CSN2 : 1; /*!< Swap the write data inside the QSPI controller itself.                */
      __IO uint32_t
        QSPI_WR_DATA_SWAP_MNL_CSN3 : 1; /*!< Swap the write data inside the QSPI controller itself.                */
      __IO uint32_t
        QSPI_RD_DATA_SWAP_MNL_CSN0 : 1; /*!< Swap the read data inside the QSPIcontroller it self.                 */
      __IO uint32_t
        QSPI_RD_DATA_SWAP_MNL_CSN1 : 1; /*!< Swap the read data inside the QSPIcontroller itself.                  */
      __IO uint32_t
        QSPI_RD_DATA_SWAP_MNL_CSN2 : 1; /*!< Swap the read data inside the QSPIcontroller it-self                  */
      __IO uint32_t
        QSPI_RD_DATA_SWAP_MNL_CSN3 : 1; /*!< Swap the read data inside the QSPIcontroller itself                   */
      __IO uint32_t QSPI_MAN_MODE_CONF_CSN1 : 2; /*!< Configures the QSPI flash for Single/Dual/Quad mode operation
                                                         in manual mode for chip select1 csn1                                  */
      __IO uint32_t
        QSPI_MAN_MODE_CONF_CSN2 : 2; /*!< Configures the QSPI flash for Single or Dual or Quad mode operation
                                                         in manual mode for chip select2 csn2                                  */
      __IO uint32_t
        QSPI_MAN_MODE_CONF_CSN3 : 2;  /*!< Configures the QSPI flash for Single or Dual or Quad mode operation
                                                         in manual mode for chip select3 csn3                                  */
      __IO uint32_t LOOP_BACK_EN : 1; /*!< Internal loop back test mode.                                         */
      __IO uint32_t
        QSPI_MANUAL_DDR_PHASE : 1;       /*!< DDR operations can be performed even in manual mode                   */
      __IO uint32_t QSPI_DDR_CLK_EN : 1; /*!< DDR operations can be performed even in manual mode                   */
    } QSPI_MANUAL_CONFIG2_b;             /*!< BitSize                                                               */
  };
  __IO uint32_t RESERVED3;

  union {
    __IO uint32_t QSPI_FIFO_THRLD; /*!< QSPI FIFO Threshold Register                                          */

    struct {
      __IO uint32_t FIFO_AEMPTY_THRLD : 4; /*!< FIFO almost empty threshold                                           */
      __IO uint32_t FIFO_AFULL_THRLD : 4;  /*!< FIFO almost full threshold                                            */
      __IO uint32_t WFIFO_RESET : 1;       /*!< Write fifo reset                                                      */
      __IO uint32_t RFIFO_RESET : 1;       /*!< Read fifo reset                                                       */
    } QSPI_FIFO_THRLD_b;                   /*!< BitSize                                                               */
  };

  union {
    __I uint32_t QSPI_MANUAL_STATUS; /*!< QSPI Manual Status Register                                           */

    struct {
      __I uint32_t QSPI_BUSY : 1;          /*!< State of Manual mode.                                                 */
      __I uint32_t FIFO_FULL_WFIFO_S : 1;  /*!< Status indication for Wfifo in manual mode                            */
      __I uint32_t FIFO_AFULL_WFIFO_S : 1; /*!< Status indication for Wfifo in manual mode                            */
      __I uint32_t FIFO_EMPTY_WFIFO : 1;   /*!< Status indication for Wfifo in manual mode                            */
      __I uint32_t FIFO_AEMPTY_WFIFO : 1;  /*!< Status indication for Wfifo in manual mode                            */
      __I uint32_t FIFO_FULL_RFIFO : 1;    /*!< Status indication for Rfifo in manual mode                            */
      __I uint32_t FIFO_AFULL_RFIFO : 1;   /*!< Status indication for Rfifo in manual mode                            */
      __I uint32_t FIFO_EMPTY_RFIFO_S : 1; /*!< Status indication for Rfifo in manual mode                            */
      __I uint32_t
        FIFO_AEMPTY_RFIFO_S : 1;           /*!< Status indication for Rfifo in manual mode                            */
      __I uint32_t GSPI_MANUAL_RD_CNT : 1; /*!< This is a result of 10 bits ORing counter                             */
      __I uint32_t
        AUTO_MODE_FSM_IDLE_SCLK : 1;   /*!< Auto mode idle signal to track auto controller is busy or idle.       */
      __I uint32_t QSPI_AUTO_MODE : 1; /*!< QSPI controller status.                                               */
      __I uint32_t QSPI_AUTO_MODE_FRM_REG_SCLK : 1; /*!< QSPI auto mode status. Valid only when HW_CTRLD_QSPI_MODE_CTRL
                                                         is zero.                                                              */
      __I uint32_t HW_CTRLD_MODE_SCLK : 1; /*!< QSPI mode status in HW_CTRLD_MODE                                     */
      __I uint32_t
        HW_CTRLD_MODE_CTRL_SCLK : 1; /*!< HW_CTRLD_MODE status                                                  */
    } QSPI_MANUAL_STATUS_b;          /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_INTR_MASK; /*!< QSPI Interrupt Mask Register                                          */

    struct {
      __IO uint32_t QSPI_INTR_MASK : 1; /*!< Interrupt Status bit                                                  */
      __IO uint32_t
        FIFO_AEMPTY_RFIFO_MASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_AFULL_RFIFO_MASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_AEMPTY_WFIFO_MASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_AFULL_WFIFO_MASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_FULL_WFIFO_MASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_EMPTY_RFIFO_MASK : 1; /*!< NONE                                                                  */
    } QSPI_INTR_MASK_b;            /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_INTR_UNMASK; /*!< QSPI Interrupt Unmask Register                                        */

    struct {
      __IO uint32_t QSPI_INTR_UNMASK : 1; /*!< Interrupt Status bit                                                  */
      __IO uint32_t
        FIFO_AEMPTY_RFIFO_UN : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_AFULL_RFIFO_UNMASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_AEMPTY_WFIFO_UNMASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_AFULL_WFIFO_UNMASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_FULL_WFIFO_UNMASK : 1; /*!< NONE                                                                  */
      __IO uint32_t
        FIFO_EMPTY_RFIFO_UNMASK : 1; /*!< NONE                                                                  */
    } QSPI_INTR_UNMASK_b;            /*!< BitSize                                                               */
  };

  union {
    __I uint32_t QSPI_INTR_STS; /*!< QSPI Interrupt Status Register                                        */

    struct {
      __I uint32_t QSPI_INTR_LVL : 1; /*!< Interrupt Status bit                                                  */
      __I uint32_t
        FIFO_AEMPTY_RFIFO_LVL : 1; /*!< NONE                                                                  */
      __I uint32_t
        FIFO_AFULL_RFIFO_LVL : 1; /*!< NONE                                                                  */
      __I uint32_t
        FIFO_AEMPTY_WFIFO_LVL : 1; /*!< NONE                                                                  */
      __I uint32_t
        FIFO_AFULL_WFIFO_LVL : 1; /*!< NONE                                                                  */
      __I uint32_t
        FIFO_FULL_WFIFO_LVL : 1; /*!< NONE                                                                  */
      __I uint32_t
        FIFO_EMPTY_RFIFO_LVL : 1; /*!< NONE                                                                  */
    } QSPI_INTR_STS_b;            /*!< BitSize                                                               */
  };

  union {
    __O uint32_t QSPI_INTR_ACK; /*!< QSPI Interrupt Acknowledge Register                                   */

    struct {
      __O uint32_t QSPI_INTR_ACK : 1; /*!< Interrupt Status bit                                                  */
      __O uint32_t
        FIFO_AEMPTY_RFIFO_ACK : 1; /*!< NONE                                                                  */
      __O uint32_t
        FIFO_AFULL_RFIFO_ACK : 1; /*!< NONE                                                                  */
      __O uint32_t
        FIFO_AEMPTY_WFIFO_ACK : 1; /*!< NONE                                                                  */
      __O uint32_t
        FIFO_AFULL_WFIFO_ACK : 1; /*!< NONE                                                                  */
      __O uint32_t
        FIFO_FULL_WFIFO_ACK : 1; /*!< NONE                                                                  */
      __O uint32_t
        FIFO_EMPTY_RFIFO_ACK : 1; /*!< NONE                                                                  */
    } QSPI_INTR_ACK_b;            /*!< BitSize                                                               */
  };

  union {
    __I uint32_t QSPI_STS_MC; /*!< QSPI State Machine Monitor Register                                   */

    struct {
      __I uint32_t BUS_CTRL_PSTATE : 4;    /*!< Bus controller present state                                          */
      __I uint32_t AUTO_CTRL_PSTATE : 3;   /*!< Auto controller present state                                         */
      __I uint32_t QSPI_MASTER_PSTATE : 3; /*!< Qspi master present state                                             */
    } QSPI_STS_MC_b;                       /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_AUTO_CONFIG_1_CSN1; /*!< QSPI Auto Controller Configuration 1 CSN1 Register                    */

    struct {
      __IO uint32_t
        QSPI_EXT_BYTE_MODE_CSN1 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_DUMMY_MODE_CSN1 : 2; /*!< Mode of operation of QSPI in instruction phase                        */
      __IO uint32_t
        QSPI_ADDR_MODE_CSN1 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_CMD_MODE_CSN1 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_DATA_MODE_CSN1 : 2; /*!< Mode of operation of QSPI in DATA phase.                              */
      __I uint32_t QSPI_EXTRA_BYTE_CSN1 : 8; /*!< Value of the extra byte to be transmitted, if the extra byte
                                                         mode is enabled.                                                      */
      __IO uint32_t
        QSPI_EXTRA_BYTE_EN_CSN1 : 2; /*!< Mode of operation of QSPI in DATA phase.                              */
      uint32_t : 3;
      __O uint32_t QSPI_PG_JUMP_CSN1 : 1; /*!< NONE                                                                  */
      __I uint32_t QSPI_DUMMY_BYTES_INCR_CSN1 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode.                                                             */
      __I uint32_t QSPI_DUMMY_BYTES_WRAP_CSN1 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode in case of wrap instruction.                                 */
    } QSPI_AUTO_CONFIG_1_CSN1_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t
      QSPI_AUTO_CONFIG_2_CSN1_REG; /*!< QSPI Auto Controller Configuration 2 CSN1 Register                    */

    struct {
      __IO uint32_t QSPI_RD_SWAP_AUTO_CSN1 : 1; /*!< Swap the read data from the flash in byte order for chip select1
                                                         csn1 in auto mode.                                                    */
      __IO uint32_t
        QSPI_ADR_SIZE_16BIT_AUTO_MODE_CSN1 : 1; /*!< NONE                                                          */
      __IO uint32_t
        QSPI_CONTI_RD_EN_CSN1 : 1;         /*!< Continuous read enable bit.                                           */
      __IO uint32_t DUMMY_BYTES_WR_RD : 1; /*!< Dummy bytes to the model to be read or to be write.                   */
      __IO uint32_t
        QSPI_DUMMY_BYTES_JMP_CSN1 : 4;     /*!< Dummy cycles to be selected in case of JUMP                           */
      __IO uint32_t QSPI_RD_INST_CSN1 : 8; /*!< Read instruction to be used for the selected SPI modes and when
                                                         wrap is not needed or supported                                       */
      __IO uint32_t QSPI_RD_WRAP_INST_CSN1 : 8; /*!< Read instruction to be used for the selected SPI modes and when
                                                         wrap is not needed or supported                                       */
      __IO uint32_t
        QSPI_PG_JMP_INST_CSN1 : 8;   /*!< Read instruction to be used, when Page jump is to be used.            */
    } QSPI_AUTO_CONFIG_2_CSN1_REG_b; /*!< BitSize                                                               */
  };
  __IO uint32_t QSPI_MANUAL_RDWR_FIFO[16]; /*!< QSPI FIFOs                                                            */
  union {
    __IO uint32_t QSPI_MANUAL_WRITE_DATA2; /*!< QSPI Manual Write Data 2 Register                                     */

    struct {
      __IO uint32_t
        QSPI_MANUAL_WRITE_DATA2 : 5; /*!< Number of bits to be written in write mode                            */
      uint32_t : 2;
      __IO uint32_t USE_PREV_LENGTH : 1; /*!< Use previous length.                                                  */
    } QSPI_MANUAL_WRITE_DATA2_b;         /*!< BitSize                                                               */
  };
  union {
    __IO uint32_t QSPI_AUTO_CONFIG_1_CSN2; /*!< QSPI Auto Controller Configuration 1 CSN2 Register                    */

    struct {
      __IO uint32_t
        QSPI_EXT_BYTE_MODE_CSN2 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_DUMMY_MODE_CSN2 : 2; /*!< Mode of operation of QSPI in instruction phase                        */
      __IO uint32_t
        QSPI_ADDR_MODE_CSN2 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_CMD_MODE_CSN2 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_DATA_MODE_CSN2 : 2; /*!< Mode of operation of QSPI in DATA phase.                              */
      __I uint32_t QSPI_EXTRA_BYTE_CSN2 : 8; /*!< Value of the extra byte to be transmitted, if the extra byte
                                                         mode is enabled.                                                      */
      __IO uint32_t
        QSPI_EXTRA_BYTE_EN_CSN2 : 2; /*!< Mode of operation of QSPI in DATA phase.                              */
      uint32_t : 3;
      __O uint32_t QSPI_PG_JUMP_CSN2 : 1; /*!< NONE                                                                  */
      __I uint32_t QSPI_DUMMY_BYTES_INCR_CSN2 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode.                                                             */
      __I uint32_t QSPI_DUMMY_BYTES_WRAP_CSN2 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode in case of wrap instruction.                                 */
    } QSPI_AUTO_CONFIG_1_CSN2_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t
      QSPI_AUTO_CONFIG_2_CSN2_REG; /*!< QSPI Auto Controller Configuration 2 CSN2 Register                    */

    struct {
      __IO uint32_t QSPI_RD_SWAP_AUTO_CSN2 : 1; /*!< Swap the read data from the flash in byte order for chip select1
                                                         csn1 in auto mode.                                                    */
      __IO uint32_t
        QSPI_ADR_SIZE_16BIT_AUTO_MODE_CSN2 : 1; /*!< NONE                                                          */
      __IO uint32_t
        QSPI_CONTI_RD_EN_CSN2 : 1; /*!< Continuous read enable bit.                                           */
      __IO uint32_t
        DUMMY_BYTES_WR_RD_CSN2 : 1; /*!< Dummy bytes to the model to be read or to be write.                   */
      __IO uint32_t
        QSPI_DUMMY_BYTES_JMP_CSN2 : 4;     /*!< Dummy cycles to be selected in case of JUMP                           */
      __IO uint32_t QSPI_RD_INST_CSN2 : 8; /*!< Read instruction to be used for the selected SPI modes and when
                                                         wrap is not needed or supported                                       */
      __IO uint32_t QSPI_RD_WRAP_INST_CSN2 : 8; /*!< Read instruction to be used for the selected SPI modes and when
                                                         wrap is not needed or supported                                       */
      __IO uint32_t
        QSPI_PG_JMP_INST_CSN2 : 8;   /*!< Read instruction to be used, when Page jump is to be used.            */
    } QSPI_AUTO_CONFIG_2_CSN2_REG_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QSPI_AUTO_CONFIG_1_CSN3; /*!< QSPI Auto Controller Configuration 1 CSN3 Register                    */

    struct {
      __IO uint32_t
        QSPI_EXT_BYTE_MODE_CSN3 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_DUMMY_MODE_CSN3 : 2; /*!< Mode of operation of QSPI in instruction phase                        */
      __IO uint32_t
        QSPI_ADDR_MODE_CSN3 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_CMD_MODE_CSN3 : 2; /*!< Mode of operation of QSPI in instruction phase.                       */
      __IO uint32_t
        QSPI_DATA_MODE_CSN3 : 2; /*!< Mode of operation of QSPI in DATA phase.                              */
      __I uint32_t QSPI_EXTRA_BYTE_CSN3 : 8; /*!< Value of the extra byte to be transmitted, if the extra byte
                                                         mode is enabled.                                                      */
      __IO uint32_t
        QSPI_EXTRA_BYTE_EN_CSN3 : 2; /*!< Mode of operation of QSPI in DATA phase.                              */
      uint32_t : 3;
      __O uint32_t QSPI_PG_JUMP_CSN3 : 1; /*!< NONE                                                                  */
      __I uint32_t QSPI_DUMMY_BYTES_INCR_CSN2 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode.                                                             */
      __I uint32_t QSPI_DUMMY_BYTES_WRAP_CSN2 : 4; /*!< Specifies the number of dummy bytes 0 to 7 for the selected
                                                         SPI mode in case of wrap instruction.                                 */
    } QSPI_AUTO_CONFIG_1_CSN3_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t
      QSPI_AUTO_CONFIG_2_CSN3_REG; /*!< QSPI Auto Controller Configuration 2 CSN3 Register                    */

    struct {
      __IO uint32_t QSPI_RD_SWAP_AUTO_CSN3 : 1; /*!< Swap the read data from the flash in byte order for chip select1
                                                         csn1 in auto mode.                                                    */
      __IO uint32_t
        QSPI_ADR_SIZE_16BIT_AUTO_MODE_CSN3 : 1; /*!< NONE                                                          */
      __IO uint32_t
        QSPI_CONTI_RD_EN_CSN3 : 1; /*!< Continuous read enable bit.                                           */
      __IO uint32_t
        DUMMY_BYTES_WR_RD_CSN3 : 1; /*!< Dummy bytes to the model to be read or to be write.                   */
      __IO uint32_t
        QSPI_DUMMY_BYTES_JMP_CSN3 : 4;     /*!< Dummy cycles to be selected in case of JUMP                           */
      __IO uint32_t QSPI_RD_INST_CSN3 : 8; /*!< Read instruction to be used for the selected SPI modes and when
                                                         wrap is not needed or supported                                       */
      __IO uint32_t QSPI_RD_WRAP_INST_CSN3 : 8; /*!< Read instruction to be used for the selected SPI modes and when
                                                         wrap is not needed or supported                                       */
      __IO uint32_t
        QSPI_PG_JMP_INST_CSN3 : 8;   /*!< Read instruction to be used, when Page jump is to be used.            */
    } QSPI_AUTO_CONFIG_2_CSN3_REG_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t
      QSPI_AUTO_CONFIG_3_CSN0_REG; /*!< QSPI Auto Controller Configuration 3 CSN0 Register                    */

    struct {
      uint32_t : 12;
      __IO uint32_t
        QSPI_DDR_CMD_MODE_CSN0 : 1; /*!< DDR Command mode                                                      */
      __IO uint32_t
        QSPI_DDR_ADDR_MODE_CSN0 : 1; /*!< DDR Address mode                                                      */
      __IO uint32_t
        QSPI_DDR_DUMMY_MODE_CSN0 : 1; /*!< DDR Address mode                                                      */
      __IO uint32_t
        QSPI_DDR_EXTRA_MODE_CSN0 : 1; /*!< DDR Address mode                                                      */
      __IO uint32_t
        QSPI_DDR_DATA_MODE_CSN0 : 1; /*!< DDR Address mode                                                      */
    } QSPI_AUTO_CONFIG_3_CSN0_REG_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t
      QSPI_AUTO_CONFIG_3_CSN1_REG; /*!< QSPI Auto Controller Configuration 3 CSN1 Register                    */

    struct {
      uint32_t : 12;
      __IO uint32_t
        QSPI_DDR_CMD_MODE_CSN1 : 1; /*!< DDR Command mode                                                      */
      __IO uint32_t
        QSPI_DDR_ADDR_MODE_CSN1 : 1; /*!< DDR Address mode                                                      */
      __IO uint32_t
        QSPI_DDR_DUMMY_MODE_CSN1 : 1; /*!< DDR Address mode                                                      */
      __IO uint32_t
        QSPI_DDR_EXTRA_MODE_CSN1 : 1; /*!< DDR Address mode                                                      */
      __IO uint32_t
        QSPI_DDR_DATA_MODE_CSN1 : 1; /*!< DDR Address mode                                                      */
    } QSPI_AUTO_CONFIG_3_CSN1_REG_b; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED[11];

  union {
    __IO uint32_t
      OCTASPI_BUS_CONTROLLER_2_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t SET_IP_MODE : 1;    /*!< This bit enables the qspi interface pins into HiZ mode                */
      __IO uint32_t AES_NONCE_INIT : 1; /*!< This bit enables the AES initialization with nonce                    */
      __IO uint32_t AES_SEC_ENABLE : 1; /*!< This bit enables the AES security enable or not                       */
      __IO uint32_t DUAL_MODE_EN : 1;   /*!< Dual flash mode enable control.                                       */
      __IO uint32_t CSN0_2_CSN : 2;     /*!< Map csn0 to the programmed csn. It is valid for both manual
                                                         and auto modes                                                        */
      __IO uint32_t CSN1_2_CSN : 2;     /*!< Map csn1 to the programmed csn. It is valid for both manual
                                                         and auto modes                                                        */
      __IO uint32_t CSN2_2_CSN : 2;     /*!< Map csn2 to the programmed csn. It is valid for both manual
                                                         and auto modes                                                        */
      __IO uint32_t CSN3_2_CSN : 2;     /*!< Map csn3 to the programmed csn. It is valid for both manual
                                                         and auto modes                                                        */
      __IO uint32_t AES_SEC_ENABLE_SG1 : 1; /*!< This bit enables the AES security enable or not for segment
                                                         1                                                                     */
      __IO uint32_t AES_SEC_ENABLE_SG2 : 1; /*!< This bit enables the AES security enable or not for segment
                                                         2                                                                     */
      __IO uint32_t AES_SEC_ENABLE_SG3 : 1; /*!< This bit enables the AES security enable or not for segment
                                                         3                                                                     */
      __IO uint32_t AES_SEC_ENABLE_SG4 : 1; /*!< This bit enables the AES security enable or not for segment
                                                         4                                                                     */
      __IO uint32_t
        DUAL_MODE_SWAP_LINES : 1; /*!< This bit controls the 8 lines of qspi with 4 bit swap manner          */
      uint32_t : 2;
      __IO uint32_t
        DUAL_STAGE_EN_MANUAL : 1;     /*!< Dual stage en for dual flash mode                                     */
    } OCTASPI_BUS_CONTROLLER_2_REG_b; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED1[15];

  union {
    __IO uint32_t
      QSPI_SRAM_CTRL_CSN_REG[4]; /*!< none                                                                  */

    struct {
      __IO uint32_t BIT_8_MODE : 1;      /*!< Flash 8bit (1 byte) boundary mode                                     */
      __IO uint32_t BYTE_32_MODE : 1;    /*!< Flash 32 byte boundary mode                                           */
      __IO uint32_t ADDR_16BIT_MODE : 1; /*!< Send only lower 16bits of Address enable.                             */
      uint32_t : 5;
      __IO uint32_t CMD_MODE : 2;  /*!< writing cmd mode                                                      */
      __IO uint32_t ADDR_MODE : 2; /*!< writing address mode                                                  */
      __IO uint32_t DATA_MODE : 2; /*!< writing address mode                                                  */
      uint32_t : 2;
      __IO uint32_t WR_CMD : 8;    /*!< Command to be used for writing                                        */
    } QSPI_SRAM_CTRL_CSN_REG_b[4]; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED2[2];
  __IO uint32_t SEMI_AUTO_MODE_ADDR_REG; /*!< Byte address to read the data from flash in semi auto mode.
                                                         It is valid only semi auto mode enable bit is asserted                */

  union {
    __IO uint32_t
      SEMI_AUTO_MODE_CONFIG_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t
        QSPI_SEMI_AUTO_BSIZE : 8; /*!< This is burst size to read data from flash in semi auto mode          */
      __IO uint32_t
        QSPI_SEMI_AUTO_HSIZE : 2;  /*!< Indicates number of bytes valid in each transaction                   */
    } SEMI_AUTO_MODE_CONFIG_REG_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t
      SEMI_AUTO_MODE_CONFIG2_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t QSPI_SEMI_AUTO_RD_CNT : 12; /*!< Total number of bytes to be read flash continuously from the
                                                         address given by SEMI_AUTO_MODE_ADDR_REG                              */
      __IO uint32_t QSPI_SEMI_AUTO_MODE_EN : 1; /*!< Enable for semi auto mode read operation. Make sure manual mode
                                                         read/write operation is completed before asserting this bit           */
      __IO uint32_t QSPI_SEMI_AUTO_RD_BUSY : 1; /*!< Indicates status of semi auto mode read status. If it is high,
                                                         semi auto mode read operation is progressing                          */
    } SEMI_AUTO_MODE_CONFIG2_REG_b; /*!< BitSize                                                               */
  };
} QSPI_Type;

/* =========================================================================================================================== */
/* ================                                            CRC                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief CRC is used in all wireless communication as a first data integrity check (CRC)
  */

typedef struct { /*!< (@ 0x45080000) CRC Structure                                              */

  union {
    __IOM uint32_t
      CRC_GEN_CTRL_SET_REG; /*!< (@ 0x00000000) General control set register                               */

    struct {
      __IOM uint32_t SOFT_RST : 1;   /*!< (@ 0x00000000) Soft reset. This clears the FIFO and settles
                                                                    all the state machines to their IDLE                       */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use.                                   */
    } CRC_GEN_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t CRC_GEN_CTRL_RESET; /*!< (@ 0x00000004) General control reset register                             */

    struct {
      __IOM uint32_t RESERVED1 : 32; /*!< (@ 0x00000000) Reserved for future use.                                   */
    } CRC_GEN_CTRL_RESET_b;
  };

  union {
    __IM uint32_t CRC_GEN_STS; /*!< (@ 0x00000008) General status register                                    */

    struct {
      __IM uint32_t CALC_DONE : 1;          /*!< (@ 0x00000000) When the computation of final CRC with the data
                                                                    out of fifo, this will get set to 1 otherwise
                                                                    0                                                          */
      __IM uint32_t DIN_NUM_BYTES_DONE : 1; /*!< (@ 0x00000001) When number of bytes requested for computation
                                                                    of final CRC is read from fifo by internal
                                                                    FSM, this will get set to 1 otherwise 0.                   */
      __IM uint32_t RESERVED1 : 30; /*!< (@ 0x00000002) Reserved for future use.                                   */
    } CRC_GEN_STS_b;
  };

  union {
    __IOM uint32_t CRC_POLYNOMIAL; /*!< (@ 0x0000000C) This register holds the polynomial with which
                                                                    the final CRC is computed.                                 */

    struct {
      __IOM uint32_t POLYNOMIAL : 32; /*!< (@ 0x00000000) Polynomial register. This register holds the
                                                                    polynomial with which the final CRC is
                                                                    computed.When write Polynomial will be
                                                                    updated.When read read polynomial.                         */
    } CRC_POLYNOMIAL_b;
  };

  union {
    __IOM uint32_t
      CRC_POLYNOMIAL_CTRL_SET; /*!< (@ 0x00000010) Polynomial control set register                            */

    struct {
      __IOM uint32_t POLYNOMIAL_WIDTH_SET : 5; /*!< (@ 0x00000000) Polynomial width set. Number of bits/width of
                                                                    the polynomial has to be written here for
                                                                    the computation of final CRC. If a new
                                                                    width has to be configured, clear the existing
                                                                    length first by writing 0x1f in polynomial_ctrl_reset
                                                                    register. When read, actual polynomial
                                                                    width is read.                                             */
      __IOM uint32_t RESERVED1 : 27; /*!< (@ 0x00000005) Reserved for future use.                                   */
    } CRC_POLYNOMIAL_CTRL_SET_b;
  };

  union {
    __IOM uint32_t
      CRC_POLYNOMIAL_CTRL_RESET; /*!< (@ 0x00000014) Polynomial control set register                            */

    struct {
      __IOM uint32_t POLYNOMIAL_WIDTH_SET : 5; /*!< (@ 0x00000000) Polynomial width reset. If a new width has to
                                                                    be configured, clear the existing length
                                                                    first by writing 0x1f. When read, actual
                                                                    polynomial width is read.                                  */
      __IOM uint32_t RESERVED1 : 27; /*!< (@ 0x00000005) Reserved for future use.                                   */
    } CRC_POLYNOMIAL_CTRL_RESET_b;
  };

  union {
    __IOM uint32_t CRC_LFSR_INIT_VAL; /*!< (@ 0x00000018) LFSR initial value                                         */

    struct {
      __IOM uint32_t LFSR_INIT : 32; /*!< (@ 0x00000000) This holds LFSR initialization value. When ever
                                                                    LFSR needs to be initialized, this has
                                                                    to be updated with the init value and trigger
                                                                    init_lfsr in LFSR_INIT_CTRL_SET register.
                                                                    For example, in WiFi case, 0xffffffff is
                                                                    used as init value of LFSR.                                */
    } CRC_LFSR_INIT_VAL_b;
  };

  union {
    __IOM uint32_t
      CRC_LFSR_INIT_CTRL_SET; /*!< (@ 0x0000001C) LFSR state initialization control set register             */

    struct {
      __IOM uint32_t CLEAR_LFSR : 1;           /*!< (@ 0x00000000) Clear LFSR state. When this is set, LFSR state
                                                                    is cleared to 0                                            */
      __IOM uint32_t INIT_LFSR : 1;            /*!< (@ 0x00000001) Initialize LFSR state. When this is set LFSR
                                                                    state will be initialized with LFSR_INIT_VAL/bit
                                                                    swapped LFSR_INIT_VAL in the next cycle                    */
      __IOM uint32_t USE_SWAPPED_INIT_VAL : 1; /*!< (@ 0x00000002) Use bit swapped init value. If this is set bit
                                                                    swapped version of LFSR init value will
                                                                    be loaded / initialized to LFSR state                      */
      __IOM uint32_t RESERVED1 : 29; /*!< (@ 0x00000003) Reserved for future use.                                   */
    } CRC_LFSR_INIT_CTRL_SET_b;
  };

  union {
    __IOM uint32_t
      CRC_LFSR_INIT_CTRL_RESET; /*!< (@ 0x00000020) LFSR state initialization control reset register           */

    struct {
      __IOM uint32_t RESERVED1 : 1; /*!< (@ 0x00000000) Reserved for future use.                                   */
      __IOM uint32_t RESERVED2 : 1; /*!< (@ 0x00000001) Reserved for future use.                                   */
      __IOM uint32_t USE_SWAPPED_INIT_VAL : 1; /*!< (@ 0x00000002) Use bit swapped init value. If this is set bit
                                                                    swapped version of LFSR init value will
                                                                    be loaded / initialized to LFSR state                      */
      __IOM uint32_t RESERVED3 : 29; /*!< (@ 0x00000003) Reserved for future use.                                   */
    } CRC_LFSR_INIT_CTRL_RESET_b;
  };

  union {
    __OM uint32_t CRC_DIN_FIFO; /*!< (@ 0x00000024) Data input FIFO register                                   */

    struct {
      __OM uint32_t DIN_FIFO : 32; /*!< (@ 0x00000000) FIFO input port is mapped to this register. Data
                                                                    on which the final CRC has to be computed
                                                                    has to be loaded to this FIFO                              */
    } CRC_DIN_FIFO_b;
  };

  union {
    __IOM uint32_t CRC_DIN_CTRL_SET; /*!< (@ 0x00000028) Input data control set register                            */

    struct {
      __IOM uint32_t DIN_WIDTH_REG : 5; /*!< (@ 0x00000000) Valid number of bits in the input data in din_width_from_reg
                                                                    set mode. Before writing a new value into
                                                                    this, din_ctrl_reset_reg has to be written
                                                                    with 0x1f to clear this field as these
                                                                    are set/clear bits.                                        */
      __IOM uint32_t DIN_WIDTH_FROM_REG : 1; /*!< (@ 0x00000005) Valid number of bits in the input data. In default,
                                                                    number of valid bits in the input data
                                                                    is taken from ULI (uli_be). If this is
                                                                    set, whatever is the input size, only din_ctrl_reg[4:0]
                                                                    is taken as valid length/width for inout
                                                                    data.                                                      */
      __IOM uint32_t DIN_WIDTH_FROM_CNT : 1; /*!< (@ 0x00000006) Valid number of bits in the input data. In default,
                                                                    number of valid bits in the input data
                                                                    is taken from ULI (uli_be). If this is
                                                                    set, a mix of ULI length and number of
                                                                    bytes remaining will form the valid bits
                                                                    (which ever is less that will be considered
                                                                    as valid bits).                                            */
      __IOM uint32_t USE_SWAPPED_DIN : 1;    /*!< (@ 0x00000007) Use bit swapped input data. If this is set, input
                                                                    data will be swapped and filled in to FIFO.
                                                                    Whatever read out from FIFO will be directly
                                                                    fed to LFSR engine.                                        */
      __IOM uint32_t RESET_FIFO_PTRS : 1;    /*!< (@ 0x00000008) Reset fifo pointer. This clears the FIFO.When
                                                                    this is set, FIFO will be cleared.                         */
      __IOM uint32_t RESERVED1 : 15; /*!< (@ 0x00000009) Reserved for future use.                                   */
      __IOM uint32_t FIFO_AEMPTY_THRESHOLD : 4; /*!< (@ 0x00000018) FIFO almost empty threshold value. This has to
                                                                    be cleared by writing 0x0f000000 into din_ctrl_reset
                                                                    before updating any new value.                             */
      __IOM uint32_t FIFO_AFULL_THRESHOULD : 4; /*!< (@ 0x0000001C) FIFO almost full threshold value. This has to
                                                                    be cleared by writing 0xf0000000 into din_ctrl_reset
                                                                    before updating any new value                              */
    } CRC_DIN_CTRL_SET_b;
  };

  union {
    __IOM uint32_t
      CRC_DIN_CTRL_RESET_REG; /*!< (@ 0x0000002C) Input data control set register                            */

    struct {
      __IOM uint32_t DIN_WIDTH_REG : 5; /*!< (@ 0x00000000) Valid number of bits in the input data in din_width_from_reg
                                                                    set mode. Before writing a new value into
                                                                    this, din_ctrl_reset_reg has to be written
                                                                    with 0x1f to clear this field as these
                                                                    are set/clear bits.                                        */
      __IOM uint32_t DIN_WIDTH_FROM_REG : 1; /*!< (@ 0x00000005) Valid number of bits in the input data. In default,
                                                                    number of valid bits in the input data
                                                                    is taken from ULI (uli_be). If this is
                                                                    set, whatever is the input size, only din_ctrl_reg[4:0]
                                                                    is taken as valid length/width for inout
                                                                    data.                                                      */
      __IOM uint32_t DIN_WIDTH_FROM_CNT : 1; /*!< (@ 0x00000006) Valid number of bits in the input data. In default,
                                                                    number of valid bits in the input data
                                                                    is taken from ULI (uli_be). If this is
                                                                    set, a mix of ULI length and number of
                                                                    bytes remaining will form the valid bits
                                                                    (which ever is less that will be considered
                                                                    as valid bits).                                            */
      __IOM uint32_t USE_SWAPPED_DIN : 1;    /*!< (@ 0x00000007) Use bit swapped input data. If this is set input
                                                                    data will be swapped and filled in to FIFO.
                                                                    Whatever read out from FIFO will be directly
                                                                    fed to LFSR engine.                                        */
      __IOM uint32_t RESERVED1 : 1;  /*!< (@ 0x00000008) Reserved for future use.                                   */
      __IOM uint32_t RESERVED2 : 15; /*!< (@ 0x00000009) Reserved for future use.                                   */
      __IOM uint32_t FIFO_AEMPTY_THRESHOLD : 4; /*!< (@ 0x00000018) FIFO almost empty threshold value. This has to
                                                                    be cleared by writing 0x0f000000 into din_ctrl_reset
                                                                    before updating any new value.                             */
      __IOM uint32_t FIFO_AFULL_THRESHOULD : 4; /*!< (@ 0x0000001C) FIFO almost full threshold value. This has to
                                                                    be cleared by writing 0xf0000000 into din_ctrl_reset
                                                                    before updating any new value                              */
    } CRC_DIN_CTRL_RESET_REG_b;
  };

  union {
    __IOM uint32_t CRC_DIN_NUM_BYTES; /*!< (@ 0x00000030) Data input FIFO register                                   */

    struct {
      __IOM uint32_t
        DIN_NUM_BYTES : 32; /*!< (@ 0x00000000) in out data number of bytes                                */
    } CRC_DIN_NUM_BYTES_b;
  };

  union {
    __IM uint32_t CRC_DIN_STS; /*!< (@ 0x00000034) Input data status register                                 */

    struct {
      __IM uint32_t FIFO_EMPTY : 1;  /*!< (@ 0x00000000) FIFO empty indication status                               */
      __IM uint32_t FIFO_AEMPTY : 1; /*!< (@ 0x00000001) FIFO almost empty indication status.                       */
      __IM uint32_t FIFO_AFULL : 1;  /*!< (@ 0x00000002) FIFO almost full indication status                         */
      __IM uint32_t FIFO_FULL : 1;   /*!< (@ 0x00000003) FIFO full indication status                                */
      __IM uint32_t FIFO_OCC : 6;    /*!< (@ 0x00000004) FIFO occupancy                                             */
      __IM uint32_t RESERVED1 : 22;  /*!< (@ 0x0000000A) Reserved for future use.                                   */
    } CRC_DIN_STS_b;
  };

  union {
    __IOM uint32_t CRC_LFSR_STATE; /*!< (@ 0x00000038) LFSR state register                                        */

    struct {
      __IOM uint32_t LFSR_STATE : 32; /*!< (@ 0x00000000) If LFSR dynamic loading is required this can
                                                                    be used for writing the LFSR state directly.               */
    } CRC_LFSR_STATE_b;
  };
} CRC_Type; /*!< Size = 60 (0x3c)                                                          */

/* ================================================================================ */
/* ================                      EFUSE                     ================ */
/* ================================================================================ */

/* ================================================================================ */
/* ================                      EFUSE                     ================ */
/* ================================================================================ */

typedef struct { /*!< (@ 0x4600C000) EFUSE Structure                                            */

  union {
    __IOM uint32_t EFUSE_DA_ADDR_REG; /*!< (@ 0x00000000) Direct Access Registers                                    */

    struct {
      __IOM uint32_t ADDR_BITS : 16; /*!< (@ 0x00000000) These bits specifies the address to write or
                                                                    read from EFUSE macro model                                */
      __IOM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } EFUSE_DA_ADDR_REG_b;
  };

  union {
    __IOM uint32_t
      EFUSE_DA_CTRL_SET_REG; /*!< (@ 0x00000004) Direct Access Set Registers                                */

    struct {
      __IOM uint32_t PGENB : 1;      /*!< (@ 0x00000000) Set Program enable                                         */
      __IOM uint32_t CSB : 1;        /*!< (@ 0x00000001) Set Chip Enable                                            */
      __IOM uint32_t STROBE : 1;     /*!< (@ 0x00000002) Set strobe enable                                          */
      __IOM uint32_t LOAD : 1;       /*!< (@ 0x00000003) Set Load enable                                            */
      __IOM uint32_t RESERVED1 : 12; /*!< (@ 0x00000004) reserved1                                                  */
      __IOM uint32_t RESERVED2 : 16; /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_DA_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t
      EFUSE_DA_CTRL_CLEAR_REG; /*!< (@ 0x00000008) Direct Access Clear Registers                              */

    struct {
      __IOM uint32_t PGENB : 1;     /*!< (@ 0x00000000) Clear Program enable                                       */
      __IOM uint32_t CSB : 1;       /*!< (@ 0x00000001) Clear Chip Enable                                          */
      __IM uint32_t RESERVED1 : 1;  /*!< (@ 0x00000002) reserved1                                                  */
      __IOM uint32_t LOAD : 1;      /*!< (@ 0x00000003) Clear Load enable                                          */
      __IM uint32_t RESERVED2 : 12; /*!< (@ 0x00000004) reserved2                                                  */
      __IM uint32_t RESERVED3 : 16; /*!< (@ 0x00000010) reserved3                                                  */
    } EFUSE_DA_CTRL_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t EFUSE_CTRL_REG; /*!< (@ 0x0000000C) Control Register                                           */

    struct {
      __IOM uint32_t EFUSE_ENABLE : 1;             /*!< (@ 0x00000000) This bit specifies whether the EFUSE module is
                                                                    enabled or not                                             */
      __IOM uint32_t EFUSE_DIRECT_PATH_ENABLE : 1; /*!< (@ 0x00000001) This bit specifies whether the EFUSE direct path
                                                                    is enabled or not for direct accessing
                                                                    of the EFUSE pins                                          */
      __IOM uint32_t
        ENABLE_EFUSE_WRITE : 1;     /*!< (@ 0x00000002) Controls the switch on VDDIQ for eFuse read/write.         */
      __IM uint32_t RESERVED1 : 13; /*!< (@ 0x00000003) reserved1                                                  */
      __IM uint32_t RESERVED2 : 16; /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_CTRL_REG_b;
  };

  union {
    __IOM uint32_t
      EFUSE_READ_ADDR_REG; /*!< (@ 0x00000010) Read address Register                                      */

    struct {
      __IOM uint32_t READ_ADDR_BITS : 13; /*!< (@ 0x00000000) These bits specifies the address from which read
                                                                    operation has to be performed                              */
      __IM uint32_t RESERVED1 : 2;  /*!< (@ 0x0000000D) reserved1                                                  */
      __OM uint32_t DO_READ : 1;    /*!< (@ 0x0000000F) Enables read FSM after EFUSE is enabled                    */
      __IM uint32_t RESERVED2 : 16; /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_READ_ADDR_REG_b;
  };

  union {
    __IOM uint32_t
      EFUSE_READ_DATA_REG; /*!< (@ 0x00000014) Read address Register                                      */

    struct {
      __IOM uint32_t READ_DATA_BITS : 8; /*!< (@ 0x00000000) These bits specifies the data bits that are read
                                                                    from a given address specified in the EFUSE_READ_ADDRESS_
                                                                    EGISTER bits 8:0                                           */
      __IM uint32_t RESERVED1 : 7;     /*!< (@ 0x00000008) reserved1                                                  */
      __IM uint32_t READ_FSM_DONE : 1; /*!< (@ 0x0000000F) Indicates read fsm is done. After this read data
                                                                    is available in EFUSE_READ_DATA_REGISTER
                                                                    bits 7:0                                                   */
      __IM uint32_t RESERVED2 : 16;    /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_READ_DATA_REG_b;
  };

  union {
    __IM uint32_t EFUSE_STATUS_REG; /*!< (@ 0x00000018) Read address Register                                      */

    struct {
      __IM uint32_t EFUSE_ENABLED : 1; /*!< (@ 0x00000000) This bit specifies whether the EFUSE is enabled
                                                                    or not                                                     */
      __IM uint32_t RESERVED1 : 1;     /*!< (@ 0x00000001) reserved1                                                  */
      __IM uint32_t EFUSE_DOUT_SYNC : 8; /*!< (@ 0x00000002) This bit specifies the 8-bit data read out from
                                                                    the EFUSE macro. This is synchronized with
                                                                    pclk                                                       */
      __IM uint32_t RESERVED2 : 6;  /*!< (@ 0x0000000A) reserved2                                                  */
      __IM uint32_t RESERVED3 : 16; /*!< (@ 0x00000010) reserved3                                                  */
    } EFUSE_STATUS_REG_b;
  };

  union {
    __IOM uint32_t
      EFUSE_RD_TMNG_PARAM_REG; /*!< (@ 0x0000001C) none                                                       */

    struct {
      __IOM uint32_t TSUR_CS : 4;   /*!< (@ 0x00000000) CSB to STROBE setup time into read mode                    */
      __IOM uint32_t TSQ : 4;       /*!< (@ 0x00000004) Q7-Q0 access time from STROBE rising edge                  */
      __IOM uint32_t THRA : 4;      /*!< (@ 0x00000008) for 32x8 macro: A4 A0 to STROBE hold time into
                                                                    Read mode 5122x8 macro: A8 A0 to STROBE
                                                                    hold time into Read mode                                   */
      __IM uint32_t RESERVED1 : 4;  /*!< (@ 0x0000000C) reserved1                                                  */
      __IM uint32_t RESERVED2 : 16; /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_RD_TMNG_PARAM_REG_b;
  };
  __IM uint32_t RESERVED;

  union {
    __IOM uint32_t
      EFUSE_MEM_MAP_LENGTH_REG; /*!< (@ 0x00000024) none                                                       */

    struct {
      __IOM uint32_t
        EFUSE_MEM_MAP_LEN : 1;      /*!< (@ 0x00000000) 0: 8 bit read 1: 16 bit read                               */
      __IM uint32_t RESERVED1 : 15; /*!< (@ 0x00000001) reserved1                                                  */
      __IM uint32_t RESERVED2 : 16; /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_MEM_MAP_LENGTH_REG_b;
  };

  union {
    __IOM uint32_t
      EFUSE_READ_BLOCK_STARTING_LOCATION; /*!< (@ 0x00000028) Starting address from which the read has to be
                                                                    blocked. Once the end address is written,
                                                                    it cannot be changed till power on reset
                                                                    is given                                                   */

    struct {
      __IOM uint32_t
        EFUSE_READ_BLOCK_STARTING_LOCATION : 16; /*!< (@ 0x00000000) Starting address from which the read has to be
                                                                    blocked. Once the end address is written,
                                                                    it cannot be changed till power on reset
                                                                    is given.                                                  */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } EFUSE_READ_BLOCK_STARTING_LOCATION_b;
  };

  union {
    __IOM uint32_t EFUSE_READ_BLOCK_END_LOCATION; /*!< (@ 0x0000002C) Starting address from which the read has to be
                                                                    blocked. Once the end address is written,
                                                                    it cannot be changed till power on reset
                                                                    is given                                                   */

    struct {
      __IOM uint32_t
        EFUSE_READ_BLOCK_END_LOCATION : 16; /*!< (@ 0x00000000) End address till which the read has to be blocked.
                                                                    Once the end address is written , it cannot
                                                                    be changed till power on reset is given.                   */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reserved1                                                  */
    } EFUSE_READ_BLOCK_END_LOCATION_b;
  };

  union {
    __IOM uint32_t EFUSE_READ_BLOCK_ENABLE_REG; /*!< (@ 0x00000030) The Transmit Poll Demand register enables the
                                                                    Transmit DMA to check whether or not the
                                                                    current descriptor is owned by DMA                         */

    struct {
      __IOM uint32_t
        efuse_read_block_enable : 1; /*!< (@ 0x00000000) Enable for blocking the read access from a programmable
                                                                    memory location                                            */
      __IM uint32_t RESERVED1 : 15;  /*!< (@ 0x00000001) reserved1                                                  */
      __IM uint32_t RESERVED2 : 16;  /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_READ_BLOCK_ENABLE_REG_b;
  };

  union {
    __IOM uint32_t
      EFUSE_DA_CLR_STROBE_REG; /*!< (@ 0x00000034) none                                                       */

    struct {
      __IOM uint32_t EFUSE_STROBE_CLR_CNT : 9; /*!< (@ 0x00000000) Strobe signal Clear count in direct access mode.
                                                                    value depends on APB clock frequency of
                                                                    eFuse controller                                           */
      __IOM uint32_t
        EFUSE_STROBE_ENABLE : 1;    /*!< (@ 0x00000009) none                                                       */
      __IM uint32_t RESERVED1 : 6;  /*!< (@ 0x0000000A) reserved1                                                  */
      __IM uint32_t RESERVED2 : 16; /*!< (@ 0x00000010) reserved2                                                  */
    } EFUSE_DA_CLR_STROBE_REG_b;
  };
} EFUSE_Type;

/* =========================================================================================================================== */
/* ================                                            CCI                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief CCI module helps external memories and peripherals to communicate with internal AHB bus with less number of pins (CCI)
  */

typedef struct { /*!< (@ 0x46170000) CCI Structure                                              */

  union {
    __IOM uint32_t CCI_CONTROL; /*!< (@ 0x00000000) CCI control register                                       */

    struct {
      __IOM uint32_t NUM_SLAVES : 2; /*!< (@ 0x00000000) Indicates the number of slaves                             */
      __IOM uint32_t
        ENABLED_SLAVES : 3;     /*!< (@ 0x00000002) Indicates Slaves enable.                                   */
      __IOM uint32_t EBT_S : 1; /*!< (@ 0x00000005) Support for Early Burst Termination                        */
      __IOM uint32_t ADDR_WIDTH_CONFIG : 2; /*!< (@ 0x00000006) address width configuration of AHB slave during
                                                                    address phase                                              */
      __IOM uint32_t
        TRANSLATE_ENABLE : 1;       /*!< (@ 0x00000008) translation enable                                         */
      __IOM uint32_t MODE : 3;      /*!< (@ 0x00000009) This bit represents mode of the interface                  */
      __IOM uint32_t RESERVED1 : 1; /*!< (@ 0x0000000C) reserved1                                                  */
      __IOM uint32_t
        SLAVE_PRIORITY : 3; /*!< (@ 0x0000000D) This bits will represents priority of the slaves           */
      __IOM uint32_t
        TIME_OUT_PRG : 10;          /*!< (@ 0x00000010) configurable time out value for slave response.            */
      __IOM uint32_t RESERVED2 : 3; /*!< (@ 0x0000001A) reserved2                                                  */
      __IOM uint32_t
        DISABLE_TIME_OUT_FOR_DATA_ACCESS : 1; /*!< (@ 0x0000001D) configurable time out value for slave response.  */
      __IOM uint32_t
        SELECT_TIME_OUT_INTR_OR_MSG_INTR : 1; /*!< (@ 0x0000001E) configurable time out value for slave response.  */
      __IOM uint32_t
        CCI_CTRL_ENABLE : 1; /*!< (@ 0x0000001F) configurable time out value for slave response.            */
    } CCI_CONTROL_b;
  };

  union {
    __IOM uint32_t CCI_LSB_A_S1; /*!< (@ 0x00000004) Lower Address of slave 0 supported                         */

    struct {
      __IOM uint32_t LOWER_ADDRESS : 32; /*!< (@ 0x00000000) Lower Address of slave 0 supported. Make sure
                                                                    that slave0 is enabled.                                    */
    } CCI_LSB_A_S1_b;
  };

  union {
    __IOM uint32_t CCI_LSB_A_S2; /*!< (@ 0x00000008) LOWER Address of slave 0 supported                         */

    struct {
      __IOM uint32_t LOWER_ADDRESS : 32; /*!< (@ 0x00000000) Lower Address of slave 1 supported. Make sure
                                                                    that slave1 is enabled.                                    */
    } CCI_LSB_A_S2_b;
  };
  __IM uint32_t RESERVED;

  union {
    __IOM uint32_t CCI_MSB_A_S1; /*!< (@ 0x00000010) upper Address of slave 0 supported                         */

    struct {
      __IOM uint32_t HIGHER_ADDRESS : 32; /*!< (@ 0x00000000) Higher Address of slave 0 supported. Make sure
                                                                    that slave0 is enabled.                                    */
    } CCI_MSB_A_S1_b;
  };

  union {
    __IOM uint32_t CCI_MSB_A_S2; /*!< (@ 0x00000014) UPPER Address of slave 0 supported                         */

    struct {
      __IOM uint32_t HIGHER_ADDRESS : 32; /*!< (@ 0x00000000) Higher Address of slave 1 supported. Make sure
                                                                    that slave1 is enabled.                                    */
    } CCI_MSB_A_S2_b;
  };
  __IM uint32_t RESERVED1[4];

  union {
    __IOM uint32_t
      CCI_MODE_INTR_STATUS; /*!< (@ 0x00000028) Interrupt Status                                           */

    struct {
      __IOM uint32_t RESERVED1 : 3;  /*!< (@ 0x00000000) Reserved1                                                  */
      __IOM uint32_t INTR_CLEAR : 2; /*!< (@ 0x00000003) By setting this bits will clear the interrupt
                                                                    status                                                     */
      __IOM uint32_t RESERVED2 : 6;  /*!< (@ 0x00000005) Reserved2                                                  */
      __IM uint32_t INTR_STATUS : 2; /*!< (@ 0x0000000B) These bits will represents the status of the
                                                                    interrupt in read mode                                     */
      __IOM uint32_t RESERVED3 : 19; /*!< (@ 0x0000000D) RESER                                                      */
    } CCI_MODE_INTR_STATUS_b;
  };
  __IM uint32_t RESERVED2[117];

  union {
    __IOM uint32_t
      CCI_FIFO_THRESHOLD_REG; /*!< (@ 0x00000200) CCI fifo threshold                                         */

    struct {
      __IOM uint32_t
        FIFO_AFULL_THRESHOLD : 5; /*!< (@ 0x00000000) ALMOST full threshold                                      */
      __IOM uint32_t
        FIFO_AEMPTY_THRESHOLD : 5;   /*!< (@ 0x00000005) ALMOST empty threshold                                     */
      __IOM uint32_t RESERVED1 : 22; /*!< (@ 0x0000000A) RESERVED1                                                  */
    } CCI_FIFO_THRESHOLD_REG_b;
  };

  union {
    __IOM uint32_t CCI_TRANS_ADDRESS; /*!< (@ 0x00000204) cci trans address                                          */

    struct {
      __IOM uint32_t
        TRANSLATION_ADDRESS_VALID : 1; /*!< (@ 0x00000000) Translation is enabled or not                           */
      __IOM uint32_t
        TRANSLATION_ADDRESS : 31; /*!< (@ 0x00000001) Address offset for translation address                     */
    } CCI_TRANS_ADDRESS_b;
  };

  union {
    __IOM uint32_t CCI_PREFETCH_CTRL; /*!< (@ 0x00000208) CCI prefetch control register                              */

    struct {
      __IOM uint32_t
        CCI_PREFETCH_EN : 1; /*!< (@ 0x00000000) cci pre-fetch enables on AHB read operation.               */
      __IOM uint32_t
        CCI_2X_CLK_ENABLE_FOR_DDR_MODE : 1; /*!< (@ 0x00000001) It is an enable for CCI 2x clock in DDR mode       */
      __IOM uint32_t RESERVED1 : 30; /*!< (@ 0x00000002) Reserved1                                                  */
    } CCI_PREFETCH_CTRL_b;
  };
} CCI_Type; /*!< Size = 524 (0x20c)                                                        */

/* ================================================================================ */
/* ================                      I2S0                      ================ */
/* ================================================================================ */

/**
 * @brief I2S(Inter-IC Sound) is transferring two-channel digital audio data from one IC device to another (I2S0)
 */
typedef struct {
  union {
    union {
      __IM uint32_t I2S_LRBR; /*!< (@ 0x00000000) Left Receive Buffer Register                               */

      struct {
        __IM uint32_t LRBR : 24;     /*!< (@ 0x00000000) Data received serially from the received channel
                                                                    input                                                      */
        __IM uint32_t RESERVED1 : 8; /*!< (@ 0x00000018) Reserved for future use                                    */
      } I2S_LRBR_b;
    };

    union {
      __OM uint32_t I2S_LTHR; /*!< (@ 0x00000000) Left Receive Buffer Register                               */

      struct {
        __OM uint32_t LTHR : 24;     /*!< (@ 0x00000000) The Left Stereo Data to be transmitted serially
                                                                    from the Transmitted channel output                        */
        __OM uint32_t RESERVED1 : 8; /*!< (@ 0x00000018) Reserved for future use                                    */
      } I2S_LTHR_b;
    };
  };

  union {
    union {
      __IM uint32_t I2S_RRBR; /*!< (@ 0x00000004) Right Receive Buffer Register                              */

      struct {
        __IM uint32_t RRBR : 24;     /*!< (@ 0x00000000) The Right Stereo Data received serially from
                                                                    the received channel input through this
                                                                    register                                                   */
        __IM uint32_t RESERVED1 : 8; /*!< (@ 0x00000018) Reserved for future use                                    */
      } I2S_RRBR_b;
    };

    union {
      __OM uint32_t I2S_RTHR; /*!< (@ 0x00000004) Right Transmit Holding Register                            */

      struct {
        __OM uint32_t RTHR : 24;     /*!< (@ 0x00000000) The Right Stereo Data to be transmitted serially
                                                                    from the Transmit channel output written
                                                                    through this register                                      */
        __OM uint32_t RESERVED1 : 8; /*!< (@ 0x00000018) Reserved for future use                                    */
      } I2S_RTHR_b;
    };
  };

  union {
    __IOM uint32_t I2S_RER; /*!< (@ 0x00000008) Receive Enable Register                                    */

    struct {
      __IOM uint32_t RXCHEN : 1;     /*!< (@ 0x00000000) This Bit enables/disables a receive channel independently
                                                                    of all other channels                                      */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_RER_b;
  };

  union {
    __IOM uint32_t I2S_TER; /*!< (@ 0x0000000C) Transmit Enable Register                                   */

    struct {
      __IOM uint32_t TXCHEN : 1;     /*!< (@ 0x00000000) This Bit enables/disables a transmit channel
                                                                    independently of all other channels                        */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_TER_b;
  };

  union {
    __IOM uint32_t I2S_RCR; /*!< (@ 0x00000010) Receive Configuration Register                             */

    struct {
      __IOM uint32_t WLEN : 3;       /*!< (@ 0x00000000) This Bits are used to program the desired data
                                                                    resolution of the receiver and enables
                                                                    LSB of the incoming left or right word                     */
      __IOM uint32_t RESERVED1 : 29; /*!< (@ 0x00000003) Reserved for future use                                    */
    } I2S_RCR_b;
  };

  union {
    __IOM uint32_t I2S_TCR; /*!< (@ 0x00000014) Transmit Configuration Register                            */

    struct {
      __IOM uint32_t WLEN : 3;       /*!< (@ 0x00000000) This Bits are used to program the desired data
                                                                    resolution of the transmitter and ensure
                                                                    that MSB of the data is transmitted first.                 */
      __IOM uint32_t RESERVED1 : 29; /*!< (@ 0x00000003) Reserved for future use                                    */
    } I2S_TCR_b;
  };

  union {
    __IM uint32_t I2S_ISR; /*!< (@ 0x00000018) Interrupt Status Register                                  */

    struct {
      __IM uint32_t RXDA : 1;       /*!< (@ 0x00000000) Receive Data Available                                     */
      __IM uint32_t RXFO : 1;       /*!< (@ 0x00000001) Receive Data FIFO                                          */
      __IM uint32_t RESERVED1 : 2;  /*!< (@ 0x00000002) Reserved for future use                                    */
      __IM uint32_t TXFE : 1;       /*!< (@ 0x00000004) Transmit FIFO Empty                                        */
      __IM uint32_t TXFO : 1;       /*!< (@ 0x00000005) Transmit FIFO                                              */
      __IM uint32_t RESERVED2 : 26; /*!< (@ 0x00000006) Reserved for future use                                    */
    } I2S_ISR_b;
  };

  union {
    __IOM uint32_t I2S_IMR; /*!< (@ 0x0000001C) Interrupt Mask Register                                    */

    struct {
      __IOM uint32_t RXDAM : 1;      /*!< (@ 0x00000000) RX Data Available Mask Interrupt                           */
      __IOM uint32_t RXFOM : 1;      /*!< (@ 0x00000001) RX FIFO Overrun Mask Interrupt                             */
      __IOM uint32_t RESERVED1 : 2;  /*!< (@ 0x00000002) Reserved for future use                                    */
      __IOM uint32_t TXFEM : 1;      /*!< (@ 0x00000004) TX FIFO Empty Interrupt                                    */
      __IOM uint32_t TXFOM : 1;      /*!< (@ 0x00000005) TX FIFO Overrun Interrupt                                  */
      __IOM uint32_t RESERVED2 : 26; /*!< (@ 0x00000006) Reserved for future use                                    */
    } I2S_IMR_b;
  };

  union {
    __IM uint32_t I2S_ROR; /*!< (@ 0x00000020) Receive Overrun Register                                   */

    struct {
      __IM uint32_t RXCHO : 1;      /*!< (@ 0x00000000) Read this bit to clear the RX FIFO data overrun
                                                                    interrupt                                                  */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_ROR_b;
  };

  union {
    __IM uint32_t I2S_TOR; /*!< (@ 0x00000024) Transmit Overrun Register                                  */

    struct {
      __IM uint32_t TXCHO : 1;      /*!< (@ 0x00000000) Read this bit to clear the TX FIFO data overrun
                                                                    interrupt                                                  */
      __IM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_TOR_b;
  };

  union {
    __IOM uint32_t I2S_RFCR; /*!< (@ 0x00000028) Receive FIFO Configuration Register0                       */

    struct {
      __IOM uint32_t RXCHDT : 4;     /*!< (@ 0x00000000) This bits program the trigger level in the RX
                                                                    FIFO at which the data available interrupt
                                                                    is generated                                               */
      __IOM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) Reserved for future use                                    */
    } I2S_RFCR_b;
  };

  union {
    __IOM uint32_t I2S_TXFCR; /*!< (@ 0x0000002C) Transmit FIFO Configuration Register                       */

    struct {
      __IOM uint32_t TXCHET : 4;    /*!< (@ 0x00000000) This bits program the trigger level in the TX
                                                                    FIFO at which the Empty Threshold Reached
                                                                    interrupt is generated                                     */
      __IM uint32_t RESERVED1 : 28; /*!< (@ 0x00000004) Reserved for future use                                    */
    } I2S_TXFCR_b;
  };

  union {
    __OM uint32_t I2S_RFF; /*!< (@ 0x00000030) Receive FIFO Flush                                         */

    struct {
      __OM uint32_t RXCHFR : 1;     /*!< (@ 0x00000000) Writing a 1 to this register flushes an individual
                                                                    RX FIFO RX channel or block must be disable
                                                                    prior to writing to this bit                               */
      __OM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_RFF_b;
  };

  union {
    __OM uint32_t I2S_TFF; /*!< (@ 0x00000034) Transmit FIFO Flush                                        */

    struct {
      __OM uint32_t TXCHFR : 1;     /*!< (@ 0x00000000) Writing a 1 to this register flushes an individual
                                                                    TX FIFO TX channel or block must be disable
                                                                    prior to writing to this bit                               */
      __OM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_TFF_b;
  };
  __IM uint32_t RSVD0; /*!< (@ 0x00000038) none                                                       */
  __IM uint32_t RSVD1; /*!< (@ 0x0000003C) none                                                       */
} I2S0_CHANNEL_CONFIG_Type;

typedef struct { /*!< (@ 0x47050000) I2S0 Structure                                             */

  union {
    __IOM uint32_t I2S_IER; /*!< (@ 0x00000000) I2S Enable Register                                        */

    struct {
      __IOM uint32_t IEN : 1;        /*!< (@ 0x00000000) Inter Block Enable                                         */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_IER_b;
  };

  union {
    __IOM uint32_t I2S_IRER; /*!< (@ 0x00000004) I2S Receiver Block Enable Register                         */

    struct {
      __IOM uint32_t RXEN : 1;       /*!< (@ 0x00000000) Receive Block Enable, Bit Overrides any Individual
                                                                    Receive Channel Enables                                    */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_IRER_b;
  };

  union {
    __IOM uint32_t I2S_ITER; /*!< (@ 0x00000008) Transmitter Block Enable                                   */

    struct {
      __IOM uint32_t TXEN : 1;       /*!< (@ 0x00000000) Transmitter Block Enable, Bit Overrides any Individual
                                                                    Transmit Channel Enables                                   */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_ITER_b;
  };

  union {
    __IOM uint32_t I2S_CER; /*!< (@ 0x0000000C) Clock Enable Register                                      */

    struct {
      __IOM uint32_t CLKEN : 1;      /*!< (@ 0x00000000) Clock generation enable/disable                            */
      __IOM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_CER_b;
  };

  union {
    __IOM uint32_t I2S_CCR; /*!< (@ 0x00000010) Clock Configuration Register                               */

    struct {
      __IOM uint32_t SCLKG : 3;      /*!< (@ 0x00000000) These bits are used to program the gating of
                                                                    sclk                                                       */
      __IOM uint32_t WSS : 2;        /*!< (@ 0x00000003) These bits are used to program the number of
                                                                    sclk cycles                                                */
      __IOM uint32_t RESERVED1 : 27; /*!< (@ 0x00000005) Reserved for future use                                    */
    } I2S_CCR_b;
  };

  union {
    __OM uint32_t I2S_RXFFR; /*!< (@ 0x00000014) Receiver Block FIFO Reset Register                         */

    struct {
      __OM uint32_t RXFFR : 1;      /*!< (@ 0x00000000) Writing a 1 To This Register Flushes All The
                                                                    RX FIFO's Receiver Block Must be Disable
                                                                    Prior to Writing This Bit                                  */
      __OM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_RXFFR_b;
  };

  union {
    __OM uint32_t I2S_TXFFR; /*!< (@ 0x00000018) Transmitter Block FIFO Reset Register                      */

    struct {
      __OM uint32_t TXFFR : 1;      /*!< (@ 0x00000000) Writing a 1 To This Register Flushes All The
                                                                    RX FIFO's Receiver Block Must be Disable
                                                                    Prior to Writing This Bit                                  */
      __OM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_TXFFR_b;
  };
  __IM uint32_t RESERVED;
  __IOM I2S0_CHANNEL_CONFIG_Type
    CHANNEL_CONFIG[4]; /*!< (@ 0x00000020) [0..3]                                                  */
  __IM uint32_t RESERVED1[40];

  union {
    __IM uint32_t I2S_RXDMA; /*!< (@ 0x000001C0) Receiver Block DMA Register                                */

    struct {
      __IM uint32_t RXDMA : 32; /*!< (@ 0x00000000) Used to cycle repeatedly through the enabled
                                                                    receive channels Reading stereo data pairs                 */
    } I2S_RXDMA_b;
  };

  union {
    __OM uint32_t I2S_RRXDMA; /*!< (@ 0x000001C4) Reset Receiver Block DMA Register                          */

    struct {
      __OM uint32_t RRXDMA : 1;     /*!< (@ 0x00000000) Writing a 1 to this self-clearing register resets
                                                                    the RXDMA register                                         */
      __OM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved for future use                                    */
    } I2S_RRXDMA_b;
  };

  union {
    __OM uint32_t I2S_TXDMA; /*!< (@ 0x000001C8) Transmitter Block DMA Register                             */

    struct {
      __OM uint32_t TXDMA : 32; /*!< (@ 0x00000000) Used to cycle repeatedly through the enabled
                                                                    transmit channels allow to writing of stereo
                                                                    data pairs                                                 */
    } I2S_TXDMA_b;
  };

  union {
    __OM uint32_t I2S_RTXDMA; /*!< (@ 0x000001CC) Reset Transmitter Block DMA Register                       */

    struct {
      __OM uint32_t RTXDMA : 1;     /*!< (@ 0x00000000) Writing a 1 to this self-clearing register resets
                                                                    the TXDMA register                                         */
      __OM uint32_t RESERVED1 : 31; /*!< (@ 0x00000001) Reserved1                                                  */
    } I2S_RTXDMA_b;
  };
  __IM uint32_t RESERVED2[8];

  union {
    __IM uint32_t I2S_COMP_PARAM_2; /*!< (@ 0x000001F0) Component Parameter 2 Register                             */

    struct {
      __IM uint32_t I2S_RX_WORDSIZE_0 : 3; /*!< (@ 0x00000000) On Read returns the value of word size of receiver
                                                                    channel 0                                                  */
      __IM uint32_t I2S_RX_WORDSIZE_1 : 3; /*!< (@ 0x00000003) On Read returns the value of word size of receiver
                                                                    channel 1                                                  */
      __IM uint32_t RESERVED1 : 26; /*!< (@ 0x00000006) Reserved1                                                  */
    } I2S_COMP_PARAM_2_b;
  };

  union {
    __IM uint32_t I2S_COMP_PARAM_1; /*!< (@ 0x000001F4) Component Parameter 1 Register                             */

    struct {
      __IM uint32_t
        APB_DATA_WIDTH : 2; /*!< (@ 0x00000000) Width of APB data bus                                      */
      __IM uint32_t
        I2S_FIFO_DEPTH_GLOBAL : 2; /*!< (@ 0x00000002) Determines FIFO depth for all channels                     */
      __IM uint32_t I2S_FIFO_MODE_EN : 1; /*!< (@ 0x00000004) Determines whether component act as Master or
                                                                    Slave                                                      */
      __IM uint32_t
        I2S_TRANSMITTER_BLOCK : 1; /*!< (@ 0x00000005) Shows the presence of the transmitter block                */
      __IM uint32_t
        I2S_RECEIVER_BLOCK : 1; /*!< (@ 0x00000006) Shows the presence of the receiver block                   */
      __IM uint32_t
        I2S_RX_CHANNELS : 2; /*!< (@ 0x00000007) Returns the number of receiver channels                    */
      __IM uint32_t
        I2S_TX_CHANNELS : 2;       /*!< (@ 0x00000009) Returns the number of transmitter channels                 */
      __IM uint32_t RESERVED1 : 5; /*!< (@ 0x0000000B) Reserved1                                                  */
      __IM uint32_t I2S_TX_WORDSIZE_0 : 3; /*!< (@ 0x00000010) Returns the value of word size of transmitter
                                                                    channel 0                                                  */
      __IM uint32_t I2S_TX_WORDSIZE_1 : 3; /*!< (@ 0x00000013) Returns the value of word size of transmitter
                                                                    channel 1                                                  */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } I2S_COMP_PARAM_1_b;
  };

  union {
    __IM uint32_t
      I2S_COMP_VERSION_REG; /*!< (@ 0x000001F8) Component Version ID                                       */

    struct {
      __IM uint32_t
        I2S_COMP_VERSION : 32; /*!< (@ 0x00000000) Return the component version(1.02)                         */
    } I2S_COMP_VERSION_REG_b;
  };

  union {
    __IM uint32_t I2S_COMP_TYPE_REG; /*!< (@ 0x000001FC) Component Type                                             */

    struct {
      __IM uint32_t
        I2S_COMP_TYPE : 32; /*!< (@ 0x00000000) Return the component type                                  */
    } I2S_COMP_TYPE_REG_b;
  };
} I2S0_Type;
/* ================================================================================ */
/* ================                     IID_AES                    ================ */
/* ================================================================================ */

/**
 * @brief The AES module provides AES encoding and decoding functionality. It can be used in a microprocessor based environment (IID_AES)
 */

typedef struct { /*!< IID_AES Structure                                                     */

  union {
    __IO uint32_t AES_KCR; /*!< AES Key Control register                                              */

    struct {
      __IO uint32_t AES_KEY_CHNG_REQ : 1; /*!< Programming 1 clears the current key and starts a request a
                                                         for a new keyAuto-reverts to 0 as soon as the request is accepted
			 */
      __IO uint32_t AES_KEY_SIZE : 1;     /*!< Size of the AES key 0: 128-bit 1: 256-bit                             */
      uint32_t : 5;
      __IO uint32_t AES_KEY_SRC : 1; /*!< Source of the AES key 0: Interface 1: Register                        */
    } AES_KCR_b;                     /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t AES_MODE_REG; /*!< AES Mode register                                                     */

    struct {
      __IO uint32_t AES_MODE : 8; /*!< The AES Mode register defines which mode of AES is used.              */
    } AES_MODE_REG_b;             /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t AES_ACT_REG; /*!< AES Action register                                                   */

    struct {
      __IO uint32_t AES_ACTION : 2; /*!< The AES Mode register defines which mode of AES is used.              */
    } AES_ACT_REG_b;                /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED[5];

  union {
    __I uint32_t AES_SR_REG; /*!< AES Status register                                                   */

    struct {
      __I uint32_t AES_BUSY : 1; /*!< Indicates that the AES core is processing data                        */
      uint32_t : 1;
      __I uint32_t AES_CLEAR_DONE : 1;  /*!< Indicates that the Clear action is finished                           */
      __I uint32_t AES_KEY_PRESENT : 1; /*!< Indicates that the Clear action is finished                           */
      uint32_t : 1;
      __I uint32_t AES_KEY_REQ : 1;  /*!< Indicates that a key must be provided                                 */
      __I uint32_t AES_DATA_REQ : 1; /*!< Indicates that data must be provided                                  */
      __I uint32_t AES_DATA_AV : 1;  /*!< Indicates that data is available                                      */
    } AES_SR_REG_b;                  /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED1[7];

  union {
    __O uint32_t AES_KEY_REG; /*!< The AES Key register is used to program a key into the AES module.    */

    struct {
      __O uint32_t AES_KEY : 32; /*!< 4 writes of 32 bits make up the 128-bit key for AES, 8 writes
                                                         make up the 256-bit key                                               */
    } AES_KEY_REG_b;             /*!< BitSize                                                               */
  };

  union {
    __O uint32_t AES_DIN_REG; /*!< AES Data In register                                                  */

    struct {
      __O uint32_t AES_DIN : 32; /*!< Data for encoding or decoding, 4 writes of 32 bits make up a
                                                         128-bit data word                                                     */
    } AES_DIN_REG_b;             /*!< BitSize                                                               */
  };

  union {
    __I uint32_t AES_DOUT_REG; /*!< AES Data out register                                                 */

    struct {
      __I uint32_t AES_DOUT : 32; /*!< Result from encoding or decoding, 4 reads of 32 bits make up
                                                         a 128-bit data word                                                   */
    } AES_DOUT_REG_b;             /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED2[36];

  union {
    union {
      __O uint32_t AES_IF_SR_C_REG; /*!< AES Interface Status Clear register                                   */

      struct {
        __O uint32_t IFB_ERROR : 1; /*!< Clears the if_error bit                                               */
      } AES_IF_SR_C_REG_b;          /*!< BitSize                                                               */
    };

    union {
      __I uint32_t AES_IF_SR_REG; /*!< AES Interface Status register                                         */

      struct {
        __I uint32_t IF_ERROR : 1; /*!< Indicates that an interface error has occurred                        */
      } AES_IF_SR_REG_b;           /*!< BitSize                                                               */
    };
  };

  union {
    __IO uint32_t AES_TEST_REG; /*!< AES Test register                                                     */

    struct {
      __IO uint32_t AES_BIST_ENABLE : 1; /*!< Isolates the iid_aes module and runs a BIST                           */
      uint32_t : 3;
      __IO uint32_t AES_BIST_RUNNING : 1; /*!< BIST is in progress or finishing up                                   */
      __IO uint32_t AES_BIST_ACTIVE : 1;  /*!< Indicates that the BIST is running                                    */
      __IO uint32_t AES_BIST_OK : 1;      /*!< Indicates that the BIST has passed                                    */
      __IO uint32_t AES_BIST_ERROR : 1;   /*!< Indicates that the BIST has failed                                    */
    } AES_TEST_REG_b;                     /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED3[6];

  union {
    __I uint32_t AES_VER_REG; /*!< AES Version register                                                  */

    struct {
      __I uint32_t AES_VERSION : 32; /*!< Version of iid_aes                                                    */
    } AES_VER_REG_b;                 /*!< BitSize                                                               */
  };
} IID_AES_Type;

/* ================================================================================ */
/* ================                     IID_QK                     ================ */
/* ================================================================================ */

/**
 * @brief The purpose of Quiddikey is to provide secure key storage without storing the key. (IID_QK)
 */

typedef struct { /*!< IID_QK Structure                                                      */

  union {
    __O uint32_t QK_CR_REG; /*!< Quiddikey Control register.The Quiddikey Control register defines
                                                         which command must be executed next.                                  */

    struct {
      __O uint32_t QK_ZEROIZE : 1; /*!< Begin Zeroize operation and go to Error state                         */
      __O uint32_t QK_ENROLL : 1;  /*!< Begin Enroll operation                                                */
      __O uint32_t QK_START : 1;   /*!< Begin Start operation                                                 */
      __O uint32_t QK_SET_IK : 1;  /*!< Begin Set Intrinsic Key operation                                     */
      __O uint32_t QK_SET_UK : 1;  /*!< Begin Set User Key operation                                          */
      __O uint32_t QK_SET_XK : 1;  /*!< Begin Set External Key operation                                      */
      __O uint32_t QK_GET_KEY : 1; /*!< Begin Get Key operation                                               */
    } QK_CR_REG_b;                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QK_KIDX_REG; /*!< The Quiddikey Key Index register defines the key index for the
                                                         next set_key command                                                  */

    struct {
      __IO uint32_t QK_KEY_INDEX : 4; /*!< Key index for Set Key operations                                      */
    } QK_KIDX_REG_b;                  /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QK_KSZ_REG; /*!< Quiddikey Key Size register                                           */

    struct {
      __IO uint32_t QK_KEY_SIZE : 6; /*!< Key size for Set Key operations                                       */
    } QK_KSZ_REG_b;                  /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t QK_KT_REG; /*!< Quiddikey Key Size register                                           */

    struct {
      __IO uint32_t QK_KEY_TARGET : 1; /*!< Target of reconstructed key                                           */
    } QK_KT_REG_b;                     /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED[4];

  union {
    __I uint32_t QK_SR_REG; /*!< Quiddikey Status register                                             */

    struct {
      __I uint32_t QK_BUSY : 1;   /*!< Indicates that operation is in progress                               */
      __I uint32_t QK_OK : 1;     /*!< Last operation was successful                                         */
      __I uint32_t QK_ERROR : 1;  /*!< Quiddikey is in the Error state and no operations can be performed
			 */
      __I uint32_t QK_XO_AV : 1;  /*!< Next part of XKPD is available                                        */
      __I uint32_t QK_KI_REQ : 1; /*!< Request for next part of key                                          */
      __I uint32_t QK_KO_AV : 1;  /*!< Next part of key is available                                         */
      __I uint32_t QK_CI_REQ : 1; /*!< Request for next part of AC/KC                                        */
      __I uint32_t QK_CO_AV : 1;  /*!< Next part of AC/KC is available                                       */
    } QK_SR_REG_b;                /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED1;

  union {
    __I uint32_t QK_AR_REG; /*!< Quiddikey allow register                                              */

    struct {
      __I uint32_t QK_ALLOW_ENROLL : 1;  /*!< Enroll operation is allowed                                           */
      __I uint32_t QK_ALLOW_START : 1;   /*!< Start operation is allowed                                            */
      __I uint32_t QK_ALLOW_SET_KEY : 1; /*!< Set Key operations are allowed                                        */
      __I uint32_t QK_ALLOW_GET_KEY : 1; /*!< Get Key operation is allowed                                          */
      uint32_t : 3;
      __I uint32_t QK_ALLOW_BIST : 1; /*!< BIST is allowed to be started                                         */
    } QK_AR_REG_b;                    /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED2[5];

  union {
    __O uint32_t QK_KI_REG; /*!< Quiddikey Key Input register                                          */

    struct {
      __IO uint32_t QK_KI : 32; /*!< Key input data                                                        */
    } QK_KI_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __O uint32_t QK_CI_REG; /*!< Quiddikey Code Input register                                         */

    struct {
      __IO uint32_t QK_CI : 32; /*!< AC/KC input data                                                      */
    } QK_CI_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __I uint32_t QK_CO_REG; /*!< Quiddikey Code Output register                                        */

    struct {
      __I uint32_t QK_CO : 32; /*!< AC/KC output data                                                     */
    } QK_CO_REG_b;             /*!< BitSize                                                               */
  };

  union {
    __I uint32_t QK_XO_REG; /*!< Quiddikey XKPD Output register                                        */

    struct {
      __I uint32_t QK_XO : 32; /*!< XKPD output data                                                      */
    } QK_XO_REG_b;             /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED3[4];

  union {
    __I uint32_t QK_KO_IDX_REG; /*!< Quiddikey Key Output Index register                                   */

    struct {
      __I uint32_t qk_ko_index : 4; /*!< Key index for the key that is currently output via the Key Output
                                                         register                                                              */
    } QK_KO_IDX_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __I uint32_t QK_KO_REG; /*!< Quiddikey Code Output register                                        */

    struct {
      __I uint32_t QK_KO : 32; /*!< Key output data                                                       */
    } QK_KO_REG_b;             /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED4[29];

  union {
    union {
      __I uint32_t QK_IF_SR_C_REG; /*!< Quiddikey Interface Status register                                   */

      struct {
        __I uint32_t IF_ERROR : 1; /*!< Clears the if_error bit                                               */
      } QK_IF_SR_C_REG_b;          /*!< BitSize                                                               */
    };

    union {
      __I uint32_t QK_IF_SR_REG; /*!< Quiddikey Interface Status register                                   */

      struct {
        __I uint32_t IF_ERROR : 1; /*!< Indicates that an interface error has occurred                        */
      } QK_IF_SR_REG_b;            /*!< BitSize                                                               */
    };
  };

  union {
    __IO uint32_t QK_TEST_REG; /*!< QK Test register                                                      */

    struct {
      __IO uint32_t QK_BIST_ENABLE : 1; /*!< Isolates the iid_quiddikey module and runs a BIST                     */
      uint32_t : 3;
      __IO uint32_t QK_BIST_RUNNING : 1; /*!< BIST is in progress or finishing up                                   */
      __IO uint32_t QK_BIST_ACTIVE : 1;  /*!< Indicates that the BIST is running                                    */
      __IO uint32_t QK_BIST_OK : 1;      /*!< Indicates that the BIST has passed                                    */
      __IO uint32_t QK_BIST_ERROR : 1;   /*!< Indicates that the BIST has failed                                    */
    } QK_TEST_REG_b;                     /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED5[6];

  union {
    __I uint32_t QK_VER_REG; /*!< QK Version register                                                   */

    struct {
      __I uint32_t QK_VERSION : 32; /*!< Version of iid_qk                                                     */
    } QK_VER_REG_b;                 /*!< BitSize                                                               */
  };
} IID_QK_Type;

/* ================================================================================ */
/* ================                    IID_RPINE                   ================ */
/* ================================================================================ */

/**
 * @brief none (IID_RPINE)
 */

typedef struct { /*!< IID_RPINE Structure                                                   */

  union {
    __IO uint32_t IID_BIST_CTRL_REG; /*!< Quiddikey Control register.The Quiddikey Control register defines
                                                         which command must be executed next.                                  */

    struct {
      __IO uint32_t QK_BIST_ENABLE : 1;  /*!< none                                                                  */
      __IO uint32_t AES_BIST_ENABLE : 1; /*!< none                                                                  */
      __IO uint32_t KH_BIST_ENABLE : 1;  /*!< none                                                                  */
    } IID_BIST_CTRL_REG_b;               /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t IID_BIST_STATUS_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t QK_BIST_ACTIVE : 1;   /*!< none                                                                  */
      __IO uint32_t QK_BIST_ERROR : 1;    /*!< Indicates that the BIST has failed                                    */
      __IO uint32_t QK_BIST_OK : 1;       /*!< Indicates that the BIST has passed                                    */
      __IO uint32_t QK_BIST_RUNNING : 1;  /*!< Indicates that the BIST is running                                    */
      __IO uint32_t AES_BIST_ACTIVE : 1;  /*!< none                                                                  */
      __IO uint32_t AES_BIST_ERROR : 1;   /*!< none                                                                  */
      __IO uint32_t AES_BIST_OK : 1;      /*!< Indicates that the BIST has passed                                    */
      __IO uint32_t AES_BIST_RUNNING : 1; /*!< Indicates that the BIST is running                                    */
      __IO uint32_t KH_BIST_STATUS : 1;   /*!< none                                                                  */
    } IID_BIST_STATUS_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t IID_CTRL_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t AES_MAX_KEY_SIZE : 1; /*!< 1 256 bit key, 0 128 bit key                                          */
      __IO uint32_t SOURCE_KEY_KH : 1;    /*!< When set KH will source the key to AES engine. When this is
                                                         not QK key output is connected to AES key input                       */
      __IO uint32_t LATCH_KEY_KH : 1;     /*!< When set KH will latch the key given by QK. When this is not
                                                         QK key output is connected to AES key input                           */
      __IO uint32_t KH_RESET_N : 1;       /*!< 0 KH will be in reset 1 Out of reset                                  */
      __IO uint32_t KH_KEY_SIZE : 1;      /*!< 0 128 bit key 1 256 bit key This is used by KH                        */
      __IO uint32_t KH_CLOCK_RATIO : 3;   /*!< Indicates the division factor to be used for generating kh_clk.
			 */
    } IID_CTRL_REG_b;                     /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t WKE_CTRL_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t ENABLE_WKE : 1;    /*!< When WKE will be enabled. This is a self clearing bit. Once
                                                         enabled WKE can not be disabled till process is done                  */
      __IO uint32_t WKE_KEY_SIZE : 1;  /*!< 0 128 bit size 1 256 bit size                                         */
      __IO uint32_t WKE_FLUSH : 1;     /*!< When set, WKE will flush out the data from AES. When WEK is
                                                         active, firmware reads to AES engine are masked. This gets cleared
                                                          once four dwords are read from AES                                   */
      __IO uint32_t WKE_COMPARE : 1;   /*!< When set, WKE will compare the data from AES engine with the
                                                         data provided by firmware                                             */
      __IO uint32_t WKE_SET_KEY : 1;   /*!< This has to be set after key available from AES                       */
      __IO uint32_t KEY_CODE_DONE : 1; /*!< This has to be set after reading key code                             */
    } WKE_CTRL_REG_b;                  /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED;

  union {
    __IO uint32_t IID_AES_CTRL_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t KEY_REQ_IN_DMA_PATH : 1;         /*!< Include key req in dma path. With this KEY Also can be loaded
                                                         using DMA.                                                            */
      __IO uint32_t AES_MAX_KEY_SIZE_FRM_REG : 1;    /*!< This is valid only when aes_max_key_size_frm_reg_en is set.
			 */
      __IO uint32_t AES_MAX_KEY_SIZE_FRM_REG_EN : 1; /*!< When set, WKE will flush out the data from AES. When WEK is
                                                         active, firmware reads to AES engine are masked. This gets cleared
                                                          once four dwords are read from AES                                   */
      __IO uint32_t OTP_KEY_LOADING : 1;             /*!< When set, WKE will compare the data from AES engine with the
                                                         data provided by firmware                                             */
    } IID_AES_CTRL_REG_b; /*!< BitSize                                                               */
  };

  union {
    __I uint32_t IID_AES_STS_REG; /*!< none                                                                  */

    struct {
      __I uint32_t DIN_FIFO_FULL : 1;   /*!< Input data fifo full indication                                       */
      __I uint32_t DOUT_FIFO_EMPTY : 1; /*!< Output data fifo empty indication                                     */
    } IID_AES_STS_REG_b;                /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED1;

  union {
    __IO uint32_t WKE_STATUS_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t WKE_ACTIVE : 1; /*!< Will be high when WKE is active                                       */
      __IO uint32_t
        WKE_KEY_FEED_IN_PROGRESS : 1; /*!< Will be high when WKE is feeding key to AES engine                    */
      __IO uint32_t
        WKE_FLUSH_IN_PROGRESS : 1; /*!< Will be high when WKE flushing out the data from AES                  */
      __IO uint32_t
        WKE_COMPARE_IN_PROGRESS : 1; /*!< Will be high when WKE is comparing the data from AES                  */
      __IO uint32_t
        WKE_SET_KEY_IN_PROGRESS : 1;   /*!< Will be high when WKE is doing set key operation with QK              */
      __IO uint32_t WKE_KEY_READY : 1; /*!< Firmware has to load the authentication, which will be compared
                                                         with AES output, when this bit is low                                 */
      __IO uint32_t WKE_CMP_DATA_READY : 1; /*!< Firmware has to load the authentication, which will be compared
                                                         with AES output, when this bit is low                                 */
      __IO uint32_t WKE_COMPARE_FAIL : 1;   /*!< This bit will be set when authentication data comparison fails
			 */
    } WKE_STATUS_REG_b; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED2;
  __IO uint32_t WKE_DATA_REG; /*!< none                                                                  */
} IID_RPINE_Type;

/* ================================================================================ */
/* ================                      SCT0                      ================ */
/* ================================================================================ */

/**
 * @brief Configurable timer is used in counting clocks, events and states with reference clock external clock and system clock  (SCT0)
 */

typedef struct { /*!< (@ 0x45060000) CT0 Structure                                              */

  union {
    __IOM uint32_t
      CT_GEN_CTRL_SET_REG; /*!< (@ 0x00000000) General control set register                               */

    struct {
      __IOM uint32_t COUNTER_IN_32_BIT_MODE : 1; /*!< (@ 0x00000000) Counter_1 and Counter_0 will be merged and used
                                                                    as a single 32 bit counter                                 */
      __IOM uint32_t
        SOFT_RESET_COUNTER_0_FRM_REG : 1; /*!< (@ 0x00000001) This is applied to 32 bits of counter only when
                                                                    the counter is in 32 bit counter mode otherwise
                                                                    this will be applied to only lower 16 bits
                                                                    of counter                                                 */
      __IOM uint32_t
        PERIODIC_EN_COUNTER_0_FRM_REG : 1; /*!< (@ 0x00000002) This is applied to 32 bits of counter only when
                                                                    the counter is in 32 bit counter mode otherwise
                                                                    this will be applied to only lower 16 bits
                                                                    of counter                                                 */
      __IOM uint32_t
        COUNTER_0_TRIG_FRM_REG : 1; /*!< (@ 0x00000003) This enables the counter to run/active                     */
      __IOM uint32_t
        COUNTER_0_UP_DOWN : 2; /*!< (@ 0x00000004) This enables the counter to run in up/down/up-down/down-up
                                                                    directions                                                 */
      __IOM uint32_t COUNTER_0_SYNC_TRIG : 1; /*!< (@ 0x00000006) This is applied to 32 bits of counter only when
                                                                    the counter is in 32 bit counter mode otherwise
                                                                    this will be applied to only lower 16 bits
                                                                    of counter. This enables the counter to
                                                                    run/active when sync is found.                             */
      __IOM uint32_t BUF_REG_0_EN : 1; /*!< (@ 0x00000007) Buffer register gets enabled for MATCH REG. MATCH_BUF_REG
                                                                    is always available and whenever this bit
                                                                    is set only, gets copied to MATCH REG.                     */
      __IOM uint32_t RESERVED1 : 9;    /*!< (@ 0x00000008) Reserved1                                                  */
      __IOM uint32_t
        SOFT_RESET_COUNTER_1_FRM_REG : 1; /*!< (@ 0x00000011) This resets the counter on the write                 */
      __IOM uint32_t
        PERIODIC_EN_COUNTER_1_FRM_REG : 1; /*!< (@ 0x00000012) This resets the counter on the write                */
      __IOM uint32_t
        COUNTER_1_TRIG_FRM : 1; /*!< (@ 0x00000013) This enables the counter to run/active                     */
      __IOM uint32_t
        COUNTER_1_UP_DOWN : 2; /*!< (@ 0x00000014) This enables the counter to run in upward direction        */
      __IOM uint32_t COUNTER_1_SYNC_TRIG : 1; /*!< (@ 0x00000016) This is applied to 32 bits of counter only when
                                                                    the counter is in 32 bit counter mode otherwise
                                                                    this will be applied to only lower 16 bits
                                                                    of counter. This enables the counter to
                                                                    run/active when sync is found.                             */
      __IOM uint32_t BUF_REG_1_EN : 1; /*!< (@ 0x00000017) Buffer register gets enabled for MATCH REG. MATCH_BUF_REG
                                                                    is always available and whenever this bit
                                                                    is set only, gets copied to MATCH REG.                     */
      __IOM uint32_t RESERVED2 : 8;    /*!< (@ 0x00000018) Reserved2                                                  */
    } CT_GEN_CTRL_SET_REG_b;
  };

  union {
    __IOM uint32_t
      CT_GEN_CTRL_RESET_REG; /*!< (@ 0x00000004) General control reset register                             */

    struct {
      __IOM uint32_t COUNTER_IN_32_BIT_MODE : 1; /*!< (@ 0x00000000) Counter_1 and Counter_0 will be merged and used
                                                                    as a single 32 bit counter                                 */
      __IM uint32_t RESERVED1 : 1; /*!< (@ 0x00000001) Reserved1                                                  */
      __IOM uint32_t
        PERIODIC_EN_COUNTER_0_FRM_REG : 1; /*!< (@ 0x00000002) This is applied to 32 bits of counter only when
                                                                    the counter is in 32 bit counter mode otherwise
                                                                    this will be applied to only lower 16 bits
                                                                    of counter                                                 */
      __IM uint32_t RESERVED2 : 1; /*!< (@ 0x00000003) Reserved2                                                  */
      __IOM uint32_t
        COUNTER_0_UP_DOWN : 2;         /*!< (@ 0x00000004) This enables the counter to run in up/down/up-down/down-up
                                                                    directions                                                 */
      __IM uint32_t RESERVED3 : 1;     /*!< (@ 0x00000006) Reserved3                                                  */
      __IOM uint32_t BUF_REG_0_EN : 1; /*!< (@ 0x00000007) Buffer register gets enabled for MATCH REG. MATCH_BUF_REG
                                                                    is always available and whenever this bit
                                                                    is set only, gets copied to MATCH REG.                     */
      __IM uint32_t RESERVED4 : 9;     /*!< (@ 0x00000008) Reserved4                                                  */
      __IM uint32_t RESERVED5 : 1;     /*!< (@ 0x00000011) Reserved5                                                  */
      __IOM uint32_t
        PERIODIC_EN_COUNTER_1_FRM_REG : 1; /*!< (@ 0x00000012) This resets the counter on the write                */
      __IM uint32_t RESERVED6 : 1; /*!< (@ 0x00000013) Reserved6                                                  */
      __IOM uint32_t
        COUNTER_1_UP_DOWN : 2;         /*!< (@ 0x00000014) This enables the counter to run in upward direction        */
      __IM uint32_t RESERVED7 : 1;     /*!< (@ 0x00000016) Reserved7                                                  */
      __IOM uint32_t BUF_REG_1_EN : 1; /*!< (@ 0x00000017) Buffer register gets enabled for MATCH REG. MATCH_BUF_REG
                                                                    is always available and whenever this bit
                                                                    is set only, gets copied to MATCH REG.                     */
      __IM uint32_t RESERVED8 : 8;     /*!< (@ 0x00000018) Reserved8                                                  */
    } CT_GEN_CTRL_RESET_REG_b;
  };

  union {
    __IM uint32_t CT_INTR_STS; /*!< (@ 0x00000008) Interrupt status                                           */

    struct {
      __IM uint32_t INTR_0_L : 1;      /*!< (@ 0x00000000) Indicates the FIFO full signal of channel-0                */
      __IM uint32_t FIFO_0_FULL_L : 1; /*!< (@ 0x00000001) Indicates the FIFO full signal of channel-0                */
      __IM uint32_t
        COUNTER_0_IS_ZERO_L : 1; /*!< (@ 0x00000002) Counter 0 hit zero in active mode.                         */
      __IM uint32_t
        COUNTER_0_IS_PEAK_L : 1;       /*!< (@ 0x00000003) Counter 0 hit peak (MATCH) in active mode.                 */
      __IM uint32_t RESERVED1 : 12;    /*!< (@ 0x00000004) Reserved1                                                  */
      __IM uint32_t INTR_1_L : 1;      /*!< (@ 0x00000010) Indicates the FIFO full signal of channel-1                */
      __IM uint32_t FIFO_1_FULL_L : 1; /*!< (@ 0x00000011) Indicates the FIFO full signal of channel-1                */
      __IM uint32_t
        COUNTER_1_IS_ZERO_L : 1; /*!< (@ 0x00000012) Counter 1 hit zero in active mode.                         */
      __IM uint32_t
        COUNTER_1_IS_PEAK_L : 1;    /*!< (@ 0x00000013) Counter 1 hit peak (MATCH) in active mode.                 */
      __IM uint32_t RESERVED2 : 12; /*!< (@ 0x00000014) Reserved2                                                  */
    } CT_INTR_STS_b;
  };

  union {
    __IOM uint32_t CT_INTR_MASK; /*!< (@ 0x0000000C) Interrupts mask                                            */

    struct {
      __IOM uint32_t INTR_0_L : 1; /*!< (@ 0x00000000) Interrupt mask signal.                                     */
      __IOM uint32_t
        FIFO_0_FULL_L : 1; /*!< (@ 0x00000001) Interrupt mask signal.                                     */
      __IOM uint32_t
        COUNTER_0_IS_ZERO_L : 1; /*!< (@ 0x00000002) Interrupt mask signal.                                     */
      __IOM uint32_t
        COUNTER_0_IS_PEAK_L : 1;     /*!< (@ 0x00000003) Interrupt mask signal.                                     */
      __IOM uint32_t RESERVED1 : 12; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t INTR_1_L : 1;   /*!< (@ 0x00000010) Interrupt mask signal.                                     */
      __IOM uint32_t
        FIFO_1_FULL_L : 1; /*!< (@ 0x00000011) Interrupt mask signal.                                     */
      __IOM uint32_t
        COUNTER_1_IS_ZERO_L : 1; /*!< (@ 0x00000012) Interrupt mask signal.                                     */
      __IOM uint32_t
        COUNTER_1_IS_PEAK_L : 1;     /*!< (@ 0x00000013) Interrupt mask signal.                                     */
      __IOM uint32_t RESERVED2 : 12; /*!< (@ 0x00000014) Reserved2                                                  */
    } CT_INTR_MASK_b;
  };

  union {
    __IOM uint32_t CT_INTER_UNMASK; /*!< (@ 0x00000010) Interrupts unmask                                          */

    struct {
      __IOM uint32_t INTR_0_L : 1; /*!< (@ 0x00000000) Interrupt unmask signal.                                   */
      __IOM uint32_t
        FIFO_0_FULL_L : 1; /*!< (@ 0x00000001) Interrupt unmask signal.                                   */
      __IOM uint32_t
        COUNTER_0_IS_ZERO_L : 1; /*!< (@ 0x00000002) Interrupt unmask signal.                                   */
      __IOM uint32_t
        COUNTER_0_IS_PEAK_L : 1;    /*!< (@ 0x00000003) Interrupt unmask signal.                                   */
      __IM uint32_t RESERVED1 : 12; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t INTR_1_L : 1;  /*!< (@ 0x00000010) Interrupt unmask signal.                                   */
      __IOM uint32_t
        FIFO_1_FULL_L : 1; /*!< (@ 0x00000011) Interrupt unmask signal                                    */
      __IOM uint32_t
        COUNTER_1_IS_ZERO_L : 1; /*!< (@ 0x00000012) Interrupt unmask signal.                                   */
      __IOM uint32_t
        COUNTER_1_IS_PEAK_L : 1;    /*!< (@ 0x00000013) Interrupt unmask signal.                                   */
      __IM uint32_t RESERVED2 : 12; /*!< (@ 0x00000014) Reserved2                                                  */
    } CT_INTER_UNMASK_b;
  };

  union {
    __IOM uint32_t CT_INTR_ACK; /*!< (@ 0x00000014) Interrupt clear/ack register                               */

    struct {
      __IOM uint32_t INTR_0_L : 1; /*!< (@ 0x00000000) Interrupt ack signal.                                      */
      __IOM uint32_t
        FIFO_0_FULL_L : 1; /*!< (@ 0x00000001) Interrupt ack signal.                                      */
      __IOM uint32_t
        COUNTER_0_IS_ZERO_L : 1; /*!< (@ 0x00000002) Interrupt ack signal.                                      */
      __IOM uint32_t
        COUNTER_0_IS_PEAK_L : 1;    /*!< (@ 0x00000003) Interrupt ack signal.                                      */
      __IM uint32_t RESERVED1 : 12; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t INTR_1_L : 1;  /*!< (@ 0x00000010) Interrupt ack signal.                                      */
      __IOM uint32_t
        FIFO_1_FULL_L : 1; /*!< (@ 0x00000011) Interrupt ack signal.                                      */
      __IOM uint32_t
        COUNTER_1_IS_ZERO_L : 1; /*!< (@ 0x00000012) Interrupt ack signal.                                      */
      __IOM uint32_t
        COUNTER_1_IS_PEAK_L : 1;     /*!< (@ 0x00000013) Interrupt ack signal.                                      */
      __IOM uint32_t RESERVED2 : 12; /*!< (@ 0x00000014) Reserved2                                                  */
    } CT_INTR_ACK_b;
  };

  union {
    __IOM uint32_t CT_MATCH_REG; /*!< (@ 0x00000018) Match value register                                       */

    struct {
      __IOM uint32_t
        COUNTER_0_MATCH : 16; /*!< (@ 0x00000000) This will be used as lower match                           */
      __IOM uint32_t
        COUNTER_1_MATCH : 16; /*!< (@ 0x00000010) This will be used as upper match                           */
    } CT_MATCH_REG_b;
  };

  union {
    __IOM uint32_t CT_MATCH_BUF_REG; /*!< (@ 0x0000001C) Match Buffer register                                      */

    struct {
      __IOM uint32_t COUNTER_0_MATCH_BUF : 16; /*!< (@ 0x00000000) This gets copied to MATCH register if bug_reg_0_en
                                                                    is set. Copying is done when counter 0
                                                                    is active and hits 0.                                      */
      __IOM uint32_t COUNTER_1_MATCH_BUF : 16; /*!< (@ 0x00000010) This gets copied to MATCH register if bug_reg_1_en
                                                                    is set. Copying is done when counter 1
                                                                    is active and hits 0.                                      */
    } CT_MATCH_BUF_REG_b;
  };

  union {
    __IM uint32_t CT_CAPTURE_REG; /*!< (@ 0x00000020) Capture Register                                           */

    struct {
      __IM uint32_t COUNTER_0_CAPTURE : 16; /*!< (@ 0x00000000) This is a latched value of counter lower part
                                                                    when the selected capture_event occurs                     */
      __IM uint32_t COUNTER_1_CAPTURE : 16; /*!< (@ 0x00000010) This is a latched value of counter upper part
                                                                    when the selected capture_event occurs                     */
    } CT_CAPTURE_REG_b;
  };

  union {
    __IOM uint32_t CT_COUNTER_REG; /*!< (@ 0x00000024) Counter Register                                           */

    struct {
      __IM uint32_t COUNTER0 : 16; /*!< (@ 0x00000000) This holds the value of counter-0                          */
      __IM uint32_t COUNTER1 : 16; /*!< (@ 0x00000010) This holds the value of counter-1                          */
    } CT_COUNTER_REG_b;
  };

  union {
    __IOM uint32_t CT_OCU_CTRL_REG; /*!< (@ 0x00000028) OCU control register                                       */

    struct {
      __IOM uint32_t OUTPUT_IS_OCU_0 : 1;        /*!< (@ 0x00000000) Indicates whether the output is in OCU mode or
                                                                    not for channel-0                                          */
      __IOM uint32_t SYNC_WITH_0 : 3;            /*!< (@ 0x00000001) Indicates whether the other channel is in sync
                                                                    with this channel                                          */
      __IOM uint32_t OCU_0_DMA_MODE : 1;         /*!< (@ 0x00000004) Indicates whether the OCU DMA mode is active
                                                                    or not for channel 0                                       */
      __IOM uint32_t OCU_0_MODE_8_16 : 1;        /*!< (@ 0x00000005) Indicates whether entire 16 bits or only 8-bits
                                                                    of the channel 0 are used in OCU mode                      */
      __IOM uint32_t MAKE_OUTPUT_0_HIGH_SEL : 3; /*!< (@ 0x00000006) Check counter ocus for possibilities. When this
                                                                    is hit output will be made high.                           */
      __IOM uint32_t MAKE_OUTPUT_0_LOW_SEL : 3;  /*!< (@ 0x00000009) Check counter ocus for possibilities. When this
                                                                    is hit output will be made low.                            */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x0000000C) Reserved1                                                  */
      __IOM uint32_t OUTPUT_1_IS_OCU : 1;        /*!< (@ 0x00000010) Indicates whether the output is in OCU mode or
                                                                    not for channel 1                                          */
      __IOM uint32_t SYNC_WITH_1 : 3;            /*!< (@ 0x00000011) Indicates whether the other channel is in sync
                                                                    with this channel                                          */
      __IOM uint32_t OCU_1_DMA_MODE : 1;         /*!< (@ 0x00000014) Indicates whether the OCU DMA mode is active
                                                                    or not for channel 1                                       */
      __IOM uint32_t OCU_1_MODE_8_16_MODE : 1;   /*!< (@ 0x00000015) Indicates whether entire 16 bits or only 8-bits
                                                                    of the channel 1 are used in OCU mode                      */
      __IOM uint32_t MAKE_OUTPUT_1_HIGH_SEL : 3; /*!< (@ 0x00000016) Check counter ocus for possibilities. When this
                                                                    is hit output will be made high.                           */
      __IOM uint32_t MAKE_OUTPUT_1_LOW_SEL : 3;  /*!< (@ 0x00000019) Check counter ocus for possibilities. When this
                                                                    is hit output will be made low.                            */
      __IOM uint32_t RESERVED2 : 4; /*!< (@ 0x0000001C) Reserved2                                                  */
    } CT_OCU_CTRL_REG_b;
  };

  union {
    __IOM uint32_t CT_OCU_COMPARE_REG; /*!< (@ 0x0000002C) OCU Compare Register                                       */

    struct {
      __IOM uint32_t OCU_COMPARE_0_REG : 16; /*!< (@ 0x00000000) Holds the threshold value of present OCU period
                                                                    which denotes the number of clock cycles
                                                                    for which the OCU output should be considered
                                                                    (counter 0)                                                */
      __IOM uint32_t OCU_COMPARE_1_REG : 16; /*!< (@ 0x00000010) Holds the threshold value of present OCU period
                                                                    which denotes the number of clock cycles
                                                                    for which the OCU output should be considered
                                                                    (counter 1)                                                */
    } CT_OCU_COMPARE_REG_b;
  };

  union {
    __IOM uint32_t
      CT_OCU_COMPARE2_REG; /*!< (@ 0x00000030) OCU Compare2 Register                                      */

    struct {
      __IOM uint32_t OCU_COMPARE2_0_REG : 16; /*!< (@ 0x00000000) Holds the threshold value of present OCU period2
                                                                    which denotes the number of clock cycles
                                                                    for which the OCU output should be considered
                                                                    (counter 0)                                                */
      __IOM uint32_t OCU_COMPARE2_1_REG : 16; /*!< (@ 0x00000010) Holds the threshold value of present OCU period2
                                                                    which denotes the number of clock cycles
                                                                    for which the OCU output should be considered
                                                                    (counter 1)                                                */
    } CT_OCU_COMPARE2_REG_b;
  };

  union {
    __IOM uint32_t CT_OCU_SYNC_REG; /*!< (@ 0x00000034) OCU Synchronization Register                               */

    struct {
      __IOM uint32_t OCU_SYNC_CHANNEL0_REG : 16; /*!< (@ 0x00000000) Starting point of channel 0 for synchronization
                                                                    purpose                                                    */
      __IOM uint32_t OCU_SYNC_CHANNEL1_REG : 16; /*!< (@ 0x00000010) Starting point of channel 1 for synchronization
                                                                    purpose                                                    */
    } CT_OCU_SYNC_REG_b;
  };

  union {
    __IOM uint32_t
      CT_OCU_COMPARE_NXT_REG; /*!< (@ 0x00000038) PWM compare next register                                  */

    struct {
      __IOM uint32_t
        OCU_COMPARE_NXT_COUNTER1 : 16; /*!< (@ 0x00000000) OCU output should be high for counter 1                 */
      __IOM uint32_t
        OCU_COMPARE_NXT_COUNTER0 : 16; /*!< (@ 0x00000010) PWM output should be high for counter 0                 */
    } CT_OCU_COMPARE_NXT_REG_b;
  };

  union {
    __IOM uint32_t CT_WFG_CTRL_REG; /*!< (@ 0x0000003C) WFG control register                                       */

    struct {
      __IOM uint32_t
        MAKE_OUTPUT_0_TGL_0_SEL : 3; /*!< (@ 0x00000000) Check the counter ocus possibilities for description
                                                                    for channel 0.                                             */
      __IOM uint32_t
        MAKE_OUTPUT_0_TGL_1_SEL : 3; /*!< (@ 0x00000003) Check the counter ocus possibilities for description
                                                                    for channel 0.                                             */
      __IOM uint32_t RESERVED1 : 2;  /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t WFG_TGL_CNT_0_PEAK : 8; /*!< (@ 0x00000008) WFG mode output toggle count clock for channel
                                                                    0.                                                         */
      __IOM uint32_t
        MAKE_OUTPUT_1_TGL_0_SEL : 3; /*!< (@ 0x00000010) Check the counter ocus possibilities for description
                                                                    for channel 1.                                             */
      __IOM uint32_t
        MAKE_OUTPUT_1_TGL_1_SEL : 3; /*!< (@ 0x00000013) Check the counter ocus possibilities for description
                                                                    for channel 1.                                             */
      __IOM uint32_t RESERVED2 : 2;  /*!< (@ 0x00000016) Reserved2                                                  */
      __IOM uint32_t WFG_TGL_CNT_1_PEAK : 8; /*!< (@ 0x00000018) WFG mode output toggle count clock for channel
                                                                    1                                                          */
    } CT_WFG_CTRL_REG_b;
  };

  union {
    __IOM uint32_t
      CT_OCU_COMPARE2_NXT_REG; /*!< (@ 0x00000040) PWM compare next register                                  */

    struct {
      __IOM uint32_t
        OCU_COMPARE2_NXT_COUNTER0 : 16; /*!< (@ 0x00000000) OCU output should be high for counter 1                */
      __IOM uint32_t
        OCU_COMPARE2_NXT_COUNTER1 : 16; /*!< (@ 0x00000010) PWM output should be high for counter 0                */
    } CT_OCU_COMPARE2_NXT_REG_b;
  };
  __IM uint32_t RESERVED[3];

  union {
    __IOM uint32_t
      CT_START_COUNTER_EVENT_SEL; /*!< (@ 0x00000050) Start counter event select register                        */

    struct {
      __IOM uint32_t START_COUNTER_0_EVENT_SEL : 6; /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    starting the Counter 0 For 32 bit counter
                                                                    mode: Event select for starting counter                    */
      __IOM uint32_t RESERVED1 : 10; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t START_COUNTER_1_EVENT_SEL : 6; /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    starting the Counter 1. For 32 bit counter
                                                                    mode: Invalid. Please refer to events table
                                                                    for description                                            */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } CT_START_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_START_COUNTER_AND_EVENT; /*!< (@ 0x00000054) Start counter AND event register                           */

    struct {
      __IOM uint32_t
        START_COUNTER_0_AND_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: AND expression valids
                                                                    for AND event in start Counter 0 event
                                                                    For 32 bit counter mode AND expression
                                                                    valids for AND event in start counter event                */
      __IOM uint32_t RESERVED1 : 4;    /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        START_COUNTER_0_AND_VLD : 4; /*!< (@ 0x00000008) none                                                      */
      __IM uint32_t RESERVED2 : 4;   /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t START_COUNTER_1_AND_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: AND expression
                                                                    valids for AND event in start counter event
                                                                    For 32 bit counter mode : Invalid                          */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        START_COUNTER_1_AND_VLD : 4; /*!< (@ 0x00000018) none                                                      */
      __IM uint32_t RESERVED4 : 4;   /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_START_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_START_COUNTER_OR_EVENT; /*!< (@ 0x00000058) Start counter OR event register                            */

    struct {
      __IOM uint32_t START_COUNTER_0_OR_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: OR expression valids
                                                                    for OR event in start Counter 0 event For
                                                                    32 bit counter mode OR expression valids
                                                                    for OR event in start counter event                        */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        START_COUNTER_0_OR_VLD : 4; /*!< (@ 0x00000008) none                                                       */
      __IOM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t
        START_COUNTER_1_OR_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: OR expression valids
                                                                    for OR event in start counter event For
                                                                    32 bit counter mode : Invalid.                             */
      __IM uint32_t RESERVED3 : 4;    /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        START_COUNTER_1_OR_VLD : 4; /*!< (@ 0x00000018) none                                                       */
      __IM uint32_t RESERVED4 : 4;  /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_START_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_CONTINUE_COUNTER_EVENT_SEL; /*!< (@ 0x0000005C) Continue counter event select register                    */

    struct {
      __IOM uint32_t
        CONTINUE_COUNTER_0_EVENT_SEL : 6; /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    continuing the Counter 0 For 32 bit counter
                                                                    mode: Event select for continuing counter                  */
      __IOM uint32_t RESERVED1 : 10; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t
        CONTINUE_COUNTER_1_EVENT_SEL : 6; /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    continuing the Counter 1 For 32 bit counter
                                                                    mode: Invalid.                                             */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } CT_CONTINUE_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_CONTINUE_COUNTER_AND_EVENT; /*!< (@ 0x00000060) Continue counter AND event register                       */

    struct {
      __IOM uint32_t
        CONTINUE_COUNTER_0_AND_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: AND expression valids
                                                                    for AND event in continue Counter 0 event
                                                                    For 32 bit counter mode AND expression
                                                                    valids for AND event in continue counter
                                                                    event.                                                     */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        CONTINUE_COUNTER_0_AND_VLD : 4; /*!< (@ 0x00000008) none                                                   */
      __IOM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t CONTINUE_COUNTER_1_AND_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: AND expression
                                                                    valids for AND event in continue counter
                                                                    event For 32 bit counter mode : Invalid                    */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        CONTINUE_COUNTER_1_AND_VLD : 4; /*!< (@ 0x00000018) none                                                   */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_CONTINUE_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_CONTINUE_COUNTER_OR_EVENT; /*!< (@ 0x00000064) Continue counter OR event register                         */

    struct {
      __IOM uint32_t
        CONTINUE_COUNTER_0_OR_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: OR expression valids
                                                                    for OR event in continue Counter 0 event
                                                                    For 32 bit counter mode OR expression valids
                                                                    for OR event in continue counter event                     */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        CONTINUE_COUNTER_0_OR_VLD : 4; /*!< (@ 0x00000008) none                                                    */
      __IOM uint32_t RESERVED2 : 4;    /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t
        CONTINUE_COUNTER_1_OR_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: OR expression valids
                                                                    for OR event in continue counter event
                                                                    For 32 bit counter mode : Invalid                          */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        CONTINUE_COUNTER_1_OR_VLD : 4; /*!< (@ 0x00000018) none                                                    */
      __IM uint32_t RESERVED4 : 4;     /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_CONTINUE_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_STOP_COUNTER_EVENT_SEL; /*!< (@ 0x00000068) Stop counter event select register                         */

    struct {
      __IOM uint32_t STOP_COUNTER_0_EVENT_SEL : 6; /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    Stopping the Counter 0 For 32 bit counter
                                                                    mode: Event select for Stopping counter                    */
      __IOM uint32_t RESERVED1 : 10; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t STOP_COUNTER_1_EVENT_SEL : 6; /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    Stopping the Counter 1 For 32 bit counter
                                                                    mode: Invalid                                              */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } CT_STOP_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_STOP_COUNTER_AND_EVENT; /*!< (@ 0x0000006C) Stop counter AND event register                            */

    struct {
      __IOM uint32_t
        STOP_COUNTER_0_AND_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: AND expression valids
                                                                    for AND event in stop Counter 0 event For
                                                                    32 bit counter mode AND expression valids
                                                                    for AND event in stop counter event                        */
      __IOM uint32_t RESERVED1 : 4;   /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        STOP_COUNTER_0_AND_VLD : 4; /*!< (@ 0x00000008) Indicates which bits in 3:0 are valid for considering
                                                                    AND event                                                  */
      __IOM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t STOP_COUNTER_1_AND_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: AND expression
                                                                    valids for AND event in stop counter event
                                                                    For 32 bit counter mode : Invalid                          */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        STOP_COUNTER_1_AND_VLD : 4; /*!< (@ 0x00000018) none                                                       */
      __IM uint32_t RESERVED4 : 4;  /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_STOP_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_STOP_COUNTER_OR_EVENT; /*!< (@ 0x00000070) Stop counter OR event register                             */

    struct {
      __IOM uint32_t STOP_COUNTER_0_OR_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: OR expression valids
                                                                    for OR event in Stop Counter 0 event For
                                                                    32 bit counter mode OR expression valids
                                                                    for OR event in Stop counter event                         */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        STOP_COUNTER_0_OR_VLD : 4;  /*!< (@ 0x00000008) none                                                       */
      __IOM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t STOP_COUNTER_1_OR_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: OR expression valids
                                                                    for OR event in Stop counter event For
                                                                    32 bit counter mode : Invalid                              */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        STOP_COUNTER_1_OR_VLD : 4; /*!< (@ 0x00000018) none                                                       */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_STOP_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_HALT_COUNTER_EVENT_SEL; /*!< (@ 0x00000074) Halt counter event select register                         */

    struct {
      __IOM uint32_t HALT_COUNTER_0_EVENT_SEL : 6;  /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    Halting the Counter 0 For 32 bit counter
                                                                    mode: Event select for Halting counter                     */
      __OM uint32_t RESUME_FROM_HALT_COUNTER_0 : 1; /*!< (@ 0x00000006) For two 16 bit counters mode: Event select for
                                                                    Halting the Counter 0 For 32 bit counter
                                                                    mode: Event select for Halting counter                     */
      __IM uint32_t RESERVED1 : 9; /*!< (@ 0x00000007) Reserved1                                                  */
      __IOM uint32_t HALT_COUNTER_1_EVENT_SEL : 6;  /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    Halting the Counter 1 For 32 bit counter
                                                                    mode: Invalid                                              */
      __OM uint32_t RESUME_FROM_HALT_COUNTER_1 : 1; /*!< (@ 0x00000016) For two 16 bit counters mode: Event select for
                                                                    Halting the Counter 0 For 32 bit counter
                                                                    mode: Event select for Halting counter                     */
      __IM uint32_t RESERVED2 : 9; /*!< (@ 0x00000017) Reserved2                                                  */
    } CT_HALT_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_HALT_COUNTER_AND_EVENT; /*!< (@ 0x00000078) Halt counter AND event register                            */

    struct {
      __IOM uint32_t
        HALT_COUNTER_0_AND_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: AND expression valids
                                                                    for AND event in stop Counter 0 event For
                                                                    32 bit counter mode AND expression valids
                                                                    for AND event in stop counter event                        */
      __IOM uint32_t RESERVED1 : 4;   /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        HALT_COUNTER_0_AND_VLD : 4; /*!< (@ 0x00000008) Indicates which bits in 3:0 are valid for considering
                                                                    AND event                                                  */
      __IM uint32_t RESERVED2 : 4;  /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t HALT_COUNTER_1_AND_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: AND expression
                                                                    valids for AND event in stop counter event
                                                                    For 32 bit counter mode : Invalid                          */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        HALT_COUNTER_1_AND_VLD : 4; /*!< (@ 0x00000018) none                                                       */
      __IM uint32_t RESERVED4 : 4;  /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_HALT_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_HALT_COUNTER_OR_EVENT; /*!< (@ 0x0000007C) Halt counter OR event register                             */

    struct {
      __IOM uint32_t HALT_COUNTER_0_OR_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: OR expression valids
                                                                    for OR event in Halt Counter 0 event For
                                                                    32 bit counter mode OR expression valids
                                                                    for OR event in Halt counter event                         */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        HALT_COUNTER_0_OR_VLD : 4; /*!< (@ 0x00000008) none                                                       */
      __IM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t HALT_COUNTER_1_OR_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: OR expression valids
                                                                    for OR event in Halt counter event For
                                                                    32 bit counter mode : Invalid                              */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        HALT_COUNTER_1_OR_VLD : 4; /*!< (@ 0x00000018) none                                                       */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_HALT_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_INCREMENT_COUNTER_EVENT_SEL; /*!< (@ 0x00000080) Increment counter event select register                  */

    struct {
      __IOM uint32_t
        INCREMENT_COUNTER_0_EVENT_SEL : 6; /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    Incrementing the Counter 0 For 32 bit counter
                                                                    mode: Event select for Incrementing counter                */
      __IM uint32_t RESERVED1 : 10; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t
        INCREMENT_COUNTER_1_EVENT_SEL : 6; /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    Incrementing the Counter 1 For 32 bit counter
                                                                    mode: Invalid                                              */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } CT_INCREMENT_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_INCREMENT_COUNTER_AND_EVENT; /*!< (@ 0x00000084) Increment counter AND event register                     */

    struct {
      __IOM uint32_t
        INCREMENT_COUNTER_0_AND_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: AND expression valids
                                                                    for AND event in stop Counter 0 event For
                                                                    32 bit counter mode AND expression valids
                                                                    for AND event in stop counter event                        */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        INCREMENT_COUNTER_0_AND_VLD : 4; /*!< (@ 0x00000008) none                                                  */
      __IM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t INCREMENT_COUNTER_1_AND_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: AND expression
                                                                    valids for AND event in stop counter event
                                                                    For 32 bit counter mode : Invalid                          */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        INCREMENT_COUNTER_1_AND_VLD : 4; /*!< (@ 0x00000018) none                                                  */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_INCREMENT_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_INCREMENT_COUNTER_OR_EVENT; /*!< (@ 0x00000088) Increment counter OR event register                       */

    struct {
      __IOM uint32_t
        INCREMENT_COUNTER_0_OR_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: OR expression valids
                                                                    for OR event in Increment Counter 0 event
                                                                    For 32 bit counter mode OR expression valids
                                                                    for OR event in Increment counter event                    */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        INCREMENT_COUNTER_0_OR_VLD : 4; /*!< (@ 0x00000008) none                                                   */
      __IM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t
        INCREMENT_COUNTER_1_OR_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: OR expression valids
                                                                    for OR event in Increment counter event
                                                                    For 32 bit counter mode : Invalid                          */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x00000014) Reserved4                                                  */
      __IOM uint32_t
        INCREMENT_COUNTER_1_OR_VLD : 4; /*!< (@ 0x00000018) none                                                   */
      __IM uint32_t RESERVED5 : 4; /*!< (@ 0x0000001C) Reserved5                                                  */
    } CT_INCREMENT_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_CAPTURE_COUNTER_EVENT_SEL; /*!< (@ 0x0000008C) Capture counter event select register                      */

    struct {
      __IOM uint32_t CAPTURE_COUNTER_0_EVENT_SEL : 6; /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    Capturing the Counter 0 For 32 bit counter
                                                                    mode: Event select for Capturing counter                   */
      __IM uint32_t RESERVED1 : 10; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t CAPTURE_COUNTER_1_EVENT_SEL : 6; /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    Capturing the Counter 1 For 32 bit counter
                                                                    mode : Invalid                                             */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } CT_CAPTURE_COUNTER_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_CAPTURE_COUNTER_AND_EVENT; /*!< (@ 0x00000090) Capture counter AND event register                         */

    struct {
      __IOM uint32_t
        CAPTURE_COUNTER_0_AND_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: AND expression valids
                                                                    for AND event in stop Counter 0 event For
                                                                    32 bit counter mode AND expression valids
                                                                    for AND event in stop counter event                        */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        CAPTURE_COUNTER_0_AND_VLD : 4; /*!< (@ 0x00000008) none                                                    */
      __IM uint32_t RESERVED2 : 4;     /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t CAPTURE_COUNTER_1_AND_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: AND expression
                                                                    valids for AND event in stop counter event
                                                                    For 32 bit counter mode : Invalid                          */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        CAPTURE_COUNTER_1_AND_VLD : 4; /*!< (@ 0x00000018) none                                                    */
      __IM uint32_t RESERVED4 : 4;     /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_CAPTURE_COUNTER_AND_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_CAPTURE_COUNTER_OR_EVENT; /*!< (@ 0x00000094) Capture counter OR event register                          */

    struct {
      __IOM uint32_t
        CAPTURE_COUNTER_0_OR_EVENT : 4; /*!< (@ 0x00000000) For two 16 bit counter mode: OR expression valids
                                                                    for OR event in Capture Counter 0 event
                                                                    For 32 bit counter mode OR expression valids
                                                                    for OR event in Capture counter event                      */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        CAPTURE_COUNTER_0_OR_VLD : 4; /*!< (@ 0x00000008) none                                                     */
      __IM uint32_t RESERVED2 : 4;    /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t
        CAPTURE_COUNTER_1_OR_EVENT : 4; /*!< (@ 0x00000010) For two 16 bit counters mode: OR expression valids
                                                                    for OR event in Capture counter event For
                                                                    32 bit counter mode : Invalid                              */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        CAPTURE_COUNTER_1_OR_VLD : 4; /*!< (@ 0x00000018) none                                                     */
      __IM uint32_t RESERVED4 : 4;    /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_CAPTURE_COUNTER_OR_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_OUTPUT_EVENT_SEL; /*!< (@ 0x00000098) Output event select register                               */

    struct {
      __IOM uint32_t OUTPUT_EVENT_SEL_0 : 6; /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    output event from Counter 0 For 32 bit
                                                                    counter mode: Event select for output event                */
      __IM uint32_t RESERVED1 : 10; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t OUTPUT_EVENT_SEL_1 : 6; /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    output event from counter 1 For 32 bit
                                                                    counter mode : Invalid                                     */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } CT_OUTPUT_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_OUTPUT_AND_EVENT_REG; /*!< (@ 0x0000009C) Output AND event Register                                  */

    struct {
      __IOM uint32_t OUTPUT_0_AND_EVENT : 4; /*!< (@ 0x00000000) AND expression for AND event in output Counter_0
                                                                    event.                                                     */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t OUTPUT_0_AND_VLD : 4; /*!< (@ 0x00000008) AND expression for AND event in output Counter_0
                                                                    event.                                                     */
      __IM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t OUTPUT_1_AND_EVENT : 4; /*!< (@ 0x00000010) AND expression for AND event in output Counter_1
                                                                    event.                                                     */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t OUTPUT_1_AND_VLD : 4; /*!< (@ 0x00000018) AND expression for AND event in output Counter_1
                                                                    event.                                                     */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_OUTPUT_AND_EVENT_REG_b;
  };

  union {
    __IOM uint32_t CT_OUTPUT_OR_EVENT; /*!< (@ 0x000000A0) Output OR event Register                                   */

    struct {
      __IOM uint32_t OUTPUT_0_OR_EVENT : 4; /*!< (@ 0x00000000) OR expression for OR event in output Counter_0
                                                                    event                                                      */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t OUTPUT_0_OR_VLD : 4; /*!< (@ 0x00000008) Indicates which bits in 3:0 are valid for considering
                                                                    OR event                                                   */
      __IM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t OUTPUT_1_OR_EVENT : 4; /*!< (@ 0x00000010) OR expression for OR event in output Counter_0
                                                                    event                                                      */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t OUTPUT_1_OR_VLD : 4; /*!< (@ 0x00000018) Indicates which bits in 3:0 are valid for considering
                                                                    OR event                                                   */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_OUTPUT_OR_EVENT_b;
  };

  union {
    __IOM uint32_t CT_INTR_EVENT_SEL; /*!< (@ 0x000000A4) Interrupt Event Select Register                            */

    struct {
      __IOM uint32_t INTR_EVENT_SEL_0 : 6; /*!< (@ 0x00000000) For two 16 bit counters mode: Event select for
                                                                    interrupt event from Counter 0 For 32 bit
                                                                    counter mode: Event select for output event                */
      __IM uint32_t RESERVED1 : 10; /*!< (@ 0x00000006) Reserved1                                                  */
      __IOM uint32_t INTR_EVENT_SEL_1 : 6; /*!< (@ 0x00000010) For two 16 bit counters mode: Event select for
                                                                    interrupt event from counter 1 For 32 bit
                                                                    counter mode : Invalid                                     */
      __IM uint32_t RESERVED2 : 10; /*!< (@ 0x00000016) Reserved2                                                  */
    } CT_INTR_EVENT_SEL_b;
  };

  union {
    __IOM uint32_t CT_INTR_AND_EVENT; /*!< (@ 0x000000A8) Interrupt AND Event Register                               */

    struct {
      __IOM uint32_t
        INTR_0_AND_EVENT : 4;       /*!< (@ 0x00000000) None                                                       */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        INTR_0_AND_VLD : 4;        /*!< (@ 0x00000008) None                                                       */
      __IM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t
        INTR_1_AND_EVENT : 4;      /*!< (@ 0x00000010) None                                                       */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        INTR_1_AND_VLD : 4;        /*!< (@ 0x00000018) None                                                       */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_INTR_AND_EVENT_b;
  };

  union {
    __IOM uint32_t
      CT_INTR_OR_EVENT_REG; /*!< (@ 0x000000AC) Interrupt OR Event Register                                */

    struct {
      __IOM uint32_t
        INTR_0_OR_EVENT : 4;        /*!< (@ 0x00000000) None                                                       */
      __IOM uint32_t RESERVED1 : 4; /*!< (@ 0x00000004) Reserved1                                                  */
      __IOM uint32_t
        INTR_0_OR_VLD : 4;         /*!< (@ 0x00000008) None                                                       */
      __IM uint32_t RESERVED2 : 4; /*!< (@ 0x0000000C) Reserved2                                                  */
      __IOM uint32_t
        INTR_1_OR_EVENT : 4;       /*!< (@ 0x00000010) None                                                       */
      __IM uint32_t RESERVED3 : 4; /*!< (@ 0x00000014) Reserved3                                                  */
      __IOM uint32_t
        INTR_1_OR_VLD : 4;         /*!< (@ 0x00000018) None                                                       */
      __IM uint32_t RESERVED4 : 4; /*!< (@ 0x0000001C) Reserved4                                                  */
    } CT_INTR_OR_EVENT_REG_b;
  };
} CT0_Type; /*!< Size = 176 (0xb0)                                                         */

/* =========================================================================================================================== */
/* ================               CT_MUX_REG                                         ================ */
/* =========================================================================================================================== */

/**
  * @brief Configurable timer is used in counting clocks, events and states with reference clock
                   external clock and system clock (CT_MUX_REG)   */

typedef struct { /*!< (@ 0x4506F000) CT_MUX_REG Structure                                       */
  union {
    __IOM uint32_t CT_MUX_SEL_0_REG; /*!< (@ 0x00000000) MUX_SEL_0_REG Register                                     */

    struct {
      __IOM uint32_t MUX_SEL_0 : 4;  /*!< [3..0] Select value to select first output value fifo_0_full[0]
                                                     out of all the fifo_0_full_muxed signals of counter 0                     */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved1                                                         */
    } CT_MUX_SEL_0_REG_b;
  };

  union {
    __IOM uint32_t CT_MUX_SEL_1_REG; /*!< (@ 0x00000004) MUX_SEL_1_REG Register                                     */

    struct {
      __IOM uint32_t MUX_SEL_1 : 4;  /*!< [3..0] Select value to select first output value fifo_0_full[1]
                                                     out of all the fifo_0_full_muxed signals of counter 0                     */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved1                                                         */
    } CT_MUX_SEL_1_REG_b;
  };

  union {
    __IOM uint32_t CT_MUX_SEL_2_REG; /*!< (@ 0x00000008) MUX_SEL_2_REG Register                                     */

    struct {
      __IOM uint32_t MUX_SEL_2 : 4;  /*!< [3..0] Select value to select first output value fifo_1_full[0]
                                                     out of all the fifo_1_full_muxed signals of counter 1                     */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved1                                                         */
    } CT_MUX_SEL_2_REG_b;
  };

  union {
    __IOM uint32_t CT_MUX_SEL_3_REG; /*!< (@ 0x0000000C) MUX_SEL_3_REG Register                                     */

    struct {
      __IOM uint32_t MUX_SEL_3 : 4;  /*!< [3..0] Select value to select first output value fifo_1_full[1]
                                                     out of all the fifo_1_full_muxed signals of counter 1                     */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved1                                                         */
    } CT_MUX_SEL_3_REG_b;
  };
  __IM uint32_t RESERVED[2];

  union {
    __IOM uint32_t
      CT_OUTPUT_EVENT1_ADC_SEL; /*!< (@ 0x00000018) OUTPUT_EVENT_ADC_SEL Register                              */

    struct {
      __IOM uint32_t OUTPUT_EVENT_ADC_SEL : 4; /*!< [3..0] Select signals to select one output event out of all
                                                     the output events output_event_0 output_event_1, output_event_2,
                                                     output_event_3 to enable ADC module                                       */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved1                                                         */
    } CT_OUTPUT_EVENT1_ADC_SEL_b;
  };

  union {
    __IOM uint32_t
      CT_OUTPUT_EVENT2_ADC_SEL; /*!< (@ 0x0000001C) OUTPUT_EVENT_ADC_SEL Register                              */

    struct {
      __IOM uint32_t OUTPUT_EVENT_ADC_SEL : 4; /*!< [3..0] Select signals to select one output event out of all
                                                     the output events output_event_0 output_event_1, output_event_2,
                                                     output_event_3 to enable ADC module                                       */
      __IOM uint32_t RESERVED1 : 28; /*!< [31..4] Reserved1                                                         */
    } CT_OUTPUT_EVENT2_ADC_SEL_b;
  };
} CT_MUX_REG_Type; /*!< Size = 32 (0x20)                                                          */

/* ================================================================================ */
/* ================                    ETHERNET                    ================ */
/* ================================================================================ */

/**
 * @brief Ether MAC 10/100/1000 Universal enables a host to transmit and receive data over Ethernet in compliance with the IEEE 802.3-2002standard (ETHERNET)
 */

typedef struct {
  union {
    __IO uint32_t MAC_ADDR_HIGH_REG; /*!< The MAC Address1 High register holds the upper 16 bits of the
                                                         6-byte second MAC address of thestation.                              */

    struct {
      __IO uint32_t MAC_ADDR1_HIGH : 16; /*!< This field contains the upper 16 bits (47:32) of the 6-byte
                                                         second MAC address.                                                   */
      uint32_t : 8;
      __IO uint32_t MBC_ADDR1_HIGH : 6; /*!< Mask Byte Control                                                     */
      __IO uint32_t SA : 1;             /*!< When this bit is set, the MAC Address1[47:0] is used to compare
                                                         with the SAfields of the received frame                               */
      __IO uint32_t AE : 1;             /*!< When this bit is set, the Address filter module uses the second
                                                         MAC address forperfect filtering                                      */
    } MAC_ADDR_HIGH_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MAC_ADDR_LOW_REG; /*!< The MAC Address1 Low register holds the lower 32 bits of the
                                                         6-byte second MAC address of the station                              */

    struct {
      __IO uint32_t MAC_ADDR1_LOW : 32; /*!< This field contains the upper 16 bits (47:32) of the 6-byte
                                                         second MAC address.                                                   */
    } MAC_ADDR_LOW_REG_b;               /*!< BitSize                                                               */
  };
} ETHERNET_MAC_16_ADDR_Type;

typedef struct {
  union {
    __IO uint32_t MAC_ADDR_HIGH_REG; /*!< The MAC Address1 High register holds the upper 16 bits of the
                                                         6-byte second MAC address of thestation.                              */

    struct {
      __IO uint32_t MAC_ADDR1_HIGH : 16; /*!< This field contains the upper 16 bits (47:32) of the 6-byte
                                                         second MAC address.                                                   */
      uint32_t : 8;
      __IO uint32_t MBC_ADDR1_HIGH : 6; /*!< Mask Byte Control                                                     */
      __IO uint32_t SA : 1;             /*!< When this bit is set, the MAC Address1[47:0] is used to compare
                                                         with the SAfields of the received frame                               */
      __IO uint32_t AE : 1;             /*!< When this bit is set, the Address filter module uses the second
                                                         MAC address forperfect filtering                                      */
    } MAC_ADDR_HIGH_REG_b;              /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MAC_ADDR0_LOW_REG19; /*!< The MAC Address1 Low register holds the lower 32 bits of the
                                                         6-byte second MAC address of thestation                               */

    struct {
      __IO uint32_t MAC_ADDR1_LOW : 32; /*!< This field contains the upper 16 bits (47:32) of the 6-byte
                                                         second MAC address.                                                   */
    } MAC_ADDR0_LOW_REG19_b;            /*!< BitSize                                                               */
  };
} ETHERNET_MAC_31_ADDR_Type;

/* ================================================================================ */
/* ================                    ETHERNET                    ================ */
/* ================================================================================ */

/**
 * @brief Ether MAC 10/100/1000 Universal enables a host to transmit and receive data over Ethernet in compliance with the IEEE 802.3-2002standard (ETHERNET)
 */

typedef struct { /*!< ETHERNET Structure                                                    */

  union {
    __IO uint32_t MAC_CONFIG_REG0; /*!< This is the operation mode register for the MAC.                      */

    struct {
      uint32_t : 2;
      __IO uint32_t RE : 1;   /*!< Receiver Enable                                                       */
      __IO uint32_t TE : 1;   /*!< Transmitter Enable                                                    */
      __IO uint32_t DC : 1;   /*!< Deferral Check                                                        */
      __IO uint32_t BL : 2;   /*!< Back-Off Limit                                                        */
      __IO uint32_t ACS : 1;  /*!< Automatic Pad/CRC Stripping                                           */
      __IO uint32_t LUD : 1;  /*!< Link Up/Down                                                          */
      __IO uint32_t DR : 1;   /*!< Disable Retry                                                         */
      __IO uint32_t IPC : 1;  /*!< Checksum Offload                                                      */
      __IO uint32_t DM : 1;   /*!< Duplex Mode                                                           */
      __IO uint32_t LM : 1;   /*!< Loop-back Mode                                                        */
      __IO uint32_t DO : 1;   /*!< Disable Receive Own                                                   */
      __IO uint32_t FES : 1;  /*!< Indicates the speed in Fast Ethernet (MII) mode:                      */
      __IO uint32_t PS : 1;   /*!< Selects between GMII and MII:                                         */
      __IO uint32_t DCRS : 1; /*!< Disable Carrier Sense During Transmission                             */
      __IO uint32_t IFG : 3;  /*!< Inter-Frame Gap These bits control the minimum IFG between frames
                                                         during transmission                                                   */
      __IO uint32_t JE : 1;   /*!< Jumbo Frame Enable                                                    */
      __IO uint32_t BE : 1;   /*!< Frame Burst Enable                                                    */
      __IO uint32_t JD : 1;   /*!< Jabber Disable                                                        */
      __IO uint32_t WD : 1;   /*!< Watchdog Disable When this bit is set, the GMAC disables the
                                                         watchdog timer on the receiver, andcan receive frames of up
                                                          to 16,384 bytes                                                      */
      __IO uint32_t TC : 1;   /*!< Transmit Configuration in RGMII/SGMII                                 */
    } MAC_CONFIG_REG0_b;      /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MAC_FRAME_FILTER_REG1; /*!< This is the operation mode register for the MAC.                      */

    struct {
      __IO uint32_t PR : 1;   /*!< Promiscuous Mode                                                      */
      __IO uint32_t HUC : 1;  /*!< Hash Unicast                                                          */
      __IO uint32_t HMC : 1;  /*!< Hash Multicast                                                        */
      __IO uint32_t DAIF : 1; /*!< DA Inverse Filtering                                                  */
      __IO uint32_t PM : 1;   /*!< Pass All Multicast                                                    */
      __IO uint32_t DBF : 1;  /*!< Disable Broadcast Frames                                              */
      __IO uint32_t PCF : 2;  /*!< Pass Control Frames                                                   */
      __IO uint32_t SAIF : 1; /*!< SA Inverse Filtering                                                  */
      __IO uint32_t SAF : 1;  /*!< Source Address Filter Enable                                          */
      __IO uint32_t HPF : 1;  /*!< Hash or Perfect Filter                                                */
      uint32_t : 20;
      __IO uint32_t RA : 1;    /*!< Receive All                                                           */
    } MAC_FRAME_FILTER_REG1_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t HASH_TABLE_HIGH_REG2; /*!< This is the operation mode register for the MAC.                      */

    struct {
      __IO uint32_t HTH : 32; /*!< Hash Table HighThis field contains the upper 32 bits of Hash
                                                         table.                                                                */
    } HASH_TABLE_HIGH_REG2_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t HASH_TABLE_LOW_REG3; /*!< This is the operation mode register for the MAC.                      */

    struct {
      __IO uint32_t HTL : 32; /*!< Hash Table LowThis field contains the lower 32 bits of Hash
                                                         table.                                                                */
    } HASH_TABLE_LOW_REG3_b;  /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t GMII_ADDR_REG4; /*!< This is the operation mode register for the MAC.                      */

    struct {
      __IO uint32_t GB : 1; /*!< GMII Busy                                                             */
      __IO uint32_t GW : 1; /*!< GMII Write. When set this bit tells the PHY that this will be
                                                         a Write operation using the GMIIData register                         */
      __IO uint32_t CR : 3; /*!< CSR Clock Range. The CSR Clock Range selection determines the
                                                         clk_csr_i frequency and isused to decide the frequency of the
                                                          MDC clock                                                            */
      uint32_t : 1;
      __IO uint32_t GR : 5; /*!< GMII RegisterThese bits select the desired GMII register in
                                                         the selected PHY device                                               */
      __IO uint32_t PR : 5; /*!< Physical Layer AddressThis field tells which of the 32 possible
                                                         PHY devices are being accessed                                        */
    } GMII_ADDR_REG4_b;     /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t GMII_DATA_REG5; /*!< This is the operation mode register for the MAC.                      */

    struct {
      __IO uint32_t GD : 16; /*!< GMII Data.This contains the 16-bit data value read from the
                                                         PHY after a Management Read operation or the 16-bit data value
                                                          to be written to the PHY before a ManagementWrite operation          */
    } GMII_DATA_REG5_b;      /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t FLOW_CTRL_REG6; /*!< The Flow Control register controls the generation and reception
                                                         of the Control (Pause Command)frames by the GMAC's Flow control
                                                          module.                                                              */

    struct {
      __IO uint32_t FCB_BPA : 1; /*!< Flow Control Busy/Back pressure Activate                              */
      __IO uint32_t TFE : 1;     /*!< Transmit Flow Control Enable                                          */
      __IO uint32_t RFE : 1;     /*!< Receive Flow Control Enable                                           */
      __IO uint32_t UP : 1;      /*!< Unicast Pause Frame Detect                                            */
      __IO uint32_t PLT : 2;     /*!< Pause Low Threshold                                                   */
      __IO uint32_t DZPQ : 1;    /*!< Disable Zero-Quanta Pause                                             */
      uint32_t : 9;
      __IO uint32_t PT : 16; /*!< Pause Time                                                            */
    } FLOW_CTRL_REG6_b;      /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t VLAN_TAG_REG7; /*!< The Flow Control register controls the generation and reception
                                                         of the Control (Pause Command)frames by the GMAC's Flow control
                                                          module.                                                              */

    struct {
      __IO uint32_t VL : 16; /*!< VLAN Tag Identifier.This contains the 802.1Q VLAN Tag to identify
                                                         the VLAN frames, and is used tocompare with the 15th and 16th
                                                          bytes of the receiving frames for VLAN frames                        */
    } VLAN_TAG_REG7_b;       /*!< BitSize                                                               */
  };
  __I uint32_t VERSION_REG8; /*!< The Version register s contents identify the version of the
                                                         core.                                                                 */
  __I uint32_t RESERVED;
  __IO uint32_t RMT_WKUP_FMFILTER_REG; /*!< This is the address through which the remote Wake-up Frame Filter
                                                         registers(wkupfmfilter_reg) are written/read by the Application.This
                                                          register contains the higher 16 bits of the 7th MAC address.
	 */

  union {
    __IO uint32_t PMT_CTRL_STS_REG; /*!< The PMT CSR program the request wake-up events and monitor the
                                                         wake-up events.                                                       */

    struct {
      __IO uint32_t PD : 1;  /*!< When set, all received frames will be dropped                         */
      __IO uint32_t MPE : 1; /*!< Magic Packet Enable.When set, enables generation of a power
                                                         management event due to MagicPacket reception                         */
      __IO uint32_t WFE : 1; /*!< Wake-Up Frame Enable.When set, enables generation of a power
                                                         management event due to wakeupframe reception                         */
      uint32_t : 2;
      __IO uint32_t MPR : 1; /*!< When set, this bit indicates the power management event was
                                                         generated bythe reception of a Magic Packet                           */
      __IO uint32_t WFR : 1; /*!< When set, this bit indicates the power management event was
                                                         generateddue to reception of a wake-up frame                          */
      uint32_t : 2;
      __IO uint32_t GU : 1; /*!< When set, enables any unicast packet filtered by the GMAC (DAF)
                                                         addressrecognition to be a wake-up frame                              */
      uint32_t : 21;
      __IO uint32_t WFFRPR : 1; /*!< When set, resets the Remote Wake-up Frame Filter register pointer
                                                         to000                                                                 */
    } PMT_CTRL_STS_REG_b;       /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED1[2];

  union {
    __I uint32_t INTR_STS_REG14; /*!< The Interrupt Status register contents identify the events in
                                                         the GMAC-CORE that can generateinterrupt                              */

    struct {
      __I uint32_t RGMII_INTR_STS : 1; /*!< This bit is set due to any change in value of the Link Status
                                                         of RGMII interface                                                    */
      __I uint32_t
        PCS_LINK_STATUS_CHANGED : 1; /*!< This bit is set due to any change in Link Status in the TBI/RTBI/SGMII
                                                         PHYinterface                                                          */
      __I uint32_t PCS_AUTO_NEGOTIATION_COMPLETE : 1; /*!< This bit is set when the Auto-negotiation is completed in the
                                                         TBI/RTBI/SGMII PHYinterface                                           */
      __I uint32_t PMT_INTR_STS : 1;                  /*!< This bit is set when the Auto-negotiation is completed in the
                                                         TBI/RTBI/SGMII PHYinterface                                           */
      __I uint32_t MMC_INTR_STS : 1;                  /*!< This bit is set high whenever any of bits 7:5 is set high and
                                                         cleared only when all ofthese bits are low.                           */
      __I uint32_t MMC_RX_INTR_STS : 1; /*!< MMC Receive Interrupt Status                                          */
      __I uint32_t MMC_TX_INTR_STS : 1; /*!< MMC Transmit Interrupt Status                                         */
      __I uint32_t
        MMC_RX_CH_OFF_INTR_STS : 1; /*!< MMC Receive Checksum Offload Interrupt Status                         */
    } INTR_STS_REG14_b;             /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t INTR_MASK_REG15; /*!< The Interrupt Mask Register bits enables the user to mask the
                                                         interrupt signal due to the correspondingevent in the Interrupt
                                                          Status Register                                                      */

    struct {
      __IO uint32_t RGMII_M : 1;             /*!< This bit when set, will disable the assertion of the interrupt
                                                         signal due to the settingof RGMII Interrupt Status bit in Register14. */
      __IO uint32_t PCS_LINK_M : 1;          /*!< This bit when set, will disable the assertion of the interrupt
                                                         signal due to the settingof RGMII Interrupt Status bit in Register14. */
      __IO uint32_t PCS_AN_COMPLETION_M : 1; /*!< This bit when set, will disable the assertion of the interrupt
                                                         signal due to the settingof RGMII Interrupt Status bit in Register14. */
      __IO uint32_t PMT_M : 1;               /*!< This bit when set, will disable the assertion of the interrupt
                                                         signal due to the settingof PMT Interrupt Status bit in Register14    */
    } INTR_STS_REG15_b; /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MAC_ADDR0_HIGH_REG; /*!< The MAC Address0 High register holds the upper 16 bits of the
                                                         6-byte first MAC address of the station                               */

    struct {
      __IO uint32_t MAC_ADDR0_HIGH : 16; /*!< This field contains the upper 16 bits (47:32) of the 6-byte
                                                         first MAC address                                                     */
      uint32_t : 15;
      __I uint32_t MO : 1;  /*!< Always 1                                                              */
    } MAC_ADDR0_HIGH_REG_b; /*!< BitSize                                                               */
  };
  __IO uint32_t MAC_ADDR0_LOW_REG; /*!< The MAC Address0 Low register holds the lower 32 bits of the
                                                         6-byte first MAC address of the station                               */
  ETHERNET_MAC_16_ADDR_Type
    MAC_16_ADDR[15]; /*!< MAC_16_ADDR0                                                          */

  union {
    __IO uint32_t AN_CTRL_REG48; /*!< The AN Control register enables and/or restarts auto-negotiation.     */

    struct {
      uint32_t : 9;
      __IO uint32_t RAN : 1; /*!< Restart Auto-Negotiation                                              */
      uint32_t : 2;
      __IO uint32_t ANE : 1; /*!< Auto-Negotiation Enable                                               */
      uint32_t : 1;
      __IO uint32_t ELE : 1; /*!< External Loopback Enable                                              */
      uint32_t : 1;
      __IO uint32_t ECD : 1;       /*!< Enable Comma Detect                                                   */
      __IO uint32_t LR : 1;        /*!< Enable Comma Detect                                                   */
      __IO uint32_t SGMII_RAL : 1; /*!< SGMII RAL Control                                                     */
    } AN_CTRL_REG48_b;             /*!< BitSize                                                               */
  };

  union {
    __I uint32_t AN_CTRL_REG49; /*!< The AN Status register indicates the link and the auto-negotiation
                                                         status                                                                */

    struct {
      uint32_t : 2;
      __I uint32_t LS : 1;  /*!< Link Status                                                           */
      __I uint32_t ANA : 1; /*!< Auto-Negotiation Ability                                              */
      uint32_t : 1;
      __I uint32_t ANC : 1; /*!< Auto-Negotiation Complete                                             */
      uint32_t : 2;
      __I uint32_t ES : 1; /*!< Extended Status                                                       */
    } AN_CTRL_REG49_b;     /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t ANA_ADV_REG50; /*!< The Auto-Negotiation Advertisement register indicates the link
                                                         and the auto-negotiation status.                                      */

    struct {
      uint32_t : 5;
      __IO uint32_t FD : 1;  /*!< Full-Duplex.This bit, when set high, indicates that the GMAC
                                                         supports Full-Duplex.                                                 */
      __IO uint32_t HD : 1;  /*!< Half-Duplex                                                           */
      __IO uint32_t PSE : 2; /*!< Pause Encoding                                                        */
      uint32_t : 3;
      __IO uint32_t RFE : 2; /*!< Remote Fault Encoding                                                 */
      uint32_t : 1;
      __I uint32_t NP : 1; /*!< Next Page Support                                                     */
    } ANA_ADV_REG50_b;     /*!< BitSize                                                               */
  };

  union {
    __I uint32_t ANL_PA_REG51; /*!< The Auto-Negotiation Link Partner Ability register contains
                                                         the advertised ability of the link partner                            */

    struct {
      uint32_t : 5;
      __I uint32_t FD : 1;   /*!< Full-Duplex.This bit, when set high, indicates that the GMAC
                                                         supports Full-Duplex.                                                 */
      __IO uint32_t HD : 1;  /*!< Half-Duplex                                                           */
      __IO uint32_t PSE : 2; /*!< Pause Encoding                                                        */
      uint32_t : 3;
      __IO uint32_t RFE : 2; /*!< Remote Fault Encoding                                                 */
      __IO uint32_t ACK : 1; /*!< Acknowledge                                                           */
      __I uint32_t NP : 1;   /*!< Next Page Support                                                     */
    } ANL_PA_REG51_b;        /*!< BitSize                                                               */
  };

  union {
    __I uint32_t ANE_REG52; /*!< The Auto-Negotiation Expansion register indicates whether a
                                                         new base page from the link partner hasbeen received                  */

    struct {
      uint32_t : 1;
      __I uint32_t NPR : 1; /*!< New Page Received                                                     */
      __I uint32_t NPA : 1; /*!< Next Page Ability                                                     */
    } ANE_REG52_b;          /*!< BitSize                                                               */
  };

  union {
    __I uint32_t TBI_EXT_STS_REG53; /*!< The TBI Extended Status Register register indicates all modes
                                                         of operation of the GMAC.                                             */

    struct {
      uint32_t : 14;
      __I uint32_t GHD : 1; /*!< 1000BASE-X Half-Duplex Capable                                        */
      __I uint32_t GFD : 1; /*!< 1000BASE-X Full-Duplex Capable                                        */
    } TBI_EXT_STS_REG53_b;  /*!< BitSize                                                               */
  };

  union {
    __I uint32_t TBI_EXT_STS_REG54; /*!< The SGMII/RGMII Status register indicates the status signals
                                                         received by the SGMII/RGMII from the PHY                              */

    struct {
      __I uint32_t LINK_MODE : 1;   /*!< Indicates the current mode of operation of the link                   */
      __I uint32_t LINK_SPEED : 2;  /*!< Indicates the current speed of the link:                              */
      __I uint32_t LINK_STATUS : 1; /*!< Indicates whether the link is up 1 or down 0.                         */
    } TBI_EXT_STS_REG54_b;          /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED2[9];

  union {
    __IO uint32_t MMC_CTRL_REG; /*!< The MMC Control register establishes the operating mode of the
                                                         management counters.                                                  */

    struct {
      __IO uint32_t MMC_CR : 1;  /*!< Counters Reset.When set, all counters will be reset. This bit
                                                         will be cleared automaticallyafter 1 clock cycle                      */
      __IO uint32_t MMC_CSR : 1; /*!< Counter Stop Roll over.When set, counter after reaching maximum
                                                         value will not roll over to zero.                                     */
      __IO uint32_t MMC_RR : 1;  /*!< Reset on Read. When set, counter after reaching maximum value
                                                         will not roll over to zero.                                           */
      __IO uint32_t MMC_CF : 1;  /*!< MMC Counter Freeze.When set, this bit freezes all the MMC counters
                                                         to their current value.                                               */
    } MMC_CTRL_REG_b;            /*!< BitSize                                                               */
  };

  union {
    __I uint32_t MMC_RX_INTR_REG; /*!< The MMC Receive Interrupt register maintains the interrupts
                                                         generated when receive statistic countersreach half their maximum
                                                          values                                                               */

    struct {
      __I uint32_t BIT_0 : 1;  /*!< The bit is set when the rx framecount_gb counter reaches half
                                                         themaximum value                                                      */
      __I uint32_t BIT_1 : 1;  /*!< The bit is set when the rxoctetcount_gb counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_2 : 1;  /*!< The bit is set when the rxoctetcount_g counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_3 : 1;  /*!< The bit is set when the rxbroadcastframes_g counter reaches
                                                         half themaximum value                                                 */
      __I uint32_t BIT_4 : 1;  /*!< The bit is set when the rxmulticastframes_g counter reaches
                                                         half themaximum value.                                                */
      __I uint32_t BIT_5 : 1;  /*!< The bit is set when the rxcrcerror counter reaches half the
                                                         maximumvalue.                                                         */
      __I uint32_t BIT_6 : 1;  /*!< The bit is set when the rxalignmenterror counter reaches half
                                                         themaximum value                                                      */
      __I uint32_t BIT_7 : 1;  /*!< The bit is set when the rxrunterror counter reaches half the
                                                         maximumvalue.                                                         */
      __I uint32_t BIT_8 : 1;  /*!< The bit is set when the rxjabbererror counter reaches half the
                                                         maximumvalue.                                                         */
      __I uint32_t BIT_9 : 1;  /*!< The bit is set when the rxundersize_g counter reaches half the
                                                         maximumvalue.                                                         */
      __I uint32_t BIT_10 : 1; /*!< The bit is set when the rxoversize_g counter reaches half the
                                                         maximumvalue.                                                         */
      __I uint32_t BIT_11 : 1; /*!< The bit is set when the rx64octets_gb counter reaches half the
                                                         maximumvalue.                                                         */
      __I uint32_t BIT_12 : 1; /*!< The bit is set when the rx65to127octets_gb counter reaches half
                                                         themaximum value.                                                     */
      __I uint32_t BIT_13 : 1; /*!< The bit is set when the rx128to255octets_gb counter reaches
                                                         half themaximum value.                                                */
      __I uint32_t BIT_14 : 1; /*!< The bit is set when the rx256to511octets_gb counter reaches
                                                         half themaximum value                                                 */
      __I uint32_t BIT_15 : 1; /*!< The bit is set when the rx512to1023octets_gb counter reaches
                                                         half themaximum value.                                                */
      __I uint32_t BIT_16 : 1; /*!< The bit is set when the rx1024tomaxoctets_gb counter reaches
                                                         half themaximum value.                                                */
      __I uint32_t BIT_17 : 1; /*!< The bit is set when the rxunicastframes_gb counter reaches half
                                                         themaximum value.                                                     */
      __I uint32_t BIT_18 : 1; /*!< The bit is set when the rx length error counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_19 : 1; /*!< The bit is set when the rx out of range type counter reaches
                                                         half themaximum value                                                 */
      __I uint32_t BIT_20 : 1; /*!< The bit is set when the rx pause frames counter reaches half
                                                         the maximum value                                                     */
      __I uint32_t BIT_21 : 1; /*!< The bit is set when the rx fifo overflow counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_22 : 1; /*!< The bit is set when the rxvlanframes_gb counter reaches half
                                                         themaximum value.                                                     */
      __I uint32_t BIT_23 : 1; /*!< The bit is set when the rx watchdog error counter reaches half
                                                         themaximum value.                                                     */
    } MMC_RX_INTR_REG_b;       /*!< BitSize                                                               */
  };

  union {
    __I uint32_t MMC_TX_INTR_REG; /*!< The MMC Transmit Interrupt register maintains the interrupts
                                                         generated when transmit statisticcounters reach half their maximum
                                                          values                                                               */

    struct {
      __I uint32_t BIT_0 : 1;  /*!< The bit is set when the txoctetcount_gb counter reaches half
                                                         the maximum value.                                                    */
      __I uint32_t BIT_1 : 1;  /*!< The bit is set when the txframecount_gb counter reaches half
                                                         the maximum value.                                                    */
      __I uint32_t BIT_2 : 1;  /*!< The bit is set when the txbroadcastframes_g counter reaches
                                                         half the maximum value.                                               */
      __I uint32_t BIT_3 : 1;  /*!< The bit is set when the txmulticastframes_g counter reaches
                                                         half the maximum value.                                               */
      __I uint32_t BIT_4 : 1;  /*!< The bit is set when the tx64to127octets_gb counter reaches half
                                                         the maximum value.                                                    */
      __I uint32_t BIT_5 : 1;  /*!< The bit is set when the tx65to127octets_gb counter reaches half
                                                         the maximum value                                                     */
      __I uint32_t BIT_6 : 1;  /*!< The bit is set when the tx128to255octets_gb counter reaches
                                                         half the maximum value.                                               */
      __I uint32_t BIT_7 : 1;  /*!< The bit is set when the tx256to511octets_gb counter reaches
                                                         half the maximum value.                                               */
      __I uint32_t BIT_8 : 1;  /*!< The bit is set when the tx512to1023octets_gb counter reaches
                                                         half the maximum value                                                */
      __I uint32_t BIT_9 : 1;  /*!< The bit is set when the tx1024tomaxoctets_gb counter reaches
                                                         half the maximum value                                                */
      __I uint32_t BIT_10 : 1; /*!< The bit is set when the txunicastframes_gb counter reaches half
                                                         the maximum value.                                                    */
      __I uint32_t BIT_11 : 1; /*!< The bit is set when the txmulticastframes_gb counter reaches
                                                         half the maximum value.                                               */
      __I uint32_t BIT_12 : 1; /*!< The bit is set when the txbroadcastframes_gb counter reaches
                                                         half the maximum value                                                */
      __I uint32_t BIT_13 : 1; /*!< The bit is set when the txunderflowerror counter reaches half
                                                         the maximum value.                                                    */
      __I uint32_t BIT_14 : 1; /*!< The bit is set when the txsinglecol_g counter reaches half the
                                                         maximum value                                                         */
      __I uint32_t BIT_15 : 1; /*!< The bit is set when the txmulticol_g counter reaches half the
                                                         maximum value                                                         */
      __I uint32_t BIT_16 : 1; /*!< The bit is set when the txdeferred counter reaches half the
                                                         maximum value                                                         */
      __I uint32_t BIT_17 : 1; /*!< The bit is set when the txlatecol counter reaches half the maximum
                                                         value                                                                 */
      __I uint32_t BIT_18 : 1; /*!< The bit is set when the txexesscol counter reaches half the
                                                         maximum value.                                                        */
      __I uint32_t BIT_19 : 1; /*!< The bit is set when the txcarriererror counter reaches half
                                                         the maximum value                                                     */
      __I uint32_t BIT_20 : 1; /*!< The bit is set when the txoctetcount_g counter reaches half
                                                         the maximum value                                                     */
      __I uint32_t BIT_21 : 1; /*!< The bit is set when the txframecount_g counter reaches half
                                                         the maximum value.                                                    */
      __I uint32_t BIT_22 : 1; /*!< The bit is set when the txoexcessdef counter reaches half the
                                                         maximum value                                                         */
      __I uint32_t BIT_23 : 1; /*!< The bit is set when the tx pause frames error counter reaches
                                                         half the maximum value.                                               */
      __I uint32_t BIT_24 : 1; /*!< The bit is set when the txvlanframes_g counter reaches half
                                                         the maximum value                                                     */
    } MMC_TX_INTR_REG_b;       /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MMC_RX_INTR_MASK_REG; /*!< MMC Receive Interrupt Mask register maintains the masks for
                                                         the interrupts generated whenreceive statistic counters reach
                                                          half their maximum value                                             */

    struct {
      __I uint32_t BIT_0 : 1;  /*!< Setting this bit masks the interrupt when the rxframecount_gb
                                                         counter reaches halfthe maximum value                                 */
      __I uint32_t BIT_1 : 1;  /*!< Setting this bit masks the interrupt when the rxoctetcount_gb
                                                         counter reaches halfthe maximum value                                 */
      __I uint32_t BIT_2 : 1;  /*!< Setting this bit masks the interrupt when the rxoctetcount_g
                                                         counter reaches half the maximumvalue.                                */
      __I uint32_t BIT_3 : 1;  /*!< Setting this bit masks the interrupt when the rxbroadcastframes_g
                                                         counter reaches half themaximum value                                 */
      __I uint32_t BIT_4 : 1;  /*!< The bit is set when the rxmulticastframes_g counter reaches
                                                         half themaximum value.                                                */
      __I uint32_t BIT_5 : 1;  /*!< Setting this bit masks the interrupt when the rxcrcerror counter
                                                         reaches half the maximumvalue.                                        */
      __I uint32_t BIT_6 : 1;  /*!< The bit is set when the rxalignmenterror counter reaches half
                                                         themaximum value                                                      */
      __I uint32_t BIT_7 : 1;  /*!< Setting this bit masks the interrupt when the rxrunterror counter
                                                         reaches half the maximumvalue.                                        */
      __I uint32_t BIT_8 : 1;  /*!< Setting this bit masks the interrupt when the rxjabbererror
                                                         counter reaches half the maximumvalue.                                */
      __I uint32_t BIT_9 : 1;  /*!< Setting this bit masks the interrupt when the rxundersize_g
                                                         counter reaches half the maximumvalue.                                */
      __I uint32_t BIT_10 : 1; /*!< The bit is set when the rxoversize_g counter reaches half the
                                                         maximumvalue.                                                         */
      __I uint32_t BIT_11 : 1; /*!< Setting this bit masks the interrupt when the rx64octets_gb
                                                         counter reaches half the maximumvalue.                                */
      __I uint32_t BIT_12 : 1; /*!< Setting this bit masks the interruptwhen the rx65to127octets_gb
                                                         counter reaches half themaximum value.                                */
      __I uint32_t BIT_13 : 1; /*!< Setting this bit masks the interrupt when the rx128to255octets_gb
                                                         counter reaches half themaximum value.                                */
      __I uint32_t BIT_14 : 1; /*!< Setting this bit masks the interrupt when the rx256to511octets_gb
                                                         counter reaches half themaximum value                                 */
      __I uint32_t BIT_15 : 1; /*!< Setting this bit masks the interrupt when the rx512to1023octets_gb
                                                         counter reaches half themaximum value.                                */
      __I uint32_t BIT_16 : 1; /*!< Setting this bit masks the interrupt when the rx1024tomaxoctets_gb
                                                         counter reaches half themaximum value.                                */
      __I uint32_t BIT_17 : 1; /*!< Setting this bit masks the interrupt when the rxunicastframes_gb
                                                         counter reaches half themaximum value.                                */
      __I uint32_t BIT_18 : 1; /*!< Setting this bit masks the interrupt when the rx length error
                                                         counter reaches half the maximumvalue.                                */
      __I uint32_t BIT_19 : 1; /*!< Setting this bit masks the interrupt when the rx out of range
                                                         type counter reaches half themaximum value                            */
      __I uint32_t BIT_20 : 1; /*!< Setting this bit masks the interrupt when the rxpauseframes
                                                         counter reaches halfthe maximum value                                 */
      __I uint32_t BIT_21 : 1; /*!< Setting this bit masks the interrupt when the rxfifooverflow
                                                         counter reaches half themaximum value                                 */
      __I uint32_t BIT_22 : 1; /*!< Setting this bit masks the interrupt when the rxvlanframes_gb
                                                         counter reaches halfthe maximum value                                 */
      __I uint32_t BIT_23 : 1; /*!< Setting this bit masks the interrupt when the rxframecount_gb
                                                         counter reaches halfthe maximum value                                 */
    } MMC_RX_INTR_MASK_REG_b;  /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MMC_TX_INTR_MASK_REG; /*!< The MMC Transmit Interrupt Mask register maintains the masks
                                                         for the interrupts generated whentransmit statistic counters
                                                          reach half their maximum value                                       */

    struct {
      __I uint32_t BIT_0 : 1;  /*!< Setting this bit masks the interrupt when the txoctetcount_gb
                                                         counter reaches halfthe maximum value                                 */
      __I uint32_t BIT_1 : 1;  /*!< Setting this bit masks the interrupt when the txframecount_gb
                                                         counter reaches halfthe maximum value                                 */
      __I uint32_t BIT_2 : 1;  /*!< Setting this bit masks the interrupt when the txbroadcastframes_g
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_3 : 1;  /*!< Setting this bit masks the interrupt when the txmulticastframes_g
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_4 : 1;  /*!< Setting this bit masks the interrupt when the tx64to127octets_gb
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_5 : 1;  /*!< Setting this bit masks the interrupt when the tx65to127octets_gb
                                                         counter reaches half the maximum value                                */
      __I uint32_t BIT_6 : 1;  /*!< Setting this bit masks the interrupt when the tx128to255octets_gb
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_7 : 1;  /*!< Setting this bit masks the interrupt when the tx256to511octets_gb
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_8 : 1;  /*!< Setting this bit masks the interrupt when the tx512to1023octets_gb
                                                         counter reaches half the maximum value                                */
      __I uint32_t BIT_9 : 1;  /*!< Setting this bit masks the interrupt when the tx1024tomaxoctets_gb
                                                         counter reaches half the maximum value                                */
      __I uint32_t BIT_10 : 1; /*!< Setting this bit masks the interrupt when the txunicastframes_gb
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_11 : 1; /*!< Setting this bit masks the interrupt when the txmulticastframes_gb
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_12 : 1; /*!< Setting this bit masks the interrupt when the txbroadcastframes_gb
                                                         counter reaches half the maximum value                                */
      __I uint32_t BIT_13 : 1; /*!< Setting this bit masks the interrupt when the txunderflowerror
                                                         counter reaches half the maximum value.                               */
      __I uint32_t BIT_14 : 1; /*!< Setting this bit masks the interrupt when the txsinglecol_g
                                                         counter reaches half the maximum value                                */
      __I uint32_t BIT_15 : 1; /*!< Setting this bit masks the interrupt when the txmulticol_g counter
                                                         reaches half the maximum value                                        */
      __I uint32_t BIT_16 : 1; /*!< Setting this bit masks the interrupt when the txdeferred counter
                                                         reaches half the maximum value                                        */
      __I uint32_t BIT_17 : 1; /*!< Setting this bit masks the interrupt when the txlatecol counter
                                                         reaches half the maximum value                                        */
      __I uint32_t BIT_18 : 1; /*!< Setting this bit masks the interrupt when the txexesscol counter
                                                         reaches half the maximum value.                                       */
      __I uint32_t BIT_19 : 1; /*!< Setting this bit masks the interrupt when the txcarriererror
                                                         counter reaches half the maximum value                                */
      __I uint32_t BIT_20 : 1; /*!< Setting this bit masks the interrupt when the txoctetcount_g
                                                         counter reaches half the maximum value                                */
      __I uint32_t BIT_21 : 1; /*!< Setting this bit masks the interrupt when the txframecount_g
                                                         counter reaches halfthe maximum value.                                */
      __I uint32_t BIT_22 : 1; /*!< Setting this bit masks the interrupt when the txoexcessdef counter
                                                         reaches half themaximum value.                                        */
      __I uint32_t BIT_23 : 1; /*!< Setting this bit masks the interrupt when the txpauseframes
                                                         counter reaches halfthe maximum value.                                */
      __I uint32_t BIT_24 : 1; /*!< Setting this bit masks the interrupt when the txvlanframes_g
                                                         counter reaches halfthe maximum value.                                */
    } MMC_TX_INTR_MASK_REG_b;  /*!< BitSize                                                               */
  };

  __I uint32_t RESERVED3[59];

  union {
    __IO uint32_t MMC_RX_CHOFF_INTR_MASK_REG; /*!< The MMC Receive Checksum Offload Interrupt Mask register maintains
                                                         the masks for the interrupts generated when the receive IPC
                                                          (Checksum Offload) statistic counters reach half their maximum
                                                          value                                                                */

    struct {
      __IO uint32_t BIT_0 : 1;  /*!< Setting this bit masks the interrupt when the rxipv4_gd_frms
                                                         counter reaches halfthe maximum value.                                */
      __IO uint32_t BIT_1 : 1;  /*!< Setting this bit masks the interrupt when the rxipv4_gd_frms
                                                         counter reaches halfthe maximum value.                                */
      __IO uint32_t BIT_2 : 1;  /*!< Setting this bit masks the interrupt when the rxipv4_gd_frms
                                                         counter reaches halfthe maximum value.                                */
      __IO uint32_t BIT_3 : 1;  /*!< Setting this bit masks the interrupt when the rxipv4_frag_frms
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_4 : 1;  /*!< Setting this bit masks the interrupt when the rxipv4_udsbl_frms
                                                         counter reacheshalf the maximum value                                 */
      __IO uint32_t BIT_5 : 1;  /*!< Setting this bit masks the interrupt when the rxipv6_gd_frms
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_6 : 1;  /*!< Setting this bit masks the interrupt when the rxipv6_hdrerr_frms
                                                         counter reacheshalf the maximum value                                 */
      __IO uint32_t BIT_7 : 1;  /*!< Setting this bit masks the interrupt when the rxipv6_nopay_frms
                                                         counter reacheshalf the maximum value.                                */
      __IO uint32_t BIT_8 : 1;  /*!< Setting this bit masks the interrupt when the rxudp_gd_frms
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_9 : 1;  /*!< Setting this bit masks the interrupt when the rxudp_err_frms
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_10 : 1; /*!< Setting this bit masks the interrupt when the rxtcp_gd_frms
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_11 : 1; /*!< Setting this bit masks the interrupt when the rxtcp_err_frms
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_12 : 1; /*!< Setting this bit masks the interrupt when the rxicmp_gd_frms
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_13 : 1; /*!< Setting this bit masks the interrupt when the rxicmp_err_frms
                                                         counter reaches halfthe maximum value.                                */
      uint32_t : 2;
      __IO uint32_t BIT_16 : 1;     /*!< Setting this bit masks the interrupt when the rxipv4_gd_octets
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_17 : 1;     /*!< Setting this bit masks the interrupt when the rxipv4_hdrerr_octets
                                                         counter reacheshalf the maximum value.                                */
      __IO uint32_t BIT_18 : 1;     /*!< Setting this bit masks the interrupt when the rxipv4_nopay_octets
                                                         counter reacheshalf the maximum value.                                */
      __IO uint32_t BIT_19 : 1;     /*!< Setting this bit masks the interrupt when the rxipv4_frag_octets
                                                         counter reacheshalf the maximum value                                 */
      __IO uint32_t BIT_20 : 1;     /*!< Setting this bit masks the interrupt when the rxipv4_udsbl_octets
                                                         counter reacheshalf the maximum value.                                */
      __IO uint32_t BIT_21 : 1;     /*!< Setting this bit masks the interrupt when the rxipv6_gd_octets
                                                         counter reaches halfthe maximum value.                                */
      __IO uint32_t BIT_22 : 1;     /*!< Setting this bit masks the interrupt when the rxipv6_hdrerr_octets
                                                         counter reacheshalf the maximum value.                                */
      __IO uint32_t BIT_23 : 1;     /*!< Setting this bit masks the interrupt when the rxipv6_nopay_octets
                                                         counter reacheshalf the maximum value                                 */
      __IO uint32_t BIT_24 : 1;     /*!< Setting this bit masks the interrupt when the rxudp_gd_octets
                                                         counter reaches halfthe maximum value.                                */
      __IO uint32_t BIT_25 : 1;     /*!< Setting this bit masks the interrupt when the rxudp_err_octets
                                                         counter reaches halfthe maximum value                                 */
      __IO uint32_t BIT_26 : 1;     /*!< Setting this bit masks the interrupt when the rxtcp_gd_octets
                                                         counter reaches halfthe maximum value.                                */
      __IO uint32_t BIT_27 : 1;     /*!< Setting this bit masks the interrupt when the rxtcp_err_octets
                                                         counter reaches halfthe maximum value.                                */
      __IO uint32_t BIT_28 : 1;     /*!< Setting this bit masks the interrupt when the rxicmp_gd_octets
                                                         counter reacheshalf the maximum value                                 */
      __IO uint32_t BIT_29 : 1;     /*!< Setting this bit masks the interrupt when the rxicmp_err_octets
                                                         counter reacheshalf the maximum value.                                */
    } MMC_RX_CHOFF_INTR_MASK_REG_b; /*!< BitSize                                                               */
  };

  __I uint32_t RESERVED4;
  union {
    __I uint32_t MMC_RX_CHOFF_INTR_REG; /*!< The MMC Receive Checksum Offload Interrupt register maintains
                                                         the interrupts generated whenreceive IPC statistic counters
                                                          reach half their maximum values                                      */

    struct {
      __I uint32_t BIT_0 : 1;  /*!< The bit is set when the rxipv4_gd_frms counter reaches half
                                                         the maximum value                                                     */
      __I uint32_t BIT_1 : 1;  /*!< The bit is set when the rxipv4_hdrerr_frms counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_2 : 1;  /*!< The bit is set when the rxipv4_nopay_frms counter reaches half
                                                         the maximumvalue.                                                     */
      __IO uint32_t BIT_3 : 1; /*!< The bit is set when the rxipv4_frag_frms counter reaches half
                                                         the maximumvalue                                                      */
      __I uint32_t BIT_4 : 1;  /*!< The bit is set when the rxipv4_udsbl_frms counter reaches half
                                                         the maximumvalue                                                      */
      __I uint32_t BIT_5 : 1;  /*!< The bit is set when the rxipv6_gd_frms counter reaches half
                                                         the maximum value                                                     */
      __I uint32_t BIT_6 : 1;  /*!< The bit is set when the rxipv6_hdrerr_frms counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_7 : 1;  /*!< The bit is set when the rxipv6_nopay_frms counter reaches half
                                                         the maximumvalue                                                      */
      __I uint32_t BIT_8 : 1;  /*!< The bit is set when the rxudp_gd_frms counter reaches half the
                                                         maximum value                                                         */
      __I uint32_t BIT_9 : 1;  /*!< The bit is set when the rxudp_err_frms counter reaches half
                                                         the maximum value                                                     */
      __I uint32_t BIT_10 : 1; /*!< The bit is set when the rxtcp_gd_frms counter reaches half the
                                                         maximum value                                                         */
      __I uint32_t BIT_11 : 1; /*!< The bit is set when the rxtcp_err_frms counter reaches half
                                                         the maximum value.                                                    */
      __I uint32_t BIT_12 : 1; /*!< The bit is set when the rxicmp_gd_frms counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_13 : 1; /*!< The bit is set when the rxicmp_err_frms counter reaches half
                                                         the maximumvalue.                                                     */
      uint32_t : 2;
      __I uint32_t BIT_16 : 1; /*!< The bit is set when the rxipv4_gd_octets counter reaches half
                                                         the maximumvalue                                                      */
      __I uint32_t BIT_17 : 1; /*!< The bit is set when the rxipv4_hdrerr_octets counter reaches
                                                         half the maximumvalue.                                                */
      __I uint32_t BIT_18 : 1; /*!< The bit is set when the rxipv4_nopay_octets counter reaches
                                                         half the maximumvalue                                                 */
      __I uint32_t BIT_19 : 1; /*!< The bit is set when the rxipv4_frag_octets counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_20 : 1; /*!< The bit is set when the rxipv4_udsbl_octets counter reaches
                                                         half the maximumvalue.                                                */
      __I uint32_t BIT_21 : 1; /*!< The bit is set when the rxipv6_gd_octets counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_22 : 1; /*!< The bit is set when the rxipv6_hdrerr_octets counter reaches
                                                         half the maximumvalue.                                                */
      __I uint32_t BIT_23 : 1; /*!< The bit is set when the rxipv6_nopay_octets counter reaches
                                                         half the maximumvalue.                                                */
      __I uint32_t BIT_24 : 1; /*!< The bit is set when the rxudp_gd_octets counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_25 : 1; /*!< The bit is set when the rxudp_err_octets counter reaches half
                                                         the maximumvalue                                                      */
      __I uint32_t BIT_26 : 1; /*!< The bit is set when the rxtcp_gd_octets counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_27 : 1; /*!< The bit is set when the rxtcp_err_octets counter reaches half
                                                         the maximumvalue.                                                     */
      __I uint32_t BIT_28 : 1; /*!< The bit is set when the rxicmp_gd_octets counter reaches half
                                                         the maximumvalue                                                      */
      __I uint32_t BIT_29 : 1; /*!< The bit is set when the rxicmp_err_octets counter reaches half
                                                         the maximumvalue.                                                     */
    } MMC_RX_CHOFF_INTR_REG_b; /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED5[382];
  ETHERNET_MAC_31_ADDR_Type
    MAC_31_ADDR[15]; /*!< MAC_31_ADDR0                                                          */
  __I uint32_t RESERVED6[481];

  union {
    __IO uint32_t BUS_MODE_REG0; /*!< The Bus Mode register establishes the bus operating modes for
                                                         the DMA.                                                              */

    struct {
      __IO uint32_t SWR : 1; /*!< Software Reset                                                        */
      __IO uint32_t DA : 1;  /*!< DMA Arbitration scheme                                                */
      __IO uint32_t DSL : 5; /*!< Descriptor Skip Length                                                */
      uint32_t : 1;
      __IO uint32_t PBL : 6;      /*!< Programmable Burst Length.These bits indicate the maximum number
                                                         of beats to be transferred in one DMAtransaction                      */
      __IO uint32_t PR : 2;       /*!< Rx:Tx priority ratio                                                  */
      __IO uint32_t FB : 1;       /*!< Fixed Burst                                                           */
      __IO uint32_t RPBL : 6;     /*!< RxDMA PBL                                                             */
      __IO uint32_t USP : 1;      /*!< Use Separate PBL                                                      */
      __IO uint32_t PBL_MODE : 1; /*!< When set high, this bit multiplies the PBL value programmed
                                                         four times                                                            */
      __IO uint32_t AAL : 1;      /*!< Address-Aligned Beats                                                 */
    } BUS_MODE_REG0_b;            /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t TX_POLL_DEMAND_REG1; /*!< The Transmit Poll Demand register enables the Transmit DMA to
                                                         check whether or not the currentdescriptor is owned by DMA            */

    struct {
      __IO uint32_t TPD : 32; /*!< Transmit Poll Demand                                                  */
    } TX_POLL_DEMAND_REG1_b;  /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t RX_POLL_DEMAND_REG; /*!< The Receive Poll Demand register enables the receive DMA to
                                                         check for new descriptors                                             */

    struct {
      __IO uint32_t RPD : 32; /*!< Receive Poll Demand                                                   */
    } RX_POLL_DEMAND_REG_b;   /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t RC_DESC_LIST_ADDR_REG3; /*!< The Receive Descriptor List Address register points to the start
                                                         of the Receive Descriptor List                                        */

    struct {
      __I uint32_t START_RX_LIST : 32; /*!< This field contains the base address of the First Descriptor
                                                         in the Receive Descriptor list.                                       */
    } RC_DESC_LIST_ADDR_REG3_b;        /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t TX_DESC_LIST_ADDR_REG4; /*!< The Receive Descriptor List Address register points to the start
                                                         of the Receive Descriptor List                                        */

    struct {
      __I uint32_t START_TX_LIST : 32; /*!< This field contains the base address of the First Descriptor
                                                         in the Transmit Descriptor list.                                      */
    } TX_DESC_LIST_ADDR_REG4_b;        /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t STATUS_REG5; /*!< The Status register contains all the status bits that the DMA
                                                         reports to the host                                                   */

    struct {
      __IO uint32_t TI : 1;  /*!< Transmit Interrupt.This bit indicates that frame transmission
                                                         is finished and TDES1 31 is set inthe First Descriptor.               */
      __IO uint32_t TPS : 1; /*!< Transmit Process Stopped                                              */
      __IO uint32_t TU : 1;  /*!< Transmit Buffer Unavailable                                           */
      __IO uint32_t TJT : 1; /*!< Transmit Jabber Time out                                              */
      __IO uint32_t OVF : 1; /*!< Receive Overflow                                                      */
      __IO uint32_t UNF : 1; /*!< Transmit Underflow                                                    */
      __IO uint32_t RI : 1;  /*!< Receive Interrupt                                                     */
      __IO uint32_t RU : 1;  /*!< Receive Buffer Unavailable                                            */
      __IO uint32_t RPS : 1; /*!< Receive Process Stopped                                               */
      __IO uint32_t RWT : 1; /*!< Receive Watchdog Time out                                             */
      __IO uint32_t ETI : 1; /*!< Early Transmit Interrupt                                              */
      uint32_t : 2;
      __IO uint32_t FBI : 1;  /*!< Fatal Bus Error Interrupt                                             */
      __IO uint32_t ERI : 1;  /*!< Early Receive Interrupt                                               */
      __IO uint32_t AIS : 1;  /*!< Abnormal Interrupt Summary                                            */
      __IO uint32_t NIS : 1;  /*!< Normal Interrupt Summary                                              */
      __I uint32_t RS : 3;    /*!< Receive Process State                                                 */
      __I uint32_t TS : 3;    /*!< Transmit Process State                                                */
      __I uint32_t EB_23 : 1; /*!< Error Bits.These bits indicate the type of error that caused
                                                         a Bus Error                                                           */
      __I uint32_t EB_24 : 1; /*!< Error Bits.These bits indicate the type of error that caused
                                                         a Bus Error                                                           */
      __I uint32_t EB_25 : 1; /*!< Error Bits.These bits indicate the type of error that caused
                                                         a Bus Error                                                           */
      __I uint32_t GLI : 1;   /*!< GMAC Line interface Interrupt                                         */
      __I uint32_t GMI : 1;   /*!< GMAC MMC Interrupt                                                    */
      __I uint32_t GPI : 1;   /*!< GMAC PMT Interrupt                                                    */
    } STATUS_REG5_b;          /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t OPR_MODE_REG6; /*!< The Operation Mode register establishes the Transmit and Receive
                                                         operating modes and commands                                          */

    struct {
      uint32_t : 1;
      __IO uint32_t SR : 1;  /*!< Start/Stop Receive                                                    */
      __IO uint32_t OSF : 1; /*!< Operate on Second Frame                                               */
      __I uint32_t RTC : 2;  /*!< Receive Threshold Control.These two bits control the threshold
                                                         level of the MTL Receive FIFO.                                        */
      uint32_t : 1;
      __IO uint32_t FUF : 1; /*!< Forward Undersized Good Frames                                        */
      __IO uint32_t FEF : 1; /*!< Forward Error Frames                                                  */
      __IO uint32_t EFC : 1; /*!< Enable HW flow control                                                */
      __IO uint32_t RFA : 2; /*!< Threshold for activating flow control                                 */
      __IO uint32_t RFD : 2; /*!< Threshold for deactivating flow control                               */
      __IO uint32_t ST : 1;  /*!< Start/Stop Transmission Command                                       */
      __IO uint32_t TTC : 3; /*!< Transmit Threshold Control                                            */
      uint32_t : 3;
      __IO uint32_t FTF : 1;   /*!< When this bit is set, the transmit FIFO controller logic is
                                                         reset to its defaultvalues and thus all data in the Tx FIFO
                                                          is lost/flushed                                                      */
      __IO uint32_t TSF : 1;   /*!< Transmit Store and Forward                                            */
      __IO uint32_t RFD_2 : 1; /*!< MSB of Threshold for Deactivating Flow Control                        */
      __IO uint32_t RFA_2 : 1; /*!< MSB of Threshold for Activating Flow Control                          */
      __IO uint32_t DFF : 1;   /*!< Disable Flushing of Received Frames                                   */
      __IO uint32_t RSF : 1;   /*!< Receive Store and Forward                                             */
      __IO uint32_t DT : 1;    /*!< Disable Dropping of TCP/IP Checksum Error Frames                      */
    } OPR_MODE_REG6_b;         /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t IER_REG7; /*!< The Interrupt Enable register enables the interrupts reported
                                                         by Register5                                                          */

    struct {
      __IO uint32_t TIE : 1; /*!< Transmit Interrupt Enable                                             */
      __IO uint32_t TSE : 1; /*!< Transmit Stopped Enable                                               */
      __IO uint32_t TUE : 1; /*!< Transmit Buffer Unavailable Enable                                    */
      __IO uint32_t TJE : 1; /*!< Transmit Jabber Time out Enable                                       */
      __IO uint32_t OVE : 1; /*!< Overflow Interrupt Enable                                             */
      __IO uint32_t UNE : 1; /*!< Underflow Interrupt Enable                                            */
      __IO uint32_t RIE : 1; /*!< Receive Interrupt Enable                                              */
      __IO uint32_t RUE : 1; /*!< Receive Buffer Unavailable Enable                                     */
      __IO uint32_t RSE : 1; /*!< Receive Stopped Enable                                                */
      __IO uint32_t RWE : 1; /*!< Receive Watchdog Time out Enable                                      */
      __IO uint32_t ETE : 1; /*!< Early Transmit Interrupt Enable                                       */
      uint32_t : 2;
      __IO uint32_t FBE : 1; /*!< Fatal Bus Error Enable                                                */
      __IO uint32_t ERE : 1; /*!< Early Receive Interrupt Enable                                        */
      __IO uint32_t AIE : 1; /*!< Abnormal Interrupt Summary Enable                                     */
      __IO uint32_t NIE : 1; /*!< Normal Interrupt Summary Enable                                       */
    } IER_REG7_b;            /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t MFBOC_REG8; /*!< The Interrupt Enable register enables the interrupts reported
                                                         by Register5                                                          */

    struct {
      __IO uint32_t TIE : 16;              /*!< Indicates the number of frames missed by the controller due
                                                         to the Host ReceiveBuffer being unavailable                           */
      __IO uint32_t OVERFLOW_BIT_M : 1;    /*!< Overflow bit for Missed Frame Counter                                 */
      __IO uint32_t MISSEED_FRAMES : 11;   /*!< Indicates the number of frames missed by the application              */
      __IO uint32_t OVERFLOW_BIT_FIFO : 1; /*!< Overflow bit for Missed Frame Counter                                 */
    } MFBOC_REG8_b;                        /*!< BitSize                                                               */
  };
  __I uint32_t RESERVED7[9];
  __I uint32_t CHTX_DESC_REG18;     /*!< Host Transmit Descriptor Address Pointer Cleared on Reset. Pointer
                                                         updated by DMA during operationBuffer being unavailable               */
  __I uint32_t CHRX_DESC_REG19;     /*!< Host Receive Descriptor Address PointerCleared on Reset. Pointer
                                                         updated by DMA during operation                                       */
  __I uint32_t CHRX_DESC_REG20;     /*!< Host Transmit Buffer Address PointerCleared on Reset. Pointer
                                                         updated by DMA during operation                                       */
  __I uint32_t CHRX_BUF_ADDR_REG21; /*!< Host Receive Buffer Address PointerCleared on Reset. Pointer
                                                         updated by DMA during operation                                       */
} ETHERNET_Type;

/* =========================================================================================================================== */
/* ================                                           EGPIO                                           ================ */
/* =========================================================================================================================== */

/**
  * @brief ENHANCED GENERAL PERPOSE INPUT/OUTPUT (EGPIO)
  */

typedef struct { /*!< (@ 0x46130000) EGPIO Structure                                            */
  __IOM EGPIO_PIN_CONFIG_Type
    PIN_CONFIG[80]; /*!< (@ 0x00000000) [0..79]                                                    */
  __IM uint32_t RESERVED[704];
  __IOM EGPIO_PORT_CONFIG_Type
    PORT_CONFIG[6]; /*!< (@ 0x00001000) [0..5]                                                     */
  __IM uint32_t RESERVED1[80];
  __IOM EGPIO_INTR_Type INTR[6]; /*!< (@ 0x00001200) [0..5]                                                     */
  __IM uint32_t RESERVED2[4];
  __IOM EGPIO_GPIO_GRP_INTR_Type
    GPIO_GRP_INTR[4]; /*!< (@ 0x00001240) [0..3]                                                   */
} EGPIO_Type;         /*!< Size = 4704 (0x1260)                                                      */

typedef struct { /*!<  SDIO structure           */

  union {
    __IO uint32_t SDIO_INTR_FN1_STATUS_CLEAR_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_CLR : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 Interrupt is enabled =0 - Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_CLR : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_CLR : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_CLR : 1;   /*!<  This bit is used to enable CMD52 interrupt.   */
      __IO uint32_t SDIO_PWR_LEV_INT_CLR : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_CLR : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_CLR : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_CLR : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN1_STATUS_CLEAR_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN1_ENABLE_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_EN : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 Interrupt is enabled =0 - Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_EN : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_EN : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_EN : 1;   /*!<  This bit is used to enable CMD52 interrupt.   */
      __IO uint32_t SDIO_PWR_LEV_INT_EN : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_EN : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_EN : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_EN : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN1_ENABLE_REG_b;
  };
  union {
    __IO uint32_t SDIO_INTR_FN1_MASK_REG; /*!<  SDIO Function1 Interrupt Mask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_MSK : 1; /*!<  This bit is used to mask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_MSK : 1; /*!<  This bit is used to mask CMD53 read interrupt */
      __IO uint32_t
        SDIO_CSA_MSK : 1; /*!<  This bit is used to mask CMD53 CSA interrupt.Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_CMD52_MSK : 1;   /*!<  This bit is used to mask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_MSK : 1; /*!<  This bit is used to mask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_MSK : 1; /*!<  This bit is used to mask CRC error interrupt */
      __IO uint32_t
        SDIO_ABORT_MSK : 2; /*!<  This bit is used to mask abort interrupt Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_TOUT_MSK : 1; /*!<  This bit is used to mask "read FIFO wait time over" interrupt */
    } SDIO_INTR_FN1_MASK_REG_b;
  };
  union {
    __IO uint32_t SDIO_INTR_FN1_UNMASK_REG; /*!<  SDIO Function1 Interrupt Unmask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_UNMSK : 1;     /*!<  This bit is used to unmask CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_UNMSK : 1;   /*!<  This bit is used to unmask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_UNMSK : 1; /*!<  This bit is used to unmask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_UNMSK : 1; /*!<  This bit is used to unmask CRC error interrupt. */
      __IO uint32_t SDIO_ABORT_UNMSK : 2;   /*!<  This bit is used to unmask abort interrupt */
      __IO uint32_t SDIO_TOUT_UNMSK : 1;    /*!<  This bit is used to unmask ?read FIFO wait time over? interrupt */
    } SDIO_INTR_FN1_UNMASK_REG_b;
  };
  union {
    __I uint32_t SDIO_BLK_LEN_REG; /*!< SDIO Block Length Register */
    struct {
      __I uint32_t SDIO_BLK_LEN : 12; /*!< Length of each block for the last received CMD53 */
      uint32_t : 4;                   /*!<  reseerved for future use */
    } SDIO_BLK_LEN_REG_b;
  };

  union {
    __I uint32_t SDIO_BLK_CNT_REG; /*!<SDIO Block Count Register */
    struct {
      __I uint32_t SDIO_BLK_CNT : 9; /*!< Block count for the last received CMD53 */
      uint32_t : 7;                  /*!< Reserved for future use. */
    } SDIO_BLK_CNT_REG_b;
  };

  union {
    __I uint32_t SDIO_ADDRESS_REG; /*!< SDIO Address Register */
    struct {
      __I uint32_t SDIO_ADDR : 16; /*!< Lower 16-bits of the 17-bit address field in the last received CMD53 */

    } SDIO_ADDRESS_REG_b;
  };
  __IO uint32_t SDIO_CMD52_RDATA_REGISTER;

  __IO uint32_t SDIO_CMD52_WDATA_REGISTER;

  union {
    __I uint32_t SDIO_INTR_STATUS_REG; /*!<  SDIO Interrupt Function Number Register */
    struct {
      __I uint32_t SDIO_INTR_FN_NUM : 3; /*!< Indicates the function number to which interrupt is pending.
                                                         This register is provided to enable the software to decode the interrupt register easily.
                                                         Once this interrupt is cleared this register gets the next function number to which interrupt is pending (if simultaneous interrupts are pending).
                                                         2 \96 function 2 3 \96 function 3 4 \96 function 4 5 \96 function 5 0 \96 there is no pending interrupt */
      uint32_t : 13;                     /*!<  reserved */
    } SDIO_INTR_STATUS_REG_b;
  };

  union {
    __I uint32_t SDIO_INTR_FN_NUMBER_REG; /*!<  SDIO Interrupt Function Number Register */
    struct {
      __I uint32_t SDIO_INTR_FN_NUM : 3; /*!< Indicates the function number to which interrupt is pending.
                                                         This register is provided to enable the software to decode the interrupt register easily.
                                                         Once this interrupt is cleared this register gets the next function number to which interrupt is pending (if simultaneous interrupts are pending).
                                                         2 \96 function 2 3 \96 function 3 4 \96 function 4 5 \96 function 5 0 \96 there is no pending interrupt */
      uint32_t : 13;                     /*!<  reserved */
    } SDIO_INTR_FN_NUMBER_REG_b;
  };

  union {
    __IO uint32_t SDIO_FIFO_STATUS_REG; /*!< SDIO FIFO Status Register */
    struct {
      __I uint32_t SDIO_WFIFO_FULL : 1;     /*!< When set, indicates the WFIFO is almost full*/
      __I uint32_t SDIO_WFIFO_AFULL : 1;    /*!< When set, indicates that WFIFO is almost full.*/
      __I uint32_t SDIO_RFIFO_EMPTY : 1;    /*!< When set, indicates the RFIFO is empty */
      __I uint32_t SDIO_RFIFO_AEMPTY : 1;   /*!< When set, indicates that RFIFO is almost empty.*/
      __I uint32_t SDIO_CURRENT_FN_NUM : 3; /*!< Indicates the function number of the last received command */
      __I uint32_t
        SDIO_BUS_CONTROL_STATE : 5; /*!< SDIO bus control state 7 \96 When set, indicates FSM is in idle state 8 \96 When set, indicates FSM is in CMD52 read state 9 \96 When set, indicates FSM is in CMD52 write state 10 \96 When set, indicates FSM is
                                                         CMD53 read state 11 \96 When set, indicates FSM is CMD53 write state */
    } SDIO_FIFO_STATUS_REG_b;
  };

  union {
    __I uint32_t SDIO_FIFO_OCC_REG; /*!< SDIO FIFO Occupancy Register */
    struct {
      __I uint32_t SDIO_WFIFO_OCC : 8;   /*!<  Indicates the occupancy level of the write FIFO */
      __I uint32_t SDIO_RFIFO_AVAIL : 8; /*!<  Indicates the available space in the read FIFO */
    } SDIO_FIFO_OCC_REG_b;
  };

  union {
    __I uint32_t SDIO_HOST_INTR_SET_REG; /*!< SDIO Host Interrupt Set Register */
    struct {
      __I uint32_t SDIO_INTSET_FN2 : 1; /*!< This bit is used to raise an interrupt to host for function2. */
      __I uint32_t SDIO_INTSET_FN3 : 1; /*!< This bit is used to raise an interrupt to host for function3 */
      __I uint32_t SDIO_INTSET_FN4 : 1; /*!< This bit is used to raise an interrupt to host for function4 */
      __I uint32_t SDIO_INTSET_FN5 : 1; /*!< This bit is used to raise an interrupt to host for function5 */
      uint32_t : 12;                    /*!< reserved */
    } SDIO_HOST_INTR_SET_REG_b;
  };

  union {
    __I uint32_t SDIO_HOST_INTR_CLEAR_REG; /*!<  SDIO Host Interrupt Clear Register */
    struct {
      __I uint32_t SDIO_INTCLR_FN2 : 1; /*!<  This bit is used to clear the interrupt to host for function2 */
      __I uint32_t SDIO_INTCLR_FN3 : 1; /*!<  This bit is used to clear the interrupt to host from function3 */
      __I uint32_t SDIO_INTCLR_FN4 : 1; /*!<  This bit is used to clear the interrupt to host for function4.*/
      __I uint32_t SDIO_INTCLR_FN5 : 1; /*!< This bit is used to clear the interrupt to host for function5 */
      uint32_t : 12;
    } SDIO_HOST_INTR_CLEAR_REG_b;
  };

  __IO uint32_t RESERVED;
  __O uint32_t SDIO_RFIFO_DATA_REG[256]; /*!< SDIO Read FIFO Data Register  */

  __I uint32_t SDIO_WFIFO_DATA_REG[256]; /*!< SDIO Write FIFO Data Register   */

  union {
    __IO uint32_t SDIO_INTR_FN2_STATUS_CLEAR_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_CLR : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 Interrupt is enabled =0 - Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_CLR : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_CLR : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_CLR : 1;   /*!<  This bit is used to enable CMD52 interrupt.   */
      __IO uint32_t SDIO_PWR_LEV_INT_CLR : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_CLR : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_CLR : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_CLR : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN2_STATUS_CLEAR_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN2_ENABLE_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_EN : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1\91- Interrupt is enabled =0\91- Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_EN : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_EN : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_EN : 1;   /*!<  This bit is used to enable CMD52 interrupt*/
      __IO uint32_t SDIO_PWR_LEV_INT_EN : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_EN : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_EN : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_EN : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN2_ENABLE_REG_b;
  };
  union {
    __IO uint32_t SDIO_INTR_FN2_MASK_REG; /*!<  SDIO Function1 Interrupt Mask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_MSK : 1; /*!<  This bit is used to mask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_MSK : 1; /*!<  This bit is used to mask CMD53 read interrupt */
      __IO uint32_t
        SDIO_CSA_MSK : 1; /*!<  This bit is used to mask CMD53 CSA interrupt.Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_CMD52_MSK : 1;   /*!<  This bit is used to mask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_MSK : 1; /*!<  This bit is used to mask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_MSK : 1; /*!<  This bit is used to mask CRC error interrupt */
      __IO uint32_t
        SDIO_ABORT_MSK : 2; /*!<  This bit is used to mask abort interrupt Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_TOUT_MSK : 1; /*!<  This bit is used to mask "read FIFO wait time over" interrupt */
    } SDIO_INTR_FN2_MASK_REG_b;
  };
  union {
    __IO uint32_t SDIO_INTR_FN2_UNMASK_REG; /*!<  SDIO Function1 Interrupt Unmask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_UNMSK : 1;     /*!<  This bit is used to unmask CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_UNMSK : 1;   /*!<  This bit is used to unmask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_UNMSK : 1; /*!<  This bit is used to unmask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_UNMSK : 1; /*!<  This bit is used to unmask CRC error interrupt. */
      __IO uint32_t SDIO_ABORT_UNMSK : 2;   /*!<  This bit is used to unmask abort interrupt */
      __IO uint32_t SDIO_TOUT_UNMSK : 1;    /*!<  This bit is used to unmask ?read FIFO wait time over? interrupt */
    } SDIO_INTR_FN2_UNMASK_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN3_STATUS_CLEAR_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_CLR : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 Interrupt is enabled =0 - Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_CLR : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_CLR : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_CLR : 1;   /*!<  This bit is used to enable CMD52 interrupt.   */
      __IO uint32_t SDIO_PWR_LEV_INT_CLR : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_CLR : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_CLR : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_CLR : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN3_STATUS_CLEAR_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN3_ENABLE_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_EN : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1\91- Interrupt is enabled =0\91- Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_EN : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_EN : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_EN : 1;   /*!<  This bit is used to enable CMD52 interrupt*/
      __IO uint32_t SDIO_PWR_LEV_INT_EN : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_EN : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_EN : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_EN : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN3_ENABLE_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN3_MASK_REG; /*!<  SDIO Function1 Interrupt Mask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_MSK : 1; /*!<  This bit is used to mask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_MSK : 1; /*!<  This bit is used to mask CMD53 read interrupt */
      __IO uint32_t
        SDIO_CSA_MSK : 1; /*!<  This bit is used to mask CMD53 CSA interrupt.Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_CMD52_MSK : 1;   /*!<  This bit is used to mask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_MSK : 1; /*!<  This bit is used to mask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_MSK : 1; /*!<  This bit is used to mask CRC error interrupt */
      __IO uint32_t
        SDIO_ABORT_MSK : 2; /*!<  This bit is used to mask abort interrupt Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_TOUT_MSK : 1; /*!<  This bit is used to mask "read FIFO wait time over" interrupt */
    } SDIO_INTR_FN3_MASK_REG_b;
  };
  union {
    __IO uint32_t SDIO_INTR_FN3_UNMASK_REG; /*!<  SDIO Function1 Interrupt Unmask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_UNMSK : 1;     /*!<  This bit is used to unmask CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_UNMSK : 1;   /*!<  This bit is used to unmask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_UNMSK : 1; /*!<  This bit is used to unmask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_UNMSK : 1; /*!<  This bit is used to unmask CRC error interrupt. */
      __IO uint32_t SDIO_ABORT_UNMSK : 2;   /*!<  This bit is used to unmask abort interrupt */
      __IO uint32_t SDIO_TOUT_UNMSK : 1;    /*!<  This bit is used to unmask ?read FIFO wait time over? interrupt */
    } SDIO_INTR_FN3_UNMASK_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN4_STATUS_CLEAR_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_CLR : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 Interrupt is enabled =0 - Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_CLR : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_CLR : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_CLR : 1;   /*!<  This bit is used to enable CMD52 interrupt.   */
      __IO uint32_t SDIO_PWR_LEV_INT_CLR : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_CLR : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_CLR : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_CLR : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN4_STATUS_CLEAR_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN4_ENABLE_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_EN : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 - Interrupt is enabled =0 -Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_EN : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_EN : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_EN : 1;   /*!<  This bit is used to enable CMD52 interrupt*/
      __IO uint32_t SDIO_PWR_LEV_INT_EN : 1; /*!<  This bit is used to enable power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_INT_EN : 1; /*!<  This bit is used to enable CRC error interrupt*/
      __IO uint32_t SDIO_ABORT_INT_EN : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_EN : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt */
    } SDIO_INTR_FN4_ENABLE_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN4_MASK_REG; /*!<  SDIO Function1 Interrupt Mask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_MSK : 1; /*!<  This bit is used to mask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_MSK : 1; /*!<  This bit is used to mask CMD53 read interrupt */
      __IO uint32_t
        SDIO_CSA_MSK : 1; /*!<  This bit is used to mask CMD53 CSA interrupt.Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_CMD52_MSK : 1;   /*!<  This bit is used to mask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_MSK : 1; /*!<  This bit is used to mask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_MSK : 1; /*!<  This bit is used to mask CRC error interrupt */
      __IO uint32_t
        SDIO_ABORT_MSK : 2; /*!<  This bit is used to mask abort interrupt Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_TOUT_MSK : 1; /*!<  This bit is used to mask "read FIFO wait time over" interrupt */
    } SDIO_INTR_FN4_MASK_REG_b;
  };
  union {
    __IO uint32_t SDIO_INTR_FN4_UNMASK_REG; /*!<  SDIO Function1 Interrupt Unmask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_UNMSK : 1;     /*!<  This bit is used to unmask CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_UNMSK : 1;   /*!<  This bit is used to unmask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_UNMSK : 1; /*!<  This bit is used to unmask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_UNMSK : 1; /*!<  This bit is used to unmask CRC error interrupt. */
      __IO uint32_t SDIO_ABORT_UNMSK : 2;   /*!<  This bit is used to unmask abort interrupt */
      __IO uint32_t SDIO_TOUT_UNMSK : 1;    /*!<  This bit is used to unmask ?read FIFO wait time over? interrupt */
    } SDIO_INTR_FN4_UNMASK_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN5_STATUS_CLEAR_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_CLR : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 Interrupt is enabled =0 - Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_CLR : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_CLR : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_CLR : 1;   /*!<  This bit is used to enable CMD52 interrupt.   */
      __IO uint32_t SDIO_PWR_LEV_INT_CLR : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_CLR : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_CLR : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_CLR : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN5_STATUS_CLEAR_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN5_ENABLE_REG; /*!<  SDIO Function1 Interrupt Enable Register   */

    struct {
      __IO uint32_t
        SDIO_WR_INT_EN : 1; /*!<  This bit is used to enable CMD53 write interrupt. =1 Interrupt is enabled =0\91- Interrupt is disabled */
      __IO uint32_t SDIO_RD_INT_EN : 1;      /*!<  This bit is used to enable CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_INT_EN : 1;     /*!<  This bit is used to enable CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_INT_EN : 1;   /*!<  This bit is used to enable CMD52 interrupt*/
      __IO uint32_t SDIO_PWR_LEV_INT_EN : 1; /*!<  This bit is used to enable power level change interrupt. */
      __IO uint32_t SDIO_CRC_ERR_INT_EN : 1; /*!<  This bit is used to enable CRC error interrupt.*/
      __IO uint32_t SDIO_ABORT_INT_EN : 1;   /*!<  This bit is used to enable abort interrupt   */
      __IO uint32_t SDIO_TOUT_INT_EN : 1;    /*!<  This bit is used to enable ?read FIFO wait time over? interrupt. */
    } SDIO_INTR_FN5_ENABLE_REG_b;
  };

  union {
    __IO uint32_t SDIO_INTR_FN5_MASK_REG; /*!<  SDIO Function1 Interrupt Mask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_MSK : 1; /*!<  This bit is used to mask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_MSK : 1; /*!<  This bit is used to mask CMD53 read interrupt */
      __IO uint32_t
        SDIO_CSA_MSK : 1; /*!<  This bit is used to mask CMD53 CSA interrupt.Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_CMD52_MSK : 1;   /*!<  This bit is used to mask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_MSK : 1; /*!<  This bit is used to mask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_MSK : 1; /*!<  This bit is used to mask CRC error interrupt */
      __IO uint32_t
        SDIO_ABORT_MSK : 2; /*!<  This bit is used to mask abort interrupt Setting this bit will mask the interrupt Clearing this bit has no effect */
      __IO uint32_t SDIO_TOUT_MSK : 1; /*!<  This bit is used to mask "read FIFO wait time over" interrupt */
    } SDIO_INTR_FN5_MASK_REG_b;
  };
  union {
    __IO uint32_t SDIO_INTR_FN5_UNMASK_REG; /*!<  SDIO Function1 Interrupt Unmask Register */
    struct {
      __IO uint32_t SDIO_WR_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 write interrupt. */
      __IO uint32_t SDIO_RD_INT_UNMSK : 1;  /*!<  This bit is used to unmask CMD53 read interrupt */
      __IO uint32_t SDIO_CSA_UNMSK : 1;     /*!<  This bit is used to unmask CMD53 CSA interrupt */
      __IO uint32_t SDIO_CMD52_UNMSK : 1;   /*!<  This bit is used to unmask CMD52 interrupt */
      __IO uint32_t SDIO_PWR_LEV_UNMSK : 1; /*!<  This bit is used to unmask power level change interrupt */
      __IO uint32_t SDIO_CRC_ERR_UNMSK : 1; /*!<  This bit is used to unmask CRC error interrupt. */
      __IO uint32_t SDIO_ABORT_UNMSK : 2;   /*!<  This bit is used to unmask abort interrupt */
      __IO uint32_t SDIO_TOUT_UNMSK : 1;    /*!<  This bit is used to unmask ?read FIFO wait time over? interrupt */
    } SDIO_INTR_FN5_UNMASK_REG_b;
  };
  union {
    __IO uint32_t SDIO_ERROR_COND_CHK_ENABLE_REG; /*!<SDIO error condition check enable register  */
    struct {
      __IO uint32_t
        SDIO_CRC_EN : 1; /*!<  When set, stops the DMA from doing data accesses till CRC error interrupt is cleared */
      __IO uint32_t
        SDIO_ABORT_EN : 1; /*!<  When set, stops the DMA from doing data accesses till ABORT interrupt is cleared */
      __IO uint32_t
        SDIO_SPI_RD_DATA_ERROR_EN : 1; /*!< When set, stops the DMA from doing data accesses till read data error interrupt is cleared in SPI mode  */
      uint32_t : 13;
    } SDIO_ERROR_COND_CHK_ENABLE_REG_b;
  };

  union {
    __IO uint32_t SDIO_ERROR_COND_STATE_REG; /*!< SDIO error condition state register  */
    struct {
      __IO uint32_t SDIO_ERROR_BYTE_CNT : 12; /*!< Indicates byte count when one of the error condition occurred*/
      uint32_t : 4;                           /*!< reserved  */
      __IO uint32_t SDIO_ERROR_BLK_CNT : 1;   /*!< Indicates block count when one of error condition occurred */
      uint32_t : 9;                           /*!< reserved*/
    } SDIO_ERROR_COND_STATE_REG_b;
  };

  union {
    __I uint32_t SDIO_BOOT_CONFIG_VALS_0_REG; /*!< SDIO Boot Config Values Register 0  */
    struct {
      __I uint32_t OCR_R : 24; /*!< Operating conditions. The value written by bootloader can be read here  */
      __I uint32_t
        CSA_MSBYTE : 24; /*!< MS byre of CSA address. Lower 24 bits of CSA will come through SDIO CSA registers. Whenever CSA access is done,
                                                         32-bit address will be prepared using these fields.   */
    } SDIO_BOOT_CONFIG_VALS_0_REG_b;
  };

  union {
    __I uint32_t SDIO_BOOT_CONFIG_VALS_1_REG; /*!< SDIO Boot Config Values Register 1  */
    struct {
      __I uint32_t
        NO_OF_IO_FUNCTIONS : 3; /*!< Indicates number functions supported. The value written by bootloader can be read here.  */
      __I uint32_t COMBOCARD : 1; /*!< When set, combo mode will be enabled  */
      __I uint32_t
        SDMEM_IGNOTRE_SDMEM_PRESENT : 1; /*!< When set, sdmem_present signal, coming from GPIO, will be ignored.  */
      __I uint32_t
        ADMEM_DRIVE_HIZ_MB_READ : 1; /*!< When set, High will be driven in the second cycle of interrupt period during sd memory mb read transfer */
      __I uint32_t
        SDMEM_DISABLE_INTERRUPT_MB_READ : 1; /*!< When set, interrupt will be not be driven during sd memory mb read transfer */
      __I uint32_t
        IGNORE_DISABLE_HS : 1; /*!< if ignore_disable_hs is set, sdmem_disable_high_speed_switching coming from combo mode module is ignored  */
      uint32_t : 24;
    } SDIO_BOOT_CONFIG_VALS_1_REG_b;
  };
} SDIO0_Type;

/* ================================================================================ */
/* ================              SPI SLAVE register map             ================ */
/* ================================================================================ */
typedef struct {

  union {
    __IO uint16_t SPI_HOST_INTR; /*!< SPI Host interupt resgister */
    struct {
      __IO uint16_t SPI_HOST_INTR : 8; /*!< These bits indicate the interrupt vector */
    } SPI_HOST_INTR_b;
  };

  union {
    __IO uint16_t SPI_RFIFO_START; /*!< SPI FIFO start Level Register */
    struct {
      __IO uint16_t SPI_RFIFO_ST : 8; /*!< Minimum FIFO occupancy level */
    } SPI_RFIFO_START_b;
  };

  __IO uint16_t SPI_RFIFO_AFULL_LEV;  /*!< TODO */
  __IO uint16_t SPI_RFIFO_AEMPTY_LEV; /*!< TODO */

  union {
    __IO uint16_t SPI_MODE; /*!< SPI mode Register */
    struct {
      __IO uint16_t SPEED_MODE : 1; /*!< This bit enables the SPI high speed mode
                                                                          0 - Low speed mode
                                                                          1 - High speed mode
			 */
      __IO uint16_t FIX_EN : 1;
    } SPI_MODE_b;
  };

  union {
    __IO uint16_t SPI_INTR_STATUS; /*!< SPI interrupt status register*/
    struct {
      __IO uint16_t SPI_WR_REQ : 1;      /*!< Write request received interrupt */
      __IO uint16_t SPI_RD_REQ : 1;      /*!< Read request received interrupt  */
      __IO uint16_t SPI_CS_DEASSERT : 1; /*!< SPI chip deassert interrupt      */

    } SPI_INTR_STATUS_b;
  };

  union {
    __IO uint16_t SPI_INTR_EN; /*!< SPI interrupt enable register*/
    struct {
      __IO uint16_t SPI_WR_INT_EN : 1;          /*!< SPI chip deassert  interrupt enable  */
      __IO uint16_t SPI_RD_INT_EN : 1;          /*!< Read request received interrupt enable */
      __IO uint16_t SPI_CS_DEASSERT_INT_EN : 1; /*!< Write request received interrupt enable */
    } SPI_INTR_EN_b;
  };

  union {
    __IO uint16_t SPI_INTR_MASK; /*!< SPI interrupt Mask register*/
    struct {
      __IO uint16_t SPI_WR_INT_MASK : 1;          /*!< Mask SPI chip deassert interrupt  */
      __IO uint16_t SPI_RD_INT_MASK : 1;          /*!< Mask Read request received interrupt */
      __IO uint16_t SPI_CS_DEASSERT_INT_MASK : 1; /*!< Mask write request received interrupt */
    } SPI_INTR_MASK_b;
  };

  union {
    __IO uint16_t SPI_INTR_UNMASK; /*!< SPI interrupt unmask register*/
    struct {
      __IO uint16_t SPI_WR_INT_UNMASK : 1;          /*!< Unmask SPI chip deassert interrupt  */
      __IO uint16_t SPI_RD_INT_UNMASK : 1;          /*!< Unmask Read request received interrupt */
      __IO uint16_t SPI_CS_DEASSERT_INT_UNMASK : 1; /*!< Unmask Write request received interrupt */
    } SPI_INTR_UNMASK_b;
  };

  __I uint16_t SPI_LENGTH;

  union {
    __I uint16_t SPI_COMMAND;
    struct {
      __I uint16_t SPI_CMD_C2 : 8;
      __I uint16_t SPI_CMD_C1 : 8;
    } SPI_COMMAND_b;
  };

  __I uint16_t SPI_DEV_ID;  /*!< SPI device ID */
  __I uint16_t SPI_VERSION; /*!< SPI version number*/

  union {
    __I uint16_t SPI_STATUS;
    struct {
      __I uint16_t SPI_RFIFO_FULL : 1;   /*!< Indicates Read FIFO is full
                                                                         0 -Not Full , 1 - Full  */
      __I uint16_t SPI_RFIFO_AFULL : 1;  /*!< Indicates Read FIFO is lomost full
                                                                          0 -Not almost Full , 1 - almost Full  */
      __I uint16_t SPI_WFIFO_EMPTY : 1;  /*!< Indicates Write FIFO is empty
                                                                         0 - write FIFO is empty , 1 - Write FIFO is not empty  */
      __I uint16_t SPI_WFIFO_AEMPTY : 1; /*!< Indicates Write FIFO is almost full
                                                                          0 - FIFO is alomost empty , 1 - FIFO is not almost empty  */
    } SPI_STATUS_b;
  };

  union {
    __I uint16_t SPI_BC_STATE;
    struct {
      __I uint16_t BC_IDLE : 1;
      __I uint16_t BC_EN : 1;
      __I uint16_t BC_CTRL_BUSY : 1;
      __I uint16_t BC_INT_REG_RD : 1;
      __I uint16_t BC_INT_REG_FIFO_WR : 1;
      __I uint16_t BC_INT_REG_WR_WAIT : 1;
      __I uint16_t BC_INT_REG_FIFO_RD : 1;
      __I uint16_t BC_BC_INT_REG_WR : 1;
      __I uint16_t BC_AHB_MASTER : 1;
      __I uint16_t AHB_MASTER_WAIT : 1;
      __I uint16_t AHB_SLAVE_WR_LEN : 1;
      __I uint16_t AHB_SLAVE_WR_CMD : 1;
      __I uint16_t AHB_SLAVE_WR_INTR : 1;
      __I uint16_t BC_AHB_SLAVE_WAIT : 1;
    } SPI_BC_STATE_b;
  };

  __I uint16_t RESERVED[8];

} RSI_SSPI_T;

/*POWER AND CLOCK REGISTER */

/* ================================================================================ */
/* ================                     M4_PLL                     ================ */
/* ================================================================================ */

/**
 * @brief Real timer is used in counting clocks, micro seconds, milli seconds, seconds and minutes with both ref clock and system (SoC) clock (M4_PLL)
 */

typedef struct { /*!< M4_PLL Structure                                                      */
  /*0x00*/
  union {
    __IO uint32_t CLK_ENABLE_SET_REG1; /*!< none                                                                  */
    struct {

      __IO uint32_t USART1_PCLK_ENABLE_b : 1;
      __IO uint32_t USART1_SCLK_ENABLE_b : 1;
      __IO uint32_t USART2_PCLK_ENABLE_b : 1;
      __IO uint32_t USART2_SCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t RESERVED4 : 1;
      __IO uint32_t CT_CLK_ENABLE_b : 1;
      __IO uint32_t CT_PCLK_ENABLE_b : 1;
      __IO uint32_t ICACHE_CLK_ENABLE_b : 1;
      __IO uint32_t ICACHE_CLK_2X_ENABLE_b : 1;
      __IO uint32_t RPDMA_HCLK_ENABLE_b : 1;
      __IO uint32_t SOC_PLL_SPI_CLK_ENABLE_b : 1;
      __IO uint32_t RESERVED5 : 1;
      __IO uint32_t IID_CLK_ENABLE_b : 1;
      __IO uint32_t SDIO_SYS_HCLK_ENABLE_b : 1;
      __IO uint32_t CRC_CLK_ENABLE_b : 1;
      __IO uint32_t M4SS_UM_CLK_STATIC_EN_b : 1;
      __IO uint32_t RESERVED6 : 1;
      __IO uint32_t ETH_HCLK_ENABLE_b : 1;
      __IO uint32_t HWRNG_PCLK_ENABLE_b : 1;
      __IO uint32_t GNSS_MEM_CLK_ENABLE_b : 1;
      __IO uint32_t CCI_PCLK_ENABLE_b : 1;
      __IO uint32_t CCI_HCLK_ENABLE_b : 1;
      __IO uint32_t CCI_CLK_ENABLE_b : 1;
      __IO uint32_t MASK_HOST_CLK_WAIT_FIX_b : 1;
      __IO uint32_t MASK31_HOST_CLK_CNT_b : 1;
      __IO uint32_t SD_MEM_INTF_CLK_ENABLE_b : 1;
      __IO uint32_t MASK_HOST_CLK_AVAILABLE_FIX_b : 1;
      __IO uint32_t ULPSS_CLK_ENABLE_b : 1;
    } CLK_ENABLE_SET_REG1_b; /*!< BitSize                                                               */
  };
  /*0x04*/
  union {
    __IO uint32_t CLK_ENABLE_CLEAR_REG1; /*!< none                                                                  */

    struct {
      __IO uint32_t USART1_PCLK_ENABLE_b : 1;
      __IO uint32_t USART1_SCLK_ENABLE_b : 1;
      __IO uint32_t USART2_PCLK_ENABLE_b : 1;
      __IO uint32_t USART2_SCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t RESERVED4 : 1;
      __IO uint32_t CT_CLK_ENABLE_b : 1;
      __IO uint32_t CT_PCLK_ENABLE_b : 1;
      __IO uint32_t ICACHE_CLK_ENABLE_b : 1;
      __IO uint32_t ICACHE_CLK_2X_ENABLE_b : 1;
      __IO uint32_t RPDMA_HCLK_ENABLE_b : 1;
      __IO uint32_t SOC_PLL_SPI_CLK_ENABLE_b : 1;
      __IO uint32_t RESERVED5 : 1;
      __IO uint32_t IID_CLK_ENABLE_b : 1;
      __IO uint32_t SDIO_SYS_HCLK_ENABLE_b : 1;
      __IO uint32_t CRC_CLK_ENABLE_b : 1;
      __IO uint32_t M4SS_UM_CLK_STATIC_EN_b : 1;
      __IO uint32_t RESERVED6 : 1;
      __IO uint32_t ETH_HCLK_ENABLE_b : 1;
      __IO uint32_t HWRNG_PCLK_ENABLE_b : 1;
      __IO uint32_t GNSS_MEM_CLK_ENABLE_b : 1;
      __IO uint32_t CCI_PCLK_ENABLE_b : 1;
      __IO uint32_t CCI_HCLK_ENABLE_b : 1;
      __IO uint32_t CCI_CLK_ENABLE_b : 1;
      __IO uint32_t MASK_HOST_CLK_WAIT_FIX_b : 1;
      __IO uint32_t MASK31_HOST_CLK_CNT_b : 1;
      __IO uint32_t SD_MEM_INTF_CLK_ENABLE_b : 1;
      __IO uint32_t MASK_HOST_CLK_AVAILABLE_FIX_b : 1;
      __IO uint32_t ULPSS_CLK_ENABLE_b : 1;

    } CLK_ENABLE_CLR_REG1_b; /*!< BitSize                                                               */
  };
  /*0x08*/
  union {
    __IO uint32_t CLK_ENABLE_SET_REG2; /*!< none                                                                  */
    struct {
      __IO uint32_t GEN_SPI_MST1_HCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t CAN1_PCLK_ENABLE_b : 1;
      __IO uint32_t CAN1_CLK_ENABLE_b : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t UDMA_HCLK_ENABLE_b : 1;
      __IO uint32_t I2C_BUS_CLK_ENABLE_b : 1;
      __IO uint32_t I2C_2_BUS_CLK_ENABLE_b : 1;
      __IO uint32_t SSI_SLV_PCLK_ENABLE_b : 1;
      __IO uint32_t SSI_SLV_SCLK_ENABLE_b : 1;
      __IO uint32_t QSPI_CLK_ENABLE_b : 1;
      __IO uint32_t QSPI_HCLK_ENABLE_b : 1;
      __IO uint32_t I2SM_SCLK_ENABLE_b : 1;
      __IO uint32_t I2SM_INTF_SCLK_ENABLE_b : 1;
      __IO uint32_t I2SM_PCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t QE_PCLK_ENABLE_b : 1;
      __IO uint32_t MCPWM_PCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED4 : 1;
      __IO uint32_t SGPIO_PCLK_ENABLE_b : 1;
      __IO uint32_t EGPIO_PCLK_ENABLE_b : 1;
      __IO uint32_t ARM_CLK_ENABLE_b : 1;
      __IO uint32_t SSI_MST_PCLK_ENABLE_b : 1;
      __IO uint32_t SSI_MST_SCLK_ENABLE_b : 1;
      __IO uint32_t MEM2_CLK_ENABLE_b : 1;
      __IO uint32_t MEM_CLK_ULP_ENABLE_b : 1;
      __IO uint32_t ROM_CLK_ENABLE_b : 1;
      __IO uint32_t PLL_INTF_CLK_ENABLE_b : 1;
      __IO uint32_t SEMAPHORE_CLK_ENABLE_b : 1;
      __IO uint32_t TOT_CLK_ENABLE_b : 1;
      __IO uint32_t RMII_SOFT_RESET_b : 1;

    } CLK_ENABLE_SET_REG2_b; /*!< BitSize                                                               */
  };
  /*0X0C*/
  union {
    __IO uint32_t CLK_ENABLE_CLEAR_REG2; /*!< none                                                                  */

    struct {
      __IO uint32_t GEN_SPI_MST1_HCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t CAN1_PCLK_ENABLE_b : 1;
      __IO uint32_t CAN1_CLK_ENABLE_b : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t UDMA_HCLK_ENABLE_b : 1;
      __IO uint32_t I2C_BUS_CLK_ENABLE_b : 1;
      __IO uint32_t I2C_2_BUS_CLK_ENABLE_b : 1;
      __IO uint32_t SSI_SLV_PCLK_ENABLE_b : 1;
      __IO uint32_t SSI_SLV_SCLK_ENABLE_b : 1;
      __IO uint32_t QSPI_CLK_ENABLE_b : 1;
      __IO uint32_t QSPI_HCLK_ENABLE_b : 1;
      __IO uint32_t I2SM_SCLK_ENABLE_b : 1;
      __IO uint32_t I2SM_INTF_SCLK_ENABLE_b : 1;
      __IO uint32_t I2SM_PCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t QE_PCLK_ENABLE_b : 1;
      __IO uint32_t MCPWM_PCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED4 : 1;
      __IO uint32_t SGPIO_PCLK_ENABLE_b : 1;
      __IO uint32_t EGPIO_PCLK_ENABLE_b : 1;
      __IO uint32_t ARM_CLK_ENABLE_b : 1;
      __IO uint32_t SSI_MST_PCLK_ENABLE_b : 1;
      __IO uint32_t SSI_MST_SCLK_ENABLE_b : 1;
      __IO uint32_t MEM2_CLK_ENABLE_b : 1;
      __IO uint32_t MEM_CLK_ULP_ENABLE_b : 1;
      __IO uint32_t ROM_CLK_ENABLE_b : 1;
      __IO uint32_t PLL_INTF_CLK_ENABLE_b : 1;
      __IO uint32_t SEMAPHORE_CLK_ENABLE_b : 1;
      __IO uint32_t TOT_CLK_ENABLE_b : 1;
      __IO uint32_t RMII_SOFT_RESET_b : 1;

    } CLK_ENABLE_CLR_REG2_b; /*!< BitSize                                                               */
  };
  /*0x10*/
  union {
    __IO uint32_t CLK_ENABLE_SET_REG3; /*!< none                                                                  */

    struct {
      __IO uint32_t BUS_CLK_ENABLE_b : 1;
      __IO uint32_t M4_CORE_CLK_ENABLE_b : 1;
      __IO uint32_t CM_BUS_CLK_ENABLE_b : 1;
      __IO uint32_t RESERVED : 1;
      __IO uint32_t MISC_CONFIG_PCLK_ENABLE_b : 1;
      __IO uint32_t EFUSE_CLK_ENABLE_b : 1;
      __IO uint32_t ICM_CLK_ENABLE_b : 1;
      __IO uint32_t MEM1_CLK_ENABLE_b : 1;
      __IO uint32_t MEM3_CLK_ENABLE_b : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t USB_PHY_CLK_IN_ENABLE_b : 1;
      __IO uint32_t QSPI_CLK_ONEHOT_ENABLE_b : 1;
      __IO uint32_t QSPI_M4_SOC_SYNC_b : 1;
      __IO uint32_t RESERVED4 : 1;
      __IO uint32_t EGPIO_CLK_ENABLE_b : 1;
      __IO uint32_t I2C_CLK_ENABLE_b : 1;
      __IO uint32_t I2C_2_CLK_ENABLE_b : 1;
      __IO uint32_t EFUSE_PCLK_ENABLE_b : 1;
      __IO uint32_t SGPIO_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_64K_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_128K_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_SDIO_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_USB_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t ROM_MISC_STATIC_ENABLE_b : 1;
      __IO uint32_t M4_SOC_CLK_FOR_OTHER_ENABLE_b : 1;
      __IO uint32_t ICACHE_ENABLE_b : 1;
      __IO uint32_t RES : 4;
    } CLK_ENABLE_SET_REG3_b; /*!< BitSize                                                               */
  };
  /*0x14*/
  union {
    __IO uint32_t CLK_ENABLE_CLEAR_REG3; /*!< none                                                                  */

    struct {
      __IO uint32_t BUS_CLK_ENABLE_b : 1;
      __IO uint32_t M4_CORE_CLK_ENABLE_b : 1;
      __IO uint32_t CM_BUS_CLK_ENABLE_b : 1;
      __IO uint32_t RESERVED : 1;
      __IO uint32_t MISC_CONFIG_PCLK_ENABLE_b : 1;
      __IO uint32_t EFUSE_CLK_ENABLE_b : 1;
      __IO uint32_t ICM_CLK_ENABLE_b : 1;
      __IO uint32_t MEM1_CLK_ENABLE_b : 1;
      __IO uint32_t MEM3_CLK_ENABLE_b : 1;
      __IO uint32_t I2SS_INTF_SCLK_ENABLE_b : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t I2SS_SCLK_ENABLE_b : 1;
      __IO uint32_t USB_PHY_CLK_IN_ENABLE_b : 1;
      __IO uint32_t QSPI_CLK_ONEHOT_ENABLE_b : 1;
      __IO uint32_t QSPI_M4_SOC_SYNC_b : 1;
      __I uint32_t RESERVED4 : 1;
      __IO uint32_t EGPIO_CLK_ENABLE_b : 1;
      __IO uint32_t I2C_CLK_ENABLE_b : 1;
      __IO uint32_t I2C_2_CLK_ENABLE_b : 1;
      __IO uint32_t EFUSE_PCLK_ENABLE_b : 1;
      __IO uint32_t SGPIO_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_64K_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_128K_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_SDIO_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t TASS_M4SS_USB_SWITCH_CLK_ENABLE_b : 1;
      __IO uint32_t ROM_MISC_STATIC_ENABLE_b : 1;
      __IO uint32_t M4_SOC_CLK_FOR_OTHER_ENABLE_b : 1;
      __IO uint32_t ICACHE_ENABLE_b : 1;
      __IO uint32_t RES : 4;
    } CLK_ENABLE_CLR_REG3_b; /*!< BitSize                                                               */
  };
  /*0x18*/
  union {
    __IO uint32_t CLK_CONFIG_REG1; /*!< Clk Config Register1                                                  */

    struct {
      __IO uint32_t QSPI_CLK_SEL : 3;             //\92D0
      __IO uint32_t QSPI_CLK_DIV_FAC : 6;         //\92D1
      __IO uint32_t QSPI_CLK_SWALLOW_SEL : 1;     //\92B 0
      __IO uint32_t RESERVED : 1;                 //\92B 0
      __IO uint32_t SSI_MST_SCLK_DIV_FAC : 4;     //\92D1
      __IO uint32_t SSI_MST_SCLK_SEL : 3;         //\92D1
      __IO uint32_t PLL_INTF_CLK_SEL : 1;         //\92B 0
      __IO uint32_t PLL_INTF_CLK_DIV_FAC : 4;     //\92D2
      __IO uint32_t PLL_INTF_CLK_SWALLOW_SEL : 1; //\92B 0
      __IO uint32_t GEN_SPI_MST1_SCLK_SEL : 3;    //\92B0
      __IO uint32_t RESERVED1 : 5;

    } CLK_CONFIG_REG1_b; /*!< BitSize                                                               */
  };
  /*0x1C*/
  union {
    __IO uint32_t CLK_CONFIG_REG2; /*!< Clk Config Register2                                                  */

    struct {
      __IO uint32_t USART1_SCLK_SEL : 3;      //\92D0
      __IO uint32_t USART1_SCLK_DIV_FAC : 4;  //\92D1
      __IO uint32_t USART2_SCLK_SEL : 3;      //\92D0
      __IO uint32_t USART2_SCLK_DIV_FAC : 4;  //\92D1
      __IO uint32_t RESERVED3 : 3;            //\92D0
      __IO uint32_t RESERVED2 : 4;            //\92D1
      __IO uint32_t RESERVED1 : 2;            //\92D0
      __IO uint32_t RESERVED : 1;             //\92B0
      __IO uint32_t CCI_CLK_DIV_FAC : 4;      //\92D1
      __IO uint32_t QSPI_ODD_DIV_SEL : 1;     //\92B0
      __IO uint32_t USART1_SCLK_FRAC_SEL : 1; //\92B0
      __IO uint32_t USART2_SCLK_FRAC_SEL : 1; //\92B0
      __IO uint32_t RESERVED0 : 1;            //\92B0

    } CLK_CONFIG_REG2_b; /*!< BitSize                                                               */
  };
  /*0x20*/
  union {
    __IO uint32_t CLK_CONFIG_REG3; /*!< Clk Config Register3                                                  */

    struct {
      __IO uint32_t CAN1_CLK_DIV_FAC : 8; //\92D1
      __IO uint32_t MCU_CLKOUT_SEL : 4;
      __IO uint32_t MCU_CLKOUT_DIV_FAC : 6;
      __IO uint32_t MCU_CLKOUT_ENABLE : 1;
      __IO uint32_t RESERVED : 13;

    } CLK_CONFIG_REG3_b; /*!< BitSize                                                               */
  };
  /*0x24*/
  union {
    __IO uint32_t CLK_CONFIG_REG4; /*!< Clk Config Register4                                                  */

    struct {
      __IO uint32_t SOC_PLL_CLK_BYP_SEL : 2;
      __IO uint32_t I2S_PLL_CLK_BYP_SEL : 2;   //\92D3
      __IO uint32_t MODEM_PLL_CLK_BYP_SEL : 2; //\92D1
      __IO uint32_t INTF_PLL_CLK_BYP_SEL : 2;
      __IO uint32_t SOC_INTF_PLL_BYPCLK_CLKCLNR_ON : 1;
      __IO uint32_t SOC_INTF_PLL_BYPCLK_CLKCLNR_OFF : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t I2S_PLL_BYPCLK_CLKCLNR_ON : 1;
      __IO uint32_t I2S_PLL_BYPCLK_CLKCLNR_OFF : 1;
      __IO uint32_t MODEM_PLL_BYPCLK_CLKCLNR_ON : 1;
      __IO uint32_t MODEM_PLL_BYPCLK_CLKCLNR_OFF : 1;
      __IO uint32_t BYPASS_SOC_PLL_CLK : 1;
      __IO uint32_t BYPASS_I2S_PLL_CLK : 1;
      __IO uint32_t BYPASS_MODEM_PLL_CLK1 : 1;
      __IO uint32_t BYPASS_MODEM_PLL_CLK2 : 1;
      __IO uint32_t BYPASS_INTF_PLL_CLK : 1;
      __IO uint32_t SLEEP_CLK_SEL : 2; //'H0
      __IO uint32_t USB_SYSCLK_CLKCLNR_ON : 1;
      __IO uint32_t USB_SYSCLK_CLKCLNR_OFF : 1;
      __IO uint32_t ULPSS_CLK_DIV_FAC : 6;
      __IO uint32_t CCI_CLK_SEL : 1; //\92B0

    } CLK_CONFIG_REG4_b; /*!< BitSize                                                               */
  };
  /*0x28*/
  union {
    __IO uint32_t CLK_CONFIG_REG5; /*!< Clk Config Register5                                                  */

    struct {
      __IO uint32_t M4_SOC_CLK_SEL : 4;      // \92D0
      __IO uint32_t M4_SOC_CLK_DIV_FAC : 6;  //\92D1
      __IO uint32_t I2S_CLK_SEL : 1;         //\92B 0
      __IO uint32_t I2S_CLK_DIV_FAC : 6;     //\92D1
      __IO uint32_t CT_CLK_SEL : 3;          //\92D0
      __IO uint32_t CT_CLK_DIV_FAC : 6;      // \92D1
      __IO uint32_t M4_SOC_HOST_CLK_SEL : 1; // \92B0
      __IO uint32_t RESERVED : 1;            //\92B 0
      __IO uint32_t ULPSS_ODD_DIV_SEL : 1;   //\92B0
      __IO uint32_t USB_CLK_SEL : 2;         // \92B0
      __IO uint32_t RESERVED1 : 1;
    } CLK_CONFIG_REG5_b; //28              /*!< BitSize                                                               */
  };

  __IO uint32_t RESERVED0; //0x2C
  __IO uint32_t RESERVED1; //0x30
  __IO uint32_t RESERVED2; //0x34
  __IO uint32_t RESERVED3; //0x38
  __IO uint32_t RESERVED4; //0x3C
  /*0x40*/
  union {
    __IO uint32_t SD_MEM_CLOCK_REG; /*!< SDMEM clock Register                                                  */

    struct {
      __IO uint32_t SD_MEM_INTF_CLK_DIV_FAC : 6;     //\92D8
      __IO uint32_t SD_MEM_INTF_CLK_SEL : 3;         //\92D0
      __IO uint32_t SD_MEM_INTF_CLK_SWALLOW_SEL : 1; //\92B ; // 0
      __IO uint32_t RESERVED : 22;
    } SD_MEM_CLOCK_REG_b; /*!< BitSize                                                               */
  };
  /*0x44*/
  union {
    __IO uint32_t
      DYN_CLK_GATE_DISABLE_REG; /*!< Dynamic Clk Ctrl Disable Register                                     */

    struct {
      __IO uint32_t SDIO_SYS_HCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t BUS_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t GPDMA_HCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t EGPIO_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t SGPIO_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t TOT_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t HWRNG_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t USART1_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t USART1_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t USART2_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t USART2_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t SSI_SLV_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t SSI_SLV_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t RESERVED : 1;
      __IO uint32_t I2SM_INTF_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t SEMAPHORE_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t ARM_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t SSI_MST_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t MEM1_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t MEM2_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t MEM_CLK_ULP_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t MEM3_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t RESERVED4 : 1;
      __IO uint32_t RESERVED5 : 1;
      __IO uint32_t SSI_MST_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t ICACHE_DYN_GATING_DISABLE_b : 1;
      __IO uint32_t CCI_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t MISC_CONFIG_PCLK_DYN_CTRL_DISABLE_b : 1;
    } DYN_CLK_GATE_DISABLE_REG_b; /*!< BitSize                                                               */
  };

  __I uint32_t RESERVED5; //0x48
  __I uint32_t RESERVED6; //0x4C
  /*0x50*/
  union {
    __IO uint32_t PLL_ENABLE_SET_REG; /*!< PLL Enable SET Register                                               */

    struct {
      __IO uint32_t SOCPLL_SPI_SW_RESET : 1; //\92B0
      __IO uint32_t RESERVED : 31;
    } PLL_ENABLE_SET_REG_b; /*!< BitSize                                                               */
  };
  /*0x54*/
  union {
    __IO uint32_t PLL_ENABLE_CLR_REG; /*!< PLL Enable Clear Register                                             */

    struct {
      __IO uint32_t SOCPLL_SPI_SW_RESET : 1; //\92B0
      __IO uint32_t RESERVED : 31;
    } PLL_ENABLE_CLR_REG_b; /*!< BitSize                                                               */
  };
  /*0x58*/
  union {
    __I uint32_t PLL_STAT_REG; /*!< PLL Status Register                                                   */

    struct {
      __I uint32_t LCDPLL_LOCK : 1;
      __I uint32_t DDRPLL_LOCK : 1;
      __I uint32_t APPLL_LOCK : 1;
      __I uint32_t INTFPLL_LOCK : 1;
      __I uint32_t I2SPLL_LOCK : 1;
      __I uint32_t SOCPLL_LOCK : 1;
      __I uint32_t MODEMPLL_LOCK : 1;
      __I uint32_t PLL_LOCK_DATA_TRIG : 1;
      __I uint32_t M4_SOC_CLK_SWITCHED : 1;
      __I uint32_t QSPI_CLK_SWITCHED : 1;
      __I uint32_t USART1_SCLK_SWITCHED : 1;
      __I uint32_t USART2_SCLK_SWITCHED : 1;
      __I uint32_t GEN_SPI_MST1_SCLK_SWITCHED : 1;
      __I uint32_t SSI_MST_SCLK_SWITCHED : 1;
      __I uint32_t SD_MEM_INTF_CLK_SWITCHED : 1;
      __I uint32_t CT_CLK_SWITCHED : 1;
      __I uint32_t M4_TA_SOC_CLK_SWITCHED_SDIO : 1;
      __I uint32_t I2S_CLK_SWITCHED : 1;
      __I uint32_t PLL_INTF_CLK_SWITCHED : 1;
      __I uint32_t RESERVED0 : 1;
      __I uint32_t RESERVED1 : 1;
      __I uint32_t SLEEP_CLK_SWITCHED : 1;
      __I uint32_t MCU_CLKOUT_SWITCHED : 1;
      __I uint32_t CCI_CLK_SWITCHED : 1;
      __I uint32_t M4_TA_SOC_CLK_SWITCHED_USB : 1;
      __I uint32_t CC_CLOCK_MUX_SWITCHED : 1;
      __I uint32_t TASS_M4SS_64K_CLK_SWITCHED : 1;
      __I uint32_t USART1_CLK_SWITCHED : 1;
      __I uint32_t USART2_CLK_SWITCHED : 1;
      __I uint32_t TASS_M4SS_128K_CLK_SWITCHED : 1;
      __I uint32_t CLK_FREE_OR_SLP_SWITCHED : 1;
      __I uint32_t ULP_REF_CLK_SWITCHED : 1;
    } PLL_STAT_REG_b; /*!< BitSize                                                               */
  };
  /*0x5C*/
  union {
    __IO uint32_t PLL_LOCK_INT_MASK_REG; /*!< PLL Lock Interrupt Mask Register                                      */

    struct {
      __IO uint32_t LCD_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t DDR_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t AP_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t INTF_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t I2S_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t SOC_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t MODEM_PLL_LOCK_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t PLL_LOCK_DATA_TRIGGER_MASK_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t LCD_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t DDR_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t AP_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t INTF_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t I2S_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t SOC_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t MODEM_PLL_LOCK_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t PLL_LOCK_DATA_TRIGGER_MASK_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t RESERVED : 16;

    } PLL_LOCK_INT_MASK_REG_b; /*!< BitSize                                                               */
  };
  /*0x60*/
  union {
    __IO uint32_t PLL_LOCK_INT_CLR_REG; /*!< PLL Lock Interrupt Clear Register                                     */

    struct {
      __IO uint32_t LCD_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t DDR_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t AP_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t INTF_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t I2S_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t SOC_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t MODEM_PLL_LOCK_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t PLL_LOCK_DATA_TRIGGER_CLEAR_BIT_OF_RISING_EDGE : 1;
      __IO uint32_t LCD_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t DDR_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t AP_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t INTF_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t I2S_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t SOC_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t MODEM_PLL_LOCK_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t PLL_LOCK_DATA_TRIGGER_CLEAR_BIT_OF_FALLING_EDGE : 1;
      __IO uint32_t RESERVED : 16;
    } PLL_LOCK_INT_CLR_REG_b; /*!< BitSize                                                               */
  };

  /*0x64*/
  union {
    __IO uint32_t PLL_LOCK_INT_DATA_REG; /*!< PLL Lock Interrupt Data Register                                      */

    struct {
      __IO uint32_t LCD_PLL_LOCK : 1;
      __IO uint32_t DDR_PLL_LOCK : 1;
      __IO uint32_t AP_PLL_LOCK : 1;
      __IO uint32_t INTF_PLL_LOCK : 1;
      __IO uint32_t I2S_PLL_LOCK : 1;
      __IO uint32_t SOC_PLL_LOCK : 1;
      __IO uint32_t MODEM_PLL_LOCK : 1;
      __IO uint32_t RESERVED : 25;
    } PLL_LOCK_INT_DATA_REG_b; /*!< BitSize                                                               */
  };
  /*0x68*/
  union {
    __IO uint32_t SLEEP_CALIB_REG; /*!< Sleep Calibration Register                                            */

    struct {
      __IO uint32_t SLP_CALIB_START_b : 1;     // \92B 0
      __IO uint32_t SLP_CALIB_CYCLES : 2;      //\92D0
      __IO uint32_t SLP_CALIB_DURATION_b : 16; //NA
      __I uint32_t SLP_CALIB_DONE_b : 1;       // NA
      __IO uint32_t RES : 12;                  // NA
    } SLEEP_CALIB_REG_b; /*!< BitSize                                                               */
  };
  /*0x6C*/
  union {
    __IO uint32_t CLK_CALIB_CTRL_REG1; /*!< Clock Calibration Control Register1                                   */

    struct {
      __IO uint32_t CC_SOFT_RST_b : 1;        //\92B 0
      __IO uint32_t CC_START_b : 1;           //\92B 0
      __IO uint32_t CC_CHANGE_TEST_CLK_b : 1; //\92B 0
      __IO uint32_t CC_CLKIN_SEL_b : 4;       //\92D0
      __IO uint32_t RESERVED : 25;
    } CLK_CALIB_CTRL_REG1_b; /*!< BitSize                                                               */
  };
  /*0x70*/
  union {
    __IO uint32_t CLK_CALIB_CTRL_REG2; /*!< Clock Calibration Control Register2                                   */

    struct {
      __IO uint32_t CC_NUM_REF_CLKS : 32;
    } CLK_CALIB_CTRL_REG2_b; /*!< BitSize                                                               */
  };
  /*0x74*/
  union {
    __I uint32_t CLK_CALIB_STS_REG1; /*!< Clock Calibration Status Register1                                    */

    struct {
      __I uint32_t CC_DONE_b : 1;  // \92B 0
      __I uint32_t CC_ERROR_b : 1; //\92B 0
      __IO uint32_t RESERVED : 30;
    } CLK_CALIB_STS_REG1_b; /*!< BitSize                                                               */
  };
  /*0x78*/
  union {
    __I uint32_t CLK_CALIB_STS_REG2; /*!< Clock Calibration Status Register1                                    */

    struct {
      __I uint32_t CC_NUM_TEST_CLKS : 32;
    } CLK_CALIB_STS_REG2_b; /*!< BitSize                                                               */
  };
  /*0x7C*/
  union {
    __IO uint32_t CLK_CONFIG_REG6; /*!< Clock Calibration Status Register1                                    */

    struct {
      __IO uint32_t IID_KH_CLK_DIV_FAC : 3;  //\92D1
      __IO uint32_t USB_PHY_CLK_DIV_FAC : 2; //\92D1
      __IO uint32_t PADCFG_PCLK_DIV_FAC : 4;
      __IO uint32_t RESERVED : 23;
    } CLK_CONFIG_REG6_b; /*!< BitSize                                                               */
  };
  /*0x80*/
  union {
    __IO uint32_t
      DYN_CLK_GATE_DISABLE_REG2; /*!< Clock Calibration Status Register1                                    */

    struct {
      __IO uint32_t SOC_PLL_SPI_CLK_DYN_CTRL_DISABLE_b : 1; //\92B 0
      __IO uint32_t I2C_BUS_DYN_CTRL_DISABLE_b : 1;         //\92B 0
      __IO uint32_t I2C_2_BUS_CLK_DYN_CTRL_DISABLE_b : 1;   //\92B 0
      __IO uint32_t CT_PCLK_DYN_CTRL_DISABLE_b : 1;         //\92B 0
      __IO uint32_t CAN1_PCLK_DYN_CTRL_DISABLE_b : 1;       //\92B 0
      __IO uint32_t I2SM_PCLK_DYN_CTRL_DISABLE_b : 1;       //\92B 0
      __IO uint32_t EFUSE_CLK_DYN_CTRL_DISABLE_b : 1;       //\92B 0
      __IO uint32_t EFUSE_PCLK_DYN_CTRL_DISABLE_b : 1;      //\92B 0
      __IO uint32_t PWR_CTRL_CLK_DYN_CTRL_DISABLE_b : 1;    //\92B 1
      __IO uint32_t RESERVED : 23;
    } DYN_CLK_GATE_DISABLE_REG2_b; /*!< BitSize                                                               */
  };
  /*0x84*/
  union {
    __IO uint32_t PLL_LOCK_INT_STATUS_REG; /*!< PLL Lock Interrupt Data Register                                      */

    struct {
      __IO uint32_t LCD_PLL_LOCK_OF_RISING_EDGE : 1;
      __IO uint32_t DDR_PLL_LOCK_OF_RISING_EDGE : 1;
      __IO uint32_t AP_PLL_LOCK_OF_RISING_EDGE : 1;
      __IO uint32_t INTF_PLL_LOCK_OF_RISING_EDGE : 1;
      __IO uint32_t I2S_PLL_LOCK_OF_RISING_EDGE : 1;
      __IO uint32_t SOC_PLL_LOCK_OF_RISING_EDGE : 1;
      __IO uint32_t MODEM_PLL_LOCK_OF_RISING_EDGE : 1;
      __IO uint32_t PLL_LOCK_DATA_TRIGGER_INTR_OF_RISING_EDGE : 1;
      __IO uint32_t LCD_PLL_LOCK_OF_FALLING_EDGE : 1;
      __IO uint32_t DDR_PLL_LOCK_OF_FALLING_EDGE : 1;
      __IO uint32_t AP_PLL_LOCK_OF_FALLING_EDGE : 1;
      __IO uint32_t INTF_PLL_LOCK_OF_FALLING_EDGE : 1;
      __IO uint32_t I2S_PLL_LOCK_OF_FALLING_EDGE : 1;
      __IO uint32_t SOC_PLL_LOCK_OF_FALLING_EDGE : 1;
      __IO uint32_t MODEM_PLL_LOCK_OF_FALLING_EDGE : 1;
      __IO uint32_t PLL_LOCK_DATA_TRIGGER_INTR_OF_FALLING_EDGE : 1;
      __IO uint32_t RESERVED : 16;
    } PLL_LOCK_INT_STATUS_REG_b; /*!< BitSize                                                               */
  };

} M4CLK_Type;

/*Time period*/

typedef struct {

  /*0x00*/
  union {
    __IO uint32_t
      MCU_CAL_RO_TIMEPERIOD_READ; /*!< none                                                                  */

    struct {
      __IO uint32_t TIMEPERIOD_RO : 25; /*!<Calibrated RO timeperiod    */
      __IO uint32_t RESERVED : 7;       /*!<    */
    } MCU_CAL_RO_TIMEPERIOD_READ_b;     /*!< BitSize                                                               */
  };
  /*0x04*/
  union {
    __IO uint32_t
      MCU_CAL_TIMER_CLOCK_PERIOD; /*!< none                                                                  */

    struct {
      __IO uint32_t RTC_TIMER_CLK_PERIOD : 25;              /*!<Calibrated RO timeperiod    */
      __IO uint32_t RESERVED : 6;                           /*!<    */
      __IO uint32_t SPI_RTC_TIMER_CLK_PERIOD_APPLIED_b : 1; /*!<    */
    } MCU_CAL_TIMER_CLOCK_PERIOD_b; /*!< BitSize                                                               */
  };

  /*0x08*/
  union {
    __IO uint32_t MCU_CAL_TEMP_PROG_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t BYPASS_CALIB_PG : 1; /*!<Calibrated RO timeperiod    */
      __IO uint32_t RESERVED1 : 15;      /*!<    */
      __IO uint32_t MAX_TEMP_CHANGE : 5; /*!<    */
      __IO uint32_t TEMP_TRIGGER_TIME_SEL : 2;
      __IO uint32_t PERIODIC_TEMP_CALIB_EN : 1;
      __IO uint32_t RTC_TIMER_PERIOD_MUX_SEL : 1;
      __IO uint32_t RESERVED2 : 7;
    } MCU_CAL_TEMP_PROG_REG_b; /*!< BitSize                                                               */
  };

  /*0x0C*/
  union {
    __IO uint32_t MCU_CAL_START_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t ALPHA_RO : 3;      /*!<Calibrated RO timeperiod    */
      __IO uint32_t ALPHA_RC : 3;      /*!<    */
      __IO uint32_t NO_OF_RO_CLKS : 4; /*!<    */
      __IO uint32_t NO_OF_RC_CLKS : 3;
      __IO uint32_t RC_SETTLE_TIME : 3;
      __IO uint32_t RO_TRIGGER_TIME_SEL : 2;
      __IO uint32_t RC_TRIGGER_TIME_SEL : 3;
      __IO uint32_t PERIODIC_RO_CALIB_EN : 1;
      __IO uint32_t PERIODIC_RC_CALIB_EN : 1;
      __IO uint32_t START_CALIB_RO : 1;
      __IO uint32_t START_CALIB_RC : 1;
      __IO uint32_t RC_XTAL_MUX_SEL : 1;
      __IO uint32_t LOW_POWER_TRIGGER_SEL : 1;
      __IO uint32_t VBATT_TRIGGER_TIME_SEL : 3;
      __IO uint32_t RESERVED2 : 2;
    } MCU_CAL_START_REG_b; /*!< BitSize                                                               */
  };
  /*0x10*/
  union {
    __IO uint32_t
      MCU_CAL_REF_CLK_SETTLE_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t XTAL_SETTLE : 7; /*!<Calibrated RO timeperiod    */
      __IO uint32_t RESERVED1 : 9;
      __IO uint32_t VALID_RC_TIMEPERIOD : 1;
      __IO uint32_t VALID_RO_TIMEPERIOD : 1;
      __IO uint32_t RESERVED2 : 14;
    } MCU_CAL_REF_CLK_SETTLE_REG_b; /*!< BitSize                                                               */
  };

  /*0x14*/
  union {
    __IO uint32_t
      MCU_CAL_RC_TIMEPERIOD_READ; /*!< none                                                                  */

    struct {
      __IO uint32_t TIMEPERIOD_RC : 25; /*!<Calibrated RO timeperiod    */
      __IO uint32_t RESERVED : 7;
    } MCU_CAL_RC_TIMEPERIOD_READ_b; /*!< BitSize                                                               */
  };

  /*0x18*/
  union {
    __IO uint32_t
      MCU_CAL_REF_CLK_TIEMPERIOD_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t TIMEPERIOD_REF_CLK : 24; /*!<Calibrated RO timeperiod    */
      __IO uint32_t RESERVED : 8;
    } MCU_CAL_REF_CLK_TIEMPERIOD_REG_b; /*!< BitSize                                                               */
  };

} TIME_PERIOD_Type;

/*End of time period */

/*MCU WDT */

typedef struct { /*!< (@ 0x24048300) MCU_WDT Structure                                          */

  union {
    __IOM uint32_t
      MCU_WWD_INTERRUPT_TIMER; /*!< (@ 0x00000000) WATCHDOG interrupt timer register                          */

    struct {
      __IOM uint32_t
        WWD_INTERRUPT_TIMER : 5;    /*!< (@ 0x00000000) Watchdog Timer programming values                          */
      __IM uint32_t RESERVED1 : 27; /*!< (@ 0x00000005) reser                                                      */
    } MCU_WWD_INTERRUPT_TIMER_b;
  };

  union {
    __IOM uint32_t
      MCU_WWD_SYSTEM_RESET_TIMER; /*!< (@ 0x00000004) MCU watchdog system reset register                         */

    struct {
      __IOM uint32_t
        WWD_SYSTEM_RESET_TIMER : 5; /*!< (@ 0x00000000) Watch dog soc reset delay timer programming values         */
      __IM uint32_t RESERVED1 : 27; /*!< (@ 0x00000005) reser                                                      */
    } MCU_WWD_SYSTEM_RESET_TIMER_b;
  };

  union {
    __IOM uint32_t
      MCU_WWD_WINDOW_TIMER; /*!< (@ 0x00000008) watchdog window timer register                             */

    struct {
      __IOM uint32_t WINDOW_TIMER : 4; /*!< (@ 0x00000000) watchdog window timer                                      */
      __IM uint32_t RESERVED1 : 28;    /*!< (@ 0x00000004) reser                                                      */
    } MCU_WWD_WINDOW_TIMER_b;
  };

  union {
    __IOM uint32_t
      MCU_WWD_ARM_STUCK_EN; /*!< (@ 0x0000000C) watchdog arm stuck enable register                         */

    struct {
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000000) reser                                                      */
      __OM uint32_t
        PROCESSOR_STUCK_RESET_EN : 1; /*!< (@ 0x00000010) Enable to reset M4 core on seeing LOCKUP signal          */
      __IOM uint32_t RESERVED2 : 7;   /*!< (@ 0x00000011) reser                                                      */
      __IM uint32_t
        PROCESSOR_STUCK_RESET_EN_ : 1; /*!< (@ 0x00000018) Read signal for processor stuck reset enable            */
      __IOM uint32_t RESERVED3 : 7;    /*!< (@ 0x00000019) reser                                                      */
    } MCU_WWD_ARM_STUCK_EN_b;
  };

  union {
    __IOM uint32_t
      MCU_WWD_MODE_AND_RSTART; /*!< (@ 0x00000010) WATCHDOG mode and restart register                         */

    struct {
      __IOM uint32_t
        WWD_MODE_RSTART : 1;        /*!< (@ 0x00000000) restart pulse to restart watchdog timer                    */
      __IM uint32_t RESERVED1 : 15; /*!< (@ 0x00000001) reser                                                      */
      __IOM uint32_t
        WWD_MODE_EN_STATUS : 8;    /*!< (@ 0x00000010) Watchdog timer mode                                        */
      __IM uint32_t RESERVED2 : 8; /*!< (@ 0x00000018) reser                                                      */
    } MCU_WWD_MODE_AND_RSTART_b;
  };
  __IM uint32_t RESERVED;

  union {
    __IOM uint32_t MCU_WWD_KEY_ENABLE; /*!< (@ 0x00000018) watchdog key enable register                               */

    struct {
      __OM uint32_t
        WWD_KEY_ENABLE : 32; /*!< (@ 0x00000000) enable access to program Watch dog registers               */
    } MCU_WWD_KEY_ENABLE_b;
  };
} MCU_WDT_Type;

/*end of MCU WDT */

/*MCU Calender */
typedef struct {

  /*0x00*/
  /*0x04*/
  /*0x08*/
  /*0x0C*/
  /*0x10*/
  /*0x14*/
  /*0x18*/
  __IM uint32_t RESERVED[7];
  /*0x1C*/
  union {
    __IOM uint32_t MCU_CAL_ALARM_PROG_1; /*!< (@ 0x0000001C) MCU calender alarm prog register 1*/

    struct {
      __IOM uint32_t PROG_ALARM_MSEC : 10; /*!< (@ 0x00000000) milli seconds value of alarm time*/
      __IOM uint32_t PROG_ALARM_SEC : 6;   /*!< (@ 0x0000000A) seconds value of alarm time      */
      __IOM uint32_t PROG_ALARM_MIN : 6;   /*!< (@ 0x00000010) mins value of alarm time         */
      __IOM uint32_t PROG_ALARM_HOUR : 5;  /*!< (@ 0x00000016) hours value of alarm time        */
      __IM uint32_t RESERVED1 : 5;         /*!< (@ 0x0000001B) reser                            */
    } MCU_CAL_ALARM_PROG_1_b;              /*!< BitSize                                                               */
  };
  /*0x20*/
  union {
    __IOM uint32_t MCU_CAL_ALARM_PROG_2; /*!< (@ 0x00000020) MCU calender alarm prog register 2 */

    struct {
      __IOM uint32_t PROG_ALARM_DAY : 5;     /*!< (@ 0x00000000) day count in alarm time 1-31      */
      __IM uint32_t RESERVED1 : 3;           /*!< (@ 0x00000005) reser                             */
      __IOM uint32_t PROG_ALARM_MONTH : 4;   /*!< (@ 0x00000008) month count in alarm time         */
      __IM uint32_t RESERVED2 : 4;           /*!< (@ 0x0000000C) reser                             */
      __IOM uint32_t PROG_ALARM_YEAR : 7;    /*!< (@ 0x00000010) year count in alarm time 0 - 99   */
      __IOM uint32_t PROG_ALARM_CENTURY : 2; /*!< (@ 0x00000017) century count in alarm time       */
      __IM uint32_t RESERVED3 : 6;           /*!< (@ 0x00000019) reser                             */
      __IOM uint32_t ALARM_EN : 1;           /*!< (@ 0x0000001F) alarm function enable for calendar*/
    } MCU_CAL_ALARM_PROG_2_b; /*!< BitSize                                                               */
  };
  /*0x24*/
  union {
    __IOM uint32_t MCU_CAL_POWERGATE_REG; /*!< (@ 0x00000024) MCU calender powergate register */
    struct {
      __IOM uint32_t PG_EN_CALENDER : 1;        /*!< (@ 0x00000000) Start calender block      */
      __IOM uint32_t ENABLE_CALENDER_COMBI : 1; /*!< (@ 0x00000001) Enable calender combitional logic block */
      __IM uint32_t RES : 30;                   /*!< (@ 0x00000002) reser */
    } MCU_CAL_POWERGATE_REG_b;
  };
  /*0x28*/
  union {
    __IOM uint32_t MCU_CAL_PROG_TIME_1; /*!< (@ 0x00000028) MCU calendar prog time 1 register*/
    struct {
      __IOM uint32_t
        PROG_MSEC : 10; /*!< (@ 0x00000000) Milli seconds value to be programmed to real time in calendar when pro_time_trig is  1   */
      __IOM uint32_t
        PROG_SEC : 6; /*!< (@ 0x0000000A) seconds value to be programmed to real time in calendar when pro_time_trig is 1 */
      __IOM uint32_t
        PROG_MIN : 6; /*!< (@ 0x00000010) minutes value to be programmed to real time in calendar when pro_time_trig is 1 */
      __IOM uint32_t
        PROG_HOUR : 5; /*!< (@ 0x00000016) hours value to be programmed to real time in calendar when pro_time_trig is 1 */
      __IM uint32_t RESERVED2 : 5; /*!< (@ 0x0000001B) reser                            */
    } MCU_CAL_PROG_TIME_1_b;       /*!< BitSize                                                               */
  };
  /*0x2C*/
  union {
    __IOM uint32_t MCU_CAL_PROG_TIME_2; /*!< (@ 0x0000002C) MCU calendar prog time 2 register*/
    struct {
      __IOM uint32_t
        PROG_DAY : 5; /*!< (@ 0x00000000) day count value to be programmed to real time in calendar when pro_time_trig is 1 */
      __IOM uint32_t PROG_WEEK_DAY : 3; /*!< (@ 0x00000005) program which week day it is     */
      __IOM uint32_t
        PROG_MONTH : 4; /*!< (@ 0x00000008) month value to be programmed to real time in calendar when pro_time_trig is 1 */
      __IM uint32_t RES : 4; /*!< (@ 0x0000000C) reser     */
      __IOM uint32_t
        PROG_YEAR : 7; /*!< (@ 0x00000010) year value to be programmed to real time in calendar when pro_time_trig is 1   */
      __IOM uint32_t
        PROG_CENTURY : 2; /*!< (@ 0x00000017) century value to be programmed to real time in calendar when pro_time_trig is 1 */
      __IM uint32_t RESERVED1 : 6;      /*!< (@ 0x00000019) reser                            */
      __OM uint32_t PROG_TIME_TRIG : 1; /*!< (@ 0x0000001F) load the programmed to the real time in calendar block   */
    } MCU_CAL_PROG_TIME_2_b;            /*!< BitSize                                                               */
  };
  /*0x30*/
  union {
    __IOM uint32_t
      MCU_CAL_READ_TIME_MSB; /*!< (@ 0x00000030) MCU calendar read time msb                                 */
    struct {
      __IOM uint32_t WEEK_DAY : 3;     /*!< (@ 0x00000000) week day                                                   */
      __IOM uint32_t MONTHS_COUNT : 4; /*!< (@ 0x00000003) months count                                               */
      __IOM uint32_t YEAR_COUNT : 7;   /*!< (@ 0x00000007) years count                                                */
      __IOM uint32_t
        CENTURY_COUNT : 2;          /*!< (@ 0x0000000E) century count                                              */
      __IM uint32_t RESERVED1 : 16; /*!< (@ 0x00000010) reser                                                      */
    } MCU_CAL_READ_TIME_MSB_b;      /*!< BitSize                                                               */
  };
  /*0x34*/
  union {
    __IOM uint32_t
      MCU_CAL_READ_TIME_LSB; /*!< (@ 0x00000034) MCU calendar read time lsb                                 */
    struct {
      __IOM uint32_t
        MILLISECONDS_COUNT : 10; /*!< (@ 0x00000000) milliseconds count                                         */
      __IOM uint32_t
        SECONDS_COUNT : 6;            /*!< (@ 0x0000000A) seconds count                                              */
      __IOM uint32_t MINS_COUNT : 6;  /*!< (@ 0x00000010) mins count                                                 */
      __IOM uint32_t HOURS_COUNT : 5; /*!< (@ 0x00000016) hours count                                                */
      __IOM uint32_t DAYS_COUNT : 5;  /*!< (@ 0x0000001B) days count                                                 */
    } MCU_CAL_READ_TIME_LSB_b;        /*!< BitSize                                                               */
  };
  /*0x38*/
  union {
    __IOM uint32_t MCU_CAL_READ_COUNT_TIMER; /*!< (@ 0x00000038) MCU calendar read count timer    */

    struct {
      __IO uint32_t READ_COUNT_TIMER : 27; /*!<    */
      __IO uint32_t RESERVED1 : 5;
    } MCU_CAL_READ_COUNT_TIMER_b; /*!< BitSize                                                               */
  };
  /*0x3C*/
  union {
    __IOM uint32_t MCU_CAL_SLEEP_CLK_COUNTERS; /*!< (@ 0x0000003C) MCU calendar sleep clock counter */

    struct {
      __IM uint32_t
        SLEEP_CLK_DURATION : 12; /*!< (@ 0x00000000) No of sleep clks with respect to APB clock so far from the posedge of sleep clk*/
      __IM uint32_t RESERVED1 : 4;                 /*!< (@ 0x0000000C) reser   */
      __IM uint32_t PCLK_COUNT_WRT_SLEEP_CLK : 12; /*!< (@ 0x00000010) no. of APB clks in 1 sleep clock duration    */
      __IM uint32_t RESERVED2 : 4;                 /*!< (@ 0x0000001C) reser  */
    } MCU_CAL_SLEEP_CLK_COUNTERS_b; /*!< BitSize                                                               */
  };

  /*0x40*/
  union {
    __OM uint32_t MCU_CAL_KEY_EANBLE; /*!< (@ 0x00000040) MCU calendar key enable          */

    struct {
      __OM uint32_t RTC_KEY : 32; /*!< (@ 0x00000000) enable access to program Watch dog registers               */
    } MCU_CAL_KEY_EANBLE_b;       /*!< BitSize                                                               */
  };
} RTC_Type; /*!< Size = 68 (0x44)        */

/*End of MCU Calender*/

/*start MCU battery Flip Flops */

typedef struct {

  /*0x00*/
  union {
    __IO uint32_t
      M4SS_BYPASS_PWRCTRL_REG1; /*!< none                                                                  */

    struct {
      __IO uint32_t RES : 2;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_SD_MEM_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_M4_ULP_AON_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_EFUSE_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_CCI_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_PERI3_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_PERI2_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_PERI1_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_RPDMA_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_HIF_USB_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_HIF_SDIO_SPI_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_HIF_ETHERNET_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_QSPI_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_IID_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_ICACHE_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_M4_FPU_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_M4_DEBUG_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_M4_CORE_b : 1;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_AON_b : 1;
      __IO uint32_t RESERVED1 : 2;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_ROM_b : 1;
      __IO uint32_t RESERVED2 : 9;

    } M4SS_BYPASS_PWRCTRL_REG1_b; /*!< BitSize                                                               */
  };

  /*0x04*/
  union {
    __IO uint32_t
      M4SS_BYPASS_PWRCTRL_REG2; /*!< none                                                                  */

    struct {
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_SRAM_1 : 10; /*!<   */
      __IO uint32_t RESERVED1 : 6;
      __IO uint32_t BYPASS_M4SS_PWRCTL_ULP_SRAM_2 : 4;
      __IO uint32_t RESERVED2 : 12;
    } M4SS_BYPASS_PWRCTRL_REG2_b; /*!< BitSize                                                               */
  };

  /*0x08*/
  union {
    __IO uint32_t M4SS_PWRCTRL_SET_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t RES : 2;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_SD_MEM_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_EFUSE_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_CCI_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_PERI3_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_PERI2_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_PERI1_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_RPDMA_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_HIF_USB_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_HIF_SDIO_SPI_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_HIF_ETHERNET_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_QSPI_b : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_ICACHE_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_M4_FPU_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_M4_DEBUG_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_M4_CORE_b : 1;
      __IO uint32_t RESERVED1 : 3;
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_ROM_b : 1;
      __IO uint32_t RESERVED2 : 9;
    } M4SS_PWRCTRL_SET_REG_b; /*!< BitSize                                                               */
  };

  /*0x0C*/
  union {
    __IO uint32_t M4SS_PWRCTRL_CLEAR_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t RES : 2;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_SD_MEM_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_EFUSE_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_CCI_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_PERI3_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_PERI2_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_PERI1_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_RPDMA_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_HIF_USB_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_HIF_SDIO_SPI_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_HIF_ETHERNET_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_QSPI_b : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_ICACHE_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_M4_FPU_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_M4_DEBUG_b : 1;
      __IO uint32_t M4SS_PWRGATE_EN_N_ULP_M4_CORE_b : 1;
      __IO uint32_t RESERVED1 : 3;
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_ROM_b : 1;
      __IO uint32_t RESERVED2 : 9;
    } M4SS_PWRCTRL_CLEAR_REG_b; /*!< BitSize                                                               */
  };

  /*0x10*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_SET_REG1; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_1 : 10;
      __IO uint32_t RESERVED1 : 6;
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_2 : 4;
      __IO uint32_t RESERVED2 : 12;
    } M4_SRAM_PWRCTRL_SET_REG1_b; /*!< BitSize                                                               */
  };

  /*0x14*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_CLEAR_REG1; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_1 : 10;
      __IO uint32_t RESERVED1 : 6;
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_2 : 4;
      __IO uint32_t RESERVED2 : 12;
    } M4_SRAM_PWRCTRL_CLEAR_REG1_b; /*!< BitSize                                                               */
  };

  /*0x18*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_SET_REG2; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 10;
      __IO uint32_t RESERVED1 : 6;
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_2 : 4;
      __IO uint32_t RESERVED2 : 12;
    } M4_SRAM_PWRCTRL_SET_REG2_b; /*!< BitSize                                                               */
  };

  /*0x1C*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_CLEAR_REG2; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 10;
      __IO uint32_t RESERVED1 : 6;
      __IO uint32_t M4SS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_2 : 4;
      __IO uint32_t RESERVED2 : 12;
    } M4_SRAM_PWRCTRL_CLEAR_REG2_b; /*!< BitSize                                                               */
  };

  /*0x20*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_SET_REG3; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_1 : 10;
      __IO uint32_t RESERVED1 : 6;
      __IO uint32_t M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_2 : 4;
      __IO uint32_t RESERVED2 : 12;
    } M4_SRAM_PWRCTRL_SET_REG3_b; /*!< BitSize                                                               */
  };

  /*0x24*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_CLEAR_REG3; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_1 : 10;
      __IO uint32_t RESERVED1 : 6;
      __IO uint32_t M4SS_SRAM_INPUT_DISABLE_ISOLATION_ULP_2 : 4;
      __IO uint32_t RESERVED2 : 12;
    } M4_SRAM_PWRCTRL_CLEAR_REG3_b; /*!< BitSize                                                               */
  };

  /*0x28*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_SET_REG4; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_SRAM_DS_1 : 15;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t M4SS_SRAM_DS_2 : 12;
      __IO uint32_t RESERVED2 : 4;
    } M4_SRAM_PWRCTRL_SET_REG4_b; /*!< BitSize                                                               */
  };

  /*0x2C*/
  union {
    __IO uint32_t
      M4_SRAM_PWRCTRL_CLEAR_REG4; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_SRAM_DS_1 : 15;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t M4SS_SRAM_DS_2 : 12;
      __IO uint32_t RESERVED2 : 4;
    } M4_SRAM_PWRCTRL_CLEAR_REG4_b; /*!< BitSize                                                               */
  };
  /*0x30*/
  __IO uint32_t RESERVED2;

  /*0x34*/
  union {
    __IO uint32_t M4SS_TASS_CTRL_SET_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_CTRL_TASS_AON_PWRGATE_EN : 1;
      __IO uint32_t M4SS_CTRL_TASS_AON_DISABLE_ISOLATION_BYPASS : 1;
      __IO uint32_t M4SS_CTRL_TASS_AON_PWR_DMN_RST_BYPASS : 1;
      __IO uint32_t RESERVED1 : 13;
      __IO uint32_t M4SS_CTRL_TASS_EXT_PWRGATE_EN_N_ULP_ROM : 8;
      __IO uint32_t RESERVED2 : 8;
    } M4SS_TASS_CTRL_SET_REG_b; /*!< BitSize                                                               */
  };

  /*0x38*/
  union {
    __IO uint32_t
      M4SS_TASS_CTRL_CLEAR_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t M4SS_CTRL_TASS_AON_PWRGATE_EN : 1;
      __IO uint32_t M4SS_CTRL_TASS_AON_DISABLE_ISOLATION_BYPASS : 1;
      __IO uint32_t M4SS_CTRL_TASS_AON_PWR_DMN_RST_BYPASS : 1;
      __IO uint32_t RESERVED1 : 13;
      __IO uint32_t M4SS_CTRL_TASS_EXT_PWRGATE_EN_N_ULP_ROM : 8;
      __IO uint32_t RESERVED2 : 8;
    } M4SS_TASS_CTRL_CLEAR_REG_b; /*!< BitSize                                                               */
  };

  /*0x3C*/
  union {
    __IO uint32_t M4_ULP_MODE_CONFIG; /*!< none                                                                  */

    struct {
      __IO uint32_t ULPMODE_ISOLATION_CTRL_ULPSS : 1;
      __IO uint32_t ULPMODE_ISOLATION_CTRL_M4SS_AON : 1;
      __IO uint32_t ULPMODE_ISOLATION_CTRL_M4_ULP : 1;
      __IO uint32_t ULPMODE_ISOLATION_CTRL_M4_CORE : 1;
      __IO uint32_t ULPMODE_ISOLATION_CTRL_M4_DEBUG : 1;
      __IO uint32_t ULPMODE_ISOLATION_CTRL_M4_ROM : 1;
      __IO uint32_t RES : 26;
    } M4_ULP_MODE_CONFIG_b; /*!< BitSize                                                               */
  };

  /*0x40*/
  union {
    __IO uint32_t
      ULPSS_BYPASS_PWRCTRL_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t RES : 2;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_AON : 1;
      __IO uint32_t BYPASS_ULPSDCSS_PWRCTRL_ULP_AON : 1;
      __IO uint32_t RESERVED : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_MISC : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_CAP : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_VAD : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_UART : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_SSI : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_I2S : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_I2C : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_AUX : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_IR : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_UDMA : 1;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_FIM : 1;
      __IO uint32_t RESERVED2 : 3;
      __IO uint32_t BYPASS_ULPTASS_PWRCTL_ULP_SRAM : 8;
      __IO uint32_t RESERVED3 : 5;

    } ULPSS_BYPASS_PWRCTRL_REG_b; /*!< BitSize                                                               */
  };

  /*0x44*/
  union {
    __IO uint32_t ULPSS_PWRCTRL_SET_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t RES : 18;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_MISC : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_CAP : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_VAD : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_UART : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_SSI : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_I2S : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_I2C : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_AUX : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_IR : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_UDMA : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_FIM : 1;
      __IO uint32_t RESERVED : 3;

    } ULPSS_PWRCTRL_SET_REG_b; /*!< BitSize                                                               */
  };

  /*0x48*/
  union {
    __IO uint32_t ULPSS_PWRCTRL_CLEAR_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t RES : 18;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_MISC : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_CAP : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_VAD : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_UART : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_SSI : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_I2S : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_I2C : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_AUX : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_IR : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_UDMA : 1;
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_FIM : 1;
      __IO uint32_t RESERVED : 3;
    } ULPSS_PWRCTRL_CLEAR_REG_b; /*!< BitSize                                                               */
  };

  /*0x4C*/
  union {
    __IO uint32_t
      ULPSS_RAM_PWRCTRL_SET_REG1; /*!< none                                                                  */

    struct {
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM : 8;
      __IO uint32_t RESERVED1 : 8;
      __IO uint32_t RESERVED2 : 16;

    } ULPSS_RAM_PWRCTRL_SET_REG1_b; /*!< BitSize                                                               */
  };

  /*0x50*/
  union {
    __IO uint32_t
      ULPSS_RAM_PWRCTRL_CLEAR_REG1; /*!< none                                                                  */

    struct {
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM : 8;
      __IO uint32_t RESERVED1 : 8;
      __IO uint32_t RESERVED2 : 16;
    } ULPSS_RAM_PWRCTRL_CLEAR_REG1_b; /*!< BitSize                                                               */
  };

  /*0x54*/
  union {
    __IO uint32_t
      ULPSS_RAM_PWRCTRL_SET_REG2; /*!< none                                                                  */

    struct {
      __IO uint32_t ULPTASS_SRAM_INPUT_DISABLE_ISOLATION_ULP : 8;
      __IO uint32_t RESERVED1 : 8;
      __IO uint32_t SRAM_DS_ULP_PROC_1 : 10;
      __IO uint32_t RESERVED2 : 6;
    } ULPSS_RAM_PWRCTRL_SET_REG2_b; /*!< BitSize                                                               */
  };

  /*0x58*/
  union {
    __IO uint32_t
      ULPSS_RAM_PWRCTRL_CLEAR_REG2; /*!< none                                                                  */

    struct {
      __IO uint32_t ULPTASS_SRAM_INPUT_DISABLE_ISOLATION_ULP : 8;
      __IO uint32_t RESERVED1 : 8;
      __IO uint32_t SRAM_DS_ULP_PROC : 10;
      __IO uint32_t RESERVED2 : 6;
    } ULPSS_RAM_PWRCTRL_CLEAR_REG2_b; /*!< BitSize                                                               */
  };

  /*0x5C*/
  union {
    __IO uint32_t ULPSS_RAM_PWRCTRL_SET_REG3;
    struct {
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 8;
      __IO uint32_t RES : 24;
    } ULPSS_RAM_PWRCTRL_SET_REG3_b;
  };

  /*0x60*/

  union {
    __IO uint32_t ULPSS_RAM_PWRCTRL_CLEAR_REG3;
    struct {
      __IO uint32_t ULPTASS_EXT_PWRGATE_EN_N_ULP_SRAM_PERI_1 : 8;
      __IO uint32_t RES : 24;
    } ULPSS_RAM_PWRCTRL_CLEAR_REG3_b;
  };
  /*0x64*/
  union {
    __IO uint32_t MCU_FSM_CTRL_BYPASS; /*!< none                                                                  */

    struct {
      __IO uint32_t MCU_XTAL_EN_40MHZ_BYPASS_CTRL : 1;
      __IO uint32_t MCU_XTAL_EN_40MHZ_BYPASS : 1;
      __IO uint32_t MCU_PMU_SHUT_DOWN_BYPASS_CTRL : 1;
      __IO uint32_t MCU_PMU_SHUT_DOWN_BYPASS : 1;
      __IO uint32_t MCU_BUCK_BOOST_ENABLE_BYPASS_CTRL : 1;
      __IO uint32_t MCU_BUCK_BOOST_ENABLE_BYPASS : 1;
      __IO uint32_t RES : 26;
    } MCU_FSM_CTRL_BYPASS_b; /*!< BitSize                                                               */
  };

  /*0x68*/
  union {
    __IO uint32_t MCU_PMU_LDO_CTRL_SET; /*!< none                                                                  */

    struct {
      __IO uint32_t MCU_FLASH_LDO_EN : 1;
      __IO uint32_t MCU_SCO_LDO_EN : 1;
      __IO uint32_t MCU_DCDC_EN : 1;
      __IO uint32_t RESERVED : 14;
      __IO uint32_t MCU_SOC_LDO_LVL : 1;
      __IO uint32_t MCU_DCDC_LVL : 1;
      __IO uint32_t RES : 13;
    } MCU_PMU_LDO_CTRL_SET_b; /*!< BitSize                                                               */
  };

  /*0x6C	*/
  union {
    __IO uint32_t MCU_PMU_LDO_CTRL_CLEAR;
    struct {
      __IO uint32_t MCU_FLASH_LDO_EN : 1;
      __IO uint32_t MCU_SCO_LDO_EN : 1;
      __IO uint32_t MCU_DCDC_EN : 1;
      __IO uint32_t RESERVED : 14;
      __IO uint32_t MCU_SOC_LDO_LVL : 1;
      __IO uint32_t MCU_DCDC_LVL : 1;
      __IO uint32_t RES : 13;
    } MCU_PMU_LDO_CTRL_CLEAR_b;
  };
  /*0x70*/
  uint32_t RESERVED6;
  /*0x74*/
  uint32_t RESERVED7;
  /*0x78*/
  uint32_t RESERVED8;
  /*0x7C*/
  uint32_t RESERVED9;

  /*0x80*/
  union {
    __IO uint32_t PLLCCI_PWRCTRL_REG; /*!< none                                                                  */

    struct {
      __IO uint32_t I2SPLL_ISO_EN : 1;
      __IO uint32_t I2SPLL_BYPASS_ISO_GEN : 1;
      __IO uint32_t INTFPLL_ISO_EN : 1;
      __IO uint32_t INTFPLL_BYPASS_ISO_GEN : 1;
      __IO uint32_t SOCPLL_ISO_ENABLE : 1;
      __IO uint32_t SOCPLL_BYPASS_ISO_GEN : 1;
      __IO uint32_t SOCPLL_SPI_PG_EN : 1;
      __IO uint32_t SOCPLL_VDD13_ISO_EN_b : 1;
      __IO uint32_t RES : 24;
    } PLLCCI_PWRCTRL_REG_b; /*!< BitSize                                                               */
  };

  /*0x84*/
  union {
    __IO uint32_t DLL_PWRCTRL_REG;
    struct {
      __IO uint32_t QSPI_DLL_RX_ISO_ENABLE : 1;
      __IO uint32_t QSPI_DLL_RX_BYPASS_ISO_GEN : 1;
      __IO uint32_t QSPI_DLL_RX_PG_EN_N : 1;
      __IO uint32_t RESERVED : 1;
      __IO uint32_t QSPI_DLL_TX_ISO_ENABLE : 1;
      __IO uint32_t QSPI_DLL_TX_BYPASS_ISO_GEN : 1;
      __IO uint32_t QSPI_DLL_TX_PG_EN_N : 1;
      __IO uint32_t RESERVED1 : 25;
    } DLL_PWRCTRL_REG_b;
  };

  uint32_t RES1[30];
} MCU_Battery;

/*START OF MCU FSM*/
typedef struct {

  /*0x00*/
  union {
    __IO uint32_t MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE;
    struct {
      __IO uint32_t MCUFSM_SHUTDOWN_ENABLE : 1;
      __IO uint32_t RES : 1;
      __IO uint32_t LP_SLEEP_MODE_b : 1;
      __IO uint32_t M4SS_RAM_RETENTION_MODE_EN : 1;
      __IO uint32_t M4ULP_RAM_RETENTION_MODE_EN_b : 1;
      __IO uint32_t TA_RAM_RETENTION_MODE_EN : 1;
      __IO uint32_t ULPSS_RAM_RETENTION_MODE_EN : 1;
      __IO uint32_t M4ULP_RAM16K_RETENTION_MODE_EN : 1;
      __IO uint32_t LDO_SOC_ON_b : 1;
      __IO uint32_t LDO_FLASH_ON_b : 1;
      __O uint32_t PMU_DCDC_ON_b : 1;
      __IO uint32_t SKIP_XTAL_WAIT_TIME : 1;
      __IO uint32_t RESERVED1 : 2;
      __IO uint32_t MCUFSM_WAKEUP_NWPFSM : 1;
      __O uint32_t SLEEP_WAKEUP : 1;
      __IO uint32_t TIMER_BASED_WAKEUP_b : 1;
      __IO uint32_t HOST_BASED_WAKEUP_b : 1;
      __IO uint32_t WIRELESS_BASED_WAKEUP_b : 1;
      __IO uint32_t M4_PROC_BASED_WAKEUP_b : 1;
      __IO uint32_t GPIO_BASED_WAKEUP_b : 1;
      __IO uint32_t COMPR_BASED_WAKEUP_b : 1;
      __IO uint32_t WAKEIF_BASED_WAKEUP_b : 1;
      __IO uint32_t WIC_BASED_WAKEUP_b : 1;
      __IO uint32_t ULPSS_BASED_WAKEUP_b : 1;
      __IO uint32_t SDCSS_BASED_WAKEUP_b : 1;
      __IO uint32_t ALARM_BASED_WAKEUP_b : 1;
      __IO uint32_t SEC_BASED_WAKEUP_b : 1;
      __IO uint32_t MSEC_BASED_WAKEUP_b : 1;
      __IO uint32_t WDT_INTR_BASED_WAKEUP_b : 1;
      __IO uint32_t ULPSS_BASED_SLEEP : 1;
      __IO uint32_t SDCSS_BASED_SLEEP : 1;
    } MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b;
  };

  /*0x04*/
  union {
    __IO uint32_t MCU_FSM_PERI_CONFIG_REG;
    struct {
      __IO uint32_t ULP_MCU_MODE_EN : 1;
      __IO uint32_t M4SS_CONTEXT_SWITCH_TOP_ULP_MODE : 2;
      __IO uint32_t WICENREQ : 1;
      __IO uint32_t RESERVED1 : 12;
      __IO uint32_t BGPMU_SAMPLING_EN_R : 1;
      __IO uint32_t RESERVED2 : 15;

    } MCU_FSM_PERI_CONFIG_REG_b;
  };

  /*0x08*/
  union {
    __IO uint32_t GPIO_WAKEUP_REGISTER;
    struct {
      __IO uint32_t GPIO_0_WAKEUP : 1;
      __IO uint32_t GPIO_1_WAKEUP : 1;
      __IO uint32_t GPIO_2_WAKEUP : 1;
      __IO uint32_t GPIO_3_WAKEUP : 1;
      __IO uint32_t GPIO_4_WAKEUP : 1;
      __IO uint32_t RESERVED1 : 11;
      __IO uint32_t CONTINIOUS_START : 1;
      __IO uint32_t CONTINIOUS_TIMER_ENABLE : 1;
      __IO uint32_t DS_TIMER_SOFT_RESET : 1;
      __IO uint32_t RESERVED2 : 13;
    } GPIO_WAKEUP_REGISTER_b;
  };

  /*0x0C*/
  union {
    __IO uint32_t MCU_FSM_DEEP_SLEEP_DURATION_LSB_REG;
    struct {
      __IO uint32_t MCUFSM_DEEPSLEEP_DURATION_COUNT : 32;
    } MCU_FSM_DEEP_SLEEP_DURATION_LSB_REG_b;
  };

  /*0x10*/
  union {
    __IO uint32_t MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG;
    struct {
      __IO uint32_t MCUFSM_PMU_POWERGOOD_DURATION_COUNT : 5;
      __IO uint32_t RESERVED1 : 11;
      __IO uint32_t MCUFSM_XTAL_GOODTIME_DURATION_COUNT : 5;
      __IO uint32_t RESERVED2 : 11;
    } MCU_FSM_XTAL_AND_PMU_GOOD_COUNT_REG_b;
  };

  /*0x14*/
  union {
    __IO uint32_t MCU_FSM_POWER_CTRL_AND_DELAY;
    struct {
      __IO uint32_t PS2_PMU_LDO_OFF_DELAY : 5;
      __IO uint32_t RESERVED : 3;
      __IO uint32_t PS4_SOCLDO_ON_DELAY : 4;
      __IO uint32_t PG4_BUCK_ON_DELAY : 4;
      __IO uint32_t FSM_PERI_SOC_LDO_EN : 1;
      __IO uint32_t FSM_PERI_DCDC_EN : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t POWER_MUX_SEL_ULPSS : 1;
      __IO uint32_t POWER_MUX_SEL_M4_ULP : 2;
      __IO uint32_t POWER_MUX_SEL_M4_ULP_RAM_16K : 2;
      __IO uint32_t POWER_MUX_SEL_M4_ULP_RAM : 2;
      __IO uint32_t POWER_MUX_SEL_ULPSS_RAM : 2;
      __IO uint32_t RESERVED3 : 4;
    } MCU_FSM_POWER_CTRL_AND_DELAY_b;
  };

  /*0x18*/
  union {
    __IO uint32_t MCU_FSM_CLKS_REG;
    struct {
      __IO uint32_t RESERVED : 2;
      __IO uint32_t HF_FSM_CLK_SELECT : 3;
      __IO uint32_t RESERVED1 : 10;
      __IO uint32_t HF_FSM_CLK_SWITCHED_SYNC : 1;
      __IO uint32_t HF_FSM_CLK_FREQ : 6;
      __IO uint32_t US_DIV_COUNT : 2;
      __IO uint32_t HF_FSM_GEN_2MHZ : 1;
      __IO uint32_t HF_FSM_CLK_EN : 1;
      __IO uint32_t RESERVED2 : 6;

    } MCU_FSM_CLKS_REG_b;
  };
  /*0x1C*/
  union {
    __IO uint32_t MCU_FSM_REF_CLK_REG;
    struct {
      __IO uint32_t M4SS_REF_CLK_SEL : 3;
      __IO uint32_t RESERVED1 : 4;
      __IO uint32_t M4SS_REF_CLK_CLEANER_OFF_b : 1;
      __IO uint32_t M4SS_REF_CLK_CLEANER_ON_b : 1;
      __IO uint32_t RESERVED2 : 3;
      __IO uint32_t TASS_REF_CLK_SEL : 3;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t ULPSS_REF_CLK_SEL_b : 3;
      __IO uint32_t RESERVED4 : 4;
      __IO uint32_t ULPSS_REF_CLK_CLEANER_OFF_b : 1;
      __IO uint32_t ULPSS_REF_CLK_CLEANER_ON_b : 1;
      __IO uint32_t RESERVED5 : 3;
      __IO uint32_t SDCSS_CLK_SEL_b : 2;
      __IO uint32_t SDCSS_CLK_EN_b : 1;
      __IO uint32_t SDCSS_STATIC_CLK_EN_b : 1;
    } MCU_FSM_REF_CLK_REG_b;
  };
  /*0x20*/
  union {
    __IO uint32_t MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP;
    struct {
      __IO uint32_t FIRST_BOOTUP_MCU_N_b : 1;
      __IO uint32_t RAM_RETENTION_STATUS_M4SS_b : 1;
      __IO uint32_t RETENTION_DOMAIN_ON_b : 1;
      __IO uint32_t CHIP_MODE_VALID_b : 1;
      __IO uint32_t STORAGE_DOMAIN_ON_b : 1;
      __IO uint32_t RESERVED : 10;
      __IO uint32_t MCU_FSM_RESET_N_SYNC_b : 1;
      __IO uint32_t MCU_ULP_32KHZ_RC_CLK_EN_b : 1;
      __IO uint32_t MCU_ULP_32KHZ_RO_CLK_EN_b : 1;
      __IO uint32_t MCU_ULP_32KHZ_XTAL_CLK_EN_b : 1;
      __IO uint32_t MCU_ULP_MHZ_RC_CLK_EN_b : 1;
      __IO uint32_t MCU_ULP_20MHZ_RING_OSC_CLK_EN_b : 1;
      __IO uint32_t MCU_ULP_DOUBLER_CLK_EN_b : 1;
      __IO uint32_t MCU_ULP_40MHZ_CLK_EN_b : 1;
      __IO uint32_t RESERVED1 : 9;
    } MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b;
  };

  /*0x24*/
  union {
    __IO uint32_t MCU_FSM_CRTL_PDM_AND_ENABLES;
    struct {
      __IO uint32_t ENABLE_WDT_IN_SLEEP_b : 1;
      __IO uint32_t ENABLE_WURX_DETECTION_b : 1;
      __IO uint32_t RESET_MCU_BBF_DM_EN_b : 1;
      __IO uint32_t DISABLE_TURNOFF_SRAM_PERI_b : 1;
      __IO uint32_t ENABLE_SRAM_DS_CRTL_b : 1;
      __IO uint32_t RESERVED : 11;
      __IO uint32_t POWER_ENABLE_FSM_PERI_b : 1;
      __IO uint32_t POWER_ENABLE_TIMESTAMPING_b : 1;
      __IO uint32_t POWER_ENABLE_DEEPSLEEP_TIMER_b : 1;
      __IO uint32_t POWER_ENABLE_RETENTION_DM_b : 1;
      __IO uint32_t RESERVED2 : 12;
    } MCU_FSM_CRTL_PDM_AND_ENABLES_b;
  };

  /*0x28*/
  union {
    __IO uint32_t MCU_GPIO_TIMESTAMPING_CONFIG;
    struct {
      __IO uint32_t ENABLE_GPIO_TIMESTAMPING_b : 1;
      __IO uint32_t TIMESTAMPING_ON_GPIO0_b : 1;
      __IO uint32_t TIMESTAMPING_ON_GPIO1_b : 1;
      __IO uint32_t TIMESTAMPING_ON_GPIO2_b : 1;
      __IO uint32_t TIMESTAMPING_ON_GPIO3_b : 1;
      __IO uint32_t TIMESTAMPING_ON_GPIO4_b : 1;
      __IO uint32_t RESERVED1 : 10;
      __IO uint32_t RESERVED2 : 16;
    } MCU_GPIO_TIMESTAMPING_CONFIG_b;
  };

  /*0x2C*/
  union {
    __IO uint32_t MCU_GPIO_TIMESTAMP_READ;
    struct {
      __IO uint32_t GPIO_EVENT_COUNT_PARTIAL : 11;
      __IO uint32_t RESERVED : 5;
      __IO uint32_t GPIO_EVENT_COUNT_FULL : 11;
      __IO uint32_t RESERVED2 : 5;
    } MCU_GPIO_TIMESTAMP_READ_b;
  };

  /*0x30*/
  union {
    __IO uint32_t MCU_SLEEPHOLD_REQ;
    struct {
      __IO uint32_t SLEEPHOLDREQn : 1;
      __IO uint32_t SLEEPHOLDACKn : 1;
      __IO uint32_t RESERVED1 : 14;
      __IO uint32_t SELECT_FSM_MODE : 1;
      __IO uint32_t RESERVED2 : 15;
    } MCU_SLEEPHOLD_REQ_b;
  };

  /*0x34*/
  __IO uint32_t RESERVED4;

  /*0x38*/
  union {
    __IO uint32_t MCU_FSM_WAKEUP_STATUS_REG;
    struct {
      __IO uint32_t WAKEUP_STATUS : 11;
      __IO uint32_t RESERVED : 5;
      __IO uint32_t MCU_FIRST_POWERUP_POR : 1;
      __IO uint32_t MCU_FIRST_POWERUP_RESET_N : 1;
      __IO uint32_t RESERVED2 : 14;
    } MCU_FSM_WAKEUP_STATUS_REG_b;
  };

  /*0x3C*/
  union {
    __IO uint32_t MCU_FSM_WAKEUP_STATUS_CLEAR;
    struct {
      __IO uint32_t WWD_INTERRUPT_STATUS_CLEAR_b : 1;
      __IO uint32_t MILLI_SEC_BASED_STATUS_CLEAR_b : 1;
      __IO uint32_t RTC_SEC_BASED_STATUS_CLEAR_b : 1;
      __IO uint32_t RTC_ALARM_BASED_WAKEUP_STATUS_CLEAR_b : 1;
      __IO uint32_t COMP1_BASED_WAKEUP_STATUS_CLEAR_b : 1;
      __IO uint32_t COMP2_BASED_WAKEUP_STATUS_CLEAR_b : 1;
      __IO uint32_t COMP3_BASED_WAKEUP_STATUS_CLEAR_b : 1;
      __IO uint32_t COMP4_BASED_WAKEUP_STATUS_CLEAR_b : 1;
      __IO uint32_t COMP5_BASED_WAKEUP_STATUS_CLEAR_b : 1;
      __IO uint32_t COMP6_BASED_WAKEUP_STATUS_CLEAR_b : 1;
      __IO uint32_t RF_WAKEUP_CLEAR_b : 1;
      __IO uint32_t RESERVED : 21;
    } MCU_FSM_WAKEUP_STATUS_CLEAR_b;
  };

  /*0x40*/
  union {
    __IO uint32_t MCU_FSM_PMU_STATUS_REG;
    struct {
      __IO uint32_t SCDCDC_LP_MODE_EN : 1;
      __IO uint32_t BGPMU_SLEEP_EN_R_b : 1;
      __IO uint32_t RESERVED : 15;
      __IO uint32_t STANDBY_LDORF_R : 1;
      __IO uint32_t STANDBY_LDOSOC_R : 1;
      __IO uint32_t STANDBY_DC1P3_R : 1;
      __IO uint32_t POWERGOOD_LDOSOC : 1;
      __IO uint32_t POWERGOOD_LDORF : 1;
      __IO uint32_t POWERGOOD_DC1P3 : 1;
      __IO uint32_t LEVEL_OK_DC1P3 : 1;
      __IO uint32_t CL_FLAG_DC1P3 : 1;
      __IO uint32_t RESERVED1 : 7;
    } MCU_FSM_PMU_STATUS_REG_b;
  };

  /*0x44*/
  union {
    __IO uint32_t MCU_FSM_PMUX_CTRLS_RET;
    struct {
      __IO uint32_t POWER_SW_CTRL_TASS_RAM_IN_RETAIN : 1;
      __IO uint32_t POWER_SW_CTRL_M4SS_RAM_IN_RETAIN : 1;
      __IO uint32_t POWER_SW_CTRL_M4ULP_RAM_IN_RETAIN : 2;
      __IO uint32_t POWER_SW_CTRL_M4ULP_RAM16K_IN_RETAIN : 2;
      __IO uint32_t POWER_SW_CTRL_ULPSS_RAM_IN_RETAIN : 2;
      __IO uint32_t RESERVED : 24;
    } MCU_FSM_PMUX_CTRLS_RET_b;
  };
  /*0x48*/
  union {
    __IO uint32_t MCU_FSM_TOGGLE_COUNT;
    struct {
      __IO uint32_t TOGGLE_COUNT_RSTART : 1;
      __IO uint32_t RESERVED1 : 14;
      __IO uint32_t LATCH_TOGGLE_DATA : 1;
      __IO uint32_t GPIO_TOGGLE_COUNT : 12;
      __IO uint32_t RESERVED : 3;
      __IO uint32_t TOGGLE_DATA_READY : 1;
    } MCU_FSM_TOGGLE_COUNT_b;
  };

} MCU_Fsm_t;

/*End of mcu FSM */

/*Start process sensor */
typedef struct {
  union {
    __IO uint32_t PROCESS_SENSOR_ENABLE_AND_READ;
    struct {
      __IO uint32_t PROCESS_SENSOR_EN : 1;
      __IO uint32_t PS_RING_CLK_START : 1;
      __IO uint32_t PS_CLK_SW_ON : 1;
      __IO uint32_t PS_CLK_SW_OFF : 1;
      __IO uint32_t NUM_CYCLES : 2;
      __IO uint32_t RESERVED : 10;
      __IO uint32_t PS_COUNT : 17;
    } PROCESS_SENSOR_ENABLE_AND_READ_b;
  };
} MCU_ProcessSensor_Type;
/*End process sensor */

/*End MCU battery Flip Flops */

typedef struct {
  /*0x1C*/
  union {
    __IO uint32_t NPSS_GPIO_CTRLS; /*!< none                                                                  */
    struct {
      __IO uint32_t NPSS_GPIO_MODE : 3;
      __IO uint32_t NPSS_GPIO_REN : 1;
      __IO uint32_t NPSS_GPIO_OEN : 1;
      __IO uint32_t NPSS_GPIO_OUT : 1;
      __IO uint32_t RESERVED1 : 2;
      __IO uint32_t NPSS_GPIO_POLARITY : 1;
      __IO uint32_t RESERVED2 : 7;
      __IO uint32_t USE_ULPSS_PAD : 1;
      __IO uint32_t RESERVED4 : 15;

    } NPSS_GPIO_CTRLS_b; /*!< BitSize                                                               */
  };
} NPSS_GPIO_CNTRL_Type;

/*START RETENTION  */
typedef struct {

  /*0x00*/
  union {
    __IO uint32_t MCURET_QSPI_WR_OP_DIS;
    struct {
      __IO uint32_t M4SS_QSPI_WRSR_WR_OP_DISABLE : 1;
      __O uint32_t TASS_QSPI_WRSR_WR_OP_DISABLE : 1;
      __IO uint32_t RESERVED : 30;
    } MCURET_QSPI_WR_OP_DIS_b;
  };

  /*0x04*/
  union {
    __IO uint32_t MCURET_BOOTSTATUS;
    struct {
      __IO uint32_t BOOT_STATUS : 31;
      __IO uint32_t RESERVED : 1;
    } MCURET_BOOTSTATUS_b;
  };

  /*0x08*/
  __IO uint32_t RESERVED2;

  /*0x0C*/
  union {
    __IO uint32_t CHIP_CONFIG_MCU_READ; /*!< none                                                                  */

    struct {
      __IO uint32_t DISABLE_M4 : 1;
      __IO uint32_t LIMIT_M4_FREQ_110MHZ_b : 1;
      __IO uint32_t DISABLE_M4_ULP_MODE : 1;
      __IO uint32_t DISABLE_USB : 1;
      __IO uint32_t DISABLE_ETHERNET : 1;
      __IO uint32_t DISABLE_CCI : 1;
      __IO uint32_t RESERVED : 4;
      __IO uint32_t M4_FLASH_SIZE : 3;
      __IO uint32_t DISABLE_FIM_COP : 1;
      __IO uint32_t DISABLE_VAP : 1;
      __IO uint32_t DISABLE_TOUCH : 1;
      __IO uint32_t DISABLE_WURX : 1;
      __IO uint32_t DISABLE_ANALOG_PERIPH : 1;
      __IO uint32_t DISABLE_JTAG : 1;
      __IO uint32_t DISABLE_M4SS_KH_ACCESS : 1;
      __IO uint32_t DISABLE_M4SS_ACCESS_FRM_TASS_SEC : 1;
      __IO uint32_t DISABLE_CAN_INTERFACE : 1;
      __IO uint32_t RESERVED1 : 10;

    } CHIP_CONFIG_MCU_READ_b; /*!< BitSize                                                               */
  };
  /*0x10 : TEST mux no need to write APIs for this */
  union {
    __IO uint32_t MCUAON_CTRL_REG4;
    struct {
      __IO uint32_t RES : 16;
      __IO uint32_t ULP_GPIO_2_TEST_MODE_OUT_SEL : 4;
      __IO uint32_t ULP_GPIO_1_TEST_MODE_OUT_SEL : 4;
      __IO uint32_t ULP_GPIO_0_TEST_MODE_OUT_SEL : 4;
      __IO uint32_t ULP_GPIOS_IN_TEST_MODE : 1;
      __IO uint32_t RESERVED : 3;
    };
  };
  /*0x14*/
  __IO uint32_t RESERVED4;

  /*0x18*/
  __IO uint32_t RESERVED5;

  /*0x1C*/
  NPSS_GPIO_CNTRL_Type NPSS_GPIO_CNTRL[5];
} MCU_RETENTION_Type;

/*END RETENTION  */

typedef struct {
  /*0x1C*/
  union {
    __IO uint32_t NPSS_GPIO_CTRLS; /*!< none                                                                  */
    struct {
      __IO uint32_t NPSS_GPIO_MODE : 3;
    } NPSS_GPIO_CTRLS_b; /*!< BitSize                                                               */
  };
} TEMPSENSOR_Type;

/*TEMPERATURE SENSOR START */
typedef struct {
  /*0x00*/
  union {
    __IO uint32_t
      TS_ENABLE_AND_TEMPERATURE_DONE; /*!< none                                                                  */
    struct {
      __IO uint32_t TEMP_SENS_EN : 1;
      __IO uint32_t REF_CLK_SEL : 1;
      __IO uint32_t CONT_COUNT_FREEZE : 10;
      __IO uint32_t TEMP_MEASUREMENT_DONE : 1;
      __IO uint32_t RESERVED : 19;
    } TS_ENABLE_AND_TEMPERATURE_DONE_b;
  }; /*!< BitSize                                                               */

  /*0x04*/
  union {
    __IO uint32_t TS_SLOPE_SET; /*!< none                                                                  */
    struct {
      __IO uint32_t SLOPE : 10;
      //FIXME :  improper bit locations in TRM
      __IO uint32_t RESERVED : 6;
      __IO uint32_t TEMPERATURE_SPI : 11;
      __IO uint32_t TEMP_UPDATED : 1;
      __IO uint32_t BJT_BASED_TEMP : 1;
      __IO uint32_t RESERVED1 : 3;
    } TS_SLOPE_SET_b; /*!< BitSize                                                               */
  };
  /*0x08*/
  union {
    __IO uint32_t
      TS_FE_COUNTS_NOMINAL_SETTINGS; /*!< none                                                                  */
    struct {
      __IO uint32_t F2_NOMINAL : 10;
      __IO uint32_t RESERVED : 6;
      __IO uint32_t NOMINAL_TEMPERATURE : 7;
      __IO uint32_t RESERVED1 : 9;
    } TS_FE_COUNTS_NOMINAL_SETTINGS_b; /*!< BitSize                                                               */
  };
  /*0x0C*/
  union {
    __IO uint32_t TS_COUNTS_READ; /*!< none                                                                  */
    struct {
      __IO uint32_t COUNT_F2 : 10;
      __IO uint32_t RESERVED : 6;
      __IO uint32_t COUNT_F1 : 10;
      __IO uint32_t RESERVED1 : 6;
    } TS_COUNTS_READ_b; /*!< BitSize                                                               */
  };
  /*0x10*/
  union {
    __IO uint32_t TEMPERATURE_READ; /*!< none                                                                  */
    struct {
      __IO uint32_t TEMPERATURE_RD : 11;
      __IO uint32_t RESERVED : 21;
    } TEMPERATURE_READ_b; /*!< BitSize                                                               */
  };
} MCU_TEMP_Type;

/*TEMPERATURE END */

/*MCU AON START*/
typedef struct {
  /*0x00*/
  union {
    __IO uint32_t
      MCUAON_NPSS_PWRCTRL_SET_REG; /*!< none                                                                  */
    struct {
      __IO uint32_t RES : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUBFFS : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUFSM : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCURTC : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUWDT : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUPS : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUTS : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUSTORE1 : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUSTORE2 : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUSTORE3 : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_TIMEPERIOD : 1;
      __IO uint32_t RESERVED : 5;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_NWPAPB_MCU_CTRL : 1;
      __IO uint32_t RESERVED1 : 15;
    } MCUAON_NPSS_PWRCTRL_SET_REG_b;
  };
  /*0x04*/
  union {
    __IO uint32_t
      MCUAON_NPSS_PWRCTRL_CLEAR_REG; /*!< none                                                                  */
    struct {
      __IO uint32_t RES : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUBFFS : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUFSM : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCURTC : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUWDT : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUPS : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUTS : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUSTORE1 : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUSTORE2 : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_MCUSTORE3 : 1;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_TIMEPERIOD : 1;
      __IO uint32_t RESERVED : 5;
      __IO uint32_t SLPSS_PWRGATE_EN_N_ULP_NWPAPB_MCU_CTRL : 1;
      __IO uint32_t RESERVED1 : 15;
    } MCUAON_NPSS_PWRCTRL_CLEAR_REG_b;
  };

  /*0x08*/
  __IO uint32_t RESERVED1;

  /*0x0C*/
  union {
    __IO uint32_t MCUAON_IPMU_RESET_CTRL; /*!< none                                                                  */
    struct {
      __IO uint32_t ULP_ANALOG_SPI_RESET_N : 1;
      __IO uint32_t IPMU_SPI_RESET_N : 1;
      __IO uint32_t RESERVED : 30;
    } MCUAON_IPMU_RESET_CTRL_b;
  };

  /*0x10*/
  union {
    __IO uint32_t MCUAON_SHELF_MODE; /*!< none                                                                  */
    struct {
      __IO uint32_t ENTER_SHELF_MODE : 16;
      __IO uint32_t SHUTDOWN_WAKEUP_MODE : 2;
      __IO uint32_t SHELF_MODE_GPIOBASED : 1;
      __IO uint32_t SHELF_MODE_WAKEUP_DELAY : 3;
      __IO uint32_t RESERVED1 : 10;
    } MCUAON_SHELF_MODE_b;
  };

  /*0x14*/
  union {
    __IO uint32_t MCUAON_GEN_CTRLS; /*!< none                                                                  */
    struct {
      __IO uint32_t XTAL_CLK_FROM_GPIO : 1;
      __IO uint32_t ULP_ANALOG_WAKEUP_ACCESS : 1;
      __IO uint32_t RES : 14;
      __IO uint32_t ENABLE_PDO : 1;
      __IO uint32_t NPSS_SUPPLY_0P9 : 1;
      __IO uint32_t RESERVED : 14;
    } MCUAON_GEN_CTRLS_b;
  };
  /*0x18*/
  union {
    __IO uint32_t MCUAON_PDO_CTRLS; /*!< none                                                                  */
    struct {
      __IO uint32_t SOC_B_IO_DOMAIN_EN_B : 1;
      __IO uint32_t SOC_L_IO_DOMAIN_EN_B : 1;
      __IO uint32_t SOC_T_IO_DOMAIN_EN_B : 1;
      __IO uint32_t QSPI_IO_DOMAIN_EN_B : 1;
      __IO uint32_t SDIO_IO_DOMAIN_EN_B : 1;
      __IO uint32_t RES : 27;
    } MCUAON_PDO_CTRLS_b;
  };

  /*0x1C*/
  union {
    __IO uint32_t MCUAON_WDT_CHIP_RST; /*!< none                                                                  */
    struct {
      __IO uint32_t MCU_WDT_BASED_CHIP_RESET : 1;
      __IO uint32_t RESERVED : 31;
    } MCUAON_WDT_CHIP_RST_b;
  };

  /*0x20*/
  union {
    __IO uint32_t
      MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS; /*!< none                                                                  */
    struct {
      __IO uint32_t AON_KHZ_CLK_SEL : 3;
      __IO uint32_t AON_KHZ_CLK_SEL_CLOCK_SWITCHED : 1;
      __IO uint32_t RES : 12;
      __IO uint32_t MCU_FIRST_POWERUP_POR : 1;
      __IO uint32_t MCU_FIRST_POWERUP_RESET_N : 1;
      __IO uint32_t RESERVED : 14;
    } MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b;
  };

} MCU_AON_Type;

/*MCU AON END */

/*START ULPSS CLOCKS */

typedef struct {
  union {
    __IO uint32_t ULP_SOC_GPIO_0_MODE_REG;
    struct {
      __IO uint32_t ULP_SOCGPIO_N_MODE : 3;
      __IO uint32_t RESERVED : 29;
    } ULP_SOC_GPIO_0_MODE_REG_b;
  };
} ULP_SOC_GPIO_MODE_t;

typedef struct {
  /*0x00*/
  union {
    __IO uint32_t ULP_MISC_SOFT_SET_REG;
    struct {
      __IO uint32_t PCM_ENABLE_b : 1;
      __IO uint32_t PCM_FSYNC_START_b : 1;
      __IO uint32_t BIT_RES : 2;
      __IO uint32_t IR_PCLK_EN_b : 1;
      __IO uint32_t PCLK_ENABLE_I2C_b : 1;
      __IO uint32_t CLK_ENABLE_I2S_b : 1;
      __IO uint32_t PCLK_ENABLE_SSI_MASTER_b : 1;
      __IO uint32_t SCLK_ENABLE_SSI_MASTER_b : 1;
      __IO uint32_t PCLK_ENABLE_UART_b : 1;
      __IO uint32_t SCLK_ENABLE_UART_b : 1;
      __IO uint32_t FIM_PCLK_ENABLE_b : 1;
      __IO uint32_t VAD_PCLK_ENABLE_b : 1;
      __IO uint32_t CLK_ENABLE_TIMER_b : 1;
      __IO uint32_t EGPIO_CLK_EN_b : 1;
      __IO uint32_t REG_ACCESS_SPI_CLK_EN_b : 1;
      __IO uint32_t FIM_CLK_EN_b : 1;
      __IO uint32_t VAD_CLK_EN_b : 1;
      __IO uint32_t CLK_ENABLE_ULP_MEMORIES_b : 1;
      __IO uint32_t EGPIO_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t EGPIO_PCLK_ENABLE_b : 1;
      __IO uint32_t TIMER_PCLK_EN_b : 1;
      __IO uint32_t AUX_ULP_EXT_TRIG_1_SEL_b : 1;
      __IO uint32_t AUX_ULP_EXT_TRIG_2_SEL_b : 1;
      __IO uint32_t AUX_SOC_EXT_TRIG_1_SEL_b : 1;
      __IO uint32_t AUX_SOC_EXT_TRIG_2_SEL_b : 1;
      __IO uint32_t ULPSS_M4SS_SLV_SEL_b : 1;
      __IO uint32_t ULPSS_TASS_QUASI_SYNC_b : 1;
      __IO uint32_t RESERVED : 2;
      __IO uint32_t FIM_AHB_CLK_ENABLE_b : 1;
      __IO uint32_t TOUCH_SENSOR_PCLK_ENABLE_b : 1;
    } ULP_MISC_SOFT_SET_REG_b;
  };

  /*0x04*/
  union {
    __IO uint32_t ULP_TA_PERI_ISO_REG;
    struct {
      __IO uint32_t UDMA_ISO_CNTRL_b : 1;
      __IO uint32_t IR_ISO_CNTRL_b : 1;
      __IO uint32_t I2C_ISO_CNTRL_b : 1;
      __IO uint32_t I2S_ISO_CNTRL_b : 1;
      __IO uint32_t SSI_ISO_CNTRL_b : 1;
      __IO uint32_t UART_ISO_CNTRL_b : 1;
      __IO uint32_t AUX_A2D_ISO_CNTRL_b : 1;
      __IO uint32_t VAD_ISO_CNTRL_b : 1;
      __IO uint32_t TOUCH_ISO_CNTRL_b : 1;
      __IO uint32_t PROC_MISC_ISO_CNTRL_b : 1;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t RESERVED2 : 1;
      __IO uint32_t RESERVED3 : 1;
      __IO uint32_t FIM_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_1_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_2_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_3_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_4_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_5_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_6_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_7_ISO_CNTRL_b : 1;
      __IO uint32_t MEM_2K_8_ISO_CNTRL_b : 1;
      __IO uint32_t RESERVED4 : 9;
    } ULP_TA_PERI_ISO_REG_b;
  };

  /*0x08*/
  union {
    __IO uint32_t ULP_TA_PERI_RESET_REG;
    struct {
      __IO uint32_t UDMA_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t IR_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t I2C_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t I2S_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t SSI_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t UART_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t AUX_A2D_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t VAD_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t TOUCH_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t PROC_MISC_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t COMP1_OUTPUT_CNTRL_b : 1;
      __IO uint32_t COMP2_OUTPUT_CNTRL_b : 1;
      __IO uint32_t RESERVED1 : 2;
      __IO uint32_t FIM_SOFT_RESET_CNTRL_b : 1;
      __IO uint32_t RESERVED : 17;
    } ULP_TA_PERI_RESET_REG_b;
  };

  /*0x0C*/
  __IO uint32_t RESERVED0;

  /*0x10*/
  __IO uint32_t RESERVED8;

  /*0x14*/
  union {
    __IO uint32_t ULP_TA_CLK_GEN_REG;
    struct {
      __IO uint32_t ULP2M4_A2A_BRDG_CLK_EN_b : 1;
      __IO uint32_t ULP_PROC_CLK_SEL : 4;
      __IO uint32_t ULP_PROC_CLK_DIV_FACTOR : 8;
      __IO uint32_t RES : 19;
    } ULP_TA_CLK_GEN_REG_b;
  };

  /*0x18*/
  union {
    __IO uint32_t ULP_I2C_SSI_CLK_GEN_REG;
    struct {
      __IO uint32_t ULP_I2C_CLK_EN_b : 1;
      __IO uint32_t RESERVED1 : 4;
      __IO uint32_t RESERVED2 : 8;
      __IO uint32_t RESERVED3 : 3;
      __IO uint32_t ULP_SSI_CLK_EN_b : 1;
      __IO uint32_t ULP_SSI_CLK_DIV_FACTOR : 7;
      __IO uint32_t RESERVED4 : 4;
      __IO uint32_t ULP_SSI_CLK_SEL : 4;
    } ULP_I2C_SSI_CLK_GEN_REG_b;
  };

  /*0x1C*/
  union {
    __IO uint32_t ULP_I2S_CLK_GEN_REG;
    struct {
      __IO uint32_t ULP_I2S_CLK_EN_b : 1;
      __IO uint32_t ULP_I2S_CLK_SEL_b : 4;
      __IO uint32_t ULP_I2S_CLKDIV_FACTOR : 8;
      __IO uint32_t ULP_I2S_MASTER_SLAVE_MODE_b : 1;
      __IO uint32_t ULP_I2S_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t ULP_I2S_LOOP_BACK_MODE_b : 1;
      __IO uint32_t ULP_I2S_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t ULP_I2S_PCLK_EN_b : 1;
      __IO uint32_t RESERVED2 : 13;
    } ULP_I2S_CLK_GEN_REG_b;
  };

  /*0x20*/
  union {
    __IO uint32_t ULP_UART_CLK_GEN_REG;
    struct {
      __IO uint32_t ULP_UART_FRAC_CLK_SEL_b : 1;
      __IO uint32_t ULP_UART_CLK_SEL : 4;
      __IO uint32_t ULP_UART_CLKDIV_FACTOR : 3;
      __IO uint32_t RESERVED : 24;
    } ULP_UART_CLK_GEN_REG_b;
  };

  /*0x24*/
  union {
    __IO uint32_t M4LP_CTRL_REG;
    struct {
      __IO uint32_t RESERVED0 : 2;
      __IO uint32_t ULP_M4_CORE_CLK_ENABLE_b : 1;
      __IO uint32_t ULP_MEM_CLK_ULP_ENABLE_b : 1;
      __IO uint32_t ULP_MEM_CLK_ULP_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t RESERVED1 : 27;

    } M4LP_CTRL_REG_b;
  };

  /*0x28*/
  union {
    __IO uint32_t CLOCK_STAUS_REG;
    struct {
      __O uint32_t CLOCK_SWITCHED_UART_CLK_b : 1;
      __O uint32_t CLOCK_SWITCHED_I2S_CLK_b : 1;
      __O uint32_t CLOCK_SWITCHED_CORTEX_SLEEP_CLK_b : 1;
      __O uint32_t CLOCK_SWITCHED_PROC_CLK_b : 1;
      __O uint32_t CLOCK_SWITCHED_I2C_b : 1;
      __O uint32_t CLOCK_SWITCHED_SSI_b : 1;
      __O uint32_t CLOCK_SWITCHED_VAD_b : 1;
      __O uint32_t CLOCK_SWITCHED_AUXADC_b : 1;
      __O uint32_t CLOCK_SWITCHED_TIMER_b : 1;
      __O uint32_t CLOCK_SWITCHED_TOUCH_SENSOR_b : 1;
      __O uint32_t CLOCK_SWITCHED_FCLK_VAD_b : 1;
      __O uint32_t CLOCK_SWITCHED_SCLK_VAD_b : 1;
      __O uint32_t CLOCK_SWITCHED_SYSTICK_b : 1;
      __O uint32_t RESERVED : 19;
    } CLOCK_STAUS_REG_b;
  };

  /*0x2C*/
  union {
    __IO uint32_t ULP_TOUCH_CLK_GEN_REG;
    struct {
      __IO uint32_t ULP_TOUCH_CLK_EN_b : 1;
      __IO uint32_t ULP_TOUCH_CLK_SEL : 4;
      __IO uint32_t ULP_TOUCH_CLKDIV_FACTOR : 8;
      __IO uint32_t RESERVED : 19;
    } ULP_TOUCH_CLK_GEN_REG_b;
  };

  /*0x30*/
  union {
    __IO uint32_t ULP_TIMER_CLK_GEN_REG;
    struct {
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t ULP_TIMER_CLK_SEL : 4;
      __IO uint32_t RESERVED2 : 8;
      __IO uint32_t ULP_TIMER_IN_SYNC_b : 1;
      __IO uint32_t RESERVED3 : 18;
    } ULP_TIMER_CLK_GEN_REG_b;
  };

  /*0x34*/
  union {
    __IO uint32_t ULP_AUXADC_CLK_GEN_REG;
    struct {
      __IO uint32_t ULP_AUX_CLK_EN_b : 1;
      __IO uint32_t ULP_AUX_CLK_SEL : 4;
      __IO uint32_t ULP_AUX_CLKDIV_FACTOR : 8;
      __IO uint32_t RESERVED1 : 19;
    } ULP_AUXADC_CLK_GEN_REG_b;
  };

  /*0x38*/
  union {
    __IO uint32_t ULP_VAD_CLK_GEN_REG;
    struct {
      __IO uint32_t ULP_VAD_CLK_EN_b : 1;
      __IO uint32_t ULP_VAD_CLK_SEL : 3;
      __IO uint32_t ULP_VAD_FCLK_EN : 1;
      __IO uint32_t ULP_VAD_FCLK_SEL : 4;
      __IO uint32_t ULP_VAD_CLKDIV_FACTOR : 8;
      __IO uint32_t RESERVED : 15;

    } ULP_VAD_CLK_GEN_REG_b;
  };

  /*0x3C*/
  union {
    __IO uint32_t BYPASS_I2S_CLK_REG;
    struct {
      __IO uint32_t BYPASS_I2S_PLL_SEL : 1;
      __IO uint32_t BYPASS_I2S_PLL_CLK_CLN_ON : 1;
      __IO uint32_t BYPASS_I2S_PLL_CLK_CLN_OFF : 1;
      __IO uint32_t RESERVED : 29;

    } BYPASS_I2S_CLK_REG_b;
  };

  /*0x40*/
  __IO uint32_t RESERVED2;

  /*0x44*/
  union {
    __IO uint32_t ULP_RM_RME_REG;
    struct {
      __IO uint32_t ULP_MEM_RME_b : 1;
      __IO uint32_t ULP_MEM_RM : 2;
      __IO uint32_t RESERVED1 : 1;
      __IO uint32_t ULP_MEM_RME_SRAM_b : 1;
      __IO uint32_t ULP_MEM_RM_SRAM : 2;
      __IO uint32_t RESERVED2 : 25;
    } ULP_RM_RME_REG_b;
  };

  /*0x48*/
  union {
    __IO uint32_t ULP_CLK_ENABLE_REG;
    struct {
      __IO uint32_t ULP_32KHZ_RO_CLK_EN_PROG_b : 1;
      __IO uint32_t ULP_32KHZ_RC_CLK_EN_PROG_b : 1;
      __IO uint32_t ULP_32KHZ_XTAL_CLK_EN_PROG_b : 1;
      __IO uint32_t ULP_DOUBLER_CLK_EN_PROG_b : 1;
      __IO uint32_t ULP_20MHZ_RO_CLK_EN_PROG_b : 1;
      __IO uint32_t ULP_MHZ_RC_CLK_EN_PROG_b : 1;
      __IO uint32_t SOC_CLK_EN_PROG_b : 1;
      __IO uint32_t I2S_PLLCLK_EN_PROG_b : 1;
      __IO uint32_t REF_CLK_EN_IPS_PROG_b : 1;
      __IO uint32_t RESERVED : 23;
    } ULP_CLK_ENABLE_REG_b;
  };

  /*0x4C*/
  __IO uint32_t RESERVED3;

  /*0x50*/
  union {
    __IO uint32_t SYSTICK_CLK_GEN_REG;
    struct {
      __IO uint32_t SYSTICK_CLK_EN_b : 1;
      __IO uint32_t SYSTICK_CLK_SEL : 4;
      __IO uint32_t SYSTICK_CLKDIV_FACTOR : 8;
      __IO uint32_t RESERVED : 19;
    } SYSTICK_CLK_GEN_REG_b;
  };
  /*0x54*/
  __IO uint32_t RESERVED4;
  /*0x58*/
  __IO uint32_t RESERVED5;

  /*0x5C*/
  __IO uint32_t RESERVED6;

  /*0x60 -  0x9C*/
  ULP_SOC_GPIO_MODE_t ULP_SOC_GPIO_MODE_REG[16];

  /*0xA0*/
  union {
    __IO uint32_t ULP_DYN_CLK_CTRL_DISABLE;
    struct {
      __IO uint32_t I2C_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t I2S_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t SSI_MST_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t SSI_MST_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t UART_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t UART_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t TIMER_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t TIMER_SCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t REG_ACCESS_SPI_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t FIM_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t VAD_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t AUX_PCLK_EN_b : 1;
      __IO uint32_t AUX_CLK_EN_b : 1;
      __IO uint32_t AUX_MEM_EN_b : 1;
      __IO uint32_t AUX_PCLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t AUX_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t AUX_CLK_MEM_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t UDMA_CLK_ENABLE_b : 1;
      __IO uint32_t IR_CLK_ENABLE_b : 1;
      __IO uint32_t IR_CLK_DYN_CTRL_DISABLE_b : 1;
      __IO uint32_t RESERVED : 12;
    } ULP_DYN_CLK_CTRL_DISABLE_b;
  };

  //*0xA4*/
  union {
    __IO uint32_t SLP_SENSOR_CLK_REG;
    struct {
      __IO uint32_t DIVISON_FACTOR : 8;
      __IO uint32_t ENABLE_b : 1;
      __IO uint32_t RESERVED : 23;
    } SLP_SENSOR_CLK_REG_b;
  };
} ULPCLK_Type;
/*END ULPSS CLOCKS*/

/*END OF POWER AND CLOCK REGISTERS */

/* =========================================================================================================================== */
/* ================                                            FIM                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief FIM support fixed point Multiplications implemented through programmable shifting. (FIM)
  */

typedef struct { /*!< (@ 0x02407000) FIM Structure                                              */

  union {
    __IOM uint32_t FIM_MODE_INTERRUPT; /*!< (@ 0x00000000) Configuration for FIM Operation Mode and Interrupt
                                                                    Control                                                    */

    struct {
      __IOM uint32_t LATCH_MODE : 1; /*!< [0..0] Enable latch mode                                                  */
      __IOM uint32_t OPER_MODE : 8;  /*!< [8..1] Indicates the Mode of Operation to be performed.                   */
      __IM uint32_t RESERVED1 : 1;   /*!< [9..9] reserved1                                                          */
      __OM uint32_t INTR_CLEAR : 1;  /*!< [10..10] Writing 1 to this bit clears the interrupt                       */
      __IM uint32_t RESERVED2 : 21;  /*!< [31..11] reserved2                                                        */
    } FIM_MODE_INTERRUPT_b;
  };

  union {
    __IOM uint32_t FIM_INP1_ADDR; /*!< (@ 0x00000004) This register used for COP input address for
                                                                    0 register.                                                */

    struct {
      __IOM uint32_t INP1_ADDR : 13; /*!< [12..0] Indicates the Start Address of 1st Input Data for FIM
                                                     Operations                                                                */
      __IM uint32_t RESERVED1 : 19;  /*!< [31..13] reserved1                                                        */
    } FIM_INP1_ADDR_b;
  };

  union {
    __IOM uint32_t FIM_INP2_ADDR; /*!< (@ 0x00000008) This register used for COP input address for
                                                                    1 register                                                 */

    struct {
      __IOM uint32_t INP2_ADDR : 13; /*!< [12..0] Indicates the Start Address of 2nd Input Data for FIM
                                                     Operations                                                                */
      __IM uint32_t RESERVED1 : 19;  /*!< [31..13] reserved1                                                        */
    } FIM_INP2_ADDR_b;
  };

  union {
    __IOM uint32_t FIM_OUT_ADDR; /*!< (@ 0x0000000C) Memory Offset Address for Output from FIM Operations       */

    struct {
      __IOM uint32_t OUT_ADDR : 13; /*!< [12..0] Indicates the Start Address of Output Data for FIM Operations     */
      __IM uint32_t RESERVED1 : 19; /*!< [31..13] reserved1                                                        */
    } FIM_OUT_ADDR_b;
  };

  union {
    __IOM uint32_t FIM_SCALAR_POLE_DATA1; /*!< (@ 0x00000010) Indicates the Input Scalar Data for Scalar Operations
                                                                    indicates the feedback coefficient for IIR
                                                                    Operations                                                 */

    struct {
      __IOM uint32_t
        SCALAR_POLE_DATA1 : 32; /*!< [31..0] Pole 0/Scalar Value                                               */
    } FIM_SCALAR_POLE_DATA1_b;
  };

  union {
    __IOM uint32_t FIM_POLE_DATA2; /*!< (@ 0x00000014) Feedback coefficient for IIR filter operation              */

    struct {
      __IOM uint32_t POLE_DATA2 : 32; /*!< [31..0] Indicates the feedback coefficient for IIR Operations             */
    } FIM_POLE_DATA2_b;
  };

  union {
    __IOM uint32_t FIM_SAT_SHIFT; /*!< (@ 0x00000018) Configuration for precision of Output Data for
                                                                    FIM Operations                                             */

    struct {
      __IOM uint32_t SAT_VAL : 5;   /*!< [4..0] Indicates the number of MSB's to be saturated for Output
                                                     Data                                                                      */
      __IM uint32_t RESERVED1 : 5;  /*!< [9..5] reserved1                                                          */
      __IOM uint32_t SHIFT_VAL : 6; /*!< [15..10] Indicates the number of bits to be right-shifted for
                                                     Output Data                                                               */
      __IM uint32_t : 2;
      __IM uint32_t RESERVED2 : 14; /*!< [31..18] reserved2                                                        */
    } FIM_SAT_SHIFT_b;
  };

  union {
    __IOM uint32_t FIM_CONFIG_REG1; /*!< (@ 0x0000001C) Configuration Register for FIM Operations.                 */

    struct {
      __IOM uint32_t MAT_LEN : 6;   /*!< [5..0] Indicates the number of columns in 1st input for Matrix
                                                     Multiplication. This is same as number of rows in 2nd input
                                                     for Matrix Multiplication.                                                */
      __IOM uint32_t INP1_LEN : 10; /*!< [15..6] Indicates the length of 1st input for FIM Operations
                                                     other than filtering (FIR, IIR) and Interpolation                         */
      __IOM uint32_t INP2_LEN : 10; /*!< [25..16] Indicates the length of 2nd input for FIM Operations
                                                     other than filtering (FIR, IIR) and Interpolation.                        */
      __IM uint32_t RESERVED1 : 6;  /*!< [31..26] reserved1                                                        */
    } FIM_CONFIG_REG1_b;
  };

  union {
    __IOM uint32_t FIM_CONFIG_REG2; /*!< (@ 0x00000020) Configuration Register for FIM Operations                  */

    struct {
      __OM uint32_t START_OPER : 1; /*!< [0..0] Start trigger for the FIM operations,this is reset upon
                                                     write register                                                            */
      __IM uint32_t RES : 7;        /*!< [7..1] reserved5                                                          */
      __IOM uint32_t CPLX_FLAG : 2; /*!< [9..8] Complex Flag,not valid in matrix mode                              */
      __IOM uint32_t COL_M2 : 6;    /*!< [15..10] Indicates the number of columns in 2nd input for Matrix
                                                     Multiplication                                                            */
      __IOM uint32_t ROW_M1 : 6;    /*!< [21..16] Indicates the number of rows in 1st input for Matrix
                                                     Multiplication                                                            */
      __IOM uint32_t INTRP_FAC : 6; /*!< [27..22] Indicates the Interpolation Factor                               */
      __IM uint32_t RESERVED1 : 4;  /*!< [31..28] reserved1                                                        */
    } FIM_CONFIG_REG2_b;
  };
} FIM_Type; /*!< Size = 36 (0x24)  */

typedef struct { /*!< NWP FSM one register Structure                                                      */

  union {
    __IO uint32_t TASS_REF_CLOCK_SELECT;
    struct {
      __IO uint32_t M4SS_REF_CLK_SEL_NWP : 3;
      __IO uint32_t RESERVED0 : 1;
      __IO uint32_t ULPSS_REF_CLK_SEL_NWP : 3;
      __IO uint32_t RESERVED1 : 9;
      __IO uint32_t TASS_REF_CLK_SEL_NWP : 3;
      __IO uint32_t RESERVED2 : 3;
      __IO uint32_t TASS_REF_CLK_CLEANER_OFF_NWP : 1;
      __IO uint32_t TASS_REF_CLK_CLEANER_ON_NWP : 1;
      __IO uint32_t RESERVED3 : 8;
    } TASS_REF_CLOCK_SELECT_b;
  };

} NWP_Fsm_t;

/* =========================================================================================================================== */
/* ================                                           USBHS                                           ================ */
/* =========================================================================================================================== */

/**
  * @brief The Universal Serial Bus (USB) is a cable bus that supports data exchange between a host computer and 
                     a wide range of simultaneously accessible peripherals (USBHS)
  */

typedef struct {               /*!< (@ 0x20210000) USBHS Structure                                            */
  __IM uint32_t USB_ID;        /*!< (@ 0x00000000) IDENTIFICATION_REGISTER                                    */
  __IM uint32_t USB_HWGENERAL; /*!< (@ 0x00000004) General Hardware Parameters Register                       */
  __IM uint32_t USB_HWHOST;    /*!< (@ 0x00000008) Host Hardware Parameters Register                          */
  __IM uint32_t USB_HWDEVICE;  /*!< (@ 0x0000000C) Device Hardware Parameters Register                        */
  __IM uint32_t USB_HWTXBUF;   /*!< (@ 0x00000010) TX Buffer Hardware Parameters Register                     */
  __IM uint32_t USB_HWRXBUF;   /*!< (@ 0x00000014) RX Buffer Hardware Parameters Register                     */
  __IM uint32_t RESERVED[26];
  __IOM uint32_t USB_GPTIMER0LD;   /*!< (@ 0x00000080) General Purpose Timer 0 Load Register                      */
  __IOM uint32_t USB_GPTIMER0CTRL; /*!< (@ 0x00000084) General Purpose Timer 0 Control Register                   */
  __IOM uint32_t USB_GPTIMER1LD;   /*!< (@ 0x00000088) general purpose timer 1 load register                      */
  __IOM uint32_t USB_GPTIMER1CTRL; /*!< (@ 0x0000008C) General Purpose Timer 1 Control Register                   */
  __IOM uint32_t USB_SBUSCFG;      /*!< (@ 0x00000090) Control for the System Bus interface Register              */
  __IM uint32_t RESERVED1[27];
  __IM uint32_t USB_CAPLENGTH; /*!< (@ 0x00000100) Capability Length Register                                 */
  __IM uint32_t USB_HCSPARAMS; /*!< (@ 0x00000104) Host Ctrl. Structural Parameters Register                  */
  __IM uint32_t USB_HCCPARAMS; /*!< (@ 0x00000108) Host Ctrl. Capability Parameters Register                  */
  __IM uint32_t RESERVED2[5];
  __IM uint32_t USB_DCIVERSION; /*!< (@ 0x00000120) Dev. Interface Version Number Register                     */
  __IM uint32_t USB_DCCPARAMS;  /*!< (@ 0x00000124) Device Ctrl. Capability Parameters Register                */
  __IM uint32_t RESERVED3[6];

  union {
    __IOM uint32_t USBCMD_H; /*!< (@ 0x00000140) USB Command(host mode) Register                            */
    __IOM uint32_t USBCMD_D; /*!< (@ 0x00000140) USB Command(device mode) Register                          */
  };

  union {
    __IOM uint32_t USBSTS_H; /*!< (@ 0x00000144) USB Status (host mode)Register                             */
    __IOM uint32_t USBSTS_D; /*!< (@ 0x00000144) USB Status(device mode) Register                           */
  };

  union {
    __IOM uint32_t USBINTR_H; /*!< (@ 0x00000148) USB Interrupt Enable(host mode))Register                   */
    __IOM uint32_t USBINTR_D; /*!< (@ 0x00000148) USB Interrupt Enable(device mode) Register                 */
  };

  union {
    __IOM uint32_t USB_FRINDEX_H; /*!< (@ 0x0000014C) USB Frame Index(host mode) Register                        */
    __IOM uint32_t USB_FRINDEX_D; /*!< (@ 0x0000014C) USB Frame Index (device mode)Register                      */
  };
  __IOM uint32_t USB_CTRLDSSEGMENT; /*!< (@ 0x00000150) Control Data Structure Segment Register                    */

  union {
    __IOM uint32_t
      USB_PERIODICLISTBASE;        /*!< (@ 0x00000154) Frame List Base Address Register                           */
    __IOM uint32_t USB_DEVICEADDR; /*!< (@ 0x00000154) USB Device Address Register                                */
  };

  union {
    __IOM uint32_t USB_ASYNCLISTADDR; /*!< (@ 0x00000158) Next Asynchronous List Address Register                    */
    __IOM uint32_t
      USB_ENDPOINTLISTADDR; /*!< (@ 0x00000158) Address at Endpoint list in memory Register                */
  };
  __IOM uint32_t USB_TTCTRL;       /*!< (@ 0x0000015C) TT Status and Control Register                             */
  __IOM uint32_t USB_BURSTSIZE;    /*!< (@ 0x00000160) Programmable Burst Size Register                           */
  __IOM uint32_t USB_TXFILLTUNING; /*!< (@ 0x00000164) Host Transmit Pre_Buffer Packet Tuning Register            */
  __IM uint32_t RESERVED4;
  __IOM uint32_t IC_USB;            /*!< (@ 0x0000016C) IC_USB enable and voltage negotiation Register             */
  __IOM uint32_t USB_ULPI_VIEWPORT; /*!< (@ 0x00000170) ULPI Viewport Registerr                                    */
  __IM uint32_t RESERVED5;
  __IOM uint32_t USB_ENDPTNAK;   /*!< (@ 0x00000178) Endpoint NAK Registerr                                     */
  __IOM uint32_t USB_ENDPTNAKEN; /*!< (@ 0x0000017C) Endpoint NAK Enable Register                               */
  __IOM uint32_t USB_CONFIGFLAG; /*!< (@ 0x00000180) Configured Flag Register                                   */

  union {
    __IOM uint32_t USB_PORTSC1_H; /*!< (@ 0x00000184) Multi Port Status/Control(host mode) Register              */
    __IOM uint32_t USB_PORTSC1_D; /*!< (@ 0x00000184) Multi Port Status/Control (device mode)Register            */
  };
  __IM uint32_t RESERVED6[8];
  __IOM uint32_t USBMODE_D;          /*!< (@ 0x000001A8) USB Device Mode(device) Register                           */
  __IOM uint32_t USB_ENDPTSETUPSTAT; /*!< (@ 0x000001AC) Endpoint Setup Status Register                             */
  __IOM uint32_t USB_ENDPTPRIME;     /*!< (@ 0x000001B0) Endpoint Initialization Register                           */
  __IOM uint32_t USB_ENDPTFLUSH;     /*!< (@ 0x000001B4) Endpoint De_Initialize Register                            */
  __IOM uint32_t USB_ENDPTSTAT;      /*!< (@ 0x000001B8) Endpoint Status Register                                   */
  __IOM uint32_t USB_ENDPTCOMPLETE;  /*!< (@ 0x000001BC) Endpoint Complete Register                                 */
  __IOM uint32_t USB_ENDPTCTRL0;     /*!< (@ 0x000001C0) Endpoint Control Address 0 Register                        */
  __IOM uint32_t USB_ENDPTCTRL1;     /*!< (@ 0x000001C4) Endpoint Control Address 1 Register                        */
  __IOM uint32_t USB_ENDPTCTRL2;     /*!< (@ 0x000001C8) Endpoint Control Address 2 Register                        */
  __IOM uint32_t USB_ENDPTCTRL3;     /*!< (@ 0x000001CC) Endpoint Control Address 3 Register                        */
  __IOM uint32_t USB_ENDPTCTRL4;     /*!< (@ 0x000001D0) Endpoint Control Address 4 Register                        */
  __IOM uint32_t USB_ENDPTCTRL5;     /*!< (@ 0x000001D4) Endpoint Control Address 5 Register                        */
} USBHS_Type;                        /*!< Size = 472 (0x1d8)                                                        */

/* =========================================================================================================================== */
/* ================                                           OPAMP                                           ================ */
/* =========================================================================================================================== */

/**
  * @brief The opamps top consists of 3 general purpose Operational Amplifiers (OPAMP) offering rail-to-rail inputs and outputs (OPAMP)
  */

typedef struct { /*!< (@ 0x24043A14) OPAMP Structure                                            */

  union {
    __IOM uint32_t OPAMP_1; /*!< (@ 0x00000000) Programs opamp1                                            */

    struct {
      __IOM uint32_t
        OPAMP1_ENABLE : 1; /*!< [0..0] To enable opamp 1                                                  */
      __IOM uint32_t
        OPAMP1_LP_MODE : 1; /*!< [1..1] Enable or disable low power mode                                   */
      __IOM uint32_t
        OPAMP1_R1_SEL : 2; /*!< [3..2] Programmability to select resister bank R1                         */
      __IOM uint32_t
        OPAMP1_R2_SEL : 3; /*!< [6..4] Programmability to select resister bank R2                         */
      __IOM uint32_t
        OPAMP1_EN_RES_BANK : 1; /*!< [7..7] enables the resistor bank 1 for enable 0 for disable               */
      __IOM uint32_t
        OPAMP1_RES_MUX_SEL : 3; /*!< [10..8] selecting input for registor bank                                 */
      __IOM uint32_t
        OPAMP1_RES_TO_OUT_VDD : 1; /*!< [11..11] connect resistor bank to out or vdd i.e 0-out and 1-vdd          */
      __IOM uint32_t
        OPAMP1_OUT_MUX_EN : 1; /*!< [12..12] out mux enable                                                   */
      __IOM uint32_t
        OPAMP1_INN_SEL : 3; /*!< [15..13] selecting -ve input of opamp                                     */
      __IOM uint32_t
        OPAMP1_INP_SEL : 4; /*!< [19..16] selecting +ve input of opamp                                     */
      __IOM uint32_t
        OPAMP1_OUT_MUX_SEL : 1; /*!< [20..20] to connect opamp1 output to pad                                  */
      __IOM uint32_t
        MEMS_RES_BANK_EN : 1;          /*!< [21..21] enables mems res bank                                            */
      __IOM uint32_t VREF_MUX_EN : 4;  /*!< [25..22] vref mux enable                                                  */
      __IOM uint32_t RESERVED1 : 1;    /*!< [26..26] res                                                              */
      __IOM uint32_t VREF_MUX_SEL : 4; /*!< [30..27] vref mux enable                                                  */
      __IOM uint32_t
        OPAMP1_DYN_EN : 1; /*!< [31..31] dynamic enable for opamp1                                        */
    } OPAMP_1_b;
  };

  union {
    __IOM uint32_t OPAMP_2; /*!< (@ 0x00000004) Programs opamp2                                            */

    struct {
      __IOM uint32_t
        OPAMP2_ENABLE : 1; /*!< [0..0] enables the opamp2                                                 */
      __IOM uint32_t
        OPAMP2_LP_MODE : 1; /*!< [1..1] select the power mode 0-normal mode and 1-low power mode           */
      __IOM uint32_t
        OPAMP2_R1_SEL : 2; /*!< [3..2] Programmability to select resister bank R1                         */
      __IOM uint32_t
        OPAMP2_R2_SEL : 3; /*!< [6..4] Programmability to select resister bank R2                         */
      __IOM uint32_t
        OPAMP2_EN_RES_BANK : 1; /*!< [7..7] enables the resistor bank 1 for enable 0 for disable               */
      __IOM uint32_t
        OPAMP2_RES_MUX_SEL : 3; /*!< [10..8] selecting input for registor bank                                 */
      __IOM uint32_t OPAMP2_RES_TO_OUT_VDD : 2; /*!< [12..11] connect resistor bank to out or vdd or gnd or DAC i.e
                                                     0-out and 1-vdd 2-DAC 3-gnd                                               */
      __IOM uint32_t
        OPAMP2_OUT_MUX_EN : 1; /*!< [13..13] out mux enable                                                   */
      __IOM uint32_t
        OPAMP2_INN_SEL : 2; /*!< [15..14] selecting -ve input of opamp                                     */
      __IOM uint32_t
        OPAMP2_INP_SEL : 3; /*!< [18..16] selecting +ve input of opamp2                                    */
      __IOM uint32_t
        OPAMP2_DYN_EN : 1;           /*!< [19..19] dynamic enable for opamp2                                        */
      __IOM uint32_t RESERVED1 : 12; /*!< [31..20] res                                                              */
    } OPAMP_2_b;
  };

  union {
    __IOM uint32_t OPAMP_3; /*!< (@ 0x00000008) Programs opamp3                                            */

    struct {
      __IOM uint32_t
        OPAMP3_ENABLE : 1; /*!< [0..0] enables the opamp3 1 for enable 0 for disable                      */
      __IOM uint32_t
        OPAMP3_LP_MODE : 1; /*!< [1..1] select the power mode 0-normal mode and 1-low power mode           */
      __IOM uint32_t
        OPAMP3_R1_SEL : 2; /*!< [3..2] Programmability to select resister bank R1                         */
      __IOM uint32_t
        OPAMP3_R2_SEL : 3; /*!< [6..4] Programmability to select resister bank R2                         */
      __IOM uint32_t
        OPAMP3_EN_RES_BANK : 1; /*!< [7..7] enables the resistor bank 1 for enable 0 for disable               */
      __IOM uint32_t
        OPAMP3_RES_MUX_SEL : 3; /*!< [10..8] selecting input for registor bank                                 */
      __IOM uint32_t
        OPAMP3_RES_TO_OUT_VDD : 1; /*!< [11..11] connect resistor bank to out or vdd i.e 0-out and 1-vdd          */
      __IOM uint32_t
        OPAMP3_OUT_MUX_EN : 1; /*!< [12..12] out mux enable                                                   */
      __IOM uint32_t
        OPAMP3_INN_SEL : 2; /*!< [14..13] selecting -ve input of opamp                                     */
      __IOM uint32_t
        OPAMP3_INP_SEL : 3; /*!< [17..15] selecting +ve input of opamp                                     */
      __IOM uint32_t
        OPAMP3_DYN_EN : 1;           /*!< [18..18] dynamic enable for opamp2                                        */
      __IOM uint32_t RESERVED1 : 13; /*!< [31..19] res                                                              */
    } OPAMP_3_b;
  };
} OPAMP_Type; /*!< Size = 12 (0xc)                */

/**
  * @brief AUX_ADC_DAC_COMP_ADC_CH_BIT_MAP_CONFIG [ADC_CH_BIT_MAP_CONFIG] ([0..15])
  */
typedef struct {
  union {
    __IOM uint32_t ADC_CH_BIT_MAP_CONFIG_0; /*!< (@ 0x00000000) This is configuration register0 to explain the
                                                                    bit map for ADC channels                                   */

    struct {
      __IOM uint32_t
        CHANNEL_BITMAP : 32; /*!< [31..0] This field explain the bit map for ADC channels                   */
    } ADC_CH_BIT_MAP_CONFIG_0_b;
  };

  union {
    __IOM uint32_t ADC_CH_BIT_MAP_CONFIG_1; /*!< (@ 0x00000004) This is configuration register1 to explain the
                                                                    bit map for ADC channels                                   */

    struct {
      __IOM uint32_t
        CHANNEL_BITMAP : 32; /*!< [31..0] This field explain the bit map for ADC channels                   */
    } ADC_CH_BIT_MAP_CONFIG_1_b;
  };

  union {
    __IOM uint32_t ADC_CH_BIT_MAP_CONFIG_2; /*!< (@ 0x00000008) This is configuration register2 to explain the
                                                                    bit map for ADC channels                                   */

    struct {
      __IOM uint32_t
        CHANNEL_BITMAP : 32; /*!< [31..0] This field explain the bit map for ADC channels                   */
    } ADC_CH_BIT_MAP_CONFIG_2_b;
  };

  union {
    __IOM uint32_t ADC_CH_BIT_MAP_CONFIG_3; /*!< (@ 0x0000000C) This is configuration register3 to explain the
                                                                    bit map for ADC channels                                   */

    struct {
      __IOM uint32_t
        CHANNEL_BITMAP : 32; /*!< [31..0] This field explain the bit map for ADC channels                   */
    } ADC_CH_BIT_MAP_CONFIG_3_b;
  };
} AUX_ADC_DAC_COMP_ADC_CH_BIT_MAP_CONFIG_Type; /*!< Size = 16 (0x10)      */

/* =========================================================================================================================== */
/* ================                                          ADC_DAC                                          ================ */
/* =========================================================================================================================== */

/**
  * @brief The ADC-DAC Controller works on a ADC with a resolution of 12bits at 10Maga sample per second  when ADC reference Voltage is greater than 2.8v or 5Maga sample per second when ADC reference Voltage is less than 2.8v. (ADC_DAC)
  */

typedef struct { /*!< (@ 0x24043800) AUX_ADC_DAC_COMP Structure                                 */

  union {
    __IOM uint32_t AUXDAC_CTRL_1; /*!< (@ 0x00000000) Control register1 for DAC                                  */

    struct {
      __IOM uint32_t ENDAC_FIFO_CONFIG : 1; /*!< [0..0] This bit activates the DAC path in Aux ADC-DAC controller.
                                                     Data samples will be played on  DAC only when this bit
                                                     is set.                                                                   */
      __IOM uint32_t
        DAC_STATIC_MODE : 1; /*!< [1..1] This bit is used to select non-FIFO mode in DAC.                   */
      __IOM uint32_t
        DAC_FIFO_FLUSH : 1; /*!< [2..2] This bit is used to flush the DAC FIFO.                            */
      __IOM uint32_t DAC_FIFO_THRESHOLD : 3; /*!< [5..3] These bits control the DAC FIFO threshold. When used
                                                     by DMA, this will act as almost full threshold. For NWP,
                                                     it acts as almost empty threshold                                         */
      __IOM uint32_t DAC_ENABLE_F : 1;       /*!< [6..6] This bit is used to enable AUX DAC controller ,valid
                                                     only when DAC enable is happpen                                           */
      __IOM uint32_t RESERVED1 : 2; /*!< [8..7] It is recommended to write these bits to 0                         */
      __IOM uint32_t DAC_TO_CTRL_ADC : 1; /*!< [9..9] When set, AUX-DAC control is handed over to Aux ADC-DAC
                                                     controller. By default,  AUX-DAC is under the control of
                                                     baseband.                                                                 */
      __IOM uint32_t RESERVED2 : 22; /*!< [31..10] Reserved2                                                        */
    } AUXDAC_CTRL_1_b;
  };

  union {
    __IOM uint32_t AUXADC_CTRL_1; /*!< (@ 0x00000004) Control register1 for ADC                                  */

    struct {
      __IOM uint32_t ADC_ENABLE : 1; /*!< [0..0] This bits activates the ADC path in Aux ADC-DAC controller.        */
      __IOM uint32_t
        ADC_STATIC_MODE : 1; /*!< [1..1] This bit is used to select non-FIFO mode in ADC.                   */
      __IOM uint32_t
        ADC_FIFO_FLUSH : 1; /*!< [2..2] This bit is used to flush the ADC FIFO                             */
      __IOM uint32_t ADC_FIFO_THRESHOLD : 3;       /*!< [5..3] These bits control the ADC FIFO threshold. When used
                                                     by DMA, this will act as almost empty threshold. For NWP,
                                                     it acts as almost full threshold.                                         */
      __IOM uint32_t ADC_MULTIPLE_CHAN_ACTIVE : 1; /*!< [6..6] This bit is used to control the auxadc sel signal going
                                                     to the Aux ADC.                                                           */
      __IOM uint32_t
        ADC_CH_SEL_MSB : 2; /*!< [8..7] It is recommended to write these bits to 0                         */
      __IOM uint32_t
        BYPASS_NOISE_AVG : 1;        /*!< [9..9] ADC in Bypass noise avg mode.                                      */
      __IOM uint32_t EN_ADC_CLK : 1; /*!< [10..10] Enable AUX ADC Divider output clock                              */
      __IOM uint32_t RESERVED1 : 1;  /*!< [11..11] Reserved1                                                        */
      __IOM uint32_t ADC_CH_SEL_LS : 2; /*!< [13..12] Aux ADC channel number from which the data has to be
                                                     sampled This is valid only when adc multiple channel active
                                                     is zero. When channel number is greater than three, upper
                                                     bits should also be programmed ADC CHANNEL SELECT MS to
                                                     bits in this register                                                     */
      __IOM uint32_t RESERVED2 : 13; /*!< [26..14] Reserved2                                                        */
      __IOM uint32_t
        ADC_NUM_PHASE : 1;          /*!< [27..27] ADC number of phase                                              */
      __IOM uint32_t RESERVED3 : 4; /*!< [31..28] Reserved3                                                        */
    } AUXADC_CTRL_1_b;
  };

  union {
    __IOM uint32_t AUXDAC_CLK_DIV_FAC; /*!< (@ 0x00000008) DAC clock division register                                */

    struct {
      __IOM uint32_t
        DAC_CLK_DIV_FAC : 10;        /*!< [9..0] These bits control the DAC clock division factor                   */
      __IOM uint32_t RESERVED1 : 22; /*!< [31..10] Reserved1                                                        */
    } AUXDAC_CLK_DIV_FAC_b;
  };

  union {
    __IOM uint32_t AUXADC_CLK_DIV_FAC; /*!< (@ 0x0000000C) ADC clock division register                                */

    struct {
      __IOM uint32_t
        ADC_CLK_DIV_FAC : 10;       /*!< [9..0] These bits control the Total-Duration of the ADC clock             */
      __IOM uint32_t RESERVED1 : 6; /*!< [15..10] Reserved1                                                        */
      __IOM uint32_t
        ADC_CLK_ON_DUR : 9;         /*!< [24..16] These bits control the On-Duration of the ADC clock              */
      __IOM uint32_t RESERVED2 : 7; /*!< [31..25] Reserved2                                                        */
    } AUXADC_CLK_DIV_FAC_b;
  };

  union {
    __IOM uint32_t AUXDAC_DATA_REG; /*!< (@ 0x00000010) Writing to this register will fill DAC FIFO for
                                                                    streaming Data to DAC                                      */

    struct {
      __OM uint32_t AUXDAC_DATA : 10; /*!< [9..0] Writing to this register will fill DAC FIFO for streaming
                                                     Data to DAC                                                               */
      __IOM uint32_t RESERVED1 : 22;  /*!< [31..10] Reserved1                                                        */
    } AUXDAC_DATA_REG_b;
  };

  union {
    __IOM uint32_t AUXADC_DATA; /*!< (@ 0x00000014) AUXADC Data Read through Register.                         */

    struct {
      __IM uint32_t AUXADC_DATA : 12; /*!< [11..0] AUXADC Data Read through Register                                 */
      __IM uint32_t AUXADC_CH_ID : 4; /*!< [15..12] Channel ID of AUX DATA sample, Valid only in FIFO mode           */
      __IOM uint32_t RESERVED1 : 16;  /*!< [31..16] Reserved1                                                        */
    } AUXADC_DATA_b;
  };

  union {
    __IOM uint32_t ADC_DET_THR_CTRL_0; /*!< (@ 0x00000018) ADC detection threshold control 0                          */

    struct {
      __IOM uint32_t
        ADC_INPUT_DETECTION_THRESHOLD_0 : 8;      /*!< [7..0] The value against which the ADC output has to be compared
                                                     is to be programmed  in this register                                     */
      __IOM uint32_t COMP_LESS_THAN_EN : 1;       /*!< [8..8] When set, Aux ADC-DAC controller raises an interrupt
                                                     to processor when the Aux ADC output falls below the  programmed
                                                     Aux ADC detection threshold.                                              */
      __IOM uint32_t COMP_GRTR_THAN_EN : 1;       /*!< [9..9] When set, Aux ADC-DAC controller raises an interrupt
                                                     to processor when the Aux ADC output is greater than the
                                                     programmed Aux ADC detection threshold..                                  */
      __IOM uint32_t COMP_EQ_EN : 1;              /*!< [10..10] When set, Aux ADC-DAC controller raises an interrupt
                                                     to processor when  the Aux ADC output is equal to the programmed
                                                     Aux ADC detection threshold                                               */
      __IOM uint32_t RANGE_COMPARISON_ENABLE : 1; /*!< [11..11] When set, Aux ADC-DAC controller raises an interrupt
                                                     to processor when the Aux ADC output falls within the range
                                                     specified in AUX ADC Detection threshold0  and AUX ADC
                                                     Detection threshold1                                                      */
      __IOM uint32_t
        ADC_INPUT_DETECTION_THRESHOLD_1 : 2; /*!< [13..12] Carries upper two bits of ADC detection threshold       */
      __IOM uint32_t RESERVED1 : 18; /*!< [31..14] Reserved1                                                        */
    } ADC_DET_THR_CTRL_0_b;
  };

  union {
    __IOM uint32_t ADC_DET_THR_CTRL_1; /*!< (@ 0x0000001C) ADC detection threshold control 1                          */

    struct {
      __IOM uint32_t
        ADC_INPUT_DETECTION_THRESHOLD_2 : 8; /*!< [7..0] The value against which the ADC output has to be compared
                                                     is to be programmed  in this register.                                    */
      __IOM uint32_t COMP_LESS_THAN_EN : 1;  /*!< [8..8] When set, Aux ADC-DAC controller raises an interrupt
                                                     to NWP when the Aux ADC output  falls below the programmed
                                                     Aux ADC detection threshold.                                              */
      __IOM uint32_t COMP_GRTR_THAN_EN : 1;  /*!< [9..9] When set, Aux ADC-DAC controller raises an interrupt
                                                     to NWP when the Aux ADC output is greater than the programmed
                                                     Aux ADC detection threshold.                                              */
      __IOM uint32_t COMP_EQ_EN : 1;         /*!< [10..10] When set, Aux ADC-DAC controller raises an interrupt
                                                     to NWP when the  Aux ADC output is equal to the programmed
                                                     Aux ADC detection threshold.                                              */
      __IOM uint32_t
        ADC_DETECTION_THRESHOLD_2_UPPER_BITS : 2; /*!< [12..11] Upper 2 bits of ADC detection threshold 2          */
      __IOM uint32_t RESERVED1 : 19; /*!< [31..13] Reserved1                                                        */
    } ADC_DET_THR_CTRL_1_b;
  };

  union {
    __IOM uint32_t INTR_CLEAR_REG; /*!< (@ 0x00000020) ADC detection threshold control 1                          */

    struct {
      __IOM uint32_t CLR_INTR : 1;  /*!< [0..0] This bit is used to clear threshold detection interrupt            */
      __IOM uint32_t RESERVED1 : 7; /*!< [7..1] Reserved1                                                          */
      __IOM uint32_t INTR_CLEAR_REG : 16; /*!< [23..8] If enabled, corresponding first_mem_switch_intr bits
                                                     will be cleared.                                                          */
      __IOM uint32_t RESERVED2 : 8; /*!< [31..24] Reserved2                                                        */
    } INTR_CLEAR_REG_b;
  };

  union {
    __IOM uint32_t INTR_MASK_REG; /*!< (@ 0x00000024) Mask interrupt register                                    */

    struct {
      __IOM uint32_t
        THRESHOLD_DETECTION_INTR_EN : 1; /*!< [0..0] When Cleared, threshold detection interrupt will be unmasked  */
      __IOM uint32_t
        DAC_FIFO_EMPTY_INTR_MASK : 1; /*!< [1..1] When Cleared, dac_FIFO_empty interrupt will be unmasked          */
      __IOM uint32_t
        DAC_FIFO_AEMPTY_INTR_MASK : 1; /*!< [2..2] When Cleared, adc FIFO full interrupt will be unmasked          */
      __IOM uint32_t
        ADC_FIFO_FULL_INTR_MASK : 1; /*!< [3..3] When Cleared, adc FIFO full interrupt will be unmasked            */
      __IOM uint32_t
        ADC_FIFO_AFULL_INTR_MASK : 1; /*!< [4..4] When Cleared, adc FIFO afull interrupt will be unmasked          */
      __IOM uint32_t
        ADC_FIFO_OVERFLOW_INTR_MASK : 1; /*!< [5..5] When Cleared, dac FIFO underrun interrupt will be unmasked    */
      __IOM uint32_t
        DAC_FIFO_UNDERRUN_INTR_MASK : 1; /*!< [6..6] When Cleared, dac FIFO underrun interrupt will be unmasked    */
      __IOM uint32_t
        FIRST_MEM_SWITCH_INTR_MASK : 16; /*!< [22..7] When Cleared, first_mem_switch_intr will be unmasked         */
      __IOM uint32_t RESERVED1 : 9; /*!< [31..23] Reserved1                                                        */
    } INTR_MASK_REG_b;
  };

  union {
    __IM uint32_t INTR_STATUS_REG; /*!< (@ 0x00000028) Status interrupt register                                  */

    struct {
      __IM uint32_t
        ADC_THRESHOLD_DETECTION_INTR : 1;  /*!< [0..0] This bit is set when ADC threshold matches with the programmed
                                                     conditions This will be be cleared as soon as this interrupt
                                                     is acknowledged by processor                                              */
      __IM uint32_t DAC_FIFO_EMPTY : 1;    /*!< [1..1] Set when DAC FIFO is empty. This bit gets cleared when
                                                     the DAC FIFO at least a single sample is available in DAC
                                                     FIFO                                                                      */
      __IM uint32_t DAC_FIFO_AEMPTY : 1;   /*!< [2..2] Set when the FIFO occupancy grater than or equal to DAC
                                                     FIFO threshold.                                                           */
      __IM uint32_t ADC_FIFO_FULL : 1;     /*!< [3..3] Set when ADC FIFO is full,This bit gets cleared when
                                                     data is read from the FIFO                                                */
      __IM uint32_t ADC_FIFO_AFULL : 1;    /*!< [4..4] Set when ADC FIFO occupancy less than or equal to ADC
                                                     FIFO threshold                                                            */
      __IM uint32_t ADC_FIFO_OVERFLOW : 1; /*!< [5..5] Set when a write attempt is made to ADC FIFO when the
                                                     FIFO is already full                                                      */
      __IM uint32_t DAC_FIFO_UNDERRUN : 1; /*!< [6..6] Set when a read is done on DAC FIFO when the FIFO is
                                                     empty                                                                     */
      __IM uint32_t FIRST_MEM_SWITCH_INTR : 16; /*!< [22..7] Interrupt indicating the first memory has been filled
                                                     and the DMA write is being shifted to second memory chunk
                                                     for ping-pong operation                                                   */
      __IM uint32_t RESERVED1 : 9; /*!< [31..23] Reserved1                                                        */
    } INTR_STATUS_REG_b;
  };

  union {
    __IM uint32_t
      INTR_MASKED_STATUS_REG; /*!< (@ 0x0000002C) Interrupt masked status register                           */

    struct {
      __IM uint32_t
        ADC_THRESHOLD_DETECTION_INTR_MASKED : 1; /*!< [0..0] Masked Interrupt. This bit is set when ADC threshold
                                                     matches with  the programmed conditions                                   */
      __IM uint32_t
        DAC_FIFO_EMPTY_MASKED : 1; /*!< [1..1] Masked Interrupt.Set when DAC FIFO is empty                        */
      __IM uint32_t DAC_FIFO_AEMPTY_MASKED : 1; /*!< [2..2] Masked Interrupt. Set when the FIFO occupancy less than
                                                     equal to DAC FIFO threshold.                                              */
      __IM uint32_t
        ADC_FIFO_FULL_MASKED : 1; /*!< [3..3] Masked Interrupt. Set when ADC FIFO is full.                       */
      __IM uint32_t ADC_FIFO_AFULL_MASKED : 1;         /*!< [4..4] Masked Interrupt. Set when ADC FIFO occupancy greater
                                                     than ADC FIFO threshold                                                   */
      __IM uint32_t ADC_FIFO_OVERFLOW_MASKED : 1;      /*!< [5..5] Masked Interrupt. Set when a write attempt is made to
                                                     ADC FIFO when the FIFO is already full.                                   */
      __IM uint32_t DAC_FIFO_UNDERRUN_MASKED : 1;      /*!< [6..6] Masked Interrupt. Set when a read is done on DAC FIFO
                                                     when the FIFO is empty.                                                   */
      __IM uint32_t FIRST_MEM_SWITCH_INTR_MASKED : 16; /*!< [22..7] Masked Interrupt status indicating the first memory
                                                     has been filled and the DMA write is being shifted  to
                                                     second memory chunk for ping-pong operation                               */
      __IM uint32_t RESERVED1 : 9; /*!< [31..23] Reserved1                                                        */
    } INTR_MASKED_STATUS_REG_b;
  };

  union {
    __IM uint32_t FIFO_STATUS_REG; /*!< (@ 0x00000030) Interrupt masked status register                           */

    struct {
      __IM uint32_t DAC_FIFO_FULL : 1; /*!< [0..0] Set when DAC FIFO is full. In word mode, FIFO will be
                                                     shown as full unless there is space for 16-bits.                          */
      __IM uint32_t
        DAC_FIFO_AFULL : 1; /*!< [1..1] Set when DAC FIFO occupancy greater than FIFO threshold            */
      __IM uint32_t ADC_FIFO_EMPTY : 1; /*!< [2..2] Set when FIFO is empty. This bit gets cleared when the
                                                     ADC FIFO is not empty.                                                    */
      __IM uint32_t
        ADC_FIFO_AEMPTY : 1; /*!< [3..3] Set when the FIFO occupancy less than ADC FIFO threshold           */
      __IM uint32_t DAC_FIFO_EMPTY : 1; /*!< [4..4] Set when FIFO is empty. This bit gets cleared when the
                                                     DAC FIFO is not empty.                                                    */
      __IM uint32_t
        DAC_FIFO_AEMPTY : 1;           /*!< [5..5] Set when the FIFO occupancy less than DAC FIFO threshold           */
      __IM uint32_t ADC_FIFO_FULL : 1; /*!< [6..6] Set when ADC FIFO is full. This bit gets cleared when
                                                     data is read from the FIFO.                                               */
      __IM uint32_t
        ADC_FIFO_AFULL : 1;         /*!< [7..7] Set when ADC FIFO occupancy greater than ADC FIFO threshold.       */
      __IM uint32_t RESERVED1 : 24; /*!< [31..8] Reserved1                                                         */
    } FIFO_STATUS_REG_b;
  };
  __IM uint32_t RESERVED;
  __IOM AUX_ADC_DAC_COMP_ADC_CH_BIT_MAP_CONFIG_Type
    ADC_CH_BIT_MAP_CONFIG[16]; /*!< (@ 0x00000038) [0..15]                      */

  union {
    __IOM uint32_t ADC_CH_OFFSET[16]; /*!< (@ 0x00000138) This Register specifies initial offset value
                                                                    with respect  to AUX_ADC clock after which
                                                                    Channel(0-16)should be sampled.                            */

    struct {
      __IOM uint32_t CH_OFFSET : 16; /*!< [2..0] This Register field specifies initial offset value with
                                                     respect  to AUX_ADC clock after which Channel(0-16)should
                                                     be sampled.                                                               */
      __IOM uint32_t RESERVED1 : 16; /*!< [31..3] Reserved1                                                         */
    } ADC_CH_OFFSET_b[16];
  };

  union {
    __IOM uint32_t ADC_CH_FREQ[16]; /*!< (@ 0x00000178) This register specifies Sampling frequency rate
                                                                    at which AUX ADC Date is sampled for  Channel(1
                                                                    to 16 )                                                    */

    struct {
      __IOM uint32_t CH_FREQ_VALUE : 16; /*!< [15..0] This register specifies Sampling frequency rate at which
                                                     AUX ADC Date is sampled for Channel all respective channel
                                                     (1-16)                                                                    */
      __IOM uint32_t RESERVED1 : 16; /*!< [31..16] Reserved1                                                        */
    } ADC_CH_FREQ_b[16];
  };
  __IM uint32_t RESERVED1[3];

  union {
    __IOM uint32_t AUXADC_CONFIG_1; /*!< (@ 0x000001C4) This register explain configuration parameter
                                                                    for AUXADC                                                 */

    struct {
      __IOM uint32_t RESERVED1 : 17; /*!< [16..0] Reserved1                                                         */
      __IOM uint32_t
        AUXADC_INP_SEL : 5; /*!< [21..17] Mux select for positive input of adc                             */
      __IOM uint32_t
        AUXADC_INN_SEL : 4; /*!< [25..22] Mux select for negetive input of adc                             */
      __IOM uint32_t
        AUXADC_DIFF_MODE : 1;       /*!< [26..26] AUX ADC Differential Mode select                                 */
      __IOM uint32_t RESERVED2 : 5; /*!< [31..27] Reserved2                                                        */
    } AUXADC_CONFIG_1_b;
  };
  __IM uint32_t RESERVED2;

  union {
    __IOM uint32_t ADC_SEQ_CTRL; /*!< (@ 0x000001CC) This register explain configuration parameter
                                                                    for AUXADC                                                 */

    struct {
      __IOM uint32_t
        ADC_SEQ_CTRL_PING_PONG : 16; /*!< [15..0] To enable/disable per channel DAM mode (One-hot coding)          */
      __IOM uint32_t
        ADC_SEQ_CTRL_DMA_MODE : 16; /*!< [31..16] To enable/disable per channel ping-pong operation (One-hot
                                                     coding).                                                                  */
    } ADC_SEQ_CTRL_b;
  };

  union {
    __IOM uint32_t VAD_BBP_ID; /*!< (@ 0x000001D0) This register explain VDD BBP ID                           */

    struct {
      __IOM uint32_t BPP_ID : 4; /*!< [3..0] Channel id for bbp samples.                                        */
      __IOM uint32_t BPP_EN : 1; /*!< [4..4] Enables Aux-ADC samples to BBP                                     */
      __IOM uint32_t
        AUX_ADC_BPP_EN : 1;         /*!< [5..5] Enable Indication for BBP                                          */
      __IOM uint32_t RESERVED1 : 2; /*!< [7..6] Reserved1                                                          */
      __IOM uint32_t VAD_ID : 4;    /*!< [11..8] Enable VAD identification                                         */
      __IOM uint32_t VAD_EN : 1;    /*!< [12..12] Enable VAD                                                       */
      __IOM uint32_t RESERVED2 : 3; /*!< [15..13] RESERVED2                                                        */
      __IOM uint32_t DISCONNET_MODE : 16; /*!< [31..16] Per channel discontinuous mode enable signal. When
                                                     discontinuous mode is enabled, data is sampled only once
                                                     from that channel and the enable bit is reset to 0.                       */
    } VAD_BBP_ID_b;
  };

  union {
    __IOM uint32_t ADC_INT_MEM_1; /*!< (@ 0x000001D4) This register explain start address of first/second
                                                                    buffer corresponding to the channel location
                                                                    ADC INT MEM 2                                              */

    struct {
      __IOM uint32_t PROG_WR_DATA : 32; /*!< [31..0] These 32-bits specifies the start address of first/second
                                                     buffer corresponding to the channel  location ADC INT MEM                 */
    } ADC_INT_MEM_1_b;
  };

  union {
    __IOM uint32_t ADC_INT_MEM_2; /*!< (@ 0x000001D8) This register explain ADC INT MEM2.                        */

    struct {
      __IOM uint32_t PROG_WR_DATA : 10; /*!< [9..0] These 10-bits specify the buffer length of first/second
                                                     buffer corresponding to the channel location ADC INT MEM2                 */
      __IOM uint32_t PROG_WR_ADDR : 5;  /*!< [14..10] These bits correspond to the address of the internal
                                                     memory basing on the channel number,  whose information
                                                     we want to program                                                        */
      __IOM uint32_t PROG_WR_DATA1 : 1; /*!< [15..15] Valid bit for first/second buffers corresponding to
                                                     ADC INT MEM2                                                              */
      __IOM uint32_t RESERVED3 : 16; /*!< [31..16] Reserved3                                                        */
    } ADC_INT_MEM_2_b;
  };

  union {
    __IOM uint32_t
      INTERNAL_DMA_CH_ENABLE; /*!< (@ 0x000001DC) This register is internal channel enable                   */

    struct {
      __IOM uint32_t PER_CHANNEL_ENABLE : 16; /*!< [15..0] Enable bit for Each channel,like channel0 for bit0 to
                                                     channel15 for bit15 etc                                                   */
      __IOM uint32_t RESERVED3 : 15; /*!< [30..16] Reserved3                                                        */
      __IOM uint32_t INTERNAL_DMA_ENABLE : 1; /*!< [31..31] When Set, Internal DMA will be used for reading ADC
                                                     samples from ADC FIFO and  writing them to ULP SRAM Memories.             */
    } INTERNAL_DMA_CH_ENABLE_b;
  };

  union {
    __IOM uint32_t TS_PTAT_ENABLE; /*!< (@ 0x000001E0) This register is enable PTAT for temperature
                                                                    sensor                                                     */

    struct {
      __IOM uint32_t TS_PTAT_EN : 1; /*!< [0..0] BJT based Temperature sensor                                       */
      __IOM uint32_t RESERVED1 : 31; /*!< [31..1] Reserved1                                                         */
    } TS_PTAT_ENABLE_b;
  };
  __IM uint32_t RESERVED3[7];

  union {
    __IOM uint32_t BOD; /*!< (@ 0x00000200) Programs resistor bank, reference buffer and
                                                                    scaler                                                     */

    struct {
      __IOM uint32_t RESERVED1 : 3; /*!< [2..0] Reserved1                                                          */
      __IOM uint32_t REFBUF_EN : 1; /*!< [3..3] Reference buffer configuration 1 for enable 0 for disable          */
      __IOM uint32_t
        REFBUF_VOLT_SEL : 4;         /*!< [7..4] selection of voltage of reference buffer                           */
      __IOM uint32_t BOD_RES_EN : 1; /*!< [8..8] configuration of register bank 1 for enable and 0 for
                                                     disable                                                                   */
      __IOM uint32_t BOD_THRSH : 5;  /*!< [13..9] Programmability for resistor bank                                 */
      __IOM uint32_t RESERVED2 : 18; /*!< [31..14] Reserved2                                                        */
    } BOD_b;
  };

  union {
    __IOM uint32_t COMPARATOR1; /*!< (@ 0x00000204) Programs comparators1 and comparators2                     */

    struct {
      __IOM uint32_t CMP1_EN : 1; /*!< [0..0] To enable comparator1                                              */
      __IOM uint32_t
        CMP1_EN_FILTER : 1;         /*!< [1..1] To enable filter for comparator 1                                  */
      __IOM uint32_t CMP1_HYST : 2; /*!< [3..2] Programmability to control hysteresis of comparator1               */
      __IOM uint32_t
        CMP1_MUX_SEL_P : 4; /*!< [7..4] Select for positive input of comparator_1                          */
      __IOM uint32_t
        CMP1_MUX_SEL_N : 4;       /*!< [11..8] Select for negative input of comparator_1                         */
      __IOM uint32_t CMP2_EN : 1; /*!< [12..12] To enable comparator 2                                           */
      __IOM uint32_t
        CMP2_EN_FILTER : 1;         /*!< [13..13] To enable filter for comparator 2                                */
      __IOM uint32_t CMP2_HYST : 2; /*!< [15..14] Programmability to control hysteresis of comparator2             */
      __IOM uint32_t
        CMP2_MUX_SEL_P : 4; /*!< [19..16] Select for positive input of comparator_2                        */
      __IOM uint32_t
        CMP2_MUX_SEL_N : 4;         /*!< [23..20] Select for negative input of comparator_2                        */
      __IOM uint32_t RESERVED1 : 8; /*!< [31..24] Reserved1                                                        */
    } COMPARATOR1_b;
  };

  union {
    __IOM uint32_t AUXADC_CONFIG_2; /*!< (@ 0x00000208) This register is AUX-ADC config2                           */

    struct {
      __IOM uint32_t RESERVED1 : 11; /*!< [10..0] Reserved1                                                         */
      __IOM uint32_t
        AUXADC_CONFIG_ENABLE : 1;    /*!< [11..11] Aux ADC Configuration Enable                                     */
      __IOM uint32_t RESERVED2 : 20; /*!< [31..12] Reserved2                                                        */
    } AUXADC_CONFIG_2_b;
  };

  union {
    __IOM uint32_t AUXDAC_CONIG_1; /*!< (@ 0x0000020C) This register is AUX-DAC config1                           */

    struct {
      __IOM uint32_t AUXDAC_EN_S : 1; /*!< [0..0] Enable signal DAC                                                  */
      __IOM uint32_t
        AUXDAC_OUT_MUX_EN : 1; /*!< [1..1] Aux OUT mux Enable                                                 */
      __IOM uint32_t
        AUXDAC_OUT_MUX_SEL : 1;     /*!< [2..2] AUXDAC OUT MUX SELECT Enable                                       */
      __IOM uint32_t RESERVED1 : 1; /*!< [3..3] Reserved1                                                          */
      __IOM uint32_t
        AUXDAC_DATA_S : 10; /*!< [13..4] Satatic AUX Dac Data                                              */
      __IOM uint32_t
        AUXDAC_DYN_EN : 1;           /*!< [14..14] Satatic AUX Dac Data                                             */
      __IOM uint32_t RESERVED2 : 17; /*!< [31..15] RESERVED2                                                        */
    } AUXDAC_CONIG_1_b;
  };
  union {
    __IOM uint32_t AUX_LDO; /*!< (@ 0x00000210) This register is AUX-LDO configuration                     */

    struct {
      __IOM uint32_t LDO_CTRL : 4; /*!< [3..0] Enable ldo control field                                           */
      __IOM uint32_t
        LDO_DEFAULT_MODE : 1;        /*!< [4..4] ldo default mode enable                                            */
      __IOM uint32_t BYPASS_LDO : 1; /*!< [5..5] bypass the LDO                                                     */
      __IOM uint32_t ENABLE_LDO : 1; /*!< [6..6] Turn LDO                                                           */
      __IOM uint32_t DYN_EN : 1;     /*!< [7..7] Dynamic Enable                                                     */
      __IOM uint32_t RESERVED1 : 24; /*!< [31..8] It is recommended to write these bits to 0.                       */
    } AUX_LDO_b;
  };
} AUX_ADC_DAC_COMP_Type; /*!< Size = 528 (0x210)                                                        */

/* =========================================================================================================================== */
/* ================                                            IR                                             ================ */
/* =========================================================================================================================== */

/**
  * @brief IR Decoder are used for the decoding the external ir sensor input. (IR)
  */

typedef struct { /*!< (@ 0x24040C00) IR Structure                                               */

  union {
    __IOM uint32_t IR_OFF_TIME_DURATION; /*!< (@ 0x00000000) This register used for IR sleep duration timer
                                                                    value.                                                     */

    struct {
      __IOM uint32_t
        IR_OFF_TIME_DURATION : 17; /*!< [16..0] This field define ir off time                                     */
      __IM uint32_t RES : 15;      /*!< [31..17] reserved5                                                        */
    } IR_OFF_TIME_DURATION_b;
  };

  union {
    __IOM uint32_t IR_ON_TIME_DURATION; /*!< (@ 0x00000004) This register used for IR Detection duration
                                                                    timer value.                                               */

    struct {
      __IOM uint32_t
        IR_ON_TIME_DURATION : 12; /*!< [11..0] This field define ir on time for ir detection on                  */
      __IM uint32_t RES : 20;     /*!< [31..12] reserved5                                                        */
    } IR_ON_TIME_DURATION_b;
  };

  union {
    __IOM uint32_t IR_FRAME_DONE_THRESHOLD; /*!< (@ 0x00000008) This register used count with respect to 32KHz
                                                                    clock after not more toggle are expected
                                                                    to a given pattern.                                        */

    struct {
      __IOM uint32_t IR_FRAME_DONE_THRESHOLD : 15; /*!< [14..0] count with respect to 32KHz clock after not more toggle
                                                     are expected to a given pattern                                           */
      __IM uint32_t RES : 17; /*!< [31..15] reserved5                                                        */
    } IR_FRAME_DONE_THRESHOLD_b;
  };

  union {
    __IOM uint32_t IR_DET_THRESHOLD; /*!< (@ 0x0000000C) This register used Minimum Number of edges to
                                                                    detected during on-time failing which IR
                                                                    detection is re-stated.                                    */

    struct {
      __IOM uint32_t IR_DET_THRESHOLD : 7; /*!< [6..0] Minimum Number of edges to detected during on-time failing
                                                     which IR detection is re-stated.                                          */
      __IM uint32_t RES : 25; /*!< [31..7] reserved5                                                         */
    } IR_DET_THRESHOLD_b;
  };

  union {
    __IOM uint32_t IR_CONFIG; /*!< (@ 0x00000010) This register used to configure the ir structure
                                                                    for application purpose.                                   */

    struct {
      __IOM uint32_t EN_IR_DET : 1;      /*!< [0..0] Enable IR detection logic bit if bit 1 then detection
                                                     enable if 0 then not enable.                                              */
      __IOM uint32_t IR_DET_RSTART : 1;  /*!< [1..1] Enable IR detection re-start logic bit if bit 1 then
                                                     re-start.                                                                 */
      __IOM uint32_t EN_CLK_IR_CORE : 1; /*!< [2..2] Enable 32KHz clock to IR Core bit ,if bit 1 then clock
                                                     gating disable and bit is 0 then clock gating Enable                      */
      __IM uint32_t RES : 5; /*!< [7..3] reserved5                                                          */
      __IOM uint32_t EN_CONT_IR_DET : 1; /*!< [8..8] This bit is Enable continues IR detection,When enabled
                                                     there will be no power cycling on External IR Sensor.                     */
      __IM uint32_t RES1 : 7; /*!< [15..9] reserved6                                                         */
      __IOM uint32_t
        SREST_IR_CORE : 1;     /*!< [16..16] This bit is used soft reset IR core block                        */
      __IM uint32_t RES2 : 15; /*!< [31..17] reserved7                                                        */
    } IR_CONFIG_b;
  };

  union {
    __IOM uint32_t IR_MEM_ADDR_ACCESS; /*!< (@ 0x00000014) This register used to access memory address for
                                                                    application purpose.                                       */

    struct {
      __IOM uint32_t IR_MEM_ADDR : 7;  /*!< [6..0] This field is used to IR read address.                             */
      __IM uint32_t RES : 2;           /*!< [8..7] reserved5                                                          */
      __IOM uint32_t IR_MEM_RD_EN : 1; /*!< [9..9] This field used to IR memory read enable.                          */
      __IM uint32_t RES1 : 22;         /*!< [31..10] reserved6                                                        */
    } IR_MEM_ADDR_ACCESS_b;
  };

  union {
    __IM uint32_t IR_MEM_READ; /*!< (@ 0x00000018) This register used to IR Read data from memory.            */

    struct {
      __IM uint32_t
        IR_MEM_DATA_OUT : 16; /*!< [15..0] This field is used to IR Read data from memory.                   */
      __IM uint32_t RES : 8;  /*!< [23..16] reserved5                                                        */
      __IM uint32_t IR_DATA_MEM_DEPTH : 7; /*!< [30..24] This field used to indicated valid number of IR Address
                                                     in the memory to be read.                                                 */
      __IM uint32_t RES1 : 1; /*!< [31..31] reserved6                                                        */
    } IR_MEM_READ_b;
  };
} IR_Type; /*!< Size = 28 (0x1c)                                                          */

/**
 * @brief Ethernet MAC
 */
typedef struct {                       /*!< (@ 0x20240000) ETH Structure                                              */
  __IOM uint32_t MAC_CONFIG_REG;       /*!< (@ 0x00000000) ETHERNET REG                                               */
  __IOM uint32_t MAC_FRAME_FILTER_REG; /*!< (@ 0x00000004) ETHERNET REG                                               */
  __IOM uint32_t
    MAC_HASH_TABLE_HIGH_REG; /*!< (@ 0x00000008) ETHERNET REG                                               */
  __IOM uint32_t
    MAC_HASH_TABLE_LOW_REG;         /*!< (@ 0x0000000C) ETHERNET REG                                               */
  __IOM uint32_t MAC_GMII_ADDR_REG; /*!< (@ 0x00000010) ETHERNET REG                                               */
  __IOM uint32_t MAC_GMII_DATA_REG; /*!< (@ 0x00000014) ETHERNET REG                                               */
  __IOM uint32_t MAC_FLOW_CTRL_REG; /*!< (@ 0x00000018) ETHERNET REG                                               */
  __IOM uint32_t MAC_VLAN_TAG_REG;  /*!< (@ 0x0000001C) ETHERNET REG                                               */
  __IOM uint32_t MAC_VER_REG;       /*!< (@ 0x00000020) ETHERNET REG                                               */
  __IM uint32_t RESERVED;
  __IOM uint32_t
    MAC_REM_WAKEUP_FRAME_FILTER_REG; /*!< (@ 0x00000028) ETHERNET REG                                             */
  __IOM uint32_t
    MAC_PMT_CTRL_STATUS_REG; /*!< (@ 0x0000002C) ETHERNET REG                                               */
  __IM uint32_t RESERVED1[2];
  __IOM uint32_t MAC_STATUS_REG;     /*!< (@ 0x00000038) ETHERNET REG                                               */
  __IOM uint32_t MAC_INTR_MASK_REG;  /*!< (@ 0x0000003C) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR0_HIGH_REG; /*!< (@ 0x00000040) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR0_LOW_REG;  /*!< (@ 0x00000044) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR1_HIGH_REG; /*!< (@ 0x00000048) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR1_LOW_REG;  /*!< (@ 0x0000004C) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR2_HIGH_REG; /*!< (@ 0x00000050) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR2_LOW_REG;  /*!< (@ 0x00000054) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR3_HIGH_REG; /*!< (@ 0x00000058) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR3_LOW_REG;  /*!< (@ 0x0000005C) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR4_HIGH_REG; /*!< (@ 0x00000060) ETHERNET REG                                               */
  __IOM uint32_t MAC_ADDR4_LOW_REG;  /*!< (@ 0x00000064) ETHERNET REG                                               */
  __IM uint32_t RESERVED2[38];
  __IOM uint32_t MMC_CTRL_REG;         /*!< (@ 0x00000100) ETHERNET REG                                               */
  __IOM uint32_t MMC_INTR_RX_REG;      /*!< (@ 0x00000104) ETHERNET REG                                               */
  __IOM uint32_t MMC_INTR_TX_REG;      /*!< (@ 0x00000108) ETHERNET REG                                               */
  __IOM uint32_t MMC_INTR_MASK_RX_REG; /*!< (@ 0x0000010C) ETHERNET REG                                               */
  __IOM uint32_t MMC_INTR_MASK_TX_REG; /*!< (@ 0x00000110) ETHERNET REG                                               */
  __IM uint32_t RESERVED3[13];
  __IOM uint32_t
    MMC_TX_UNDERFLOW_ERROR_REG; /*!< (@ 0x00000148) ETHERNET REG                                               */
  __IOM uint32_t
    MMC_TX_SINGLE_COL_REG; /*!< (@ 0x0000014C) ETHERNET REG                                               */
  __IM uint32_t RESERVED4;
  __IOM uint32_t MMC_TX_DEFFERED_REG; /*!< (@ 0x00000154) ETHERNET REG                                               */
  __IM uint32_t RESERVED5[4];
  __IOM uint32_t
    MMC_TX_FRAME_COUNT_G_REG; /*!< (@ 0x00000168) ETHERNET REG                                               */
  __IM uint32_t RESERVED6[5];
  __IOM uint32_t
    MMC_TX_FRAME_COUNT_GB_REG; /*!< (@ 0x00000180) ETHERNET REG                                               */
  __IM uint32_t RESERVED7[4];
  __IOM uint32_t MMC_RX_CRC_ERROR_REG; /*!< (@ 0x00000194) ETHERNET REG                                               */
  __IM uint32_t RESERVED8[15];
  __IOM uint32_t
    MMC_RX_FIFO_OVER_FLOW_REG; /*!< (@ 0x000001D4) ETHERNET REG                                               */
  __IM uint32_t RESERVED9[906];
  __IOM uint32_t DMA_BUS_MODE_REG; /*!< (@ 0x00001000) Ethernet register                                          */
  __IOM uint32_t
    DMA_TX_POLL_DEMAND_REG; /*!< (@ 0x00001004) Ethernet register                                          */
  __IOM uint32_t
    DMA_RX_POLL_DEMAND_REG; /*!< (@ 0x00001008) Ethernet register                                          */
  __IOM uint32_t
    DMA_RX_DESC_LIST_ADDR_REG; /*!< (@ 0x0000100C) Ethernet register                                          */
  __IOM uint32_t
    DMA_TX_DESC_LIST_ADDR_REG;      /*!< (@ 0x00001010) Ethernet register                                          */
  __IOM uint32_t DMA_STATUS_REG;    /*!< (@ 0x00001014) Ethernet register                                          */
  __IOM uint32_t DMA_OPER_MODE_REG; /*!< (@ 0x00001018) Ethernet register                                          */
  __IOM uint32_t DMA_INTR_EN_REG;   /*!< (@ 0x0000101C) Ethernet register                                          */
  __IOM uint32_t
    DMA_MISSED_FRAME_BOC_REG; /*!< (@ 0x00001020) Ethernet register                                          */
  __IM uint32_t RESERVED10[9];
  __IOM uint32_t
    DMA_CURRENT_HOST_TX_DESC_REG; /*!< (@ 0x00001048) Ethernet register                                          */
  __IOM uint32_t
    DMA_CURRENT_HOST_RX_DESC_REG; /*!< (@ 0x0000104C) Ethernet register                                          */
  __IOM uint32_t
    DMA_CURRENT_HOST_TX_BUFF_ADDR_REG; /*!< (@ 0x00001050) Ethernet register                                      */
  __IOM uint32_t
    DMA_CURRENT_HOST_RX_BUFF_ADDR_REG; /*!< (@ 0x00001054) Ethernet register                                      */
} ETH_Type;                            /*!< Size = 4184 (0x1058)                                                      */

/* =========================================================================================================================== */
/* ================                                            VAD                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief VAD (Voice-Activity-Detection) is an hardware accelerator to detect voice activity
                on the samples provided by the Processor (VAD)
  */

typedef struct { /*!< (@ 0x24043400) VAD Structure                                              */

  union {
    __IOM uint32_t VAD_CONF_REG1; /*!< (@ 0x00000000) Configuration Registers1 for VAD                           */

    struct {
      __IOM uint32_t
        SAMPLS_PER_FRAME : 10; /*!< [9..0] Indicates the number of samples in one processing frame            */
      __IOM uint32_t
        THRESHOLD_MAG : 10; /*!< [19..10] Indicates the magnitude threshold for the AMDF algorithm         */
      __IOM uint32_t
        SMPLS_PER_ADDR : 2; /*!< [21..20] Indicates the bit width of the samples                           */
      __IOM uint32_t
        NORMALIZE_FRAME : 1; /*!< [22..22] Normalize frame                                                  */
      __IOM uint32_t
        ENABLE_DC_REMOVAL : 1;       /*!< [23..23] enable dc removal                                                */
      __IOM uint32_t FULL_WIDTH : 1; /*!< [24..24] This bit is define number of samples                             */
      __IOM uint32_t RESERVED2 : 7;  /*!< [31..25] Reserved2                                                        */
    } VAD_CONF_REG1_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG2; /*!< (@ 0x00000004) Configuration Registers2 for VAD                           */

    struct {
      __IOM uint32_t SMPLS_ZERO_CROSS : 10; /*!< [9..0] Indicates the threshold for number of zero crossings
                                                     to ensure detection  using ZCR algorithm                                  */
      __IOM uint32_t RESERVED1 : 10; /*!< [19..10] Reserved1                                                        */
      __IOM uint32_t RESERVED2 : 12; /*!< [31..20] Reserved2                                                        */
    } VAD_CONF_REG2_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG3; /*!< (@ 0x00000008) Configuration Registers3 for VAD                           */

    struct {
      __IOM uint32_t THRESHOLD_FRAME_ENERGY : 10; /*!< [9..0] Indicates the threshold for energy of the samples over
                                                     the entire processing frame to start the algorithm execution.             */
      __IOM uint32_t
        THRESHOLD_SMPL_COLLECT : 10; /*!< [19..10] Indicates the threshold for energy of the samples               */
      __IOM uint32_t PROG_SMPLS_FOR_ENERGY_CHECK : 2; /*!< [21..20] Indicates the number of samples for validating the
                                                     energy of the samples  before further processing                          */
      __IOM uint32_t MAX_DELAY : 10; /*!< [31..22] Max delay used in ACF, WACF and AMDF algorithms                  */
    } VAD_CONF_REG3_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG4; /*!< (@ 0x0000000C) Configuration Registers4 for VAD                           */

    struct {
      __IOM uint32_t
        THRESHOLD_ACF : 12; /*!< [11..0] Energy threshold for ACF algorithm                                */
      __IOM uint32_t
        THRESHOLD_WACF : 12;        /*!< [23..12] Energy threshold for WACF algorithm                              */
      __IOM uint32_t RESERVED1 : 8; /*!< [31..24] It is recommended to write these bits to 0                       */
    } VAD_CONF_REG4_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG5; /*!< (@ 0x00000010) Configuration Registers5 for VAD                           */

    struct {
      __IOM uint32_t THRESHOLD_NULL : 12;       /*!< [11..0] Indicates the threshold for determining a Null for AMDF
                                                     algorithm to  detect Voice activity.                                      */
      __IOM uint32_t THRESHOLD_NULL_COUNT : 10; /*!< [21..12] Indicates the threshold for number of Null's for AMDF
                                                     algorithm to  detect Voice activity.                                      */
      __IOM uint32_t
        AMDF_BY_SQARING : 1;        /*!< [22..22] Enable or disable sqaring function for ADMF                      */
      __IOM uint32_t RESERVED1 : 9; /*!< [31..23] It is recommended to write these bits to 0                       */
    } VAD_CONF_REG5_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG6; /*!< (@ 0x00000014) Configuration Registers6 for VAD                           */

    struct {
      __IOM uint32_t THRESHOLD_PEAK : 12;       /*!< [11..0] Indicates the threshold for determining a Peak for AMDF
                                                     algorithm  to detect Voice activity                                       */
      __IOM uint32_t THRESHOLD_PEAK_COUNT : 10; /*!< [21..12] Indicates the threshold for number of Peak's for AMDF
                                                     algorithm to detect Voice activity.                                       */
      __IOM uint32_t RESERVED1 : 10; /*!< [31..22] It is recommended to write these bits to 0                       */
    } VAD_CONF_REG6_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG7; /*!< (@ 0x00000018) Configuration Registers7 for VAD                           */

    struct {
      __IOM uint32_t
        START_DELAY_VAL : 10; /*!< [9..0] Start delay used in ACF, WACF and AMDF algorithms                  */
      __IOM uint32_t
        END_DELAY_VAL : 10; /*!< [19..10] End delay used in ACF, WACF and AMDF algorithms                  */
      __IOM uint32_t CHOOSE_VAD_METHOD : 3; /*!< [22..20] Indicates the combination of Algorithms to be used
                                                     for Voice Activity Detection                                              */
      __IOM uint32_t
        DATA_SOURCE_SELECT : 2;     /*!< [24..23] Data source input select VAD peripheral                          */
      __IOM uint32_t RESERVED1 : 7; /*!< [31..25] It is recommended to write these bits to 0                       */
    } VAD_CONF_REG7_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG8; /*!< (@ 0x0000001C) Configuration Registers8 for VAD                           */

    struct {
      __IOM uint32_t INP_DATA : 10; /*!< [9..0] 10 bit Samples Data used for VAD                                   */
      __IOM uint32_t
        EN_VAD_PROCESS : 1;            /*!< [10..10] Enable vad process                                               */
      __IOM uint32_t RESERVED1 : 11;   /*!< [21..11] It is recommended to write these bits to 0                       */
      __IM uint32_t VAD_PROC_DONE : 1; /*!< [22..22] Indicates that the VAD processing is done                        */
      __IOM uint32_t RESERVED2 : 9;    /*!< [31..23] It is recommended to write these bits to 0                       */
    } VAD_CONF_REG8_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG9; /*!< (@ 0x00000020) Configuration Registers9 for VAD                           */

    struct {
      __IOM uint32_t RESERVED1 : 1;  /*!< [0..0] It is recommended to write these bits to 0                         */
      __IOM uint32_t PING_ADDR : 13; /*!< [13..1] Indicates the Start Address of Ping Memory containing
                                                     the VAD samples                                                           */
      __IOM uint32_t PONG_ADDR : 13; /*!< [26..14] Indicates the Start Address of Pong Memory containing
                                                     the VAD samples                                                           */
      __OM uint32_t
        PING_INT_CLEAR : 1; /*!< [27..27] clears the VAD interrupt on Ping Buffer samples                  */
      __OM uint32_t
        PONG_INT_CLEAR : 1;         /*!< [28..28] clears the VAD interrupt on Pong Buffer samples                  */
      __IOM uint32_t RESERVED2 : 3; /*!< [31..29] It is recommended to write these bits to 0                       */
    } VAD_CONF_REG9_b;
  };

  union {
    __IOM uint32_t VAD_CONF_REG10; /*!< (@ 0x00000024) Configuration Registers10 for VAD                          */

    struct {
      __IOM uint32_t
        PROG_DC_REMOVAL : 12; /*!< [11..0] 12 bit DC correction for ADC source                               */
      __IM uint32_t : 17;
      __IOM uint32_t RESERVED2 : 3; /*!< [31..29] It is recommended to write these bits to 0                       */
    } VAD_CONF_REG10_b;
  };
} VAD_Type; /*!< Size = 40 (0x28)                                                          */

/* =========================================================================================================================== */
/* ================                                           SDIOH                                           ================  */
/* =========================================================================================================================== */

/**
 * @brief The SD or SDIO or MMC Host Controller (SDIOH) is compatible with Standard SD Host 3.0 and eMMC5.0 Specification.
                 The SDIOH supports three key interfaces (SD memory, SDIO and eMMC) (SDIOH) */

typedef struct { /*!< (@ 0x20220000) SDIOH Structure                                            */

  union {
    __IOM uint32_t
      SMIH_SDMA_SYSTEM_ADDRESS_REGISTER; /*!< (@ 0x00000000) SDMA System Address Register                          */

    struct {
      __IOM uint32_t
        SDMA_SYSTEM_ADDRESS_OR_ARGUMENT_2 : 32; /*!< [31..0] This register contains the physical system memory address
                                                     used for DMA transfers or the second argument for the Auto
                                                     CMD23.                                                                    */
    } SMIH_SDMA_SYSTEM_ADDRESS_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_BLOCK_SIZE_REGISTER; /*!< (@ 0x00000004) Block Size Register                                        */

    struct {
      __IOM uint16_t TRANSFER_BLOCK_SIZE : 12; /*!< [11..0] This register specifies the block size of data transfers
                                                     for CMD17, CMD18, CMD24, CMD25, and CMD53. Values ranging
                                                     from 1 up to the maximum buffer size                                      */
      __IOM uint16_t
        HOST_SDMA_BUFFER_BOUNDARY : 3; /*!< [14..12] Host Driver to update the SDMA System Address register.
                                                     At the end of transfer, the SMIHC Controller may issue
                                                     or may not issue DMA Interrupt.                                           */
      __IOM uint16_t RESERVED1 : 1;    /*!< [15..15] Reserved1                                                        */
    } SMIH_BLOCK_SIZE_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_BLOCK_COUNT_REGISTER; /*!< (@ 0x00000006) Block Count Register                                       */

    struct {
      __IOM uint16_t
        BLOCK_COUNT_FOR_CURRENT_TRANSFER : 16; /*!< [15..0] This register is enabled when Block Count Enable in
                                                     the Transfer Mode  register is set to 1 and is valid only
                                                     for multiple block transfers.                                             */
    } SMIH_BLOCK_COUNT_REGISTER_b;
  };

  union {
    __IOM uint32_t
      SMIH_ARGUMENT1_REGISTER; /*!< (@ 0x00000008) argument 1 Register                                        */

    struct {
      __IOM uint32_t COMMAND_ARGUMENT_1 : 32; /*!< [31..0] The SD command argument is specified as bit 39-8 of
                                                     Command-Format in the Physical Layer Specification.                       */
    } SMIH_ARGUMENT1_REGISTER_b;
  };

  union {
    __IOM uint16_t
      TRANSFER_MODE_REGISTER; /*!< (@ 0x0000000C) Transfer Mode Register                                     */

    struct {
      __IOM uint16_t DMA_ENABLE : 1;         /*!< [0..0] This bit enables DMA functionality. DMA can be enabled
                                                     only  if it is supported as indicated in the Capabilities
                                                     register.                                                                 */
      __IOM uint16_t BLOCK_COUNT_ENABLE : 1; /*!< [1..1] This bit is used to enable the Block Count register,
                                                     which is only relevant for multiple block transfers. When
                                                     this bit is 0, the Block Count register is disabled,                      */
      __IOM uint16_t
        AUTO_CMD_ENABLE : 2; /*!< [3..2] This field determines use of auto command functions.               */
      __IOM uint16_t
        DATA_TRANSFER_DIRECTION_SELECT : 1; /*!< [4..4] This bit defines the direction of DAT line data transfers.
                                                     The bit is set to 1 by the Host Driver to transfer data
                                                     from the SD card  to the SMIHC Controller and                             */
      __IOM uint16_t MULTI_OR_SINGLE_BLOCK_SELECT : 1; /*!< [5..5] This bit is set when issuing multiple-block transfer
                                                     commands using DAT line.                                                  */
      __IM uint16_t RESERVED1 : 4;  /*!< [9..6] RESERVED1                                                          */
      __IOM uint16_t MMC_CMD23 : 1; /*!< [10..10] The Host driver has to set this bit for MMC CMD23                */
      __IOM uint16_t STREAM_MODE_ENABLE : 1; /*!< [11..11] The Host driver has to set this bit for MMC CMD11 or
                                                     CMD20 Stream read/Write Operations.                                       */
      __IOM uint16_t
        SPI_MODE_ENABLE : 1; /*!< [12..12] Enable or disable SPI mode                                       */
      __IOM uint16_t
        BOOT_ACK_ENABLE : 1; /*!< [13..13] Enable or disable boot ack                                       */
      __IOM uint16_t ALTERNATE_BOOT_OPERATION : 1; /*!< [14..14] Host Driver should set this bit only for Alternate
                                                     Boot Operation. For Normal Transaction, this bit should
                                                     always be zero.                                                           */
      __IOM uint16_t BOOT_OPERATION : 1; /*!< [15..15] Host Driver should set this bit only for Boot Operation.
                                                     For Normal Transaction, this bit should always be zero.                   */
    } TRANSFER_MODE_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_COMMAND_REGISTER; /*!< (@ 0x0000000E) Command Register                                           */

    struct {
      __IOM uint16_t
        RESPONSE_TYPE_SELECT : 2;   /*!< [1..0] This register explain the response type select.                    */
      __IOM uint16_t RESERVED1 : 1; /*!< [2..2] RESERVED1                                                          */
      __IOM uint16_t COMMAND_CRC_CHECK_ENABLE : 1; /*!< [3..3] If this bit is set to 1, the SMIHC Controller shall check
                                                     the CRC field in the response.  If an error is detected,
                                                     it is reported as a Command CRC RSI_SMIH_ERROR.                                    */
      __IOM uint16_t
        COMMAND_INDEX_CHECK_ENABLE : 1;       /*!< [4..4] If this bit is set to 1, the SMIHC Controller shall check
                                                     the Index field in the response to see if it has  the same
                                                     value as the command index.                                               */
      __IOM uint16_t DATA_PRESENT_SELECT : 1; /*!< [5..5] This bit is set to 1 to indicate that data is present
                                                     and shall  be transferred using the DAT line.                             */
      __IOM uint16_t COMMAND_TYPE : 2;        /*!< [7..6] There are three types of special commands like Suspend,
                                                     Resume and Abort. These bits shall be set to 00b for all
                                                     other commands.                                                           */
      __IOM uint16_t COMMAND_INDEX : 6;       /*!< [13..8] These bits shall be set to the command number (CMD0-63,
                                                     ACMD0-63) that is specified in bits 45-40 of the Command-Format
                                                     in the Physical Layer Specification                                       */
      __IOM uint16_t RESERVED2 : 2; /*!< [15..14] RESERVED2                                                        */
    } SMIH_COMMAND_REGISTER_b;
  };

  __IM uint16_t SMIH_RESPONSE_REGISTER0;
  __IM uint16_t SMIH_RESPONSE_REGISTER1;
  __IM uint16_t SMIH_RESPONSE_REGISTER2;
  __IM uint16_t SMIH_RESPONSE_REGISTER3;
  __IM uint16_t SMIH_RESPONSE_REGISTER4;
  __IM uint16_t SMIH_RESPONSE_REGISTER5;
  __IM uint16_t SMIH_RESPONSE_REGISTER6;
  __IM uint16_t SMIH_RESPONSE_REGISTER7;

  __IOM uint32_t SMIH_BUFFER_DATA_PORT_REGISTER;

  union {
    __IOM uint32_t
      SMIH_PRESENT_STATE_REGISTER; /*!< (@ 0x00000024) Present State Register                                     */

    struct {
      __IM uint32_t COMMAND_INHIBIT_CMD : 1; /*!< [0..0] If this bit is 0, it indicates the CMD line is not in
                                                     use and the SMIHC, Controller can issue a SD Command using
                                                     the CMD line. This bit is set immediately after the Command
                                                     register (00Fh) is written.                                               */
      __IM uint32_t COMMAND_INHIBIT_DAT : 1; /*!< [1..1] This status bit is generated if either the DAT Line Active
                                                     or the read Transfer Active is set to 1., If this bit is
                                                     0, it indicates the SMIHC Controller can issue the next
                                                     SD Command Commands with busy signal belong to Command
                                                     Inhibit (DAT) (ex. R1b, R5b type).                                        */
      __IM uint32_t DAT_LINE_ACTIVE : 1;     /*!< [2..2] This bit indicates whether one of the DAT line on SD
                                                     Bus is in use.                                                            */
      __IM uint32_t
        RE_TUNING_REQUEST : 1;     /*!< [3..3] Re-Tuning Request.                                                 */
      __IM uint32_t Reserved1 : 4; /*!< [7..4] Reserved1                                                          */
      __IM uint32_t WRITE_TRANSFER_ACTIVE : 1; /*!< [8..8] This status indicates a write transfer is active. If
                                                     this bit is 0,  it means no valid write data exists in
                                                     the SMIHC Controller..                                                    */
      __IM uint32_t READ_TRANSFER_ACTIVE : 1;  /*!< [9..9] This status is used for detecting completion of a read
                                                     transfer.                                                                 */
      __IM uint32_t BUFFER_WRITE_ENABLE : 1;   /*!< [10..10] This status is used for non-DMA write transfers.The
                                                     SMIHC Controller can implement multiple buffers to transfer
                                                     data efficiently. This read only flag indicates if space
                                                     is available for write data.                                              */
      __IM uint32_t BUFFER_READ_ENABLE : 1;    /*!< [11..11] This status is used for non-DMA read transfers. The
                                                     SMIHC Controller may implement multiple buffers to transfer
                                                     data efficiently.  This read only flag indicates that valid
                                                     data exists in the host side.                                             */
      __IOM uint32_t Reserved2 : 4;    /*!< [15..12] Reserved2                                                        */
      __IM uint32_t CARD_INSERTED : 1; /*!< [16..16] This bit indicates whether a card has been inserted.
                                                     The SMIHC Controller shall debounce this signal so that
                                                     the Host Driver  will not need to wait for it to stabilize.               */
      __IM uint32_t CARD_STATE_STABLE : 1; /*!< [17..17] This bit is used for testing. If it is 0, the Card
                                                     Detect Pin Level is not stable. If this bit is set to 1,
                                                     it means the Card Detect Pin Level is stable.                             */
      __IM uint32_t
        CARD_DETECT_PIN_LEVEL : 1; /*!< [18..18] This bit reflects the inverse value of the SDCD pin.Debouncing
                                                     is not performed on this bit. This bit may be valid when
                                                     Card State Stable is set to 1, but it is not guaranteed
                                                     because of propagation delay.                                             */
      __IM uint32_t : 1;
      __IM uint32_t DAT_LINE_SIGNAL_LEVEL : 4; /*!< [23..20] This status is used to check the DAT line level to
                                                     recover from errors and for debugging.                                    */
      __IM uint32_t CMD_LINE_SIGNAL_LEVEL : 1; /*!< [24..24] This status is used to check the CMD line level to
                                                     recover from errors and for debugging.                                    */
      __IOM uint32_t Reserved3 : 7; /*!< [31..25] Reserved3                                                        */
    } SMIH_PRESENT_STATE_REGISTER_b;
  };

  union {
    __IOM uint8_t
      SMIH_HOST_CONTROL_1_REGISTER; /*!< (@ 0x00000028) Present State Register                                     */

    struct {
      __IOM uint8_t LED_CONTROL : 1;         /*!< [0..0] This bit is used to caution the user not to remove the
                                                     card while the SD card is being accessed.If the software
                                                     is going to issue multiple SD  commands, this bit can be
                                                     set during all these transactions.                                        */
      __IOM uint8_t DATA_TRANSFER_WIDTH : 1; /*!< [1..1] This bit selects the data width of the SMIHC Controller.The
                                                     Host Driver shall set it to match the data width of the
                                                     SD card.                                                                  */
      __IOM uint8_t HIGH_SPEED_ENABLE : 1;   /*!< [2..2] This bit is optional. Before setting this bit, the Host
                                                     Driver  shall check the High Speed Support in the Capabilities
                                                     register.                                                                 */
      __IOM uint8_t DMA_SELECT : 2;          /*!< [4..3] One of supported DMA modes can be selected.  The host
                                                     driver shall check support of DMA modes by referring                      */
      __IOM uint8_t
        EXTENDED_DATA_TRANSFER_WIDTH : 1;       /*!< [5..5] This bit controls 8-bit bus width mode for embedded device.
                                                     Support of this function is indicated in 8-bit Support
                                                     for Embedded Device in the Capabilities register.                         */
      __IOM uint8_t CARD_DETECT_TEST_LEVEL : 1; /*!< [6..6] This bit is enabled while the Card Detect Signal Selection
                                                     is set to 1 and it indicates  card inserted or not.                       */
      __IOM uint8_t
        CARD_DETECT_SIGNAL_SELECTION : 1; /*!< [7..7] This bit selects source for the card detection.               */
    } SMIH_HOST_CONTROL_1_REGISTER_b;
  };

  union {
    __IOM uint8_t
      SMIH_POWER_CONTROL_REGISTER; /*!< (@ 0x00000029) Power Control Register                                     */

    struct {
      __IOM uint8_t SD_BUS_POWER : 1;          /*!< [0..0] Before setting this bit, the SD Host Driver shall set
                                                     SD Bus Voltage Select.  If the SMIHC Controller detects
                                                     the No Card state, this bit shall be cleared.                             */
      __IOM uint8_t SD_BUS_VOLTAGE_SELECT : 3; /*!< [3..1] By setting these bits, the Host Driver selects the voltage
                                                     level for the SD card.  Before setting this register, the
                                                     Host Driver shall check the Voltage.                                      */
      __IOM uint8_t SMIH_OD_PP : 1; /*!< [4..4] This bit is used only in MMC mode.                                 */
      __IOM uint8_t SMIH_RST_N : 1; /*!< [5..5] External Hardware reset, used only in MMC mode                     */
      __IOM uint8_t Reserved1 : 2;  /*!< [7..6] Reserved1                                                          */
    } SMIH_POWER_CONTROL_REGISTER_b;
  };

  union {
    __IOM uint8_t
      SMIH_BLOCK_GAP_CONTROL_REGISTER; /*!< (@ 0x0000002A) Power Control Register                                   */

    struct {
      __IOM uint8_t
        STOP_AT_BLOCK_GAP_REQUEST : 1;     /*!< [0..0] This bit is used to stop executing read and write transaction
                                                     at the next block gap for non-DMA, SDMA and ADMA transfers.
                                                     The Host Driver                                                           */
      __IOM uint8_t CONTINUE_REQUEST : 1;  /*!< [1..1] This bit is used to restart a transaction which was stopped
                                                     using  Stop At Block Gap request. To cancel stop at block
                                                     gap Stop At Block Gap to Zero And set this bit to restart
                                                     the transfer.                                                             */
      __IOM uint8_t READ_WAIT_CONTROL : 1; /*!< [2..2] The read wait function is optional for SDIO cards. If
                                                     the card supports read wait, set this bit to enable use
                                                     of the read wait protocol to stop read data using the DAT[2]
                                                     line.                                                                     */
      __IOM uint8_t INTERRUPT_AT_BLOCK_GAP : 1; /*!< [3..3] This bit is valid only in 4-bit mode of the SDIO  card
                                                     and selects a sample point in the interrupt cycle. Setting
                                                     to 1 enables interrupt detection at the block gap for a
                                                     multiple block transfer.                                                  */
      __IOM uint8_t Reserved1 : 4; /*!< [7..4] Reserved1                                                          */
    } SMIH_BLOCK_GAP_CONTROL_REGISTER_b;
  };
  union {
    __IOM uint8_t SMIH_WAKEUP_CONTROL_REGISTER;
    struct {
      __IOM uint8_t WAKEUP_EVENT_ENABLE_ON_CARD_INTERRUPT : 1;
      __IOM uint8_t WAKEUP_EVENT_ENABLE_ON_SDCARD_INSERTION : 1;
      __IOM uint8_t WAKEUP_EVENT_ENABLE_ON_SDCARD_REMOVAL : 1;
      __IOM uint8_t Reserved1 : 5;

    } SMIH_WAKEUP_CONTROL_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_CLOCK_CONTROL_REGISTER; /*!< (@ 0x0000002C) Clock Control Register                                     */

    struct {
      __IOM uint16_t INTERNAL_CLOCK_ENABLE : 1; /*!< [0..0] This bit is set to 0 when the Host Driver is not using
                                                     the  SMIHC Controller or the SMIHC Controller awaits a
                                                     wakeup interrupt.                                                         */
      __IOM uint16_t INTERNAL_CLOCK_STABLE : 1; /*!< [1..1] This bit is set to 1 when SD Clock is stable after  writing
                                                     to Internal Clock Enable in this register to 1.                           */
      __IOM uint16_t SD_CLOCK_ENABLE : 1;       /*!< [2..2] The SMIHC Controller shall stop SDCLK when writing this
                                                     bit to 0.                                                                 */
      __IOM uint16_t Reserved1 : 3; /*!< [5..3] Reserved1                                                          */
      __IOM uint16_t
        UPPER_BITS_OF_SDCLK_FREQUENCY_SELECT : 2; /*!< [7..6] PE_SMIH Controller shall support these bits to expand
                                                     SDCLK Frequency Select to 10-bit                                          */
      __IOM uint16_t SDCLK_FREQUENCY_SELECT : 8;  /*!< [15..8] This register is used to select the frequency of SDCLK
                                                     pin.                                                                      */
    } SMIH_CLOCK_CONTROL_REGISTER_b;
  };

  union {
    __IOM uint8_t
      SMIH_TIMEOUT_CONTROL_REGISTER; /*!< (@ 0x0000002E) Clock Control Register                                     */

    struct {
      __IOM uint8_t DATA_TIMEOUT_COUNTER_VALUE : 4; /*!< [3..0] This value determines the interval by which DAT line
                                                     timeouts are detected.                                                    */
      __IOM uint8_t Reserved1 : 4; /*!< [7..4] Reserved1                                                          */
    } SMIH_TIMEOUT_CONTROL_REGISTER_b;
  };

  union {
    __IOM uint8_t
      SMIH_SOFTWARE_RESET_REGISTER; /*!< (@ 0x0000002F) Software Reset Register                                    */

    struct {
      __IOM uint8_t SOFTWARE_RESET_FOR_ALL : 1; /*!< [0..0] This reset affects the entire SMIHC Controller except
                                                     for the card detection circuit.  Register bits of type
                                                     ROC, RW, RW1C, RWAC are cleared to 0.                                     */
      __IOM uint8_t
        SOFTWARE_RESET_FOR_CMD_LINE : 1; /*!< [1..1] Only part of command circuit is reset.                         */
      __IOM uint8_t SOFTWARE_RESET_FOR_DAT_LINE : 1; /*!< [2..2] Only part of data circuit is reset,DMA circuit is also
                                                     reset.                                                                    */
      __IOM uint8_t Reserved1 : 5; /*!< [7..3] Reserved1                                                          */
    } SMIH_SOFTWARE_RESET_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_NORMAL_INTERRUPT_STATUS_REGISTER; /*!< (@ 0x00000030) Normal Interrupt Status Register                  */

    struct {
      __IOM uint16_t COMMAND_COMPLETE : 1;   /*!< [0..0] This bit is set when get the end bit of the command response.
                                                     Auto CMD12 and Auto CMD23 consist of two responses.                       */
      __IOM uint16_t TRANSFER_COMPLETE : 1;  /*!< [1..1] This bit is set when a read / write transfer and a command
                                                     with busy is completed.                                                   */
      __IOM uint16_t BLOCK_GAP_EVENT : 1;    /*!< [2..2] If the Stop At Block Gap Request in the Block Gap Control
                                                     register is set, this bit is set when both a read or write
                                                     transaction is stopped at a block gap.                                    */
      __IOM uint16_t SMIH_DMA_INTERRUPT : 1; /*!< [3..3] This status is set if the SMIHC Controller detects the
                                                     Host SDMA Buffer boundary during transfer.                                */
      __IOM uint16_t BUFFER_WRITE_READY : 1; /*!< [4..4] This status is set if the Buffer Write Enable changes
                                                     from 0 to 1. Refer to the Buffer Write Enable in the Present
                                                     State register.                                                           */
      __IOM uint16_t BUFFER_READ_READY : 1;  /*!< [5..5] This status is set if the Buffer read Enable changes
                                                     from 0 to 1.  Refer to the Buffer read Enable in the Present
                                                     State register                                                            */
      __IOM uint16_t CARD_INSERTION : 1;     /*!< [6..6] This status is set if the Card Inserted in the Present
                                                     State register changes from 0 to 1.                                       */
      __IOM uint16_t CARD_REMOVAL : 1;       /*!< [7..7] This status is set if the Card Inserted in the Present
                                                     State register changes from 1 to 0                                        */
      __IM uint16_t CARD_INTERRUPT : 1;      /*!< [8..8] interrupt signal from the SD card and the interrupt to
                                                     the Host System.                                                          */
      __IM uint16_t INT_A : 1;               /*!< [9..9] This status is set if INT_A is enabled and INT_A pin
                                                     is in low level.                                                          */
      __IM uint16_t INT_B : 1;               /*!< [10..10] This status is set if INT_B is enabled and INT_B pin
                                                     is in low level.                                                          */
      __IM uint16_t INT_C : 1;               /*!< [11..11] This status is set if INT_C is enabled and INT_C pin
                                                     is in low level.                                                          */
      __IM uint16_t RE_TUNING_EVENT : 1;     /*!< [12..12] This status is set if Re-Tuning Request in the Present
                                                     State register changes from 0 to 1.                                       */
      __IOM uint16_t
        BOOT_ACK_COMPLETE_INTERRUPT : 1; /*!< [13..13] This status is set if Re-Tuning Request in the Present
                                                     State register changes from 0 to 1.                                       */
      __IOM uint16_t
        BOOT_DONE_INTERRUPT : 1; /*!< [14..14] boot acknowledge interrupt                                       */
      __IM uint16_t
        ERROR_INTERRUPT : 1; /*!< [15..15] If any of the bits in the RSI_SMIH_ERROR Interrupt Status register
                                                     are set, then this bit is set.                                            */
    } SMIH_NORMAL_INTERRUPT_STATUS_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_ERROR_INTERRUPT_STATUS_REGISTER; /*!< (@ 0x00000032) RSI_SMIH_ERROR Interrupt Status Register                    */

    struct {
      __IOM uint16_t COMMAND_TIMEOUT_ERROR : 1; /*!< [0..0] This bit is set only if no response is returned  within
                                                     64 SD clock cycles from the end bit of the command.                       */
      __IOM uint16_t
        COMMAND_CRC_ERROR : 1; /*!< [1..1] Command CRC RSI_SMIH_ERROR is generated in two cases.                       */
      __IOM uint16_t COMMAND_END_BIT_ERROR : 1; /*!< [2..2] This bit is set when detecting that the end bit of a
                                                     command response is 0.                                                    */
      __IOM uint16_t COMMAND_INDEX_ERROR : 1;   /*!< [3..3] This bit is set if a Command Index error occurs in the
                                                     command response.                                                         */
      __IOM uint16_t DATA_TIMEOUT_ERROR : 1;    /*!< [4..4] This bit is set when detecting one of following timeout
                                                     conditions ,Busy timeout for R1b,R5b type, busy timeout
                                                     after write CRC status,write CRC Status timeout,read data
                                                     timeout.                                                                  */
      __IOM uint16_t DATA_CRC_ERROR : 1;        /*!< [5..5] Occurs when detecting CRC error when transferring read
                                                     data which uses the DAT line                                              */
      __IOM uint16_t DATA_END_BIT_ERROR : 1;    /*!< [6..6] Occurs either when detecting 0 at the end bit position
                                                     of read  data which uses the DAT line or at the end bit
                                                     position of the CRC Status.                                               */
      __IOM uint16_t CURRENT_LIMIT_ERROR : 1;   /*!< [7..7] By setting the SD Bus Power bit in the Power Control
                                                     register,  the SMIHC Controller is requested to supply
                                                     power for the SD Bus                                                      */
      __IOM uint16_t AUTO_CMD_ERROR : 1;        /*!< [8..8] Auto CMD12 and Auto CMD23 use this error status. This
                                                     bit is set when detecting that one of the bits D00-D04
                                                     in Auto CMD RSI_SMIH_ERROR Status  register has changed from 0 to
                                                     1                                                                         */
      __IOM uint16_t ADMA_ERROR : 1;            /*!< [9..9] This bit is set when the SMIHC Controller detects errors
                                                     during ADMA based data transfer.                                          */
      __IOM uint16_t TUNING_ERROR : 1;
      __IOM uint16_t Reserved1 : 5;
    } SMIH_ERROR_INTERRUPT_STATUS_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_NORMAL_INTERRUPT_STATUS_ENABLE_REGISTER; /*!< (@ 0x00000034) Normal Interrupt Status Enable Register    */

    struct {
      __IOM uint16_t
        COMMAND_COMPLETE_STATUS_ENABLE : 1; /*!< [0..0] Command Complete Status Enable                             */
      __IOM uint16_t
        TRANSFER_COMPLETE_STATUS_ENABLE : 1; /*!< [1..1] Command Complete Status Enable                            */
      __IOM uint16_t
        BLOCK_GAP_EVENT_STATUS_ENABLE : 1; /*!< [2..2] Command Complete Status Enable                              */
      __IOM uint16_t
        DMA_INTERRUPT_STATUS_ENABLE : 1; /*!< [3..3] DMA Interrupt Status Enable                                   */
      __IOM uint16_t
        BUFFER_WRITE_READY_STATUS_ENABLE : 1; /*!< [4..4] DMA Interrupt Status Enable                              */
      __IOM uint16_t
        BUFFER_READ_READY_STATUS_ENABLE : 1; /*!< [5..5] Buffer read Ready Status Enable                           */
      __IOM uint16_t
        CARD_INSERTION_STATUS_ENABLE : 1; /*!< [6..6] Card Insertion Status Enable                                 */
      __IOM uint16_t
        CARD_REMOVAL_STATUS_ENABLE : 1; /*!< [7..7] Card Insertion Status Enable                                   */
      __IOM uint16_t
        CARD_INTERRUPT_STATUS_ENABLE : 1; /*!< [8..8] If this bit is set to 0, the SMIHC Controller shall clear
                                                     interrupt request to the System                                           */
      __IOM uint16_t INT_A_STATUS_ENABLE : 1;
      __IOM uint16_t INT_B_STATUS_ENABLE : 1;
      __IOM uint16_t INT_C_STATUS_ENABLE : 1;
      __IOM uint16_t RETUNING_EVENT_STATUS_ENABLE : 1;

      __IOM uint16_t
        BOOT_ACK_COMPLETE_INTERRUPT_STATUS_ENABLE : 1; /*!< [13..13] If this bit is set to 0, the SMIHC Controller shall
                                                     clear interrupt request to the System                                     */
      __IOM uint16_t
        BOOT_DONE_INTERRUPT_STATUS_ENABLE : 1; /*!< [14..14] Boot Done Interrupt Status Enable                     */
      __IM uint16_t FIXED_TO_0 : 1;            /*!< [15..15] The Host Driver shall control error interrupts using
                                                     the  RSI_SMIH_ERROR Interrupt Status Enable register.                              */
    } SMIH_NORMAL_INTERRUPT_STATUS_ENABLE_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_ERROR_INTERRUPT_STATUS_ENABLE_REGISTER; /*!< (@ 0x00000036) RSI_SMIH_ERROR Interrupt Status Enable Register      */

    struct {
      __IOM uint16_t
        COMMAND_TIMEOUT_ERROR_STATUS_ENABLE : 1; /*!< [0..0] Command Timeout RSI_SMIH_ERROR Status Enable                   */
      __IOM uint16_t
        COMMAND_CRC_ERROR_STATUS_ENABLE : 1; /*!< [1..1] Command Timeout RSI_SMIH_ERROR Status Enable                       */
      __IOM uint16_t
        COMMAND_END_BIT_ERROR_STATUS_ENABLE : 1; /*!< [2..2] Command Timeout RSI_SMIH_ERROR Status Enable                   */
      __IOM uint16_t
        COMMAND_INDEX_ERROR_STATUS_ENABLE : 1; /*!< [3..3] Command Index RSI_SMIH_ERROR Status Enable                       */
      __IOM uint16_t
        DATA_TIMEOUT_ERROR_STATUS_ENABLE : 1; /*!< [4..4] Data Timeout RSI_SMIH_ERROR Status Enable                         */
      __IOM uint16_t
        DATA_CRC_ERROR_STATUS_ENABLE : 1; /*!< [5..5] Data Timeout RSI_SMIH_ERROR Status Enable                             */
      __IOM uint16_t
        DATA_END_BIT_ERROR_STATUS_ENABLE : 1; /*!< [6..6] Data End Bit RSI_SMIH_ERROR Status Enable                         */
      __IOM uint16_t
        CURRENT_LIMIT_ERROR_STATUS_ENABLE : 1; /*!< [7..7] Data End Bit RSI_SMIH_ERROR Status Enable                        */
      __IOM uint16_t
        AUTO_CMD_ERROR_STATUS_ENABLE : 1; /*!< [8..8] Auto CMD RSI_SMIH_ERROR Status Enable                                 */
      __IOM uint16_t
        ADMA_ERROR_STATUS_ENABLE : 1; /*!< [9..9] Auto CMD RSI_SMIH_ERROR Status Enable                                     */
      __IOM uint16_t
        TUNING_ERROR_STATUS_ENABLE : 1; /*!< [10..10] Tuning RSI_SMIH_ERROR Status Enable                                   */
      __IOM uint16_t Reserved1 : 5; /*!< [15..11] Reserved1                                                        */
    } SMIH_ERROR_INTERRUPT_STATUS_ENABLE_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_NORMAL_INTERRUPT_SIGNAL_ENABLE_REGISTER; /*!< (@ 0x00000038) Normal Interrupt Signal Enable Register    */

    struct {
      __IOM uint16_t
        COMMAND_COMPLETE_SIGNAL_ENABLE : 1; /*!< [0..0] Command Complete Signal Enable                             */
      __IOM uint16_t
        TRANSFER_COMPLETE_SIGNAL_ENABLE : 1; /*!< [1..1] Command Complete Signal Enable                            */
      __IOM uint16_t
        BLCOK_GAP_EVENT_SIGNAL_ENABLE : 1; /*!< [2..2] Block Gap Event Signal Enable                               */
      __IOM uint16_t
        DMA_INTERRUPT_SIGNAL_ENABLE : 1; /*!< [3..3] DMA Interrupt Signal Enable                                   */
      __IOM uint16_t
        BUFFER_WRITE_READY_SIGNAL_ENABLE : 1; /*!< [4..4] Buffer Write Ready Signal Enable                         */
      __IOM uint16_t
        BUFFER_READ_READY_SIGNAL_ENABLE : 1; /*!< [5..5] Buffer read Ready Signal Enable                           */
      __IOM uint16_t
        CARD_INSERTION_SIGNAL__ENABLE : 1; /*!< [6..6] Card Insertion Signal Enable                                */
      __IOM uint16_t
        CARD_REMOVAL_SIGNAL__ENABLE : 1; /*!< [7..7] Card Insertion Signal Enable                                  */
      __IOM uint16_t
        CARD_INTERRUPT_SIGNAL_ENABLE : 1; /*!< [8..8] Card Interrupt Signal Enable                                 */
      __IOM uint16_t
        INT_A_SIGNAL_ENABLE : 1; /*!< [9..9] INT_A Signal Enable                                                */
      __IOM uint16_t
        INT_B_SIGNAL_ENABLE : 1; /*!< [10..10] INT_B Signal Enable                                              */
      __IOM uint16_t
        INT_C_SIGNAL_ENABLE : 1; /*!< [11..11] INT_C Signal Enable                                              */
      __IOM uint16_t
        RE_TUNNING_EVENT_SIGNAL_ENABLE : 1; /*!< [12..12] Re-Tuning Event Signal Enable                            */
      __IM uint16_t
        BOOT_ACK_COMPLETE_INTERRUPT_SIGNAL_ENABLE : 1; /*!< [13..13] Boot Ack Complete Interrupt Signal Enable     */
      __IM uint16_t
        BOOT_DONE_INTERRUPT_SIGNAL_ENABLE : 1; /*!< [14..14] Boot Ack Complete Interrupt Signal Enable             */
      __IM uint16_t FIXED_TO_0 : 1;            /*!< [15..15] The Host Driver shall control error interrupts  using
                                                     the RSI_SMIH_ERROR Interrupt Signal Enable register.                               */
    } SMIH_NORMAL_INTERRUPT_SIGNAL_ENABLE_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_ERROR_INTERRUPT_SIGNAL_ENABLE_REGISTER; /*!< (@ 0x0000003A) RSI_SMIH_ERROR Interrupt Signal Enable Register      */

    struct {
      __IOM uint16_t
        COMMAND_TIMEOUT_ERROR_SIGNAL_ENABLE : 1; /*!< [0..0] Command Timeout RSI_SMIH_ERROR Signal Enable                   */
      __IOM uint16_t
        COMMAND_CRC_ERROR_SIGNAL_ENABLE : 1; /*!< [1..1] Command CRC RSI_SMIH_ERROR Signal Enable                           */
      __IOM uint16_t
        COMMAND_END_BIT_ERROR_SIGNAL_ENABLE : 1; /*!< [2..2] Command End Bit Signal Enable                         */
      __IOM uint16_t
        COMMAND_INDEX_ERROR_SIGNAL_ENABLE : 1; /*!< [3..3] Command Index Signal Enable                             */
      __IOM uint16_t
        DATA_TIMEOUT_ERROR_SIGNAL_ENABLE : 1; /*!< [4..4] Data Timeout RSI_SMIH_ERROR Signal Enable                         */
      __IOM uint16_t
        DATA_CRC_ERROR_SIGNAL_ENABLE : 1; /*!< [5..5] Data CRC RSI_SMIH_ERROR Signal Enable                                 */
      __IOM uint16_t
        DATA_END_BIT_ERROR_SIGNAL_ENABLE : 1; /*!< [6..6] Data CRC RSI_SMIH_ERROR Signal Enable                             */
      __IOM uint16_t
        CURRENT_LIMIT_ERROR_SIGNAL_ENABLE : 1; /*!< [7..7] Data CRC RSI_SMIH_ERROR Signal Enable                            */
      __IOM uint16_t
        AUTO_CMD_ERROR_SIGNAL_ENABLE : 1; /*!< [8..8] Auto CMD RSI_SMIH_ERROR Signal Enable                                 */
      __IOM uint16_t
        ADMA_ERROR_SIGNAL_ENABLE : 1; /*!< [9..9] ADMA RSI_SMIH_ERROR Signal Enable                                         */
      __IOM uint16_t
        TUNING_ERROR_SIGNAL_ENABLE : 1; /*!< [10..10] Tuning RSI_SMIH_ERROR Signal Enable                                   */
      __IOM uint16_t RESERVED1 : 5; /*!< [15..11] reserved1                                                        */
    } SMIH_ERROR_INTERRUPT_SIGNAL_ENABLE_REGISTER_b;
  };

  union {
    __IM uint16_t
      SMIH_AUTO_CMD_ERROR_STATUS_REGISTER; /*!< (@ 0x0000003C) Auto CMD RSI_SMIH_ERROR Status Register                      */

    struct {
      __IM uint16_t AUTO_CMD12_NOT_EXECUTED : 1; /*!< [0..0] If memory multiple block data transfer is not started
                                                     due to command error, this bit is not set because it is
                                                     not necessary to issue Auto CMD12                                         */
      __IM uint16_t AUTO_CMD_TIME_ERROR : 1;     /*!< [1..1] This bit is set if no response is returned within 64
                                                     SDCLK cycles from the end bit of command.                                 */
      __IM uint16_t AUTO_CMD_CRC_ERROR : 1;      /*!< [2..2] This bit is set when detecting a CRC error in the command
                                                     response                                                                  */
      __IM uint16_t AUTO_CMD_END_BIT_ERROR : 1;  /*!< [3..3] This bit is set when detecting that the end bit of command
                                                     response is 0.                                                            */
      __IM uint16_t AUTO_CMD_INDEX_ERROR : 1;
      __IM uint16_t RESERVED1 : 2; /*!< [6..5] reserved1                                                          */
      __IM uint16_t
        COMMAND_NOT_ISSUED_BY_AUTO_CMD12_ERROR : 1; /*!< [7..7] Setting this bit to 1 means CMD_wo_DAT is not executed
                                                     due to an Auto CMD12 RSI_SMIH_ERROR (D04-D01) in this register.                    */
      __IM uint16_t Reserved1 : 8; /*!< [15..8] reserved2                                                         */
    } SMIH_AUTO_CMD_ERROR_STATUS_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_HOST_CONTROL_2_REGISTER; /*!< (@ 0x0000003E) Host Control 2 Register                                    */

    struct {
      __IOM uint16_t UHS_MODE_SELECT : 3; /*!< [2..0] This field is used to select one of UHS-I modes and effective
                                                     when 1.8V Signaling Enable is set to 1.                                   */
      __IOM uint16_t _1_8V_SIGNALING_ENABLE : 1; /*!< [3..3] This bit controls voltage regulator for I/O cell. 3.3V
                                                     is supplied  to the card regardless of signaling voltage.
                                                     Setting this bit  from 0 to 1 starts changing signal voltage
                                                     from 3.3V to 1.8V.                                                        */
      __IOM uint16_t
        DRIVER_STRENGTH_SELECT : 2;       /*!< [5..4] SMIHC Controller output driver in 1.8V signaling is selected
                                                     by this bit.  In 3.3V signaling, this field is not effective.             */
      __IOM uint16_t EXECUTE_TUNNING : 1; /*!< [6..6] This bit is set to 1 to start tuning procedure and  automatically
                                                     cleared when tuning procedure is completed.                               */
      __IOM uint16_t SAMPLING_CLOCK_SELECT : 1; /*!< [7..7] SMIHC Controller uses this bit to select sampling clock
                                                     to  receive CMD and DAT. This bit is set by tuning procedure
                                                     and valid after the completion of tuning.                                 */
      __IOM uint16_t Reserved1 : 6; /*!< [13..9] Reserved1                                                         */
      __IOM uint16_t
        ASYNCHRONOUS_INTERRUPT_ENABLE : 1; /*!< [14..14] This bit can be set to 1 if a card that supports asynchronous
                                                     interrupts and Asynchronous Interrupt Support is set to
                                                     1 in the Capabilities register.                                           */
      __IOM uint16_t PRESET_VALUE_ENABLE : 1; /*!< [15..15] As the operating SDCLK frequency and I/O driver strength
                                                     depend on the Host System implementation, it is difficult
                                                     to determine these parameters in the Standard Host Driver.                */
    } SMIH_HOST_CONTROL_2_REGISTER_b;
  };

  union {
    __IOM uint64_t SMIH_CAPABILITIES_REGISTER;
    struct {
      __IOM uint64_t TIMEOUT_CLOCK_FREQUENCY : 6;
      __IOM uint64_t Reserved1 : 1;
      __IOM uint64_t TIMEOUT_CLOCK_UINT : 7;
      __IOM uint64_t BASE_CLOCK_FREQUENCY : 8;
      __IOM uint64_t MAXIMUM_BLOCK_LENGTH : 2;
      __IOM uint64_t _8BIT_SUPPORT_FOR_EMDDED_DEVICE : 1;
      __IOM uint64_t ADMA2_SUPPORT : 1;
      __IOM uint64_t Reserved2 : 1;
      __IOM uint64_t HISH_SPEED_SUPPORT : 1;
      __IOM uint64_t SDMA_SUPPORT : 1;
      __IOM uint64_t SUSPEND_RESUME_SUPPORT : 1;
      __IOM uint64_t SDR50_SUPPORT : 1;
      __IOM uint64_t SDR104_SUPPORT : 1;
      __IOM uint64_t DDR50_SUPPORT : 1;
      __IOM uint64_t HS200_SUPPORT : 1;
      __IOM uint64_t DRIVER_TYPEA_SUPPORT : 1;
      __IOM uint64_t DRIVER_TYPEC_SUPPORT : 1;
      __IOM uint64_t DRIVER_TYPED_SUPPORT : 1;
      __IOM uint64_t HS400_SUPPORT : 1;
      __IOM uint64_t Reserved3 : 4;
      __IOM uint64_t RESERVED4 : 1;
      __IOM uint64_t USE_TUNING_FOR_SDR50 : 1;
      __IOM uint64_t RETUNING_MODES : 2;
      __IOM uint64_t CLOCK_MULTIPLIER : 8;
      __IOM uint64_t Reserved4 : 8;
    } SMIH_CAPABILITIES_REGISTER_b;
  };

  __IOM uint64_t SMIH_MAXIMUM_CURRENT_CAPABILITIES_REGISTER;
  __IM uint16_t RESERVED2[2]; //50,52

  union {
    __IM uint32_t
      SMIH_ADMA_ERROR_STATUS_REGISTER; /*!< (@ 0x00000054) ADMA RSI_SMIH_ERROR Status Register                              */

    struct {
      __IM uint32_t ADMA_ERROR_STATE : 2; /*!< [1..0] This field indicates the state of ADMA when error is
                                                     occurred during ADMA data transfer. This field never indicates
                                                     10 because ADMA never stops in this state.                                */
      __IM uint32_t
        ADMA_LENGHT_MISMATCH_ERROR : 1; /*!< [2..2] While Block Count Enable being set, the total data length
                                                     specified by the Descriptor table is different from that
                                                     specified by the Block Count and Block Length.                            */
      __IM uint32_t RESERVED1 : 29; /*!< [31..3] Reserved1                                                         */
    } SMIH_ADMA_ERROR_STATUS_REGISTER_b;
  };

  __IOM uint16_t SMIH_ADMA_SYSTEM_ADDRESS0_REGISTER;
  __IOM uint16_t SMIH_ADMA_SYSTEM_ADDRESS1_REGISTER;
  __IOM uint16_t SMIH_ADMA_SYSTEM_ADDRESS2_REGISTER;
  __IOM uint16_t SMIH_ADMA_SYSTEM_ADDRESS3_REGISTER;

  union {
    __IM uint16_t
      SMIH_PRESET_VALUE_REGISTER[8]; /*!< (@ 0x00000060) Preset Value for Initialization register                  */

    struct {
      __IM uint16_t SDCLK_FREQUENCY_SELECT_VALUE : 10; /*!< [9..0] 10-bit preset value to set SDCLK Frequency Select in
                                                     the Clock Control  Register is described by a host system.                */
      __IM uint16_t CLOCK_GENERATOR_SELECT_VALUE : 1;  /*!< [10..10] This bit is effective when Host Controller supports
                                                     programmable clock generator                                              */
      __IM uint16_t RESERVED1 : 2; /*!< [12..11] reserved1                                                        */
      __IM uint16_t
        UPPER_BIT_OF_DRIVER_STRENGTH_SELECT : 1; /*!< [13..13] This bit reflects the SMIH_DRIVE_STRENGTH2  in association
                                                     with 14-15 bits of Drive Strength select                                  */
      __IM uint16_t DRIVER_STRENGTH_SELECT_VALUE : 2; /*!< [15..14] Driver Strength is supported by 1.8V signaling bus
                                                     speed modes. This field is meaningless for 3.3V signaling.                */
    } SMIH_PRESET_VALUE_REGISTER_b[8];
  };
  __IM uint32_t RESERVED4[28];

  union {
    __IOM uint32_t
      SMIH_TX_TUNE_REGISTER; /*!< (@ 0x000000E0) Tx Tune Register                                           */

    struct {
      __IOM uint32_t
        TAP_POINT_VALUE : 6; /*!< [5..0] Tap point value                                                    */
      __IOM uint32_t
        AUTO_INCREMENT : 1; /*!< [6..6] Increment locally or not.                                          */
      __IOM uint32_t INCREMENT_TAP_POINT_VALUE : 3; /*!< [9..7] Increment value,When Auto increment bit6 is set to 1,
                                                     Host controller will increment Tap point based on the value
                                                     programmed in this field.                                                 */
      __IOM uint32_t RESERVED1 : 22; /*!< [31..10] reserved1                                                        */
    } SMIH_TX_TUNE_REGISTER_b;
  };

  union {
    __IOM uint32_t
      SMIH_RX_TUNE_REGISTER; /*!< (@ 0x000000E4) Rx Tune Register                                           */

    struct {
      __IOM uint32_t
        TAP_POINT_VALUE : 6; /*!< [5..0] Tap point value                                                    */
      __IOM uint32_t
        AUTO_INCREMENT : 1; /*!< [6..6] Increment locally or not.                                          */
      __IOM uint32_t INCREMENT_TAP_POINT_VALUE : 3; /*!< [9..7] Increment value,When Auto increment bit6 is set to 1,
                                                     Host controller will increment Tap point based on the value
                                                     programmed in this field.                                                 */
      __IOM uint32_t RESERVED1 : 22; /*!< [31..10] reserved1                                                        */
    } SMIH_RX_TUNE_REGISTER_b;
  };

  union {
    __IOM uint32_t
      SMIH_DS_TUNE_REGISTER; /*!< (@ 0x000000E8) Ds Tune Register                                           */

    struct {
      __IOM uint32_t
        TAP_POINT_VALUE : 6; /*!< [5..0] Tap point value                                                    */
      __IOM uint32_t
        AUTO_INCREMENT : 1; /*!< [6..6] Increment locally or not.                                          */
      __IOM uint32_t INCREMENT_TAP_POINT_VALUE : 3; /*!< [9..7] Increment value,When Auto increment bit6 is set to 1,
                                                     Host controller will increment Tap point based on the value
                                                     programmed in this field.                                                 */
      __IOM uint32_t RESERVED1 : 22; /*!< [31..10] reserved1                                                        */
    } SMIH_DS_TUNE_REGISTER_b;
  };

  union {
    __IOM uint16_t
      SMIH_AHB_BURST_SIZE_REGISTER; /*!< (@ 0x000000EC) AHB Burst Size Register                                    */

    struct {
      __IOM uint16_t
        AHB_MASTER_BURST_SIZE_REGISTER : 7; /*!< [6..0] AHB Master performs Burst operations as per this register.
                                                     The default Burst operations performed by SMIHC AHB Master
                                                     is INCR4, INCR8 and INCR16.                                               */
      __IM uint16_t : 3;
      __IOM uint16_t RESERVED1 : 6; /*!< [15..10] reserved1                                                        */
    } SMIH_AHB_BURST_SIZE_REGISTER_b;
  };
  __IM uint16_t RESERVED5;
  __IM uint32_t RESERVED6;

  union {
    __IM uint32_t
      SMIH_SDH_REVISION_ID_REGISTER; /*!< (@ 0x000000F4) SDH Revision ID Register                                  */

    struct {
      __IM uint32_t MAJOR : 8;       /*!< [7..0] Both SW and HW changes required                                    */
      __IM uint32_t MINOR : 8;       /*!< [15..8] HW Changes alone No Software Changes required                     */
      __IM uint32_t MAINTENANCE : 8; /*!< [23..16] Minor BUG Fixes in HW alone                                      */
      __IM uint32_t RESERVED1 : 8;   /*!< [31..24] reserved1                                                        */
    } SMIH_SDH_REVISION_ID_REGISTER_b;
  };
  __IM uint32_t RESERVED7;

  union {
    __IOM uint16_t
      SMIH_SLOT_INTERRUPT_STATUS_REGISTER; /*!< (@ 0x000000FC) Slot Interrupt Status Register                      */

    struct {
      __IM uint16_t INTERRUPT_SIGNAL_FOR_A_SLOT : 1; /*!< [0..0] This status bit indicates the logical OR of Interrupt
                                                     Signal and Wakeup Signal for the slot.                                    */
      __IOM uint16_t RESERVED1 : 15; /*!< [15..1] reserved1                                                         */
    } SMIH_SLOT_INTERRUPT_STATUS_REGISTER_b;
  };

  union {
    __IM uint16_t
      SMIH_HOST_CONTROLLER_VERSION_REGISTER; /*!< (@ 0x000000FE) Slot Interrupt Status Register                    */

    struct {
      __IM uint16_t
        SPECIFICATION_VERSION_NUMBER : 8;      /*!< [7..0] This status indicates the Host Controller Spec Version,The
                                                     upper and lower 4-bits indicate the version.                              */
      __IM uint16_t VENDOR_VERSION_NUMBER : 8; /*!< [15..8] This status is reserved for the vendor version number.The
                                                     Host Driver should  not use this status.                                  */
    } SMIH_HOST_CONTROLLER_VERSION_REGISTER_b;
  };
} SMIH_Type;

/* --------------------  End of section using anonymous unions  ------------------- */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__ICCARM__)
/* leave anonymous unions enabled */
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
#pragma warning restore
#else
#warning Not supported compiler type
#endif

/* ================================================================================ */
/* ================              Peripheral memory map             ================ */
/* ================================================================================ */

#define I2C0_BASE             0x44010000UL
#define I2C1_BASE             0x47040000UL
#define I2C2_BASE             0x24040000UL
#define NWP_FSM_BASE          0x41300110UL
#define MCPWM_BASE            0x47070000UL
#define UDMA0_BASE            0x44030000UL
#define UDMA1_BASE            0x24078000UL
#define GPDMA_G_BASE          0x21080000UL
#define GPDMA_C_BASE          0x21081004UL
#define HWRNG_BASE            0x45090000UL
#define TIMERS_BASE           0x24042000UL
#define CAN1_BASE             0x45070000UL
#define QEI_BASE              0x47060000UL
#define UART0_BASE            0x44000000UL
#define UART1_BASE            0x45020000UL
#define USART0_BASE           0x44000100UL
#define ULP_UART_BASE         0x24041800UL
#define GSPI0_BASE            0x45030000UL
#define GSPI1_BASE            0x47030000UL
#define SSI0_BASE             0x44020000UL
#define SSISalve_BASE         0x45010000UL
#define SSI2_BASE             0x24040800UL
#define SIO_BASE              0x47000000UL
#define QSPI_BASE             0x12000000UL
#define CRC_BASE              0x45080000UL
#define EFUSE_BASE            0x4600C000UL
#define CCI_BASE              0x46170000UL
#define I2S0_BASE             0x47050000UL
#define I2S1_BASE             0x24040400UL
#define IID_AES_BASE          0x20480500UL
#define IID_QK_BASE           0x20480600UL
#define IID_RPINE_BASE        0x20480400UL
#define CT0_BASE              0x45060000UL
#define CT1_BASE              0x45060100UL
#define CT2_BASE              0x45061000UL
#define CT3_BASE              0x45061100UL
#define CT_MUX_REG_BASE       0x4506F000UL
#define ETHERNET_BASE         0x20240000UL
#define EGPIO_BASE            0x46130000UL
#define EGPIO1_BASE           0x2404C000UL
#define SSDIO_BASE            0x40000000UL
#define SSPI_BASE             0x20200000UL
#define FIM_BASE              0x24070000UL
#define USB_BASE              0x20210000UL
#define AUX_ADC_DAC_COMP_BASE 0x24043800UL
#define OPAMP_BASE            0x24043A14UL
#define IR_BASE               0x24040C00UL
#define SMIH_BASE             0x20220000UL

/*Clock and power*/
#define M4_CLK_BASE               0x46000000UL
#define MCU_RTC_BASE              0x24048200UL
#define MCU_WDT_BASE              0x24048300UL
#define MCU_BATTERY_FF            0x24048400UL
#define MCU_RETENTION             0x24048600UL
#define MCU_TEMP_SENSOR_BASE      0x24048500UL
#define MCU_NPSS_AON_BASE         0x24048000UL
#define MCU_FSAM_BASE             0x24048100UL
#define MCU_PROCESSOR_SENSOR_BASE 0x24048540UL
#define ULP_CLK_BASE              0x24041400UL
#define VAD_BASE                  0x24043400UL

/*End of clock and power */

/* ================================================================================ */
/* ================             Peripheral declaration             ================ */
/* ================================================================================ */

#define I2C0             ((I2C0_Type *)I2C0_BASE)
#define I2C1             ((I2C0_Type *)I2C1_BASE)
#define I2C2             ((I2C0_Type *)I2C2_BASE)
#define MCPWM            ((MCPWM_Type *)MCPWM_BASE)
#define UDMA0            ((UDMA0_Type *)UDMA0_BASE)
#define UDMA1            ((UDMA0_Type *)UDMA1_BASE)
#define GPDMA_G          ((GPDMA_G_Type *)GPDMA_G_BASE)
#define GPDMA_C          ((GPDMA_C_Type *)GPDMA_C_BASE)
#define HWRNG            ((HWRNG_Type *)HWRNG_BASE)
#define TIMERS           ((TIMERS_Type *)TIMERS_BASE)
#define CAN1             ((CAN_Type *)CAN1_BASE)
#define QEI              ((QEI_Type *)QEI_BASE)
#define UART0            ((USART0_Type *)UART0_BASE)
#define UART1            ((USART0_Type *)UART1_BASE)
#define USART0           ((USART0_Type *)USART0_BASE)
#define ULP_UART         ((USART0_Type *)ULP_UART_BASE)
#define GSPI0            ((GSPI0_Type *)GSPI0_BASE)
#define GSPI1            ((GSPI0_Type *)GSPI1_BASE)
#define SSI0             ((SSI0_Type *)SSI0_BASE)
#define SSISlave         ((SSI0_Type *)SSISalve_BASE)
#define SSI2             ((SSI0_Type *)SSI2_BASE)
#define SIO              ((SIO_Type *)SIO_BASE)
#define QSPI             ((QSPI_Type *)QSPI_BASE)
#define CRC              ((CRC_Type *)CRC_BASE)
#define EFUSE            ((EFUSE_Type *)EFUSE_BASE)
#define CCI              ((CCI_Type *)CCI_BASE)
#define I2S0             ((I2S0_Type *)I2S0_BASE)
#define I2S1             ((I2S0_Type *)I2S1_BASE)
#define IID_AES          ((IID_AES_Type *)IID_AES_BASE)
#define IID_QK           ((IID_QK_Type *)IID_QK_BASE)
#define IID_RPINE        ((IID_RPINE_Type *)IID_RPINE_BASE)
#define CT0              ((CT0_Type *)CT0_BASE)
#define CT1              ((CT0_Type *)CT1_BASE)
#define CT2              ((CT0_Type *)CT2_BASE)
#define CT3              ((CT0_Type *)CT3_BASE)
#define CT_MUX_REG       ((CT_MUX_REG_Type *)CT_MUX_REG_BASE)
#define ETHERNET         ((ETHERNET_Type *)ETHERNET_BASE)
#define ETH              ((ETH_Type *)ETHERNET_BASE)
#define EGPIO            ((EGPIO_Type *)EGPIO_BASE)
#define EGPIO1           ((EGPIO_Type *)EGPIO1_BASE)
#define SDIO             ((SDIO0_Type *)SSDIO_BASE)
#define SPI_SLAVE        ((RSI_SPI_SLAVE_T *)SSPI_BASE)
#define FIM              ((FIM_Type *)FIM_BASE)
#define USB              ((USBHS_Type *)USB_BASE)
#define AUX_ADC_DAC_COMP ((AUX_ADC_DAC_COMP_Type *)AUX_ADC_DAC_COMP_BASE)
#define OPAMP            ((OPAMP_Type *)OPAMP_BASE)
#define IR               ((IR_Type *)IR_BASE)
#define VAD              ((VAD_Type *)VAD_BASE)
#define SMIH             ((SMIH_Type *)SMIH_BASE)

/*clock and power */
#define M4CLK           ((M4CLK_Type *)M4_CLK_BASE)
#define RTC             ((RTC_Type *)MCU_RTC_BASE)
#define WDT             ((MCU_WDT_Type *)MCU_WDT_BASE)
#define TIME_PERIOD     ((TIME_PERIOD_Type *)MCU_RTC_BASE)
#define BATT_FF         ((MCU_Battery *)MCU_BATTERY_FF)
#define MCU_RET         ((MCU_RETENTION_Type *)MCU_RETENTION)
#define MCU_TEMP        ((MCU_TEMP_Type *)MCU_TEMP_SENSOR_BASE)
#define MCU_AON         ((MCU_AON_Type *)MCU_NPSS_AON_BASE)
#define MCU_FSM         ((MCU_Fsm_t *)MCU_FSAM_BASE)
#define PROCESSOR_SENSE ((MCU_ProcessSensor_Type *)MCU_PROCESSOR_SENSOR_BASE)
#define ULPCLK          ((ULPCLK_Type *)ULP_CLK_BASE)
#define NWP_FSM         ((NWP_Fsm_t *)NWP_FSM_BASE)

/*end of clock and power */

/** @} */ /* End of group Device_Peripheral_Registers */
/** @} */ /* End of group RS1xxxx */
/** @} */ /* End of group Redpine Signals Inc. */

#define MISC_CONFIG_BASE 0x46008000UL

#ifdef __cplusplus
}
#endif

#endif /* __RS1XXXX_H__*/
