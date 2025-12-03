#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UpdateChecker UpdateChecker;

typedef struct {
    const FuriString* url;
    const FuriString* id;
    const FuriString* version;
    const FuriString* sha256;
    const FuriString* changelog;
} UpdaterCheckerInfo;

typedef void (
    *UpdateCheckerDoneCallback)(bool is_success, UpdaterCheckerInfo* update_info, void* context);

UpdateChecker* update_checker_alloc(void);
void update_checker_free(UpdateChecker* instance);

void update_checker_set_done_callback(
    UpdateChecker* instance,
    UpdateCheckerDoneCallback callback,
    void* context);

bool update_checker_run(UpdateChecker* instance, const char* url, const char* channel_id);

#ifdef __cplusplus
}
#endif
