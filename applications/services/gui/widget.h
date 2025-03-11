#pragma once

#include <stdint.h>

typedef struct Widget Widget;

Widget* widget_alloc(Widget* parent);

void widget_free(Widget* instance);

Widget* widget_get_parent(const Widget* instance);
