#include "lv_theme_front.h"
#include "lv_theme_common.h"

#define COLOR_BG_NORMAL  lv_color_black()
#define COLOR_FG_NORMAL  lv_color_hex(0x666666)
#define COLOR_BG_FOCUSED lv_color_black()
#define COLOR_FG_FOCUSED lv_color_white()

#define SCROLLBAR_WIDTH (0)

#define MENU_ITEM_PAD_HOR (2)
#define MENU_ITEM_PAD_VER (0)

typedef struct {
    lv_style_t screen;
    lv_style_t normal;
    lv_style_t focused;
    lv_style_t transparent;
    lv_style_t scrollbar;
    lv_style_t menu_item;
    lv_style_t menu_icon;
    lv_style_t menu_sublabel;
    lv_style_t submenu;
    lv_style_t submenu_cursor;
    lv_style_t timer_label;
    lv_style_t margin_right;
} my_theme_styles_t;

typedef struct {
    lv_theme_t base;
    my_theme_styles_t styles;
} my_theme_t;

static void style_init(my_theme_t* theme) {
    lv_style_init(&theme->styles.screen);
    lv_style_set_bg_opa(&theme->styles.screen, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.screen, COLOR_BG_NORMAL);

    lv_style_init(&theme->styles.normal);
    lv_style_set_text_opa(&theme->styles.normal, LV_OPA_COVER);
    lv_style_set_text_color(&theme->styles.normal, COLOR_FG_NORMAL);
    lv_style_set_text_font(&theme->styles.normal, theme->base.font_normal);

    lv_style_init(&theme->styles.focused);
    lv_style_set_text_opa(&theme->styles.focused, LV_OPA_COVER);
    lv_style_set_text_color(&theme->styles.focused, COLOR_FG_FOCUSED);

    lv_style_init(&theme->styles.transparent);
    lv_style_set_bg_opa(&theme->styles.transparent, LV_OPA_TRANSP);
    lv_style_set_text_opa(&theme->styles.transparent, LV_OPA_TRANSP);
    lv_style_set_image_opa(&theme->styles.transparent, LV_OPA_TRANSP);

    lv_style_init(&theme->styles.menu_item);
    lv_style_set_pad_hor(&theme->styles.menu_item, MENU_ITEM_PAD_HOR);
    lv_style_set_pad_ver(&theme->styles.menu_item, MENU_ITEM_PAD_VER);

    lv_style_init(&theme->styles.menu_icon);
    lv_style_set_image_opa(&theme->styles.menu_icon, LV_OPA_COVER);

    lv_style_init(&theme->styles.menu_sublabel);
    lv_style_set_pad_left(&theme->styles.menu_sublabel, 3);

    lv_style_init(&theme->styles.submenu);
    lv_style_set_pad_row(&theme->styles.submenu, 1);

    lv_style_init(&theme->styles.submenu_cursor);
    lv_style_set_pad_left(&theme->styles.submenu_cursor, 2);
    lv_style_set_pad_right(&theme->styles.submenu_cursor, 1);

    lv_style_init(&theme->styles.timer_label);
    lv_style_set_pad_row(&theme->styles.timer_label, 1);
    lv_style_set_text_color(&theme->styles.timer_label, COLOR_FG_FOCUSED);

    lv_style_init(&theme->styles.scrollbar);
    lv_style_set_bg_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scrollbar, COLOR_FG_FOCUSED);
    lv_style_set_width(&theme->styles.scrollbar, SCROLLBAR_WIDTH);

    lv_style_init(&theme->styles.margin_right);
    lv_style_set_margin_right(&theme->styles.margin_right, 4);
}

static void theme_apply_callback(lv_theme_t* th, lv_obj_t* obj) {
    my_theme_t* theme = (my_theme_t*)th;

    if(lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &theme->styles.screen, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &lv_obj_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &lv_bar_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_INDICATOR);

    } else if(lv_obj_check_type(obj, &widget_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &label_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &menu_lvgl_class)) {
        lv_obj_set_scroll_snap_y(obj, LV_SCROLL_SNAP_CENTER);

    } else if(lv_obj_check_type(obj, &menu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.menu_item, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &menu_icon_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_icon, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.margin_right, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.margin_right, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &menu_sublabel_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_sublabel, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &submenu_lvgl_class)) {
        lv_obj_set_scroll_snap_y(obj, LV_SCROLL_SNAP_CENTER);

        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.submenu, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &submenu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &submenu_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_list_lvgl_class)) {
        lv_obj_set_scroll_snap_y(obj, LV_SCROLL_SNAP_CENTER);

        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &var_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &var_item_editor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_EDITED);

    } else if(lv_obj_check_type(obj, &var_item_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN | LV_STATE_EDITED);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &timer_label_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.timer_label, LV_PART_MAIN);
    }
}

// Public API
lv_theme_t* lv_theme_front_alloc(lv_display_t* disp) {
    my_theme_t* theme = malloc(sizeof(my_theme_t));

    theme->base.disp = disp;
    theme->base.font_small = &lv_font_tiny_6;
    theme->base.font_normal = &lv_font_tiny5_8;
    theme->base.font_large = &lv_font_ark_numerals_regular_10;
    theme->base.apply_cb = theme_apply_callback;

    style_init(theme);

    return (lv_theme_t*)theme;
}
