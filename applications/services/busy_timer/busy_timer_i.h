#pragma once

#include "busy_timer.h"

#include <furi.h>
#include <api_lock.h>

#define TAG "BusyTimer"

typedef enum {
    BusyTimerApiMessageTypeGetSnapshot,
    BusyTimerApiMessageTypeSetSnapshot,
    BusyTimerApiMessageTypeMax,
} BusyTimerApiMessageType;

typedef struct {
    BusyTimerSnapshot* snapshot;
} BusyTimerApiMessageGetSnapshot;

typedef struct {
    const BusyTimerSnapshot* snapshot;
} BusyTimerApiMessageSetSnapshot;

typedef struct {
    BusyTimerApiMessageType type;
    FuriApiLock lock;
    union {
        BusyTimerApiMessageGetSnapshot get_snapshot;
        BusyTimerApiMessageSetSnapshot set_snapshot;
    };
} BusyTimerApiMessage;

struct BusyTimer {
    FuriEventLoop* event_loop;
    FuriMessageQueue* api_queue;
    BusyTimerSnapshot snapshot;
};
