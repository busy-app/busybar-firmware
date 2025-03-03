#pragma once

#include "status_lights.h"

#include <furi/furi.h>

#define TAG "StatusLightsSrv"

typedef union {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct {
        uint8_t h;
        uint8_t s;
        uint8_t v;
    };
} Color;

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    Color color;
};
