#pragma once

#include "canvas.h"
#include <gui/modules/image.h>
#include <gui/modules/anim_player.h>
#include <gui/modules/label.h>
#include <gui/modules/countdown.h>
#include "widgets/rectangle.h"

typedef struct {
    CanvasSrv* canvas;
    char* id;
} CanvasWidgetTimeoutContext;

typedef struct {
    FuriEventLoopTimer* timeout_timer;
    CanvasWidgetTimeoutContext* timeout_context;
    CanvasElementType type;
    GuiDisplayId display;
    size_t z_index;
    union {
        Image* image;
        AnimPlayer* anim_player;
        Label* text;
        Countdown* countdown;
        RectangleWidget* rectangle;
    };
} CanvasWidget;

void canvas_widget_update(CanvasWidget* widget, Widget* root, const CanvasElement* element);

void canvas_widget_delete(CanvasWidget* widget);

void canvas_widget_to_front(CanvasWidget* widget);
