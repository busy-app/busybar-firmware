#pragma once

#include "busy_timer.h"

#include <furi.h>

#define TAG "BusyTimer"

struct BusyTimer {
    FuriEventLoop* event_loop;
    FuriMessageQueue* api_queue;
    BusyTimerSnapshot snapshot;
};
