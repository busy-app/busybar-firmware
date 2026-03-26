#pragma once

#include "matter.h"
#include "matter_cd.h"

#include "matter_common_i.h"

#include <api_lock.h>

#include <intercom/intercom.h>

#define TAG "Matter"

typedef enum {
    MatterCustomEventRequest = 1UL << 0,
} MatterCustomEvent;

typedef enum {
    MatterApiMessageTypeSetSwitchState,
    MatterApiMessageTypeSetSwitchStartupMode,
    MatterApiMessageTypeStartCommissioning,
    MatterApiMessageTypeGetFabrics,
    MatterApiMessageTypeFactoryReset,
    MatterApiMessageTypeMax,
} MatterApiMessageType;

typedef struct {
    MatterSwitchState state;
} MatterApiMessageSetSwitchState;

typedef struct {
    MatterSwitchStartupMode mode;
} MatterApiMessageSetSwitchStartupMode;

typedef struct {
    MatterCommissioningInfo* info;
} MatterApiMessageStartCommissioning;

typedef struct {
    MatterCommissionedFabrics* fabrics;
} MatterApiMessageGetFabrics;

typedef union {
    MatterApiMessageSetSwitchState set_switch_state;
    MatterApiMessageSetSwitchStartupMode set_switch_startup_mode;
    MatterApiMessageStartCommissioning start_commissioning;
    MatterApiMessageGetFabrics get_fabrics;
} MatterApiMessageData;

typedef struct {
    MatterApiMessageType type;
    MatterApiMessageData data;
    MatterStatus* status;
    FuriApiLock lock;
} MatterApiMessage;

struct MatterSrv {
    FuriEventLoop* event_loop;
    FuriSemaphore* api_semaphore;
    FuriMessageQueue* frame_queue;
    FuriPubSub* pubsub;
    FuriState* switch_state;
    IntercomChannel* intercom_ch;
    MatterApiMessage api_message;
    MatterCd cd;
    MatterCommissionedFabrics fabrics;
    bool first_frame_sent;
};
