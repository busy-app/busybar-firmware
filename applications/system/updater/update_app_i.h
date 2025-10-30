#pragma once

#include "worker/update_task.h"

#include <gui/gui.h>
#include <gui/scene_manager.h>

#include <storage/storage.h>

typedef struct {
    Gui* gui;
    Storage* storage;
    FuriEventLoop* event_loop;
    UpdateTask* update_task;
    SceneManager* scene_manager;

    Widget* back_container;
    Widget* front_container;

    const char* update_status;
    uint8_t update_percent;
} UpdaterApp;
