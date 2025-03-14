#pragma once

#include <gui/widget.h>

typedef struct AnimImage AnimImage;

AnimImage* anim_image_alloc(Widget* parent);

void anim_image_free(AnimImage* instance);

bool anim_image_set_source(AnimImage* instance, const char* file_path);

void anim_image_start(AnimImage* instance);

void anim_image_stop(AnimImage* instance);
