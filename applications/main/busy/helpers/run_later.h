#pragma once

#include <core/event_loop.h>

typedef struct RunLater RunLater;

typedef void (*RunLaterCallback)(void* context);

RunLater* run_later(
    FuriEventLoop* event_loop,
    RunLaterCallback callback,
    void* context,
    uint32_t delay_ms);

void run_later_cancel(RunLater* instance);
