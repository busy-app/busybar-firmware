#pragma once

#include "status_lights.h"
#include "status_lights_preset_defs.h"

#include <furi/furi.h>

#define TAG "StatusLightsSrv"

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
    FuriEventLoopTimer* timer;

    StatusLightsGenericPreset* preset_instance;
    const StatusLightsPresetBase* preset_api;
};
