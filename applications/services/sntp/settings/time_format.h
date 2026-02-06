/**
 * @file time_format.h
 * @brief Display time format description
 */
#pragma once

typedef enum {
    SntpSettingTimeFormat24h = 0,
    SntpSettingTimeFormat12h,

    SntpSettingTimeFormatCount,
} SntpSettingTimeFormat;
