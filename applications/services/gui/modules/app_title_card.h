#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AppTitleCard AppTitleCard;

AppTitleCard* app_title_card_alloc(Widget* parent);

void app_title_card_free(AppTitleCard* instance);

Widget* app_title_card_get_base(AppTitleCard* instance);

void app_title_card_set_image(AppTitleCard* instance, const char* file_path);

void app_title_card_set_text(AppTitleCard* instance, const char* text);

#ifdef __cplusplus
}
#endif
