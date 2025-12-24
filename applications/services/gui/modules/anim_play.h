/**
 * @file anim_play.h
 * @brief A widget that plays an AnimFile.
 */
#pragma once

#include <gui/widget.h>
#include <anim_file/anim_file.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnimPlay AnimPlay;

AnimPlay* anim_play_alloc(Widget* parent);

void anim_play_free(AnimPlay* instance);

Widget* anim_play_get_base(AnimPlay* instance);

bool anim_play_set_source(AnimPlay* instance, const char* path);

AnimFile* anim_play_get_file(AnimPlay* instance);

void anim_play_start(AnimPlay* instance);

void anim_play_pause(AnimPlay* instance);

#ifdef __cplusplus
}
#endif
