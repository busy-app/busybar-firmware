#pragma once

#include <gui/widget.h>

typedef struct ImageAnimation ImageAnimation;

ImageAnimation* image_animation_alloc(Widget* parent);

void image_animation_free(ImageAnimation* instance);

bool image_animation_set_source(ImageAnimation* instance, const char* file_path);

void image_animation_start(ImageAnimation* instance);

void image_animation_stop(ImageAnimation* instance);
