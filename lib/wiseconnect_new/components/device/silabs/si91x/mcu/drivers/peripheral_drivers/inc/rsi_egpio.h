/***************************************************************************/ /**
* @file  rsi_egpio.h
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

// Includes Files

#include "rsi_ccp_common.h"
#include "base_types.h"

#ifndef RSI_EGPIO_H
#define RSI_EGPIO_H

#ifdef __cplusplus
extern "C" {
#endif

// EGPIO numbers
#define GPIO0  0  // EGPIO number 0
#define GPIO1  1  // EGPIO number 1
#define GPIO2  2  // EGPIO number 2
#define GPIO3  3  // EGPIO number 3
#define GPIO4  4  // EGPIO number 4
#define GPIO5  5  // EGPIO number 5
#define GPIO6  6  // EGPIO number 6
#define GPIO7  7  // EGPIO number 7
#define GPIO8  8  // EGPIO number 8
#define GPIO9  9  // EGPIO number 9
#define GPIO10 10 // EGPIO number 10
#define GPIO11 11 // EGPIO number 11
#define GPIO12 12 // EGPIO number 12
#define GPIO13 13 // EGPIO number 13
#define GPIO14 14 // EGPIO number 14
#define GPIO15 15 // EGPIO number 14

// EGPIO Ports(NOTE : Each ports consists of 16 GPIO pins numbered from (0 - 15))
#define EGPIO_PORT0 0 // EGPIO port number 0
#define EGPIO_PORT1 1 // EGPIO port number 1
#define EGPIO_PORT2 2 // EGPIO port number 2

// EGPIO PIN INTERRUPTS
// NOTE : Total 8 pin interrupts are supported , these interrupts can be mapped to any of the GPIO ports
#define EGPIO_PIN_INTERRUPT0 0 //  Select interrupt channel 0
#define EGPIO_PIN_INTERRUPT1 1 //  Select interrupt channel 1
#define EGPIO_PIN_INTERRUPT2 2 //  Select interrupt channel 2
#define EGPIO_PIN_INTERRUPT3 3 //  Select interrupt channel 3
#define EGPIO_PIN_INTERRUPT4 4 //  Select interrupt channel 4
#define EGPIO_PIN_INTERRUPT5 5 //  Select interrupt channel 5
#define EGPIO_PIN_INTERRUPT6 6 //  Select interrupt channel 6
#define EGPIO_PIN_INTERRUPT7 7 //  Select interrupt channel 7

// EGPIO GROUP INTERUPTS
// NOTE : Total 2 group interrupts are supported
#define EGPIO_GROUP_INTERRUPT0 0 // Select EGPIO Group interrupt channel 0
#define EGPIO_GROUP_INTERRUPT1 1 // Select EGPIO Group interrupt channel 1

// Pin multiplexing
// NOTE : Each GPIO supports up to 8 multiplexing functions those can be selected by passing following macros into the pin mux function
#define EGPIO_PIN_MUX_MODE0  0U  //  Select pin mode 0
#define EGPIO_PIN_MUX_MODE1  1U  //  Select pin mode 1
#define EGPIO_PIN_MUX_MODE2  2U  //  Select pin mode 2
#define EGPIO_PIN_MUX_MODE3  3U  //  Select pin mode 3
#define EGPIO_PIN_MUX_MODE4  4U  //  Select pin mode 4
#define EGPIO_PIN_MUX_MODE5  5U  //  Select pin mode 5
#define EGPIO_PIN_MUX_MODE6  6U  //  Select pin mode 6
#define EGPIO_PIN_MUX_MODE7  7U  //  Select pin mode 7
#define EGPIO_PIN_MUX_MODE8  8U  //  Select pin mode 8
#define EGPIO_PIN_MUX_MODE9  9U  //  Select pin mode 9
#define EGPIO_PIN_MUX_MODE10 10U //  Select pin mode 10
#define EGPIO_PIN_MUX_MODE11 11U //  Select pin mode 11
#define EGPIO_PIN_MUX_MODE12 12U //  Select pin function 12
#define EGPIO_PIN_MUX_MODE13 13U //  Select pin function 13
#define EGPIO_PIN_MUX_MODE14 14U //  Select pin function 14
#define EGPIO_PIN_MUX_MODE15 15U //  Select pin function 15

// interrupt clear flags
#define EGPIO_PIN_INT_CLR_FALLING BIT(2) //  Falling edge interrupt clear
#define EGPIO_PIN_INT_CLR_RISING  BIT(1) //  Raising edge interrupt clear
#define INTERRUPT_STATUS_CLR      BIT(0) //  interrupts are cleared
#define WAKEUP_INTERRUPT          BIT(1)

// GPIO directions
#define EGPIO_CONFIG_DIR_INPUT            1U //  Configure EGPIO as input mode
#define EGPIO_CONFIG_DIR_OUTPUT           0U //  Configure EGPIO as output mode
#define NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR (0x41300000 + 0x004)

// MISC host
#define MISC_HOST (*(volatile uint32_t *)(0x46008000 + 0x0C))

// m4 PAD configuration defines

#define PAD_CONFIG_REG(x) (*(volatile uint32_t *)(0x46004000 + 4 * x)) //  REN enable bit(this should be enable)
#define PADSELECTION \
  (*(volatile uint32_t *)(0x41300000 + 0x610)) //  PAD selection (0 t0 21) A value of 1 on this gives control to M4SS

#if defined(SLI_SI917)
#define PADSELECTION_1 \
  (*(volatile uint32_t *)(0x41300000 + 0x618)) //  PAD selection (22 to 33) A value of 1 on this gives control to M4SS
#endif
#define SDIO_CNTD_TO_TASS \
  (*(volatile uint32_t *)NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR) //  sdio connected to tass  (0 for M4SS and 1 for TASS)

// Ulp pad configuration defines
#define ULP_PAD_CONFIG_REG_0 (*(volatile uint32_t *)(0x2404A000 + 0x0))
#define ULP_PAD_CONFIG_REG_1 (*(volatile uint32_t *)(0x2404A000 + 0x4))
#define ULP_PAD_CONFIG_REG_2 (*(volatile uint32_t *)(0x2404A000 + 0x8))

// ulp_socgpio_n_mode
#define ULP_SOC_GPIO_MODE(x) (*(volatile uint32_t *)(0x24041400 + 4 * x)) // x= 0 to 15

// MISC host

#define HOST_PADS_GPIO_MODE (*(volatile uint32_t *)(0x46008000 + 0x44))

// Ulp Ren enable
#define CLOCK_ENABLE (*(uint32_t *)(0x24041400 + 0x00))

typedef enum en_driver_state {
  HiZ      = 0, //  0 for  HiZ      (P1=0,P2=0)
  Pullup   = 1, //  1 for  Pullup   (P1=0,P2=1)
  Pulldown = 2, //  2 for  Pulldown (P1=1,P2=0)
  Repeater = 3  //  3 for  Repeater (P1=1,P2=1)
} en_driver_state_t;
typedef enum en_driver_strength_select {
  two_milli_amps    = 0, //  0 for two_milli_amps   (E1=0,E2=0)
  four_milli_amps   = 1, //  1 for four_milli_amps  (E1=0,E2=1)
  eight_milli_amps  = 2, //  2 for eight_milli_amps (E1=1,E2=0)
  twelve_milli_amps = 3  //  3 for twelve_milli_amps(E1=1,E2=1)
} en_driver_strength_select_t;

#define DDS_MASK 0xC0 // driver disabled state
#define DSS_MASK 0x3  // driver strength select
#define POS_MASK 0x4  // power on start
#define ST_MASK  0x8  // active high schmitt trigger
#define RE_MASK  0x10 // receiver enable
#define SR_MASK  0x20 // slew rate

#define ULP_DDS_MASK_SET_1_AND_3 0xC0   // driver disabled state
#define ULP_DDS_MASK_SET_2_AND_4 0xC000 // driver disabled state
#define ULP_DSS_MASK_SET_1_AND_3 0x3    // driver strength select
#define ULP_DSS_MASK_SET_2_AND_4 0x300  // driver strength select
#define ULP_POS_MASK_SET_1_AND_3 0x4    // power on start
#define ULP_POS_MASK_SET_2_AND_4 0x400  // power on start
#define ULP_ST_MASK_SET_1_AND_3  0x8    // active high schmitt trigger
#define ULP_ST_MASK_SET_2_AND_4  0x800  // active high schmitt trigger
#define ULP_SR_MASK_SET_1_AND_3  0x20   // slew rate
#define ULP_SR_MASK_SET_2_AND_4  0x2000 // slew rate

typedef enum en_ulp_driver_disable_state {
  ulp_HiZ      = 0, //  0 for  HiZ      (P1=0,P2=0)
  ulp_Pullup   = 1, //  1 for  Pullup   (P1=0,P2=1)
  ulp_Pulldown = 2, //  2 for  Pulldown (P1=1,P2=0)
  ulp_Repeater = 3  //  3 for  Repeater (P1=1,P2=1)
} en_ulp_driver_disable_state_t;
typedef enum en_ulp_driver_strength_select {
  ulp_two_milli_amps    = 0, //  0 for two_milli_amps   (E1=0,E2=0)
  ulp_four_milli_amps   = 1, //  1 for four_milli_amps  (E1=0,E2=1)
  ulp_eight_milli_amps  = 2, //  2 for eight_milli_amps (E1=1,E2=0)
  ulp_twelve_milli_amps = 3  //  3 for twelve_milli_amps(E1=1,E2=1)
} en_ulp_driver_strength_select_t;

void egpio_set_dir(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, boolean_t dir);

void egpio_set_pin(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t val);

boolean_t egpio_get_pin(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

boolean_t egpio_get_dir(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

void egpio_pin_int_sel(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t port, uint8_t pin);

void egpio_set_int_fall_edge_enable(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_set_int_fall_edge_disable(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_set_int_rise_edge_enable(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_set_int_rise_edge_disable(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_set_int_low_level_enable(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_int_mask(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_int_un_mask(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_set_int_low_level_disable(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_set_int_high_level_enable(EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_set_int_high_level_disable(EGPIO_Type *pEGPIO, uint8_t intCh);

uint8_t egpio_get_int_stat(const EGPIO_Type *pEGPIO, uint8_t intCh);

void egpio_int_clr(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t flags);

void egpio_set_pin_mux(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t u8Mux);

void egpio_ulp_soc_gpio_mode(ULPCLK_Type *pULPCLK, uint8_t gpio, uint8_t mode);

void egpio_set_port_mask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

void egpio_set_port_un_mask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

void egpio_port_masked_load(EGPIO_Type *pEGPIO, uint8_t port, uint16_t u16Val);

void egpio_set_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);

void egpio_port_load(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);

void egpio_word_load(EGPIO_Type *pEGPIO, uint8_t pin, uint16_t val);

void egpio_clr_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);

void egpio_toggle_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);

uint16_t egpio_get_port(const EGPIO_Type *pEGPIO, uint8_t port);

void egpio_group_int_one_enable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

void egpio_group_int_one_disable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

void egpio_group_int_two_enable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

void egpio_group_int_mask(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_un_Mask(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_enable(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_disable(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_level(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_edge(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_and(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_or(EGPIO_Type *pEGPIO, uint8_t grpInt);

uint32_t egpio_group_int_stat(const EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_wkeup_Enable(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_wkeup_disable(EGPIO_Type *pEGPIO, uint8_t grpInt);

void egpio_group_int_clr(EGPIO_Type *pEGPIO, uint8_t grpInt, uint8_t u8ClrFlags);

void egpio_group_int_two_disable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);

void egpio_set_group_int_one_pol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol);

void egpio_set_group_int_two_pol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol);

void egpio_host_pads_gpio_mode_enable(uint8_t u8GpioNum);

void egpio_host_pads_gpio_mode_disable(uint8_t u8GpioNum);

void egpio_pad_selection_enable(uint8_t padNum);

void egpio_pad_selection_disable(uint8_t padNum);

void egpio_pad_receiver_enable(uint8_t u8GpioNum);

void egpio_pad_receiver_disable(uint8_t u8GpioNum);

void egpio_pad_sdio_connected(void);

void egpio_pad_driver_disable_state(uint8_t u8GpioNum, en_driver_state_t endstate);

void egpio_pad_driver_strength_select(uint8_t u8GpioNum, en_driver_strength_select_t strength);

void egpio_pad_power_on_start_enable(uint8_t u8GpioNum, uint8_t val);

void egpio_pad_active_high_schmitt_trigger(uint8_t u8GpioNum, uint8_t val);

void egpio_pad_slew_rate_controll(uint8_t u8GpioNum, uint8_t val);

void egpio_ulp_pad_receiver_enable(uint8_t gpio);

void egpio_ulp_pad_receiver_disable(uint8_t u8GpioNum);

void egpio_ulp_pad_driver_disable_state(uint8_t u8GpioNum, en_ulp_driver_disable_state_t disablestate);

void egpio_ulp_pad_driver_strength_select(uint8_t u8GpioNum, en_ulp_driver_strength_select_t strength);

void egpio_ulp_pad_power_on_start_enable(uint8_t u8GpioNum, uint8_t val);

void egpio_ulp_pad_active_high_schmitt_trigger(uint8_t u8GpioNum, uint8_t val);

void egpio_ulp_pad_slew_rate_controll(uint8_t u8GpioNum, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif // RSI_EGPIO_H
