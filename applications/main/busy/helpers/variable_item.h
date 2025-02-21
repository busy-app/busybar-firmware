#pragma once

#include <lvgl.h>

lv_obj_t* lv_variable_item_add(lv_obj_t* parent, const char* text);

void lv_variable_item_set_text(lv_obj_t* item, const char* text);

void lv_variable_item_set_range(lv_obj_t* item, int32_t min, int32_t max);

void lv_variable_item_set_step(lv_obj_t* item, int32_t step);

void lv_variable_item_set_value(lv_obj_t* item, int32_t value);

void lv_variable_item_set_binary(lv_obj_t* item, bool set);

void lv_variable_item_set_min_as_inf(lv_obj_t* item, bool set);
