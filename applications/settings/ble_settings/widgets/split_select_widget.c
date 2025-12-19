#include "split_select_widget.h"

#include <gui/widget_i.h>

#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>
#include <gui/modules/submenu.h>

struct SplitWidget {
    FlexLayout* flex;
    Label* text;
    Submenu* submenu;
};

SplitWidget* split_widget_alloc(Widget* base, const char* title) {
    furi_assert(base);
    SplitWidget* instance = malloc(sizeof(SplitWidget));

    instance->flex = flex_layout_alloc(base, FlexLayoutTypeRow);
    widget_set_size(flex_layout_get_base(instance->flex), FRONT_DISPLAY_W, FRONT_DISPLAY_H);

    instance->text = label_alloc(flex_layout_get_base(instance->flex));
    widget_set_width(label_get_base(instance->text), FRONT_DISPLAY_W / 2);
    widget_set_height(label_get_base(instance->text), FRONT_DISPLAY_H);
    widget_set_align(label_get_base(instance->text), AlignLeftMid);

    label_set_long_content_mode(instance->text, LabelLongContentModeWrap, 1000);

    label_set_text(instance->text, title);

    instance->submenu = submenu_alloc(flex_layout_get_base(instance->flex));
    widget_set_align(submenu_get_base(instance->submenu), AlignTopRight);
    widget_set_width(submenu_get_base(instance->submenu), FRONT_DISPLAY_W / 2);

    return instance;
}

void split_widget_free(SplitWidget* instance) {
    furi_assert(instance);

    label_free(instance->text);
    submenu_free(instance->submenu);
    flex_layout_free(instance->flex);
}

void split_widget_add_button(
    SplitWidget* instance,
    const char* text,
    uint32_t index,
    WidgetButtonCallback callback,
    void* ctx) {
    furi_assert(instance);
    furi_assert(text);

    submenu_add_item(instance->submenu, text, index, callback, ctx);
}
