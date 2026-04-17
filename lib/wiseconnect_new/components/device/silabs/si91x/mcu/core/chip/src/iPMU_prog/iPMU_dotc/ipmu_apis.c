#ifdef SLI_SI91X_MCU_ENABLE_IPMU_APIS
/******************************************************************************
* @file  ipmu_apis.c
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
#ifdef SLI_SI91X_MCU_INTERFACE
#include <stdint.h>
#include "rsi_ipmu.h"
#include "rsi_system_config.h"
#define IPMU_DOTC_PROG
#define IPMU_CALIB_DATA
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint8_t uint8;
#define cmemcpy memcpy
#endif
#ifndef SLI_SI91X_MCU_INTERFACE
#include "data_types.h"
#include "defines.h"
#include "pm.h"
#include "soc_intf.h"
#include "lib_intf.h"
#include "gpio.h"
#include "timer.h"
#include "semaphore.h"
#include "error_code.h"
#include "common_intf.h"
#include "sleep_state.h"
#include "pll.h"
#include "pmu.h"
#include "misc_config.h"
#include "intr.h"
#include "bb_rf_prog_intf.h"
#include "bb_rf_prog.h"
#include "wlan_coex_intf.h"
#include "dev_ps.h"
#include "common_lmac.h"
#include "dbg.h"
#include "soft_timer.h"
#include "bb_rf_prog.h"
#include "dma.h"
#include "dynamic_clock.h"
#include "structures.h"
#include "ps.h"
#include "spi.h"
#include "hwrng.h"
#include "fmc.h"
#ifdef CHIP_9118
#include "pwr_ctrl.h"
#endif
#include "rsi_global_non_rom.h"
#include "obe_low_pwr.h"
#include "common_ram.h"
#include "bb_rf_prog_ram.h"
#include "soc_address_map.h"
#include "ipmu_defines.h"
#include "no_host_intf.h" /* BT_DEV_MODE */
#include "bt/bt.h"
#include "bt/bt_common.h"
#include "wakefi.h"
#include "cortex_m4.h"
#include "bb_rf_calib.h"
#include "wrappers_common.h"
#endif
void program_ipmu_data(const uint32_t *src);
#ifndef BT_LE_ONLY_MODE
#ifdef SLI_SI91X_MCU_INTERFACE
void ipmu_init_mcu(void);
#else
void ipmu_init_mcu(void) __attribute__((section(".common_non_tcm_code")));
#endif
void set_scdc(uint32 Deepsleep);
void ipmu_init_mcu(void)
{
#ifdef IPMU_DOTC_PROG
  program_ipmu_data(ipmu_common_config); //IPMU1
  program_ipmu_data(pmu_common_config);  //PMU1
#ifndef SLI_SI91X_MCU_INTERFACE
  if (HF_RC_CLK_MODE == 0) {
    program_ipmu_data(m20rc_osc_trim_efuse); //M20RC_EFUSE
  } else if (HF_RC_CLK_MODE == 1) {
    program_ipmu_data(m32rc_osc_trim_efuse); //M32RC_EFUSE
  }
  program_ipmu_data(dblr_32m_trim_efuse);  //DBLR_EFUSE
  program_ipmu_data(m20ro_osc_trim_efuse); //M20RO_EFUSE
  program_ipmu_data(ro_32khz_trim_efuse);  //RO32K_EFUSE
  if (RC_CLK_MODE == 0) {
    program_ipmu_data(rc_16khz_trim_efuse); //RC16K_EFUSE
  } else if (RC_CLK_MODE == 1) {
    program_ipmu_data(rc_32khz_trim_efuse); //RC32K_EFUSE
  } else if (RC_CLK_MODE == 2) {
    program_ipmu_data(rc_64khz_trim_efuse); //RC64K_EFUSE
  }
#endif
#ifdef SLI_SI91X_MCU_INTERFACE
  if (HF_RC_CLK_MODE == 0) {
    /* program the trim value for 20Mhz RC oscillator */
    RSI_IPMU_M20rcOsc_TrimEfuse(); //M20RC_EFUSE
  } else if (HF_RC_CLK_MODE == 1) {
    /* program the trim value for 32Mhz RC  oscillator */
    RSI_IPMU_M32rc_OscTrimEfuse(); //M32RC_EFUSE
  }
  /* program the trim value for RC 32MHz doubler  */
  RSI_IPMU_DBLR32M_TrimEfuse();  //DBLR_EFUSE
                                 /* program the trim value for 20MHz RC oscillator */
  RSI_IPMU_M20roOsc_TrimEfuse(); //M20RO_EFUSE
                                 /* program the trim value for 32KHZ RO oscillator */
  RSI_IPMU_RO32khz_TrimEfuse();  //RO32K_EFUSE
  if (RC_CLK_MODE == 0) {
    /* program the trim value for 16KHz RC oscillator */
    RSI_IPMU_RC16khz_TrimEfuse(); //RC16K_EFUSE
  } else if (RC_CLK_MODE == 1) {
    /* program the trim value for 32KHz RC oscillator */
    RSI_IPMU_RC32khz_TrimEfuse(); //RC32K_EFUSE
  } else if (RC_CLK_MODE == 2) {
    /* program the trim value for 64KHz RC oscillator */
    RSI_IPMU_RC64khz_TrimEfuse(); //RC64K_EFUSE
  }
#endif
  program_ipmu_data(vbatt_status_trim_efuse); //VBATT_MON_EFUSE
#ifndef SLI_SI91X_MCU_INTERFACE
  program_ipmu_data(ro_ts_efuse);            //RO_TS_EFUSE
  program_ipmu_data(vbg_tsbjt_efuse);        //VBG_TS_EFUSE
  program_ipmu_data(auxadc_off_diff_efuse);  //AUXADC_DIFFOFF_EFUSE
  program_ipmu_data(auxadc_gain_diff_efuse); //AUXADC_DIFFGAIN_EFUSE
  program_ipmu_data(auxadc_off_se_efuse);    //AUXADC_SEOFF_EFUSE
  program_ipmu_data(auxadc_gain_se_efuse);   //AUXADC_SEGAIN_EFUSE
#endif
  program_ipmu_data(bg_trim_efuse);       //bg_trim_efuse
  program_ipmu_data(blackout_trim_efuse); //blackout_trim_efuse
  program_ipmu_data(poc_bias_efuse);      //poc_bias_efuse
  program_ipmu_data(buck_trim_efuse);
  program_ipmu_data(ldosoc_trim_efuse);
  program_ipmu_data(dpwm_freq_trim_efuse);
  program_ipmu_data(delvbe_tsbjt_efuse);
  program_ipmu_data(xtal1_bias_efuse);
#ifdef SLI_SI91X_MCU_INTERFACE
  program_ipmu_data(xtal2_bias_efuse);
#endif
#ifdef V1P20_IPMU_CHANGES
  //http://192.168.1.215:8090/display/RS9116/Release+V1.19+Porting+Guide
  //Retention-LDO Configuration
  program_ipmu_data(retn_ldo_lpmode);        //RETN1
  program_ipmu_data(retnLP_volt_trim_efuse); //RETNLP_TRIM_EFUSE
#ifdef SCDC_EXTCAP_MODE
  program_ipmu_data(lp_scdc_extcapmode); //SCDC1
#endif
#endif
#endif
}
#endif

#ifdef IPMU_CALIB_DATA
/**
 * @brief   This function reads, modifies & writes to the iPMU registers based on different PMU SPI register type
 * @param   void
 * @return  void
 */
void update_ipmu_data(uint32_t reg_addr, uint32_t reg_type, uint32_t data, uint32_t mask)
{
  uint32_t value = 0;

  if (reg_type == ULP_SPI) {
    value = PMU_DIRECT_ACCESS(reg_addr);
  } else if (reg_type == PMU_SPI) {
    value = PMU_SPI_DIRECT_ACCESS(reg_addr);
  } else if (reg_type == DIRECT) {
    value = *(volatile uint32_t *)(reg_addr);
  }
  value &= ~mask;
  value |= data;
  if (reg_type == ULP_SPI) {
    PMU_DIRECT_ACCESS(reg_addr) = value;
  } else if (reg_type == PMU_SPI) {
    PMU_SPI_DIRECT_ACCESS(reg_addr) = value;
  } else if (reg_type == DIRECT) {
    *(volatile uint32_t *)(reg_addr) = value;
  }
}
uint16 scdc_active;
uint16 scdc_sleep;
/**
 * @brief   This function prepares the data from the ipmu calib structure content and writes to each specific register
 * @param   void
 * @return  void
 */
void update_ipmu_calib_data(const efuse_ipmu_t *ipmu_calib_data) __attribute__((section(".common_tcm_code")));
void update_ipmu_calib_data(const efuse_ipmu_t *ipmu_calib_data)
{
  uint32_t data;
  uint32_t mask;
  uint32 calibratedtrim_scdc;
  uint32 calibratedtrim;
#ifndef SLI_SI91X_MCU_INTERFACE
  common_info_ext_t *glbl_common_info_ext_p = (common_info_ext_t *)glbl_common_info.reserved_ptr;
#endif
  //ROW 4,5,6
#ifdef SLI_SI91X_MCU_INTERFACE
  data = (ipmu_calib_data->trim_0p5na1 << 18);
  mask = MASK_BITS(1, 18);
#else
  data = (0 << 18) | (ipmu_calib_data->resbank_trim << 10);
  mask = MASK_BITS(1, 18) | MASK_BITS(2, 10);
#endif
  update_ipmu_data(iPMU_SPARE_REG1_OFFSET, ULP_SPI, data, mask);

  //ROW 5
  data = (ipmu_calib_data->bg_r_ptat_vdd_ulp << 19);
  mask = MASK_BITS(3, 19);
  if (ipmu_calib_data->set_vref1p3 == 0) {
    data |= (2 << 10) | (2 << 7);
  } else {
    data |= (uint32_t)(ipmu_calib_data->scdc_dcdc_trim << 10) | (ipmu_calib_data->scdc_hpldo_trim << 7);
  }
  mask |= MASK_BITS(3, 10) | MASK_BITS(3, 7);
  update_ipmu_data(BG_SCDC_PROG_REG_1_OFFSET, ULP_SPI, data, mask);

  scdc_active = (uint16)data;

  calibratedtrim      = ((data >> 7) & 7);
  calibratedtrim_scdc = ((data >> 10) & 7);
#ifdef DEEPSLEEP_SCDC_0P95
  if (calibratedtrim_scdc < 4) {
    calibratedtrim_scdc = calibratedtrim_scdc + 2;
  } else if (calibratedtrim_scdc == 4) {
    calibratedtrim_scdc = calibratedtrim_scdc + 1;
  }
  if (calibratedtrim == 0) {
    calibratedtrim = calibratedtrim + 3;
  } else if (calibratedtrim == 1) {
    calibratedtrim = calibratedtrim + 1;
  } else if ((calibratedtrim == 2) || (calibratedtrim == 3)) {
    calibratedtrim = calibratedtrim + 2;
  } else if (calibratedtrim == 4) {
    calibratedtrim = calibratedtrim + 1;
  }
#else
  if (calibratedtrim_scdc < 5) {
    calibratedtrim_scdc = calibratedtrim_scdc + 1;
  }
  if (calibratedtrim == 0) {
    calibratedtrim = calibratedtrim + 2;
  } else if (calibratedtrim == 1) {
    calibratedtrim = calibratedtrim - 1;
  } else if ((calibratedtrim == 2) || (calibratedtrim == 3) || (calibratedtrim == 4)) {
    calibratedtrim = calibratedtrim + 1;
  }
#endif
  scdc_sleep = (uint16)((calibratedtrim << 7) | (calibratedtrim_scdc << 10));
  //ROW 2
  data = ipmu_calib_data->bg_r_vdd_ulp;
  mask = MASK_BITS(5, 0);
  update_ipmu_data(BG_BLACKOUT_REG_OFFSET, ULP_SPI, data, mask);

  //ROW 7,17
  update_ipmu_data(ULPCLKS_MRC_CLK_REG_OFFSET, ULP_SPI, (ipmu_calib_data->trim_sel << 14), MASK_BITS(7, 14));

  //ROW 9
  update_ipmu_data(ULPCLKS_DOUBLER_XTAL_REG_OFFSET, ULP_SPI, (ipmu_calib_data->del_2x_sel << 15), MASK_BITS(6, 15));

  //ROW 10
  update_ipmu_data(ULPCLKS_32KRO_CLK_REG_OFFSET,
                   ULP_SPI,
                   (ipmu_calib_data->ro_32khz_00_trim << 16),
                   MASK_BITS(5, 16)); //rahul

  //ROW 11
  if (RC_CLK_MODE == 0) {
    data = (ipmu_calib_data->fine_trim_16k << 14);
  } else if (RC_CLK_MODE == 1) {
    data = (ipmu_calib_data->fine_trim_32k << 14);
  } else if (RC_CLK_MODE == 2) {
    data = (ipmu_calib_data->fine_trim_64k << 14);
  }
  mask = MASK_BITS(7, 14);
  update_ipmu_data(ULPCLKS_32KRC_CLK_REG_OFFSET, ULP_SPI, data, mask);

  //ROW 12
  if (XTAL_SEL == 1) {
    data = ipmu_calib_data->xtal1_trim_32k << 13;
  } else if (XTAL_SEL == 2) {
    data = ipmu_calib_data->xtal2_trim_32k << 13;
  }
  update_ipmu_data(ULPCLKS_32KXTAL_CLK_REG_OFFSET, ULP_SPI, data, MASK_BITS(4, 13));

  //ROW 13
  update_ipmu_data(ULPCLKS_HF_RO_CLK_REG_OFFSET, ULP_SPI, (ipmu_calib_data->trim_ring_osc << 14), MASK_BITS(7, 14));

  //ROW 16
  data = (ipmu_calib_data->f2_nominal);
  mask = MASK_BITS(10, 0);
  update_ipmu_data((TEMP_SENSOR_BASE_ADDRESS + TS_NOMINAL_SETTINGS_OFFSET), DIRECT, data, mask);

  //ROW 18
  data = *(volatile uint32_t *)(ULP_TASS_MISC_CONFIG_REG + 0x34);
  //! For AUX_LDO register access, need to enable ulp_aux_clock
  *(volatile uint32_t *)(ULP_TASS_MISC_CONFIG_REG + 0x34) = 0x1;
  update_ipmu_data((AUX_BASE_ADDR + 0x210), DIRECT, ipmu_calib_data->ldo_ctrl, MASK_BITS(4, 0));
  //! restore the reg
  *(volatile uint32_t *)(ULP_TASS_MISC_CONFIG_REG + 0x34) = data;

  //ROW 34
  if (ipmu_calib_data->set_vref1p3 == 0) {
    data = (0xC << 17);
  } else {
    data = (ipmu_calib_data->set_vref1p3 << 17);
  }
  mask = MASK_BITS(4, 17);
  update_ipmu_data(PMU_1P3_CTRL_REG_OFFSET, PMU_SPI, data, mask);

  //ROW 38
  data = ipmu_calib_data->trim_r1_resistorladder;
  mask = MASK_BITS(4, 0);
  update_ipmu_data(SPARE_REG_3_OFFSET, PMU_SPI, data, mask);

  //ROW 48
  data = (ipmu_calib_data->dpwm_freq_trim) << 13;
  mask = MASK_BITS(4, 13);
  update_ipmu_data(PMU_ADC_REG_OFFSET, PMU_SPI, data, mask);

#ifdef V1P20_IPMU_CHANGES
  if (ipmu_calib_data->set_vref1p3 == 0) {
    data = (0x2 << 18);
  } else {
    data = ((ipmu_calib_data->retn_ldo_lptrim) << 18);
  }
  mask = MASK_BITS(3, 18);
  update_ipmu_data(BG_LDO_REG_OFFSET, ULP_SPI, data, mask);
#endif
#ifndef SLI_SI91X_MCU_INTERFACE
  glbl_common_info_ext_p->eeprom_calib_values.vbatt_status_trim_efuse = ipmu_calib_data->vbatt_status_1;
#endif
}
/**
 * @brief   This API is used to reduce or increase the SCDC voltage by 1 value 
 * @param   Deepsleep : provide '1' before going to sleep.(i.e.Reduce the SCDC voltage by one value)
 *                     provide '0' after wake up.(i.e.Set the SCDC voltage to the actual value)
 * @return  void
 */
void set_scdc(uint32 Deepsleep)
{
  uint32 mask;

  mask = MASK_BITS(3, 10) | MASK_BITS(3, 7);
  update_ipmu_data(BG_SCDC_PROG_REG_1_OFFSET, ULP_SPI, Deepsleep ? scdc_sleep : scdc_active, mask);
}
/**
 * @brief   This function checks for magic byte in efuse/flash, copies the content if valid data present and calls to update the ipmu registers
 * @param   void
 * @return  void
 */
uint32_t init_ipmu_calib_data(uint32_t m4_present)
{
  (void)m4_present;
  efuse_ipmu_t ipmu_calib_data;
  efuse_ipmu_t *ipmu_calib_data_p;
#ifndef SLI_SI91X_MCU_INTERFACE
  common_info_ext_t *glbl_common_info_ext_p;
#endif
  ipmu_calib_data_p = &ipmu_calib_data;
#ifndef SLI_SI91X_MCU_INTERFACE
  common_info_t *glbl_common_info_p = &glbl_common_info;
  glbl_common_info_ext_p            = (common_info_ext_t *)glbl_common_info_p->reserved_ptr;

  if ((*(uint8 *)(MANF_DATA_BASE_ADDR + BASE_OFFSET_BB_RF_IN_FLASH)) != MAGIC_WORD) {
    //NO CALIB DATA. Return
    return 1;
  }
#endif
  if (*(uint8 *)(MANF_DATA_BASE_ADDR + IPMU_VALUES_OFFSET) == 0x00) {
    // NO CALIB DATA. Return
    return 1;
  }
  if (*(uint8 *)(MANF_DATA_BASE_ADDR + IPMU_VALUES_OFFSET) == 0xFF) {
    // NO CALIB DATA. Return
    return 1;
  }
  memcpy(ipmu_calib_data_p, (efuse_ipmu_t *)(MANF_DATA_BASE_ADDR + IPMU_VALUES_OFFSET), sizeof(efuse_ipmu_t));
#ifndef SLI_SI91X_MCU_INTERFACE
  glbl_common_info_ext_p->auxadc_offset_single   = ipmu_calib_data_p->auxadc_offset_single;
  glbl_common_info_ext_p->vbg_tsbjt_efuse        = ipmu_calib_data_p->vbg_tsbjt_efuse;
  glbl_common_info_ext_p->str_bjt_temp_sense_off = ipmu_calib_data_p->str_bjt_temp_sense_off;
  if (m4_present == 0) {
#endif
    //Dummy read
    PMU_SPI_DIRECT_ACCESS(PMU_PFM_REG_OFFSET);

    update_ipmu_calib_data(ipmu_calib_data_p);
#ifdef SLI_SI91X_MCU_INTERFACE
    RSI_IPMU_UpdateIpmuCalibData_efuse(ipmu_calib_data_p);
#endif
    //Dummy read
    PMU_SPI_DIRECT_ACCESS(PMU_PFM_REG_OFFSET);
#ifndef SLI_SI91X_MCU_INTERFACE
  }
#endif
#ifndef SLI_SI91X_MCU_INTERFACE
  //ROW 38
  update_ipmu_data((NWP_AHB_ADDR + 0x604), DIRECT, (ipmu_calib_data_p->scale_soc_ldo_vref << 31), MASK_BITS(1, 31));
#endif
  return 0;
}
#endif
#ifdef IPMU_DOTC_PROG
#ifdef SLI_SI91X_MCU_INTERFACE
void program_ipmu_data(const uint32_t *src);
#else
void program_ipmu_data(const uint32_t *src) __attribute__((section(".common_non_tcm_code")));
#endif
void program_ipmu_data(const uint32_t *src)
{
  uint32_t write_data;
  uint32_t num_of_reg;
  uint32_t mask = 0;
  uint32_t read_data;
  uint32_t addr;
  uint32_t ls_shift;
  uint32_t ms_shift;
  uint32_t mask_bits;
  uint32_t inx = 0;
  num_of_reg   = src[inx];
  inx++;

  //Dummy Read
  PMU_SPI_DIRECT_ACCESS(PMU_PFM_REG_OFFSET);

  while (num_of_reg--) {
    addr = src[inx];
    //! 32bit Write data [31:27] ms shift and [26:22] ls shift and [21:0] data
    write_data = src[inx + 1];
    //! extracting bit positions in which the data should be updated
    ls_shift = (write_data >> LS_SHIFT) & 0x1F;
    ms_shift = (write_data >> MS_SHIFT) & 0x1F;
    //! Extracting 21-bit data from the structure
    write_data = write_data & IPMU_DATAMASK;
    read_data  = *(uint32_t *)addr;
    //! no of bits be updated
    mask_bits = ms_shift - ls_shift + 1;
    //! creating mask for the no of bits to be updated
    while (mask_bits--) {
      mask |= BIT(mask_bits);
    }
    //! resetting the bits in the read_data to update the data and writing the data to it
    read_data &= ~(mask << ls_shift);
    read_data |= (write_data & mask) << ls_shift;
    *(uint32_t *)addr = read_data;
    inx += 2;
    mask = 0;
  }
}
#endif
#ifdef SLI_SI91X_MCU_INTERFACE
void shut_down_non_wireless_mode_pds(void);
#else
// Power Domain Controls
void shut_down_non_wireless_mode_pds(void) __attribute__((section(".common_non_tcm_code")));
#endif
void shut_down_non_wireless_mode_pds(void)
{
  uint32_t reg_val = 0;
  // WuRX
  PMU_DIRECT_ACCESS(iPMU_SPARE_REG2_OFFSET) &= ~(wurx_lvl_shift_en);
  PMU_DIRECT_ACCESS(iPMU_SPARE_REG2_OFFSET) &= ~(wurx_pg_en_1);

  PMU_DIRECT_ACCESS(ULPCLKS_REF_CLK_REG_OFFSET) &= ~pass_clk_40m_buffer_enable;

#ifndef IPMU_DOTC_PROG
  PMU_DIRECT_ACCESS(0x125) |= (BIT(2));             //! Added by Nagaraj
  PMU_DIRECT_ACCESS(0x127) &= ~((BIT(3)) | BIT(4)); //! Added by Nagaraj
#else
  program_ipmu_data(ana_perif_ptat_common_config2);
  program_ipmu_data(ipmu_bod_clks_common_config2);
#endif

  reg_val = (PMU_DIRECT_ACCESS(POWERGATE_REG_READ_OFFSET) >> 5);
  reg_val &= ~(cmp_npss_pg_enb | ulp_ang_clks_pg_enb | wurx_corr_pg_enb | wurx_pg_enb | auxadc_pg_enb | auxdac_pg_enb);
  PMU_DIRECT_ACCESS(POWERGATE_REG_WRITE_OFFSET) = reg_val;
#ifndef SLI_SI91X_MCU_INTERFACE
  // MHz RC
  PMU_DIRECT_ACCESS(ULPCLKS_MRC_CLK_REG_OFFSET) &= ~rc_mhz_en;
  MCU_FSM_DIRECT_ACCESS(MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP) &= ~mcu_ulp_mhz_rc_clk_en;
#endif
#if 0
  reg_val = (PMU_DIRECT_ACCESS(POWERGATE_REG_READ_OFFSET) >> 5);
  reg_val &= ~(wurx_corr_pg_enb | wurx_pg_enb);
  PMU_DIRECT_ACCESS(POWERGATE_REG_WRITE_OFFSET) = reg_val;

  // BG-PMU_SPI
  reg_val = (PMU_DIRECT_ACCESS(POWERGATE_REG_READ_OFFSET) >> 5);
  reg_val &= ~(ulp_ang_pwrsupply_pg_enb | wurx_pg_enb);
  PMU_DIRECT_ACCESS(POWERGATE_REG_WRITE_OFFSET) = reg_val;

  // Button-Wakeup
  PMU_DIRECT_ACCESS(BOD_TEST_PG_VBATT_STATUS_REG_OFFSET) &= ~bod_pwrgate_en_n_ulp_button_calib;

  // Clk Calibration
  PMU_DIRECT_ACCESS(ULPCLKS_TRIM_SEL_REG_OFFSET) &= ~calib_powergate_en;

  // Adaptive Calibration
  PMU_DIRECT_ACCESS(ULPCLKS_ADAPTIVE_REG_OFFSET) &= ~adapt_powergate_en;

  // High Freq RO
  PMU_DIRECT_ACCESS(ULPCLKS_HF_RO_CLK_REG_OFFSET) &= ~ro_hf_en;
  MCU_FSM_DIRECT_ACCESS(MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP) &= ~mcu_ulp_20mhz_ring_osc_clk_en;
  ULP_DIRECT_ACCESS(NWP_FSM_FIRST_BOOTUP) &= ~nwp_ulp_20mhz_ring_osc_clk_en;
 
  // 32KHz XTAL
  PMU_DIRECT_ACCESS(ULPCLKS_32KXTAL_CLK_REG_OFFSET) &= ~xtal_32khz_en;
  MCU_FSM_DIRECT_ACCESS(MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP) &= ~mcu_ulp_32khz_xtal_clk_en;
  ULP_DIRECT_ACCESS(NWP_FSM_FIRST_BOOTUP) &= ~nwp_ulp_32khz_xtal_clk_en;

  // 32MHz Doubler
  PMU_DIRECT_ACCESS(ULPCLKS_DOUBLER_XTAL_REG_OFFSET) &= ~doubler_en;
  MCU_FSM_DIRECT_ACCESS(MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP) &= ~mcu_ulp_doubler_clk_en;
  ULP_DIRECT_ACCESS(NWP_FSM_FIRST_BOOTUP) &= ~nwp_ulp_doubler_clk_en;

  // MHz RC
  PMU_DIRECT_ACCESS(ULPCLKS_MRC_CLK_REG_OFFSET) &= ~rc_mhz_en;
  MCU_FSM_DIRECT_ACCESS(MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP) &= ~mcu_ulp_mhz_rc_clk_en;
  ULP_DIRECT_ACCESS(NWP_FSM_FIRST_BOOTUP) &= ~nwp_ulp_mhz_rc_clk_en;

  // Tempsense_RO
  TS_ENABLE_AND_TEMPERATURE_DONE &= ~temp_sens_en;

  // Tempsense_BJT
  //TEMP_SENSOR_BJT &= ~BIT(0);

  // Aux-ADC/Aux-DAC
  reg_val = (PMU_DIRECT_ACCESS(POWERGATE_REG_READ_OFFSET) >> 5);
  reg_val &= ~(auxadc_pg_enb | auxdac_pg_enb);
  PMU_DIRECT_ACCESS(POWERGATE_REG_WRITE_OFFSET) = reg_val;
#endif
}
#ifdef SLI_SI91X_MCU_INTERFACE
void ipmu_init(void);
#else
void ipmu_init(void) __attribute__((section(".common_non_tcm_code")));
#endif
void ipmu_init(void)
{
#ifndef SLI_SI91X_MCU_INTERFACE
  //http://192.168.1.215:8090/display/IPMU40GF/Registers+Summary
  uint32_t ipmu_calb_data_invalid = 1;
#endif
#ifndef IPMU_DOTC_PROG
  uint32_t pmu_1p3_ctrl_data;
#endif
  uint32_t pmu_1p2_ctrl_word;
  uint32_t bypass_curr_ctrl_data;
  const retention_boot_status_word_t *retention_reg = (const retention_boot_status_word_t *)MCURET_BOOTSTATUS;

  //! If M4 present and host interface with M4(M4 master) case, total IPMU and MCU FSM registers has to be initialised in M4.
  //! Always NWP FSM registers has to be programmed here.
  //! In WiSE MCU BLE only mode IPMU programming will be done in MCU

#ifndef SLI_SI91X_MCU_INTERFACE
#ifndef BT_LE_ONLY_MODE
  if (!retention_reg->m4_present) {
    ipmu_init_mcu();
  }
#endif
#ifdef IPMU_CALIB_DATA
  //! Read calib data from efuse/flash and update to iPMU registers
  ipmu_calb_data_invalid = init_ipmu_calib_data(retention_reg->m4_present);
#endif
#else
  ipmu_init_mcu();
  //! Read calib data from efuse/flash and update to iPMU registers
  init_ipmu_calib_data(retention_reg->m4_present);
#endif

#ifndef DISABLE_M4SS_SWITCH_OFF
#ifndef SLI_SI91X_MCU_INTERFACE
  if (!retention_reg->m4_present)
#endif
  {
#ifdef SLI_SI91X_MCU_INTERFACE
    MCU_PMU_LDO_CTRL_CLEAR_REG_1 =
#else
    MCU_PMU_LDO_CTRL_CLEAR =
#endif
      (mcu_soc_ldo_lvl | mcu_dcdc_lvl); //Dynamic control of LDO-SoC and VOUTBCK o/p volt from NWP
    MCU_FSM_PMU_CTRL |= standby_dc1p3;  //Dynamic control of PFM Mode from NWP
    MCU_FSM_DIRECT_ACCESS(MCU_FSM_PMU_STATUS_REG_OFFSET) |= (scdcdc_lp_mode_en);
    MCU_FSM_DIRECT_ACCESS(MCU_FSM_PMU_STATUS_REG_OFFSET) |= (bgpmu_sleep_en);

#ifdef IPMU_DOTC_PROG
#ifndef IPMU_POWER_ENHANCEMENT
    //This is commented sothat retention ldo trim value BG_LDO_REG_OFFSET[20:18] is not over written
    program_ipmu_data(retn_ldo_lpmode);
#endif
#else
    PMU_DIRECT_ACCESS(0x129) |= (BIT(15) | BIT(16));           //! Added by Nagaraj
    PMU_DIRECT_ACCESS(0x140) |= (BIT(15) | BIT(18) | BIT(19)); //! Added by Nagaraj
#endif

#ifdef IPMU_DOTC_PROG
#if 0
    program_ipmu_data(lp_scdc_extcapmode);
#endif
#else
    PMU_DIRECT_ACCESS(BG_SCDC_PROG_REG_2_OFFSET);
    PMU_DIRECT_ACCESS(SCDC_CTRL_REG_0_OFFSET)    = 0x3e002f;
    PMU_DIRECT_ACCESS(BG_SCDC_PROG_REG_2_OFFSET) = 0x200020;
    PMU_DIRECT_ACCESS(BG_LDO_REG_OFFSET)         = 0x58000;
#endif

    PMU_DIRECT_ACCESS(SELECT_BG_CLK_OFFSET) |= BIT(1); //! BG-PMU Clock configured to MCU_FSM_SLEEP_CLK
#ifndef SLI_SI91X_MCU_INTERFACE
    shut_down_non_wireless_mode_pds();
#endif
#ifdef SLI_SI91X_MCU_INTERFACE
    MCUAON_GEN_CTRLS_REG &= ~NPSS_SUPPLY_0P9_BIT; //Dynamic switching of NPSS/iPMU supply during sleep
    MCUAON_GEN_CTRLS_REG |= ENABLE_PDO_BIT;       //Dynamic control of PADs PDO during sleep
#else
    MCUAON_GEN_CTRLS_REG &= ~NPSS_SUPPLY_0P9; //Dynamic switching of NPSS/iPMU supply during sleep
    MCUAON_GEN_CTRLS_REG |= ENABLE_PDO;       //Dynamic control of PADs PDO during sleep
#endif
#ifndef IPMU_DOTC_PROG
    PMU_SPI_DIRECT_ACCESS(PMU_LDO_REG_OFFSET) &= ~LDOSOC_DEFAULT_MODE_EN; //LDO-SoC o/p Volt is configurable from NPSS
    PMU_SPI_DIRECT_ACCESS(PMU_PWRTRAIN_REG_OFFSET) |= BYPASS_LDORF_CTRL;  //LDO-FLASH is configurable from NPSS

    if (ipmu_calb_data_invalid) {
      PMU_SPI_DIRECT_ACCESS(0x1DA) = 0x2818;
      PMU_SPI_DIRECT_ACCESS(0x1DD) = 0x26249A;
      PMU_SPI_DIRECT_ACCESS(0x1D0) = 0x132241;
      PMU_SPI_DIRECT_ACCESS(0x1D2) = 0xE80D7;
      PMU_SPI_DIRECT_ACCESS(0x1D3) = 0x872B;
    }

    // Setting VOUTBCK to 1.35V (Finally Done from Calibration Data in EFUSE)
    pmu_1p3_ctrl_data = PMU_SPI_DIRECT_ACCESS(PMU_1P3_CTRL_REG_OFFSET);
    pmu_1p3_ctrl_data &= ~(0xF << 17);
    PMU_SPI_DIRECT_ACCESS(PMU_1P3_CTRL_REG_OFFSET) = (pmu_1p3_ctrl_data | (0xA << 17));
    ////////////////////////////////////////////////////////////
#endif

    // Setting VOUTBCK_LOW to 1.25V based on the data obtained from Calibration Data
    bypass_curr_ctrl_data = PMU_SPI_DIRECT_ACCESS(PMU_1P3_CTRL_REG_OFFSET);
    pmu_1p2_ctrl_word     = ((bypass_curr_ctrl_data >> 17) & 0xF) - 2;
    bypass_curr_ctrl_data = PMU_SPI_DIRECT_ACCESS(BYPASS_CURR_CTRL_REG_OFFSET);
    bypass_curr_ctrl_data &= (uint32_t)(~(0xF << 5));
    PMU_SPI_DIRECT_ACCESS(BYPASS_CURR_CTRL_REG_OFFSET) = (bypass_curr_ctrl_data | (pmu_1p2_ctrl_word << 5));

    PMU_DIRECT_ACCESS(BG_SLEEP_TIMER_REG_OFFSET) |= BIT(19); //bgs_active_timer_sel
    //http://192.168.1.215:8090/display/RS9116/MCU+AON+Registers#MCUAONRegisters-MCUAON_SHELF_MODE
    MCUAON_SHELF_MODE |= (7 << 19); //Programmable delay for resetting Chip
  }
#endif

#ifndef SLI_SI91X_MCU_INTERFACE

#ifdef BGPMU_SAMPLING_EN
  ULP_DIRECT_ACCESS(NWP_PMU_CTRLS) |= (scdcdc_lp_mode_en);
  ULP_DIRECT_ACCESS(NWP_PMU_CTRLS) |= (bgpmu_sleep_en); //Disabling Brownout Detection. enable once issue is fixed.
#endif
#ifdef DCDC_STANDBY_MODE_EN
  ULP_DIRECT_ACCESS(NWP_PMU_CTRLS) |= standby_dc1p3;
#endif

  //BIT(16) is added for smooth reset of bandgap in 1.4REV Silicon
  ULP_DIRECT_ACCESS(NWPAON_POR_CTRL_BITS) |= (poc_cntrl_reg_0 | BIT(16));

  //! RC MHz disable from NWP
  ULP_DIRECT_ACCESS(NWP_FSM_FIRST_BOOTUP) &= ~nwp_ulp_mhz_rc_clk_en;

  TASS_FSM_CTRL_BYPASS &= ~ta_pmu_shut_down_bypass;
  TASS_FSM_CTRL_BYPASS &= ~ta_pmu_shut_down_bypass_cntrl;

  TASS_FSM_CTRL_BYPASS |= ta_buck_boost_enable_bypass;
#endif
}
#ifdef SLI_SI91X_MCU_INTERFACE
void configure_uulp_gpio_to_1p8v(void);
#else
void configure_uulp_gpio_to_1p8v(void) __attribute__((section(".common_non_tcm_code")));
#endif
void configure_uulp_gpio_to_1p8v(void)
{
  //! UULP GPIO to operate at 1.8v
  //http://192.168.1.215:8090/display/IPMU40GF/Registers+Summary
  PMU_DIRECT_ACCESS(iPMU_SPARE_REG1_OFFSET); //Dummy read
  PMU_DIRECT_ACCESS(iPMU_SPARE_REG1_OFFSET) |= BIT(13);
}

#ifdef SLI_SI91X_MCU_INTERFACE
void configure_ipmu_mode(uint32_t mode);
#else
void configure_ipmu_mode(uint32_t mode) __attribute__((section(".common_non_tcm_code")));
#endif
void configure_ipmu_mode(uint32_t mode)
{
  if (mode) {
    //program_ipmu_data(hpldo_volt_trim_efuse); //HPLDO_TRIM_EFUSE
    program_ipmu_data(hpldo_tran);                   //SCDC3
    program_ipmu_data(buck_fast_transient_duty_1p8); //SCDC3
    program_ipmu_data(LDOFLASH_BYPASS);              //keeping ldoflash in bypass mode in 1p8V supply mode;
  } else {
    //program_ipmu_data(scdc_volt_trim_efuse); //SCDC_TRIM_EFUSE
    program_ipmu_data(ipmu_scdc_enable);   //EXT CAP Mode
    program_ipmu_data(lp_scdc_extcapmode); //EXT CAP Mode
  }
}
#ifndef SLI_SI91X_MCU_INTERFACE
void disable_ipmu_write_access() __attribute__((section(".common_non_tcm_code")));
void disable_ipmu_write_access()
{
  PMU_DIRECT_ACCESS(SELECT_BG_CLK_OFFSET);
  PMU_DIRECT_ACCESS(SELECT_BG_CLK_OFFSET) &=
    ~(latch_transparent_lf | latch_transparent_hf | latch_top_spi); //! Added by Nagaraj
  PMU_DIRECT_ACCESS(SELECT_BG_CLK_OFFSET);
}
// temp sensor
uint16 adc_read_data_func()
{
  uint16 adc_data;
  uint32_t dummy_read;
  uint32_t adc_data_avg;
  //uint16 fifo_data[16][32];
  uint32_t inxx2 = 0;

  adc_data_avg = 0;

  for (dummy_read = 0; dummy_read < 32; dummy_read++) {
    while ((*(volatile uint32_t *)(0x24043830)) & BIT(2))
      ;
    adc_data = ((*(volatile uint32_t *)(0x24043814)) & 0xFFF);
    adc_data = BIT(11) ^ adc_data;
    if (dummy_read > 15) {
      adc_data_avg += adc_data;
    }
    //fifo_data[inxx2][dummy_read] = adc_data;
  }
  adc_data_avg /= 16;
  inxx2 = (inxx2 + 1) & 15;
  return adc_data_avg;
}

uint32_t adccalibDone;
#define ULP_SPI_MEM_MAP(REG_ADR) (*((uint32_t volatile *)(PMU_SPI_BASE_ADDR + (0xa000 + (REG_ADR * 4)))))
#define AUX_LDO                  *(volatile uint32_t *)(0x24043a10)
void calibrate_adc()
{
  uint32_t auxadcCalibValueLoad = 0, auxadcCalibValue = 0;

  if (adccalibDone == 0) {
    ULP_SPI_MEM_MAP(0x110) |= BIT(11);
    ULP_SPI_MEM_MAP(0x110) |= (BIT(12) | BIT(13) | BIT(8) | BIT(7) | BIT(6));
    while (*(volatile uint32_t *)(0x24050000 + 0x02) & BIT(8))
      ;
    while (!(ULP_SPI_MEM_MAP(0x1C1) & BIT(0)))
      ;
    while ((ULP_SPI_MEM_MAP(0x1C1) & BIT(0)))
      ;
    usleep(20);
    auxadcCalibValue = ULP_SPI_MEM_MAP(0x112);
    usleep(20);
    auxadcCalibValueLoad |= BIT(0) | BIT(7);
    auxadcCalibValueLoad |= (auxadcCalibValue & 0x1F) << 2;
    auxadcCalibValueLoad |= (((auxadcCalibValue >> 6) & 0x1F) << 8);
    auxadcCalibValueLoad |= (((auxadcCalibValue >> 11) & 0x1F) << 13);
    adccalibDone = 1;
  }
  ULP_SPI_MEM_MAP(0x111) = (auxadcCalibValueLoad);

  while (*(volatile uint32_t *)(0x24050000 + 0x02) & BIT(8))
    ;

  /* read calibrated p and n values */
  //calibrated_value = ULP_SPI_MEM_MAP(0x112);
}

void get_ipmu_temperature(common_info_t *glbl_common_info_p)
{
  int32 adc_off;
  int32 Vbg;
  int32 Voffset;
  uint32_t pmu_reg;
  int32 irf_temperature;
  int32 adc_bjt;
  int32 adc_bg;
  //  int32 dbg_adc_bjt[32];
  //  int32 dbg_adc_bg[32];
  // int16 temp[32];
  uint8 inxx                                = 0;
  common_info_ext_t *glbl_common_info_ext_p = (common_info_ext_t *)glbl_common_info_p->reserved_ptr;

  if ((glbl_common_info_p->host_if == CORTEX_M4) || (!glbl_common_info_ext_p->chip_version_1p4_or_more)) {
    irf_temperature = 70;
    goto end_temp_measurement;
  }
  get_m4ss_access();
  *(volatile uint32_t *)(0x2405a000 + (0x142 * 4)); //dummy read

  //reg_val1 = PMU_DIRECT_ACCESS(BG_SCDC_PROG_REG_1_OFFSET);

  //PMU_DIRECT_ACCESS(BG_SCDC_PROG_REG_1_OFFSET) = reg_val1 | BIT(4);

  *(volatile uint32_t *)(0x2405a000 + (0x143 * 4)) = 0;
  *(volatile uint32_t *)(0x2405a000 + (0x142 * 4)) = 0x1F910;

  ULPSS_PWRCTRL_SET_REG = ulpss_ext_pwrgate_en_n_ulp_AUX;

  ULP_DYN_CLK_CTRL_DISABLE |= BIT(11) | BIT(12) | BIT(13);
  ULP_MISC_SOFT_SET_REG &= 0xfff8001f;
  ULP_MISC_SOFT_SET_REG |= 0x0007ffe0;
  ULP_AUXADC_CLK_GEN_REG = 0x1;
  AUX_LDO                = 0x7b;

  *(volatile uint32_t *)(0x2404380C) = 40;

  // wait for 50us
  usleep(100);

  //*(uint32_t*)(0x2404C000 + 9 *(0x10))=(7<<2);//FIXME DELETE

  pmu_reg                                          = ~0x1E & (*(volatile uint32_t *)(0x2405a000 + (0x129 * 4)));
  *(volatile uint32_t *)(0x2405a000 + (0x129 * 4)) = pmu_reg | (8 << 1);
  *(volatile uint32_t *)(0x24043a18)               = 0x2e005;

  while (*(volatile uint32_t *)(0x24050000 + 0x02) & BIT(8))
    ;

  *(volatile uint32_t *)(0x24043800 + 0x24) &= ~BIT(4);
  *((volatile uint32_t *)(0x24043800 + (512 + 8))) = 0x00000C15; //OPAMP2 Channel21
  *((volatile uint32_t *)(0x24043800 + (78 * 4)))  = 0x00000000;
  *((volatile uint32_t *)(0x24043800 + (94 * 4)))  = 0x00000001;
  *((volatile uint32_t *)(0x24043800 + (119 * 4))) = 0x00000001;
  *((volatile uint32_t *)(0x24043800 + (1 * 4)))   = 0x8000C05;

  usleep(400);
  calibrate_adc();

  adc_bg = adc_read_data_func();

  *((volatile uint32_t *)(0x24043800 + (1 * 4)))   = 0x0;
  *((volatile uint32_t *)(0x24043800 + (512 + 8))) = 0x15;
  ////////////TS BJT /////////////////////////

  TS_SLOPE_SET |= BIT(28);
  VAD_BBP_ID |= BIT(16);
  //To control ADC channel
  ADC_CHANNEL = 0x002e0000;
  //For ADC channel contro
  ADC_CHANNEL_CNTRL = 0x00000000;
  //Internal DMA disable and channel enable
  DMA = 0x00000001;

  //Enbale temperature sensor
  TEM = 0x00000001;

  //BJT TEMP TEST MUX
  *(volatile uint32_t *)(0x2405a000 + (0x143 * 4)) = 0x3C0010;
  *(volatile uint32_t *)(0x2405a000 + (0x140 * 4)) = 0x220;
  while (*(volatile uint32_t *)(0x24050000 + 0x02) & BIT(8))
    ;

  *(volatile uint32_t *)(0x2404380C) = 20;
  *(volatile uint32_t *)(0x24043800 + 0x24) &= ~BIT(4);
  *((volatile uint32_t *)(0x24043800 + (512 + 8))) = 0x00000C17;
  *((volatile uint32_t *)(0x24043800 + (1 * 4)))   = 0x8000C05;

  adc_bjt = adc_read_data_func();

  ADC_CHANNEL = 0x00000000;
  ADC_CHANNEL;
  /* turn off adc */
  *((volatile uint32_t *)(0x24043800 + (512 + 8))) &= ~((BIT(10) | BIT(11)));
  *((volatile uint32_t *)(0x24043800 + (1 * 4))) &= ~BIT(0);
  //PMU_DIRECT_ACCESS(BG_SCDC_PROG_REG_1_OFFSET) = reg_val1;

  adc_off = glbl_common_info_ext_p->auxadc_offset_single;
  Vbg     = glbl_common_info_ext_p->vbg_tsbjt_efuse;
  Voffset = 961 - glbl_common_info_ext_p->str_bjt_temp_sense_off;

  //dbg_adc_bg[inxx] = adc_bg;
  //dbg_adc_bjt[inxx] = adc_bjt;
  irf_temperature = -268 + (310 * (((Vbg * (adc_bjt - adc_off)) / (adc_bg - adc_off)) + Voffset) / 1000) + 40;

  //temp[inxx] = irf_temperature -40;
  if ((irf_temperature > 100) || (irf_temperature < 50)) {
    //while(1);
  }
  if (inxx == 7)
    while (1)
      ;
  inxx = (inxx + 1) & 31;
  release_m4ss_access();
end_temp_measurement:
  glbl_common_info_ext_p->chip_temperature = irf_temperature;
}
#endif

#endif
