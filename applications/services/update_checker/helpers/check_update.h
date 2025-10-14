#pragma once

#include <furi.h>

typedef struct CheckUpdate CheckUpdate;

typedef enum {
    CheckUpdateStatusSuccess = (1UL << 1),
    CheckUpdateStatusError = (1UL << 2),
    CheckUpdateStatusDone = (1UL << 3),
    CheckUpdateStatusNoNewVersion = (1UL << 4),
    CheckUpdateStatusNewVersion = (1UL << 5),
} CheckUpdateStatus;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CheckUpdateCallbackDone)(CheckUpdateStatus status, void* context);

CheckUpdate* check_update_init(void);
void check_update_free(CheckUpdate* instance);
void check_update_set_callback_done(
    CheckUpdate* instance,
    CheckUpdateCallbackDone callback,
    void* context);
void check_update_run(CheckUpdate* instance);
bool check_update_is_processing_done(CheckUpdate* instance);

void check_update_set_json_url(CheckUpdate* instance, const char* url);
void check_update_set_channel_id(CheckUpdate* instance, const char* channel_id);

bool check_update_is_new_version(CheckUpdate* instance);
void check_update_get_current_version(CheckUpdate* instance, FuriString* current_version);
void check_update_get_new_version(CheckUpdate* instance, FuriString* new_version);
void check_update_get_new_firmware_url(CheckUpdate* instance, FuriString* url);
void check_update_get_new_firmware_sha256(CheckUpdate* instance, FuriString* sha256);

#ifdef __cplusplus
}
#endif
