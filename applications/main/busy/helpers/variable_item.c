#include "variable_item.h"

#include <lvgl/src/core/lv_obj_private.h>
#include <lvgl/src/core/lv_obj_class_private.h>

#include <lvgl/src/widgets/button/lv_button_private.h>

#define MY_CLASS         (&lv_variable_item_class)
#define MY_SPINBOX_CLASS (&lv_variable_item_spinbox_class)

typedef struct {
    lv_obj_t obj;
    lv_obj_t* label;
    lv_obj_t* spinbox;
} lv_variable_item_t;

typedef struct {
    lv_button_t button;
    lv_obj_t* label;
    int32_t min;
    int32_t max;
    int32_t step;
    int32_t value;
    bool binary;
    bool min_as_inf;
} lv_variable_item_spinbox_t;

static void lv_variable_item_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lv_variable_item_spinbox_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lv_variable_item_spinbox_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void lv_variable_item_spinbox_update(lv_variable_item_spinbox_t* spinbox);

const lv_obj_class_t lv_variable_item_class = {
    .constructor_cb = lv_variable_item_constructor,
    .base_class = &lv_obj_class,
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .name = "variable-item",
    .instance_size = sizeof(lv_variable_item_t),
};

const lv_obj_class_t lv_variable_item_spinbox_class = {
    .base_class = &lv_button_class,
    .constructor_cb = lv_variable_item_spinbox_constructor,
    .event_cb = lv_variable_item_spinbox_event,
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .name = "variable-item-spinbox",
    .instance_size = sizeof(lv_variable_item_spinbox_t),
};

static void variable_item_event_callback(lv_event_t* event) {
    lv_group_t* group = lv_event_get_user_data(event);
    lv_group_set_editing(group, true);

    for(lv_indev_t* indev = lv_indev_get_next(NULL); indev != NULL;
        indev = lv_indev_get_next(indev)) {
        lv_indev_set_group(indev, group);
    }
}

lv_obj_t* lv_variable_item_add(lv_obj_t* parent, const char* text) {
    lv_obj_t* item = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(item);

    lv_variable_item_set_text(item, text);

    return item;
}

void lv_variable_item_set_text(lv_obj_t* obj, const char* text) {
    LV_ASSERT_OBJ(MY_CLASS, obj);

    lv_variable_item_t* item = (void*)obj;
    lv_label_set_text(item->label, text);
}

void lv_variable_item_set_range(lv_obj_t* obj, int32_t min, int32_t max) {
    LV_ASSERT_OBJ(MY_CLASS, obj);

    lv_variable_item_t* item = (void*)obj;
    lv_variable_item_spinbox_t* spinbox = (void*)item->spinbox;

    spinbox->min = min;
    spinbox->max = max;

    if(spinbox->value < spinbox->min) {
        spinbox->value = spinbox->min;
    }
    if(spinbox->value > spinbox->max) {
        spinbox->value = spinbox->max;
    }

    lv_variable_item_spinbox_update(spinbox);
}

void lv_variable_item_set_step(lv_obj_t* obj, int32_t step) {
    LV_ASSERT_OBJ(MY_CLASS, obj);

    lv_variable_item_t* item = (void*)obj;
    lv_variable_item_spinbox_t* spinbox = (void*)item->spinbox;

    spinbox->step = step;
    spinbox->value = spinbox->value - (spinbox->value % step);

    lv_variable_item_spinbox_update(spinbox);
}

void lv_variable_item_set_value(lv_obj_t* obj, int32_t value) {
    LV_ASSERT_OBJ(MY_CLASS, obj);

    lv_variable_item_t* item = (void*)obj;
    lv_variable_item_spinbox_t* spinbox = (void*)item->spinbox;

    if(value < spinbox->min) {
        value = spinbox->min;
    }
    if(value > spinbox->max) {
        value = spinbox->max;
    }

    spinbox->value = value - (value % spinbox->step);

    lv_variable_item_spinbox_update(spinbox);
}

void lv_variable_item_set_binary(lv_obj_t* obj, bool set) {
    LV_ASSERT_OBJ(MY_CLASS, obj);

    lv_variable_item_t* item = (void*)obj;
    lv_variable_item_spinbox_t* spinbox = (void*)item->spinbox;

    if(spinbox->binary == set) {
        return;
    }

    spinbox->binary = set;

    if(set) {
        spinbox->value = !!spinbox->value;
        spinbox->min = 0;
        spinbox->max = 1;
        spinbox->step = 1;
    }

    lv_variable_item_spinbox_update(spinbox);
}

void lv_variable_item_set_min_as_inf(lv_obj_t* obj, bool set) {
    LV_ASSERT_OBJ(MY_CLASS, obj);

    lv_variable_item_t* item = (void*)obj;
    lv_variable_item_spinbox_t* spinbox = (void*)item->spinbox;

    if(spinbox->min_as_inf == set) {
        return;
    }

    spinbox->min_as_inf = set;

    lv_variable_item_spinbox_update(spinbox);
}

static void lv_variable_item_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_variable_item_t* item = (lv_variable_item_t*)obj;

    lv_obj_t* button = lv_button_create(obj);
    item->label = lv_label_create(button);

    lv_group_t* group = lv_group_create();
    lv_obj_add_event_cb(button, variable_item_event_callback, LV_EVENT_SHORT_CLICKED, group);

    item->spinbox = lv_obj_class_create_obj(MY_SPINBOX_CLASS, obj);
    lv_obj_class_init_obj(item->spinbox);
    lv_obj_set_pos(item->spinbox, 38, 0);

    lv_group_add_obj(group, item->spinbox);
}

static void lv_variable_item_spinbox_update(lv_variable_item_spinbox_t* spinbox) {
    lv_obj_t* label = spinbox->label;
    const int32_t value = spinbox->value;

    if(spinbox->binary) {
        if(value == spinbox->min) {
            lv_label_set_text(label, "◃ OFF ▹");
        } else {
            lv_label_set_text(label, "◃ ON ▹");
        }

    } else {
        if(value == spinbox->min && spinbox->min_as_inf) {
            lv_label_set_text(label, "◃ ∞ ▹");
        } else {
            const int32_t hh = value / 60;
            const int32_t mm = value % 60;

            if(hh == 0) {
                lv_label_set_text_fmt(label, "◃ %ld m ▹", mm);
            } else if(mm == 0) {
                lv_label_set_text_fmt(label, "◃ %ld h ▹", hh);
            } else {
                lv_label_set_text_fmt(label, "◃ %ld:%02ld ▹", hh, mm);
            }
        }
    }
}

static void lv_variable_item_spinbox_increment(lv_variable_item_spinbox_t* spinbox) {
    if(spinbox->value < spinbox->max) {
        spinbox->value += spinbox->step;
        lv_variable_item_spinbox_update(spinbox);
    }
}

static void lv_variable_item_spinbox_decrement(lv_variable_item_spinbox_t* spinbox) {
    if(spinbox->value > spinbox->min) {
        spinbox->value -= spinbox->step;
        lv_variable_item_spinbox_update(spinbox);
    }
}

static void lv_variable_item_spinbox_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    // TODO: Set this in theme
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_EDITED);

    lv_variable_item_spinbox_t* spinbox = (lv_variable_item_spinbox_t*)obj;

    spinbox->label = lv_label_create(obj);
    spinbox->min = 0;
    spinbox->max = 99;
    spinbox->step = 1;
    spinbox->value = 0;

    lv_variable_item_spinbox_update(spinbox);
}

static void lv_variable_item_spinbox_finish_editing(lv_variable_item_spinbox_t* spinbox) {
    lv_group_t* group = lv_obj_get_group((lv_obj_t*)spinbox);
    lv_group_set_editing(group, false);

    for(lv_indev_t* indev = lv_indev_get_next(NULL); indev != NULL;
        indev = lv_indev_get_next(indev)) {
        lv_indev_set_group(indev, lv_group_get_default());
    }

    lv_obj_t* item = lv_obj_get_parent((lv_obj_t*)spinbox);
    lv_obj_send_event(item, LV_EVENT_VALUE_CHANGED, &spinbox->value);
}

static void lv_variable_item_spinbox_event(const lv_obj_class_t* class_p, lv_event_t* event) {
    LV_UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_CLASS, event);
    if(res != LV_RESULT_OK) return;

    lv_obj_t* obj = lv_event_get_target(event);
    lv_variable_item_spinbox_t* spinbox = (void*)obj;

    const lv_event_code_t event_code = lv_event_get_code(event);

    if(event_code == LV_EVENT_SHORT_CLICKED) {
        lv_variable_item_spinbox_finish_editing(spinbox);

    } else if(event_code == LV_EVENT_KEY) {
        const uint32_t key = *((uint32_t*)lv_event_get_param(event));

        if(key == LV_KEY_RIGHT) {
            lv_variable_item_spinbox_increment(spinbox);
        } else if(key == LV_KEY_LEFT) {
            lv_variable_item_spinbox_decrement(spinbox);
        } else if(key == LV_KEY_ESC) {
            lv_variable_item_spinbox_finish_editing(spinbox);
        }

    } else if(event_code == LV_EVENT_DELETE) {
        lv_group_t* group = lv_obj_get_group(obj);
        LV_ASSERT(group != lv_group_get_default());
        lv_group_delete(group);
    }
}
