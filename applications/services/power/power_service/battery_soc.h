/**
 * @file battery_soc.h
 * Battery state-of-charge estimation module
 */

#pragma once

#include <furi.h>

typedef FuriWait MsSinceBoot;

typedef struct BatterySoc BatterySoc;

typedef struct {
    const char* storage_path; //<! path to file where this module may store internal data. may be NULL

    int32_t nominal_capacity_mah;

    int32_t current_max_ma;
    int32_t voltage_empty_mv;
    int32_t voltage_full_mv;
    int32_t hysteresis_mv;
} BatterySocSettings;

BatterySoc* battery_soc_alloc(const BatterySocSettings* settings);

void battery_soc_free(BatterySoc* soc);

/**
 * @brief Saves internal persistent state to a file in storage
 */
void battery_soc_sync(BatterySoc* soc);

typedef struct {
    MsSinceBoot timestamp;
    int32_t charge_current_ua; //<! negative means discharging
    int32_t voltage_uv;
} BatterySocMeasurements;

typedef enum {
    BatterySocLevelFlagNoData = 1 << 0, //<! no data known yet
    BatterySocLevelFlagKnownHealth = 1 << 1, //<! `health_percent` field is meaningful
    BatterySocLevelFlagChargeAccurate = 1 << 2, //<! `charge_percent` field reports data from Coulomb-based algorithm (instead of Volt-based)
    BatterySocLevelFlagKnownDetails = 1 << 3, //<! detailed battery characteristics available (in `detailed` field)
    BatterySocLevelFlagMAX,
} BatterySocLevelFlag;

typedef struct {
    BatterySocLevelFlag flags;

    // just the most accurate estimation:
    float charge_percent;
    float health_percent; //<! must have `KnownHealth` flag
    
    // detailed information for debugging:
    struct {
        float voltage_based_percent; //<! Volt-based algorithm charge percentage estimation
        float charge_based_percent; //<! Coulomb-based algorithm charge percentage estimation

        int32_t charge_capacity_mah; //<! mAh the battery takes before charger stops charging
        int32_t discharge_capacity_mah; //<! mAh the battery releases before device shuts off
        float efficiency; //<! `discharge_capacity_mah` / `charge_capacity_mah` (percentage)

        float charge_error; //<! actual charge level is within +/- `charge_error` percent (not percentage points) of `charge_percent`
        float charge_cycles; //<! charge cycle count
    } detailed;
} BatterySocLevel;

/**
 * @brief Feeds the algorithm instantaneous measurements from the charger IC
 */
BatterySocLevel battery_soc_feed_measurements(BatterySoc* soc, const BatterySocMeasurements* measurements);
