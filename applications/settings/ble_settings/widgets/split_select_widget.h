#pragma once

#include <gui/gui.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>

typedef void (*WidgetButtonCallback)(uint32_t index, void* context);

typedef struct SplitWidget SplitWidget;

SplitWidget* split_widget_alloc(Widget* base, const char* title);
void split_widget_free(SplitWidget* split_widget);

void split_widget_add_button(
    SplitWidget* split_widget,
    const char* text,
    uint32_t index,
    WidgetButtonCallback callback,
    void* ctx);
