#include "bq25798.h"

#define TAG "bq25798"

#define I_IN_MAX_DEFAULT 500

bool bq25798_init(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);

    Bq25987Reg48PartInformation data = {};
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
    Bq25987Reg09TerminationControl data = {.REG_RST = 1};
    return furi_hal_i2c_read_mem(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG09_TERMINATION_CONTROL,
        (uint8_t*)&data,
        sizeof(data),
        BQ25798_I2C_TIMEOUT);
}

void bq25798_dump_status(FuriHalI2cBusHandle* handle) {
    uint8_t status_regs[5];
    furi_hal_i2c_read_mem(handle, BQ25798_I2C_ADDRESS, 0x1B, status_regs, 5, BQ25798_I2C_TIMEOUT);
    FURI_LOG_I(
        TAG,
        "st %02X %02X %02X %02X %02X",
        status_regs[0],
        status_regs[1],
        status_regs[2],
        status_regs[3],
        status_regs[4]);

    furi_hal_i2c_read_mem(
        handle, BQ25798_I2C_ADDRESS, 0x20, &status_regs[0], 2, BQ25798_I2C_TIMEOUT);
    furi_hal_i2c_read_mem(
        handle, BQ25798_I2C_ADDRESS, 0x26, &status_regs[2], 2, BQ25798_I2C_TIMEOUT);
    FURI_LOG_I(
        TAG,
        "FAULT st %02X %02X fl %02X %02X",
        status_regs[0],
        status_regs[1],
        status_regs[2],
        status_regs[3]);

    uint16_t ibus = 0;
    furi_hal_i2c_read_reg_16(handle, BQ25798_I2C_ADDRESS, 0x31, &ibus, BQ25798_I2C_TIMEOUT);

    uint16_t ibat = 0;
    furi_hal_i2c_read_reg_16(handle, BQ25798_I2C_ADDRESS, 0x33, &ibat, BQ25798_I2C_TIMEOUT);

    furi_hal_i2c_write_reg_16(handle, BQ25798_I2C_ADDRESS, 0x06, 0x64, BQ25798_I2C_TIMEOUT);

    uint8_t ilim = 0;
    furi_hal_i2c_read_reg_8(handle, BQ25798_I2C_ADDRESS, 0x7, &ilim, BQ25798_I2C_TIMEOUT);
    FURI_LOG_I(TAG, "ibus %u ibat %u ilim %u", ibus, ibat, ilim * 10);
}

bool bq25798_set_cfg(FuriHalI2cBusHandle* handle) {
    furi_assert(handle);

    uint8_t cfg_temp = 0;

    // Disable watchdog
    furi_hal_i2c_read_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG10_CHARGER_CONTROL_1,
        &cfg_temp,
        BQ25798_I2C_TIMEOUT);
    cfg_temp |= (1 << 3); // WD_RST
    cfg_temp &= ~(7 << 0); // Watchdog disable
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG10_CHARGER_CONTROL_1,
        cfg_temp,
        BQ25798_I2C_TIMEOUT);

    // Mask unused irqs
    uint32_t irq_mask = ~(Bq25987IrqFlagVbusPresent);// | Bq25987IrqFlagChargeStatus);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG28_CHARGER_MASK_0,
        irq_mask & 0xFF,
        BQ25798_I2C_TIMEOUT);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG29_CHARGER_MASK_1,
        (irq_mask >> 8) & 0xFF,
        BQ25798_I2C_TIMEOUT);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG2A_CHARGER_MASK_2,
        (irq_mask >> 16) & 0xFF,
        BQ25798_I2C_TIMEOUT);
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG2B_CHARGER_MASK_3,
        (irq_mask >> 24) & 0xFF,
        BQ25798_I2C_TIMEOUT);

    // ADC enable
    furi_hal_i2c_write_reg_8(
        handle, BQ25798_I2C_ADDRESS, BQ25798_REG2E_ADC_CONTROL, (1 << 7), BQ25798_I2C_TIMEOUT);

    // Disable Dp/Dm detection
    furi_hal_i2c_write_reg_8(
        handle, BQ25798_I2C_ADDRESS, BQ25798_REG11_CHARGER_CONTROL_2, 0, BQ25798_I2C_TIMEOUT);

    // Disable ILIM_HIZ
    furi_hal_i2c_read_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG14_CHARGER_CONTROL_5,
        &cfg_temp,
        BQ25798_I2C_TIMEOUT);
    cfg_temp &= ~(1 << 1); // EN_EXTILIM: 0
    furi_hal_i2c_write_reg_8(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG14_CHARGER_CONTROL_5,
        cfg_temp,
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

bool bq25798_get_irq_flags(FuriHalI2cBusHandle* handle, uint32_t* flags) {
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

bool bq25798_get_battery_voltage(FuriHalI2cBusHandle* handle, float* value) {
    furi_assert(handle);
    furi_assert(value);

    return true;
}

bool bq25798_get_vbus_voltage(FuriHalI2cBusHandle* handle, float* value) {
    furi_assert(handle);
    furi_assert(value);

    return true;
}

bool bq25798_set_input_current_limit(FuriHalI2cBusHandle* handle, float value) {
    furi_assert(handle);
    furi_assert(value <= 3.3f);

    uint32_t cur_ma = value * 1000.f;
    furi_hal_i2c_write_reg_16(
        handle,
        BQ25798_I2C_ADDRESS,
        BQ25798_REG06_INPUT_CURRENT_LIMIT,
        cur_ma / 10,
        BQ25798_I2C_TIMEOUT);

    return true;
}
