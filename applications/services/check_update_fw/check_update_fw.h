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
} CheckUpdateFwEventType;

typedef struct {
    CheckUpdateFwEventType type;
} CheckUpdateFwEvent;

FuriPubSub* check_update_fw_get_pubsub(CheckUpdateFw* instance);

#ifdef __cplusplus
}
#endif
