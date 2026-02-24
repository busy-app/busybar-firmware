#include "overlap_fader.h"

#include "../widget_i.h"

#define MY_CLASS (&overlap_fader_lvgl_class)

struct OverlapFader {
    Widget base;

    lv_grad_dsc_t gradient_dscriptor;
};

typedef struct {
    lv_align_t alignment;
    lv_grad_dir_t grad_dir;
    lv_opa_t opa_start;
    lv_opa_t opa_end;
} SideConfiguration;

const lv_obj_class_t overlap_fader_lvgl_class;

static const SideConfiguration side_configurations[];

/* LVGL-specific code */

static void overlap_fader_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_add_flag(obj, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
}

/* public API */

OverlapFader* overlap_fader_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    return (OverlapFader*)obj;
}

void overlap_fader_free(OverlapFader* instance) {
    furi_check(instance);

    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* overlap_fader_get_base(OverlapFader* instance) {
    furi_check(instance);

    return (Widget*)instance;
}

void overlap_fader_align_to(OverlapFader* instance, Widget* target, OverlapFaderSide side) {
    furi_check(instance);
    furi_check(target);

    const SideConfiguration* side_configuration = &side_configurations[side];

    instance->gradient_dscriptor.dir = side_configuration->grad_dir;
    instance->gradient_dscriptor.stops_count = 2;

    instance->gradient_dscriptor.stops[0].color = lv_color_black();
    instance->gradient_dscriptor.stops[0].frac = 0;
    instance->gradient_dscriptor.stops[0].opa = side_configuration->opa_start;

    instance->gradient_dscriptor.stops[1].color = lv_color_black();
    instance->gradient_dscriptor.stops[1].frac = 255;
    instance->gradient_dscriptor.stops[1].opa = side_configuration->opa_end;

    lv_obj_set_style_bg_grad(TO_LV_OBJ(instance), &instance->gradient_dscriptor, LV_PART_MAIN);
    lv_obj_align_to(TO_LV_OBJ(instance), TO_LV_OBJ(target), side_configuration->alignment, 0, 0);
}

static const SideConfiguration side_configurations[] = {
    [OverlapFaderSideLeft] =
        {
            .alignment = LV_ALIGN_OUT_LEFT_MID,
            .grad_dir = LV_GRAD_DIR_HOR,
            .opa_start = LV_OPA_TRANSP,
            .opa_end = LV_OPA_COVER,
        },
    [OverlapFaderSideRight] =
        {
            .alignment = LV_ALIGN_OUT_RIGHT_MID,
            .grad_dir = LV_GRAD_DIR_HOR,
            .opa_start = LV_OPA_COVER,
            .opa_end = LV_OPA_TRANSP,
        },
    [OverlapFaderSideTop] =
        {
            .alignment = LV_ALIGN_OUT_TOP_MID,
            .grad_dir = LV_GRAD_DIR_VER,
            .opa_start = LV_OPA_TRANSP,
            .opa_end = LV_OPA_COVER,
        },
    [OverlapFaderSideBottom] =
        {
            .alignment = LV_ALIGN_OUT_BOTTOM_MID,
            .grad_dir = LV_GRAD_DIR_VER,
            .opa_start = LV_OPA_COVER,
            .opa_end = LV_OPA_TRANSP,
        },
};

static_assert(COUNT_OF(side_configurations) == OverlapFaderSidesCount);

/* LVGL class descriptor */

const lv_obj_class_t overlap_fader_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = overlap_fader_lvgl_constructor,
    .name = "overlap-fader",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(OverlapFader),
};
