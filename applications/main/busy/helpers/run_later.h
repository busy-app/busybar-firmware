#pragma once

#include <core/event_loop.h>

typedef void (*RunLaterCallback)(void* context);

void run_later(
    FuriEventLoop* event_loop,
    RunLaterCallback callback,
    void* context,
    uint32_t delay_ms);
