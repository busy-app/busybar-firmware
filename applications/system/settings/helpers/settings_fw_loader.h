#pragma once
#include <furi.h>

typedef struct SettingsFwLoader SettingsFwLoader;

typedef struct {
    size_t total_download_size;
    size_t received_download_size;
    uint32_t speed_bytes_per_sec;
} SettingsFwLoaderStatus;

typedef void (*SettingsFwLoaderCallbackStatus)(SettingsFwLoaderStatus status, void* context);
typedef void (*SettingsFwLoaderCallbackState)(FuriString* state, void* context);

SettingsFwLoader* settings_fw_loader_alloc(void);
void settings_fw_loader_free(SettingsFwLoader* instance);
void settings_fw_loader_run(SettingsFwLoader* instance, const char* url);
bool settings_fw_loader_is_processing_done(SettingsFwLoader* instance);
void settings_fw_loader_set_status_callback(
    SettingsFwLoader* instance,
    SettingsFwLoaderCallbackStatus callback,
    void* context);
void settings_fw_loader_set_state_callback(
    SettingsFwLoader* instance,
    SettingsFwLoaderCallbackState callback,
    void* context);
