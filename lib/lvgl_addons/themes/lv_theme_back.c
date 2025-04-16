#include "lv_theme_back.h"

#include <lvgl.h>
#include <lvgl/src/themes/lv_theme_private.h>

#define COLOR_BG_NORMAL  lv_color_black()
#define COLOR_FG_NORMAL  lv_color_hex(0x666666)
#define COLOR_BG_FOCUSED lv_color_black()
#define COLOR_FG_FOCUSED lv_color_white()

#define SCROLLBAR_WIDTH (3)

#define SUBMENU_ITEM_RADIUS  (3)
#define SUBMENU_ITEM_PAD_VER (4)

#define VAR_LIST_ITEM_PAD_VER (4)

typedef struct {
    lv_style_t screen;
    lv_style_t normal;
    lv_style_t focused;
    lv_style_t inverted;
    lv_style_t scrollbar;
    lv_style_t submenu_item;
    lv_style_t var_list_item;
    lv_style_t margin_right;
} my_theme_styles_t;

typedef struct {
    lv_theme_t base;
    my_theme_styles_t styles;
} my_theme_t;

/** Custom widgets */

// Widget
extern const lv_obj_class_t widget_lvgl_class;
// Label
extern const lv_obj_class_t label_lvgl_class;
// Submenu
extern const lv_obj_class_t submenu_lvgl_class;
extern const lv_obj_class_t submenu_item_lvgl_class;
// VarItemList
extern const lv_obj_class_t var_item_list_lvgl_class;
extern const lv_obj_class_t var_item_lvgl_class;
extern const lv_obj_class_t var_item_editor_lvgl_class;

static void style_init(my_theme_t* theme) {
    lv_style_init(&theme->styles.screen);
    lv_style_set_bg_opa(&theme->styles.screen, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.screen, COLOR_BG_NORMAL);
    lv_style_set_text_color(&theme->styles.screen, COLOR_FG_NORMAL);
    lv_style_set_text_font(&theme->styles.screen, theme->base.font_normal);

    lv_style_init(&theme->styles.normal);
    lv_style_set_bg_color(&theme->styles.normal, COLOR_BG_NORMAL);
    lv_style_set_text_color(&theme->styles.normal, COLOR_FG_NORMAL);
    lv_style_set_text_font(&theme->styles.normal, theme->base.font_normal);

    lv_style_init(&theme->styles.focused);
    lv_style_set_bg_color(&theme->styles.focused, COLOR_BG_FOCUSED);
    lv_style_set_text_color(&theme->styles.focused, COLOR_FG_FOCUSED);

    lv_style_init(&theme->styles.inverted);
    lv_style_set_bg_opa(&theme->styles.inverted, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.inverted, COLOR_FG_FOCUSED);
    lv_style_set_text_color(&theme->styles.inverted, COLOR_BG_FOCUSED);

    lv_style_init(&theme->styles.submenu_item);
    lv_style_set_pad_ver(&theme->styles.submenu_item, SUBMENU_ITEM_PAD_VER);
    lv_style_set_radius(&theme->styles.submenu_item, SUBMENU_ITEM_RADIUS);

    lv_style_init(&theme->styles.var_list_item);
    lv_style_set_pad_ver(&theme->styles.var_list_item, VAR_LIST_ITEM_PAD_VER);

    lv_style_init(&theme->styles.scrollbar);
    lv_style_set_bg_opa(&theme->styles.scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&theme->styles.scrollbar, COLOR_FG_FOCUSED);
    lv_style_set_width(&theme->styles.scrollbar, SCROLLBAR_WIDTH);

    // TODO: This should be handled by the root size
    lv_style_init(&theme->styles.margin_right);
    lv_style_set_margin_right(&theme->styles.margin_right, 12);
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

    } else if(lv_obj_check_type(obj, &submenu_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        // TODO: Remove when root size is fixed
        lv_obj_add_style(obj, &theme->styles.margin_right, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &submenu_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.inverted, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.submenu_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_list_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.scrollbar, LV_PART_SCROLLBAR);
        // TODO: Remove when root size is fixed
        lv_obj_add_style(obj, &theme->styles.margin_right, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &theme->styles.var_list_item, LV_PART_MAIN);

    } else if(lv_obj_check_type(obj, &var_item_editor_lvgl_class)) {
        lv_obj_add_style(obj, &theme->styles.normal, LV_PART_MAIN);
        lv_obj_add_style(obj, &theme->styles.focused, LV_PART_MAIN | LV_STATE_FOCUSED);
    }
}

// Public API
lv_theme_t* lv_theme_back_alloc(lv_display_t* disp) {
    my_theme_t* theme = malloc(sizeof(my_theme_t));

    theme->base.disp = disp;
    theme->base.font_small = &lv_font_tiny_6;
    theme->base.font_normal = &lv_font_cubic_12;
    theme->base.font_large = &lv_font_cubic_12;
    theme->base.apply_cb = theme_apply_callback;

    style_init(theme);

    return (lv_theme_t*)theme;
}
