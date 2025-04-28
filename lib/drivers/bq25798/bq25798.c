#include "bq25798.h"

#include <furi.h>
#include <furi_hal_i2c.h>

#define TAG "bq25798"

#define I_IN_MAX_DEFAULT 500

static bool
    bq25798_read_mem(FuriHalI2cBusHandle* handle, uint8_t mem_addr, uint8_t* data, size_t len) {
    return furi_hal_i2c_read_mem(
        handle, BQ25798_I2C_ADDRESS, mem_addr, data, len, BQ25798_I2C_TIMEOUT);
}

static bool bq25798_write_reg_8(FuriHalI2cBusHandle* handle, uint8_t reg_addr, uint8_t data) {
    return furi_hal_i2c_write_reg_8(
        handle, BQ25798_I2C_ADDRESS, reg_addr, data, BQ25798_I2C_TIMEOUT);
}

static bool bq25798_write_reg_16(FuriHalI2cBusHandle* handle, uint8_t reg_addr, uint16_t data) {
    return furi_hal_i2c_write_reg_16(
        handle, BQ25798_I2C_ADDRESS, reg_addr, data, BQ25798_I2C_TIMEOUT);
}

static bool bq25798_read_reg_8(FuriHalI2cBusHandle* handle, uint8_t reg_addr, uint8_t* data) {
    return furi_hal_i2c_read_reg_8(
        handle, BQ25798_I2C_ADDRESS, reg_addr, data, BQ25798_I2C_TIMEOUT);
}

bool bq25798_init(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);

    Bq25798Reg48PartInformation data = {};
    bool ret =
        bq25798_read_mem(handle, BQ25798_REG48_PART_INFORMATION, (uint8_t*)&data, sizeof(data));
    FURI_LOG_I(TAG, "ret: %u pn: %u rev: %u", ret, data.PN, data.DEV_REV);

    if(!ret || data.PN != 0b011) {
        return false;
    }

    return true;
}

bool bq25798_reset(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);
    Bq25798Reg09TerminationControl data = {.REG_RST = 1};
    return bq25798_read_mem(
        handle, BQ25798_REG09_TERMINATION_CONTROL, (uint8_t*)&data, sizeof(data));
}

bool bq25798_set_cfg(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);

    {
        Bq25798Reg10ChargerControl1 reg = {};
        bq25798_read_reg_8(handle, BQ25798_REG10_CHARGER_CONTROL_1, (uint8_t*)&reg);

        // Reset the watchdog
        reg.WD_RST = 1;

        // Watchdog disable
        reg.WATCHDOG = 0;

        bq25798_write_reg_8(handle, BQ25798_REG10_CHARGER_CONTROL_1, *(uint8_t*)&reg);
    }

    {
        // Mask unused irqs
        uint32_t flags_mask = ~(Bq25798ChargerFlagVbusPresent);
        bq25798_write_reg_8(handle, BQ25798_REG28_CHARGER_MASK_0, flags_mask & 0xFF);
        bq25798_write_reg_8(handle, BQ25798_REG29_CHARGER_MASK_1, (flags_mask >> 8) & 0xFF);
        bq25798_write_reg_8(handle, BQ25798_REG2A_CHARGER_MASK_2, (flags_mask >> 16) & 0xFF);
        bq25798_write_reg_8(handle, BQ25798_REG2B_CHARGER_MASK_3, (flags_mask >> 24) & 0xFF);
    }

    {
        // ADC enable
        Bq25798Reg2EADCControl reg = {.ADC_EN = 1};
        bq25798_write_reg_8(handle, BQ25798_REG2E_ADC_CONTROL, *(uint8_t*)&reg);
    }

    {
        // Disable Dp/Dm detection
        Bq25798Reg11ChargerControl2 reg = {0};
        bq25798_write_reg_8(handle, BQ25798_REG11_CHARGER_CONTROL_2, *(uint8_t*)&reg);
    }

    {
        Bq25798Reg12ChargerControl3 reg = {0};
        bq25798_read_reg_8(handle, BQ25798_REG12_CHARGER_CONTROL_3, (uint8_t*)&reg);

        // Disable PFM in forward mode
        reg.PFM_FWD_DIS = 1;

        bq25798_write_reg_8(handle, BQ25798_REG12_CHARGER_CONTROL_3, *(uint8_t*)&reg);
    }

    {
        Bq25798Reg14ChargerControl5 reg = {0};
        bq25798_read_reg_8(handle, BQ25798_REG14_CHARGER_CONTROL_5, (uint8_t*)&reg);

        // Disable the external ILIM_HIZ pin input current regulation
        reg.EN_EXTILIM = 0;

        // Enable the IBAT discharge sensing at battery only or OTG condition
        reg.EN_IBAT = 1;

        // Ship FET populated
        reg.SFET_PRESENT = 1;

        bq25798_write_reg_8(handle, BQ25798_REG14_CHARGER_CONTROL_5, *(uint8_t*)&reg);
    }

    // Set default USB current (500ma)
    bq25798_set_input_current_limit(handle, I_IN_MAX_DEFAULT);

    return true;
}

bool bq25798_get_charger_status(FuriHalI2cBusHandle* handle, Bq25798ChargerStatus* status) {
    furi_assert(handle);
    furi_assert(status);

    bool ret = bq25798_read_mem(handle, BQ25798_REG1B_CHARGER_STATUS_0, status->data, 5);

    return ret;
}

bool bq25798_get_charger_fault(FuriHalI2cBusHandle* handle, Bq25798ChargerFault* fault) {
    furi_assert(handle);
    furi_assert(fault);

    bool ret = bq25798_read_mem(handle, BQ25798_REG20_FAULT_STATUS_0, fault->data, 2);

    return ret;
}

bool bq25798_get_charger_irq_flags(FuriHalI2cBusHandle* handle, uint32_t* flags) {
    furi_assert(handle);
    furi_assert(flags);

    uint8_t regs_data[4];
    bool ret = bq25798_read_mem(handle, BQ25798_REG22_CHARGER_FLAG_0, regs_data, 4);

    *flags = regs_data[0] | (regs_data[1] << 8) | (regs_data[2] << 16) | (regs_data[3] << 24);

    return ret;
}

bool bq25798_get_adc_values(FuriHalI2cBusHandle* handle, Bq25798AdcValues* values) {
    furi_assert(handle);
    furi_assert(values);

    uint8_t regs_data[4 * 2];
    bool ret = bq25798_read_mem(handle, BQ25798_REG31_IBUS_ADC, regs_data, 3 * 2);
    if(!ret) {
        return ret;
    }

    values->usb_i = (regs_data[0] << 8) | regs_data[1];
    values->bat_i = (regs_data[2] << 8) | regs_data[3];
    values->usb_v = (regs_data[4] << 8) | regs_data[5];

    ret = bq25798_read_mem(handle, BQ25798_REG3B_VBAT_ADC, regs_data, 4 * 2);

    values->bat_v = (regs_data[0] << 8) | regs_data[1];
    values->sys_v = (regs_data[2] << 8) | regs_data[3];
    values->temp_bat_pct = ((regs_data[4] << 8) | regs_data[5]) * 0.0976563f;
    values->temp_charger = ((regs_data[6] << 8) | regs_data[7]) * 0.5f;

    return ret;
}

bool bq25798_set_input_current_limit(FuriHalI2cBusHandle* handle, uint32_t value_ma) {
    furi_assert(handle);
    furi_assert(value_ma <= 3300);
    bq25798_write_reg_16(handle, BQ25798_REG06_INPUT_CURRENT_LIMIT, value_ma / 10);
    return true;
}

bool bq25798_set_charge_current_limit(FuriHalI2cBusHandle* handle, uint32_t value_ma) {
    furi_assert(handle);
    furi_assert(value_ma <= 5000);
    return bq25798_write_reg_16(handle, BQ25798_REG03_CHARGE_CURRENT_LIMIT, value_ma / 10 & 0x1ff);
}

bool bq25798_set_charge_voltage_limit(FuriHalI2cBusHandle* handle, uint32_t value_mv) {
    furi_assert(handle);
    furi_assert(value_mv <= 4200);
    return bq25798_write_reg_16(handle, BQ25798_REG01_CHARGE_VOLTAGE_LIMIT, value_mv / 10 & 0x7ff);
}

bool bq25798_charge_enable(FuriHalI2cBusHandle* handle, bool enabled) {
    furi_assert(handle);

    Bq25798Reg0FChargerControl0 reg_temp = {0};
    bq25798_read_reg_8(handle, BQ25798_REG0F_CHARGER_CONTROL_0, (uint8_t*)&reg_temp);
    reg_temp.EN_CHG = enabled;
    return bq25798_write_reg_8(handle, BQ25798_REG0F_CHARGER_CONTROL_0, *(uint8_t*)&reg_temp);
}

void bq25798_power_switch(FuriHalI2cBusHandle* handle, Bq25798PowerSwitch mode) {
    furi_assert(handle);
    furi_assert(mode <= Bq25798PowerReset);

    Bq25798Reg11ChargerControl2 reg_temp = {0};
    bq25798_read_reg_8(handle, BQ25798_REG11_CHARGER_CONTROL_2, (uint8_t*)&reg_temp);
    // Do not add 10s delay before switching off
    reg_temp.SDRV_DLY = 1;
    // Apply the mode
    reg_temp.SDRV_CTRL = mode;
    bq25798_write_reg_8(handle, BQ25798_REG11_CHARGER_CONTROL_2, *(uint8_t*)&reg_temp);
}
