/***************************************************************************/ /**
 * @file sl_si91x_power_manager_config_2.h
 * @brief Power Manager Configuration NWP Medium and MCU Medium RAM Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_POWER_MANAGER_CONFIG_2_H
#define SL_SI91X_POWER_MANAGER_CONFIG_2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_power_manager.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Peripheral Configuration

// <h>High Power Peripherals
// <q SL_HP_M4_DEBUG_PERIPHERAL> M4 DEBUG
// <i> Default: 1
#define SL_HP_M4_DEBUG_PERIPHERAL 1

// <q SL_HP_QSPI_PERIPHERAL> QSPI
// <i> Default: 1
#define SL_HP_QSPI_PERIPHERAL 1

// <q SL_HP_RPDMA_PERIPHERAL> RPDMA
// <i> Default: 1
#define SL_HP_RPDMA_PERIPHERAL 1

// <q SL_HP_SDIO_SPI_PERIPHERAL> SDIO SPI
// <i> Default: 1
#define SL_HP_SDIO_SPI_PERIPHERAL 1
// </h>

// <h>Low Power Peripherals
// <q SL_ULP_AUX_PERIPHERAL> ULP AUX
// <i> Default: 1
#define SL_ULP_AUX_PERIPHERAL 1

// <q SL_ULP_FIM_PERIPHERAL> ULP FIM
// <i> Default: 1
#define SL_ULP_FIM_PERIPHERAL 1

// <q SL_ULP_I2C_PERIPHERAL> ULP I2C
// <i> Default: 1
#define SL_ULP_I2C_PERIPHERAL 1

// <q SL_ULP_I2S_PERIPHERAL> ULP I2S
// <i> Default: 1
#define SL_ULP_I2S_PERIPHERAL 1

// <q SL_ULP_IR_PERIPHERAL> ULP IR
// <i> Default: 1
#define SL_ULP_IR_PERIPHERAL 1

// <q SL_ULP_SSI_PERIPHERAL> ULP SSI
// <i> Default: 1
#define SL_ULP_SSI_PERIPHERAL 1

// <q SL_ULP_UART_PERIPHERAL> ULP UART
// <i> Default: 1
#define SL_ULP_UART_PERIPHERAL 1

// <q SL_ULP_UDMA_PERIPHERAL> ULP UDMA
// <i> Default: 1
#define SL_ULP_UDMA_PERIPHERAL 1
// </h>

// <h>Ultra Low Power Peripherals
// <q SL_UULP_MCU_STORE1_PERIPHERAL> MCU STORE 1
// <i> Default: 1
#define SL_UULP_MCU_STORE1_PERIPHERAL 1

// <q SL_UULP_MCU_STORE2_PERIPHERAL> MCU STORE 2
// <i> Default: 1
#define SL_UULP_MCU_STORE2_PERIPHERAL 1

// <q SL_UULP_MCU_STORE3_PERIPHERAL> MCU STORE 3
// <i> Default: 1
#define SL_UULP_MCU_STORE3_PERIPHERAL 1

// <q SL_UULP_RTC_PERIPHERAL> RTC (CALENDAR)
// <i> Default: 1
#define SL_UULP_RTC_PERIPHERAL 1

// <q SL_UULP_TIME_PERIOD_PERIPHERAL> TIME PERIOD
// <i> Default: 1
#define SL_UULP_TIME_PERIOD_PERIPHERAL 1

// <q SL_UULP_UPS_PERIPHERAL> UPS
// <i> Default: 1
#define SL_UULP_UPS_PERIPHERAL 1

// <q SL_UULP_UTS_PERIPHERAL> UTS
// <i> Default: 1
#define SL_UULP_UTS_PERIPHERAL 1

// <q SL_UULP_WDT_PERIPHERAL> WATCHDOG TIMER
// <i> Default: 1
#define SL_UULP_WDT_PERIPHERAL 1
// </h>

// </h>

// <h>RAM Configuration

// <e>Configure RAM Banks using Size
#define SL_RAM_RETENTION_USING_SIZE 1

#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_1 4
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_2 8
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_3 12
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_4 16
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_5 32
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_6 64
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_7 128
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_8 192
#define SL_LP_RAM_RETENTION_USING_SIZE_BANK_9 256
// <o SL_LP_RAM_RETENTION_SIZE> Low Power Ram Retention Size (in KB)
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_1=> 4 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_2=> 8 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_3=> 12 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_4=> 16 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_5=> 32 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_6=> 64 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_7=> 128 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_8=> 192 KB
//   <SL_LP_RAM_RETENTION_USING_SIZE_BANK_9=> 256 KB
// <i> Selection of LP RAM Retention Size
#define SL_LP_RAM_RETENTION_SIZE SL_LP_RAM_RETENTION_USING_SIZE_BANK_9

#define SL_ULP_RAM_RETENTION_USING_SIZE_BANK_1 2
#define SL_ULP_RAM_RETENTION_USING_SIZE_BANK_2 4
// <o SL_ULP_RAM_RETENTION_SIZE> Ultra Low Power Ram Retention Size (in KB)
//   <SL_ULP_RAM_RETENTION_USING_SIZE_BANK_1=> 2 KB
//   <SL_ULP_RAM_RETENTION_USING_SIZE_BANK_2=> 4 KB
// <i> Selection of ULP RAM Retention Size
#define SL_ULP_RAM_RETENTION_SIZE SL_ULP_RAM_RETENTION_USING_SIZE_BANK_2
// </e>

// <e>Configure RAM Banks using Bank Number
#define SL_RAM_RETENTION_USING_NUMBER 0
// <q SL_LP_RAM_BANK1> LOW POWER RAM BANK 1
// <i> Default: 1
#define SL_LP_RAM_BANK1 1

// <q SL_LP_RAM_BANK2> LOW POWER RAM BANK 2
// <i> Default: 1
#define SL_LP_RAM_BANK2 1

// <q SL_LP_RAM_BANK3> LOW POWER RAM BANK 3
// <i> Default: 1
#define SL_LP_RAM_BANK3 1

// <q SL_LP_RAM_BANK4> LOW POWER RAM BANK 4
// <i> Default: 1
#define SL_LP_RAM_BANK4 1

// <q SL_LP_RAM_BANK5> LOW POWER RAM BANK 5
// <i> Default: 1
#define SL_LP_RAM_BANK5 1

// <q SL_LP_RAM_BANK6> LOW POWER RAM BANK 6
// <i> Default: 1
#define SL_LP_RAM_BANK6 1

// <q SL_LP_RAM_BANK7> LOW POWER RAM BANK 7
// <i> Default: 1
#define SL_LP_RAM_BANK7 1

// <q SL_LP_RAM_BANK8> LOW POWER RAM BANK 8
// <i> Default: 1
#define SL_LP_RAM_BANK8 1

// <q SL_LP_RAM_BANK9> LOW POWER RAM BANK 9
// <i> Default: 1
#define SL_LP_RAM_BANK9 1

// <q SL_ULP_RAM_BANK2> ULTRA LOW POWER RAM BANK 2
// <i> Default: 1
#define SL_ULP_RAM_BANK2 1

// <q SL_ULP_RAM_BANK3> ULTRA LOW POWER RAM BANK 3
// <i> Default: 1
#define SL_ULP_RAM_BANK3 1
// </e>

// </h>

// <<< end of configuration section >>>

sl_power_peripheral_t peripheral_config = {
  .m4ss_peripheral = (0
#if (SL_HP_M4_DEBUG_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_M4SS_PG_M4_DEBUG
#endif
#if (SL_HP_RPDMA_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_M4SS_PG_RPDMA
#endif
#if (SL_HP_QSPI_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_M4SS_PG_QSPI
#endif
#if (SL_HP_SDIO_SPI_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_M4SS_PG_SDIO_SPI
#endif
                      ),
  .ulpss_peripheral = (0
#if (SL_ULP_AUX_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX
#endif
#if (SL_ULP_FIM_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM
#endif
#if (SL_ULP_I2C_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
#endif
#if (SL_ULP_I2S_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S
#endif
#if (SL_ULP_IR_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR
#endif
#if (SL_ULP_SSI_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
#endif
#if (SL_ULP_UART_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_UART
#endif
#if (SL_ULP_UDMA_PERIPHERAL == 0)
                       | SL_SI91X_POWER_MANAGER_ULPSS_PG_UDMA
#endif
                       ),
  .npss_peripheral = (0
#if (SL_UULP_MCU_STORE1_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE1
#endif
#if (SL_UULP_MCU_STORE2_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2
#endif
#if (SL_UULP_MCU_STORE3_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3
#endif
#if (SL_UULP_RTC_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_MCURTC
#endif
#if (SL_UULP_TIME_PERIOD_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_TIMEPERIOD
#endif
#if (SL_UULP_UPS_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS
#endif
#if (SL_UULP_UTS_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS
#endif
#if (SL_UULP_WDT_PERIPHERAL == 0)
                      | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT
#endif
                      ),
};

sl_power_ram_retention_config_t ram_configuration = {
#if (SL_RAM_RETENTION_USING_SIZE == 1)
  .configure_ram_banks = false,
  .m4ss_ram_size_kb    = SL_LP_RAM_RETENTION_SIZE,
  .ulpss_ram_size_kb   = SL_ULP_RAM_RETENTION_SIZE,
#else
  .configure_ram_banks = true,
  .m4ss_ram_banks      = (0
#if (SL_LP_RAM_BANK1 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_1
#endif
#if (SL_LP_RAM_BANK2 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_2
#endif
#if (SL_LP_RAM_BANK3 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_3
#endif
#if (SL_LP_RAM_BANK3 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_3
#endif
#if (SL_LP_RAM_BANK4 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_4
#endif
#if (SL_LP_RAM_BANK5 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_5
#endif
#if (SL_LP_RAM_BANK6 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_6
#endif
#if (SL_LP_RAM_BANK7 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_7
#endif
#if (SL_LP_RAM_BANK8 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8
#endif
#if (SL_LP_RAM_BANK9 == 0)
                     | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
#endif
                     ),
  .ulpss_ram_banks     = (0
#if (SL_ULP_RAM_BANK2 == 0)
                      | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2
#endif
#if (SL_ULP_RAM_BANK3 == 0)
                      | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3
#endif
                      ),
#endif
};

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_POWER_MANAGER_CONFIG_2_H