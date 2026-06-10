#include "menu_base.h"
#include "../widget_i.h"

struct MenuBase {
    Widget base;
    lv_group_t* group;
};

extern const lv_obj_class_t menu_base_lvgl_class;
