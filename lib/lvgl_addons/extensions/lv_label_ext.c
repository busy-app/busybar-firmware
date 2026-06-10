#include "lv_label_ext.h"

void lv_label_ext_set_anim_speed(lv_obj_t* object, uint32_t speed) {
    furi_check(speed > 0);

    LV_ASSERT_OBJ(object, &lv_label_class);

    lv_obj_update_layout(object);

    const lv_font_t* font = lv_obj_get_style_text_font(object, LV_PART_MAIN);
    int32_t letter_space = lv_obj_get_style_text_letter_space(object, LV_PART_MAIN);
    int32_t line_space = lv_obj_get_style_text_line_space(object, LV_PART_MAIN);

    lv_point_t size;
    const char* text = lv_label_get_text(object);
    lv_text_flag_t flags = lv_label_get_recolor(object) ? LV_TEXT_FLAG_RECOLOR : LV_TEXT_FLAG_NONE;
    lv_text_get_size(&size, text, font, letter_space, line_space, LV_COORD_MAX, flags);

    int32_t wait_char_gap = lv_font_get_glyph_width(font, ' ', ' ') * LV_LABEL_WAIT_CHAR_COUNT;
    uint64_t total_distance = size.x + wait_char_gap;

    uint32_t duration = (total_distance * 60 * 1000) / speed;
    lv_obj_set_style_anim_duration(object, duration, LV_PART_MAIN);
}
