#pragma once

#include "view_port.h"

#include <lvgl.h>
#include <lvgl/src/core/lv_obj_private.h>

struct ViewPort {
    lv_obj_t obj;
};

static_assert(offsetof(ViewPort, obj) == 0);

extern const lv_obj_class_t lv_view_port_class;
