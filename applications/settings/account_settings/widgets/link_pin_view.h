#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LinkPinView LinkPinView;

LinkPinView* link_pin_view_front_alloc(Widget* parent);

LinkPinView* link_pin_view_back_alloc(Widget* parent);

void link_pin_view_free(LinkPinView* instance);

void link_pin_view_set_state(LinkPinView* instance, const char* pin_code);

#ifdef __cplusplus
}
#endif
