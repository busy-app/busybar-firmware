#pragma once

#include <furi/furi.h>

/** Record key to access the UpdateChecker instance. */
#define RECORD_UPDATE_CHECKER "update_checker"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct UpdateChecker UpdateChecker;

typedef enum {
    UpdateCheckerEventNewVersion,
    UpdateCheckerEventNoNewVersion,
    UpdateCheckerEventNoWifiConnection,
    UpdateCheckerEventError,
} UpdateCheckerEventType;

typedef struct {
    UpdateCheckerEventType type;
} UpdateCheckerEvent;

FuriPubSub* update_checker_get_pubsub(UpdateChecker* instance);

void update_checker_check_update(UpdateChecker* instance);
bool update_checker_is_new_version(UpdateChecker* instance);
void update_checker_get_current_version(UpdateChecker* instance, FuriString* current_version);
void update_checker_get_new_version(UpdateChecker* instance, FuriString* new_version);
void update_checker_get_new_firmware_url(UpdateChecker* instance, FuriString* url);
void update_checker_get_new_firmware_sha256(UpdateChecker* instance, FuriString* sha256);

#ifdef __cplusplus
}
#endif
