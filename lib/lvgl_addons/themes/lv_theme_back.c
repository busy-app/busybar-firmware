#include "lv_theme_back.h"
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

    lv_style_init(&theme->styles.inverted);
    lv_style_set_bg_opa(&theme->styles.inverted, LV_OPA_COVER);
    lv_style_set_text_opa(&theme->styles.inverted, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.inverted, COLOR_FG_FOCUSED);
    lv_style_set_text_color(&theme->styles.inverted, COLOR_BG_FOCUSED);

    lv_style_init(&theme->styles.transparent);
    lv_style_set_bg_opa(&theme->styles.transparent, LV_OPA_TRANSP);
    lv_style_set_text_opa(&theme->styles.transparent, LV_OPA_TRANSP);

    lv_style_init(&theme->styles.subtractive);
    lv_style_set_blend_mode(&theme->styles.subtractive, LV_BLEND_MODE_SUBTRACTIVE);

    lv_style_init(&theme->styles.menu);
    lv_style_set_pad_left(&theme->styles.menu, MENU_PAD_ALL);
    lv_style_set_pad_top(&theme->styles.menu, MENU_PAD_ALL);
    lv_style_set_pad_row(&theme->styles.menu, MENU_PAD_ALL);
    lv_style_set_pad_right(&theme->styles.menu, MENU_PAD_ALL + SCROLLBAR_WIDTH * 2);

    lv_style_init(&theme->styles.menu_item);
    lv_style_set_pad_hor(&theme->styles.menu_item, MENU_ITEM_PAD_HOR);
    lv_style_set_pad_top(&theme->styles.menu_item, MENU_ITEM_PAD_VER);
    lv_style_set_pad_bottom(&theme->styles.menu_item, MENU_ITEM_PAD_VER - 1);
    lv_style_set_radius(&theme->styles.menu_item, MENU_ITEM_RADIUS);

    lv_style_init(&theme->styles.menu_icon);
    lv_style_set_margin_right(&theme->styles.menu_icon, 6);

    lv_style_init(&theme->styles.menu_sublabel);
    lv_style_set_pad_top(&theme->styles.menu_sublabel, MENU_ITEM_PAD_HOR / 2);
    lv_style_set_pad_left(&theme->styles.menu_sublabel, MENU_ITEM_PAD_HOR / 2);
    lv_style_set_text_font(&theme->styles.menu_sublabel, theme->base.font_small);

    lv_style_init(&theme->styles.submenu_item);
    lv_style_set_pad_hor(&theme->styles.submenu_item, MENU_ITEM_PAD_HOR);
    lv_style_set_pad_top(&theme->styles.submenu_item, MENU_ITEM_PAD_VER);
    lv_style_set_pad_bottom(&theme->styles.submenu_item, MENU_ITEM_PAD_VER - 1);
    lv_style_set_pad_column(&theme->styles.submenu_item, SUBMENU_ITEM_PAD_COL);
    lv_style_set_radius(&theme->styles.submenu_item, MENU_ITEM_RADIUS);

    lv_style_init(&theme->styles.var_item);
    lv_style_set_pad_hor(&theme->styles.var_item, MENU_ITEM_PAD_HOR);
    lv_style_set_pad_top(&theme->styles.var_item, MENU_ITEM_PAD_VER);
    lv_style_set_pad_bottom(&theme->styles.var_item, MENU_ITEM_PAD_VER - 1);
    lv_style_set_pad_column(&theme->styles.var_item, SUBMENU_ITEM_PAD_COL);
    lv_style_set_radius(&theme->styles.var_item, MENU_ITEM_RADIUS);

    lv_style_init(&theme->styles.var_item_editor);
    lv_style_set_margin_hor(&theme->styles.var_item_editor, -MENU_ITEM_PAD_HOR);
    lv_style_set_margin_top(&theme->styles.var_item_editor, -MENU_ITEM_PAD_VER);
    lv_style_set_margin_bottom(&theme->styles.var_item_editor, -(MENU_ITEM_PAD_VER - 1));

    lv_style_init(&theme->styles.scrollbar);
    lv_style_set_bg_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scrollbar, COLOR_FG_FOCUSED);
    lv_style_set_width(&theme->styles.scrollbar, SCROLLBAR_WIDTH);

    lv_style_init(&theme->styles.nav_header);
    lv_style_set_pad_all(&theme->styles.nav_header, 2);
    lv_style_set_pad_bottom(&theme->styles.nav_header, 4);
    lv_style_set_pad_column(&theme->styles.nav_header, 2);
    lv_style_set_text_color(&theme->styles.nav_header, COLOR_FG_DIMMED);
    lv_style_set_text_font(&theme->styles.nav_header, theme->base.font_small);

    lv_style_init(&theme->styles.timer_card);
    lv_style_set_bg_opa(&theme->styles.timer_card, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.timer_card, COLOR_FG_FOCUSED);
    lv_style_set_pad_row(&theme->styles.timer_card, 8);
    lv_style_set_pad_ver(&theme->styles.timer_card, 4);
    lv_style_set_translate_y(&theme->styles.timer_card, 2);
    lv_style_set_radius(&theme->styles.timer_card, MENU_ITEM_RADIUS);
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
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &menu_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &menu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.menu_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &menu_icon_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_icon, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.subtractive, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &menu_sublabel_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_sublabel, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &submenu_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &submenu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &submenu_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &var_item_list_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);

    } else if(lv_obj_check_type(obj, &var_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.var_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_editor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.var_item, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.var_item_editor, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_EDITED);

    } else if(lv_obj_check_type(obj, &var_item_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN | LV_STATE_EDITED);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &nav_header_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.nav_header, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &timer_card_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.timer_card, LV_PART_MAIN);
    }
}

// Public API
lv_theme_t* lv_theme_back_alloc(lv_display_t* disp) {
    my_theme_t* theme = malloc(sizeof(my_theme_t));

    theme->base.disp = disp;
    theme->base.font_small = &lv_font_tiny5_8;
    theme->base.font_normal = &lv_font_cubic_12;
    theme->base.font_large = &lv_font_cubic_12;
    theme->base.apply_cb = theme_apply_callback;

    style_init(theme);

    return (lv_theme_t*)theme;
}
