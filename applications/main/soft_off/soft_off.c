#include "soft_off/storage_macros.h"
#include <furi.h>

#include <loader/loader.h>

#include <gui/gui.h>
#include <gui/modules/anim_player.h>
#include <low_power/low_power.h>

typedef enum {
    SoftOffThreadFlagExit = 1 << 0,
    SoftOffThreadFlagAnimationCompleted = 1 << 1,
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

static void soft_off_animation_finished_callback(
    AnimPlayer* anim_player,
    const AnimFileFrameInfo* frame,
    void* context) {
    furi_assert(context);
    UNUSED(anim_player);

    if(frame->flags & AnimFileFrameFlagError) return;
    if(!(frame->flags & AnimFileFrameFlagFinished)) return;

    FuriThread* thread = context;
    furi_thread_flags_set(thread, SoftOffThreadFlagAnimationCompleted);
}

int32_t soft_off_app(void* arg) {
    UNUSED(arg);
    Loader* loader = furi_record_open(RECORD_LOADER);
    loader_set_priority(loader, 0);

    Gui* gui = furi_record_open(RECORD_GUI);
    LowPower* low_power = furi_record_open(RECORD_LOW_POWER);

    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, soft_off_signal_callback, thread);

    AnimPlayer* anim_player;
    with_gui(gui, {
        GuiLayer* layer = gui_get_layer(gui, GuiLayerIdMain);
        Widget* root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);

        anim_player = anim_player_alloc(root);
        anim_player_set_source(anim_player, SOFT_OFF_ANIM_PATH("turn_off_72x16.anim"));
        anim_player_set_frame_callback(anim_player, soft_off_animation_finished_callback, thread);

        anim_player_set_section(anim_player, AnimFilePlayFlagNone, ANIM_FILE_DEFAULT_SECTION);
    });

    while(1) {
        uint32_t flags = furi_thread_flags_wait(
            SoftOffThreadFlagExit | SoftOffThreadFlagAnimationCompleted,
            FuriFlagWaitAny,
            FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0);
        if(flags & SoftOffThreadFlagExit) {
            break;
        }
        if(flags & SoftOffThreadFlagAnimationCompleted) {
            low_power_unlock(low_power);
        }
    }
    with_gui(gui, { anim_player_free(anim_player); });
    low_power_lock(low_power);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_LOADER);
    furi_record_close(RECORD_LOW_POWER);

    return 0;
}
