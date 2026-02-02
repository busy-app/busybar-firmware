#pragma once

#include <furi.h>

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t build;
    uint8_t security;
    uint8_t rom_id;
    uint8_t chip_id;
    uint8_t customer_id;
} SlRpsNwpVersion;

void sl_rps_format_nwp_version(FuriString* string, const SlRpsNwpVersion* data);
