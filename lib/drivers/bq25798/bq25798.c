#include "bq25798.h"

#include <furi.h>
#include <furi_hal_i2c.h>

#define TAG "bq25798"

#define I_IN_MAX_DEFAULT 500

bool bq25798_init(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);

    Bq25798Reg48PartInformation data = {};
    bool ret = furi_hal_i2c_read_mem(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG48_PART_INFORMATION,
        (uint8_t*)&data,
        sizeof(data),
        BQ25798_I2C_TIMEOUT);
    FURI_LOG_I(TAG, "ret: %u pn: %u rev: %u", ret, data.PN, data.DEV_REV);

    if(!ret || data.PN != 0b011) {
        return false;
    }

    return true;
}

bool bq25798_reset(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);
    Bq25798Reg09TerminationControl data = {.REG_RST = 1};
    return furi_hal_i2c_read_mem(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG09_TERMINATION_CONTROL,
        (uint8_t*)&data,
        sizeof(data),
        BQ25798_I2C_TIMEOUT);
}

bool bq25798_set_cfg(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);

    // Disable watchdog
    Bq25798Reg10ChargerControl1 reg10_temp = {};
    furi_hal_i2c_read_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG10_CHARGER_CONTROL_1,
        (uint8_t*)&reg10_temp,
        BQ25798_I2C_TIMEOUT);
    reg10_temp.WD_RST = 1;
    reg10_temp.WATCHDOG = 0; // Watchdog disable
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG10_CHARGER_CONTROL_1,
        *(uint8_t*)&reg10_temp,
        BQ25798_I2C_TIMEOUT);

    // Mask unused irqs
    uint32_t charger_flags_mask = ~(Bq25798ChargerFlagVbusPresent);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG28_CHARGER_MASK_0,
        charger_flags_mask & 0xFF,
        BQ25798_I2C_TIMEOUT);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG29_CHARGER_MASK_1,
        (charger_flags_mask >> 8) & 0xFF,
        BQ25798_I2C_TIMEOUT);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG2A_CHARGER_MASK_2,
        (charger_flags_mask >> 16) & 0xFF,
        BQ25798_I2C_TIMEOUT);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG2B_CHARGER_MASK_3,
        (charger_flags_mask >> 24) & 0xFF,
        BQ25798_I2C_TIMEOUT);

    // ADC enable
    Bq25798Reg2EADCControl reg2e_temp = {.ADC_EN = 1};
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG2E_ADC_CONTROL,
        *(uint8_t*)&reg2e_temp,
        BQ25798_I2C_TIMEOUT);

    // Disable Dp/Dm detection
    Bq25798Reg11ChargerControl2 reg11_temp = {0};
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG11_CHARGER_CONTROL_2,
        *(uint8_t*)&reg11_temp,
        BQ25798_I2C_TIMEOUT);

    Bq25798Reg12ChargerControl3 reg12_temp = {0};
    furi_hal_i2c_read_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG12_CHARGER_CONTROL_3,
        (uint8_t*)&reg12_temp,
        BQ25798_I2C_TIMEOUT);
    reg12_temp.PFM_FWD_DIS = 1;
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG12_CHARGER_CONTROL_3,
        *(uint8_t*)&reg12_temp,
        BQ25798_I2C_TIMEOUT);

    // Disable ILIM_HIZ
    Bq25798Reg14ChargerControl5 reg14_temp = {0};
    furi_hal_i2c_read_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG14_CHARGER_CONTROL_5,
        (uint8_t*)&reg14_temp,
        BQ25798_I2C_TIMEOUT);
    reg14_temp.EN_EXTILIM = 0;
    reg14_temp.EN_IBAT = 1;
    reg14_temp.SFET_PRESENT = 1; // Ship FET populated
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG14_CHARGER_CONTROL_5,
        *(uint8_t*)&reg14_temp,
        BQ25798_I2C_TIMEOUT);

    // NTC configuration
    Bq25798Reg17NtcControl0 ntc_cfg0_temp = {.JEITA_VSET = 3, .JEITA_ISETH = 2, .JEITA_ISETC = 1};
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG17_NTC_CONTROL_0,
        *(uint8_t*)&ntc_cfg0_temp,
        BQ25798_I2C_TIMEOUT);

    Bq25798Reg18NtcControl1 ntc_cfg1_temp = {
        .TS_COOL = 1,
        .TS_WARM = 1,
        .BHOT = 1,
        .BCOLD = 0,
        .TS_IGNORE = 0,
    };
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG18_NTC_CONTROL_1,
        *(uint8_t*)&ntc_cfg1_temp,
        BQ25798_I2C_TIMEOUT);

    // Set default USB current (500ma)
    furi_hal_i2c_write_reg_16(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG06_INPUT_CURRENT_LIMIT,
        I_IN_MAX_DEFAULT / 10,
        BQ25798_I2C_TIMEOUT);

    return true;
}

bool bq25798_get_charger_status(FuriHalI2cBusHandle* handle, Bq25798ChargerStatus* status) {
    furi_assert(handle);
    furi_assert(status);

    bool ret = furi_hal_i2c_read_mem(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG1B_CHARGER_STATUS_0,
        status->data,
        5,
        BQ25798_I2C_TIMEOUT);

    return ret;
}

bool bq25798_get_charger_fault(FuriHalI2cBusHandle* handle, Bq25798ChargerFault* fault) {
    furi_assert(handle);
    furi_assert(fault);

    bool ret = furi_hal_i2c_read_mem(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG20_FAULT_STATUS_0,
        fault->data,
        2,
        BQ25798_I2C_TIMEOUT);

    return ret;
}

bool bq25798_get_charger_irq_flags(FuriHalI2cBusHandle* handle, uint32_t* flags) {
    furi_assert(handle);
    furi_assert(flags);

    uint8_t regs_data[4];
    bool ret = furi_hal_i2c_read_mem(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG22_CHARGER_FLAG_0,
        regs_data,
        4,
        BQ25798_I2C_TIMEOUT);

    *flags = regs_data[0] | (regs_data[1] << 8) | (regs_data[2] << 16) | (regs_data[3] << 24);

    return ret;
}

bool bq25798_get_adc_values(FuriHalI2cBusHandle* handle, Bq25798AdcValues* values) {
    furi_assert(handle);
    furi_assert(values);

    uint8_t regs_data[4 * 2];
    bool ret = furi_hal_i2c_read_mem(
        handle, BQ25798_I2C_ADDRESS, BQ25798_REG31_IBUS_ADC, regs_data, 3 * 2, BQ25798_I2C_TIMEOUT);
    if(!ret) {
        return ret;
    }

    values->usb_i = (regs_data[0] << 8) | regs_data[1];
    values->bat_i = (regs_data[2] << 8) | regs_data[3];
    values->usb_v = (regs_data[4] << 8) | regs_data[5];

    ret = furi_hal_i2c_read_mem(
        handle, BQ25798_I2C_ADDRESS, BQ25798_REG3B_VBAT_ADC, regs_data, 4 * 2, BQ25798_I2C_TIMEOUT);

    values->bat_v = (regs_data[0] << 8) | regs_data[1];
    values->sys_v = (regs_data[2] << 8) | regs_data[3];
    values->temp_bat_pct = ((regs_data[4] << 8) | regs_data[5]) * 0.0976563f;
    values->temp_charger = ((regs_data[6] << 8) | regs_data[7]) * 0.5f;

    return ret;
}

bool bq25798_set_input_current_limit(FuriHalI2cBusHandle* handle, uint32_t value_ma) {
    furi_assert(handle);
    furi_assert(value_ma <= 3300);

    furi_hal_i2c_write_reg_16(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG06_INPUT_CURRENT_LIMIT,
        value_ma / 10,
        BQ25798_I2C_TIMEOUT);

    return true;
}

bool bq25798_set_charge_current_limit(FuriHalI2cBusHandle* handle, uint32_t value_ma) {
    furi_assert(handle);
    furi_assert(value_ma <= 5000);

    return furi_hal_i2c_write_reg_16(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG03_CHARGE_CURRENT_LIMIT,
        value_ma / 10 & 0x1ff,
        BQ25798_I2C_TIMEOUT);
}

bool bq25798_set_charge_voltage_limit(FuriHalI2cBusHandle* handle, uint32_t value_mv) {
    furi_assert(handle);
    furi_assert(value_mv <= 4200);

    return furi_hal_i2c_write_reg_16(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG01_CHARGE_VOLTAGE_LIMIT,
        value_mv / 10 & 0x7ff,
        BQ25798_I2C_TIMEOUT);
}

bool bq25798_charge_enable(FuriHalI2cBusHandle* handle, bool enabled) {
    furi_assert(handle);

    Bq25798Reg0FChargerControl0 reg_temp = {0};
    furi_hal_i2c_read_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG0F_CHARGER_CONTROL_0,
        (uint8_t*)&reg_temp,
        BQ25798_I2C_TIMEOUT);

    reg_temp.EN_CHG = enabled;

    return furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG0F_CHARGER_CONTROL_0,
        *(uint8_t*)&reg_temp,
        BQ25798_I2C_TIMEOUT);
}

void bq25798_power_switch(FuriHalI2cBusHandle* handle, Bq25798PowerSwitch mode) {
    furi_assert(handle);
    furi_assert(mode <= Bq25798PowerReset);

    Bq25798Reg11ChargerControl2 reg_temp = {0};
    furi_hal_i2c_read_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG11_CHARGER_CONTROL_2,
        (uint8_t*)&reg_temp,
        BQ25798_I2C_TIMEOUT);
    reg_temp.SDRV_DLY = 1;
    reg_temp.SDRV_CTRL = mode;
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG11_CHARGER_CONTROL_2,
        *(uint8_t*)&reg_temp,
        BQ25798_I2C_TIMEOUT);
}
