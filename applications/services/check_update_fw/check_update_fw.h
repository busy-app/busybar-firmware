#pragma once

#include <furi/furi.h>

/** Record key to access the CheckUpdateFw instance. */
#define RECORD_CHECK_UPDATE_FW "check_update_fw"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct CheckUpdateFw CheckUpdateFw;

typedef enum {
    CheckUpdateFwEventNewVersion,
    CheckUpdateFwEventNoNewVersion,
    CheckUpdateFwEventError,
} CheckUpdateFwEventType;

typedef struct {
    CheckUpdateFwEventType type;
} CheckUpdateFwEvent;

FuriPubSub* check_update_fw_get_pubsub(CheckUpdateFw* instance);

void check_update_fw_startup(CheckUpdateFw* instance);
bool check_update_fw_is_new_version(CheckUpdateFw* instance);
void check_update_fw_get_current_version(CheckUpdateFw* instance, FuriString* current_version);
void check_update_fw_get_new_version(CheckUpdateFw* instance, FuriString* new_version);
void check_update_fw_get_new_firmware_url(CheckUpdateFw* instance, FuriString* url);
void check_update_fw_get_new_firmware_sha256(CheckUpdateFw* instance, FuriString* sha256);

#ifdef __cplusplus
}
#endif
