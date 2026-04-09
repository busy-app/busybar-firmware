#include "power_i.h"

#define TAG "Power"

#define CHARGE_STEPS 20 // step = 5%
#define CURRENT_LOW  300 // 330
#define CURRENT_HIGH 2500 // 3300

static const uint16_t vbat_charge_low[CHARGE_STEPS + 1] = {
    3000, 3335, 3418, 3475, 3513, 3560, 3609, 3657, 3697, 3736, 3777,
    3820, 3868, 3911, 3947, 3990, 4043, 4086, 4110, 4146, 4240,
};
static const uint16_t vbat_charge_high[CHARGE_STEPS + 1] = {
    3000, 3465, 3533, 3578, 3621, 3666, 3714, 3759, 3799, 3845, 3892,
    3937, 3980, 4020, 4063, 4116, 4175, 4200, 4300, 4300, 4300,
};
static const uint16_t vbat_load_low[CHARGE_STEPS + 1] = {
    2600, 2973, 3117, 3213, 3309, 3394, 3462, 3528, 3575, 3644, 3707,
    3757, 3799, 3835, 3872, 3929, 3989, 4036, 4061, 4086, 4200,
};
static const uint16_t vbat_load_high[CHARGE_STEPS + 1] = {
    2600, 2852, 3003, 3104, 3199, 3282, 3347, 3405, 3459, 3511, 3563,
    3614, 3662, 3706, 3749, 3798, 3853, 3899, 3933, 3969, 4200,
};

static float get_percent(const uint16_t* table, uint32_t value) {
    // Handle underflow case
    if(value <= table[0]) {
        return 0.0f;
    }

    const float step_size = 100.0f / CHARGE_STEPS;

    for(uint8_t i = 0; i < CHARGE_STEPS; i++) {
        if(value < table[i + 1]) {
            float pct = i * step_size;
            pct += ((float)(value - table[i]) / (float)(table[i + 1] - table[i])) * step_size;
            return pct;
        }
    }
    return 100.0f;
}

uint8_t power_get_battery_charge(uint32_t voltage_mv, int32_t current_ma, bool is_charging) {
    float pct_low = 0.f;
    float pct_high = 0.f;
    float bat_current = fabsf((float)current_ma);

    if((is_charging) && (current_ma > 0)) {
        pct_low = get_percent(vbat_charge_low, voltage_mv);
        pct_high = get_percent(vbat_charge_high, voltage_mv);
    } else {
        pct_low = get_percent(vbat_load_low, voltage_mv);
        pct_high = get_percent(vbat_load_high, voltage_mv);
    }

    float pct = pct_low + (pct_high - pct_low) * (bat_current - CURRENT_LOW) /
                              (float)(CURRENT_HIGH - CURRENT_LOW);

    if(pct > 100.f) {
        pct = 100.f;
    }

    return (uint8_t)pct;
}
