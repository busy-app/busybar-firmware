/*******************************************************************************
* @file  rsi_ipmu.c
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

/**
 * Includes
 */
#include "rsi_system_config.h"
#include "rsi_ipmu.h"
#include "rsi_pll.h"
#include "rsi_ulpss_clk.h"
#include "sl_si91x_bjt_temperature_sensor.h"

/**
 * Defines
 */
#define SYSTEM_CLK_VAL_20MHZ ((uint32_t)(20000000)) // macro for 20MHz
#define SYSTEM_CLK_VAL_MHZ   ((uint32_t)(32000000)) // macro for 32MHz doubler

extern adc_config_t sl_bjt_config;
extern adc_ch_config_t sl_bjt_channel_config;

/**
 * @fn          void RSI_IPMU_UpdateIpmuCalibData_efuse(const efuse_ipmu_t *ipmu_calib_data)
 * @brief       This function prepares the data from the ipmu calib structure content and writes to each specific register
 * @param[in]   ipmu_calib_data : pointer of calibrate data
 * @return      none
 */
void RSI_IPMU_UpdateIpmuCalibData_efuse(const efuse_ipmu_t* ipmu_calib_data) {
    uint32_t data = 0;
    uint32_t value = 0;
    uint32_t mask = 0;
    /* over writing the efuse arrays */

#ifdef CHIP_9118
    /* POC_BIAS_EFUSE */
    data = (ipmu_calib_data->trim_0p5na1 | ipmu_calib_data->trim_0p5na2 << 1);
    mask = MASK_BITS(22, 0);
    value = poc_bias_efuse[2];
    value &= ~mask;
    value |= data;
    poc_bias_efuse[2] = value;
#endif

#if defined(SLI_SI917)
    /* POC_BIAS_EFUSE */
    data = (ipmu_calib_data->trim_0p5na1);
    mask = MASK_BITS(22, 0);
    value = poc_bias_efuse[2];
    value &= ~mask;
    value |= data;
    poc_bias_efuse[2] = value;
#endif

#ifdef CHIP_9118
    /* BG_TRIM_EFUSE */
    data = (ipmu_calib_data->bg_r_vdd_ulp | ipmu_calib_data->bg_r_ptat_vdd_ulp << 3);
    mask = MASK_BITS(22, 0);
    value = bg_trim_efuse[4];
    value &= ~mask;
    value |= data;
    bg_trim_efuse[4] = value;
#endif

#if defined(SLI_SI917)
    /* BG_TRIM_EFUSE */
    data = (ipmu_calib_data->bg_r_ptat_vdd_ulp);
    mask = MASK_BITS(22, 0);
    value = bg_trim_efuse[2];
    value &= ~mask;
    value |= data;
    bg_trim_efuse[2] = value;

    /* BG_TRIM_EFUSE */
    data = (ipmu_calib_data->bg_r_vdd_ulp);
    mask = MASK_BITS(22, 0);
    value = bg_trim_efuse[4];
    value &= ~mask;
    value |= data;
    bg_trim_efuse[4] = value;
#endif

#ifdef CHIP_9118
    /*  POC2  ( blackout_trim_efuse )*/
    data = (ipmu_calib_data->resbank_trim);
    mask = MASK_BITS(22, 0);
    value = blackout_trim_efuse[2];
    value &= ~mask;
    value |= data;
    blackout_trim_efuse[2] = value;
#endif

    /* M32RC_EFUSE */
    data = ipmu_calib_data->trim_sel;
    mask = MASK_BITS(22, 0);
    value = m32rc_osc_trim_efuse[2];
    value &= ~mask;
    value |= data;
    m32rc_osc_trim_efuse[2] = value;

    /* dblr_32m_trim_efuse */
    data = ipmu_calib_data->del_2x_sel;
    mask = MASK_BITS(22, 0);
    value = dblr_32m_trim_efuse[2];
    value &= ~mask;
    value |= data;
    dblr_32m_trim_efuse[2] = value;

    /* ro_32khz_trim_efuse */
    data = ipmu_calib_data->freq_trim;
    mask = MASK_BITS(22, 0);
    value = ro_32khz_trim_efuse[2];
    value &= ~mask;
    value |= data;
    ro_32khz_trim_efuse[2] = value;

    /* rc_16khz_trim_efuse */
    data = (uint32_t)(ipmu_calib_data->coarse_trim_16k | ipmu_calib_data->fine_trim_16k << 2);
    mask = MASK_BITS(22, 0);
    value = rc_16khz_trim_efuse[2];
    value &= ~mask;
    value |= data;
    rc_16khz_trim_efuse[2] = value;

    /*  rc_64khz_trim_efuse */
    data = (uint32_t)(ipmu_calib_data->coarse_trim_64k | ipmu_calib_data->fine_trim_64k << 2);
    mask = MASK_BITS(22, 0);
    value = rc_64khz_trim_efuse[2];
    value &= ~mask;
    value |= data;
    rc_64khz_trim_efuse[2] = value;

    /* xtal1_bias_efuse */
    data = (ipmu_calib_data->xtal1_trim_32k);
    mask = MASK_BITS(22, 0);
    value = xtal1_bias_efuse[2];
    value &= ~mask;
    value |= data;
    xtal1_bias_efuse[2] = value;

    /* xtal2_bias_efuse  */
    data = (ipmu_calib_data->xtal2_trim_32k);
    mask = MASK_BITS(22, 0);
    value = xtal2_bias_efuse[2];
    value &= ~mask;
    value |= data;
    xtal2_bias_efuse[2] = value;

#ifndef AT_EFUSE_DATA_1P19
    /* m20rc_osc_trim_efuse  */
    data = ((ipmu_calib_data->reserved1) & 0x7F);
    mask = MASK_BITS(22, 0);
    value = m20rc_osc_trim_efuse[2];
    value &= ~mask;
    value |= data;
    m20rc_osc_trim_efuse[2] = value;
#endif

#ifdef AT_EFUSE_DATA_1P19
    /* m20rc_osc_trim_efuse  */
    data = (ipmu_calib_data->trim_sel_20Mhz);
    mask = MASK_BITS(22, 0);
    value = m20rc_osc_trim_efuse[2];
    value &= ~mask;
    value |= data;
    m20rc_osc_trim_efuse[2] = value;
#endif

    /*m20ro_osc_trim_efuse   */
    data = (ipmu_calib_data->trim_ring_osc);
    mask = MASK_BITS(22, 0);
    value = m20ro_osc_trim_efuse[2];
    value &= ~mask;
    value |= data;
    m20ro_osc_trim_efuse[2] = value;

    /* vbatt_status_trim_efuse */
    data = (ipmu_calib_data->vbatt_status_1);
    mask = MASK_BITS(22, 0);
    value = vbatt_status_trim_efuse[2];
    value &= ~mask;
    value |= data;
    vbatt_status_trim_efuse[2] = value;

    /*  ro_ts_efuse */
    data = (ipmu_calib_data->f2_nominal);
    mask = MASK_BITS(22, 0);
    value = ro_ts_efuse[2];
    value &= ~mask;
    value |= data;
    ro_ts_efuse[2] = value;

    /*  ro_tempsense_config */
    data = (ipmu_calib_data->str_temp_slope);
    mask = MASK_BITS(22, 0);
    value = ro_tempsense_config[2];
    value &= ~mask;
    value |= data;
    ro_tempsense_config[2] = value;

#ifdef AT_EFUSE_DATA_1P19
    /*BJT temperature sensor efuse value update*/
    /*  delvbe_ts_efuse*/
    data = (ipmu_calib_data->str_bjt_temp_sense_off);
    mask = MASK_BITS(22, 0);
    value = delvbe_tsbjt_efuse[2];
    value &= ~mask;
    value |= data;
    delvbe_tsbjt_efuse[2] = value;

    /* vbg_ts_efuse */
    data = (ipmu_calib_data->vbg_tsbjt_efuse);
    mask = MASK_BITS(22, 0);
    value = vbg_tsbjt_efuse[2];
    value &= ~mask;
    value |= data;
    vbg_tsbjt_efuse[2] = value;
#endif

    /* retn_ldo_lptrim */
    data = (ipmu_calib_data->retn_ldo_lptrim);
    mask = MASK_BITS(22, 0);
    value = retnLP_volt_trim_efuse[2];
    value &= ~mask;
    value |= data;
    retnLP_volt_trim_efuse[2] = value;

    /* auxadc_off_diff_efuse  */
    data = (ipmu_calib_data->auxadc_offset_diff);
    mask = MASK_BITS(22, 0);
    value = auxadc_off_diff_efuse[2];
    value &= ~mask;
    value |= data;
    auxadc_off_diff_efuse[2] = value;

    /* auxadc_gain_diff_efuse */
    data = (ipmu_calib_data->auxadc_invgain_diff);
    mask = MASK_BITS(22, 0);
    value = auxadc_gain_diff_efuse[2];
    value &= ~mask;
    value |= data;
    auxadc_gain_diff_efuse[2] = value;

    /* auxadc_off_se_efuse */
    data = (ipmu_calib_data->auxadc_offset_single);
    mask = MASK_BITS(22, 0);
    value = auxadc_off_se_efuse[2];
    value &= ~mask;
    value |= data;
    auxadc_off_se_efuse[2] = value;

    /* auxadc_gain_se_efuse */
    data = (ipmu_calib_data->auxadc_invgain_single);
    mask = MASK_BITS(22, 0);
    value = auxadc_gain_se_efuse[2];
    value &= ~mask;
    value |= data;
    auxadc_gain_se_efuse[2] = value;

    /* rc_32khz_trim_efuse  */
    data = (uint32_t)(ipmu_calib_data->coarse_trim_32k | ipmu_calib_data->fine_trim_32k << 2);
    mask = MASK_BITS(22, 0);
    value = rc_32khz_trim_efuse[2];
    value &= ~mask;
    value |= data;
    rc_32khz_trim_efuse[2] = value;

#ifndef AT_EFUSE_DATA_1P19
    /*If the 13 bits of WuRx(i.e.reserved1) in E-Fuse are zeros, 
    then overwrite this structure with the contents from RO32K_EFUSE*/
    if(!((ipmu_calib_data->reserved1) >> 7)) {
        ro_32khz_trim00_efuse[2] = ro_32khz_trim_efuse[2];
    } else {
        /*then write the contents of the (reserved1) last 5 bits
      allotted to RO32K_00 */
        data = ((ipmu_calib_data->reserved1) & (0xF80));
        mask = MASK_BITS(22, 0);
        value = ro_32khz_trim00_efuse[2];
        value &= ~mask;
        value |= data;
        ro_32khz_trim00_efuse[2] = value;
    }
#endif

#ifdef AT_EFUSE_DATA_1P19

    /*If the 13 bits of WuRx(i.e.reserved1) in E-Fuse are zeros, 
    then overwrite this structure with the contents from RO32K_EFUSE*/
    if(!(ipmu_calib_data->ro_32khz_00_trim)) {
        ro_32khz_trim00_efuse[2] = ro_32khz_trim_efuse[2];
    } else {
        /*then write the contents of the (reserved1) last 5 bits
      allotted to RO32K_00 */
        data = (ipmu_calib_data->ro_32khz_00_trim);
        mask = MASK_BITS(22, 0);
        value = ro_32khz_trim00_efuse[2];
        value &= ~mask;
        value |= data;
        ro_32khz_trim00_efuse[2] = value;
    }
#endif

    data = ipmu_calib_data->set_vref1p3;
    mask = MASK_BITS(22, 0);
    value = buck_trim_efuse[2];
    value &= ~mask;
    value |= data;
    buck_trim_efuse[2] = value;

    data = ipmu_calib_data->trim_r1_resistorladder;
    mask = MASK_BITS(22, 0);
    value = ldosoc_trim_efuse[2];
    value &= ~mask;
    value |= data;
    ldosoc_trim_efuse[2] = value;

    data = ipmu_calib_data->dpwm_freq_trim;
    mask = MASK_BITS(22, 0);
    value = dpwm_freq_trim_efuse[2];
    value &= ~mask;
    value |= data;
    dpwm_freq_trim_efuse[2] = value;

#ifdef AT_EFUSE_DATA_1P19
    /* scdc_hpldo_trim */
    data = ipmu_calib_data->scdc_hpldo_trim;
    mask = MASK_BITS(22, 0);
    value = hpldo_volt_trim_efuse[2];
    value &= ~mask;
    value |= data;
    hpldo_volt_trim_efuse[2] = value;

    /* scdc_dcdc_trim */
    data = ipmu_calib_data->scdc_dcdc_trim;
    mask = MASK_BITS(22, 0);
    value = scdc_volt_trim_efuse[2];
    value &= ~mask;
    value |= data;
    scdc_volt_trim_efuse[2] = value;
#endif

    return;
}

/*==============================================*/
/**
 * @fn         void RSI_IPMU_InitCalibData(void)
 * @brief      This function checks for magic byte in efuse/flash, copies the content if valid data present and calls to update the ipmu registers
 * @return     none
 */
void RSI_IPMU_InitCalibData(void) {
    efuse_ipmu_t global_ipmu_calib_data;
#ifdef CHIP_9118
    uint8_t* calib_data_read_adr = NULL;

    /* Read the MCU boot status register */
    volatile retention_boot_status_word_t* retention_reg =
        (retention_boot_status_word_t*)MCURET_BOOTSTATUS;
    /* Read the NWP BBFFs storage register */
    volatile npss_boot_status_word0_t* npss_boot_status =
        (npss_boot_status_word0_t*)MCU_BBFF_STORAGE1_ADDR;

    if(retention_reg->product_mode == MCU) {
        calib_data_read_adr = (uint8_t*)(MCU_MANF_DATA_BASE_ADDR);
    }
    if(retention_reg->product_mode == WISEMCU) {
        if(npss_boot_status->ta_flash_present) {
            calib_data_read_adr = (uint8_t*)(TA_MANF_DATA_BASE_ADDR);
        } else {
            calib_data_read_adr = (uint8_t*)(MCU_MANF_DATA_BASE_ADDR);
        }
    }

    /* Checks the Calibration values are present at MCU flash */
    if((*(uint32_t*)(calib_data_read_adr + IPMU_VALUES_OFFSET)) == 0x00) {
        //NO CALIB DATA. Return
        return;
    }
    /* Checks the `Calibration values are present at MCU flash */
    if((*(uint32_t*)(calib_data_read_adr + IPMU_VALUES_OFFSET)) == 0xFFFFFFFF) {
        //NO CALIB DATA. Return
        return;
    }

    memcpy(
        (void*)&global_ipmu_calib_data,
        (void*)(calib_data_read_adr + IPMU_VALUES_OFFSET),
        sizeof(efuse_ipmu_t));
    //rsi_cmd_m4_ta_secure_handshake(2,0,NULL,sizeof(efuse_ipmu_t),(uint8_t *)&global_ipmu_calib_data);
#endif

#if defined(SLI_SI917)
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
    /* Checks the Calibration values are present at MCU flash */
    if((*(uint32_t*)(COMMON_FLASH_IPMU_VALUES_OFFSET)) == 0x00) {
        //NO CALIB DATA. Return
        return;
    }
    memcpy(
        (void*)&global_ipmu_calib_data,
        (void*)(COMMON_FLASH_IPMU_VALUES_OFFSET),
        sizeof(efuse_ipmu_t));
#else
    /* Checks the Calibration values are present at MCU flash */
    if((*(uint32_t*)(DUAL_FLASH_IPMU_VALUES_OFFSET)) == 0x00) {
        //NO CALIB DATA. Return
        return;
    }
    memcpy(
        (void*)&global_ipmu_calib_data,
        (void*)(DUAL_FLASH_IPMU_VALUES_OFFSET),
        sizeof(efuse_ipmu_t));
#endif
#endif

    //Dummy read
    (void)PMU_SPI_DIRECT_ACCESS(PMU_PFM_REG_OFFSET);

    RSI_IPMU_UpdateIpmuCalibData_efuse(&global_ipmu_calib_data);

    //Dummy read
    (void)PMU_SPI_DIRECT_ACCESS(PMU_PFM_REG_OFFSET);
}

/*==============================================*/
/**
 * @brief   This function Initialize IPMU and MCU FSM blocks
 * @param   void
 * @return  void
 */
void RSI_Ipmu_Init(void) {
    ipmu_init();
}

/**
 * @brief   This function configures chip supply mode
 * @param   void
 * @return  void
 */
void RSI_Configure_Ipmu_Mode(void) {
    double temperature = 0;

#if defined(ENABLE_1P8V)
    (void)temperature;
    /*configures chip supply mode to HP-LDO */
    configure_ipmu_mode(HP_LDO_MODE);
#else
    /* Read the temperature; if it is within the range of 0 to 60 degrees, switch the chip supply to SCDC mode. Otherwise, maintain the default LDO supply mode.*/
    // sl_si91x_bjt_temperature_sensor_init(sl_bjt_channel_config, sl_bjt_config);
    // sl_si91x_bjt_temperature_sensor_read_data(&temperature);
    // sl_si91x_bjt_temperature_sensor_deinit(sl_bjt_config);
    if((temperature > 0) && (temperature <= 60)) {
        /*configures chip supply mode to SCDC */
        configure_ipmu_mode(SCDC_MODE);
    }
#endif
}
void update_efuse_system_configs(int data, uint32_t config_ptr[]) {
    uint32_t mask = 0;
    uint32_t value = 0;
    mask = MASK_BITS(22, 0);
    value = config_ptr[2];
    value &= ~mask;
    value |= (uint32_t)data;
    config_ptr[2] = value;
}
/**
 * @brief   This function configures the Lower voltage level for DC-DC to 1.25V based on the DC-DC Trim value(for 1.35V) obtained from Calibration
 * @return  none
 */

void RSI_Configure_DCDC_LowerVoltage(void) {
    uint32_t pmu_1p2_ctrl_word;
    uint32_t bypass_curr_ctrl_data;

    bypass_curr_ctrl_data = PMU_SPI_DIRECT_ACCESS(PMU_1P3_CTRL_REG_OFFSET);
    pmu_1p2_ctrl_word = ((bypass_curr_ctrl_data >> 17) & 0xF) - 2;
    bypass_curr_ctrl_data = PMU_SPI_DIRECT_ACCESS(BYPASS_CURR_CTRL_REG_OFFSET);
    bypass_curr_ctrl_data &= (uint32_t)(~(0xF << 5));
    PMU_SPI_DIRECT_ACCESS(BYPASS_CURR_CTRL_REG_OFFSET) =
        (bypass_curr_ctrl_data | (pmu_1p2_ctrl_word << 5));
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_PowerGateSet(uint32_t mask_vlaue)
 * @brief     This API is used to power-up the IPMU peripherals.
 * @param[in]   mask_vlaue   : Ored value of peripheral power gate defines
 *                             Possible values for this parameter are the following
 *                             CMP_NPSS_PG_ENB
 *                             ULP_ANG_CLKS_PG_ENB
 *                             ULP_ANG_PWRSUPPLY_PG_ENB
 *                             WURX_PG_ENB
 *                             WURX_CORR_PG_ENB
 *                             AUXADC_PG_ENB
 *                             AUXADC_BYPASS_ISO_GEN
 *                             AUXADC_ISOLATION_ENABLE
 *                             AUXDAC_PG_ENB
 * @return      none
 */

void RSI_IPMU_PowerGateSet(uint32_t mask_vlaue) {
    uint32_t impuPowerGate = 0;
    impuPowerGate = ULP_SPI_MEM_MAP(POWERGATE_REG_WRITE);
    while(GSPI_CTRL_REG1 & SPI_ACTIVE)
        ;
    impuPowerGate = (impuPowerGate >> 5);
    impuPowerGate |= mask_vlaue;
    ULP_SPI_MEM_MAP(POWERGATE_REG_WRITE) = impuPowerGate;
    while(GSPI_CTRL_REG1 & SPI_ACTIVE)
        ;
    /*Dummy read*/
    impuPowerGate = ULP_SPI_MEM_MAP(POWERGATE_REG_WRITE);
    (void)impuPowerGate;
    return;
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_PowerGateClr(uint32_t mask_vlaue)
 * @brief     This API is used to power-down the IPMU peripherals.
 * @param[in]   mask_vlaue   : Ored value of peripheral power gate defines
 *                             Possible values for this parameter are the following
 *                             CMP_NPSS_PG_ENB
 *                             ULP_ANG_CLKS_PG_ENB
 *                             ULP_ANG_PWRSUPPLY_PG_ENB
 *                             WURX_PG_ENB
 *                             WURX_CORR_PG_ENB
 *                             AUXADC_PG_ENB
 *                             AUXADC_BYPASS_ISO_GEN
 *                             AUXADC_ISOLATION_ENABLE
 *                             AUXDAC_PG_ENB
 * @return      none
 */

void RSI_IPMU_PowerGateClr(uint32_t mask_vlaue) {
    uint32_t impuPowerGate = 0;

    if(mask_vlaue & (WURX_CORR_PG_ENB | WURX_PG_ENB)) {
        ULP_SPI_MEM_MAP(0x141) &= ~BIT(20);
        ULP_SPI_MEM_MAP(0x141) &= ~BIT(21);
    }

    impuPowerGate = ULP_SPI_MEM_MAP(POWERGATE_REG_WRITE);
    while(GSPI_CTRL_REG1 & SPI_ACTIVE)
        ;
    impuPowerGate = (impuPowerGate >> 5);
    impuPowerGate &= ~mask_vlaue;
    ULP_SPI_MEM_MAP(POWERGATE_REG_WRITE) = impuPowerGate;
    while(GSPI_CTRL_REG1 & SPI_ACTIVE)
        ;
    return;
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_ClockMuxSel(uint8_t bg_pmu_clk)
 * @brief     This API is used to select clock to the BG-PMU
 * @param[in]   bg_pmu_clk : Selects the clock source to the BG-PMU module
 *          1: RO 32KHz clock
 *          2: MCU FSM clock
 * @return      none
 */

void RSI_IPMU_ClockMuxSel(uint8_t bg_pmu_clk) {
    bg_pmu_clk = (bg_pmu_clk - 1);
    ULP_SPI_MEM_MAP(SELECT_BG_CLK) &= ~(BIT(0) | BIT(1));
    while(GSPI_CTRL_REG1 & SPI_ACTIVE)
        ;
    ULP_SPI_MEM_MAP(SELECT_BG_CLK) |= BIT(bg_pmu_clk);
    while(GSPI_CTRL_REG1 & SPI_ACTIVE)
        ;
    return;
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_32MHzClkClib(void)
 * @brief     This API is used to auto calibrate the 32MHz RC clock
 * @return      trim value on success
 */

uint32_t RSI_IPMU_32MHzClkClib(void) {
    volatile int i;
    volatile int trim_value = 0;
    /*Enables RC 32MHz clock and*/
    ULP_SPI_MEM_MAP(0x104) = (0x3FFFFF & 0x41368000);
    /*Enable XTAL 40MHz clock through NPSS*/
    *(volatile uint32_t*)0x41300120 |= BIT(22);
    i = 1000000;
    while(i--)
        ;
    /*Selects NPSS reference clock to be CLK-40M_SOC*/
    ULP_SPI_MEM_MAP(0x106) = (0x3FFFFF & 0x41A48000);
    /*Change spi trim select to 0*/
    ULP_SPI_MEM_MAP(0x107) = (0x3FFFFF & 0x41C04A14);
    /*Pointing clks test out 1 to RC 32M clock*/
    ULP_SPI_MEM_MAP(0x10D) = (0x3FFFFF & 0x43600000);
    /*Pointing clks test out to IPMU_TEST_OUT_0 = SOC[8] in mode 5*/
    ULP_SPI_MEM_MAP(0x143) = (0x3FFFFF & 0x50C00610);
    /*Enable the high frequency clock calibration
   * Enable the clock gate for npss ref clk
   * Select the RC32M clock to calibrate
   * */
    ULP_SPI_MEM_MAP(0x10A) = (0x3FFFFF & 0x42922290);
    i = 100000;
    while(i--)
        ;
    do {
        /*wait for calibration done indication*/
    } while((!(ULP_SPI_MEM_MAP(0x30C))) & BIT(20));
    /*Calibrated trim value*/
    trim_value = (int)ULP_SPI_MEM_MAP(0x30C);
    trim_value = (trim_value >> 11);
    trim_value = (trim_value & 0x7F);
    /*Programming the calibrated trim to SPI register.*/
    ULP_SPI_MEM_MAP(0x104) |= (uint32_t)(trim_value << 14);
    /*pointing the trim select to SPI*/
    ULP_SPI_MEM_MAP(0x107) = (0x3FFFFF & 0x41C05A14);
    return (uint32_t)trim_value;
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_ProgramConfigData(const uint32_t *config)
 * @brief     This API is used to program the any mcu configuration structure
 * @param[in]   config : pointer configuration
 * @return      RSI_OK on success
 */

rsi_error_t RSI_IPMU_ProgramConfigData(const uint32_t* config) {
    uint32_t index = 0;
    uint32_t program_len = 0;
    uint32_t reg_addr = 0;
    uint32_t reg_write_data = 0;
    uint32_t clear_cnt = 0;
    uint32_t cnt = 0;
    uint32_t reg_read_data = 0;
    uint32_t write_mask = 0;
    uint32_t write_bit_pos = 0;
    uint8_t msb = 0;
    uint8_t lsb = 0;

    if(config == NULL) {
        return INVALID_PARAMETERS;
    }
    /*Compute the number of entries in the array to program*/
    program_len = config[index];
    if(program_len == 0U) {
        return INVALID_PARAMETERS;
    }
    for(index = 0; index < program_len; index++) {
        reg_addr = config[(2U * index) + 1];
        reg_write_data = config[2U * (index + 1)];

        lsb = ((reg_write_data >> LSB_POSITION) & POSITION_BITS_MASK);
        msb = ((reg_write_data >> MSB_POSITION) & POSITION_BITS_MASK);

        clear_cnt = (msb - lsb) + 1U;
        /*MSB and LSB position counts validation */
        if(clear_cnt > MAX_BIT_LEN) {
            // Return error
            return INVALID_PARAMETERS;
        }
        /*Read register*/
        reg_read_data = *(volatile uint32_t*)reg_addr;
        cnt = lsb;
        write_mask = 0;
        write_bit_pos = 0;
        do {
            reg_read_data &= ~BIT(cnt);
            write_mask |= BIT(write_bit_pos);
            cnt++;
            write_bit_pos++;
        } while(cnt < (clear_cnt + lsb));
        reg_write_data &= write_mask;
        /*Write to the hardware register*/
        reg_write_data = (reg_read_data | (reg_write_data << lsb));
        *(volatile uint32_t*)reg_addr = reg_write_data;
    }
    return RSI_OK;
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_APB_ProgramConfigData(const uint32_t *config)
 * @brief     This API is used to program the any mcu configuration structure
 * @param[in]   config : pointer configuration 
 * @return      reg_write_data on success.
 */

uint32_t RSI_APB_ProgramConfigData(const uint32_t* config) {
    uint32_t index = 0;
    uint32_t program_len = 0;
    uint32_t clear_cnt = 0;
    uint32_t cnt = 0;
    uint32_t reg_write_data = 0;
    uint32_t write_mask = 0;
    uint32_t write_bit_pos = 0;
    uint8_t msb = 0;
    uint8_t lsb = 0;

    if(config == NULL) {
        return INVALID_PARAMETERS;
    }
    /*Compute the number of entries in the array to program*/
    program_len = config[index];
    if(program_len == 0U) {
        return INVALID_PARAMETERS;
    }
    for(index = 0; index < program_len; index++) {
        reg_write_data = config[2U * (index + 1)];

        lsb = ((reg_write_data >> LSB_POSITION) & POSITION_BITS_MASK);
        msb = ((reg_write_data >> MSB_POSITION) & POSITION_BITS_MASK);

        clear_cnt = (msb - lsb) + 1U;
        /*MSB and LSB position counts validation */
        if(clear_cnt > MAX_BIT_LEN) {
            // Return error
            return INVALID_PARAMETERS;
        }
        cnt = lsb;
        write_mask = 0;
        write_bit_pos = 0;
        do {
            write_mask |= BIT(write_bit_pos);
            cnt++;
            write_bit_pos++;
        } while(cnt < (clear_cnt + lsb));
        reg_write_data &= write_mask;
        /*Write to the hardware register*/
        reg_write_data = reg_write_data << lsb;
    }
    return reg_write_data;
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_CommonConfig(void)
 * @brief     This API is used to program the default system start-up IPMU hardware programming.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_CommonConfig(void) {
    return RSI_IPMU_ProgramConfigData(ipmu_common_config);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_PMUCommonConfig(void)
 * @brief     This API is used to program the default system start-up PMU hardware programming.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_PMUCommonConfig(void) {
    return RSI_IPMU_ProgramConfigData(pmu_common_config);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_PocbiasCurrent(void)
 * @brief     This API is used to Decreasing the bias current of RETN_LDO .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_PocbiasCurrent(void) {
    return RSI_IPMU_ProgramConfigData(poc_bias_current);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_PocbiasCurrent11(void)
 * @brief     This API is used to increasing the bias current of RETN_LDO .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_PocbiasCurrent11(void) {
    return RSI_IPMU_ProgramConfigData(poc_bias_current_11);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RO32khzTrim00Efuse(void)
 * @brief     This API is used to trim to 32Khz RO .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_RO32khzTrim00Efuse(void) {
    return RSI_IPMU_ProgramConfigData(ro_32khz_trim00_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RetnLdoVoltsel(void)
 * @brief     This API is used to setting RETN_LDO voltage to 0.7V.
 * @return      RSI_IPMU_ProgramConfigData on success .
 */

rsi_error_t RSI_IPMU_RetnLdoVoltsel(void) {
    return RSI_IPMU_ProgramConfigData(retn_ldo_voltsel);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RetnLdo0p75(void)
 * @brief     This API is used to setting RETN_LDO voltage to 0.75V .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_RetnLdo0p75(void) {
    return RSI_IPMU_ProgramConfigData(retn_ldo_0p75);
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_RetnLdoLpmode(void) 
 * @brief     This API is used to program enabling the RETN_LDO LP MODE  .
 * @return      none
 */

void RSI_IPMU_RetnLdoLpmode(void) {
    RSI_IPMU_ProgramConfigData(retn_ldo_lpmode);
    RSI_IPMU_ProgramConfigData(retnLP_volt_trim_efuse);
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_Retn_Voltage_Reduction(void)
 * @brief     This API is used to reduce the RETN_LDO voltage by 0.05V.
 * @return      none
 */

void RSI_IPMU_Retn_Voltage_Reduction(void) {
    uint32_t value;
    uint32_t mask;
    value = retnLP_volt_trim_efuse[2];
    mask = MASK_BITS(3, 0);
    value &= mask;
    if(value < RET_LDO_TRIM_VALUE_CHECK) {
        retnLP_volt_trim_efuse[2] += RET_LDO_VOL_DECREASE;
    }
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_Retn_Voltage_To_Default(void)
 * @brief     This API is used to change the RETN_LDO voltage to 0.8V.
 * @return      none
 */

void RSI_IPMU_Retn_Voltage_To_Default(void) {
    uint32_t mask;
    mask = MASK_BITS(22, 0);
    retnLP_volt_trim_efuse[2] &= ~mask;
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_Set_Higher_Pwm_Ro_Frequency_Mode_to_PMU(void)
 * @brief     This API is used to enable Higher Pwm Ro Frequency Mode for PMU.
 * @return      none
 */

void RSI_IPMU_Set_Higher_Pwm_Ro_Frequency_Mode_to_PMU(void) {
    PMU_SPI_MEM_MAP(PMU_FREQ_MODE_REG) &= ~(LOW_FREQ_PWM);
}
/*==============================================*/
/**
 * @fn          void RSI_IPMU_RetnLdoHpmode(void)
 * @brief     This API is used to program enabling the RETN_LDO HP MODE .
 * @return      none
 */

void RSI_IPMU_RetnLdoHpmode(void) {
    RSI_IPMU_ProgramConfigData(retn_ldo_hpmode);
    RSI_IPMU_ProgramConfigData(retnHP_volt_trim_efuse);
}
/**
 * @fn         void RSI_IPMU_SCDC_Enable(void)
 * @brief     This API is used to enable SCDC mode .
 * @return      none
 */
void RSI_IPMU_SCDC_Enable(void) {
    RSI_IPMU_ProgramConfigData(ipmu_scdc_enable);
}
/**
 * @fn         void RSI_IPMU_HP_LDO_Enable(void)
 * @brief     This API is used to enable LDO mode .
 * @return      none
 */
void RSI_IPMU_HP_LDO_Enable(void) {
    RSI_IPMU_ProgramConfigData(hp_ldo_voltsel);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_M32rc_OscTrimEfuse(void)
 * @brief     This API is used to program the trim value for 32Mhz RC oscillator .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_M32rc_OscTrimEfuse(void) {
    system_clocks.rc_mhz_clock = DEFAULT_MHZ_RC_CLOCK;

    if(system_clocks.m4_ref_clock_source == ULP_MHZ_RC_CLK) {
        system_clocks.m4ss_ref_clk = DEFAULT_MHZ_RC_CLOCK;
    }
    if(system_clocks.ulp_ref_clock_source == ULPSS_ULP_MHZ_RC_CLK) {
        system_clocks.ulpss_ref_clk = DEFAULT_MHZ_RC_CLOCK;
    }

    return RSI_IPMU_ProgramConfigData(m32rc_osc_trim_efuse);
}

/*==============================================*/
/**
 * @fn        rsi_error_t RSI_IPMU_M20rcOsc_TrimEfuse(void)
 * @brief     This API is used to program the trim value for 20MHz RC oscillator
 * @return    RSI_IPMU_ProgramConfigData on success
 */

rsi_error_t RSI_IPMU_M20rcOsc_TrimEfuse(void) {
    rsi_error_t error_status;

    error_status = RSI_IPMU_ProgramConfigData(m20rc_osc_trim_efuse);

    if(error_status == RSI_OK) {
        system_clocks.rc_mhz_clock = SYSTEM_CLK_VAL_20MHZ;
        if(system_clocks.m4_ref_clock_source == ULP_MHZ_RC_CLK) {
            system_clocks.m4ss_ref_clk = SYSTEM_CLK_VAL_20MHZ;
        }
        if(system_clocks.ulp_ref_clock_source == ULPSS_ULP_MHZ_RC_CLK) {
            system_clocks.ulpss_ref_clk = SYSTEM_CLK_VAL_20MHZ;
        }
    }

    return error_status;
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_DBLR32M_TrimEfuse(void)
 * @brief     This API is used to program DBLR 32MHz trim value  .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_DBLR32M_TrimEfuse(void) {
    system_clocks.doubler_clock = SYSTEM_CLK_VAL_MHZ;
    return RSI_IPMU_ProgramConfigData(dblr_32m_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_M20roOsc_TrimEfuse(void)
 * @brief     This API is used to program the trim value for 20Mhz RO oscillator .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_M20roOsc_TrimEfuse(void) {
    system_clocks.ro_20mhz_clock = DEFAULT_20MHZ_RO_CLOCK;
    if(system_clocks.m4_ref_clock_source == ULP_20MHZ_RINGOSC_CLK) {
        system_clocks.m4ss_ref_clk = DEFAULT_20MHZ_RO_CLOCK;
    }
    if(system_clocks.ulp_ref_clock_source == ULPSS_ULP_20MHZ_RINGOSC_CLK) {
        system_clocks.ulpss_ref_clk = DEFAULT_20MHZ_RO_CLOCK;
    }
    return RSI_IPMU_ProgramConfigData(m20ro_osc_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RO32khz_TrimEfuse(void)
 * @brief     This API is used to program the trim value for 32KHz RO oscillator .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_RO32khz_TrimEfuse(void) {
    system_clocks.ro_32khz_clock = DEFAULT_32KHZ_RO_CLOCK;

    return RSI_IPMU_ProgramConfigData(ro_32khz_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RC16khz_TrimEfuse(void)
 * @brief     This API is used to program the trim value for 16KHz RC oscillator .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_RC16khz_TrimEfuse(void) {
    system_clocks.rc_32khz_clock = 16000;
    return RSI_IPMU_ProgramConfigData(rc_16khz_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RC64khz_TrimEfuse(void)
 * @brief     This API is used to program the trim value for 64KHz RC oscillator .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_RC64khz_TrimEfuse(void) {
    system_clocks.rc_32khz_clock = 64000;
    return RSI_IPMU_ProgramConfigData(rc_64khz_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RC32khz_TrimEfuse(void)
 * @brief     This API is used to program the trim value for 32KHz RC oscillator .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_RC32khz_TrimEfuse(void) {
    system_clocks.rc_32khz_clock = 32000;
    return RSI_IPMU_ProgramConfigData(rc_32khz_trim_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_RO_TsEfuse()
 * @brief     This API is used to program the nominal_count .
 * @return      RSI_APB_ProgramConfigData on success.
 */

uint32_t RSI_IPMU_RO_TsEfuse() {
    return RSI_APB_ProgramConfigData(ro_ts_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_RO_TsConfig(void)
 * @brief     This API is used to program the RO Temp sensor slope .
 * @return      RSI_APB_ProgramConfigData on success.
 */

uint32_t RSI_IPMU_RO_TsConfig(void) {
    return RSI_APB_ProgramConfigData(ro_tempsense_config);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Vbattstatus_TrimEfuse(void)
 * @brief     This API is used to program the trim value for Vbatt status .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Vbattstatus_TrimEfuse(void) {
    return RSI_IPMU_ProgramConfigData(vbatt_status_trim_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_Vbg_Tsbjt_Efuse(void)
 * @brief     This API is used to program the BG voltage for BJT temperature sensor
 * @return      RSI_APB_ProgramConfigData on success.
 */

uint32_t RSI_IPMU_Vbg_Tsbjt_Efuse(void) {
    return RSI_APB_ProgramConfigData(vbg_tsbjt_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_Auxadcoff_DiffEfuse(void)
 * @brief     This API is used to program The offset value for AUX ADC differential mode .
 * @return      RSI_APB_ProgramConfigData on success.
 */

uint32_t RSI_IPMU_Auxadcoff_DiffEfuse(void) {
    return RSI_APB_ProgramConfigData(auxadc_off_diff_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_Auxadcgain_DiffEfuse(void)
 * @brief     This API is used to program The gain value for AUX ADC differential mode   .
 * @return      RSI_APB_ProgramConfigData on success.
 */
uint32_t RSI_IPMU_Auxadcgain_DiffEfuse(void) {
    return RSI_APB_ProgramConfigData(auxadc_gain_diff_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_Auxadcoff_SeEfuse(void)
 * @brief     This API is used to program The offset value for AUX ADC single ended mode    .
 * @return      RSI_APB_ProgramConfigData on success.
 */

uint32_t RSI_IPMU_Auxadcoff_SeEfuse(void) {
    return RSI_APB_ProgramConfigData(auxadc_off_se_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_Auxadcgain_SeEfuse(void)
 * @brief     This API is used to program The gain value for AUX ADC single mode    .
 * @return      RSI_APB_ProgramConfigData on success.
 */

uint32_t RSI_IPMU_Auxadcgain_SeEfuse(void) {
    return RSI_APB_ProgramConfigData(auxadc_gain_se_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Bg_TrimEfuse(void)
 * @brief     This API is used to program BG trim value.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Bg_TrimEfuse(void) {
    return RSI_IPMU_ProgramConfigData(bg_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Blackout_TrimEfuse(void)
 * @brief     This API is used to program BLACKOUT thresholds .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Blackout_TrimEfuse(void) {
    return RSI_IPMU_ProgramConfigData(blackout_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_POCbias_Efuse(void)
 * @brief     This API is used to program the poc bias.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_POCbias_Efuse(void) {
    return RSI_IPMU_ProgramConfigData(poc_bias_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Buck_TrimEfuse(void)
 * @brief     This API is used to program BUCK value .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Buck_TrimEfuse(void) {
    return RSI_IPMU_ProgramConfigData(buck_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Ldosoc_TrimEfuse(void)
 * @brief     This API is used to program LDO SOC value .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Ldosoc_TrimEfuse(void) {
    return RSI_IPMU_ProgramConfigData(ldosoc_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Dpwmfreq_TrimEfuse(void)
 * @brief     This API is used to program DPWM frequency value .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Dpwmfreq_TrimEfuse(void) {
    return RSI_IPMU_ProgramConfigData(dpwm_freq_trim_efuse);
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_IPMU_Delvbe_Tsbjt_Efuse(void)
 * @brief     This API is used to program the offset voltage
 * @return      RSI_APB_ProgramConfigData on success.
 */

uint32_t RSI_IPMU_Delvbe_Tsbjt_Efuse(void) {
    return RSI_APB_ProgramConfigData(delvbe_tsbjt_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Xtal1bias_Efuse(void)
 * @brief     This API is used to program Xtal bias value .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Xtal1bias_Efuse(void) {
    return RSI_IPMU_ProgramConfigData(xtal1_bias_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Xtal2bias_Efuse(void)
 * @brief     This API is used to program Xtal2 bias value .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Xtal2bias_Efuse(void) {
    return RSI_IPMU_ProgramConfigData(xtal2_bias_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_RetnHP_Volttrim_Efuse(void)
 * @brief     This API is used to program the HP retention voltage .
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_RetnHP_Volttrim_Efuse(void) {
    return RSI_IPMU_ProgramConfigData(retnHP_volt_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_BOD_ClksCommonconfig1(void)
 * @brief     This API is used to enable bias currents to BOD.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_BOD_ClksCommonconfig1(void) {
    return RSI_IPMU_ProgramConfigData(ipmu_bod_clks_common_config1);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_BOD_ClksCommonconfig2(void)
 * @brief     This API is used to disable bias currents to BOD.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_BOD_ClksCommonconfig2(void) {
    return RSI_IPMU_ProgramConfigData(ipmu_bod_clks_common_config2);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Hpldo_volt_trim_efuse(void)
 * @brief     This API is used to set the LDO to the correct voltage based on E-Fuse.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Hpldo_volt_trim_efuse(void) {
    return RSI_IPMU_ProgramConfigData(hpldo_volt_trim_efuse);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Scdc_volt_trim_efuse(void))
 * @brief     This API is used to set the SC-DCDC to the corrrect voltage based on E-Fuse.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Scdc_volt_trim_efuse(void) {
    return RSI_IPMU_ProgramConfigData(scdc_volt_trim_efuse);
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_Reconfig_to_SCDCDC(void)
 * @brief     This API is used to the LDO has to be re-configured into SC-DCDC mode.
 * @return      none
 */

void RSI_IPMU_Reconfig_to_SCDCDC(void) {
    RSI_IPMU_ProgramConfigData(scdc_volt_sel1);
    RSI_IPMU_ProgramConfigData(scdc_volt_trim_efuse);
    RSI_IPMU_ProgramConfigData(scdc_volt_sel2);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_Lp_scdc_extcapmode(void)
 * @brief     This API is used to SCDC into the external CAP mode.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_Lp_scdc_extcapmode(void) {
    return RSI_IPMU_ProgramConfigData(lp_scdc_extcapmode);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_IPMU_BOD_Cmphyst(void)
 * @brief     This API is used to set the comparator hysteresis.
 * @return      RSI_IPMU_ProgramConfigData on success.
 */

rsi_error_t RSI_IPMU_BOD_Cmphyst(void) {
    return RSI_IPMU_ProgramConfigData(bod_cmp_hyst);
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_32KHzROClkClib(void)
 * @brief       This API is used to calibrate the 32Khz RO clock to 32khz frequency .
 * @return      none
 */

void RSI_IPMU_32KHzROClkClib(void) {
    uint32_t ro32k_trim = 0;
    uint32_t no_of_tst_clk_khz_ro = 0;

    /*Do until clock should be 32KHz */
    do {
        /* CLK40M buffer circuit will be ON */
        ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) |= BIT(21);
        /*  Off controls for clock cleaner are taken from NPSS */
        ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) &= ~BIT(16);
        /* RO 32KHz clock enable */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRO_CLK_REG_OFFSET) |=
            BIT(21); /* 32KHz RO clock is not supported */
        /* calibrated trim goes to the block */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRO_CLK_REG_OFFSET) &=
            ~(BIT(15)); /* 32KHz RO clock is not supported */
        /* Enable the clock gate for npss ref clk &wait for 1us*/
        /*  32KHz RO clock calibration */
        /* Binary search calibration enable signal for low frequency clocks RO and RC */
        ULP_SPI_MEM_MAP(ULPCLKS_CALIB_REG_ADDR) |= (BIT(4) | BIT(15) | BIT(21));
        /* wait till bit 21 becomes 1 Indicates calibration done indication*/
        while(!(ULP_SPI_MEM_MAP(ULPCLKS_CALIB_DONE_REG_ADDR) & BIT(21)))
            ;
        /* Read calibrated trim value after low frequency calibration done */
        ro32k_trim = ((ULP_SPI_MEM_MAP(ULPCLKS_CALIB_DONE_REG_ADDR) & RO_TRIM_VALUE_LF) >> 4);
        /*Mask the bits where the trim value need to write */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRO_CLK_REG_OFFSET) &=
            (uint32_t)(~MASK32KRO_TRIM_VALUE_WRITE_BITS); /* 32KHz RO clock is not supported */
        /* Programming the calibrated trim to SPI register. */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRO_CLK_REG_OFFSET) |=
            (ro32k_trim << 16); /* 32KHz RO clock is not supported */
        /*  trim given from spi goes to the block */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRO_CLK_REG_OFFSET) |=
            (BIT(15)); /* 32KHz RO clock is not supported */
        /* Measures MHz RC clock Clock Frequency  */
        no_of_tst_clk_khz_ro = RSI_Clks_Calibration(sleep_clk, khz_ro_clk);
        no_of_tst_clk_khz_ro /= 1000;
        /* Check if it is less than a particular value */
        if(no_of_tst_clk_khz_ro < PARTICULAR_FREQ_MIN) {
            ULP_SPI_MEM_MAP(iPMU_SPARE_REG1_OFFSET) |= BIT(18) | BIT(19);
        }
        /* Check if it is greater than a particular value */
        if(no_of_tst_clk_khz_ro > PARTICULAR_FREQ_MAX) {
            ULP_SPI_MEM_MAP(iPMU_SPARE_REG1_OFFSET) &= ~(BIT(18) | BIT(19));
        }
    } while((no_of_tst_clk_khz_ro < PARTICULAR_FREQ_MIN) ||
            (no_of_tst_clk_khz_ro > PARTICULAR_FREQ_MAX));

    return;
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_32KHzRCClkClib(void)
 * @brief       This API is used to calibrate the 32Khz RC clock to 32khz frequency.
 * @return      none
 */

void RSI_IPMU_32KHzRCClkClib(void) {
    uint32_t rc32k_trim = 0;
    uint32_t no_of_tst_clk_khz_rc = 0;

    /*Do until clock should be 32KHz */
    do {
        /* CLK40M buffer circuit will be ON */
        ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) |= BIT(21);
        /*  Off controls for clock cleaner are taken from NPSS */
        ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) &= ~(BIT(16));
        /*  RC 32KHz clock enable */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_OFFSET) |= BIT(21);
        /*  calibrated trim goes to the block (changes spi_trim select to 0) */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_OFFSET) &= ~(BIT(11));
        /* Enable the clock gate for npss ref clk &wait for 1us*/
        /*  32KHz RC clock calibration */
        /* Binary search calibration enable signal for low frequency clocks RO and RC */
        ULP_SPI_MEM_MAP(ULPCLKS_CALIB_REG_ADDR) |= (BIT(4) | BIT(14) | BIT(21));
        /* wait till bit 21 becomes 1 Indicates calibration done indication*/
        while(!(ULP_SPI_MEM_MAP(ULPCLKS_CALIB_DONE_REG_ADDR) & BIT(21)))
            ;
        /* Read calibrated trim value after low frequency calibration done */
        rc32k_trim = ((ULP_SPI_MEM_MAP(ULPCLKS_CALIB_DONE_REG_ADDR) & RC_TRIM_VALUE_LF) >> 4);
        /*Mask the bits where the trim value need to write */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_OFFSET) &=
            (uint32_t)~MASK32KRC_TRIM_VALUE_WRITE_BITS;
        /* Programming the calibrated trim to SPI register. */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_OFFSET) |= (rc32k_trim << 14);
        /*pointing the trim select to SPI */
        ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_OFFSET) |= (BIT(11));
        /* Measures MHz RC clock Clock Frequency  */
        no_of_tst_clk_khz_rc = RSI_Clks_Calibration(sleep_clk, khz_rc_clk);
        no_of_tst_clk_khz_rc /= 1000;
        /* Check if it is less than a particular value */
        if(no_of_tst_clk_khz_rc < PARTICULAR_FREQ_MIN) {
            ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_OFFSET) |= BIT(12) | BIT(13);
        }
        /* Check if it is greater than a particular value */
        if(no_of_tst_clk_khz_rc > PARTICULAR_FREQ_MAX) {
            ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_OFFSET) &= ~(BIT(12) | BIT(13));
        }
    } while((no_of_tst_clk_khz_rc < PARTICULAR_FREQ_MIN) ||
            (no_of_tst_clk_khz_rc > PARTICULAR_FREQ_MAX));

    return;
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_Clks_Trim32MHzRC(uint32_t freq)
 * @brief     This API is used to trim 32MHz RC clock to required frequency
 * @param[in]   freq :  Input is to get  required frequency from 13MHz to 32MHz.
 * @return      trim value for required MHz RC frequency which we can reserve for further usage  .
 */

uint32_t RSI_Clks_Trim32MHzRC(uint32_t freq) {
    volatile uint32_t no_oftst_clk_f = 0;
    volatile uint32_t no_oftst_clk = 0;
    volatile uint32_t reg_read = 0;
    volatile uint32_t trim_value = 0;

    system_clocks.rc_mhz_clock = freq;

    if(system_clocks.m4_ref_clock_source == ULP_MHZ_RC_CLK) {
        system_clocks.m4ss_ref_clk = freq;
    }
    if(system_clocks.ulp_ref_clock_source == ULPSS_ULP_MHZ_RC_CLK) {
        system_clocks.ulpss_ref_clk = freq;
    }

    /*Multiple the input frequency value with 10 e.g:20MHz as 200 */
    freq *= 10;

    /* Measures MHz RC clock Clock Frequency  */
    no_oftst_clk = RSI_Clks_Calibration(ulp_ref_clk, none);
    no_oftst_clk = no_oftst_clk_f / 100000;
    /* Trims MHz RC clock to required frequency */
    if(no_oftst_clk != freq) {
        reg_read = ULP_SPI_MEM_MAP(ULPCLKS_MRC_CLK_REG_OFFSET);
        /* Clears Trim bits(14-20) for RC 32MHz clock */
        reg_read &= (uint32_t)(~(0x7F << TRIM_LSB_MHZ));
        ULP_SPI_MEM_MAP(ULPCLKS_MRC_CLK_REG_OFFSET) = reg_read;
        /* check's from 20 bit to 14 bit  */
        for(volatile uint32_t i = TRIM_MSB_MHZ; i >= TRIM_LSB_MHZ; i--) {
            /* Measures MHz RC clock Clock Frequency  */
            no_oftst_clk_f = RSI_Clks_Calibration(ulp_ref_clk, none);
            /*To get in three digit of Measured frequency value in MHz e.g:20MHz as 200 */
            no_oftst_clk = no_oftst_clk_f / 100000;
            /* Halt the process for less than 0.3MHZ even  */
            if(freq - no_oftst_clk < MIN_DIFF_FREQ) {
                break;
            }
            /*Check whether the acquired frequency is higher than required frequency 
        If higher then clear the previous bit and set the present bit     */
            if(no_oftst_clk >= freq) {
                ULP_SPI_MEM_MAP(ULPCLKS_MRC_CLK_REG_OFFSET) &= ~BIT(i + 1);
                ULP_SPI_MEM_MAP(ULPCLKS_MRC_CLK_REG_OFFSET) |= BIT(i);
            }
            /* If lesser  set the present bit   */
            else {
                ULP_SPI_MEM_MAP(ULPCLKS_MRC_CLK_REG_OFFSET) |= BIT(i);
            }
        }
    }
    /* Trim bits(14-20) value for RC 32MHz clock   */
    trim_value = ULP_SPI_MEM_MAP(ULPCLKS_MRC_CLK_REG_OFFSET);
    trim_value &= (0x7F << TRIM_LSB_MHZ);
    trim_value = trim_value >> TRIM_LSB_MHZ;

    return trim_value;
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_20M_ROClktrim(uint8_t clkfreq)
 * @brief       This API is used to trim the 20Mhz RO clock to required frequency.
 * @param[in]   clkfreq  :  Input is to get  required frequency from 1MHz to 50MHz.
 * @return      none 
 */

void RSI_IPMU_20M_ROClktrim(uint8_t clkfreq) {
    volatile uint32_t ro50m_trim = 0;
    system_clocks.doubler_clock = (clkfreq * 1000000 * 2);

    system_clocks.ro_20mhz_clock = (clkfreq * 1000000);

    if(system_clocks.m4_ref_clock_source == ULP_20MHZ_RINGOSC_CLK) {
        system_clocks.m4ss_ref_clk = (clkfreq * 1000000);
    }
    if(system_clocks.ulp_ref_clock_source == ULPSS_ULP_20MHZ_RINGOSC_CLK) {
        system_clocks.ulpss_ref_clk = (clkfreq * 1000000);
    }
    /* CLK40M buffer circuit will be ON */
    ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) |= BIT(21);
    /*  Off controls for clock cleaner are taken from NPSS */
    ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) &= ~BIT(16);

    if(clkfreq > 10) {
        /* select input to High frequency RO */
        ULP_SPI_MEM_MAP(ULPCLKS_HF_RO_CLK_REG_OFFSET) |= BIT(13); /* RO_CLK is not supported */
    } else {
        /*select input to Low frequency RO  */
        ULP_SPI_MEM_MAP(ULPCLKS_HF_RO_CLK_REG_OFFSET) &= ~BIT(13); /* RO_CLK is not supported */
    }
    /* Enable the 50MHZ RO clock  */
    ULP_SPI_MEM_MAP(ULPCLKS_HF_RO_CLK_REG_OFFSET) |= BIT(21) |
                                                     BIT(12); /* RO_CLK is not supported */
    /*  powergate enable for calibration domain   */
    ULP_SPI_MEM_MAP(ULPCLKS_TRIM_SEL_REG_ADDR) = ENABLE_CALIB_DOMAIN;
    /* Mask the bits to write required frequency for High frequency RO clock */
    ULP_SPI_MEM_MAP(ULPCLKS_TRIM_SEL_REG_ADDR) &= (uint32_t)~0x3F;
    /*It writes that at what frequency the ROMhz need to be trim    */
    ULP_SPI_MEM_MAP(ULPCLKS_TRIM_SEL_REG_ADDR) |= clkfreq;
    /* Select the RO50M clock to calibrate */
    ULP_SPI_MEM_MAP(ULPCLKS_CALIB_REG_ADDR) = SELECT_RO_CALIB;
    /* Waiting for calibration done indication */
    while(!(ULP_SPI_MEM_MAP(ULPCLKS_CALIB_DONE_REG_ADDR) & BIT(20)))
        ;
    /* Reading calibrated trim value */
    ro50m_trim = ((ULP_SPI_MEM_MAP(ULPCLKS_CALIB_DONE_REG_ADDR) & TRIM_VALUE_BITS) >> 11);
    /*Mask the bits where the trim value need to write */
    ULP_SPI_MEM_MAP(ULPCLKS_HF_RO_CLK_REG_OFFSET) &=
        (uint32_t)~MASK_TRIM_VALUE_WRITE_BITS; /* RO_CLK is not supported */
    /* Programming the calibrated trim to SPI register. */
    ULP_SPI_MEM_MAP(ULPCLKS_HF_RO_CLK_REG_OFFSET) |=
        (ro50m_trim << 14); /* RO_CLK is not supported */
    /* pointing the trim select to SPI i.e write default values to that register */
    ULP_SPI_MEM_MAP(ULPCLKS_TRIM_SEL_REG_ADDR) = ULPCLKS_TRIM_SEL_REG_DEFAULT;
    /* CLK40M buffer circuit will be OFF */
    ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) &= ~BIT(21);
    /*  ON controls for clock cleaner are taken from NPSS */
    ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) |= BIT(16);
    /*  Disable the 50MHZ RO clock  */
    ULP_SPI_MEM_MAP(ULPCLKS_HF_RO_CLK_REG_OFFSET) &= ~BIT(21); /* RO_CLK is not supported */

    return;
}

/*==============================================*/
/**
 * @fn           uint32_t RSI_Clks_Calibration(INPUT_CLOCK_T inputclk, SLEEP_CLOCK_T sleep_clk_type)
 * @brief      This API is used for Clock Frequency measurement .
 * @param[in]    inputclk : Select the clock to be calibrated. 
 *                          - /ref INPUT_CLOCK_T
 * @param[in]    sleep_clk_type : Select the type of Khz clock(RO,RC, XTAL) if it is sleep clock.
 *                          - /ref SLEEP_CLOCK_T
 * @return       Clock Frequency in Hz .
 */

uint32_t RSI_Clks_Calibration(INPUT_CLOCK_T inputclk, SLEEP_CLOCK_T sleep_clk_type) {
    volatile uint32_t no_oftst_clk = 0;

    if(inputclk > 10) {
        return INVALID_PARAMETERS;
    }

    if(inputclk == sleep_clk) {
        if(sleep_clk_type == khz_rc_clk) {
            M4CLK->CLK_CONFIG_REG4_b.SLEEP_CLK_SEL = khz_rc_clk;
        }
        if(sleep_clk_type == khz_ro_clk) {
            M4CLK->CLK_CONFIG_REG4_b.SLEEP_CLK_SEL = khz_ro_clk; /* RO clock is not supported */
        }
        if(sleep_clk_type == khz_xtal_clk) {
            M4CLK->CLK_CONFIG_REG4_b.SLEEP_CLK_SEL = khz_xtal_clk;
        }
    }
    /* Select the clock to be calibrated*/
    M4CLK->CLK_CALIB_CTRL_REG1_b.CC_CLKIN_SEL_b = inputclk;
    /* number of ref clock cycles to be considered for calibrating */
    M4CLK->CLK_CALIB_CTRL_REG2_b.CC_NUM_REF_CLKS = 39062;
    /* Start clock calibration */
    M4CLK->CLK_CALIB_CTRL_REG1_b.CC_START_b = 0x1;
    /* Wait until the clock calibration done */
    while(!M4CLK->CLK_CALIB_STS_REG1_b.CC_DONE_b)
        ;

    if(M4CLK->CLK_CALIB_STS_REG1_b.CC_ERROR_b) {
        M4CLK->CLK_CALIB_CTRL_REG1_b.CC_SOFT_RST_b = 0x1;
    }

    /*If cc_error is not set then the clock calibration is done. */
    if(!M4CLK->CLK_CALIB_STS_REG1_b.CC_ERROR_b) {
        /* number of test clock cycles occurred for the specified number
       of ref_clock cycles*/
        no_oftst_clk = M4CLK->CLK_CALIB_STS_REG2_b.CC_NUM_TEST_CLKS;
    }

    return no_oftst_clk;
}

/*==============================================*/
/**
 * @fn          void RSI_IPMU_64KHZ_RCClktrim(void)
 * @brief       This API is used to trim the 64khz RC clock.
 * @return      none 
 */

void RSI_IPMU_64KHZ_RCClktrim(void) {
    uint32_t i;
    uint32_t status = 0;

    system_clocks.rc_32khz_clock = 64000;

    /*Enables RC clock and changes spi_trim select to 0*/
    ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_ADDR) = ENABLE_32KHZ_CLOCK_TRIM;

    /* Enable XTAL clock from NPSS */
    *(volatile uint32_t*)0x41300120 |= BIT(22);

    // Wait for 1ms using a delay loop
    for(i = 0; i < 10000; i++) {
        // Intentional empty loop body
    }

    /* Selects NPSS reference clock to be CLK-40M_SOC */
    ULP_SPI_MEM_MAP(ULPCLKS_REFCLK_REG_ADDR) = NPSS_REF_CLOCK_40MSOC;

    /* Select number of reference cycles to 625(No of clocks of high frequency clocks in 1 cycle of 16KHz clock Default value is 1250)*/
    ULP_SPI_MEM_MAP(ULPCLKS_CALIB_REF_REG) = NUMBER_HIGH_FRQ_CLOCK;

    /* Pointing RC32KHz clock to NPSS_TESTMODE 0 */
    *(volatile uint32_t*)0x24048610 = BIT(25) | BIT(28);

    /*Configure NPSS_GPIO_0 in mode 6 */
    *(volatile uint32_t*)0x24048610 = 0x0000000E;

    /*Enable the low frequency clock calibration,enable the clock gate for npss ref clk and select the RC32K clock to calibrate */
    ULP_SPI_MEM_MAP(ULPCLKS_CALIB_REG_ADDR) = LOW_FREQ_CLOCK_CAL;

    // Wait for 1us using a delay loop
    for(i = 0; i < 100; i++) {
        // Intentional empty loop body
    }

    /* wait till bit 21 becomes 1 Indicates calibration done indication*/
    while(!(ULP_SPI_MEM_MAP(0x30C) & BIT(21)))
        ;

    /* Calibrated trim value and write the calibrated trim into EFUSE */
    status = (ULP_SPI_MEM_MAP(0x30C) >> 4) & 0x7F;
    status = ((status << 14) | BIT(11));
    ULP_SPI_MEM_MAP(ULPCLKS_32KRC_CLK_REG_ADDR) |= status;

    /*Restore the original value of reference cycles once calibration is done */
    ULP_SPI_MEM_MAP(ULPCLKS_CALIB_REF_REG) = ORIGINAL_REF_VALUE_AFTER_CAL;
}
