#pragma once

#include "matter.h"
#include "matter_cd.h"

#include "matter_common_i.h"

#include <api_lock.h>

#include <intercom/intercom.h>

#define TAG "Matter"

typedef enum {
    MatterApiRequestTypeSetSwitchState,
    MatterApiRequestTypeSetSwitchStartupMode,
    MatterApiRequestTypeReset,
    MatterApiRequestTypeCommission,
    MatterApiRequestTypeGetFabrics,
    MatterApiRequestTypeMax,
} MatterApiRequestType;

typedef struct {
    FuriApiLock lock;
    MatterApiRequestType type;
    bool success;
    union {
        struct {
            FuriString* qr_code;
            FuriString* manual_code;
            size_t window_duration;
        };
        MatterSwitchState switch_state;
        MatterCommissionedFabrics fabrics;
        MatterSwitchStartupMode startup_mode;
    };
} MatterApiRequest;

struct MatterSrv {
    FuriEventLoop* event_loop;
    FuriMessageQueue* frame_queue;
    FuriMessageQueue* request_queue;
    FuriPubSub* pubsub;
    FuriState* switch_state;
    IntercomChannel* intercom_ch;
    MatterCd cd;
    MatterCommissionedFabrics fabrics;
    bool first_frame_sent;
};
