/**
 * @file telemetry_settings.h
 * @brief Telemetry service settings (collection consent).
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TELEMETRY_SETTINGS_FILE_PATH APP_DATA_PATH("telemetry_settings.json")
#define TELEMETRY_SETTINGS_VERSION   1

typedef struct {
    bool is_enabled; /**< Telemetry collection enabled (default false) */
    bool is_disable_pending; /**< telemetry.disable not yet delivered to the backend */
} TelemetrySettings;

void telemetry_settings_load(TelemetrySettings* settings);

void telemetry_settings_save(const TelemetrySettings* settings);

#ifdef __cplusplus
}
#endif
