#include "bh1730.h"
#include "bh1730_reg.h"

#include <furi.h>
#include <furi_hal_i2c.h>

// Tint = 2.8us - typical internal clock time, from datasheet
// ITIME_ms = Tint * 964 * (256 - ITIME) - integration time in ms
// Tmt = ITIME_ms + Tint * 714 - measurement time in ms
#define BH1730_ITIME_VAL (0xDA) // Tmt = 105 ms

bool bh1730_init(FuriHalI2cBusHandle* handle) {
    furi_check(handle);

    furi_hal_i2c_acquire(handle);

    bool success = false;
    do {
        Bh1730RegControl ctrl = {
            .POWER = 1,
            .ADC_EN = 1,
            .DATA_SEL = 0,
            .ONE_TIME = 0,
            .ADC_VALID = 0, // Read only
            .ADC_INTR = 0, // Read only
        };
        if(!furi_hal_i2c_write_reg_8(
               handle,
               BH1730_I2C_ADDRESS,
               0x80 | BH1730_REG_CONTROL,
               *(uint8_t*)&ctrl,
               BH1730_I2C_TIMEOUT))
            break;

        if(!furi_hal_i2c_write_reg_8(
               handle,
               BH1730_I2C_ADDRESS,
               0x80 | BH1730_REG_TIMING,
               BH1730_ITIME_VAL,
               BH1730_I2C_TIMEOUT))
            break;

        Bh1730RegGain gain = {
            .GAIN = BH1730_REG_GAIN_X1,
        };
        if(!furi_hal_i2c_write_reg_8(
               handle,
               BH1730_I2C_ADDRESS,
               0x80 | BH1730_REG_GAIN,
               *(uint8_t*)&gain,
               BH1730_I2C_TIMEOUT))
            break;

        success = true;
    } while(false);

    furi_hal_i2c_release(handle);

    return success;
}

static bool bh1730_read_raw_data(FuriHalI2cBusHandle* handle, uint16_t* value, uint8_t reg) {
    furi_check(handle);
    furi_check(value);

    uint8_t data_buf[2] = {};
    furi_hal_i2c_acquire(handle);
    bool read_success = furi_hal_i2c_read_mem(
        handle, BH1730_I2C_ADDRESS, 0x80 | reg, data_buf, 2, BH1730_I2C_TIMEOUT);
    furi_hal_i2c_release(handle);

    if(read_success) {
        *value = (data_buf[1] << 8) | data_buf[0];
    }

    return read_success;
}

bool bh1730_read_raw_data0(FuriHalI2cBusHandle* handle, uint16_t* value) {
    return bh1730_read_raw_data(handle, value, BH1730_REG_DATA0LOW);
}

bool bh1730_read_raw_data1(FuriHalI2cBusHandle* handle, uint16_t* value) {
    return bh1730_read_raw_data(handle, value, BH1730_REG_DATA1LOW);
}

bool bh1730_read_lux(FuriHalI2cBusHandle* handle, float* value) {
    furi_check(handle);
    furi_check(value);

    uint8_t data_buf[4] = {};
    furi_hal_i2c_acquire(handle);
    bool read_success = furi_hal_i2c_read_mem(
        handle, BH1730_I2C_ADDRESS, 0x80 | BH1730_REG_DATA0LOW, data_buf, 4, BH1730_I2C_TIMEOUT);
    furi_hal_i2c_release(handle);

    if(!read_success) {
        return false;
    }

    int16_t adc0_val = (data_buf[1] << 8) | data_buf[0];
    int16_t adc1_val = (data_buf[3] << 8) | data_buf[1];
    uint8_t gain = 1;

    float itime_ms = (2.8f * 964.f * (256.f - BH1730_ITIME_VAL)) / 1000.f;

    float lux = 0.f;
    if(adc0_val != 0) {
        if(adc1_val / adc0_val < 0.26f) {
            lux = (1.290f * adc0_val - 2.733f * adc1_val) / gain * 102.6f / itime_ms;
        } else if(adc1_val / adc0_val < 0.55f) {
            lux = (0.795f * adc0_val - 0.859f * adc1_val) / gain * 102.6f / itime_ms;
        } else if(adc1_val / adc0_val < 1.09f) {
            lux = (0.510f * adc0_val - 0.345f * adc1_val) / gain * 102.6f / itime_ms;
        } else if(adc1_val / adc0_val < 2.13f) {
            lux = (0.276f * adc0_val - 0.130f * adc1_val) / gain * 102.6f / itime_ms;
        }
    }

    *value = lux;

    return true;
}
