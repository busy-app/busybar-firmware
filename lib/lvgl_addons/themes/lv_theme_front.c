#include "lv_theme_front.h"
#include "lv_theme_common.h"
#include <font_registry/font_registry.h>

#define COLOR_BG_NORMAL           lv_color_black()
#define COLOR_FG_NORMAL           lv_color_hex(0x666666)
#define COLOR_BG_FOCUSED          lv_color_black()
#define COLOR_FG_FOCUSED          lv_color_white()
#define PROGRESS_BAR_FILL_COLOR_1 lv_color_hex(0x104224)
#define PROGRESS_BAR_FILL_COLOR_2 lv_color_hex(0x16A34A)
#define PROGRESS_BAR_BG_COLOR     lv_color_hex(0x333333)

#define SCROLLBAR_WIDTH (1)

#define MENU_SUBLABEL_MAX_WIDTH (26)

typedef struct {
    lv_style_t screen;
    lv_style_t normal;
    lv_style_t focused;
    lv_style_t disabled;
    lv_style_t transparent;
    lv_style_t transparent_all;
    lv_style_t scrollbar;

    lv_style_t menu_item;
    lv_style_t menu_icon;
    lv_style_t menu_icon_animated;
    lv_style_t menu_sublabel;
    lv_style_t menu_arrow;

    lv_style_t submenu;
    lv_style_t submenu_item;
    lv_style_t submenu_cursor;

    lv_style_t dialog;
    lv_style_t dialog_text;

    lv_style_t var_item;
    lv_style_t var_item_editor;

    lv_style_t margin_right;

    lv_style_t title_card;

    lv_style_t progress_bar;
    lv_style_t progress_bar_fill;
} my_theme_styles_t;

typedef struct {
    lv_theme_t base;
    my_theme_styles_t styles;
} my_theme_t;

static void style_init(my_theme_t* theme, FontRegistry* font_registry) {
    UNUSED(font_registry);

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

    lv_style_init(&theme->styles.disabled);
    lv_style_set_text_opa(&theme->styles.disabled, LV_OPA_50);
    lv_style_set_image_opa(&theme->styles.disabled, LV_OPA_50);

    lv_style_init(&theme->styles.transparent);
    lv_style_set_bg_opa(&theme->styles.transparent, LV_OPA_TRANSP);
    lv_style_set_text_opa(&theme->styles.transparent, LV_OPA_TRANSP);
    lv_style_set_image_opa(&theme->styles.transparent, LV_OPA_TRANSP);

    lv_style_init(&theme->styles.transparent_all);
    lv_style_set_opa(&theme->styles.transparent_all, LV_OPA_TRANSP);

    lv_style_init(&theme->styles.scrollbar);
    lv_style_set_width(&theme->styles.scrollbar, SCROLLBAR_WIDTH);
    lv_style_set_length(&theme->styles.scrollbar, 3);
    lv_style_set_bg_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scrollbar, COLOR_FG_FOCUSED);
    lv_style_set_outline_opa(&theme->styles.scrollbar, LV_OPA_20);
    lv_style_set_outline_color(&theme->styles.scrollbar, COLOR_FG_FOCUSED);

    lv_style_init(&theme->styles.menu_item);
    lv_style_set_margin_top(&theme->styles.menu_item, -1);

    lv_style_init(&theme->styles.menu_icon);
    lv_style_set_image_opa(&theme->styles.menu_icon, LV_OPA_COVER);

    lv_style_init(&theme->styles.menu_icon_animated);
    lv_style_set_opa(&theme->styles.menu_icon_animated, LV_OPA_COVER);

    lv_style_init(&theme->styles.menu_sublabel);
    lv_style_set_max_width(&theme->styles.menu_sublabel, MENU_SUBLABEL_MAX_WIDTH);

    lv_style_init(&theme->styles.menu_arrow);
    lv_style_set_pad_hor(&theme->styles.menu_arrow, 1);
    lv_style_set_text_font(&theme->styles.menu_arrow, theme->base.font_normal);

    lv_style_init(&theme->styles.submenu);
    lv_style_set_pad_row(&theme->styles.submenu, 1);
    lv_style_set_pad_right(&theme->styles.submenu, 2);

    lv_style_init(&theme->styles.submenu_item);
    lv_style_set_margin_top(&theme->styles.submenu_item, -2);

    lv_style_init(&theme->styles.submenu_cursor);
    lv_style_set_pad_hor(&theme->styles.submenu_cursor, 1);
    lv_style_set_text_font(&theme->styles.submenu_cursor, theme->base.font_normal);

    lv_style_init(&theme->styles.dialog);
    lv_style_set_flex_flow(&theme->styles.dialog, LV_FLEX_FLOW_ROW);
    lv_style_set_flex_main_place(&theme->styles.dialog, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_style_set_flex_cross_place(&theme->styles.dialog, LV_FLEX_ALIGN_CENTER);
    lv_style_set_flex_track_place(&theme->styles.dialog, LV_FLEX_ALIGN_CENTER);
    lv_style_set_layout(&theme->styles.dialog, LV_LAYOUT_FLEX);

    lv_style_init(&theme->styles.dialog_text);
    lv_style_set_pad_ver(&theme->styles.dialog_text, 0);
    lv_style_set_width(&theme->styles.dialog_text, LV_PCT(100));
    lv_style_set_text_line_space(&theme->styles.dialog_text, -2);
    lv_style_set_max_height(&theme->styles.dialog_text, LV_PCT(100));

    lv_style_init(&theme->styles.var_item);
    lv_style_set_margin_top(&theme->styles.var_item, -2);
    lv_style_set_text_line_space(&theme->styles.var_item, -2);

    lv_style_init(&theme->styles.var_item_editor);
    lv_style_set_pad_column(&theme->styles.var_item_editor, 1);
    lv_style_set_pad_bottom(&theme->styles.var_item_editor, 1);
    lv_style_set_pad_right(&theme->styles.var_item_editor, 1);
    lv_style_set_pad_left(&theme->styles.var_item_editor, 2);

    lv_style_init(&theme->styles.margin_right);
    lv_style_set_margin_right(&theme->styles.margin_right, 2);

    lv_style_init(&theme->styles.title_card);
    lv_style_set_pad_column(&theme->styles.title_card, 2);
    lv_style_set_text_font(&theme->styles.title_card, theme->base.font_large);

    lv_style_init(&theme->styles.progress_bar);
    lv_style_set_bg_opa(&theme->styles.progress_bar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.progress_bar, PROGRESS_BAR_BG_COLOR);

    lv_style_init(&theme->styles.progress_bar_fill);
    lv_style_set_bg_opa(&theme->styles.progress_bar_fill, LV_OPA_COVER);
    lv_style_set_bg_grad_dir(&theme->styles.progress_bar_fill, LV_GRAD_DIR_HOR);
    lv_style_set_bg_color(&theme->styles.progress_bar_fill, PROGRESS_BAR_FILL_COLOR_1);
    lv_style_set_bg_grad_color(&theme->styles.progress_bar_fill, PROGRESS_BAR_FILL_COLOR_2);
}

static void theme_apply_callback(lv_theme_t* th, lv_obj_t* obj) {
    my_theme_t* theme = (my_theme_t*)th;

    if(lv_obj_has_class(obj, &widget_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
    }

    if(lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &theme->styles.screen, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &lv_obj_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);

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

    } else if(lv_obj_check_type(obj, &menu_icon_animated_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_icon_animated, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.transparent_all, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.margin_right, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.margin_right, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &menu_sublabel_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_sublabel, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &menu_arrow_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_arrow, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &submenu_lvgl_class)) {
        lv_obj_set_scroll_snap_y(obj, LV_SCROLL_SNAP_CENTER);

        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.submenu, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &submenu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.submenu_item, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &submenu_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.dialog, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_text_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.dialog_text, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_text_sub_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.dialog_text, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_option_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &dialog_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);
    } else if(lv_obj_check_type(obj, &var_item_list_lvgl_class)) {
        lv_obj_set_scroll_snap_y(obj, LV_SCROLL_SNAP_CENTER);

        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.var_item, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &var_item_editor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_EDITED);
        lv_obj_add_style(obj, &theme->styles.var_item_editor, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &progress_bar_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.progress_bar, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &progress_bar_fill_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.progress_bar_fill, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN | LV_STATE_EDITED);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_arrow_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN | LV_STATE_DISABLED);

    } else if(lv_obj_check_type(obj, &anim_title_card_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.title_card, LV_PART_MAIN);

#ifdef APP_BUSY
    } else if(lv_obj_check_type(obj, &countdown_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &theme_picker_arrow_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

#endif // APP_BUSY

#ifdef APP_SETTINGS_WIFI
    } else if(lv_obj_check_type(obj, &wifi_info_view_arrow_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

#endif // APP_SETTINGS_WIFI
    }
}

// Public API
lv_theme_t* lv_theme_front_alloc(lv_display_t* disp) {
    my_theme_t* theme = malloc(sizeof(my_theme_t));
    FontRegistry* font_registry = furi_record_open(RECORD_FONT_REGISTRY);

    theme->base.disp = disp;
    theme->base.font_small = font_registry_load_font(font_registry, FONT_BUSY_REGULAR_5);
    theme->base.font_normal = font_registry_load_font(font_registry, FONT_BUSY_REGULAR_5);
    theme->base.font_large = font_registry_load_font(font_registry, FONT_BUSY_REGULAR_7);
    theme->base.apply_cb = theme_apply_callback;

    style_init(theme, font_registry);

    return (lv_theme_t*)theme;
}
