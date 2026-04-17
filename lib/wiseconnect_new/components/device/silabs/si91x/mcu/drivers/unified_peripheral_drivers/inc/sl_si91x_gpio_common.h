/******************************************************************************
* @file sl_si91x_gpio_common.h
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
#ifndef SL_SI91X_GPIO_COMMONH
#define SL_SI91X_GPIO_COMMONH

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
///
#include "si91x_device.h"
/***************************************************************************/ /**
 * @addtogroup GPIO
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define PAD_REG_BASE     0x46004000UL ///< PAD configuration register base address
#define NPSS_INT_BASE    0x12080000UL ///< UULP INTR base address
#define ULP_PAD_REG_BASE 0x2404A000UL ///< ULP PAD configuration base address

#define GPIO          ((EGPIO_Type *)EGPIO_BASE)     ///< MCU HP  base address
#define ULP_GPIO      ((EGPIO_Type *)EGPIO1_BASE)    ///< MCU ULP base address
#define UULP_GPIO_FSM ((MCU_FSM_Type *)MCU_FSM_BASE) ///< SLEEP FSM base address
#define UULP_GPIO     ((MCU_RET_Type *)MCU_RET_BASE) ///< MCU retention base address

#define PAD_REG(x)          ((PAD_CONFIG_Type *)(PAD_REG_BASE + (4 * x))) ///< PAD configuration register for GPIO_n(n = 0 t0 63)
#define ULP_PAD_CONFIG0_REG ((ULP_PAD_CONFIG_Type0 *)(ULP_PAD_REG_BASE + 0x0)) ///< ULP PAD configuration register 0
#define ULP_PAD_CONFIG1_REG ((ULP_PAD_CONFIG_Type1 *)(ULP_PAD_REG_BASE + 0x4)) ///< ULP PAD configuration register 1
#define ULP_PAD_CONFIG2_REG ((ULP_PAD_CONFIG_Type2 *)(ULP_PAD_REG_BASE + 0x8)) ///< ULP PAD configuration register 2

#define UULP_PAD_CONFIG_REG(x) \
  ((UULP_PAD_CONFIG_Type *)(0x2404861C + 4 * x)) ///< UULP V_bat PAD configuration base address
#define PADSELECTION \
  (*(volatile uint32_t *)(0x41300000 + 0x610)) ///< PAD selection (0 to 21) A value of 1 on this gives control to M4SS
#define PADSELECTION_1 \
  (*(volatile uint32_t *)(0x41300000 + 0x618)) ///< PAD selection (22 to 33) A value of 1 on this gives control to M4SS
#define HOST_PADS_GPIO_MODE (*(volatile uint32_t *)(0x46008000 + 0x44)) ///< MISC host base address
#define ULP_PAD_CONFIG_REG  (*(volatile uint32_t *)(0x2404A008))        ///< ULP PAD register
#define NWP_MCUHP_GPIO_CTRL2 \
  (*(volatile uint32_t *)(0x41300000 + 0x004)) ///<  Writing 1 to this enables NWP to configure the GPIO_25 to GPIO_30
#define GPIO_NPSS_INTERRUPT_MASK_SET_REG \
  (*(volatile uint32_t *)(NPSS_INT_BASE + 0x00)) ///< NPSS mask set register base address
#define GPIO_NPSS_INTERRUPT_MASK_CLR_REG \
  (*(volatile uint32_t *)(NPSS_INT_BASE + 0x04)) ///< NPSS mask clear register base address
#define GPIO_NPSS_INTERRUPT_CLEAR_REG \
  (*(volatile uint32_t *)(NPSS_INT_BASE + 0x08)) ///< NPSS clear register base address
#define GPIO_NPSS_INTERRUPT_STATUS_REG \
  (*(volatile uint32_t *)(NPSS_INT_BASE + 0x0C)) ///< NPSS status register base address
#define GPIO_NPSS_GPIO_CONFIG_REG \
  (*(volatile uint32_t *)(NPSS_INT_BASE + 0x10)) ///< NPSS GPIO configuration register base address
#define UULP_GPIO_STATUS      (*(volatile uint32_t *)(NPSS_INT_BASE + 0x14)) ///< UULP GPIO status base address
#define GPIO_25_30_CONFIG_REG (*(volatile uint32_t *)(0X46008000 + 0x0C))    ///< GPIO(25-30) pin configuration register

#define CLR 0
#define SET 1

#define SL_DEBUG_ASSERT

#define NIBBLE_SHIFT    4      ///< Nibble shift for interrupt
#define BYTE_SHIFT      8      ///< Byte shift for interrupt
#define WORD_SHIFT      16     ///< Word shift for interrupt
#define LSB_WORD_MASK   0x00FF ///< GPIO LSB word mask
#define LSB_NIBBLE_MASK 0x0F   ///< GPIO LSB nibble mask

#define MAX_GPIO_PORT_PIN 16 ///< GPIO maximum port pins
#define HOST_PAD          12 ///< GPIO Host PAD

#define GPIO_PA_COUNT 16 ///< GPIO port A maximum pins
#define GPIO_PB_COUNT 16 ///< GPIO port B maximum pins
#define GPIO_PC_COUNT 16 ///< GPIO port C maximum pins
#define GPIO_PD_COUNT 9  ///< GPIO port D maximum pins
#define GPIO_PE_COUNT 12 ///< GPIO port E maximum pins

#define GPIO_PA_MASK 0xFFFFUL ///< GPIO port A mask
#define GPIO_PB_MASK 0xFFFFUL ///< GPIO port B mask
#define GPIO_PC_MASK 0xFFFFUL ///< GPIO port C mask
#define GPIO_PD_MASK 0x01FFUL ///< GPIO port D mask

#define SL_PERIPHERAL_CLK M4CLK ///< GPIO instance clock

#define UNUSED_VAR(expr) ((void)(expr))

#define GPIO_PA_PIN_0_5_VALIDATE 6  ///< GPIO port A validate pins (0-5)
#define GPIO_PA_PIN_MAX_VALIDATE 75 ///< GPIO port A maximum pins to validate
#define GPIO_PB_PIN_MAX_VALIDATE 59 ///< GPIO port B maximum pins to validate
#define GPIO_PC_PIN_MAX_VALIDATE 43 ///< GPIO port C maximum pins to validate
#define GPIO_PD_PIN_MAX_VALIDATE 27 ///< GPIO port D maximum pins to validate

#ifdef SL_DEBUG_ASSERT
#define SL_GPIO_ASSERT(expr) ((expr) ? (void)0U : sl_assert_failed((uint8_t *)__FILE__, __LINE__))
#else
#define SL_GPIO_ASSERT(expr) ((void)(expr))
#endif

#define SL_GPIO_VALIDATE_STRENGTH(strength)           (strength > 3 ? 0 : 1)      ///< Validate driver strength
#define SL_GPIO_VALIDATE_PARAMETER(value)             (value > 1 ? 0 : 1)         ///< Validate GPIO parameters
#define SL_GPIO_VALIDATE_DISABLE_STATE(disable_state) (disable_state > 3 ? 0 : 1) ///< Validate driver disable state
#define SL_GPIO_VALIDATE_PAD(pad_num)                 ((pad_num > 34) && (pad_num < 1) ? 0 : 1) ///< Validate GPIO HP pad selection
#define SL_GPIO_VALIDATE_PIN(pin_num)                 (((pin_num < 6) || (pin_num > 63)) ? 0 : 1) ///< Validate GPIO HP pin number
#define SL_GPIO_VALIDATE_FLAG(flag)                   ((flag > 0x0F) ? 0 : 1)    ///< Validate GPIO flags
#define SL_GPIO_VALIDATE_ULP_INTR(ulp_intr)           ((ulp_intr > 12) ? 0 : 1)  ///< Validate ULP interrupts
#define SL_GPIO_VALIDATE_ULP_PIN(pin_num)             ((pin_num > 12) ? 0 : 1)   ///< Validate ULP pins
#define SL_GPIO_VALIDATE_UULP_PIN(pin_num)            ((pin_num) > 5 ? 0 : 1)    ///< Validate UULP pins
#define SL_GPIO_VALIDATE_MODE_PARAMETER(mode)         ((mode) > 10 ? 0 : 1)      ///< Validate UULP, ULP mode
#define SL_GPIO_VALIDATE_UULP_INTR(interrupt)         ((interrupt) > 16 ? 0 : 1) ///< Validate UULP interrupt
#define SL_GPIO_VALIDATE_PORT(port)                   ((port) > 5 ? 0 : 1)       ///< Validate GPIO port
#define SL_GPIO_VALIDATE_MODE(mode)                   ((mode) > 15 ? 0 : 1)      ///< Validate GPIO mode
#define SL_GPIO_VALIDATE_INTR(interrupt)              ((interrupt > 8) ? 0 : 1)  ///< Validate GPIO interrupt
///< Validate GPIO port and pin
#define SL_GPIO_NDEBUG_PORT_PIN(port, pin)                                                        \
  (port == 0   ? (((pin < GPIO_PA_PIN_0_5_VALIDATE) || (pin > GPIO_PA_PIN_MAX_VALIDATE)) ? 0 : 1) \
   : port == 1 ? ((pin > GPIO_PB_PIN_MAX_VALIDATE) ? 0 : 1)                                       \
   : port == 2 ? ((pin > GPIO_PC_PIN_MAX_VALIDATE) ? 0 : 1)                                       \
   : port == 3 ? ((pin > GPIO_PD_PIN_MAX_VALIDATE) ? 0 : 1)                                       \
               : 0)
///< Validate GPIO host pad port and pin
#define SL_GPIO_VALIDATE_HOST_PIN(port, pin)                                                             \
  (port == SL_GPIO_PORT_A   ? (((pin >= HOST_PAD_MIN) && (pin <= HOST_PAD_MAX)) ? TRUE : FALSE)          \
   : port == SL_GPIO_PORT_B ? (((pin >= GPIO_PIN_NUMBER9) && (pin <= GPIO_PIN_NUMBER14)) ? TRUE : FALSE) \
                            : FALSE)
#define SL_GPIO_VALIDATE_ULP_PORT_PIN(port, pin)  (port == 4 ? ((pin > 11) ? 0 : 1) : 0) ///< Validate ULP port and pin
#define SL_GPIO_VALIDATE_UULP_PORT_PIN(port, pin) (port == 5 ? ((pin > 5) ? 0 : 1) : 0)  ///< Validate UULP port and pin

#define GRP_IRQ0_Handler IRQ050_Handler ///<  GPIO Group Interrupt 0
#define GRP_IRQ1_Handler IRQ051_Handler ///<  GPIO Group Interrupt 1

#define PIN_IRQ0_Handler IRQ052_Handler ///<  GPIO Pin Interrupt   0
#define PIN_IRQ1_Handler IRQ053_Handler ///<  GPIO Pin Interrupt   1
#define PIN_IRQ2_Handler IRQ054_Handler ///<  GPIO Pin Interrupt   2
#define PIN_IRQ3_Handler IRQ055_Handler ///<  GPIO Pin Interrupt   3
#define PIN_IRQ4_Handler IRQ056_Handler ///<  GPIO Pin Interrupt   4
#define PIN_IRQ5_Handler IRQ057_Handler ///<  GPIO Pin Interrupt   5
#define PIN_IRQ6_Handler IRQ058_Handler ///<  GPIO Pin Interrupt   6
#define PIN_IRQ7_Handler IRQ059_Handler ///<  GPIO Pin Interrupt   7

#define UULP_PIN_IRQ_Handler IRQ021_Handler ///< UULP Pin Interrupt 0

#define ULP_PIN_IRQ_Handler   IRQ018_Handler ///< ULP Pin Interrupt
#define ULP_GROUP_IRQ_Handler IRQ019_Handler ///< ULP Group Interrupt

#define PIN_INTR_0 0 ///< HP GPIO pin interrupt 0
#define PIN_INTR_1 1 ///< HP GPIO pin interrupt 1
#define PIN_INTR_2 2 ///< HP GPIO pin interrupt 2
#define PIN_INTR_3 3 ///< HP GPIO pin interrupt 3
#define PIN_INTR_4 4 ///< HP GPIO pin interrupt 4
#define PIN_INTR_5 5 ///< HP GPIO pin interrupt 5
#define PIN_INTR_6 6 ///< HP GPIO pin interrupt 6
#define PIN_INTR_7 7 ///< HP GPIO pin interrupt 7

#define UULP_MASK  0x00 ///< UULP GPIO pin mask
#define ULP_STATUS 0x01 ///< ULP GPIO pin status
//maintaining older macros for migration
#define UULP_INTR_1            0x01 ///< UULP GPIO pin interrupt 1
#define UULP_INTR_2            0x02 ///< UULP GPIO pin interrupt 2
#define UULP_INTR_3            0x04 ///< UULP GPIO pin interrupt 3
#define UULP_INTR_4            0x08 ///< UULP GPIO pin interrupt 4
#define UULP_INTR_5            0x10 ///< UULP GPIO pin interrupt 5
#define UULP_INTR_0_STATUS_BIT 0x01 ///< UULP GPIO pin interrupt 0
#define UULP_INTR_1_STATUS_BIT 0x02 ///< UULP GPIO pin interrupt 1
#define UULP_INTR_2_STATUS_BIT 0x04 ///< UULP GPIO pin interrupt 2
#define UULP_INTR_3_STATUS_BIT 0x08 ///< UULP GPIO pin interrupt 3
#define UULP_INTR_4_STATUS_BIT 0x10 ///< UULP GPIO pin interrupt 4

#define ULP_PIN_INTR_0 0 ///< ULP GPIO pin interrupt 0
#define ULP_PIN_INTR_1 1 ///< ULP GPIO pin interrupt 1
#define ULP_PIN_INTR_2 2 ///< ULP GPIO pin interrupt 2
#define ULP_PIN_INTR_3 3 ///< ULP GPIO pin interrupt 3
#define ULP_PIN_INTR_4 4 ///< ULP GPIO pin interrupt 4
#define ULP_PIN_INTR_5 5 ///< ULP GPIO pin interrupt 5
#define ULP_PIN_INTR_6 6 ///< ULP GPIO pin interrupt 6
#define ULP_PIN_INTR_7 7 ///< ULP GPIO pin interrupt 7

#define ULP_GROUP_INTR_0 0 ///< ULP GPIO group interrupt 0
#define ULP_GROUP_INTR_1 1 ///< ULP GPIO group interrupt 1

#define MAX_GPIO_PIN_INT 8 ///< Maximum HP GPIO pin interrupts

#define PININT0_NVIC_NAME EGPIO_PIN_0_IRQn ///< HP GPIO pin interrupt 0 number
#define PININT1_NVIC_NAME EGPIO_PIN_1_IRQn ///< HP GPIO pin interrupt 1 number
#define PININT2_NVIC_NAME EGPIO_PIN_2_IRQn ///< HP GPIO pin interrupt 2 number
#define PININT3_NVIC_NAME EGPIO_PIN_3_IRQn ///< HP GPIO pin interrupt 3 number
#define PININT4_NVIC_NAME EGPIO_PIN_4_IRQn ///< HP GPIO pin interrupt 4 number
#define PININT5_NVIC_NAME EGPIO_PIN_5_IRQn ///< HP GPIO pin interrupt 5 number
#define PININT6_NVIC_NAME EGPIO_PIN_6_IRQn ///< HP GPIO pin interrupt 6 number
#define PININT7_NVIC_NAME EGPIO_PIN_7_IRQn ///< HP GPIO pin interrupt 7 number

#define GROUP_0_INTERRUPT_NAME EGPIO_GROUP_0_IRQn ///< HP GPIO group interrupt 1 number
#define GROUP_1_INTERRUPT_NAME EGPIO_GROUP_1_IRQn ///< HP GPIO group interrupt 2 number

#define ULP_PININT0_NVIC_NAME    ULP_EGPIO_PIN_IRQn   ///< ULP GPIO pin interrupt number
#define ULP_GROUP_INTERRUPT_NAME ULP_EGPIO_GROUP_IRQn ///< ULP GPIO group interrupt number

#define UULP_PININT_NVIC_NAME NPSS_TO_MCU_GPIO_INTR_IRQn ///< UULP GPIO pin interrupt number

#define SL_GPIO_GROUP_INTERRUPT_OR     1 ///< GPIO group interrupt AND/OR
#define SL_GPIO_GROUP_INTERRUPT_WAKEUP 4 ///< GPIO group interrupt wakeup
#define SL_GPIO_ULP_PORT               4 ///< ULP GPIO port number
#define SL_GPIO_UULP_PORT              5 ///< Initializing UULP GPIO port value
#define SL_ULP_GPIO_PORT               4 ///< Refers to ULP Port

#define _MODE0  0  ///< GPIO mode 0
#define _MODE1  1  ///< GPIO mode 1
#define _MODE2  2  ///< GPIO mode 2
#define _MODE3  3  ///< GPIO mode 3
#define _MODE4  4  ///< GPIO mode 4
#define _MODE5  5  ///< GPIO mode 5
#define _MODE6  6  ///< GPIO mode 6
#define _MODE7  7  ///< GPIO mode 7
#define _MODE8  8  ///< GPIO mode 8
#define _MODE9  9  ///< GPIO mode 9
#define _MODE10 10 ///< GPIO mode 10
#define _MODE12 12 ///< GPIO mode 12
#define _MODE14 14 ///< GPIO mode 14

#define _GPIO_P_MODEL_MODE0_DISABLED                0x00000000UL ///< Mode DISABLED for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_INPUT                   0x00000001UL ///< Mode INPUT for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_INPUTPULL               0x00000002UL ///< Mode INPUTPULL for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_INPUTPULLFILTER         0x00000003UL ///< Mode INPUTPULLFILTER for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_PUSHPULL                0x00000004UL ///< Mode PUSHPULL for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_PUSHPULLALT             0x00000005UL ///< Mode PUSHPULLALT for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDOR                 0x00000006UL ///< Mode WIREDOR for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDORPULLDOWN         0x00000007UL ///< Mode WIREDORPULLDOWN for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDAND                0x00000008UL ///< Mode WIREDAND for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDANDFILTER          0x00000009UL ///< Mode WIREDANDFILTER for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDANDPULLUP          0x0000000AUL ///< Mode WIREDANDPULLUP for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDANDPULLUPFILTER    0x0000000BUL ///< Mode WIREDANDPULLUPFILTER for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDANDALT             0x0000000CUL ///< Mode WIREDANDALT for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDANDALTFILTER       0x0000000DUL ///< Mode WIREDANDALTFILTER for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDANDALTPULLUP       0x0000000EUL ///< Mode WIREDANDALTPULLUP for GPIO_P_MODEL
#define _GPIO_P_MODEL_MODE0_WIREDANDALTPULLUPFILTER 0x0000000FUL ///< Mode WIREDANDALTPULLUPFILTER for GPIO_P_MODEL

#define GPIO_P_MODEL_MODE0_DISABLED  (_GPIO_P_MODEL_MODE0_DISABLED << 0)  ///< Shifted mode DISABLED for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_INPUT     (_GPIO_P_MODEL_MODE0_INPUT << 0)     ///< Shifted mode INPUT for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_INPUTPULL (_GPIO_P_MODEL_MODE0_INPUTPULL << 0) ///< Shifted mode INPUTPULL for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_INPUTPULLFILTER \
  (_GPIO_P_MODEL_MODE0_INPUTPULLFILTER << 0) ///< Shifted mode INPUTPULLFILTER for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_PUSHPULL (_GPIO_P_MODEL_MODE0_PUSHPULL << 0) ///< Shifted mode PUSHPULL for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_PUSHPULLALT \
  (_GPIO_P_MODEL_MODE0_PUSHPULLALT << 0)                              ///< Shifted mode PUSHPULLALT for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDOR (_GPIO_P_MODEL_MODE0_WIREDOR << 0) ///< Shifted mode WIREDOR for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDORPULLDOWN \
  (_GPIO_P_MODEL_MODE0_WIREDORPULLDOWN << 0) ///< Shifted mode WIREDORPULLDOWN for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDAND (_GPIO_P_MODEL_MODE0_WIREDAND << 0) ///< Shifted mode WIREDAND for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDANDFILTER \
  (_GPIO_P_MODEL_MODE0_WIREDANDFILTER << 0) ///< Shifted mode WIREDANDFILTER for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDANDPULLUP \
  (_GPIO_P_MODEL_MODE0_WIREDANDPULLUP << 0) ///< Shifted mode WIREDANDPULLUP for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDANDPULLUPFILTER \
  (_GPIO_P_MODEL_MODE0_WIREDANDPULLUPFILTER << 0) ///< Shifted mode WIREDANDPULLUPFILTER for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDANDALT \
  (_GPIO_P_MODEL_MODE0_WIREDANDALT << 0) ///< Shifted mode WIREDANDALT for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDANDALTFILTER \
  (_GPIO_P_MODEL_MODE0_WIREDANDALTFILTER << 0) ///< Shifted mode WIREDANDALTFILTER for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDANDALTPULLUP \
  (_GPIO_P_MODEL_MODE0_WIREDANDALTPULLUP << 0) ///< Shifted mode WIREDANDALTPULLUP for GPIO_P_MODEL
#define GPIO_P_MODEL_MODE0_WIREDANDALTPULLUPFILTER \
  (_GPIO_P_MODEL_MODE0_WIREDANDALTPULLUPFILTER << 0) ///< Shifted mode WIREDANDALTPULLUPFILTER for GPIO_P_MODEL

#define GPIO_PAD_0 0 ///< GPIO PAD number 0
#define GPIO_PAD_3 3 ///< GPIO PAD number 3
#define GPIO_PAD_4 4 ///< GPIO PAD number 4
#define GPIO_PAD_7 7 ///< GPIO PAD number 7
#define GPIO_PAD_8 8 ///< GPIO PAD number 8

#define HOST_PAD_SELECT 12 ///< GPIO Host PAD selection
#define PAD_SELECT      22 ///< GPIO PAD number 22
#define HOST_PAD_MIN    25 ///< GPIO Host PAD number 25
#define HOST_PAD_MAX    30 ///< GPIO Host PAD number 30

#define PRIORITY_19 19 ///< GPIO ulp group Interrupt priority
#define PRIORITY_21 21 ///< GPIO uulp group Interrupt priority
#define PRIORITY_50 50 ///< GPIO m4 group 0 Interrupt priority
#define PRIORITY_51 51 ///< GPIO m4 group 1 Interrupt priority

#define ULP_PORT_NUM  0    ///< GPIO ULP port number
#define UULP_PIN_MASK 0x1F ///< GPIO UULP pin mask

#define BIT_0  0  ///< GPIO bit 0 in configuration register
#define BIT_8  8  ///< GPIO bit 8 in configuration register
#define BIT_16 16 ///< GPIO bit 16 in configuration register
#define BIT_24 24 ///< GPIO bit 24 in configuration register

#define PORT_MASK      0xFFFF ///< GPIO port mask
#define INTR_CLR       0x07   ///< GPIO interrupt clear
#define INTERRUPT_MASK 0x0F   ///< GPIO interrupt mask
#define MASK_CTRL      0x03
#define MASK_INTR      0x01 ///< GPIO interrupt mask

/** =========================================================================================================================== **/
/** ================                                           GPIO                                           ==================**/
/** =========================================================================================================================== **/
///@brief HP GPIO PAD configuration register fields
typedef struct {
  union {
    __IOM uint32_t PAD_CONFIG_REG; ///< (@ 0x46004000) GPIO PAD configuration register

    struct {
      __IOM uint32_t PADCONFIG_E1_E2 : 2; ///< [1..0] Drive strength selector
      __IOM uint32_t PADCONFIG_POS : 1;   ///< [2..2] Power-on-Start enable
      __IOM uint32_t PADCONFIG_SMT : 1;   ///< [3..3] Active high Schmitt trigger (Hysteresis) select
      __IOM uint32_t PADCONFIG_REN : 1;   ///< [4..4] Active high receiver enable
      __IOM uint32_t PADCONFIG_SR : 1;    ///< [5..5] Slew Rate Control
      __IOM uint32_t PADCONFIG_P1_P2 : 2; ///< [7..6] Driver disabled state control
      __IOM uint32_t RESERVED1 : 24;      ///< [31..8] Reserved1
    } GPIO_PAD_CONFIG_REG_b;
  };
} PAD_CONFIG_Type;

///@brief UULP GPIO PAD configuration register fields
typedef struct {
  union {
    __IOM uint32_t UULP_PAD_CONFIG_REG; ///< (@ 0x2404861C) UULP VBAT GPIO configuration register

    struct {
      __IOM uint32_t GPIO_MODE : 3;       ///< [2..0] GPIO Mode for UULP_VBAT_GPIO_n(n=0:4)
      __IOM uint32_t GPIO_REN : 1;        ///< [3..3] Receiver of PAD enable
      __IOM uint32_t GPIO_OEN : 1;        ///< [4..4] Direction of PAD
      __IOM uint32_t GPIO_OUTPUT : 1;     ///< [5..5] Value driven on PAD in OUTPUT mode
      __IOM uint32_t GPIO_PAD_SELECT : 1; ///< [6..6] PAD selection between M4,NWP
      __IOM uint32_t RESERVED : 1;        ///< [7..7] Reserved
      __IOM uint32_t GPIO_POLARITY : 1;   ///< [8..8] Polarity of UULP GPIO
      __IOM uint32_t RESERVED1 : 23;      ///< [31..9] Reserved1
    } UULP_GPIO_PAD_CONFIG_REG_b;
  };
} UULP_PAD_CONFIG_Type;

///@brief ULP GPIO PAD configuration register0 fields
typedef struct {
  union {
    __IOM uint32_t ULP_PAD_CONFIG_REG0;

    struct {
      __IOM uint32_t PADCONFIG_E1_E2_1 : 2; ///< [1..0] Drive strength selector for ULP_GPIO_0 - ULP_GPIO_3
      __IOM uint32_t PADCONFIG_POS_1 : 1;   ///< [2..2] Power on start enable for ULP_GPIO_0 - ULP_GPIO_3
      __IOM uint32_t PADCONFIG_SMT_1 : 1;   ///< [3..3] Active high schmitt trigger for ULP_GPIO_0 - ULP_GPIO_3
      __IOM uint32_t RESERVED : 1;          ///< [4..4] Reserved
      __IOM uint32_t PADCONFIG_SR_1 : 1;    ///< [5..5] Slew rate control for ULP_GPIO_0 - ULP_GPIO_3
      __IOM uint32_t PADCONFIG_P1_P2_1 : 2; ///< [7..6] Driver disabled state control for ULP_GPIO_0 - ULP_GPIO_3
      __IOM uint32_t PADCONFIG_E1_E2_2 : 2; ///< [9..8] Drive strength selector for ULP_GPIO_4 - ULP_GPIO_7
      __IOM uint32_t PADCONFIG_POS_2 : 1;   ///< [10..10] Power on start enable for ULP_GPIO_4 - ULP_GPIO_7
      __IOM uint32_t PADCONFIG_SMT_2 : 1;   ///< [11..11] Active high schmitt trigger for ULP_GPIO_4 - ULP_GPIO_7
      __IOM uint32_t RESERVED1 : 1;         ///< [12..12] Reserved1
      __IOM uint32_t PADCONFIG_SR_2 : 1;    ///< [13..13] Slew rate control for ULP_GPIO_4 - ULP_GPIO_7
      __IOM uint32_t PADCONFIG_P1_P2_2 : 2; ///< [15..14] Driver disabled state control for ULP_GPIO_4 - ULP_GPIO_7
      __IOM uint32_t RESERVED2 : 16;        ///< [31..16] Reserved2
    } ULP_GPIO_PAD_CONFIG_REG_0;
  };
} ULP_PAD_CONFIG_Type0;

///@brief ULP GPIO PAD configuration register1 fields
typedef struct {
  union {
    __IOM uint32_t ULP_PAD_CONFIG_REG1;

    struct {
      __IOM uint32_t PADCONFIG_E1_E2_1 : 2; ///< [1..0] Drive strength selector for ULP_GPIO_8 - ULP_GPIO_11
      __IOM uint32_t PADCONFIG_POS_1 : 1;   ///< [2..2] Power on start enable for ULP_GPIO_8 - ULP_GPIO_11
      __IOM uint32_t PADCONFIG_SMT_1 : 1;   ///< [3..3] Active high schmitt trigger for ULP_GPIO_8 - ULP_GPIO_11
      __IOM uint32_t RESERVED : 1;          ///< [4..4] Reserved
      __IOM uint32_t PADCONFIG_SR_1 : 1;    ///< [5..5] Slew rate control for ULP_GPIO_8 - ULP_GPIO_11
      __IOM uint32_t PADCONFIG_P1_P2_1 : 2; ///< [7..6] Driver disabled state control for ULP_GPIO_8 - ULP_GPIO_11
      __IOM uint32_t RESERVED1 : 8;         ///< [15..8] Reserved1
      __IOM uint32_t RESERVED2 : 16;        ///< [31..16] Reserved2
    } ULP_GPIO_PAD_CONFIG_REG_1;
  };
} ULP_PAD_CONFIG_Type1;

///@brief ULP GPIO PAD configuration register2 fields
typedef struct {
  union {
    __IOM uint32_t ULP_PAD_CONFIG_REG2;

    struct {
      __IOM uint32_t PADCONFIG_REN : 12; ///< [11..0] Active high receiver enable for ULP_GPIO_11 - ULP_GPIO_0
      __IOM uint32_t RESERVED : 20;      ///< [31..12] Reserved
    } ULP_GPIO_PAD_CONFIG_REG_2;
  };
} ULP_PAD_CONFIG_Type2;

/** @} (end addtogroup GPIO) */

#ifdef __cplusplus
}
#endif

#endif ///< SL_SI91X_GPIO_COMMONH
       /**************************************************************************************************/
