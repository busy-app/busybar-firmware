#include "soft_off/storage_macros.h"

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/anim_image.h>

#include <back_display/back_display.h>

static bool soft_off_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);
    furi_check(context);
    FuriSemaphore* exit_semaphore = context;

    if(signal == FuriSignalExit) {
        // return value ignored in case we receive multiple exit signals before
        // the thread processes them
        furi_semaphore_release(exit_semaphore);
        return true;
    }

    return false;
}

int32_t soft_off_app(void* arg) {
    UNUSED(arg);

    Gui* gui = furi_record_open(RECORD_GUI);
    BackDisplaySrv* back_display = furi_record_open(RECORD_BACK_DISPLAY);

    back_display_sleep_mode(back_display, true);

    FuriSemaphore* exit_semaphore = furi_semaphore_alloc(1, 0);
    furi_thread_set_signal_callback(
        furi_thread_get_current(), soft_off_signal_callback, exit_semaphore);

    AnimImage* anim_image;
    with_gui(gui, {
        GuiLayer* layer = gui_get_layer(gui, GuiLayerIdMain);
        Widget* root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);

        anim_image = anim_image_alloc(root);
        anim_image_set_source(anim_image, SOFT_OFF_ANIM_PATH("turn_off_72x16.anim"));
        anim_image_set_loop(anim_image, false);
    });

    furi_check(furi_semaphore_acquire(exit_semaphore, FuriWaitForever) == FuriStatusOk);
    furi_thread_set_signal_callback(furi_thread_get_current(), NULL, NULL);
    furi_semaphore_free(exit_semaphore);

    with_gui(gui, { anim_image_free(anim_image); });

    back_display_sleep_mode(back_display, false);

    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_GUI);

    return 0;
}
