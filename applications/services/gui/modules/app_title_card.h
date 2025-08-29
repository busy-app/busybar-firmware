#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AppTitleCard AppTitleCard;

AppTitleCard* app_title_card_alloc(Widget* parent, bool use_anim_image);

void app_title_card_free(AppTitleCard* instance);

Widget* app_title_card_get_base(AppTitleCard* instance);

void app_title_card_set_image(AppTitleCard* instance, const char* file_path);

void app_title_card_set_anim_image(AppTitleCard* instance, const char* file_path);

void app_title_card_set_text(AppTitleCard* instance, const char* text);

void app_title_card_run_image_anim(AppTitleCard* instance, uint32_t start, uint32_t stop);

void app_title_card_run_text_anim(
    AppTitleCard* instance,
    int32_t start,
    int32_t stop,
    uint32_t duration);

#ifdef __cplusplus
}
#endif
