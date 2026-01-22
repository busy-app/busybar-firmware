#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/label.h>
#include <gui/modules/anim_player.h>

#include <storage/storage.h>

typedef enum {
    AnimationPlayerAppEventExit,
} AnimationPlayerAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    Gui* gui;

    AnimPlayer* anim_player;
} AnimationPlayerApp;
