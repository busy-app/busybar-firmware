#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NamedLabelView NamedLabelView;

NamedLabelView* named_label_view_back_alloc(Widget* parent);

void named_label_view_back_free(NamedLabelView* instance);

void named_label_set_title(NamedLabelView* instance, const char* title);

void named_label_set_text(NamedLabelView* instance, const char* text);

void named_label_set_text_color(NamedLabelView* instance, Color color);
#ifdef __cplusplus
}
#endif
