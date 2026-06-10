#include "lv_theme_back.h"
#include "lv_theme_common.h"
#include <font_registry/font_registry.h>

#define COLOR_BG_NORMAL   lv_color_black()
#define COLOR_FG_NORMAL   lv_color_hex(0xAAAAAA)
#define COLOR_BG_FOCUSED  lv_color_black()
#define COLOR_FG_FOCUSED  lv_color_white()
#define COLOR_FG_DISABLED lv_color_hex(0x444444)

#define SCROLLBAR_WIDTH (1)

#define MENU_PAD_ALL (2)

#define MENU_ITEM_RADIUS  (4)
#define MENU_ITEM_PAD_HOR (3)
#define MENU_ITEM_PAD_VER (2)

#define SUBMENU_ITEM_PAD_COL (3)

#define QR_CODE_CARD_RADIUS (4)

typedef struct {
    lv_style_t screen;
    lv_style_t normal;
    lv_style_t focused;
    lv_style_t disabled;
    lv_style_t inverted;
    lv_style_t transparent;
    lv_style_t scrollbar;

    lv_style_t menu;
    lv_style_t menu_item;
    lv_style_t menu_icon;
    lv_style_t menu_arrow;

    lv_style_t submenu_item;
    lv_style_t submenu_cursor;

    lv_style_t var_item;
    lv_style_t var_item_editor;

    lv_style_t dialog;
    lv_style_t dialog_text_main;
    lv_style_t dialog_text_sub;
    lv_style_t dialog_cursor;
    lv_style_t dialog_option;

    lv_style_t title_card_label;
    lv_style_t title_card_container;

    lv_style_t progress_bar;
    lv_style_t progress_bar_fill;

    lv_style_t qr_code_card;
} my_theme_styles_t;

typedef struct {
    lv_theme_t base;
    my_theme_styles_t styles;
} my_theme_t;

static void style_init(my_theme_t* theme, FontRegistry* font_registry) {
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
    lv_style_set_text_opa(&theme->styles.disabled, LV_OPA_COVER);
    lv_style_set_text_color(&theme->styles.disabled, COLOR_FG_DISABLED);

    lv_style_init(&theme->styles.inverted);
    lv_style_set_bg_opa(&theme->styles.inverted, LV_OPA_COVER);
    lv_style_set_text_opa(&theme->styles.inverted, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.inverted, COLOR_FG_FOCUSED);
    lv_style_set_text_color(&theme->styles.inverted, COLOR_BG_FOCUSED);
    lv_style_set_image_recolor(&theme->styles.inverted, COLOR_BG_NORMAL);

    lv_style_init(&theme->styles.transparent);
    lv_style_set_bg_opa(&theme->styles.transparent, LV_OPA_TRANSP);
    lv_style_set_text_opa(&theme->styles.transparent, LV_OPA_TRANSP);

    lv_style_init(&theme->styles.scrollbar);
    lv_style_set_width(&theme->styles.scrollbar, SCROLLBAR_WIDTH);
    lv_style_set_pad_right(&theme->styles.scrollbar, 1);
    lv_style_set_length(&theme->styles.scrollbar, 5);
    lv_style_set_bg_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scrollbar, COLOR_FG_FOCUSED);
    lv_style_set_outline_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_outline_color(&theme->styles.scrollbar, COLOR_FG_DISABLED);

    lv_style_init(&theme->styles.menu);
    lv_style_set_pad_left(&theme->styles.menu, MENU_PAD_ALL);
    lv_style_set_pad_top(&theme->styles.menu, MENU_PAD_ALL);
    lv_style_set_pad_row(&theme->styles.menu, MENU_PAD_ALL);
    lv_style_set_pad_right(&theme->styles.menu, MENU_PAD_ALL + 4);

    lv_style_init(&theme->styles.menu_item);
    lv_style_set_pad_hor(&theme->styles.menu_item, MENU_ITEM_PAD_HOR);
    lv_style_set_pad_top(&theme->styles.menu_item, MENU_ITEM_PAD_VER);
    lv_style_set_pad_bottom(&theme->styles.menu_item, MENU_ITEM_PAD_VER - 1);
    lv_style_set_radius(&theme->styles.menu_item, MENU_ITEM_RADIUS);
    lv_style_set_flex_cross_place(&theme->styles.menu_item, LV_FLEX_ALIGN_CENTER);

    lv_style_init(&theme->styles.menu_icon);
    lv_style_set_margin_right(&theme->styles.menu_icon, 6);
    lv_style_set_image_recolor(&theme->styles.menu_icon, COLOR_FG_NORMAL);
    lv_style_set_image_recolor_opa(&theme->styles.menu_icon, LV_OPA_COVER);

    lv_style_init(&theme->styles.menu_arrow);
    lv_style_set_pad_left(&theme->styles.menu_arrow, MENU_ITEM_PAD_HOR / 2);
    lv_style_set_text_font(&theme->styles.menu_arrow, theme->base.font_normal);

    lv_style_init(&theme->styles.submenu_item);
    lv_style_set_pad_hor(&theme->styles.submenu_item, MENU_ITEM_PAD_HOR);
    lv_style_set_pad_top(&theme->styles.submenu_item, MENU_ITEM_PAD_VER);
    lv_style_set_pad_bottom(&theme->styles.submenu_item, MENU_ITEM_PAD_VER - 1);
    lv_style_set_pad_column(&theme->styles.submenu_item, SUBMENU_ITEM_PAD_COL);
    lv_style_set_radius(&theme->styles.submenu_item, MENU_ITEM_RADIUS);

    lv_style_init(&theme->styles.submenu_cursor);
    lv_style_set_width(&theme->styles.submenu_cursor, 0);
    lv_style_set_margin_right(&theme->styles.submenu_cursor, -SUBMENU_ITEM_PAD_COL);

    lv_style_init(&theme->styles.var_item);
    lv_style_set_pad_hor(&theme->styles.var_item, MENU_ITEM_PAD_HOR);
    lv_style_set_pad_top(&theme->styles.var_item, MENU_ITEM_PAD_VER);
    lv_style_set_pad_bottom(&theme->styles.var_item, MENU_ITEM_PAD_VER - 1);
    lv_style_set_pad_column(&theme->styles.var_item, SUBMENU_ITEM_PAD_COL);
    lv_style_set_radius(&theme->styles.var_item, MENU_ITEM_RADIUS);

    lv_style_init(&theme->styles.var_item_editor);
    lv_style_set_pad_column(&theme->styles.var_item_editor, 4);
    lv_style_set_margin_hor(&theme->styles.var_item_editor, -MENU_ITEM_PAD_HOR);
    lv_style_set_margin_top(&theme->styles.var_item_editor, -MENU_ITEM_PAD_VER);
    lv_style_set_margin_bottom(&theme->styles.var_item_editor, -(MENU_ITEM_PAD_VER - 1));

    lv_style_init(&theme->styles.dialog);
    lv_style_set_flex_flow(&theme->styles.dialog, LV_FLEX_FLOW_COLUMN);
    lv_style_set_flex_main_place(&theme->styles.dialog, LV_FLEX_ALIGN_START);
    lv_style_set_flex_cross_place(&theme->styles.dialog, LV_FLEX_ALIGN_START);
    lv_style_set_flex_track_place(&theme->styles.dialog, LV_FLEX_ALIGN_START);
    lv_style_set_layout(&theme->styles.dialog, LV_LAYOUT_FLEX);
    lv_style_set_pad_left(&theme->styles.dialog, MENU_PAD_ALL);
    lv_style_set_pad_top(&theme->styles.dialog, MENU_PAD_ALL);
    lv_style_set_pad_row(&theme->styles.dialog, MENU_PAD_ALL);
    lv_style_set_pad_right(&theme->styles.dialog, MENU_PAD_ALL);

    lv_style_init(&theme->styles.dialog_text_main);
    lv_style_set_flex_grow(&theme->styles.dialog_text_main, 1);
    lv_style_set_text_font(
        &theme->styles.dialog_text_main,
        font_registry_load_font(font_registry, FONT_BUSY_REGULAR_7));
    lv_style_set_margin_bottom(&theme->styles.dialog_text_main, 2);

    lv_style_init(&theme->styles.dialog_text_sub);
    lv_style_set_flex_grow(&theme->styles.dialog_text_sub, 0);
    lv_style_set_max_width(&theme->styles.dialog_text_sub, LV_PCT(60));
    lv_style_set_text_font(
        &theme->styles.dialog_text_sub,
        font_registry_load_font(font_registry, FONT_BUSY_REGULAR_7));

    lv_style_init(&theme->styles.dialog_option);
    lv_style_set_width(&theme->styles.dialog_option, LV_PCT(100));

    lv_style_init(&theme->styles.dialog_cursor);
    lv_style_set_width(&theme->styles.dialog_cursor, LV_SIZE_CONTENT);
    lv_style_set_margin_right(&theme->styles.dialog_cursor, 0);
    lv_style_set_pad_left(&theme->styles.dialog_cursor, 3);

    lv_style_init(&theme->styles.title_card_label);
    lv_style_set_text_color(&theme->styles.title_card_label, COLOR_FG_FOCUSED);
    lv_style_set_text_font(
        &theme->styles.title_card_label,
        font_registry_load_font(font_registry, FONT_BUSY_REGULAR_14));

    lv_style_init(&theme->styles.title_card_container);
    lv_style_set_layout(&theme->styles.title_card_container, LV_LAYOUT_FLEX);
    lv_style_set_flex_flow(&theme->styles.title_card_container, LV_FLEX_FLOW_ROW);
    lv_style_set_flex_cross_place(&theme->styles.title_card_container, LV_FLEX_ALIGN_CENTER);
    lv_style_set_pad_column(&theme->styles.title_card_container, 6);

    lv_style_init(&theme->styles.qr_code_card);
    lv_style_set_radius(&theme->styles.qr_code_card, QR_CODE_CARD_RADIUS);
    lv_style_set_pad_all(&theme->styles.qr_code_card, 4);
    lv_style_set_bg_opa(&theme->styles.qr_code_card, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.qr_code_card, COLOR_FG_FOCUSED);

    lv_style_init(&theme->styles.progress_bar);
    lv_style_set_bg_opa(&theme->styles.progress_bar, LV_OPA_20);
    lv_style_set_radius(&theme->styles.progress_bar, 3);

    lv_style_init(&theme->styles.progress_bar_fill);
    lv_style_set_bg_opa(&theme->styles.progress_bar_fill, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.progress_bar_fill, COLOR_FG_FOCUSED);
    lv_style_set_radius(&theme->styles.progress_bar_fill, 3);
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
        lv_obj_add_style(obj, &theme->styles.menu, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &menu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.menu_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &menu_icon_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_icon, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &menu_sublabel_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &menu_arrow_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu_arrow, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.transparent, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &submenu_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &submenu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &submenu_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_list_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.menu, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.var_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_editor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.var_item, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.var_item_editor, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_EDITED);

    } else if(lv_obj_check_type(obj, &var_item_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_arrow_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &theme->styles.menu_arrow, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.menu_arrow, LV_PART_MAIN | LV_STATE_DISABLED);

    } else if(lv_obj_check_type(obj, &dialog_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.dialog, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_text_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.dialog_text_main, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_text_sub_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.dialog_text_sub, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_option_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_item, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.dialog_option, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &dialog_cursor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.submenu_cursor, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.dialog_cursor, LV_PART_MAIN | LV_STATE_FOCUSED);

    } else if(lv_obj_check_type(obj, &progress_bar_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.progress_bar, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &progress_bar_fill_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.progress_bar_fill, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &title_card_label_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.title_card_label, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &title_card_container_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.title_card_container, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &qr_code_card_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.qr_code_card, LV_PART_MAIN);

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
lv_theme_t* lv_theme_back_alloc(lv_display_t* disp) {
    my_theme_t* theme = malloc(sizeof(my_theme_t));
    FontRegistry* font_registry = furi_record_open(RECORD_FONT_REGISTRY);

    theme->base.disp = disp;
    theme->base.font_small = font_registry_load_font(font_registry, FONT_BUSY_REGULAR_5);
    theme->base.font_normal = font_registry_load_font(font_registry, FONT_BUSY_REGULAR_9);
    theme->base.font_large = font_registry_load_font(font_registry, FONT_BUSY_BOLD_10);
    theme->base.apply_cb = theme_apply_callback;

    style_init(theme, font_registry);

    return (lv_theme_t*)theme;
}
