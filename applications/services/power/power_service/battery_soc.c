/**
 * Useful resources:
 *   - https://www.researchgate.net/publication/352428548_Implementation_of_Coulomb_Counting_Method_for_Estimating_the_State_of_Charge_of_Lithium-Ion_Battery/fulltext/60c98aa1299bf108abdd3386/Implementation-of-Coulomb-Counting-Method-for-Estimating-the-State-of-Charge-of-Lithium-Ion-Battery.pdf?origin=publication_detail&_tp=eyJjb250ZXh0Ijp7ImZpcnN0UGFnZSI6InB1YmxpY2F0aW9uIiwicGFnZSI6InB1YmxpY2F0aW9uRG93bmxvYWQiLCJwcmV2aW91c1BhZ2UiOiJwdWJsaWNhdGlvbiJ9fQ
 */

#include "battery_soc.h"
#include <toolbox/saved_struct.h>
#include <toolbox/float_tools.h>

// =======
// Logging
// =======

#define TAG "BatterySoc"

#define BATTERY_SOC_TRACE_ENABLE

#ifdef BATTERY_SOC_TRACE_ENABLE
#define BATTERY_SOC_T(...) FURI_LOG_T(__VA_ARGS__)
#else
#define BATTERY_SOC_T(...)
#endif

// ==========
// Data types
// ==========

typedef enum {
    BatterySocSavedFlagCalibratedChgCapacity = 1 << 0,
    BatterySocSavedFlagCalibratedDischgCapacity = 1 << 1,
    BatterySocSavedFlagCalibratedVoltage = 1 << 2,
    BatterySocSavedFlagMAX,
} BatterySocSavedFlag;

#define BatterySocSavedFlagCalibratedCharge (BatterySocSavedFlagCalibratedChgCapacity | BatterySocSavedFlagCalibratedDischgCapacity)
#define BatterySocSavedFlagFullyCalibrated (BatterySocSavedFlagCalibratedCharge | BatterySocSavedFlagCalibratedVoltage)

// voltage_table[is_charging][current_bucket][percent_bucket] = vbat_mv
#define BATTERY_SOC_V_TABLE_STEPS 100
#define BATTERY_SOC_V_TABLE_I_STEPS 10
typedef int16_t BatterySocVTable[2][BATTERY_SOC_V_TABLE_I_STEPS][BATTERY_SOC_V_TABLE_STEPS];

#define BATTERY_SOC_CYCLE_INTERRUPT_THRESHOLD (100'000'000ll)

#define BATTERY_SOC_SAVED_MAGIC   0xBC // Battery Calibration
#define BATTERY_SOC_SAVED_VERSION 1
typedef struct {
    BatterySocSavedFlag flags;

    int64_t total_in_uc;
    int64_t total_out_uc;

    int64_t charge_capacity_uc;
    int64_t discharge_capacity_uc;
    int64_t first_dischg_capacity_uc;

    int64_t last_error_uc;

    BatterySocVTable v_table;
} BatterySocSaved;

typedef enum {
    BatterySocStateCharging,
    BatterySocStateIdle,
    BatterySocStateDischarging,
    BatterySocStateMAX,
} BatterySocState;

struct BatterySoc {
    BatterySocSettings settings;
    BatterySocSaved saved;

    BatterySocState state;

    MsSinceBoot last_measurement;
    int64_t in_uc;
    int64_t out_uc;
    int64_t current_uc;
    bool full_charge_cycle;
    bool full_discharge_cycle;
    bool charge_accurate;
    bool limit_processed;
};

// ==================
// Internal functions
// ==================

typedef enum {
    BatterySocLimitingStateEmpty,
    BatterySocLimitingStateFull,
    BatterySocLimitingStateMAX,
} BatterySocLimitingState;

static void battery_soc_limit(BatterySoc* soc, BatterySocLimitingState state) {
    furi_assert(soc);
    furi_assert(state < BatterySocLimitingStateMAX);

    FURI_LOG_D(TAG, "Limit: %s", (state == BatterySocLimitingStateFull) ? "full" : "empty");

    if(state == BatterySocLimitingStateEmpty) {
        soc->full_charge_cycle = true;
        soc->charge_accurate = true;

        if(soc->full_discharge_cycle) {
            soc->saved.flags |= BatterySocSavedFlagCalibratedDischgCapacity;
            int64_t previous_uc = soc->saved.discharge_capacity_uc;
            soc->saved.discharge_capacity_uc = soc->out_uc;
            soc->saved.last_error_uc = soc->current_uc;
            FURI_LOG_D(TAG, "Discharge calibrated: old=[%lld mC], new=[%lld mC], error=[%lld mC]", previous_uc / 1000, soc->saved.discharge_capacity_uc / 1000, soc->saved.last_error_uc / 1000);
        } else {
            FURI_LOG_W(TAG, "Discharge was not fully tracked and continuous, unable to calibrate");
        }

        soc->current_uc = 0;
        soc->out_uc = 0;

    } else if(state == BatterySocLimitingStateFull) {
        soc->full_discharge_cycle = true;
        soc->charge_accurate = true;

        if(soc->full_charge_cycle) {
            soc->saved.flags |= BatterySocSavedFlagCalibratedChgCapacity;
            int64_t previous_uc = soc->saved.charge_capacity_uc;
            soc->saved.charge_capacity_uc = soc->in_uc;
            FURI_LOG_D(TAG, "Charge calibrated: old=[%lld mC], new=[%lld mC]", previous_uc / 1000, soc->saved.charge_capacity_uc / 1000);
            if(!soc->saved.first_dischg_capacity_uc) {
                soc->saved.first_dischg_capacity_uc = soc->saved.discharge_capacity_uc;
            }

        } else {
            FURI_LOG_W(TAG, "Charge was not fully tracked and continuous, unable to calibrate");
            if(soc->saved.flags & BatterySocSavedFlagCalibratedChgCapacity) {
                soc->current_uc = soc->saved.discharge_capacity_uc;
            }

        }
        soc->in_uc = 0;

    } else {
        furi_crash(/* unknown state */);
    }

    if(soc->saved.flags & BatterySocSavedFlagCalibratedCharge) {
        FURI_LOG_D(TAG, "Charge and discharge calibrated");
    }

    battery_soc_sync(soc);
}

static float battery_soc_volt_based(BatterySocVTable v_table, bool is_charging, int32_t voltage_mv, float relative_current) {
    furi_assert(v_table);

    float single_lookup(size_t i_bucket) {
        const int16_t* curve = v_table[is_charging][i_bucket];
        if(voltage_mv < curve[0]) return 0.0f;

        const float pct_per_step = 100.0f / BATTERY_SOC_V_TABLE_STEPS;
        for(size_t i = 0; i < BATTERY_SOC_V_TABLE_STEPS + 1; i++) {
            if(curve[i] >= voltage_mv) return i * pct_per_step;
        }

        return 100.0f;
    }

    float i_bucket = relative_current * BATTERY_SOC_V_TABLE_I_STEPS;
    size_t i_bucket_a = MIN(floorf(i_bucket), BATTERY_SOC_V_TABLE_I_STEPS - 1);
    size_t i_bucket_b = MIN(ceilf(i_bucket), BATTERY_SOC_V_TABLE_I_STEPS - 1);
    float i_bucket_mix = i_bucket - (float)i_bucket_a;

    return float_lerp(single_lookup(i_bucket_a), single_lookup(i_bucket_b), i_bucket_mix);
}

static float battery_soc_coulomb_based(const BatterySoc* soc, bool is_charging) {
    furi_assert(soc);
    float capacity = is_charging ? soc->saved.charge_capacity_uc : soc->saved.discharge_capacity_uc;
    return (float)soc->current_uc * 100.0f / capacity;
}

static void battery_soc_populate_default_v_table(BatterySoc* soc) {
    furi_assert(soc);

    static const size_t steps = 20;
    static_assert((BATTERY_SOC_V_TABLE_STEPS % steps) == 0);

    static const uint16_t default_v_table[2][2][21] = {
        // charging:
        {
            // low current:
            {
                3000, 3335, 3418, 3475, 3513, 3560, 3609, 3657, 3697, 3736, 3777,
                3820, 3868, 3911, 3947, 3990, 4043, 4086, 4110, 4146, 4240,
            },
            // high current:
            {
                3000, 3465, 3533, 3578, 3621, 3666, 3714, 3759, 3799, 3845, 3892,
                3937, 3980, 4020, 4063, 4116, 4175, 4200, 4300, 4300, 4300,
            },
        },
        // discharging:
        {
            // low current:
            {
                2600, 2973, 3117, 3213, 3309, 3394, 3462, 3528, 3575, 3644, 3707,
                3757, 3799, 3835, 3872, 3929, 3989, 4036, 4061, 4086, 4200,
            },
            // high current:
            {
                2600, 2852, 3003, 3104, 3199, 3282, 3347, 3405, 3459, 3511, 3563,
                3614, 3662, 3706, 3749, 3798, 3853, 3899, 3933, 3969, 4200,
            },
        },
    };

    for(size_t chg = 0; chg < 1; chg++) {
        for(size_t i_bkt = 0; i_bkt < 1; i_bkt++) {
            bool is_charging = chg == 1;
            for(size_t dst_i_bucket = i_bkt * 5; dst_i_bucket < (i_bkt + 1) * 5; dst_i_bucket++) {
                const uint16_t* src_curve = default_v_table[chg][i_bkt];

                for(size_t step = 0; step < steps; step++) {
                    size_t step_ratio = BATTERY_SOC_V_TABLE_STEPS / steps;
                    for(size_t i = 0; i < step_ratio; i++) {
                        soc->saved.v_table[is_charging][dst_i_bucket][(step * step_ratio) + i] = src_curve[step];
                    }
                }
            }
        }
    }

    FURI_LOG_D(TAG, "Default inaccurate voltage table populated");
}

// ==========
// Public API
// ==========

BatterySoc* battery_soc_alloc(const BatterySocSettings* settings) {
    furi_check(settings);

    BatterySoc* soc = malloc(sizeof(BatterySoc));
    memcpy(&soc->settings, settings, sizeof(*settings));

    if(settings->storage_path) {
        soc->settings.storage_path = strdup(settings->storage_path);

        if(saved_struct_load(soc->settings.storage_path, &soc->saved, sizeof(BatterySocSaved), BATTERY_SOC_SAVED_MAGIC, BATTERY_SOC_SAVED_VERSION)) {
            FURI_LOG_D(TAG, "Loaded calibration");
        } else {
            FURI_LOG_D(TAG, "No stored calibration");
        }
    }

    if(!(soc->saved.flags & BatterySocSavedFlagCalibratedCharge)) {
        FURI_LOG_W(TAG, "No charge calibration; using Volt-based state-of-charge estimation");
    }

    if(!(soc->saved.flags & BatterySocSavedFlagCalibratedVoltage)) {
        FURI_LOG_W(TAG, "No voltage calibration; Volt-based state-of-charge estimation will be inaccurate");
        battery_soc_populate_default_v_table(soc);
    }

    return soc;
}

void battery_soc_free(BatterySoc* soc) {
    battery_soc_sync(soc);

    free((char*)soc->settings.storage_path);
    free(soc);
}

void battery_soc_sync(BatterySoc* soc) {
    // return;
    furi_check(soc);

    if(!soc->settings.storage_path) return;

    if(saved_struct_save(soc->settings.storage_path, &soc->saved, sizeof(BatterySocSaved), BATTERY_SOC_SAVED_MAGIC, BATTERY_SOC_SAVED_VERSION)) {
        FURI_LOG_D(TAG, "Stored calibration");
    } else {
        FURI_LOG_E(TAG, "Failed to store calibration");
    }
}

BatterySocLevel battery_soc_feed_measurements(BatterySoc* soc, const BatterySocMeasurements* measurements) {
    furi_check(soc);
    furi_check(measurements);

    MsSinceBoot last_measurement = soc->last_measurement;
    bool is_first_measurement = last_measurement == 0;
    soc->last_measurement = measurements->timestamp;

    if(is_first_measurement) {
        return (BatterySocLevel){ .flags = BatterySocLevelFlagNoData };
    }

    furi_check(measurements->timestamp > last_measurement);
    bool is_charging = measurements->charge_current_ua > 0;

    float efficiency = 1.0f;
    if(is_charging && (soc->saved.flags & BatterySocSavedFlagCalibratedCharge)) {
        efficiency = (float)soc->saved.charge_capacity_uc / (float)soc->saved.discharge_capacity_uc;
    }

    int64_t delta_ms = measurements->timestamp - soc->last_measurement;
    int64_t delta_uc = (delta_ms * measurements->charge_current_ua) / 1000;
    soc->current_uc += delta_uc * efficiency;

    if(is_charging) {
        furi_assert(delta_uc >= 0);
        soc->saved.total_in_uc += delta_uc;
        soc->in_uc += delta_uc;
        if(soc->full_discharge_cycle) {
            if(soc->in_uc > BATTERY_SOC_CYCLE_INTERRUPT_THRESHOLD) {
                soc->full_discharge_cycle = false;
                FURI_LOG_W(TAG, "Continuous discharge cycle interrupted by charge, will not be able to calibrate");
            } else if(soc->in_uc) {
                FURI_LOG_D(TAG, "Continuous discharge discontinuity: %lld/%lld mC", soc->in_uc, BATTERY_SOC_CYCLE_INTERRUPT_THRESHOLD / 1000);
            }
        }
    } else {
        furi_assert(delta_uc <= 0);
        soc->saved.total_out_uc -= delta_uc;
        soc->out_uc += delta_uc;
        if(soc->full_charge_cycle) {
            if(soc->out_uc > BATTERY_SOC_CYCLE_INTERRUPT_THRESHOLD) {
                soc->full_charge_cycle = false;
                FURI_LOG_W(TAG, "Continuous charge cycle interrupted by discharge, will not be able to calibrate");
            } else if(soc->out_uc) {
                FURI_LOG_D(TAG, "Continuous charge discontinuity: %lld/%lld mC", soc->out_uc, BATTERY_SOC_CYCLE_INTERRUPT_THRESHOLD / 1000);
            }
        }
    }

    float relative_current = fabs(measurements->charge_current_ua / 1000.0f) / soc->settings.current_max_ma;
    int32_t voltage_mv = measurements->voltage_uv / 1000;
    float v_based_soc = battery_soc_volt_based(soc->saved.v_table, is_charging, voltage_mv, relative_current);
    
    if(voltage_mv <= soc->settings.voltage_empty_mv) {
        if(!soc->limit_processed) battery_soc_limit(soc, BatterySocLimitingStateEmpty);
        soc->limit_processed = true;
    } else if(voltage_mv >= soc->settings.voltage_full_mv) {
        if(!soc->limit_processed) battery_soc_limit(soc, BatterySocLimitingStateFull);
        soc->limit_processed = true;
    } else {
        soc->limit_processed = false;
    }

    bool charge_algorithm_accurate = soc->charge_accurate && (soc->saved.flags & BatterySocSavedFlagCalibratedCharge);
    float c_based_soc = 0.0f;
    if(charge_algorithm_accurate) {
        c_based_soc = battery_soc_coulomb_based(soc, is_charging);
        size_t i_bucket = MAX(relative_current * BATTERY_SOC_V_TABLE_I_STEPS, BATTERY_SOC_V_TABLE_I_STEPS - 1);
        size_t soc_bucket = MAX(c_based_soc / 100.0f * BATTERY_SOC_V_TABLE_STEPS, BATTERY_SOC_V_TABLE_STEPS - 1);
        soc->saved.v_table[is_charging][i_bucket][soc_bucket] = voltage_mv;
        soc->saved.flags |= BatterySocSavedFlagCalibratedVoltage;
    }

    BATTERY_SOC_T(TAG, "info:");
    BATTERY_SOC_T(TAG, "  measurements: %ld mV, %ld mA", voltage_mv, measurements->charge_current_ua / 1000);
    BATTERY_SOC_T(TAG, "  v-based: %.2f%%", v_based_soc);
    BATTERY_SOC_T(TAG, "  delta: %lld ms, %+.1f mC", delta_ms, delta_uc / 1000.0f);
    BATTERY_SOC_T(TAG, "  battery: %.1f mC", soc->current_uc / 1000.0f);
    BATTERY_SOC_T(TAG, "  cumulative: chg=[%.1f mC], dschg=[%.1f mC]",
        soc->saved.total_in_uc / 1000.0f, soc->saved.total_out_uc / 1000.0f);

    BatterySocLevel level;
    level.flags = 0;

    if(charge_algorithm_accurate) {
        level.flags |= BatterySocLevelFlagChargeAccurate;
        level.charge_percent = c_based_soc;

        level.flags |= BatterySocLevelFlagKnownHealth;
        level.health_percent = (float)soc->saved.discharge_capacity_uc * 100.0f / (float)soc->saved.first_dischg_capacity_uc;

        level.flags |= BatterySocLevelFlagKnownDetails;

        level.detailed.voltage_based_percent = v_based_soc;
        level.detailed.charge_based_percent = c_based_soc;

        level.detailed.charge_capacity_mah = soc->saved.charge_capacity_uc / 3600 / 1000;
        level.detailed.discharge_capacity_mah = soc->saved.discharge_capacity_uc / 3600 / 1000;
        level.detailed.efficiency = efficiency;

        level.detailed.charge_error = (float)soc->saved.last_error_uc * 100.0f / (float)soc->saved.discharge_capacity_uc;
        level.detailed.charge_cycles = (float)soc->saved.total_in_uc / (float)soc->saved.charge_capacity_uc;

    } else {
        level.charge_percent = v_based_soc;
    }

    return level;
}
