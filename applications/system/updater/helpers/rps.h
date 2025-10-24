#pragma once

#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FURI_PACKED {
    uint8_t build;
    uint8_t security;
    uint8_t minor;
    uint8_t major;
} UpdaterRpsFwVersion;

typedef struct FURI_PACKED {
    uint8_t patch;
    uint8_t customer_id;
    uint8_t rom_id;
    uint8_t chip_id;
} UpdaterRpsExtFwVersion;

bool updater_rps_read_version(
    File* rps_file,
    UpdaterRpsFwVersion* version,
    UpdaterRpsExtFwVersion* ext_version);

#ifdef __cplusplus
}
#endif
