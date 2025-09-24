#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

void check_update_startup(void* context);

bool check_update_is_new_version(void);
void check_update_get_current_version(FuriString* current_version);
void check_update_get_new_version(FuriString* new_version);
void check_update_get_new_firmware_url(FuriString* url);
void check_update_get_new_firmware_sha256(FuriString* sha256);
void check_update_set_current_version(const char* version);

#ifdef __cplusplus
}
#endif
