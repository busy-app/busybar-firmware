#include "lv_theme_back_debug.h"
#include "lv_theme_common.h"

#define COLOR_BG_NORMAL  lv_color_black()
#define COLOR_FG_NORMAL  lv_color_hex(0xAAAAAA)
#define COLOR_BG_FOCUSED lv_color_black()
#define COLOR_FG_FOCUSED lv_color_white()
#define COLOR_FG_DIMMED  lv_color_hex(0x777777)

#define SCROLLBAR_WIDTH (3)

#define MENU_PAD_ALL (2)

#define MENU_ITEM_RADIUS  (4)
#define MENU_ITEM_PAD_HOR (4)
#define MENU_ITEM_PAD_VER (4)

#define SUBMENU_ITEM_PAD_COL (3)

typedef struct {
    lv_style_t screen;
    lv_style_t normal;
    lv_style_t focused;
    lv_style_t inverted;
    lv_style_t transparent;
    lv_style_t subtractive;
    lv_style_t scrollbar;
    lv_style_t menu;
    lv_style_t menu_item;
    lv_style_t menu_icon;
    lv_style_t menu_sublabel;
    lv_style_t submenu_item;
    lv_style_t var_item;
    lv_style_t var_item_editor;
    lv_style_t nav_header;
    lv_style_t timer_card;
} my_theme_styles_t;

typedef struct {
    lv_theme_t base;
    my_theme_styles_t styles;
} my_theme_t;

static void style_init(my_theme_t* theme) {
    lv_style_init(&theme->styles.scrollbar);
    lv_style_set_bg_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scrollbar, COLOR_FG_FOCUSED);
    lv_style_set_width(&theme->styles.scrollbar, SCROLLBAR_WIDTH);

    lv_style_init(&theme->styles.screen);
    lv_style_set_bg_opa(&theme->styles.screen, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.screen, COLOR_BG_NORMAL);
    lv_style_set_text_color(&theme->styles.screen, COLOR_FG_NORMAL);

    lv_style_init(&theme->styles.normal);
    lv_style_set_bg_opa(&theme->styles.normal, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.normal, COLOR_BG_NORMAL);
    lv_style_set_text_color(&theme->styles.normal, COLOR_FG_NORMAL);
    lv_style_set_text_font(&theme->styles.normal, theme->base.font_normal);

    lv_style_init(&theme->styles.focused);
    lv_style_set_bg_opa(&theme->styles.focused, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.focused, COLOR_BG_NORMAL);
    lv_style_set_text_color(&theme->styles.focused, COLOR_FG_FOCUSED);
    lv_style_set_text_font(&theme->styles.focused, theme->base.font_normal);
}

static void theme_apply_callback(lv_theme_t* th, lv_obj_t* obj) {
    my_theme_t* theme = (my_theme_t*)th;

    if(lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &theme->styles.screen, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        return;
    }

    if(lv_obj_check_type(obj, &lv_obj_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
    }
#if LV_USE_BUTTON
    else if(lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
    }
#endif
#if LV_USE_BAR
    else if(lv_obj_check_type(obj, &lv_bar_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_INDICATOR);
    }
#endif
#if LV_USE_LIST
    else if(lv_obj_check_type(obj, &lv_list_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &lv_list_text_class)) {
    } else if(lv_obj_check_type(obj, &lv_list_button_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
    }
#endif
    else if(lv_obj_check_type(obj, &widget_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &label_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &submenu_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &submenu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &var_item_list_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &var_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &var_item_editor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
    }
}

// Public API
lv_theme_t* lv_theme_back_debug_alloc(lv_display_t* disp) {
    my_theme_t* theme = malloc(sizeof(my_theme_t));

    theme->base.disp = disp;
    theme->base.font_small = &lv_font_tiny_6;
    theme->base.font_normal = &lv_font_haxrcorp4089_16;
    theme->base.font_large = &lv_font_cubic_12;
    theme->base.apply_cb = theme_apply_callback;

    style_init(theme);

    return (lv_theme_t*)theme;
}
