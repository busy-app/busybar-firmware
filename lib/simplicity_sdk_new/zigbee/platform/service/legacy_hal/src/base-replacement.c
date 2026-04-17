/***************************************************************************//**
 * @file base-replacement.c
 * @brief Legacy HAL
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories, Inc, www.silabs.com</b>
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

#include PLATFORM_HEADER

#include <stdio.h>
#include "hal.h"
#include "sl_rail_util_compatible_pa.h"
#include "sl_sleeptimer.h"
#include "sl_power_manager.h"
#include "rail.h"
#include "sl_status.h"
#include "sl_component_catalog.h"

#if defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#endif

#if defined(SL_CATALOG_IOSTREAM_VUART_PRESENT)
#include "sl_iostream_vuart.h"
#endif

#if defined(SL_CATALOG_LED_PRESENT)
#include "sl_led.h"
#endif

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "sl_button.h"
#include "sl_simple_button.h"
#include "sl_simple_button_config.h"
#include "sl_simple_button_instances.h"
#endif

#if defined(SL_CATALOG_LEGACY_HAL_WDOG_PRESENT)
#include "sl_legacy_hal_wdog_config.h"
#endif

// System timer / sleeptimer
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
uint16_t halCommonGetInt16uMillisecondTick(void)
{
  return (uint16_t)halCommonGetInt64uMillisecondTick();
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t halCommonGetInt32uMillisecondTick(void)
{
  return (uint32_t)halCommonGetInt64uMillisecondTick();
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
uint64_t halCommonGetInt64uMillisecondTick(void)
{
  uint64_t ms;

  assert(sl_sleeptimer_tick64_to_ms(sl_sleeptimer_get_tick_count64(),
                                    &ms) == SL_STATUS_OK);
  return ms;
}

uint16_t halCommonGetInt16uQuarterSecondTick(void)
{
  uint32_t ticks = halCommonGetInt32uMillisecondTick();
  ticks /= MILLISECOND_TICKS_PER_QUARTERSECOND;
  return (uint16_t)ticks;
}

sl_power_manager_on_isr_exit_t sl_legacy_hal_sleep_on_isr_exit(void)
{
  return SL_POWER_MANAGER_WAKEUP;
}

void halCommonDelayMicroseconds(uint16_t us)
{
  // Note: If you find that your software is hanging in this spot, then
  // You most likely haven't run RAIL_Init() yet. If you really need
  // us precision timings before RAIL_Init() is run, consider using
  // the ustimer component instead of halCommonDelayMicroseconds().

  RAIL_DelayUs(us);
}

void halCommonDelayMilliseconds(uint16_t ms)
{
  sl_sleeptimer_delay_millisecond(ms);
}

uint16_t halInternalStartSystemTimer(void)
{
  if (sl_sleeptimer_init() != SL_STATUS_OK) {
    assert(0);
  }
  return 0;
}

void halInit(void)
{
  halInternalClassifyReset();

  // To be able to give more details on errors, we want faults enabled so
  // they're not all forced into hard faults.
  SCB->SHCSR |= (SCB_SHCSR_BUSFAULTENA_Msk
                 | SCB_SHCSR_MEMFAULTENA_Msk
#ifdef SCB_SHCSR_SECUREFAULTENA_Msk
                 | SCB_SHCSR_SECUREFAULTENA_Msk
#endif
                 | SCB_SHCSR_USGFAULTENA_Msk);

  //Fill the unused portion of the memory reserved for the stack.
  //memset() is not being used to do this in case it uses the stack
  //to store the return address.
  volatile uint32_t *dataDestination;
  //This code assumes that the __get_MSP() return value and
  //_CSTACK_SEGMENT_BEGIN are both 32-bit aligned values.
  dataDestination = (uint32_t*) (__get_MSP() - 4U);
  //Start at current stack ptr fill up until CSTACK_SEGMENT_BEGIN
  while (dataDestination >= _CSTACK_SEGMENT_BEGIN) {
    //Fill with magic value interpreted by C-SPY's Stack View
    *dataDestination-- = STACK_FILL_VALUE;
  }

  sl_power_manager_em4_unlatch_pin_retention();

#if ((SL_LEGACY_HAL_ENABLE_WATCHDOG == 1) && (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0))
  halInternalEnableWatchDog();
#endif // ((SL_LEGACY_HAL_ENABLE_WATCHDOG == 1) && (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0))

  halInternalStartSystemTimer();

  sl_rail_util_pa_init();
}

void halReboot(void)
{
  halInternalSysReset(RESET_SOFTWARE_REBOOT);
}

#if !defined(BOARD_ACTIVITY_LED)
  #if defined(SL_CATALOG_BOARD_ACTIVITY_PRESENT)
    #define BOARD_ACTIVITY_LED  sl_led_board_activity
  #elif defined(SL_CATALOG_LED0_PRESENT)
    #define BOARD_ACTIVITY_LED  sl_led_led0
  #endif
#endif

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
void halStackIndicateActivity(bool turnOn)
{
  (void)turnOn;

#if defined(SL_CATALOG_LED_PRESENT) && defined(BOARD_ACTIVITY_LED)
  if (turnOn) {
    sl_led_turn_on(&BOARD_ACTIVITY_LED);
  } else {
    sl_led_turn_off(&BOARD_ACTIVITY_LED);
  }
#endif
}

void halStackProcessBootCount(void)
{
#if defined(COMMON_TOKEN_STACK_BOOT_COUNTER)
  (void)sl_token_manager_increment_counter(COMMON_TOKEN_STACK_BOOT_COUNTER);
#endif
}

sl_status_t halCommonIdleForMilliseconds(uint32_t *duration)
{
  (void)duration;

  // Sleeping is now handled by the Power Manager. Closest we can get to the old
  // behavior is to tell the caller that sleep was interrupted.
  return SL_STATUS_SUSPENDED;
}

void halStackSymbolDelayAIsr(void)
{
  // Intentionally empty.
  // This function is defined here only to provide compatibility for code that
  // may also be built for legacy parts.
}

sl_status_t sli_util_debug_init(void)
{
#if defined(SL_CATALOG_IOSTREAM_VUART_PRESENT)
  sl_status_t status = sl_iostream_vuart_init();

  if (status == SL_STATUS_OK) {
    return SL_STATUS_OK;
  }
#endif

  return SL_STATUS_FAIL;
}

void sli_802154phy_radio_seed_random(void)
{
  uint32_t seed;
  uint16_t bytes = RAIL_GetRadioEntropy(NULL, (uint8_t *)(&seed), sizeof(seed));
  assert(bytes == sizeof(seed));
  halStackSeedRandom(seed);
}

//------------------------------------------------------------------------------

sl_status_t sli_legacy_serial_init(uint8_t port,
                                   SerialBaudRate rate,
                                   SerialParity parity,
                                   uint8_t stopBits)
{
  (void) port;
  (void) rate;
  (void) parity;
  (void) stopBits;
  return SL_STATUS_OK;
}

sl_status_t sli_legacy_serial_wait_send(uint8_t port)
{
  (void) port;
  return SL_STATUS_OK;
}

#ifdef SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
const sl_iostream_instance_info_t * sli_legacy_hal_get_iostream_info(uint8_t port)
{
  sl_iostream_type_t type = SL_IOSTREAM_TYPE_UNDEFINED;
  uint8_t periph = (uint8_t)(-1);

  switch (port) {
    case COM_VCP:
    case comPortVcp:
      type = SL_IOSTREAM_TYPE_VUART;
      periph = 0;
      break;
    case COM_USART0:
    case comPortUsart0:
      type = SL_IOSTREAM_TYPE_UART;
      periph = 0;
      break;
    case COM_USART1:
    case comPortUsart1:
      type = SL_IOSTREAM_TYPE_UART;
      periph = 1;
      break;
    case COM_USART2:
    case comPortUsart2:
      type = SL_IOSTREAM_TYPE_UART;
      periph = 2;
      break;
    case comPortUsart3:
      type = SL_IOSTREAM_TYPE_UART;
      periph = 3;
      break;
    default:
      return NULL;
  }

  for (uint8_t i = 0; i < sl_iostream_instances_count; i++) {
    if (sl_iostream_instances_info[i] != NULL
        && sl_iostream_instances_info[i]->type == type
        && sl_iostream_instances_info[i]->periph_id == periph) {
      return sl_iostream_instances_info[i];
    }
  }

  return NULL;
}
#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT

sl_status_t sli_legacy_serial_read_byte(uint8_t port, uint8_t *dataByte)
{
  (void)port;
  (void)dataByte;

#ifdef SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
  const sl_iostream_instance_info_t * info;
  info = sli_legacy_hal_get_iostream_info(port);

  if (info != NULL && info->handle != NULL) {
    sl_status_t status = sl_iostream_getchar(info->handle, (char *)dataByte);

    if (status == SL_STATUS_OK) {
      return SL_STATUS_OK;
    } else if (status == SL_STATUS_EMPTY) {
      return SL_STATUS_EMPTY;
    }
  }
#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT

  return SL_STATUS_FAIL;
}

sl_status_t sl_legacy_serial_read_data(uint8_t port,
                                       uint8_t *data,
                                       uint16_t length,
                                       uint16_t *bytesRead)
{
  (void)port;
  (void)data;
  (void)length;
  (void)bytesRead;

#ifdef SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
  const sl_iostream_instance_info_t * info;
  info = sli_legacy_hal_get_iostream_info(port);

  if (info == NULL || info->handle == NULL) {
    return SL_STATUS_FAIL;
  }

  uint16_t bytesReadInternal = 0;
  sl_status_t status;

  while (bytesReadInternal < length) {
    status = sl_iostream_getchar(info->handle, (char *)data);
    switch (status) {
      case SL_STATUS_OK:
        ++data;
        ++bytesReadInternal;
        break;

      case SL_STATUS_EMPTY:
        // empty queue is not an error for us, we just want to keep waiting
        break;

      default:
        // only store number of bytes read if the caller provided a non-NULL pointer
        if (bytesRead) {
          *bytesRead = bytesReadInternal;
        }
        return SL_STATUS_FAIL;
    }
  }

  // only store number of bytes read if the caller provided a non-NULL pointer
  if (bytesRead) {
    *bytesRead = bytesReadInternal;
  }
  return SL_STATUS_OK;
#else // !SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT

  return SL_STATUS_FAIL;
#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
}

sl_status_t sli_legacy_serial_write_byte(uint8_t port, uint8_t dataByte)
{
  (void)port;
  (void)dataByte;

#ifdef SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
  const sl_iostream_instance_info_t * info;
  info = sli_legacy_hal_get_iostream_info(port);

  if (info != NULL && info->handle != NULL) {
    sl_status_t status = sl_iostream_putchar(info->handle, (char)dataByte);

    if (status == SL_STATUS_OK) {
      return SL_STATUS_OK;
    }
  }
#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT

  return SL_STATUS_FAIL;
}

sl_status_t sli_legacy_serial_guaranteed_printf(uint8_t port,
                                                const char * formatString, ...)
{
  (void)port;
  (void)formatString;

#ifdef SL_CATALOG_LEGACY_PRINTF_PRESENT
  sl_status_t status;
  va_list ap;
  va_start(ap, formatString);
  status = sli_legacy_serial_printf_var_arg(port, formatString, ap);
  va_end(ap);
  printf("\n");
  return status;
#else // !SL_CATALOG_LEGACY_PRINTF_PRESENT

  return SL_STATUS_FAIL;
#endif // SL_CATALOG_LEGACY_PRINTF_PRESENT
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
  #define BUTTON_COUNT (SL_SIMPLE_BUTTON_COUNT)
#else
  #define BUTTON_COUNT (0U)
#endif

// Make sure our data type is large enough to handle all buttons
#if BUTTON_COUNT > 256U
  #error Legacy HAL can not accomodate more than 256 buttons!
#endif

#if (BUTTON_COUNT > 0) && LEGACY_HAL_TRANSLATE_BUTTON_INTERRUPT

SL_WEAK void sl_button_on_change(const sl_button_t *handle)
{
  sl_button_state_t sl_state = sl_button_get_state(handle);

  assert(sl_state != (sl_button_state_t)BUTTON_ERROR
         && sl_state != SL_SIMPLE_BUTTON_DISABLED);

  uint8_t em_state = (sl_state == SL_SIMPLE_BUTTON_PRESSED)
                     ? BUTTON_PRESSED
                     : BUTTON_RELEASED;

  int button;

  for (button = 0; button < BUTTON_COUNT; button++) {
    if (handle == SL_SIMPLE_BUTTON_INSTANCE(button)) {
      halButtonIsr(button, em_state);
      break;
    }
  }

  assert(button < BUTTON_COUNT);
}

SL_WEAK void halButtonIsr(uint8_t button, uint8_t state)
{
  // Weak stub implementation that can be overridden by the application
  // Will only be called for changes in buttons configured for interrupt mode
  (void) button;
  (void) state;
}

#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT + LEGACY_HAL_TRANSLATE_BUTTON_INTERRUPT
