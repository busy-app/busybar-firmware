#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnimTitleCard AnimTitleCard;

AnimTitleCard* anim_title_card_alloc(Widget* parent);

void anim_title_card_free(AnimTitleCard* instance);

Widget* anim_title_card_get_base(AnimTitleCard* instance);

void anim_title_card_set_icon(AnimTitleCard* instance, const char* file_path);

void anim_title_card_set_title(AnimTitleCard* instance, const char* title);

void anim_title_card_run_background_anim(AnimTitleCard* instance);

void anim_title_card_run_icon_anim(AnimTitleCard* instance, uint32_t start, uint32_t stop);

void anim_title_card_run_title_anim(
    AnimTitleCard* instance,
    int32_t start,
    int32_t stop,
    uint32_t duration);

#ifdef __cplusplus
}
#endif
