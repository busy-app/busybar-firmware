#include "lv_label_ext.h"

void lv_label_ext_set_anim_speed(lv_obj_t* label, uint32_t px_per_min) {
    furi_check(px_per_min > 0);

    LV_ASSERT_OBJ(label, &lv_label_class);

    const lv_font_t* font = lv_obj_get_style_text_font(label, LV_PART_MAIN);
    int32_t wait_char_width = LV_LABEL_WAIT_CHAR_COUNT * lv_font_get_glyph_width(font, ' ', ' ');
    uint64_t distance = lv_obj_get_self_width(label) + wait_char_width;
    uint32_t duration = (distance * 60 * 1000) / px_per_min;

    if(duration != lv_obj_get_style_anim_duration(label, LV_PART_MAIN)) {
        lv_obj_set_style_anim_duration(label, duration, LV_PART_MAIN);
        lv_obj_send_event(label, LV_EVENT_STYLE_CHANGED, NULL);
    }
}

void lv_label_ext_set_max_lines(lv_obj_t* label, uint32_t max_lines) {
    furi_check(max_lines > 0);

    LV_ASSERT_OBJ(label, &lv_label_class);

    const lv_font_t* font = lv_obj_get_style_text_font(label, LV_PART_MAIN);
    int32_t line_space = lv_obj_get_style_text_line_space(label, LV_PART_MAIN);
    int32_t max_height = lv_font_get_line_height(font) * max_lines + line_space * (max_lines - 1);

    lv_obj_set_style_max_height(label, max_height, LV_PART_MAIN);
}
