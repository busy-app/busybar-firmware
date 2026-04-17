/***************************************************************************/ /**
 * @file sl_si91x_button.c
 * @brief  Sample API functions for operating buttons (external interrupts).
 *  Please see the board header file for the definition of the BUTTON*
 *  instances used below.  The BUTTON* defines used here are designed to make
 *  this library generic such that only the board header needs to be modified
 *  to change which GPIO the buttons are on.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_common.h"
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_button_instances.h"
#include "si91x_device.h"
#include "sl_driver_gpio.h"
#include "sl_si91x_driver_gpio.h"
#include "rsi_retention.h"
#include "rsi_rom_egpio.h"
#if defined(DEBUG_UART)
#include "rsi_debug.h"
#endif // DEBUG_UART
#include "sl_si91x_clock_manager.h"
/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#define BUTTON_M4_INTR   6 // M4 Pin interrupt number
#define BUTTON_UULP_INTR 2 // UULP GPIO pin interrupt 2
#define AVL_INTR_NO      0 // available interrupt number

#define MAX_HP_BUTTON_COUNT   8 // The maximum number of HP GPIO interrupts that can be configured simultaneously.
#define MAX_ULP_BUTTON_COUNT  8 // The maximum number of ULP GPIO interrupts that can be configured simultaneously.
#define MAX_UULP_BUTTON_COUNT 5 // The maximum number of UULP GPIO interrupts that can be configured simultaneously.
#define SL_SI91x_MAX_BUTTON_COUNT \
  MAX_HP_BUTTON_COUNT + MAX_ULP_BUTTON_COUNT + MAX_UULP_BUTTON_COUNT // Total available GPIO interrupts

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
uint8_t HP_intr[8] = { PIN_INTR_0, PIN_INTR_1, PIN_INTR_2, PIN_INTR_3, PIN_INTR_4, PIN_INTR_5, PIN_INTR_6, PIN_INTR_7 };
uint8_t ULP_intr[8] = { ULP_PIN_INTR_0, ULP_PIN_INTR_1, ULP_PIN_INTR_2, ULP_PIN_INTR_3,
                        ULP_PIN_INTR_4, ULP_PIN_INTR_5, ULP_PIN_INTR_6, ULP_PIN_INTR_7 };
// This stores the button state so that IRQ ISRs know when to notify buttonIsrs.
#if (SL_SI91x_BUTTON_COUNT > 0)
static int8_t buttonState[SL_SI91x_BUTTON_COUNT];
const sl_button_t *HP_button_context[MAX_HP_BUTTON_COUNT];
const sl_button_t *ULP_button_context[MAX_ULP_BUTTON_COUNT];
const sl_button_t *UULP_button_context[MAX_UULP_BUTTON_COUNT];
#endif //(SL_SI91x_BUTTON_COUNT > 0)

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
void sl_si91x_button_internal_isr(const sl_button_t *handle);
static void button_HP_IRQHandler(uint8_t intr_no);
static void button_ULP_IRQHandler(uint8_t intr_no);
static void button_UULP_IRQHandler(uint8_t intr_no);
static int8_t button_get_free_ulp_context_index(void);
static int8_t button_get_free_hp_context_index(void);

/**
 * @brief New Interrupt handler for button
 * @return  Nothing
 */
void button_HP_IRQHandler(uint8_t intr_no)
{
  uint8_t index = intr_no;
  sl_si91x_button_internal_isr(HP_button_context[index]);
}

void button_ULP_IRQHandler(uint8_t intr_no)
{
  uint8_t index = intr_no;
  sl_si91x_button_internal_isr(ULP_button_context[index]);
}

void button_UULP_IRQHandler(uint8_t intr_no)
{
  uint8_t index = intr_no;
  sl_si91x_button_internal_isr(UULP_button_context[index]);
}

int8_t button_get_free_ulp_context_index(void)
{
  for (int i = 0; i < MAX_ULP_BUTTON_COUNT; i++) {
    if (ULP_button_context[i] == NULL) {
      return i;
    }
  }
  return -1;
}

int8_t button_get_free_hp_context_index(void)
{
  for (int i = 0; i < MAX_HP_BUTTON_COUNT; i++) {
    if (HP_button_context[i] == NULL) {
      return i;
    }
  }
  return -1;
}

void sl_si91x_button_init(const sl_button_t *handle)
{
  sl_status_t status                              = SL_STATUS_OK;
  sl_si91x_gpio_pin_config_t sl_button_pin_config = { { handle->port, handle->pin }, GPIO_INPUT };
  sl_gpio_t button_port_pin                       = { handle->port, handle->pin };
  uint8_t button_context_index                    = 0;

  do {

    // Set button configuration
    status = sl_gpio_set_configuration(sl_button_pin_config);
    if (status != SL_STATUS_OK) {
      break;
    }

    // Enable button interrupt
    if (handle->port == UULP_VBAT) {
      UULP_button_context[handle->pin] = handle;
      // UULP_intr identifies the interrupt source and is passed to the UULP GPIO callback for processing.
      sl_si91x_gpio_driver_configure_uulp_interrupt((sl_si91x_gpio_interrupt_config_flag_t)handle->interrupt_config,
                                                    handle->pin,
                                                    (void *)&button_UULP_IRQHandler);
    } else if (handle->port == ULP) {
      status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO); // Enable GPIO ULP_CLK
      if (status != SL_STATUS_OK) {
        break;
      }
      button_context_index                     = button_get_free_ulp_context_index();
      ULP_button_context[button_context_index] = handle;
      // ULP_intr identifies the interrupt source and is passed to the ULP GPIO callback for processing.
      sl_gpio_driver_configure_interrupt(&button_port_pin,
                                         ULP_intr[button_context_index],
                                         (sl_gpio_interrupt_flag_t)handle->interrupt_config,
                                         (void *)&button_ULP_IRQHandler,
                                         AVL_INTR_NO);
    } else {
      status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO); // Enable GPIO M4_CLK
      if (status != SL_STATUS_OK) {
        break;
      }
      button_context_index                    = button_get_free_hp_context_index();
      HP_button_context[button_context_index] = handle;
      // HP_intr identifies the interrupt source and is passed to the HP GPIO callback for processing.
      sl_gpio_driver_configure_interrupt(&button_port_pin,
                                         HP_intr[button_context_index],
                                         (sl_gpio_interrupt_flag_t)handle->interrupt_config,
                                         (void *)&button_HP_IRQHandler,
                                         AVL_INTR_NO);
    }

    buttonState[handle->button_number] = 1;
  } while (false);
}

sl_status_t sl_si91x_button_deinit(const sl_button_t *handle)
{
  sl_status_t status                      = SL_STATUS_OK;
  sl_si91x_gpio_instances_t gpio_instance = M4_GPIO_INSTANCE;
  uint8_t index                           = 0;
  uint8_t intr_no                         = 0;
  do {
    if (handle->port == UULP_VBAT) {
      intr_no       = handle->pin;
      gpio_instance = UULP_GPIO_INSTANCE;
      sl_si91x_gpio_configure_uulp_interrupt(SL_GPIO_INTERRUPT_NONE, intr_no);
      UULP_button_context[handle->pin] = NULL;
    } else if (handle->port == ULP) {
      for (index = 0; index < MAX_ULP_BUTTON_COUNT; index++) {
        if (ULP_button_context[index] == handle) {
          ULP_button_context[index] = NULL;
          break;
        }
      }
      if (index == MAX_ULP_BUTTON_COUNT) {
        status = SL_STATUS_INVALID_HANDLE;
        break;
      }
      intr_no       = index;
      gpio_instance = ULP_GPIO_INSTANCE;
      sl_si91x_gpio_configure_ulp_pin_interrupt(intr_no, SL_GPIO_INTERRUPT_NONE, handle->pin);
    } else {
      for (index = 0; index < MAX_HP_BUTTON_COUNT; index++) {
        if (HP_button_context[index] == handle) {
          HP_button_context[index] = NULL;
          break;
        }
      }
      if (index == MAX_HP_BUTTON_COUNT) {
        status = SL_STATUS_INVALID_HANDLE;
        break;
      }
      intr_no       = index;
      gpio_instance = M4_GPIO_INSTANCE;
      sl_gpio_configure_interrupt(handle->port, handle->pin, intr_no, SL_GPIO_INTERRUPT_DISABLE);
    }
    sl_gpio_driver_unregister(gpio_instance, GPIO_PIN_INTERRUPT, intr_no);
  } while (false);
  return status;
}

uint8_t sl_si91x_button_get_state(uint8_t button_number)
{
  // Note: this returns the "soft" state rather than reading the port
  //  so it gives with the interrupts and their callbacks
  return buttonState[button_number];
}

void sl_si91x_button_set_state(uint8_t button_number, int8_t state)
{
  // Note: this sets the "soft" state
  //  so it gives with the interrupts and their callbacks
  if (button_number < SL_SI91x_MAX_BUTTON_COUNT) {
    buttonState[button_number] = state;
  }
}

void sl_si91x_button_toggle_state(uint8_t button_number)
{
  // Note: this toggles the "soft" state
  //  so it gives with the interrupts and their callbacks
  if (button_number < SL_SI91x_MAX_BUTTON_COUNT) {
    buttonState[button_number] = !buttonState[button_number];
  }
}

uint8_t sl_si91x_button_state(uint8_t pin, uint8_t port)
{
  if (port == HP || port == ULP) {
    return sl_gpio_get_pin_input(port, pin);
  } else {
    return sl_si91x_gpio_get_uulp_npss_pin(pin);
  }
}

/***************************************************************************/ /**
*
* @brief DEBOUNCE operation is based upon the theory that when multiple reads in a row
* return the same value, we have passed any debounce created by the mechanical
* action of a button.
*
* Typically, software debounce is disabled by defaulting to a value of '0',
* which will cause the preprocessor to strip out the debounce code and save
* flash space.
*
* @note This is how you can configure the debounce functionality.
*
 ******************************************************************************/
#if (SL_SI91x_BUTTON_COUNT > 0)
/**
 * @brief Internal ISR for button handling with optional debounce.
 *
 * This function handles the button interrupt service routine (ISR). It reads
 * the button state and, if debounce is enabled, ensures that the state is
 * consistent for a specified number of reads before considering it stable.
 * If the button state has changed, it notifies the application via a callback.
 *
 * @param[in] pin The pin number associated with the button.
 */
void sl_si91x_button_internal_isr(const sl_button_t *handle)
{
  int8_t buttonStateNow = 0;
  buttonStateNow        = sl_si91x_button_state(handle->pin, handle->port);

#if defined(SL_SI91X_BUTTON_DEBOUNCE)
  sl_si91x_delay_ms(2); // Introduces a 2ms delay to mitigate button debounce effects.
  if (handle->interrupt_config == SL_GPIO_INTERRUPT_FALLING_EDGE) {
    if (buttonStateNow != sl_si91x_button_get_state(handle->button_number)) {
      if (buttonStateNow == LOW) {
        //state changed, notify app
        sl_si91x_button_isr(handle->pin, BUTTON_PRESSED);
      }
    }
  } else if (handle->interrupt_config == SL_GPIO_INTERRUPT_RISE_EDGE) {
    if (buttonStateNow == sl_si91x_button_get_state(handle->button_number)) {
      if (buttonStateNow == HIGH) {
        //state changed, notify app
        sl_si91x_button_isr(handle->pin, BUTTON_RELEASED);
      }
    }
  } else {
    if (buttonStateNow != sl_si91x_button_get_state(handle->button_number)) {
      //state changed, notify app
      sl_si91x_button_isr(handle->pin, BUTTON_PRESSED);
    } else {
      //state changed, notify app
      sl_si91x_button_isr(handle->pin, BUTTON_RELEASED);
    }
  }
#else
  if (buttonStateNow != sl_si91x_button_get_state(handle->button_number)) {
    //state changed, notify app
    sl_si91x_button_isr(handle->pin, BUTTON_PRESSED);
  } else {
    //state changed, notify app
    sl_si91x_button_isr(handle->pin, BUTTON_RELEASED);
  }
#endif
}

SL_WEAK void sl_si91x_button_isr(uint8_t pin, int8_t state)
{
  (void)pin;
  (void)state;
}
#endif // (SL_SI91x_BUTTON_COUNT > 0)

/*Older version APIs (Not recommended to use)*/

int8_t sl_si91x_button_state_get(uint8_t pin)
{
  // Note: this returns the "soft" state rather than reading the port
  //  so it gives with the interrupts and their callbacks
  if (pin == SL_BUTTON_BTN0_PIN) {
    return buttonState[SL_BUTTON_BTN0_NUMBER];
  } else if (pin == SL_BUTTON_BTN1_PIN) {
    return buttonState[SL_BUTTON_BTN1_NUMBER];
  }
  //if no matching button was found
  return BUTTON_STATE_INVALID;
}

void sl_si91x_button_state_set(uint8_t pin, int8_t state)
{
  // Note: this sets the "soft" state
  //  so it gives with the interrupts and their callbacks
  uint8_t index = 0;

  if (pin == SL_BUTTON_BTN0_PIN) {
    index = SL_BUTTON_BTN0_NUMBER;
  } else if (pin == SL_BUTTON_BTN1_PIN) {
    index = SL_BUTTON_BTN1_NUMBER;
  }

  if (index < SL_SI91x_MAX_BUTTON_COUNT) {
    buttonState[index] = state;
  }
}

void sl_si91x_button_state_toggle(uint8_t pin)
{
  // Note: this toggles the "soft" state
  //  so it gives with the interrupts and their callbacks
  uint8_t index = 0;

  if (pin == SL_BUTTON_BTN0_PIN) {
    index = SL_BUTTON_BTN0_NUMBER;
  } else if (pin == SL_BUTTON_BTN1_PIN) {
    index = SL_BUTTON_BTN1_NUMBER;
  }

  if (index < SL_SI91x_MAX_BUTTON_COUNT) {
    buttonState[index] = !buttonState[index];
  }
}

int8_t sl_si91x_button_pin_state(uint8_t pin)
{
  if (pin == SL_BUTTON_BTN1_PIN)
    return RSI_EGPIO_GetPin(EGPIO, SL_BUTTON_BTN1_PORT, pin) ? BUTTON_RELEASED : BUTTON_PRESSED;
  else
    return RSI_NPSSGPIO_GetPin(pin) ? BUTTON_RELEASED : BUTTON_PRESSED;
}