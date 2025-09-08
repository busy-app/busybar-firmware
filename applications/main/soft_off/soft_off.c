#include "soft_off/storage_macros.h"

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/anim_image.h>

#include <back_display/back_display.h>

typedef enum {
    SoftOffThreadFlagExit = 1 << 0,
} SoftOffThreadFlag;

static bool soft_off_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    furi_check(context);

    FuriThreadId* thread_id = context;

    if(signal == FuriSignalExit) {
        furi_thread_flags_set(thread_id, SoftOffThreadFlagExit);
        return true;
    }

    return false;
}

int32_t soft_off_app(void* arg) {
    UNUSED(arg);

    Gui* gui = furi_record_open(RECORD_GUI);
    BackDisplaySrv* back_display = furi_record_open(RECORD_BACK_DISPLAY);

    back_display_sleep_mode(back_display, true);

    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, soft_off_signal_callback, thread);

    AnimImage* anim_image;
    with_gui(gui, {
        GuiLayer* layer = gui_get_layer(gui, GuiLayerIdMain);
        Widget* root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);

        anim_image = anim_image_alloc(root);
        anim_image_set_source(anim_image, SOFT_OFF_ANIM_PATH("turn_off_72x16.anim"));
        anim_image_set_loop(anim_image, false);
    });

    furi_thread_flags_wait(SoftOffThreadFlagExit, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_set_signal_callback(thread, NULL, NULL);

    with_gui(gui, { anim_image_free(anim_image); });

    back_display_sleep_mode(back_display, false);

    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_GUI);

    return 0;
}
