/**
 * @file image.h
 * @brief A widget that displays a static image.
 */
#pragma once

#include <gui/widget.h>

typedef struct Image Image;

Image* image_alloc(Widget* parent);

void image_free(Image* instance);

bool image_set_source(Image* instance, const char* file_path);
