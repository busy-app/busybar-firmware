/**
 * @file label.h
 * @brief A simple text label widget.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Label Label;

Label* label_alloc(Widget* parent);

void label_free(Label* instance);

void label_set_font(Label* instance); // TODO: Font

void label_set_text(Label* instance, const char* text);

void label_set_text_fmt(Label* instance, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
