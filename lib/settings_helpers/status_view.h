#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct StatusView StatusView;

StatusView* status_view_alloc(Widget* parent);

void status_view_free(StatusView* instance);

Widget* status_view_get_base(StatusView* instance);

void status_view_set_icon(StatusView* instance, const char* path);

void status_view_set_header(StatusView* instance, const char* header);

void status_view_set_additional_text(StatusView* instance, const char* text);

#ifdef __cplusplus
}
#endif
