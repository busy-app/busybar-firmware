#include "../../widget_i.h"

typedef struct {
    Widget base;
    lv_group_t* group;
} MenuBase;

extern const lv_obj_class_t menu_base_lvgl_class;
