#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <core/string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t updater_stage_crc32;
    FuriString* updater_stage;
    FuriString* updater_resources;
    FuriString* updater_sil_fw;
    FuriString* updater_sil_radio_fw;
    FuriString* updater_dfu;
    uint8_t target;
} UpdaterConfig;

UpdaterConfig* updater_config_alloc(void);
void updater_config_free(UpdaterConfig* config);
void updater_config_prefix_paths(UpdaterConfig* config, const char* prefix);
bool updater_config_from_memory(UpdaterConfig* config, const char* json_data, size_t json_size);
bool updater_config_from_file(UpdaterConfig* config, File* file);

#ifdef __cplusplus
}
#endif
