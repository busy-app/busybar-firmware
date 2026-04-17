/******************************************************************************
* @file sl_si91x_i2c.c
* @brief I2C API implementation
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
#include "sl_si91x_peripheral_i2c.h"
#include "rsi_power_save.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_power_save.h"
#include <math.h>
/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/

#define I2C_RELEASE_VERSION       0       // I2C Release version
#define I2C_SQA_VERSION           0       // I2C SQA version
#define I2C_DEV_VERSION           2       // I2C Developer version
#define IC_SS_SPKLENGTH           7       // Spike Length - same for SS, FS
#define IC_FS_SPKLENGTH           1       // Spike Length for FPS
#define IC_HS_SPKLENGTH           1       // Spike Length for HS
#define SS_DESIRED_SCL_FREQ       100     // 100 kHz
#define FS_DESIRED_SCL_FREQ       400     // 400 kHz
#define SS_MIN_SCL_HIGHtime       5200    // 5200 ns for 100 kbps
#define FS_MIN_SCL_HIGHtime       1160    // 1160 ns for 400 kbps
#define FPS_MIN_SCL_HIGHtime      500     // 500 ns for 1000 kbps
#define HS_MIN_SCL_HIGHtime_100PF 132     // 132 ns for 3.4 Mbps, bus loading = 100pF
#define HS_MIN_SCL_HIGHtime_400PF 274     // 274 ns for 3.4 Mbps, bus loading = 400pF
#define SS_MIN_SCL_LOWtime        4700    // 4700 ns for 100 kbps
#define FS_MIN_SCL_LOWtime        1330    // 1330 ns for 400 kbps
#define FPS_MIN_SCL_LOWtime       500     // 500 ns for 1000 kbps
#define HS_MIN_SCL_LOWtime_100PF  161     // 161 ns for 3.4Mbps, bus loading = 100pF
#define HS_MIN_SCL_LOWtime_400PF  333     // 333 ns for 3.4Mbps, bus loading = 400pF
#define CONVERT_TO_MHZ            1000000 // Mega hertz conversion value
#define CONVERT_TO_NS             1000    // Conversion value of time period to nanosecond
#define TOTAL_COUNT_OFFSET        8       // Constant deduction required from total count in FS and SS
#define HIGHCOUNT_CONSTANT        8       // Constant deduction required from high counts
#define LOWCOUNT_CONSTANT         1       // Constant deduction required from low counts
#define HS_HIGHCOUNT_OFFSET       9       // Offset value for high counts in high speed mode
#define HS_LOWCOUNT_OFFSET        13      // Offset value for low counts in high speed mode

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static void set_i2c_clock_rate(I2C_TypeDef *i2c, uint8_t speed, uint32_t i2c_Clk);
static uint8_t get_fast_plus_offset_value(uint32_t clock);

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/
/*******************************************************************************
 * It initialized the I2C peripheral with the provided parameters.
 * This is the first function which need to be called to configure I2C parameters.
 * The actions performed in init function are: 
 *  - I2C disable
 *  - Reset the I2C peripheral
 *  - Mode Selection (Leader / Follower)
 *  - Set follower address / follower mask address(when master acts as a slave)
 *  - Set the bus frequency
 ******************************************************************************/
void sl_si91x_i2c_init(I2C_TypeDef *i2c, const sl_i2c_init_params_t *p_config)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Validates the parameters structure.
  SL_I2C_ASSERT(p_config);
  // Disables the I2C.
  sl_si91x_i2c_disable(i2c);
  // Resetting the peripheral.
  sl_si91x_i2c_reset(i2c);
  // Validating mode of the I2C received in parameter structure.
  if (p_config->mode == SL_I2C_LEADER_MODE) {
    // Enabling leader mode register.
    i2c->IC_CON_b.MASTER_MODE      = ENABLE;
    i2c->IC_CON_b.IC_SLAVE_DISABLE = ENABLE;
  } else if (p_config->mode == SL_I2C_FOLLOWER_MODE) {
    // Setting slave mode
    i2c->IC_CON_b.MASTER_MODE      = DISABLE;
    i2c->IC_CON_b.IC_SLAVE_DISABLE = DISABLE;
  }
  // Restart is enabled after every byte is transferred.
  i2c->IC_CON_b.IC_RESTART_EN = ENABLE;
  // Transmit empty interrupt is cleared.
  sl_si91x_i2c_clear_interrupts(i2c, SL_I2C_EVENT_TRANSMIT_EMPTY);
  // All pending IRQs are cleared.
  if ((uint32_t)i2c == I2C0_BASE) {
    // Clears IRQ for instance 0
    NVIC_ClearPendingIRQ(I2C0_IRQn);
  } else if ((uint32_t)i2c == I2C1_BASE) {
    // Clears IRQ for instance 1
    NVIC_ClearPendingIRQ(I2C1_IRQn);
  } else if ((uint32_t)i2c == I2C2_BASE) {
    // Clears IRQ for instance 2
    NVIC_ClearPendingIRQ(I2C2_IRQn);
  }
  // Bus speed is set according the flag received in parameter structure.
  sl_si91x_i2c_set_frequency(i2c, 0, p_config->freq, p_config->clhr);
}

/*******************************************************************************
 * I2C peripheral is enabled and again disabled.
 * Contorl register and interrupt mask registers are cleared here.
 * Interrupts are also disabled.
 * Target address and slave address are updated with 0.
 ******************************************************************************/
void sl_si91x_i2c_reset(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  sl_si91x_i2c_enable(i2c);
  sl_si91x_i2c_disable(i2c);
  // Control register is cleared which includes mode, speed.
  i2c->IC_CON = SL_I2C_CLEAR_MASK;
  // All the interrupts are masked.
  i2c->IC_INTR_MASK = DISABLE;
  i2c->IC_TAR       = SL_I2C_CLEAR_MASK;
  i2c->IC_SAR       = SL_I2C_CLEAR_MASK;
  // Interrupts are disabled.
  sl_si91x_i2c_disable_interrupts(i2c, 0);
}

/*******************************************************************************
 * It loads one byte data in the DATA register which is TX FIFO.
 * This is used to transmit only one byte at a time.
 ******************************************************************************/
void sl_si91x_i2c_tx(I2C_TypeDef *i2c, uint8_t data)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Data is filled in the FIFO.
  i2c->IC_DATA_CMD_b.DAT = data;
}

/*******************************************************************************
 * It reads one byte data in the DATA register which is RX FIFO.
 * This is used to receive only one byte at a time.
 ******************************************************************************/
uint8_t sl_si91x_i2c_rx(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Data is read from the FIFO.
  return i2c->IC_DATA_CMD_b.DAT;
}

/*******************************************************************************
 * To confiure the frequency of the I2C transmission.
 * According the input of chlr enum, the register is updated.
 * After setting register, set_i2c_clock_rate function is called @ref set_i2c_clock_rate.
 * @note: For fast speed and fast plus speed, the value of register will be same.
 ******************************************************************************/
void sl_si91x_i2c_set_frequency(I2C_TypeDef *i2c, uint32_t ref_freq, uint32_t freq_scl, sl_i2c_clock_hlr_t clhr)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  UNUSED_VARIABLE(ref_freq);

  uint32_t clock;
  clock = freq_scl / CONVERT_TO_MHZ;

  switch (clhr) {
    case SL_I2C_STANDARD_BUS_SPEED: // Standard Speed (100kHz)
      i2c->IC_CON_b.SPEED = SL_I2C_STANDARD_BUS_SPEED;
      break;

    case SL_I2C_FAST_BUS_SPEED:      // Fast Speed (400kHz)
    case SL_I2C_FAST_PLUS_BUS_SPEED: // Fast Plus Speed (1MHz)
      i2c->IC_CON_b.SPEED = SL_I2C_FAST_BUS_SPEED;
      break;

    case SL_I2C_HIGH_BUS_SPEED: // High Speed (3.4MHz)
      i2c->IC_CON_b.SPEED = SL_I2C_HIGH_BUS_SPEED;
      break;

    default:
      break;
  }

  // Setting the high-low count registers
  set_i2c_clock_rate(i2c, (uint8_t)clhr, clock);
}

/*******************************************************************************
 * It returns the system core clock frequency as a uint32_t variable.
 ******************************************************************************/
uint32_t sl_si91x_i2c_get_frequency(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Returns the system core clock in MHz.
  return SystemCoreClock;
}

/*******************************************************************************
 * It configures the low count and high count registers according to input.
 * The calculation is performed using the clock speed and values are updated.
 * @note: Fast plus and Fast mode has same register set, only the value varies.
 ******************************************************************************/
static void set_i2c_clock_rate(I2C_TypeDef *i2c, uint8_t speed, uint32_t i2c_Clk)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));

  uint16_t ss_scl_high = 0, ss_scl_low = 0;
  uint16_t fs_scl_high = 0, fs_scl_low = 0;
  uint16_t fps_scl_high = 0, fps_scl_low = 0;
  uint16_t hs_scl_high = 0, hs_scl_low = 0;
  uint32_t total_scl_cnt = 0;

  switch (speed) {
    case SL_I2C_STANDARD_BUS_SPEED:
      total_scl_cnt = ((i2c_Clk * CONVERT_TO_NS) / SS_DESIRED_SCL_FREQ) - IC_SS_SPKLENGTH - TOTAL_COUNT_OFFSET;

      ss_scl_low  = (SS_MIN_SCL_LOWtime * i2c_Clk) / CONVERT_TO_NS;
      ss_scl_high = total_scl_cnt - ss_scl_low;
      if (total_scl_cnt > ss_scl_low) {
        ss_scl_high = total_scl_cnt - ss_scl_low;
      } else {
        ss_scl_high = (SS_MIN_SCL_HIGHtime * i2c_Clk) / CONVERT_TO_NS;
      }

      // Calculated values are update in the corresponding registers.
      i2c->IC_SS_SCL_LCNT_b.IC_SS_SCL_LCNT = ss_scl_low;
      i2c->IC_SS_SCL_HCNT_b.IC_SS_SCL_HCNT = ss_scl_high;
      break;

    case SL_I2C_FAST_BUS_SPEED:
      total_scl_cnt = ((i2c_Clk * CONVERT_TO_NS) / FS_DESIRED_SCL_FREQ) - IC_SS_SPKLENGTH - TOTAL_COUNT_OFFSET;

      fs_scl_low = (FS_MIN_SCL_LOWtime * i2c_Clk) / CONVERT_TO_NS;
      if (total_scl_cnt > fs_scl_low) {
        fs_scl_high = total_scl_cnt - fs_scl_low;
      } else {
        fs_scl_high = (FS_MIN_SCL_HIGHtime * i2c_Clk) / CONVERT_TO_NS;
      }
      // Calculated values are update in the corresponding registers.
      i2c->IC_FS_SCL_LCNT_b.IC_FS_SCL_LCNT = fs_scl_low;
      i2c->IC_FS_SCL_HCNT_b.IC_FS_SCL_HCNT = fs_scl_high;
      break;

    case SL_I2C_FAST_PLUS_BUS_SPEED:
      i2c->IC_FS_SPKLEN_b.IC_FS_SPKLEN = IC_FS_SPKLENGTH;

      uint8_t offset_value = get_fast_plus_offset_value(i2c_Clk);
      fps_scl_low          = ((FPS_MIN_SCL_LOWtime * i2c_Clk) / CONVERT_TO_NS) - LOWCOUNT_CONSTANT;
      fps_scl_high         = ((FPS_MIN_SCL_HIGHtime * i2c_Clk) / CONVERT_TO_NS) - HIGHCOUNT_CONSTANT;
      // Calculated values are update in the corresponding registers.
      i2c->IC_FS_SCL_LCNT_b.IC_FS_SCL_LCNT = fps_scl_low - offset_value;
      i2c->IC_FS_SCL_HCNT_b.IC_FS_SCL_HCNT = fps_scl_high - offset_value;
      break;

    case SL_I2C_HIGH_BUS_SPEED:
      i2c->IC_HS_SPKLEN_b.IC_HS_SPKLEN = IC_HS_SPKLENGTH;

      hs_scl_low  = ((HS_MIN_SCL_LOWtime_100PF * i2c_Clk) / CONVERT_TO_NS) - LOWCOUNT_CONSTANT;
      hs_scl_high = ((HS_MIN_SCL_HIGHtime_100PF * i2c_Clk) / CONVERT_TO_NS) - HIGHCOUNT_CONSTANT;
      // Calculated values are update in the corresponding registers.
      i2c->IC_HS_SCL_LCNT_b.IC_HS_SCL_LCNT = hs_scl_low - HS_LOWCOUNT_OFFSET;
      i2c->IC_HS_SCL_HCNT_b.IC_HS_SCL_HCNT = hs_scl_high - HS_HIGHCOUNT_OFFSET;
      break;

    default:
      break;
  }
}

/*******************************************************************************
 * It calculates the offset value of high count and low count according to the 
 * input clock frequency.
 ******************************************************************************/
static uint8_t get_fast_plus_offset_value(uint32_t clock)
{
  uint8_t offset = 1;
  if ((clock >= 32) && (clock < 80)) {
    offset = (0.083 * clock) - 1.65;
  } else if (clock >= 80) {
    offset = (clock * 0.05) + 1;
  }
  return offset;
}

/*******************************************************************************
 * To get the release, sqa and dev version of I2C
 * It returns the structure for gspi version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_i2c_version_t sl_si91x_i2c_get_version(void)
{
  sl_i2c_version_t version;
  version.release = I2C_RELEASE_VERSION;
  version.major   = I2C_SQA_VERSION;
  version.minor   = I2C_DEV_VERSION;
  return version;
}
