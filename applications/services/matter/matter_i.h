#pragma once

#include "matter.h"
#include "matter_common_i.h"

#include <api_lock.h>

#include <intercom/intercom.h>

typedef enum {
    MatterCustomEventRequest = 1UL << 0,
} MatterCustomEvent;

typedef enum {
    MatterApiMessageTypeInitBackend,
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

struct Matter {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timeout_timer;
    FuriSemaphore* api_semaphore;
    FuriMessageQueue* rx_queue;
    FuriPubSub* pubsub;
    FuriState* switch_state;
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    MatterApiMessage api_message;
    MatterCommissionedFabrics fabrics;
    MatterCertificationConfig cert_config;
};

// matter_api.c

void matter_init_backend(Matter* instance);

bool matter_api_is_waiting_for_response(Matter* instance, MatterApiMessageType message_type);

void matter_api_unlock(Matter* instance, MatterStatus status);

// matter_certification.c

MatterStatus matter_certification_read_config(MatterCertificationConfig* cert_info);

MatterStatus matter_certification_get_cd(
    MatterCertificationType cert_type,
    MatterCertificateDeclaration* cd);

MatterStatus matter_certification_set_config(MatterCertificationType cert_type);
