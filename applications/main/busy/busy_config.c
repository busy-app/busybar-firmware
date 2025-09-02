#include "busy_config.h"

const BusyConfig busy_config_default = {
    .name = "",
    .default_timer_config = {
        .mode = BusyTimerModeInterval,
        .time_mn = 20, // minutes
        .work_time_mn = 20, // minutes
        .rest_time_mn = 5, // minutes
        .cycle_count = 3,
        .enable_intervals = true,
        .enable_autostart = false,
        .enable_demo_mode = false,
    }};
