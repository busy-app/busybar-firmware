/**
 * @file view_port.h
 */
#pragma once

#include "gui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ViewPort ViewPort;

ViewPort* view_port_alloc(Gui* gui, GuiDisplayId display_id, GuiLayerId layer_id);

void view_port_free(ViewPort* instance);

void view_port_set_width(ViewPort* instance, int32_t width);

void view_port_set_height(ViewPort* instance, int32_t height);

void view_port_set_size(ViewPort* instance, int32_t width, int32_t height);

void view_port_move_to_foreground(ViewPort* instance);

void view_port_move_to_background(ViewPort* instance);

#ifdef __cplusplus
}
#endif
