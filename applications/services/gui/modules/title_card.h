#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TitleCard TitleCard;

TitleCard* title_card_alloc(Widget* parent);

void title_card_free(TitleCard* instance);

Widget* title_card_get_base(TitleCard* instance);

void title_card_set_icon(TitleCard* instance, const char* file_path);

void title_card_set_title(TitleCard* instance, const char* title);

#ifdef __cplusplus
}
#endif
