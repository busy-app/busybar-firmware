#include "power_i.h"

#if defined(SRV_STORAGE)
#include <storage/storage.h>
#endif

#define TAG "PowerSoc"

// First 2 and last 2 points are encoded as full 16-bit numbers (because they
// typically have a large difference). The rest are encoded as 8-bit deltas from
// previous point to conserve space (400 bytes in flash is significant).
#define END_FULL_POINTS   2
#define TOTAL_FULL_POINTS (END_FULL_POINTS * 2)

// Some sort of imperfection in the charger offsets measured battery voltage by
// ~10mV up. This offset is not present when discharging and not dependent on
// charging current (i.e. not caused by a resistance). This offset was not
// present in out-of-band measurements.
// On one unit, this appears to be 60mV, and 10mV on all the other units.
#define CHARGER_OFFSET_MV         10
#define CHARGER_OFFSET_TRIGGER_MA +2

static const uint8_t power_crude_cal_curves[] = {
    83,  10,  178, 10,  111, 107, 78,  60,  47,  49,  49,  49,  48,  48,  35,  53,  40,  48,  49,
    40,  29,  121, 14,  53,  15,  120, 10,  231, 10,  108, 105, 78,  60,  48,  49,  50,  49,  49,
    48,  38,  51,  41,  49,  50,  40,  28,  174, 14,  92,  15,  158, 10,  27,  11,  107, 102, 77,
    61,  47,  50,  50,  50,  48,  49,  40,  50,  41,  51,  49,  40,  27,  223, 14,  128, 15,  195,
    10,  79,  11,  105, 99,  76,  61,  47,  50,  50,  50,  49,  49,  41,  49,  42,  51,  50,  40,
    26,  14,  15,  162, 15,  233, 10,  131, 11,  102, 96,  75,  62,  46,  50,  50,  50,  49,  49,
    42,  48,  42,  52,  49,  40,  25,  57,  15,  194, 15,  16,  11,  182, 11,  101, 92,  74,  61,
    46,  50,  49,  50,  49,  49,  43,  47,  41,  52,  50,  40,  24,  97,  15,  223, 15,  54,  11,
    233, 11,  98,  89,  72,  61,  46,  49,  49,  49,  49,  49,  44,  45,  41,  52,  49,  40,  23,
    135, 15,  250, 15,  93,  11,  28,  12,  96,  85,  70,  60,  45,  49,  48,  49,  48,  48,  45,
    43,  42,  51,  49,  39,  22,  169, 15,  18,  16,  132, 11,  79,  12,  93,  81,  68,  59,  45,
    47,  47,  49,  47,  48,  44,  42,  41,  52,  47,  39,  21,  201, 15,  40,  16,  171, 11,  129,
    12,  90,  78,  65,  58,  44,  46,  46,  47,  47,  47,  45,  40,  40,  51,  47,  37,  21,  229,
    15,  60,  16,  210, 11,  179, 12,  87,  74,  63,  55,  43,  46,  44,  46,  46,  46,  44,  39,
    40,  50,  46,  36,  20,  254, 15,  77,  16,  250, 11,  229, 12,  84,  69,  60,  54,  42,  44,
    42,  46,  44,  45,  44,  37,  39,  48,  45,  36,  18,  21,  16,  92,  16,  34,  12,  23,  13,
    80,  65,  57,  52,  40,  42,  41,  44,  44,  43,  43,  36,  37,  47,  44,  34,  18,  40,  16,
    105, 16,  74,  12,  72,  13,  77,  60,  54,  49,  39,  40,  40,  42,  42,  42,  42,  34,  36,
    46,  42,  32,  17,  56,  16,  115, 16,  114, 12,  121, 13,  74,  54,  51,  47,  37,  38,  37,
    41,  40,  41,  40,  32,  35,  44,  40,  32,  15,  69,  16,  122, 16,  155, 12,  170, 13,  70,
    49,  47,  44,  36,  35,  35,  39,  39,  39,  38,  30,  34,  41,  39,  30,  14,  80,  16,  127,
    16,  195, 12,  218, 13,  66,  44,  44,  40,  35,  32,  32,  37,  37,  37,  36,  29,  32,  39,
    37,  28,  13,  87,  16,  130, 16,  236, 12,  10,  14,  63,  38,  39,  37,  33,  29,  30,  34,
    36,  35,  33,  27,  30,  37,  35,  25,  13,  91,  16,  131, 16,  22,  13,  58,  14,  59,  32,
    35,  34,  30,  26,  27,  32,  34,  32,  31,  25,  28,  34,  33,  23,  11,  92,  16,  129, 16,
    63,  13,  106, 14,  54,  27,  30,  30,  28,  23,  24,  28,  32,  31,  27,  23,  26,  31,  31,
    21,  10,  91,  16,  124, 16,  105, 13,  153, 14,  50,  20,  26,  26,  26,  19,  21,  25,  30,
    28,  24,  21,  24,  27,  29,  18,  9,   86,  16,  117, 16,
};
static const PowerBatCalibration power_crude_cal = {
    .percent_points = 21,
    .current_points = 21,
    .current_range = 3000,
    .tolerance = 10,
    .curves = (uint8_t*)power_crude_cal_curves,
    .is_in_flash = true,
};

#define BAT_CAL_SIGNATURE "bsbbcal"
#define BAT_CAL_VERSION   0

typedef struct FURI_PACKED {
    char signature[7];
    uint8_t version;
    uint16_t tolerance;
    uint16_t current_range;
    uint16_t percent_points;
    uint16_t current_points;
} PowerBatCalHeader;

static inline size_t power_curve_size(uint16_t percent_points) {
    furi_check(percent_points >= TOTAL_FULL_POINTS);
    return (percent_points - TOTAL_FULL_POINTS) * sizeof(int8_t) +
           (sizeof(uint16_t) * TOTAL_FULL_POINTS);
}

static inline bool power_is_end_point(uint16_t percent_points, uint16_t index) {
    return (index < END_FULL_POINTS) || (index >= percent_points - END_FULL_POINTS);
}

PowerBatCalibration* power_get_crude_calibration(void) {
    return (PowerBatCalibration*)&power_crude_cal;
}

PowerBatCalibration* power_load_bat_calibration(const char* path) {
    furi_check(path);

#if defined(SRV_STORAGE)
    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = NULL;
    PowerBatCalibration* cal = NULL;
    uint8_t* curves = NULL;

    do {
        file = storage_file_alloc(storage);
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", path);
            break;
        }

        PowerBatCalHeader header;
        if(storage_file_read(file, &header, sizeof(header)) != sizeof(header)) {
            FURI_LOG_E(TAG, "Incomplete header: %s", path);
            break;
        }

        if(strcmp(header.signature, BAT_CAL_SIGNATURE) != 0) {
            FURI_LOG_E(TAG, "Signature mismatch: %s", path);
            break;
        }

        if(header.version != BAT_CAL_VERSION) {
            FURI_LOG_E(
                TAG,
                "Unsupported version: %s declares %hhu, we only support %d",
                path,
                header.version,
                BAT_CAL_VERSION);
            break;
        }

        size_t curves_size = power_curve_size(header.percent_points) * header.current_points;
        curves = malloc(curves_size);
        if(storage_file_read(file, curves, curves_size) != curves_size) {
            FURI_LOG_E(TAG, "Incomplete curves: %s", path);
            break;
        }

        uint16_t current_step = header.current_range * 2 / header.current_points;
        uint16_t percent_step = 100 / header.percent_points;
        FURI_LOG_I(
            TAG,
            "Loaded cal: -%hdmA<=I<=%hdmA, Istep=%hdmA, %%step=%hd%%, tol=+/-%hd%%",
            header.current_range,
            header.current_range,
            current_step,
            percent_step,
            header.tolerance);

        cal = malloc(sizeof(PowerBatCalibration));

        cal->curves = curves;
        cal->percent_points = header.percent_points;
        cal->current_points = header.current_points;
        cal->current_range = header.current_range;
        cal->tolerance = header.tolerance;
        cal->is_in_flash = false;

        success = true;
    } while(0);

    if(!success && curves) free(curves);
    if(!success) free(cal);
    if(file) storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success ? cal : NULL;
#else
    return NULL;
#endif
}

void power_unload_bat_calibration(PowerBatCalibration* cal) {
    furi_check(cal);
    if(cal->is_in_flash) return;
    free(cal->curves);
    free(cal);
}

uint8_t power_get_battery_charge(
    const PowerBatCalibration* cal,
    int32_t voltage_mv,
    int32_t current_ma) {
    furi_check(cal);

    // read comments for these macros for explanation
    if(current_ma >= CHARGER_OFFSET_TRIGGER_MA) voltage_mv -= CHARGER_OFFSET_MV;

    float normalized_current = (float)current_ma / (float)cal->current_range;
    int32_t buckets_per_sign = floorf((float)cal->current_points / 2.0f);
    int32_t current_bucket = roundf((normalized_current * buckets_per_sign) + buckets_per_sign);

    bool is_current_out_of_range = (current_bucket < 0) || (current_bucket >= cal->current_points);
    current_bucket = CLAMP(current_bucket, cal->current_points - 1, 0);

    size_t curve_size = power_curve_size(cal->percent_points);
    const uint8_t* curve = &cal->curves[current_bucket * curve_size];

    int32_t best_p = 0;
    int32_t best_mv_delta = INT32_MAX;
    float percent_per_p_point = 100.0f / (float)(cal->percent_points - 1);

    int32_t curve_point_mv = 0;
    size_t curve_ptr = 0;

    for(size_t p_point = 0; p_point < cal->percent_points; p_point++) {
        if(power_is_end_point(cal->percent_points, p_point)) {
            uint16_t full = *(const uint16_t*)&curve[curve_ptr];
            curve_point_mv = full;
            curve_ptr += 2;
        } else {
            int8_t delta = *(const int8_t*)&curve[curve_ptr];
            curve_point_mv += delta;
            curve_ptr++;
        }

        int32_t mv_delta = abs(curve_point_mv - voltage_mv);
        if(mv_delta < best_mv_delta) {
            best_p = percent_per_p_point * p_point;
            best_mv_delta = mv_delta;
        }
    }

    int32_t p_lower_bound = best_p * (1.0 - ((float)cal->tolerance / 100.0f));
    int32_t p_upper_bound = best_p * (1.0 + ((float)cal->tolerance / 100.0f));

    FURI_LOG_T(
        TAG,
        "%ldmV, %ldmA => %ld%% (%ld%%..%ld%%), Ibkt=%ld%s, ΔmV=%ld",
        voltage_mv,
        current_ma,
        best_p,
        p_lower_bound,
        p_upper_bound,
        current_bucket,
        is_current_out_of_range ? " (out of range)" : "",
        best_mv_delta);
    furi_assert(best_mv_delta < INT32_MAX);

    return best_p;
}
