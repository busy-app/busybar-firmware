#pragma once

#include "../busy_theme.h"

typedef struct ThemePickerModel ThemePickerModel;

ThemePickerModel* theme_picker_model_alloc(void);

void theme_picker_model_free(ThemePickerModel* instance);

void theme_picker_model_add_item(ThemePickerModel* instance, const BusyTheme* theme);

const BusyTheme* theme_picker_model_get_item(const ThemePickerModel* instance, uint32_t index);

uint32_t theme_picker_model_get_item_count(const ThemePickerModel* instance);
