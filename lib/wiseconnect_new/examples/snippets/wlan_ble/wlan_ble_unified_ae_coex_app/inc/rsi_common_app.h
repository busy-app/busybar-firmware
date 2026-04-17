
/*
 * rsi_common_app.h
 *
 *  Created on: 03-Nov-2018
 *     
 */

#ifndef RSI_COMMON_APP_H_
#define RSI_COMMON_APP_H_

#include "stdint.h"
#include "stdio.h"
//#include "fsl_debug_console.h"

int32_t ble_ae_set_1_advertising_enable(void);
void sl_btc_ble_adv_extended_register_callbacks_wrapper();
int32_t rsi_ble_dual_role(void);
int32_t ble_ext_scan_enable(void);

//For RT595 boards, bydefault 1.8v is set, enable ENABLE_1P8V macro if RT595 board is using

#define ENABLE_1P8V 0 //! Disable this when using 3.3v

#define RSI_DEBUG_EN 0

//! To enable WLAN task
#define WLAN_TASK_ENABLE 0

/*=======================================================================*/
//!    Powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 1 //! Set to 1 for powersave mode

/*=======================================================================*/

//#define LOG_PRINT PRINTF
#if RSI_DEBUG_EN
#define LOG_PRINT_D(...)                                  \
  {                                                       \
    osMutexAcquire(rsi_driver_cb->debug_prints_mutex, 0); \
    printf(__VA_ARGS__);                                  \
    osMutexRelease(rsi_driver_cb->debug_prints_mutex);    \
  }
#else
#define LOG_PRINT_D(...) \
  {                      \
  }
#endif

#endif
